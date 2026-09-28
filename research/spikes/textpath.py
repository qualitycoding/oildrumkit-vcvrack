from fontTools.ttLib import TTFont
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen
f = TTFont("DejaVuSans.ttf"); gs = f.getGlyphSet(); cmap = f.getBestCmap(); upm = f["head"].unitsPerEm
def text_path(s, x, y, size_mm):
    sc = size_mm / upm; pen = SVGPathPen(gs); cx = 0
    for ch in s:
        g = cmap[ord(ch)]; tp = TransformPen(pen, (sc, 0, 0, -sc, x + cx*sc, y)); gs[g].draw(tp); cx += gs[g].width
    return pen.getCommands(), cx*sc
d, w = text_path("BD SN1", 10, 20, 3.0); print(len(d), round(w,2), d[:60])
