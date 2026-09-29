#!/usr/bin/env python3
# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
# T-050: plugin.json and LICENSE per plan/DECISIONS.md D-014 (C-004: VCV manifest spec, C-018: licensing).
import json, os, re, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ALLOWED_TAGS = {"drum", "physical modeling"}
def main():
    fails = []
    def check(cond, msg):
        if not cond: fails.append(msg)
    p = os.path.join(ROOT, "plugin.json")
    if not os.path.isfile(p):
        print("FAIL T-050 plugin manifest: plugin.json missing"); print("SUMMARY 0/1 passed"); return 1
    try:
        m = json.load(open(p, encoding="utf-8"))
    except Exception as e:
        print(f"FAIL T-050 plugin manifest: invalid JSON: {e}"); print("SUMMARY 0/1 passed"); return 1
    check(m.get("slug") == "OilDrumKit", "slug must be 'OilDrumKit'")
    check(re.fullmatch(r"[A-Za-z0-9_-]+", str(m.get("slug", ""))) is not None, "slug charset")
    check(m.get("name") == "Oil Drum Kit", "name must be 'Oil Drum Kit'")
    check(m.get("brand") == "Galactic HQ", "brand must be 'Galactic HQ'")
    check(re.fullmatch(r"2\.\d+\.\d+", str(m.get("version", ""))) is not None, "version must be 2.MINOR.REVISION")
    check(m.get("license") == "Apache-2.0", "license must be 'Apache-2.0'")
    check(isinstance(m.get("author"), str) and m["author"].strip() != "", "author required")
    check(m.get("sourceUrl") == "https://github.com/qualitycoding/oildrumkit-vcvrack", "sourceUrl")
    mods = m.get("modules", [])
    check(isinstance(mods, list) and len(mods) == 1, "exactly one module")
    if isinstance(mods, list) and len(mods) == 1:
        mod = mods[0]
        check(mod.get("slug") == "OilDrumKit", "module slug must be 'OilDrumKit'")
        check(mod.get("name") == "Oil Drum Kit", "module name")
        tags = [t.lower() for t in mod.get("tags", [])]
        check("drum" in tags, "module tags must include 'Drum'")
        check(set(tags) <= ALLOWED_TAGS, f"unexpected tags {tags}")
    lic = os.path.join(ROOT, "LICENSE")
    check(os.path.isfile(lic) and "Apache License" in open(lic, encoding="utf-8", errors="replace").read(), "LICENSE must be Apache-2.0 text")
    src = ""
    for d, _, fs in os.walk(os.path.join(ROOT, "src")):
        for f in fs:
            if f.endswith((".cpp", ".hpp")): src += open(os.path.join(d, f), encoding="utf-8", errors="replace").read()
    check(re.search(r'createModel\s*<[^>]*>\s*\(\s*"OilDrumKit"\s*\)', src) is not None, "src must register createModel<...>(\"OilDrumKit\")")
    if fails:
        print("FAIL T-050 plugin manifest: " + "; ".join(fails)); print("SUMMARY 0/1 passed"); return 1
    print("PASS T-050 plugin manifest"); print("SUMMARY 1/1 passed"); return 0
sys.exit(main())
