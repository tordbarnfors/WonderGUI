# glossyblue / SelectBox — session handoff

Written 2026-09-28, at the end of a long Cowork session, to hand the work to a
Claude Code session. Everything below is either in the repo now or was verified
by measurement during that session.

**Companion document:** `claude/glossyblue-widgetkit-pipeline.md` in this same
project is the deep reference — the build pipeline, every WonderGUI internal
that was verified against engine source, and the traps. Read it before touching
`widgetkit-src/`. This document is the *state of play* and does not repeat it.

---

## Where things are

Repo: `/home/tord/Workspace/WonderGUI`. Build dir: `tuxbuild/` (CMake + ninja,
already configured). `cd tuxbuild && ninja` builds everything; `ninja widgetbench`
builds the widget gallery, which is the main visual test harness.

The kit's source of truth is `widgetkit-src/`, **not** the generated headers:

| path | what it is |
|---|---|
| `widgetkit-src/common/build.py` | the CLI: `render` and `pack` |
| `widgetkit-src/common/theme_lib.py` | drawing primitives |
| `widgetkit-src/glossyblue/specs/*.yaml` | one declarative spec per widget |
| `widgetkit-src/glossyblue/palette.yaml` | colour tokens + `exported_colors` |
| `widgetkit-src/glossyblue/header_shell.h` | **hand-maintained C++**, two generated regions |
| `widgetkit-src/glossyblue/parts/*.png` + `.json` | stage-1 output, editable checkpoint |
| `src/widgetkits/wg_glossyblue.h` | **generated — never edit** |
| `resources/glossyblue_skinblocks.png` | **generated — never edit** |

Regenerating:

```
cd widgetkit-src/common
python3 build.py render glossyblue [widget ...] [--force]
python3 build.py pack glossyblue
```

`pack` rewrites the two marked regions of `header_shell.h`'s generated copy and
deploys to `resources/` and `src/widgetkits/`. It is deterministic: identical
parts give a byte-identical atlas and header. **A spec change that only affects
metadata (`rigid_part`, `padding_pts`, `spacing_pts`) still needs
`render --force` for that widget**, because `pack` reads the JSON sidecar, not
the spec.

Editing `header_shell.h` alone (the wrapper classes, `init()`, the skins that
are not generated) still requires a `pack` to regenerate the two deployed
headers. There is no separate step.

---

## What was just done, and what is unverified

### Done and confirmed working by Tord

1. **Atlas packer replaced.** The old shelf packer sat at 27.5% occupancy;
   `pack_rects()` in `build.py` is now MaxRects wrapped in a search over bin
   width × 3 placement heuristics × 5 insertion orders. Atlas went 640×484 →
   **436×240 at 81.4%** (1210 KiB → 409 KiB RGBA8). Two invariants are asserted
   after every pack: positions land on the density grid (the manifest stores pts
   as `round(px/density)`, so an off-grid part is *recorded* half a point from
   where it is), and 1pt of clear space around every part including the atlas
   edge.

2. **SelectBox pressed state.** `glyph_shift: {Pressed: [1,1]}` moves the arrow
   and divider (artwork), `content_shift: {Pressed: [1,1]}` moves the text.
   Note `Pressed` here means *the drop-down is open* — `SelectBox::_setState`
   forces it while the popup is up.

3. **SelectBox rigid part fixed.** The arrow had been smearing when the box was
   stretched (10.5pts tall instead of 6.5 at a 40pt height, 21.5 at 100pt),
   because the run was sized from nominal glyph geometry (5pts centred at y=10)
   rather than the drawn footprint with outline and AA (7.0..13.5). Now
   `5.5..14.5`. Every other rigid part in the kit was audited the same way and
   is correct.

4. **`text` → `display` rename** (Tord's change to `wg::SelectBox`, plus the
   two compile breaks it left — see below). He confirmed: *"It builds and works
   fine now."*

### Done but NOT yet confirmed by Tord

5. **SelectBox entry indent.** `Skins::SelectBoxEntry` gained
   `_.padding = { 0, 5, 0, 5 }` so drop-down entries line up with the closed
   box's text. Delivered and committed to the repo, kit repacked, but he has not
   reported back on how it looks. **Start by checking this.** The arithmetic is
   in the pipeline doc under "Two paddings, one apparent indent".

---

## The `text` → `display` rename — full picture

Tord renamed `wg::SelectBox`'s text component from `text` to `display` and gave
the Blueprint a `DynamicText::Blueprint display` field, initialised the standard
way (`display._initFromBlueprint(bp.display)`), which also fixed a long-standing
engine bug: the closed box previously had no way to get a style at all, so it
drew nothing however many entries it held.

Two things his rename missed, both now fixed:

- `wg_selectbox.h`'s **template constructor's member-init list** still said
  `text(this)`. (The default constructor in the .cpp was already updated.)
- `wg_oldskool.h`'s SelectBox wrapper no longer compiled. Adding `display` to
  the base Blueprint makes `bp.display` a hard requirement of the template
  constructor, and oldskool's wrapper is a bare forward whose Blueprint had no
  such field. It now declares one, styled `NormalDark` to match that kit.

**Rule to carry forward:** a nested component Blueprint in a base class is a
contract every wrapper must satisfy. When a widget gains one, grep *both*
widgetkits.

A repo-wide sweep of all 374 source files under `wondergui`, `widgetkits`,
`tools` and `examples` found that **no application code touches the component
at all** — eight files mention `SelectBox`, and every `.text` near one is
`SelectBoxEntry::Blueprint{ .id = …, .text = … }`, which is that struct's own
field and correctly unchanged. So the rename needed no call-site changes beyond
the two widgetkits.

The glossyblue wrapper's old workaround (its own `textStyle`/`textLayout`
Blueprint fields, applied unconditionally) is gone, replaced by the same
gap-filling every other wrapper uses. `display` still defaults to
`NormalBright`, deliberately *not* sharing `entryTextStyle`: the closed box is a
blue raised control, the list is on `Skins::Canvas`.

---

## Things that will bite a fresh session

Full detail in the pipeline doc; these are the ones that cost the most time.

**The atlas is 2x and the surface must say so.** `BlockSkin` resolves every pts
value against the *surface's* scale, and `Surface::Blueprint.scale` defaults to
0 → 64 (1x). A PNG carries no density. The surface **must** be created with
`_.scale = 128`. At the default it does not fail — it renders garbage. `init()`
now refuses a surface whose `scale() != 128`. Two reported "bugs" during the
session turned out to be this.

**Nested Blueprints are replaced, never merged.**
`wkit::Button::create({ .label = { .text = "Save" } })` constructs a *fresh*
`DynamicText::Blueprint`, so the wrapper's `.style` default is silently gone —
and a text component with no style has no font, so you get no text at all, not a
wrong font. Two guards exist now: `init()` sets `Base::setDefaultStyle` /
`setDefaultTextLayout`, and all ten wrapper constructors gap-fill
(`if( !label.style() ) label.setStyle(...)`). Because of that,
`.label = { .text = "..." }` now works directly and `gallery_test.cpp` uses it
throughout.

**Designated initializers must follow declaration order.** Wrapper Blueprints
are alphabetical, so `.disabled` comes before `.label`. One exception already in
the engine: `entryTextStyle` sits before `entryTextLayout`, which is *not*
alphabetical — a trap worth knowing because the compiler error is opaque.

**`SizeCapsule::defaultSize` takes −1, not 0**, for "ask the child".
`{260, 0}` forces zero height and the widget vanishes.

**Measure, don't infer.** The method that worked all session: build a glyph-only
mask by differencing a block against a no-glyph render, push it through
`simulate_stretch` (the `blitNinePatch` port in `build.py`), and compare bounding
boxes. Two traps when writing such a check — stretch only the axis the widget
actually grows on, and exclude elements that are *supposed* to stretch. Both
produced false alarms on the first attempt.

---

## No build available from Cowork

The Cowork session had no shell on Tord's machine — only file staging and
commit — so **nothing in items 1–5 above was ever compiled here.** Verification
was mechanical instead: parsing Blueprint field lists out of the headers and
checking every `wkit::*::create({...})` call site for unknown fields and
declaration order, plus brace/paren balance. That caught real bugs, but it is
not a compiler. A code session with a working `ninja` should just build first.

---

## Open items, in the order I would rank them

1. **A 1x atlas for windows below scale 128.** The only open item that affects
   what users see. At scale 64 the grip's 1px ridges land on half a pixel and
   the 1pt content/glyph shifts collapse to one — detail that reads correctly in
   `preview.html` (which simulates at the atlas's own density) is simply absent
   in the app. This is a redraw, not a downscale. `render --density 1` exists,
   but `pack` refuses mixed densities, so a 1x build needs its own parts
   directory — likeliest shape is a second kit dir sharing `specs/` and
   `palette.yaml`.

2. **Switch the atlas from PNG to `.surf`.** `SurfaceFileHeader` carries
   `int16_t scale`, so the reader could set it from the file and the scale
   footgun above disappears permanently. There is no `Surface::setScale()`, so
   the kit cannot correct it itself today.

3. **Scrollbar button sizes are coupled.** The 16pt dimension must keep matching
   the handles' thickness (`ScrollbarHandleY` width / `ScrollbarHandleX`
   height). Nothing enforces it.

4. **Split handle thickness is now free.** Nothing on the bars constrains it any
   more, so 8pts can come down further if a thinner splitter is wanted.

5. **Left alone deliberately:** the capsule label's grey backdrop
   (`_pCapsuleLabelSkin`, `Colors::Plate`). It is a hole punch, not decoration —
   `LabelCapsule` puts the label at the top of the *widget* rect while the frame
   is inset by the capsule skin's 8pt top spacing, so the border line runs
   through the caption. It looks wrong in widgetbench only because the bench
   paints its root `Colors::Canvas`. There is no fix that keeps the straddling
   group-box look, because a skin cannot sample its backdrop; removing the grey
   means first moving the label above or inside the frame, which changes the
   capsule's proportions. Tord considered and declined this.

6. **Also fine to ignore:** the drop-down popup's left edge sits 1pt left of the
   *visible* closed box, because the popup attaches to the widget geo while
   `Skins::SelectBox`'s 1pt spacing insets the drawn box. Fixing it is an engine
   change (offset the popup by the skin's margin), not a kit one.
