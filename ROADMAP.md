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
  - **Spans are cheap to set up,** as a triangle costs a span on every row it crosses, which
    for small or thin triangles costs more than its pixels. Drawing triangle by triangle rather
    than row by row wouldn't change that, as a triangle is still drawn a row at a time, and would
    lose joining spans across triangles into runs (edgemaps) and touching each canvas row once.
    Without perspective, the steps of a triangle are rounded and limited once when it starts,
    spans are made in place without clearing them, pixels drawn straight onto the canvas are only
    marked when not all of a span is, and the triangles of a row are kept sorted by merging those
    starting on it with those from the row above. 6-9% fewer instructions and 5-13% less time for
    blits of rings, balls and lenses, with the same pixels.
  - **Perspective runs divide once per point:**  - **Perspective runs divide once per point:** positions and derivatives at the start of a run
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
    trying them on 128 of its values, with runs of zeros in two bytes. The corners of
    triangles are then almost all zeros. A thick ring streams in 1.6 KB rather than 7.9 KB
    (10.1 KB with weights and NodeCompressor), better than zlib or lzma on the same data.
    Encoding takes about 90 us for it on x86, mostly trying lags; trying lags 1 to 64 on 256
    values would save 8% more for eight times that. The 128 values are 8 windows of 16 spread
    over the sequence, as the start of a mesh is often unlike the rest: a BallWarp is a grid
    followed by vertices along its outline. That made balls 10-20% and rings 7-13% smaller, at
    the same speed, within 8% of trying lags on whole sequences, which takes 5 to 25 times as
    long. 4 windows of 32 did worse on small balls. A short stretch unlike the rest can still fall
    between the windows, see KNOWN_ISSUES.md.
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
    shaders for it. Leaving them out would save 4 of the 24 bytes of a vertex, so it is only done
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
- **No cell size for the generators,** only `precision`. Cell size was the size of the cells of
  the grid of nodes. With meshes that follow the shape it meant something different for each
  generator (sectors and parts of a band, strips of a cylinder, pieces of a ball, rings and pieces
  of a lens), as a size in two dimensions for pieces that are seldom rectangles, and nothing for
  PerspectiveWarp. Generators choose their pieces from precision for their shape, such as a single
  piece along a cylinder's axis where positions are linear.
- **WarpGenerators** size their pieces from `precision` and generate with integers per vertex:
  CORDIC angles and sines, integer square roots, fixed point factors. Floating point is limited to
  converting the settings once per generate and choosing the layout when the shape changes, for
  CPUs without an FPU. The layout of a band and the strips of a cylinder are kept until what they
  depend on changes, so animating angles, spans or offsets doesn't work them out again. The mesh
  is written straight into the Warpmap in one pass, after counting it without the maths, with
  values that are the same across the band worked out once rather than for each vertex: 13 us
  rather than 45 us for a thick ring on x86.
- **Precision is the same for all generators:** the largest distance on screen between where a
  pixel shows the source and where it would exactly, measured as the error in the source through
  the inverse of the exact mapping. An audit with that one measure (a scratch tool, comparing
  each generator against its exact geometry at precisions 1 to 0.05) found CircularWarp and
  RoundedRectWarp at 0.7-0.9 of precision, LensWarp at 0.3-0.7 (rings given half of it), BallWarp
  over it (1.05-1.4) and CylinderWarp over it at fine precision, its strips being at least 2
  pixels wide and the last one at each rim taking up to a pixel more. Now all keep within
  precision with 10-40% to spare: pieces are sized for precision less 1/64 pixel
  (`_piecePrecision()`, now `_pieceTolerance()`), for rounding the vertices to spx; ball pieces span
  sqrt(3.4 * precision / r) rather than sqrt(4 * ...); lens rings get 0.7 of precision.
  Cylinder strips are laid out from the rims inwards, so what is left goes to the widest strip in
  the middle and finer precision never gives fewer strips; the last two are made as wide as each
  other, and the middle is one strip across where that keeps within precision. The tests check within precision
  itself, measured on screen (the 1.5 times allowed before came from the grid), and that finer
  precision never gives fewer triangles. Positions in the source are whole spx, so a magnified
  source limits precision to about magnification / 128 pixels, which is left as it is. Any
  precision above 0 is allowed; at several pixels, generators reach their least pieces (8 sectors
  of a ring, 16 x 8 of a ball, 16 around a lens) and the outline becomes a visible polygon. Balls
  and lenses had fewer, but pieces over 22.5 degrees went over precision (2.4 times for a lens
  with falloff at precision 8), as the estimates assume small angles.
- **Precision is 1 by default, and curved outlines are kept within half a pixel**
  (`WarpGenerator::c_outlinePrecision`, `_outlinePrecision()`, now `c_outlineTolerance` and
  `_outlineTolerance()`) whatever precision is set. Most
  GUIs are expected to use 1, hardly any finer than 0.5, and 2 where performance is short. Drawn
  side by side at 0.25 to 2, the inside of shapes off by 1-2 pixels hardly shows, while curved
  outlines and edges along them off by as much show as corners. Precision also hardly changes the
  time to draw on SoftBackend at these sizes, which goes by the pixels (1 -> 2 saves 0-15%), but
  halves generating, memory and streaming. So: sectors of rings and rounded corners keep their
  radii within half a pixel, which also keeps edges of edgemaps along them round; a lens has as
  many pieces along its rim as that takes, rings inside it fewer by precision; a ball adds points
  along its outline between the crossings of the grid (exact positions in the map found by turning
  the point back onto the ball), at most 2 * pi / sqrt(8 * p / r) of them, reserved only where
  the grid alone would leave the outline off by more. At precision 2: a 160 px ball 835 rather
  than 748 triangles, a gauge 56 rather than 48, a lens with falloff about 1.9 times as many, as
  its rim's pieces go for all its rings, with outlines off by at most 0.45 pixels rather than up
  to 1.05.
- **Precision is a quality factor, higher gives more triangles.** It was the largest position
  error in pixels, so a higher value gave fewer triangles, the wrong way around for a setting
  called precision. Generators now keep within `WarpGenerator::c_tolerance / precision` pixels,
  `c_tolerance` being 1, so the default 1 still means a pixel, 2 half a pixel and 0.5 two pixels.
  What 1 means can be tuned later by changing `c_tolerance` alone. Internally pieces are sized from
  `_pieceTolerance()` and `_outlineTolerance()`, in pixels. Values of precision in the entries
  above, written before this, are in pixels: precision 0.25 there is 4 now, 2 is 0.5 and 8 is
  0.125.
- **GPU warps bias positions by 1/32768 of a texel with Nearest sampling,** so that one exactly on
  a border between texels takes the texel after it, as in SoftBackend, rather than either one by
  how the interpolation rounds, which showed as lines where an edge of a Warpmap runs along such
  a border. SoftBackend has positions in steps of 1/65536 of a pixel, so a larger bias moves
  those it has just before a border: 1/1024 made GlBackend differ in more pixels than none,
  1/65536 to 1/32768 in the fewest (gfxdevicetester's Nearest tests from 139, 19 and 19 to 13, 7
  and 1). GlBackend through a uniform, DX12Backend and MetalBackend in the fourth value of the
  third extra.
- **Pieces are at least two pixels across.** Narrower ones are only needed where the source is
  squeezed into each pixel (the rims of a cylinder seen almost to its sides at fine precision, a
  steep falloff of a lens), where they don't show, as the source is still sampled once per
  pixel, but cost a backend for each row they cross. Positions there can be off by more than
  precision: up to 1.5 times it where the source is squeezed up to 4 times, more beyond. Compared
  by drawing over a sharp test image against meshes many times finer: one pixel and two differ
  as little where the source is magnified, while two take fewer rings for steep falloffs.
- **Animation without generating again** through source offsets: `CircularWarp::rotationOffset()`,
  `RoundedRectWarp::shiftOffset()` (in parts of the outline, not degrees),
  `CylinderWarp::rotationOffset()`, `BallWarp::rotationOffset()`. A LensWarp is moved by its
  source rectangle.
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
- **BallWarp:** a ball seen from far away (orthographic, no perspective), with an
  equirectangular map of the whole ball as source, periodic in its width, its middle facing the
  viewer. Turned by tilt (the north pole towards the viewer) and roll (clockwise on screen), spun
  around its axis by a source offset. The mesh is the grid of meridians and parallels cut by the
  outline: a cell is the polygon of its corners in front of the outline and where its sides cross
  it, fanned, with a ring of quads from those crossings out to coverage 0, on a circle half a
  pixel beyond the outline. Crossings are on a circle half a pixel inside the outline, so the ring
  antialiases it as a straight edge does; crossings are worked out on the sphere (where the
  meridian or parallel crosses it), not by interpolating between grid points. Each pole has a
  vertex for each cell around it, with the u of the middle of the cell. Pieces are the same angle
  all over the ball, sqrt(4 * precision / radius) radians, which keeps positions within
  precision also at the outline, where pieces are seen most from the side. The counts are the most the grid can have for
  its size and precision, unused vertices and triangles left empty, so tilting and rolling never
  makes a new Warpmap. Integer maths per vertex, as for the other generators.
- **LensWarp:** a round lens magnifying what is under it, drawn with a source rectangle of the
  same size at the same place, so that moving it takes no generating. One profile for all looks:
  magnification m within R * (1 - falloff), then a cubic Hermite curve to no magnification and
  slope 1 at the rim, which can't fold for m >= 1 (Fritsch-Carlson) and meets the surroundings
  without a seam. falloff 0 is a magnifying glass with a hard edge, 1 a bubble. Round only, and
  magnification only (no concave lenses), for now. The mesh is the middle as one polygon in a
  strip from top to bottom (linear there, and few triangles per row), rings through the falloff
  and a coverage ring around the outline. Every other ring is turned by half a piece, so the
  sides of the triangles between rings span half a piece, which halves how far positions turn
  where s changes fast between rings, and the rings can be further apart: 20-30% fewer
  triangles than rings in line, looking the same. Rings keep both the bend along a radius
  (|s''| d^2 / 8 / s') and that turn within 0.7 of precision, and are at least two pixels apart.
  Pieces along the rim keep the outline within precision; positions between the vertices of a
  ring are off by less, as the source follows the same circle. Rings close to the middle have
  half, a quarter or an eighth as many pieces, where longer pieces keep them within precision,
  at most doubling from one ring to the next with three triangles for each inner piece between
  them: 11-19% fewer triangles for a bubble (falloff 1), nothing for lenses whose falloff starts
  further out. Fewer than 16 pieces only within an eighth of the radius. Lenses measure 0.6-0.8 of
  precision where the source isn't squeezed. Altogether 30-55% fewer triangles than with rings in
  line, a pixel apart and pieces within half of precision, and 12-27% less time to draw on
  SoftBackend, with a largest difference against a fine mesh of 41-62 rather than 30-52 levels
  where the source is magnified. Rings 1/32 pixel apart kept steep falloffs within precision
  everywhere, but only where the source is squeezed many times into each pixel, which doesn't
  show, and took up to 4 times the triangles. The layout is in floating point, a few hundred
  operations when settings change, the vertices in integers.
  A LensCapsule widget, drawing a lens over a CanvasCapsule where the pointer is, comes later.
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
