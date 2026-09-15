"""
Shared rendering primitives for widgetkit-src.

Everything here works in PTS (logical, DPI-independent units matching
WonderGUI's BlockSkin coordinate space) and only converts to actual pixels
at the last moment, using a `density` multiplier (pixels-per-pt). Render at
density=2 for a 2x-native atlas; the pts numbers written into specs/*.yaml
and the generated header never change when density changes -- only the
PNG's physical pixel size does. A `SUPERSAMPLE` factor on top of that is
purely for anti-aliasing quality and never appears in output coordinates.
"""

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter

SUPERSAMPLE = 4  # internal AA supersampling factor, independent of density


def px(pts, density):
    """Convert a pts value (or tuple of them) to rounded output pixels."""
    if isinstance(pts, (list, tuple)):
        return tuple(round(v * density) for v in pts)
    return round(pts * density)


# ---------------------------------------------------------------------------
# Low level drawing (all take sizes already in OUTPUT pixels + supersample)
# ---------------------------------------------------------------------------
def _big(size):
    w, h = size
    return int(w * SUPERSAMPLE), int(h * SUPERSAMPLE)


def rounded_rect_mask(size, radius, inset=0.0):
    w, h = size
    big_w, big_h = _big(size)
    img = Image.new("L", (big_w, big_h), 0)
    draw = ImageDraw.Draw(img)
    ins = inset * SUPERSAMPLE
    rad = max(radius * SUPERSAMPLE - ins, 0)
    draw.rounded_rectangle([ins, ins, big_w - 1 - ins, big_h - 1 - ins], radius=rad, fill=255)
    return img.resize((w, h), Image.BOX)


def ellipse_mask(size, inset=0.0):
    w, h = size
    big_w, big_h = _big(size)
    img = Image.new("L", (big_w, big_h), 0)
    draw = ImageDraw.Draw(img)
    ins = inset * SUPERSAMPLE
    draw.ellipse([ins, ins, big_w - 1 - ins, big_h - 1 - ins], fill=255)
    return img.resize((w, h), Image.BOX)


def vgrad(size, top_rgb, bottom_rgb, top_a=255, bottom_a=255, ease=1.0):
    """Vertical gradient. `ease` > 1 concentrates the colour change at the TOP
    and BOTTOM edges and flattens the middle.

    This matters for any block that is stretched along the gradient with a
    rigidPart in the middle. There the middle band is cut into a rigid run
    plus two thin flanks, and each flank stretches from a 1-2pt source slice
    -- so a linear gradient renders as a flat bright region, a rapid
    transition across the rigid run, and a flat dark region. Pushing the
    change into the frames (which never stretch) leaves the middle nearly
    uniform, so stretching it is invisible.

    The remap is 0.5 + 0.5*sign(u)*|u|^ease for u = 2t-1: it fixes t at 0,
    0.5 and 1, and its slope at the midpoint is zero for ease > 1.
    """
    w, h = size
    t = np.linspace(0, 1, h).reshape(h, 1)
    if ease != 1.0:
        u = 2.0 * t - 1.0
        t = 0.5 + 0.5 * np.sign(u) * np.abs(u) ** ease
    top = np.array(top_rgb, dtype=np.float32)
    bot = np.array(bottom_rgb, dtype=np.float32)
    rgb = (top * (1 - t) + bot * t).astype(np.uint8)
    rgb = np.repeat(rgb[:, np.newaxis, :], w, axis=1)
    alpha = (top_a * (1 - t) + bottom_a * t).astype(np.uint8)
    alpha = np.repeat(alpha, w, axis=1)
    return Image.fromarray(np.dstack([rgb, alpha]), mode="RGBA")


def diag_grad(size, dark_rgb, light_rgb):
    w, h = size
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    t = ((xx / max(w - 1, 1) + yy / max(h - 1, 1)) / 2)[..., None]
    dark = np.array(dark_rgb, dtype=np.float32)
    light = np.array(light_rgb, dtype=np.float32)
    rgb = (dark * (1 - t) + light * t).astype(np.uint8)
    alpha = np.full((h, w, 1), 255, dtype=np.uint8)
    return Image.fromarray(np.concatenate([rgb, alpha], axis=2), mode="RGBA")


def raised_panel(w, h, radius, palette, border=1.4, gloss_alpha=150, gloss_frac=0.5,
                 shape="rect", ease=1.0):
    mask_fn = ellipse_mask if shape == "ellipse" else (lambda sz, inset=0.0: rounded_rect_mask(sz, radius, inset))
    outer = mask_fn((w, h))
    inner = mask_fn((w, h), inset=border)

    body = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    body.paste(Image.new("RGBA", (w, h), tuple(palette["border"])), (0, 0), outer)
    grad = vgrad((w, h), palette["top"], palette["bottom"], ease=ease)
    body = Image.composite(grad, body, inner)

    if gloss_alpha > 0:
        gh = max(int(h * gloss_frac), 1)
        gloss = vgrad((w, gh), (255, 255, 255), (255, 255, 255), top_a=gloss_alpha, bottom_a=0)
        gloss_full = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        gloss_full.paste(gloss, (0, 0))
        body = Image.alpha_composite(body, Image.composite(gloss_full, Image.new("RGBA", (w, h), (0, 0, 0, 0)), inner))
    return body


def inset_shade(img, w, h, radius, amount=70, frac=0.45, shape="rect", border=1.4):
    mask_fn = ellipse_mask if shape == "ellipse" else (lambda sz, inset=0.0: rounded_rect_mask(sz, radius, inset))
    inner = mask_fn((w, h), inset=border)
    sh = max(int(h * frac), 1)
    shade = vgrad((w, sh), (0, 0, 0), (0, 0, 0), top_a=amount, bottom_a=0)
    shade_full = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    shade_full.paste(shade, (0, 0))
    return Image.alpha_composite(img, Image.composite(shade_full, Image.new("RGBA", (w, h), (0, 0, 0, 0)), inner))


def recessed_diagonal(w, h, radius, dark, light, border_color, border=1.2):
    outer = rounded_rect_mask((w, h), radius)
    inner = rounded_rect_mask((w, h), radius, inset=border)
    canvas = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    canvas.paste(Image.new("RGBA", (w, h), tuple(border_color)), (0, 0), outer)
    grad = diag_grad((w, h), dark, light)
    return Image.composite(grad, canvas, inner)


def flat_bevel_panel(w, h, radius, top_rgb, bottom_rgb, border_color, border=1.2, ease=1.0):
    outer = rounded_rect_mask((w, h), radius)
    inner = rounded_rect_mask((w, h), radius, inset=border)
    canvas = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    canvas.paste(Image.new("RGBA", (w, h), tuple(border_color)), (0, 0), outer)
    grad = vgrad((w, h), top_rgb, bottom_rgb, ease=ease)
    return Image.composite(grad, canvas, inner)


# ---------------------------------------------------------------------------
# Glyph overlays (operate in already-scaled output pixels)
# ---------------------------------------------------------------------------
def triangle_mask(size, box, direction="down"):
    """Anti-aliased triangle mask filling `box` = (x, y, w, h) output px.

    Two things keep the edges clean. First, the triangle is defined by the
    box's CONTINUOUS bounds (x..x+w), not inclusive pixel indices (x..x+w-1):
    with w = 2*h that makes both slopes exactly 45 degrees, so the steps are
    uniform instead of drifting by a fractional pixel per row and dropping an
    occasional double-step on one side. Second, it is rasterised supersampled
    and downscaled, so the edge is anti-aliased rather than a hard staircase.
    Because the geometry is exactly symmetric about the box centre, the
    resulting mask is symmetric too.
    """
    w_img, h_img = size
    S = SUPERSAMPLE
    x, y, w, h = box
    x0, y0, x1, y1 = x * S, y * S, (x + w) * S, (y + h) * S
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2

    pts = {
        "down":  [(x0, y0), (x1, y0), (cx, y1)],
        "up":    [(cx, y0), (x0, y1), (x1, y1)],
        "left":  [(x0, cy), (x1, y0), (x1, y1)],
        "right": [(x1, cy), (x0, y0), (x0, y1)],
    }[direction]

    big = Image.new("L", (w_img * S, h_img * S), 0)
    ImageDraw.Draw(big).polygon(pts, fill=255)
    # BOX, not LANCZOS: a box filter averages each SxS block, which is exactly
    # the coverage of that output pixel -- true supersampling. LANCZOS rings,
    # leaking alpha ~26 into the row beyond the shape (a faint halo) while
    # capping the flat base edge at 248 instead of a solid 255.
    return big.resize((w_img, h_img), Image.BOX)


def stroke_path_mask(size, points, thickness):
    """Anti-aliased mask of a polyline stroked to `thickness` output px.

    Built as a single filled polygon -- the two offset sides joined by a proper
    MITRE at each interior vertex -- then rasterised supersampled and
    box-downscaled. ImageDraw's own thick lines cannot do this: `joint="curve"`
    stamps a disc at the corner, which at glyph sizes reads as a lump and makes
    the stroke width visibly bulge through the bend.

    Mitre point for unit normals n0, n1 at distance t is
    P + t * (n0 + n1) / (1 + n0 . n1)  -- the intersection of the two offset
    edges. Near a 180 degree reversal that denominator collapses, so fall back
    to a bevel there rather than shooting the corner off to infinity.
    """
    S = SUPERSAMPLE
    pts = [np.asarray(p, dtype=float) * S for p in points]
    t = thickness * S / 2.0

    dirs = []
    for a, b in zip(pts, pts[1:]):
        v = b - a
        dirs.append(v / np.linalg.norm(v))
    norms = [np.array([-d[1], d[0]]) for d in dirs]

    def offset_side(sign):
        out = [pts[0] + norms[0] * t * sign]
        for i in range(1, len(pts) - 1):
            n0, n1 = norms[i - 1], norms[i]
            denom = 1.0 + float(np.dot(n0, n1))
            if denom < 1e-6:                       # ~180 degree turn: bevel it
                out.append(pts[i] + n0 * t * sign)
                out.append(pts[i] + n1 * t * sign)
            else:
                out.append(pts[i] + (n0 + n1) * (t * sign) / denom)
        out.append(pts[-1] + norms[-1] * t * sign)
        return out

    poly = offset_side(1) + offset_side(-1)[::-1]
    big = Image.new("L", (size[0] * S, size[1] * S), 0)
    ImageDraw.Draw(big).polygon([tuple(p) for p in poly], fill=255)
    return big.resize(size, Image.BOX)


def check_points(box):
    """The three points of a checkmark filling `box` = (x, y, w, h).

    Both strokes sit at exactly 45 degrees, which is what makes the edges
    rasterise as clean uniform steps instead of the ragged stair of an
    arbitrary slope. That constrains the box: the short arm rises w - h and the
    long arm rises h, so the box must satisfy h < w < 2h (w = 1.5h gives the
    classic long-arm-twice-the-short proportion).
    """
    x, y, w, h = box
    short = w - h                       # horizontal run of the short arm
    return [(x, y + h - short), (x + short, y + h), (x + w, y)]


def tint_mask(size, rgba, mask):
    """An RGBA layer of flat colour whose alpha is `mask` (times the colour's
    own alpha). Keeps RGB constant across the edge -- pasting a colour through
    a mask instead would scale RGB by the mask and darken every AA pixel."""
    r, g, b, a = (tuple(rgba) + (255,))[:4]
    layer = Image.new("RGBA", size, (r, g, b, 0))
    layer.putalpha(mask if a >= 255 else ImageChops.multiply(mask, Image.new("L", size, a)))
    return layer


def add_triangle(canvas, box, direction, color, outline=None):
    """Composite an anti-aliased triangle, with an optional 1px outline that
    hugs the shape uniformly (derived by dilating the mask and subtracting it,
    so the outline follows the diagonals rather than the bounding box)."""
    mask = triangle_mask(canvas.size, box, direction)
    out = canvas.copy()
    if outline is not None:
        ring = ImageChops.subtract(mask.filter(ImageFilter.MaxFilter(3)), mask)
        out.alpha_composite(tint_mask(canvas.size, outline, ring))
    out.alpha_composite(tint_mask(canvas.size, color, mask))
    return out


def add_arrow_glyph(canvas, direction, color, outline, arrow_w_px, arrow_h_px):
    """Anti-aliased arrow centred in the canvas, sharing add_triangle() with
    the select box's arrow so all five arrows in a kit are the same shape.

    `arrow_w_px`/`arrow_h_px` are the triangle's bounding box, so the caller
    states the proportions directly rather than deriving them from the
    canvas: for up/down the width is the base and the height is base-to-tip;
    for left/right those roles swap. The old version derived a single `s`
    from min(w, h) and built the triangle from inclusive pixel indices, which
    both tied the arrow's shape to the button's aspect ratio and produced the
    uneven stair-stepping described in triangle_mask().
    """
    w, h = canvas.size
    ax, aw = centered_span(w, arrow_w_px)
    ay, ah = centered_span(h, arrow_h_px)
    return add_triangle(canvas, (ax, ay, aw, ah), direction, color, outline)


def centered_span(extent, size):
    """Start offset and length for a run of `size` px centred exactly within
    `extent` px, returned as (start, length).

    Two traps make naively-centred glyphs land half a pixel (or more) off:
    the true centre of an N-px span is (N-1)/2, not N/2, and PIL's line/
    rectangle width expands asymmetrically around a coordinate. Together they
    put a 4px bar in a 24px block at rows 11..14 (centre 12.5) instead of
    10..13 (centre 11.5) -- small in absolute terms, but plainly visible as
    one arm of a cross being longer than its opposite. Snapping the run's
    parity to the extent's makes the margins on both sides exactly equal, so
    the glyph is centred by construction rather than by rounding luck.
    """
    size = max(int(round(size)), 1)
    if (extent - size) % 2:
        size = size + 1 if size < extent else size - 1
    size = max(min(size, extent), 1)
    return (extent - size) // 2, size


def add_plusminus_glyph(canvas, mode, color, bar_frac=0.56, thick_frac=0.15):
    """Draw a centred plus (mode='plus') or minus (mode='minus').

    Drawn as axis-aligned rectangles rather than wide lines: the bars are
    orthogonal, so aliased rectangles snapped to whole pixels give crisper
    edges than an anti-aliased stroke, and centered_span() guarantees the
    cross's centre coincides with the block's centre.
    """
    w, h = canvas.size
    span = min(w, h)
    bar_len, thick = span * bar_frac, span * thick_frac

    layer = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)

    bx, bw = centered_span(w, bar_len)      # horizontal bar: long axis = x
    by, bh = centered_span(h, thick)        #                 thickness = y
    d.rectangle([bx, by, bx + bw - 1, by + bh - 1], fill=tuple(color))

    if mode == "plus":
        vx, vw = centered_span(w, thick)    # vertical bar: thickness = x
        vy, vh = centered_span(h, bar_len)  #               long axis = y
        d.rectangle([vx, vy, vx + vw - 1, vy + vh - 1], fill=tuple(color))

    out = canvas.copy()
    out.alpha_composite(layer)
    return out


def add_check_glyph(canvas, color, check_w_px=None, check_h_px=None,
                    stroke_px=None, outline=None):
    """Anti-aliased checkmark, centred in the canvas.

    Replaces a proportionally-placed ImageDraw.line (points at 0.22/0.52 ..
    0.80/0.28 of the box) whose segments landed on arbitrary slopes and was
    drawn without anti-aliasing -- the stroke width wandered between 5 and 12
    px across the glyph. Geometry now comes from check_points(), so both
    strokes are true 45 degree runs of a constant width.
    """
    w, h = canvas.size
    cw = check_w_px if check_w_px else round(w * 0.64)
    ch = check_h_px if check_h_px else round(cw / 1.5)
    th = stroke_px if stroke_px else max(round(ch / 3), 1)

    cx, cw = centered_span(w, cw)
    cy, ch = centered_span(h, ch)

    mask = stroke_path_mask(canvas.size, check_points((cx, cy, cw, ch)), th)
    # Re-centre on the INK, not the geometric box: the stroke overhangs its
    # box by different amounts at each end (a mitred point at the vertex, butt
    # ends at the two tips), so a centred box still reads as off-centre.
    bbox = mask.getbbox()
    if bbox:
        dx, dy = (w - bbox[2] - bbox[0]) // 2, (h - bbox[3] - bbox[1]) // 2
        if dx or dy:
            mask = stroke_path_mask(canvas.size, check_points((cx + dx, cy + dy, cw, ch)), th)
    out = canvas.copy()
    if outline is not None:
        ring = ImageChops.subtract(mask.filter(ImageFilter.MaxFilter(3)), mask)
        out.alpha_composite(tint_mask(canvas.size, outline, ring))
    out.alpha_composite(tint_mask(canvas.size, color, mask))
    return out


def add_dot_glyph(canvas, color, dot_px=None, outline=None):
    """The radio button's centre dot: one anti-aliased circle.

    Reuses dot_line_mask with a count of 1 -- same supersampled circle the
    split-handle dots are built from -- rather than ImageDraw.ellipse at
    output resolution, which left a hard staircase (row widths jumping
    4, 8, 10, 12 on a 12px dot).
    """
    w, h = canvas.size
    diameter = dot_px if dot_px else round(min(w, h) * 0.40)
    mask = dot_line_mask(canvas.size, "horizontal", 1, diameter, 0)
    out = canvas.copy()
    if outline is not None:
        ring = ImageChops.subtract(mask.filter(ImageFilter.MaxFilter(3)), mask)
        out.alpha_composite(tint_mask(canvas.size, outline, ring))
    out.alpha_composite(tint_mask(canvas.size, color, mask))
    return out


def add_grip_lines(canvas, orientation, color_hi, color_lo, line_len_px=None):
    """Grip lines for a scrollbar handle.

    `orientation` is the direction the LINES RUN, not the handle's shape:
    "vertical_lines" draws ridges running top-to-bottom, spread left-to-right.
    Ridges run ACROSS the axis the handle is dragged along, so a wide
    (horizontal) handle takes vertical lines and a tall one takes horizontal
    lines -- which is the opposite of what the block's aspect ratio suggests,
    hence the caller states it explicitly.

    `line_len_px` is the length of each line, i.e. along the CROSS axis, which
    for a scrollbar handle is the axis that never stretches. The dimension that
    has to fit inside the handle's rigidPart span is therefore the lines'
    SPREAD (2 * spacing), not this length.
    """
    w, h = canvas.size
    cx, cy = w / 2, h / 2
    layer = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    spacing = max(min(w, h) * 0.16, 1.6)
    offs = (-1, 0, 1)
    if orientation == "horizontal_lines":   # lines run left-right, stacked vertically
        line_len = line_len_px if line_len_px else w * 0.5
        for i in offs:
            y = cy + i * spacing
            x0, x1 = cx - line_len / 2, cx + line_len / 2
            d.line([(x0, y + 0.6), (x1, y + 0.6)], fill=tuple(color_hi), width=1)
            d.line([(x0, y - 0.4), (x1, y - 0.4)], fill=tuple(color_lo), width=1)
    else:                                  # lines run up-down, side by side
        line_len = line_len_px if line_len_px else h * 0.5
        for i in offs:
            x = cx + i * spacing
            y0, y1 = cy - line_len / 2, cy + line_len / 2
            d.line([(x + 0.6, y0), (x + 0.6, y1)], fill=tuple(color_hi), width=1)
            d.line([(x - 0.4, y0), (x - 0.4, y1)], fill=tuple(color_lo), width=1)
    out = canvas.copy()
    out.alpha_composite(layer)
    return out


def dot_line_mask(size, direction, count, dot_px, gap_px, shift=(0.0, 0.0)):
    """Anti-aliased mask of `count` circles in a line, centred in `size`.

    `gap_px` is centre-to-centre. Coordinates are continuous and rasterised
    supersampled, so the true centre of a w-px canvas is w/2.0 here -- the
    (N-1)/2 rule in centered_span() applies to discrete pixel indices, not to
    a continuous shape that is then area-averaged.
    """
    S = SUPERSAMPLE
    w, h = size
    span = (count - 1) * gap_px + dot_px
    cx, cy = w / 2.0 + shift[0], h / 2.0 + shift[1]

    big = Image.new("L", (w * S, h * S), 0)
    d = ImageDraw.Draw(big)
    for i in range(count):
        off = -span / 2.0 + dot_px / 2.0 + i * gap_px
        x, y = (cx + off, cy) if direction == "horizontal" else (cx, cy + off)
        x0, y0 = (x - dot_px / 2.0) * S, (y - dot_px / 2.0) * S
        # ImageDraw's bounds are INCLUSIVE, so the far edge is -1: a box of
        # x0..x0+dot*S would cover dot*S+1 subpixels, a quarter of an output
        # pixel too many, all of it on one side -- enough to make the circle
        # visibly lopsided (margins 7 left, 6 right on a 14px dot).
        d.ellipse([x0, y0, x0 + dot_px * S - 1, y0 + dot_px * S - 1], fill=255)
    return big.resize((w, h), Image.BOX)


def add_dot_line(canvas, direction, count, dot_px, gap_px, color_hi, color_lo):
    """A row/column of engraved dots: a light copy offset down-right with the
    dark dot over it, the same bevel language the grip lines use."""
    lo = dot_line_mask(canvas.size, direction, count, dot_px, gap_px)
    hi = dot_line_mask(canvas.size, direction, count, dot_px, gap_px, shift=(1.0, 1.0))
    out = canvas.copy()
    out.alpha_composite(tint_mask(canvas.size, color_hi, hi))
    out.alpha_composite(tint_mask(canvas.size, color_lo, lo))
    return out


def add_dot_grid(canvas, color_hi, color_lo):
    w, h = canvas.size
    layer = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    r = max(min(w, h) * 0.08, 1.0)
    cx, cy = w / 2, h / 2
    off = min(w, h) * 0.20
    for dx in (-off, off):
        for dy in (-off, off):
            d.ellipse([cx + dx - r, cy + dy - r, cx + dx + r, cy + dy + r], fill=tuple(color_hi), outline=tuple(color_lo))
    out = canvas.copy()
    out.alpha_composite(layer)
    return out


def add_select_arrow(canvas, arrow_zone_px, color, outline, divider_color,
                     arrow_w_px=None, arrow_h_px=None):
    """Divider line plus a down-pointing dropdown arrow in the right-hand zone.

    The arrow is sized explicitly rather than derived from the zone, and is
    deliberately WIDER THAN TALL. The height matters structurally: the arrow
    sits in the block's right frame column, which stretches vertically when
    the widget grows, so it has to be pinned by a rigidPartY. WonderGUI
    divides the leftover height between the bands above and below the rigid
    run in proportion to their source lengths, so the arrow must leave real
    stretchable material on BOTH sides -- otherwise it either drifts off
    centre or, with nothing on either side, divides by zero. A flat arrow
    leaves that room; a tall one (the old min(w,h)*0.30 triangle spanned
    8.4 of the 10 available pts) does not.
    """
    w, h = canvas.size
    x = w - arrow_zone_px

    out = canvas.copy()
    layer = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    d.line([(x, h * 0.18), (x, h * 0.82)], fill=tuple(divider_color), width=1)
    out.alpha_composite(layer)

    aw = arrow_w_px if arrow_w_px else round(arrow_zone_px * 0.62)
    ah = arrow_h_px if arrow_h_px else max(round(aw * 0.5), 2)
    ax, aw = centered_span(arrow_zone_px, aw)   # centred in the arrow zone
    ay, ah = centered_span(h, ah)               # centred in the block height

    return add_triangle(out, (x + ax, ay, aw, ah), "down", color, outline)
