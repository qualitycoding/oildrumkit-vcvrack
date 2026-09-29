#!/usr/bin/env python3
# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
# T-051 panel SVG, T-055 layout constraints, T-056 generated files up to date (plan/DECISIONS.md D-013;
# C-019: Rack panel rules — mm units, 128.5 mm high, width = HP x 5.08 mm, no text/CSS; C-020 widget sizes).
import json, os, subprocess, sys, tempfile, filecmp
import xml.etree.ElementTree as ET
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PX_MM = 25.4 / 75.0                      # Rack: 1 HP = 15 px = 5.08 mm
WIDGET_MM = {"PJ301MPort": (23.7 * PX_MM, 23.7 * PX_MM), "RoundBlackKnob": (28.34759 * PX_MM, 28.34768 * PX_MM),
             "CKSSThree": (13.457 * PX_MM, 28.34766 * PX_MM), "CKSS": (14.0 * PX_MM, 20.64111 * PX_MM)}
EXPECTED = {**{k: ("param", "RoundBlackKnob") for k in ("DAMP", "STRIKE", "ROOM", "TUNE")},
            "HAMMER": ("param", "CKSSThree"), "LIMITER": ("param", "CKSS"),
            **{f"TRIG_{i}": ("input", "PJ301MPort") for i in range(15)},
            **{k: ("input", "PJ301MPort") for k in ("DAMP_CV", "STRIKE_CV", "ROOM_CV", "VOCT")},
            "LEFT": ("output", "PJ301MPort"), "RIGHT": ("output", "PJ301MPort")}
results = []
def record(tid, name, fails): results.append((tid, name, fails))

def load_layout():
    p = os.path.join(ROOT, "tools", "layout.json")
    if not os.path.isfile(p): return None, "tools/layout.json missing"
    try: return json.load(open(p, encoding="utf-8")), None
    except Exception as e: return None, f"layout.json invalid: {e}"

def t051(lay):
    f = []
    p = os.path.join(ROOT, "res", "OilDrumKit.svg")
    if not os.path.isfile(p): return ["res/OilDrumKit.svg missing"]
    if os.path.getsize(p) > 1_000_000: f.append("panel > 1 MB")
    try: root = ET.parse(p).getroot()
    except Exception as e: return [f"SVG does not parse: {e}"]
    hp = lay["hp"] if lay else None
    w, h = root.get("width", ""), root.get("height", "")
    if not (w.endswith("mm") and h.endswith("mm")): f.append("width/height must be in mm")
    else:
        if hp is None or abs(float(w[:-2]) - hp * 5.08) > 1e-3: f.append(f"width {w} != hp*5.08 mm")
        if abs(float(h[:-2]) - 128.5) > 1e-3: f.append(f"height {h} != 128.5mm")
        vb = [float(x) for x in root.get("viewBox", "").replace(",", " ").split()] if root.get("viewBox") else []
        if len(vb) != 4 or vb[0] != 0 or vb[1] != 0 or abs(vb[2] - float(w[:-2])) > 1e-3 or abs(vb[3] - float(h[:-2])) > 1e-3:
            f.append("viewBox must be '0 0 <width_mm> <height_mm>'")
    banned = {"text", "tspan", "style", "image", "font", "script", "foreignObject", "use"}
    npath = 0
    for el in root.iter():
        tag = el.tag.split("}")[-1]
        if tag in banned: f.append(f"forbidden element <{tag}>")
        if tag == "path": npath += 1
        for k, v in el.attrib.items():
            if k.split("}")[-1] == "href": f.append("external/internal href not allowed")
            if k == "style" and "font" in v: f.append("font styling not allowed")
    if lay and npath < len([c for c in lay.get("components", []) if c.get("label")]): f.append("fewer paths than labels (labels must be converted to paths)")
    return sorted(set(f))

def t055(lay):
    f = []
    comps = lay.get("components", [])
    ids = [c.get("id") for c in comps]
    if sorted(ids) != sorted(EXPECTED): f.append(f"component ids must be exactly {sorted(EXPECTED)}")
    if not isinstance(lay.get("hp"), int) or not (8 <= lay["hp"] <= 32): f.append("hp must be an int in 8..32")
    W = lay.get("hp", 0) * 5.08
    boxes = []
    for c in comps:
        cid = c.get("id")
        if cid in EXPECTED and (c.get("kind"), c.get("widget")) != EXPECTED[cid]: f.append(f"{cid}: kind/widget must be {EXPECTED[cid]}")
        if c.get("widget") not in WIDGET_MM: continue
        if cid.startswith("TRIG_") or cid in ("DAMP", "STRIKE", "ROOM", "TUNE", "HAMMER", "LIMITER", "LEFT", "RIGHT"):
            if not str(c.get("label", "")).strip(): f.append(f"{cid}: label required")
        bw, bh = WIDGET_MM[c["widget"]]
        x, y = float(c.get("x", -1)), float(c.get("y", -1))
        box = (x - bw / 2, y - bh / 2, x + bw / 2, y + bh / 2)
        if box[0] < 1.5 or box[2] > W - 1.5: f.append(f"{cid}: outside horizontal margins")
        if box[1] < 10.0 or box[3] > 118.5: f.append(f"{cid}: outside vertical band 10..118.5 mm")
        boxes.append((cid, box))
    for i in range(len(boxes)):
        for j in range(i + 1, len(boxes)):
            (a, A), (b, B) = boxes[i], boxes[j]
            gapx = max(B[0] - A[2], A[0] - B[2]); gapy = max(B[1] - A[3], A[1] - B[3])
            if max(gapx, gapy) < 2.0: f.append(f"{a} and {b} closer than 2 mm")
    return f

def t056():
    f = []
    gen = os.path.join(ROOT, "tools", "gen_panel.py")
    if not os.path.isfile(gen): return ["tools/gen_panel.py missing"]
    with tempfile.TemporaryDirectory() as d:
        r = subprocess.run([sys.executable, gen, "--out-dir", d], cwd=ROOT, capture_output=True, text=True, timeout=300)
        if r.returncode != 0: return [f"generator failed: {r.stderr.strip()[:300]}"]
        for rel in ("res/OilDrumKit.svg", "src/Layout.hpp"):
            a, b = os.path.join(d, rel), os.path.join(ROOT, rel)
            if not os.path.isfile(a): f.append(f"generator did not write {rel}")
            elif not os.path.isfile(b): f.append(f"{rel} not committed")
            elif not filecmp.cmp(a, b, shallow=False): f.append(f"{rel} is stale (re-run tools/gen_panel.py)")
        hpp = os.path.join(ROOT, "src", "Layout.hpp")
        if os.path.isfile(hpp):
            txt = open(hpp, encoding="utf-8").read()
            for cid in EXPECTED:
                if f" {cid} " not in txt and f" {cid}=" not in txt and f" {cid}{{" not in txt: f.append(f"Layout.hpp lacks {cid}")
    return f

lay, err = load_layout()
record("T-051", "panel SVG meets Rack panel rules", ([err] if err else []) + t051(lay) if lay else [err])
record("T-055", "layout constraints (ids, widgets, margins, 2 mm clearance)", t055(lay) if lay else [err])
record("T-056", "generated panel and Layout.hpp are up to date", t056())
ok = 0
for tid, name, fails in results:
    if fails: print(f"FAIL {tid} {name}: " + "; ".join(fails[:12]))
    else: ok += 1; print(f"PASS {tid} {name}")
print(f"SUMMARY {ok}/{len(results)} passed")
sys.exit(0 if ok == len(results) else 1)
