# glossyblue widgetkit + the widgetkit-src build pipeline

Notes on the procedurally-generated `glossyblue` widgetkit and the build system
that produces it, including WonderGUI internals that were verified against the
engine source (not guessed) and are easy to get wrong.

## Layout

`widgetkit-src/` at the repo root is the source of truth:

- `common/theme_lib.py` — drawing primitives (panels, gradients, glyph shapes).
- `common/build.py` — the CLI: `render` and `pack`.
- `<kit>/palette.yaml` — colour tokens, plus the `exported_colors` section.
- `<kit>/specs/<Name>.yaml` — one declarative spec per widget.
- `<kit>/parts/<Name>.png` + `.json` — stage-1 output, a human-editable checkpoint.
- `<kit>/header_shell.h` — hand-maintained C++, with two generated regions.
- Generated: `<kit>_skinblocks.png`, `wg_<kit>.h`, `preview.html`, deployed to
  `resources/` and `src/widgetkits/`.

Two stages, deliberately separable so art can be hand-touched between them:

    python3 build.py render <kit> [widget ...] [--force]
    python3 build.py pack <kit>

`render` only touches widgets you name or ones with no part yet, so overwriting
is always opt-in. `pack` reads only `parts/`, packs the atlas, and rewrites
two marked regions of the header — computing every `firstBlock` from the same
metadata used to draw the pixels, so coordinates are never hand-transcribed.
`pack` is deterministic: identical parts produce a byte-identical atlas and header.

Note: a spec change that affects only metadata (e.g. `rigid_part`, `spacing_pts`)
still needs a `render --force` for that widget, because `pack` reads the JSON
sidecar, not the spec.

### The two generated regions

- `BEGIN/END GENERATED SKINS` — one `BlockSkin::create` per part.
- `BEGIN/END GENERATED COLORS` — the `Colors::` namespace, from
  `palette.yaml`'s `exported_colors`. `center_of: <Part>` **measures** the
  centre pixel of `parts/<Part>.png`, so a constant meant to match a rendered
  surface is whatever the bitmap actually is. This exists because the constants
  were hand-written for a while and drifted: `Colors::Plate` had become the
  gradient's *top* colour while the bitmap's middle was 14 levels darker.
  Other forms: `border_of` / `top_of` / `bottom_of` (a palette entry) and
  `rgb:` (a literal). Each may carry a `comment` block and a `note`.

Markers are matched as whole lines by regex, and `pack` fails unless each
appears exactly once — a substring `split()` once spliced at an explanatory
comment and destroyed the file.

## The scale requirement (most important gotcha)

The atlas is rendered at **2x pt density** — 1pt = 2px. `BlockSkin` resolves
`firstBlock`/`frame`/`padding`/`rigidPart`/`spacing` (all in **pts**) against the
**surface's** scale:

    RectI firstBlock = roundToPixels(ptsToSpx(_firstBlock, pSurface->scale()))

`Surface::Blueprint.scale` defaults to `0`, which `Surface` resolves to `64` (1x),
and a PNG carries no density metadata. So the surface **must** be created with
`_.scale = 128`. At the default it does not fail — every block is sampled at half
size from half the offset and renders as garbage. `init()` now refuses a surface
whose `scale() != 128` with an explicit error rather than proceeding.

There is no `Surface::setScale()` (only a stale comment referencing one), so the
kit cannot correct this itself.

**Planned fix:** switch from PNG to the `.surf` format, whose `SurfaceFileHeader`
includes `int16_t scale`. The reader would then set the scale from the file and
the whole footgun disappears. Until then, callers patch their loading code.

`oldskool` is a 1x atlas and needs none of this — glossyblue is the odd one out.

`pts` is `typedef float` (`wg_gfxtypes.h`), so fractional point values are legal
everywhere; the generator prints integral values as integers and keeps decimals
otherwise. It used to force everything through `int()`, which silently truncated.

## rigidPartX / rigidPartY

- `begin` is in **block** coordinates; BlockSkin rebases it onto the stretchable
  middle band (`ofs -= frame.top`, then clamps).
- `sections` is a **bitmask** naming which sections the rigid run applies to —
  `YSections` (rows) for `rigidPartX`, `XSections` (cols) for `rigidPartY`.
  Both define `operator|` and an `All` value in `wg_gfxtypes.h`. Emit each name
  qualified: `YSections::Top | YSections::Bottom`, never `YSections::Top|Bottom`.
- Leftover space is split between the two flanks **in proportion to their source
  lengths**, so a rigid run must be centred in the band to stay centred as the
  widget grows, and must leave material on both sides — the engine divides by
  `(lead + trail)`, so a run covering the whole band divides by zero.
- **Trap:** if a frame consumes an entire axis (as both scrollbar handles' do),
  that axis's Center section is zero-sized and never renders, so a rigid part
  masked to `Center` alone is dead code. Use `All`.
- The pinned glyph must fit *inside* the rigid span, or its overhang still
  stretches. Glyph sizes are therefore explicit in the specs, not derived
  from the block size.
- **Size a rigid run against the glyph's DRAWN footprint, never its nominal
  geometry.** The select box's arrow is specified as 5pts tall centred at y=10,
  so its run was written as 7..13 — but with its outline and antialiasing the
  arrow actually covers 7.0..13.5, and the half point that fell outside was
  stretched along with the gradient. At a 40pt-tall box the arrow rendered
  10.5pts tall, at 100pt it rendered 21.5: a soft smear hanging off its chin,
  in the resting state, unnoticed for months. Measure the footprint by
  differencing the block against a no-glyph render of the same block.
- A frame of **zero** is fine with rigid parts. `blitNinePatch` (gen2) short-cuts
  to a plain `stretchBlit` when the frame is empty **only if** both rigid section
  masks are `None`; with a rigid part set it takes the full nine-patch path and
  the whole block is the centre cell. This is what the dot overlays rely on.

## DoubleSkin layering (the split handles)

`Skins::SplitHandleX` / `SplitHandleY` are `DoubleSkin`s over two generated
private BlockSkins: a grey bar (`_pSplitHandle*BarSkin`) with a fully
transparent dot overlay (`_pSplitHandle*DotsSkin`) on top, `skinInSkin = false`
so both layers get the whole canvas.

Why: baked into the bar, the dots had to fit a rigid run carved out of the bar's
own middle band. On an 8pt bar that band is 4pts and a rigid run may use at most
2 of them, capping the dots at ~2pts. The overlay has no border and no corners,
so it needs no frame at all — the whole block is the centre cell, the rigid run
is 6 of 8 points, and the dots are 3.5pts and pinned on *both* axes.

Blueprint notes: `skins[0]` is front, `skins[1]` is back. With `skinInSkin`
false, `_defaultSize` is `max` of the two and `_padding` comes from the *front*
skin unless set explicitly — so the padding is stated on the DoubleSkin.

`CPP_TARGET_OVERRIDES` in `build.py` maps a part name to a C++ lvalue other than
`Skins::<Name>`, which is how the layers become private pointers.

This is also the escape hatch whenever a rigid part is costing too much
stretchable material (see the select box under `glyph_shift`): move the pinned
detail to a transparent overlay and the layer underneath needs no rigid part at
all. Check first that DoubleSkin forwards whatever the lower layer was
providing — `contentShift`, in particular.

## blocks_from: two skins over one block

A spec with `blocks_from: <Other>` renders no pixels and takes no atlas space.
`pack` copies the source's block coordinates, size, strip layout and states,
and the alias keeps its own `padding_pts` / `spacing_pts` / `frame_pts`. It is
for a skin that *is* another skin with a different blueprint — `Skins::Field` is
`Skins::Canvas` with an outer margin — and it beats a near-duplicate spec on
both counts: the art cannot drift apart, and the intent is stated rather than
implied. A frame the alias omits is inherited, so it cannot silently lose the
source's frame and smear its borders when stretched. Aliases may not chain.

## Atlas packing

`pack_rects()` in `build.py` is a MaxRects packer wrapped in a search. It knows
nothing about widgets — it takes rectangles and returns positions — so it keeps
packing tightly when a part changes size, which is the whole point.

It replaced a shelf packer (fill a row left to right at a fixed 640px width,
start a new row when the next part does not fit). That packed the kit at
**27.5%** occupancy: the widest part is 358px, so every row holding only small
parts wasted most of 640px, and the fixed width meant one part growing by a
pixel could push a whole row down. The atlas is now **436x240 at 81.4%**, down
from 640x484 — 66% fewer pixels, and 1210 KiB to 409 KiB of RGBA8 video memory.

Three things are searched rather than chosen:

- **Bin width.** Every width from "just wide enough for the widest part"
  upwards, in density-sized steps, each packed into an unbounded-height bin;
  the used height falls out of the result. Smallest area wins.
- **Placement heuristic** — best-short-side-fit, best-area-fit, bottom-left.
- **Insertion order** — by max side, area, height, width or perimeter, all
  descending.

Trying all fifteen combinations costs about a second and is worth it: on this
kit the best pairing beats the worst by a factor of four (81.4% against 18.9%),
and which one wins changes as parts change size, so hard-coding today's winner
would quietly stop being right later. A much more expensive variant that also
binary-searches the bin height was tried and gained exactly nothing, so it is
not in the code.

Two invariants, both asserted after packing because a silent breach shows up as
blurred or shifted artwork rather than a crash:

- **Positions must land on the density grid.** The manifest stores atlas
  coordinates in pts as `round(px / density)`, so a part placed on an odd pixel
  at 2x would be *recorded* half a point away from where it actually is, and
  every block in it would sample half a pixel out. Each part's footprint is
  rounded up to the grid and the origin sits on it, so every derived position
  does too.
- **1pt of clear space around every part**, including at the atlas edge, so
  bilinear sampling at a block's edge cannot pull in a neighbour. Distinct from
  `blockSpacing`, which is the gap *within* a strip.

Verification after the switch: every part cropped out of the new atlas at its
new `firstBlock` was compared byte-for-byte against `parts/<Name>.png` — 20 of
20 identical, the one alias sharing its source's coordinates, no overlaps, no
gutter violations, nothing off-grid. The generated header diff was 42 lines,
all of them `firstBlock`. Fuzzing 25 randomly perturbed copies of the kit
(including one part doubled in size) never dropped below 78.8% occupancy.

## spacing vs padding vs blockSpacing

Three different things, easy to confuse:

- `padding` — inset of the **content** from the widget's edge.
- `spacing` (`Skin::margin()`) — an outer **margin**. It shrinks the rect the
  skin draws into and grows the widget's default size, so a widget carries its
  own separation from its neighbours instead of every call site setting panel
  spacing. Each side gets the full value, so two neighbours end up `2 x spacing`
  apart.
- `blockSpacing` — the gap between state blocks **inside the atlas strip**. A
  packing detail with no visual meaning. Spelled `block_spacing_pts` in specs so
  it can never be mistaken for the above.

Who carries spacing in glossyblue: **1pt** on Button, ToggleButton, SelectBox
and Field — free-standing controls that get lined up in a PackPanel. Nothing
else, on purpose: backdrops (Plate/Canvas/Window/Titlebar) must reach their
edge; PlateNoBevel abuts the scrollbars; SelectBoxEntry rows must touch; the
Scroller lays its track/handle/buttons out adjacent; the split handles fill the
drag gap exactly; and Checkbox/RadioButton are *icon* skins, where
`Icon::spacing` sets the gap to the label.

**Spacing stacks with the container's padding**, which is the trap. At spacing 2
a button on a Plate sat `5 + 2 = 7`pts from the panel edge but only 4pts from
the next button — the outer margin read as bigger than the gap it existed to
create. Fixed by halving spacing and taking a point off every container padding:

| skin | padding |
|---|---|
| Plate | 5 → 4 |
| Window | 6 → 5 |
| PlateNoBevel | 4 → 3 (tracks Plate; its extra point is Plate's outline) |
| Canvas | 2 → 1 |
| `_pLabelCapsuleSkin`, `_pInvisibleBoxSkin` | content sides 4 → 3 (the large top value is label clearance, not a content margin — left alone) |

Net effect: a control is the same distance from its panel edge as before spacing
existed (5pts on a Plate), and 2pts from its neighbour instead of 0.

`Skins::Field` is `Skins::Canvas` plus that margin, for LineEditor and
TextEditor. They had to be split because Canvas is also the SelectBox
drop-down's background, where a margin would inset the list inside the popup.
The split also let their paddings diverge usefully: Canvas went to 1 (its
SelectBoxEntry rows want their highlight to reach the edge) while Field kept 2
(text should clear the border). Note padding does *not* need compensating on
Field itself — spacing is outside the border, padding inside, so a widget's own
two values never stack.

### Two paddings, one apparent indent

The select box's drop-down entries looked indented relative to the closed box's
own text, and the cause is worth remembering because nothing in either skin
looks wrong on its own. The two texts share an origin — the popup attaches to
the widget's geo (`Placement::SouthWest` on `globalGeo()`), so its left edge and
the widget's coincide — but they are inset by different routes:

    closed box  = Skins::SelectBox spacing (1) + padding-left (5)  = 6pts
    entry       = Skins::Canvas padding-left (1) + entry skin (0)  = 1pt

`Skins::SelectBoxEntry` now carries `_.padding = { 0, 6, 0, 6 }`, putting the
entry text at 1 + 6 = 7 — one more than the resting box, see below. Three things that made this the right lever rather
than shrinking the closed box's padding (which exists to clear its outline and
rounded corners):

- Padding on a BoxSkin moves the **text** only. The skin fills and outlines the
  rect it is handed, and `SelectBox::_render` hands it the list canvas's full
  content width, so the hover/selection highlight still reaches both edges —
  which is exactly why `Skins::Canvas` was taken down to padding 1 earlier.
- The engine folds the entry skin's padding into the list's default width
  (`entryDefault.w + listPadding.w` in `_updateListCanvasSize`), so the popup
  grows rather than truncating long entries.
- Top and bottom stay **0** on purpose: the engine adds vertical entry padding
  to every `m_height`, so any value there changes row height.

Why 7 and not 6: the two texts are only ever on screen together while the
popup is open, and then the closed box is in `Pressed`, whose `content_shift`
moves its text 1pt right (1 + 5 + 1 = 7). Padding 5 matched the *resting* box,
which is never seen next to the list, and measured 1pt short in a real build.
At 6, closed text and entries start on the same pixel column with the list
open (verified in widgetbench at scale 64). Aim at the state the comparison
actually happens in, not the one the arithmetic starts from.

A related engine bug, now fixed: `SelectBox::_recalcListCanvasSize()` (called
from every `_resize()`) rebuilt the popup's default height as the bare sum of
entries, dropping the list skin's top+bottom padding, so the last entry was
clipped by exactly that much. It now seeds both the default and the matching
height with `listPadding.h`, like the constructor and `_sideCanvasResize()`.

Still unaligned by 1pt and left alone: the popup's left edge sits 1pt left of
the *visible* box, because the popup attaches to the widget geo while
`Skins::SelectBox`'s 1pt spacing insets the drawn box. Fixing that means the
engine offsetting the popup by the skin's margin — an engine change, not a kit
one.

## content_shift

`content_shift: {<State>: [x, y]}` in a spec moves the widget's **content**
(label, icon) for that state without touching the artwork — the classic
push-button effect where the caption sinks with the surface. BlockSkin adds it
in `_contentOfs` (`wg_stateskin.cpp`), in pts, so it costs no atlas space and
needs no extra blocks. Currently **1pt** down-right on Button's `Pressed`,
ToggleButton's `Checked`/`CheckedHovered` (a toggle has no Pressed block; the
checked states are the ones whose artwork looks pushed in), and SelectBox's
`Pressed`.

On amount: a static side-by-side mock-up argued for 2, but testing in a real
build settled on 1 — in use the *transition* is what you perceive, not the
end state, and at 2 the label visibly jumps rather than sinking. Worth
remembering for any other motion in this kit: mock-ups rank end states,
interaction ranks movement, and they do not agree.

**It must not disturb the block order.** `wg_blockskin.cpp` assigns block
positions as `blockOfs + pitch * index`, where `index` counts only entries
whose `blockless` is false, while the shift uses a separate counter. So
annotating a state that already has a block is safe, but a state *without* one
needs `blockless = true` as a third StateBP argument or it silently steals the
next block in the strip. The generator refuses `content_shift` on a state that
is not in the spec's `states`, rather than emitting that.

## state_style_as and glyph_shift

Two per-spec render controls, both introduced for the checkbox.

`state_style_as: {<State>: <OtherState>}` renders a state with another's
STATE_STYLE entry. RadioButton uses it too, in the opposite direction — see
"Checkbox and RadioButton pull apart" below. The checkbox maps `Checked -> Default`,
`CheckedHovered -> Hovered`, `CheckedPressed -> Pressed`,
`DisabledChecked -> Disabled`, so the checked and unchecked families have
**bit-identical backgrounds** and differ only by the tick. The reasoning: the
tick already says "on", so if the surface also changes on check, it has nothing
left to say about what the pointer is doing. Background = pointer, glyph =
value. Before this the checkbox had no Hovered or Pressed block at all and was
the one control in the kit that never reacted to the pointer.

`glyph_shift: {<State>: [x, y]}` draws the glyph on a transparent layer and
pastes it offset — the bitmap equivalent of `content_shift`, for a glyph baked
into the artwork, where `contentShift` has nothing to move. Used at 1pt by:

| part | states | why not contentShift |
|---|---|---|
| Checkbox | `CheckedPressed` | `Skins::Checkbox` is an *icon* skin; its content is nothing. (`Pressed` has no tick to move — with `flipOnRelease` false, clicking an unticked box goes straight to `CheckedPressed`.) |
| PlusMinusToggle | `Pressed`, `CheckedPressed` | a DrawerPanel's `buttonSkin`; no content |
| ScrollbarButton{Up,Down,Left,Right} | `Pressed` | no content |
| ScrollbarHandle{X,Y} | `Pressed` | no content |
| SelectBox | `Pressed` | the arrow and divider are artwork; the *text* moves by contentShift, so the box uses both |

**A shifted glyph must stay inside its rigid part**, or the part that pokes out
lands in stretchable material and smears as the widget grows. The scrollbar
handles are the tight case: the grip's 6pt spread sits at 22..33px inside a
rigid run of 16..40px, and the shift moves it to 24..35 — still inside, 2pts of
margin left. Measure this after any change to glyph size or rigid span; it
fails silently.

The select box is the case where it cost something. The run has to hold the
arrow in **both** positions, 7.0..14.5, and it has to stay centred on the band
or the arrow drifts (measured: an off-centre 6.5..13.5 run puts it 3.5pts low at
a 40pt height). Centred and containing that union means 5.5..14.5 of a 5..15
band — 0.5pt of stretchable material per flank instead of 2. Verified from 20pt
to 200pt in both states: arrow height exactly 6.5pts, centre error 0.00. The
price is the arrow column's gradient, rebuilt from a quarter as much source:
mean deviation from the box's own centre column goes 8.9 → 13.3 levels at 40pt
and 11.9 → 17.6 at 100pt. Invisible unless something forces the box taller than
its natural 20pts, which a form control rarely is — and the alternative was a
visibly smeared arrow when it does. The DoubleSkin escape hatch above is the fix
if select boxes ever do get stretched hard.

Not applied to the split handles: at 8pts thick a 1pt cross-axis shift is an
eighth of the bar's width, so the dots read as badly centred rather than
pressed (and the dots overlay has no `Pressed` block, and the shifted run would
sit flush against the edge of its rigid span). A shift along the bar's length
only would avoid all three.

Note that for SelectBox, `Pressed` means *the drop-down is open*:
`SelectBox::_setState` forces it for as long as the popup is up. The shift is a
sustained open-state look, not a click flash, which is one more reason 1pt was
the right amount.

Unshifted glyphs deliberately keep drawing straight onto the panel rather than
through a layer. Same picture in theory, but PIL composites in 8-bit and
`A over (B over C)` can round a level differently from `(A over B) over C` — a
full `render --force` after this change rewrote only Checkbox.png, which is the
check that the refactor was behaviour-neutral.

### Checkbox and RadioButton pull apart

They are the same widget class and share a strip layout, but their
`state_style_as` mappings are deliberate opposites.

A **checkbox** is an independent switch: `Checked -> Default`,
`CheckedHovered -> Hovered` and so on, so the surface only ever says what the
pointer is doing and the tick alone says on/off.

A **radio button** is one of a set where exactly one is down, so `Checked`
borrows `Pressed` and latches: the depression is what makes the set readable at
a glance without comparing dots. Measured as (top third - bottom third) with the
glyph removed — a raised block is strongly positive, a pressed one flat or
negative:

    Default +95   Hovered +105                       raised
    Disabled +22  DisabledChecked +7                 raised / down, in grey
    CheckedHovered +13                               down, and lit
    Checked -9                                       down
    CheckedPressed -18                               down, and held

STATE_STYLE's own `Checked` entry measures +4.5 — flat, neither raised nor
pressed, which is why the old checked state read as merely dull.

RadioButton has **seven** blocks, not eight: `Pressed` is unreachable, because
with `flipOnRelease` false `ToggleButton::_receive` flips `checked` on the press
itself, so an unchecked button goes straight to `CheckedPressed` and a checked
one is restored by its ToggleGroup. It falls back to `Hovered`, which is fine
mid-gesture. `CheckedPressed` must NOT be dropped the same way: `bestMatch`
sends it to `CheckedHovered` (+13 against Checked's -9), so a selected button
would appear to *rise* as you clicked it.

Related: `is_checked(state)` tests `"Checked" in state`, not `startswith`.
`DisabledChecked` does not start with "Checked", and a startswith test silently
drops the tick from a disabled ticked box.

## Nested Blueprints are replaced, never merged

The single most expensive mistake in this kit so far, hit twice:

    wkit::Button::create({ .label = { .text = "Save" } })

A designated initializer for a nested Blueprint constructs a **whole fresh
struct**, so the wrapper's `_.style` and `_.layout` defaults are silently gone.
C++ offers no way to merge into an aggregate. And the symptom is not a wrong
font but **no text at all**, because `StaticText` with no style has no font.

Two layers now guard this, both in `header_shell.h`:

1. `init()` calls `Base::setDefaultStyle(TextStyles::Default)` and
   `Base::setDefaultTextLayout(TextLayouts::LeftNoWrap)`. `StaticText::_style()`
   *already* falls back to `Base::defaultStyle()` when the component has none —
   the machinery was there all along, but nothing in WonderGUI ever called the
   setters, so the fallback was null. Two lines turn "invisible" into "visibly
   wrong style" toolkit-wide, including for plain `wg::` widgets built without
   the kit.
2. Every wrapper constructor fills in what the caller left null with its own
   intended value: `if( !label.style() ) label.setStyle(TextStyles::NormalBright);`
   and the same for layout. That restores the *right* style rather than a
   generic one, effectively giving nested blueprints merge semantics. Nine
   wrappers: Button, ToggleButton, Checkbox, RadioButton, both LabelCapsules,
   TextEditor, LineEditor, WindowTitleBar — plus SelectBox, below.

With both in place, `.label = { .text = "..." }` works directly, and
`gallery_test.cpp` was rewritten to use it: no more create-then-`setText`
helpers, every widget a single create call.

A third layer, not done: a one-time `throwError` when a text component renders
with no style and no default, in the spirit of the `scale != 128` guard.

**`wg::SelectBox` used to be a genuine engine bug — now fixed upstream.** It
took `entryTextStyle` for the drop-down list but never initialised its own text
component from anything, and its Blueprint had no field for it, so the closed
box drew nothing however many entries it held and no caller could fix it. The
kit carried a workaround (its own `textStyle`/`textLayout` fields, applied
unconditionally). That is all gone. `wg::SelectBox::Blueprint` now has
`DynamicText::Blueprint display`, the constructor does
`display._initFromBlueprint(bp.display)` like `TextDisplay` and `Button`, and
the component itself was renamed `text` → `display` for consistency with the
rest of the toolkit. The kit's wrapper is now ordinary gap-filling like the
other nine.

It still defaults to `NormalBright`, **not** the `entryTextStyle` beside it: the
closed box is a blue raised control, the same surface as a Button, while the
list is drawn on `Skins::Canvas`. Two backgrounds, two styles — sharing one left
the closed box with black text on blue.

**A nested component Blueprint in a base class is a contract.** Adding `display`
made `bp.display` a hard requirement of `wg::SelectBox`'s template constructor,
so every wrapper Blueprint passed to it must carry that field or it no longer
compiles. `wg_oldskool.h`'s SelectBox wrapper is a bare forward
(`SelectBox(const Blueprint& bp) : wg::SelectBox(bp) {}`) and broke on exactly
this; it now declares `display` too, styled `NormalDark` to match that kit's own
controls. Remember this whenever a widget gains a component blueprint: grep both
widgetkits, not just the one in front of you.

Related sizing trap in the same family: `SizeCapsule::defaultSize` takes **-1**,
not 0, for "ask the child" — `if (defaultSize.h >= 0) pref.h = defaultSize.h;`
so `{260, 0}` forces zero height and the widget vanishes.

## A skin cannot see what is behind it

`_pCapsuleLabelSkin` is a `ColorSkin` filled with `Colors::Plate`, sitting
behind `LabelAndFrameCapsule`'s caption. It is not decoration: `LabelCapsule`
places the label at the top of the *widget* rect (`placementToRect(North, ...)`)
while the frame's outline is inset by the capsule skin's 8pt top spacing, so the
border line runs straight through the caption. The fill is a hole punch.

It reads as an ugly grey patch in widgetbench because the bench paints its root
`Colors::Canvas` (241,244,247) while the punch is `Colors::Plate` (208,215,224).
The colour was chosen for a Plate backdrop; on one it would be invisible.

There is no fix that keeps the straddle, because a skin has no way to sample its
backdrop. Removing the grey means first moving the label clear of the border —
either above the frame or inside it — and both change the capsule's proportions.
Left alone deliberately after weighing it up.

## preview.html

Not CSS `border-image` — that cannot express rigid parts. `build.py` contains a
port of `blitNinePatch` (`simulate_stretch`), so the stretch demos show what
WonderGUI will actually draw. This is the main review tool; it has caught
several real bugs. It renders the content rect (size minus padding) as a dashed
overlay, notes any `spacing_pts` (an outer margin, so not in the bitmap), and
composites a part declaring `preview_over: <Part>` onto that part, so a
DoubleSkin's front layer is shown as the finished widget rather than as dots on
a checkerboard.

It embeds each block as its own image rather than referencing the atlas, so
repacking the atlas leaves `preview.html` byte-identical — which is a free
second check that a packing change moved pixels without altering them.

## Rendering conventions

- Downscale supersampled masks with `Image.BOX`, never `LANCZOS`. BOX averages
  each SxS block, which is exactly that pixel's coverage. LANCZOS rings: it leaks
  alpha outside the shape and leaves sub-255 speckles inside corners and along
  flat edges.
- Define shapes by **continuous** box bounds (`x..x+w`), not inclusive pixel
  indices (`x..x+w-1`). The latter gives fractional slopes that stair-step
  unevenly and asymmetrically.
- Centre glyphs with `centered_span()`, which snaps a run's parity to the extent's.
  The true centre of an N-px span is `(N-1)/2`, not `N/2`, and PIL's line/polygon
  width expands asymmetrically around a coordinate. Continuous supersampled
  shapes use `w/2.0` instead — the two rules are not interchangeable.
- Bevelled glyphs offset their light and dark copies by **half a pixel each
  way**, not the dark one on centre with the highlight a whole pixel off. Same
  bevel, but the pair's bounding box is then symmetric about the block centre,
  which matters when it has to sit inside a rigid run.
- All five arrows share `add_triangle()` so they are one shape; the base is twice
  the base-to-tip depth.
- Engraved detail on a coloured ground needs its dark half checked against that
  ground, not just its alpha raised. The scrollbar grip read as lit rather than
  cut because `grip_lo`'s colour sits close to the handle's own dark blue:
  measured on the mid-tone the highlight was +60 levels but the ridge only -21.
  Both are now at 140/190, where `dot_hi`/`dot_lo` already sat.
- A long bar's gradient is flattened by `gradient_ease` (a power curve that
  concentrates change at the ends), not by pinning a rigid mid-section. Measured
  at 200pts: eased-and-unpinned kept 82% of the length within 2 levels of the mid
  tone; eased-and-pinned only 41%, because the thin flanks either side of the
  rigid run get blown up into two visibly different tones.

## Audited, so you do not have to re-derive it

State of the kit as of the last pass, all of it measured rather than assumed:

- **Every rigid part holds its glyph's drawn footprint.** After the select box's
  arrow turned out to be smearing, all six parts carrying a rigid part were
  checked the same way: build a glyph-only mask by differencing the block
  against a no-glyph render, push it through `simulate_stretch` along the axis
  the widget *actually* grows on, and compare the glyph's bounding box with its
  natural size. ScrollbarHandleX/Y, SelectBox and both SplitHandle dot overlays
  hold their glyph exactly, in every state, from natural size to 12x. The select
  box was the only one that was wrong.
  Two traps in writing that audit, both of which produced false alarms first
  time round: stretch only the axis the widget stretches on (a scrollbar
  handle's thickness is fixed, so growing it across the grain proves nothing),
  and exclude elements that are *supposed* to stretch (the select box's divider
  line spans the box height by design, and lumping it in with the arrow makes
  the arrow look like it grew).
- **`wondergui.h` now includes `wg_skindisplay.h`.** The two widgetbench tests
  still include it directly, which is now redundant but harmless.
- **The pipeline is reproducible.** `render --force` over the whole kit rewrites
  every part byte-identically, and `pack` then reproduces the atlas and the
  header byte-identically. So `parts/` really is derived from `specs/` with
  nothing hand-touched, and the committed artwork matches the specs that claim
  to describe it.

## Known open items

- **A 1x atlas is needed for windows running below scale 128.** The kit is
  drawn at 2x and every pt resolves against the widget's scale, so at 64 the
  grip's 1px ridges land on half a pixel and the 1pt content/glyph shifts
  become one — detail that reads correctly in preview.html (which simulates at
  the atlas's own density) disappears in the app. This is a redraw, not a
  downscale: those details have to be drawn for 1x. `render --density 1`
  already exists, but `pack` refuses mixed densities, so a 1x build needs its
  own parts directory — likeliest shape is a second kit dir sharing specs/ and
  palette.yaml.
- Scrollbar button sizes are constrained: the 16pt dimension must keep matching
  the handles' thickness (`ScrollbarHandleY` width / `ScrollbarHandleX` height).
- Nothing on the split-handle bars constrains their thickness any more, so 8pts
  can come down further if a thinner splitter is wanted.
- RadioButton used to set `Skins::RadioButton` as the widget's `skin` rather
  than its `icon`, stretching a 14x14 frameless ellipse across the whole widget.
  Fixed to match Checkbox. **All wrappers audited since** for a skin put in a
  slot that draws it at the wrong size: one more found and fixed —
  `TreeListDrawer`'s `buttonSize` was 14x14 while `PlusMinusToggle` is drawn
  at 12x12 and has no frame, so the whole sign was resampled 17% larger. It is
  now 12x12; keep it equal to the spec's `size_pts`. Scroller skins
  (backward = Left/Up, forward = Right/Down) and everything else were correct.
