# Roadmap

Planned changes and the design decisions behind what we have, so they don't only live
in someone's memory or an old chat. Add to it when you decide something, move items to
"Decided" when they are done, and remove what is no longer planned.

## Planned

### Text system

- **Shared style table for Markdown.** Today each `MarkdownTextLayout` gets its role styles
  in its Blueprint. A shared, immutable table of named styles would let layouts share
  them and enable named spans like `[text]{.warning}` (Pandoc/Djot syntax), which md4c
  doesn't parse and we would add ourselves. Deferred to keep the first version simple.
- **Markdown block features:** full-width backgrounds for code blocks, hiding code fence
  lines, horizontal rules, tables and images. See KNOWN_ISSUES.md.
- **WYSIWYG Markdown editing,** with markers hidden while editing. For now editing is done
  with `showMarkup` set, where all markers are visible.
- **Shaping, graphemes and bidi** (likely HarfBuzz), and grapheme-aware caret movement.

### Warpmaps

- **Warpmaps as triangle meshes (being designed),** replacing the grid of nodes so that the
  number of triangles follows the shape: one or two for a PerspectiveWarp, single quads along
  the straight sides of a RoundedRectWarp. Each vertex has a position in the Warpmap and u, v
  and w (w for perspective, interpolated with u and v and divided per pixel or per short run).
  Decided so far:
  - **Vertices in spx:** positions in spx of the Warpmap, u and v in spx of the source as the
    nodes are now. A grid builder for making meshes by hand comes later.
  - **Software backends divide for perspective every 16 pixels** along a row and interpolate
    linearly in between.
  - **No overlapping triangles.** A Warpmap stays a map from each pixel to at most one position
    in the source, with no depth or draw order. Overlapping triangles give undefined rendering:
    nothing checks for it, and generators don't make them.
  - **Triangle edges are not antialiased.** Each pixel belongs to exactly one triangle (shared
    vertices, no T-junctions, a fill rule for centers on an edge), and coverage still comes from
    where the position leaves the source or crosses an edgemap edge, as with the grid. The mesh
    reaches about a pixel beyond the drawn shape, with u and v continued outside the source, so
    that coverage is 0 at its border.
- **WaveWarp:** straight waves (amplitude vector, wavelength, angle, phase) with envelopes
  (none, pinned at the start like a flag, pinned at both ends). Animated with `setPhase()`
  and generating again, or for waves without an envelope by `travel()`: generated one
  wavelength larger and drawn shifted by whole pixels with a matching source offset and a
  clip. Then **RippleWarp** for rings spreading from a point (regenerated per frame). To stay
  solid at the borders, they map onto a source inset by the amplitude rather than a clamping
  edge mode.
- **Filtering when shrinking** hard, such as near the horizon of a PerspectiveWarp or the rims
  of a CylinderWarp. Mostly backend work.
- **Skipping empty or fully covered parts** of a Warpmap, worked out by the backends from the
  nodes, if measuring on real hardware shows that it pays off.
- **Perspective for CylinderWarp,** and a warp along a path.
- **Room to extend before the backend API and stream format freeze:** a flags word in the warp
  commands and plugin functions (unknown bits ignored, so new ones may only add things whose
  absence looks acceptable, such as sub-pixel placement, filtering hints or shading), and
  Warpmap chunks that skip trailing bytes they don't know, with a flags field in
  `CreateWarpmap`. Not done yet; to be settled before the freeze.

## Decided

### Text system (2026-10)

- **UTF-8 everywhere.** Text is stored and passed as UTF-8 `std::string`/`std::string_view`.
  Invalid UTF-8 is replaced with U+FFFD when set. `Utf8::` in Wax has the helpers.
- **Byte offsets.** All character offsets between components and layouts are byte offsets.
  The `TextLayout` decides which offsets are valid caret positions.
- **Markup is part of the text** and interpreted by the `TextLayout`. `PlainTextLayout`
  interprets nothing. `MarkdownTextLayout` interprets CommonMark through md4c (vendored,
  unmodified, `wondergui/source/textlayouts/md4c`).
- **`FlowTextLayout`** is the shared engine: line breaking, rendering, hit-testing and caret
  movement through a single cursor. Subclasses only implement `_parse()`, producing styled
  runs (possibly hidden or shown as one substitute character) and indented blocks.
- **Styles stack** in this order: default style, widget style (the body text), block role,
  nested inline roles, then `State`. Undefined fields inherit. The widget style stays as the
  base style; the layout owns the styles of markup roles.
- **`TextStyle::sizeScale`** scales the inherited size when no size is set, for headings.
- **`TextDecoration` is a bitmask.** Decorations add up when styles stack; `None` removes
  inherited ones.
- **No per-character styling from code.** `setCharStyle()` and `CharBuffer`-style styled
  text are gone; styling comes from markup. Styling ranges without changing the text
  (search highlights etc.) was considered and not needed.
- **Editing Markdown** is done in source mode (`showMarkup`), where the layout maps every
  byte one to one.
- **Legacy GUI** keeps the old `Char`, `CharBuffer`, `CharSeq`, `String` and `TextTool`
  classes, moved into `wondergui/extensions/legacygui`. It is not getting new text features.

### ScrollCapsule (2026-11)

- **Scrolling doesn't need scrollbars.** `scrollX` and `scrollY` alone decide if an axis scrolls.
  Without a displayable scrollbar it still scrolls through the mouse wheel and `setViewOffset()`,
  and the scrollbar takes no space.
- **Minimum size along a scrolled axis** comes only from the scrollbar, not the child, and is
  0 if the scrollbar can't be displayed.

### Warpmaps (2026-11)

- **Nodes and periods are in spx of the source.** Periods are set only together with all nodes
  (`importAllNodes()`), since nodes are stored brought into their periods. There is no
  `setPeriods()`.
- **Stream format:** `CreateWarpmap` (no periods), `WarpmapUpdate` and `WarpmapNodes` compressed
  by NodeCompressor (UTF-8 style prefix varints of deltas). No backwards compatibility was kept,
  since the earlier format was never used.
- **Separate cell width and height,** so positions that change linearly along an axis (a
  cylinder along its axis, a plane without perspective) take one cell.
- **WarpGenerators** calculate cell sizes from `precision` (4-64 pixels along curves) and
  generate with integers only: CORDIC angles and sines, integer square roots, fixed point
  factors. Floating point is limited to converting the float settings once per generate, for
  CPUs without an FPU.
- **Animation without generating again** through source offsets: `CircularWarp::rotationOffset()`,
  `RoundedRectWarp::shiftOffset()` (in parts of the outline, not degrees),
  `CylinderWarp::rotationOffset()`.
- **No culling of empty cells in the GPU backends.** Tried for OpenGL and reverted: the gain was
  questionable for the complexity. Metadata about skippable parts is the planned alternative.
- **No edge mode** (clamping positions outside the source). Generators that need solid borders
  map onto a smaller part of the source instead.
- **PerspectiveWarp:** the Blueprint has only the four corners where the corners of the source
  end up (any convex quadrilateral, counter clockwise mirrors). `rotated()` makes corners from a
  rectangle rotated roll, then turn, then tilt around a pivot and seen from an eye at a distance
  in spx (not a field of view), straight in front of the pivot unless given. Drawing a different
  back side, or hiding it, is left to the caller.
- **wax::Base,** not wax::Core, is the static base class of Wax. `wg::Base` inherits it, and code
  using both namespaces says `wg::Base` or `wax::Base`.

### Software tiling (2026-11)

- **Tiling surfaces of any size.** SoftBackend and LinearBackend wrap tile positions by the size
  of the surface, not a power-of-two bit mask, like the GPU backends and ReferenceBackend.
- **No per-pixel wrapping.** Straight tiles step a pointer and jump back at the edge of the
  source. Transform tiles reduce their steps to less than the source and read in runs that stay
  inside it, wrapping between runs. Cheap enough that keeping a separate power-of-two path wasn't
  worth it: only nearest sampling without blending or tinting, scaled down, measured slower
  (about 15-20%).
- **Custom transform steps are rounded up past the float's rounding error** (see
  `SoftBackend::_transformStep()`), so positions that should land exactly on a pixel boundary do,
  whether stepping forwards or backwards. This matches ReferenceBackend's snapping to boundaries.
