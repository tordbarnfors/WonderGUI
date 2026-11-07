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

- **Warpmaps as triangle meshes,** replacing the grid of nodes so that the number of triangles
  follows the shape: one or two for a PerspectiveWarp, single quads along the straight sides of
  a RoundedRectWarp. Each vertex has a position in the Warpmap and u, v and w (w for
  perspective, interpolated with u and v and divided per pixel or per short run). Done in steps:
  1. **Done:** the Warpmap class and its definition, ReferenceBackend, the stream format, the C
     API, the plugin interface and the tests. Generators still calculate a grid of nodes and make
     two triangles of each cell, with cells sqrt(2) smaller than before to stay within precision.
  2. **Done:** PerspectiveWarp as the quad of its corners with weights, a pixel larger for
     antialiasing and clipped just before the horizon and to the Warpmap, in a fan of at most 7
     triangles. Exact everywhere, so its cell size and precision are gone. Generators can make
     their own mesh through `_meshSize()` and `_generateMesh()`.
  3. **Done for SoftBackend and LinearBackend:** triangles set up when the content changes
     (`SoftWarpmap::Setup`), drawn row by row with their edges stepped exactly from row to row,
     in spans classified outside/solid/mixed. Then OpenGL.
  4. DX12, Metal, and meshes that follow the shape for the other generators.

  Decided so far:
  - **Vertices in spx:** positions in spx of the Warpmap, u and v in spx of the source as the
    nodes are now, not multiplied by w. w is a separate fixed point weight per vertex, 1.0 for
    no perspective, of which only the ratios within a triangle matter. Backends multiply u and v
    by it when setting up triangles. A grid builder for making meshes by hand comes later.
  - **Software backends divide for perspective every 16 pixels** along a row and interpolate
    linearly in between. Interpolating over L pixels is off by about L^2 / (4 * d), d being the
    distance to the horizon in pixels, so runs close to the horizon are shorter: 1, 2, 4, 8 or
    16 pixels, chosen for each run from W at its start to keep within a quarter of a pixel (not
    once per triangle, which gave short runs all over a plane near the horizon). Runs with a
    border of the source or an edge of an edgemap in them get each pixel's position divided out,
    since a quarter of a pixel shows as a quarter of the coverage of a pixel along a border.
  - **Perspective in the software backends is limited to u and v within 32768 pixels of the
    source from the first corner of a triangle,** so that W * (u - u0) fits in 64 bits with
    weights scaled to 2^28. Setting up triangles uses 128 bit products, `__int128` where the
    compiler has it.
  - **No overlapping triangles.** A Warpmap stays a map from each pixel to at most one position
    in the source, with no depth or draw order. Overlapping triangles give undefined rendering:
    nothing checks for it, and generators don't make them.
  - **Triangle edges are not antialiased.** Each pixel belongs to exactly one triangle (shared
    vertices, no T-junctions, a fill rule for centers on an edge), and coverage still comes from
    where the position leaves the source or crosses an edgemap edge, as with the grid. The mesh
    reaches about a pixel beyond the drawn shape, with u and v continued outside the source, so
    that coverage is 0 at its border.
  - **Fill rule:** top-left, as GPUs have it. A center on an edge belongs to the triangle that has
    it as a left edge, or as a horizontal top edge. Centers are 32 spx into the pixel and vertex
    positions are whole spx, so the test is exact with integers.
  - **The triangle that draws a pixel gives positions over the whole pixel,** also where the
    pixel reaches outside it, for coverage as well as colors. Backends estimate coverage from the
    derivatives of that triangle's mapping, and ReferenceBackend samples it.
  - **Counts are fixed when created, content isn't.** The number of vertices and triangles is in
    the Blueprint. Vertices, triangles and periods can all be changed, periods on their own too,
    since u and v are no longer stored brought into the period: they are wrapped after
    interpolation. Unused triangles have all corners in one vertex and draw nothing.
  - **Stream format:** `WarpmapUpdate` gives ranges of vertices and triangles and the periods,
    followed by `WarpmapVertices` (pairs (w, 0), (x, y), (u, v)) and `WarpmapTriangles` (pairs
    (a, b), (c, 0)), compressed by NodeCompressor. Replaces `WarpmapNodes`, without backwards
    compatibility.
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
  triangles, if measuring on real hardware shows that it pays off.
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

- **Positions in the source and periods are in spx of the source.**
- **NodeCompressor** (UTF-8 style prefix varints of deltas) compresses the content in the stream.
  No backwards compatibility was kept with the grid format, since it was never used.
- **Separate cell width and height** for the grids of the generators, so positions that change
  linearly along an axis (a cylinder along its axis, a plane without perspective) take one cell.
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
  back side, or hiding it, is left to the caller. Its mesh has weights proportional to the divisor
  of the projection at each vertex, scaled so that the largest is `Warpmap::maxWeight` for the
  most precision, and is cut off where the divisor drops to half of its lowest at the corners,
  which keeps it off the horizon. The mesh is made in floating point, a few dozen calculations
  per generate.
- **wax::Base,** not wax::Core, is the static base class of Wax. `wg::Base` inherits it, and code
  using both namespaces says `wg::Base` or `wax::Base`.
