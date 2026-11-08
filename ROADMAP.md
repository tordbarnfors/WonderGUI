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
  3. **Done:** SoftBackend and LinearBackend set up triangles when the content changes
     (`SoftWarpmap::Setup`) and draw them row by row, with their edges stepped exactly from row
     to row, in spans classified outside/solid/mixed. GlBackend keeps the mesh of a GlWarpmap on
     the GPU as it is, in its own vertex and index buffers, and draws it once per clip rectangle
     with that as scissor.
  4. **Done:** meshes that follow the shape. CircularWarp and RoundedRectWarp share a band along
     an outline of straight sides and corners (`WarpGenerator::_bandMesh()`): quads along the
     sides, sectors split across the band around the corners, polygons where the band reaches
     past the centers of the corners, all the way around and a pixel beyond the band. CylinderWarp
     is strips along its axis, narrower towards the rims. The grid of nodes that generators could
     implement instead was removed once none did: `_meshSize()` and `_generateMesh()` are what a
     generator implements. A grid builder for making meshes by hand would share vertices between
     cells and is a helper of its own, not part of WarpGenerator.
  5. **Done, untested:** DX12Backend and MetalBackend keep the mesh as GlBackend does, with the
     vertex shader reading the vertices from the buffer of the Warpmap, drawn with its triangles
     as index buffer. See KNOWN_ISSUES.md.

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
    border of the source or an edge of an edgemap in them are split into runs that keep within
    1/64 of a pixel, since a quarter of a pixel shows as a quarter of the coverage of a pixel along
    a border. Keeping interior runs within 1/16 of a pixel too was tried: it halves how much
    colors swim over sharp textures, but drew a steep floor 50% slower (`c_runShift` in
    `wax_softbackend_warp.cpp`), so it stays at a quarter of a pixel for now.
  - **Software backends work out triangles when the content changes,** including where each edge
    starts and how it steps from row to row, so that drawing steps them without divisions, in 32
    bits but for the column. Edges along a row only limit the rows of a triangle. Only triangles
    with pixels are kept, perspective data only for those in perspective, and only the triangles
    on the rows being drawn while drawing: 190 KB for a thick ring of 992 triangles rather than
    480 KB. Without `__int128`, values are worked out in 64 bits where they fit, with the same
    rounding, which makes setting up the ring 9 times faster on a CPU without it.
  - **Perspective runs divide once per point:** positions and derivatives at the start of a run
    share one W, which a reciprocal (one division) divides by exactly, with a few steps of the
    remainder, and steps along a run are divided by its length with a shift where it is a power of
    two, as runs mostly are. Results are the same as with divisions. Coverage of a pixel
    (`halfPlaneCoverage()`) keeps its 32 bit divisions, which CPUs from the 68020 on do in
    hardware, as a reciprocal would cost about as much and give other results.
  - **PerspectiveWarp stays in floating point,** a few dozen operations per generate for a mesh of
    at most 9 vertices, a couple of milliseconds per generate without an FPU. Turning the corners
    (`rotated()`) is in integers.
  - **Vertex positions are within +-2^23 spx** (131072 pixels), so that the steps of edges fit in
    32 bits in software backends and positions are exact as floats in GPU backends.
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
  - **Sizes of the pieces of a band** are chosen for the least work in a software backend, which
    goes by the rows each triangle covers: the number of quads times their length along the
    outline plus their size across. Fewest triangles gave thin slivers, as long as the band is
    deep, which drew a thick ring 1.5 times slower. Precision is shared between how the outline curves
    within a sector (r * a^2 / 8) and the diagonals of its triangles (a * h / 4).
  - **Shape and precision fix the number of triangles,** so changing the radii of a ring or the
    outline of a frame can make generate() create a new Warpmap. Angles, spans and offsets don't
    change it, since the mesh always goes all the way around.
  - **GPU backends keep the mesh as it is** and let the GPU set up the triangles: vertices with
    u * w, v * w and w passed on without perspective correction, so they are linear over the
    canvas, and the position in the source as their ratio at each pixel, with derivatives from
    theirs, which are exact. Updating copies what changed, with nothing worked out on the CPU, and
    drawing only gives where, the source offset and the periods. u and v are relative to those of
    the first vertex, to keep the precision of floats. Working out a mapping per triangle on the
    CPU, which GlBackend first did, took four times the memory and a quarter of the time of an
    update of a thick ring.
  - **Corners of triangles are 16 bits,** a hard limit of 65536 vertices and twice as many
    triangles, which is more than any shape needs. Halves the memory of triangles everywhere and
    is the index format of the GPU buffers. Generators make their pieces larger to keep within it.
  - **Stream format:** `WarpmapUpdate` gives flags, ranges of vertices and triangles and the
    periods, followed by `WarpmapVertices` (sequences of all x, y, u and v, and w only if the flag
    `WarpmapFlags::Weights` is set) and `WarpmapTriangles` (sequences of all a, b and c), compressed
    by MeshCompressor. The sender leaves the weights out when all of an update are 1.0. Replaces
    `WarpmapNodes`, without backwards compatibility. Unknown flags are ignored.
  - **MeshCompressor** replaced NodeCompressor, whose prediction (left + above - above left in
    rows of pairs) was made for the grid of nodes: in a row of a vertex, left is another of its
    values. Each sequence is predicted from its own values a lag back (1 to 32, a column of the
    mesh for most generators), as they are or continuing their change, chosen per sequence by
    trying them on its first 128 values, with runs of zeros in two bytes. The corners of
    triangles are then almost all zeros. A thick ring streams in 1.7 KB rather than 7.9 KB
    (10.1 KB with weights and NodeCompressor), better than zlib or lzma on the same data.
    Encoding takes about 90 us for it on x86, mostly trying lags; trying lags 1 to 64 on 256
    values would save 8% more for eight times that.
  - **Vertices have coverage** (0 -> `coverageOne`), interpolated linearly over a triangle without
    perspective and multiplying the coverage of each pixel, for outlines that aren't borders of
    the source, such as of a ball covered by a map. A row of triangles from full coverage half a
    pixel inside an outline to none half a pixel outside it gives the coverage of a straight edge.
    Software backends only fade triangles that have a corner below full coverage, in spans drawn
    as mixed, so the rest costs nothing; GPU backends multiply by it. A vertex is 24 bytes, and the
    stream only sends coverage (`WarpmapFlags::Coverage`) when an update has any below full.
  - **Weights stay in Warpmap::Vertex** also for meshes without perspective, rather than a
    Blueprint flag that leaves them out. Software backends find out which triangles have
    perspective from the weights of their corners when setting them up, which a flag for the
    whole mesh wouldn't improve on, while GPU backends would need a second vertex layout and
    shaders for it. Leaving them out would save 4 of the 20 bytes of a vertex, so it is only done
    in the stream.
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
  Warpmap chunks that skip trailing bytes they don't know. `WarpmapUpdate` has a flags field,
  the rest is not done yet; to be settled before the freeze.

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
- **No backwards compatibility in the stream** with the grid format, since it was never used.
- **Separate cell width and height** for the pieces of the generators, so positions that change
  linearly along an axis (a cylinder along its axis) take one piece.
- **WarpGenerators** size their pieces from `precision` and generate with integers per vertex:
  CORDIC angles and sines, integer square roots, fixed point factors. Floating point is limited to
  converting the settings once per generate and choosing the layout when the shape changes, for
  CPUs without an FPU. The layout of a band and the strips of a cylinder are kept until what they
  depend on changes, so animating angles, spans or offsets doesn't work them out again. The mesh
  is written straight into the Warpmap in one pass, after counting it without the maths, with
  values that are the same across the band worked out once rather than for each vertex: 13 us
  rather than 45 us for a thick ring on x86.
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
