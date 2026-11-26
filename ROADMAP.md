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

### GPU backends

The GPU backends (GlBackend, DX12Backend, MetalBackend, VulkanBackend) each turned the command
stream into GPU work in their own way, so every feature was written four times. GpuBackend
(`wax/source/wax_gpubackend.h`) now does what doesn't need the graphics API, and all four are
built on it and draw with the same HLSL shaders (`wax/extensions/hlsl`). The steps were:

1. **Done:** DX12Backend onto GpuBackend and GpuEdgemap. What is left in it is its root
   signature, descriptor heaps, resource states, device loss and the hooks GpuBackend calls.
2. **Done:** MetalBackend onto GpuBackend and GpuEdgemap. Decided with Tord: its shaders
   are the HLSL, turned into MSL by DXC and SPIRV-Cross (`metalgfx/compile_shaders.py`) and
   checked in, as MoltenVK does it, rather than MSL of our own adapted to GpuBackend. The MSL is
   unreadable, but nobody edits it, and there is one shader source for three backends. Costs:
   macOS 11 (MSL 2.3, for discard to work as in D3D), and pipelines branch on constants like
   DX12 and Vulkan instead of Metal function constants. Each shader is compiled the first time a
   pipeline needs it, with fast math off as the shaders are written for. Both tested by Tord with
   gfxdevicetester on 2026-11-10.
3. **Done:** GlBackend onto GpuBackend and GpuEdgemap. Its hooks record what to do, and
   endSession() uploads the buffers once and plays the recording back, as GL did it before. The
   constants are plain uniforms, set per program when they change: one uniform buffer for all
   draws of a session, bound a range per draw, made llvmpipe copy the whole buffer for every draw.
   Decided with Tord: its shaders are
   the HLSL too, turned into GLSL 3.3 by DXC and SPIRV-Cross (`openglgfx/compile_shaders.py`),
   which also fixes up what GLSL 3.3 lacks: storage buffers become buffer textures, varyings get
   names, and y is flipped for surfaces. GL stays at 3.3, and with that on macOS. GL 4.3 would have
   needed no fixing up, but has no macOS. Code only for GL goes inside `#ifdef WAX_GLSL` in the
   HLSL, so far the vertex input of `vs_warp.hlsl`.
4. **Dropped:** giving GLSL and MSL the file split of the HLSL, as both are generated from it.

For GlBackend, still to do:

- **Timing on GPUs.** gfxdevicetester passes on Windows, on a 2013 Mac Pro and on llvmpipe, but
  has only been timed on llvmpipe, see KNOWN_ISSUES.md.

For MetalBackend, still to do:

- **Reusing buffers.** Each session creates its vertex, color and extras buffers anew, as
  MetalBackend did before, and lets its command buffer keep them until the GPU is done.
- **Smaller MSL, if compiling it at startup turns out slow.** DXC unrolls the loops of the
  shaders at -O3, which makes the MSL about 1 MB. Metal would optimize -Od SPIR-V as well.

For VulkanBackend, still to do:

- **Replacing a lost device,** as DX12Backend::setDevice() does. A lost device is reported and
  rendering stops.
- **A memory suballocator.** Every buffer and image has an allocation of its own, which stays
  well within limits for a GUI, but Windows drivers can have as few as 4096 allocations.
- **wappsdl2vk,** a Win32 framework, and running on Windows at all.

For SdlGpuBackend, still to do, see "SdlGpuBackend" under Decided:

- **What differs with Vulkan on Windows,** see KNOWN_ISSUES.md: the tests of blend modes with the
  Vulkan driver of an Arc B580. It has drawn with D3D12 and Metal since 2026-11-25, see
  "SdlGpuBackend" under Decided.
- **Timing on other GPUs than Intel UHD 770,** where it is as fast as VulkanBackend but for the
  fixed cost of a session, about 0.1 ms more, from a command buffer of its own.
- **A lost device,** which SDL_GPU doesn't tell us about. Restorers are never called.

### GPU surfaces: left after "pixels kept once"

"GPU surfaces: pixels kept once" under Decided is done but for this:

- **It went into v4 from gl-shape-performance,** in one piece with what has no API change (Tord,
  2026-11-24: "just move on with 2 and 3 locally", 2026-11-25: "you can merge branch to v4").
  breaking-changes has none of it: not the restorer, `buffered` leaving, nor what went out of
  GlSurface and MetalSurface.
- **`loadSurface()` of WonderApp gives no restorer,** so what it loads keeps its pixels (Tord,
  who plans to change its API). Until then, what an application loads through it takes twice its
  pixels on all backends, which on GlBackend is more than before.
- **Restorers are only called by DX12Backend,** which is the one that replaces its device. On
  the others a restorer says the same about what to keep, and is called once they can replace
  theirs, see "VulkanBackend: a lost device stops rendering for good" in KNOWN_ISSUES.md.
- **Alpha in one pass:** pixels with alpha written to a surface with restorer are gone through
  twice, for the alpha map and for premultiplying, where one function could do both.
- **Alpha without the second pass:** a flag for pixels the producer has premultiplied, or a
  texture with straight alpha that the shader multiplies, which is right at 1:1 and with nearest
  sampling but lets the color of transparent texels in at edges when scaled.
- **Video as YUV planes, and decoding on the GPU.**
- **Writing into the texture without staging on integrated GPUs.** Each API has something for it
  as I recall (`WriteToSubresource()`, `VK_EXT_host_image_copy`, `replaceRegion`), not looked up.
- **`RGB_565` textures for sRGB pixels,** which Tord would rather have where the driver has them.
  No API has an sRGB 565 format, so it takes decoding in the shaders and pipelines for it.

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
  5. **Done:** DX12Backend and MetalBackend keep the mesh as GlBackend does, with the vertex
     shader reading the vertices from the buffer of the Warpmap, drawn with its triangles as index
     buffer. Both are tested.

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
- **Lens rings have any number of pieces, not halvings.** They first had half, a quarter or an
  eighth of the pieces of the rim (11-19% fewer triangles for a bubble), which took halvings
  per ring, radius limits for each, a pass keeping neighbours within one halving, the rim
  rounded to a multiple of 8 and two ways to join rings. Once the rim got its pieces from the
  outline tolerance, removing that would have taken 36% more triangles at precision 0.5, 14%
  at 1 and 5% at 2 (ten lenses). Instead each ring has as few pieces as keeps it within the
  tolerance, at most as many as the rim, and at least a quarter more than the ring inside it
  where it has more, so that most rings have as many as the next. Two rings are joined by
  walking around both and taking the next piece of the one whose next piece has its middle
  first, which works for any numbers and makes their sides span at most half a piece of each.
  Where rings are as close as allowed and the change doesn't fit, a pass from the rim inwards
  gives the inner ring the pieces of the outer, moving the change inwards. Shorter code, and on
  the same ten lenses as many triangles at precision 0.5, 8% fewer at 1, 6% fewer at 2 and 7%
  fewer at 4. Single lenses range from 29% fewer to 47% more: most bubbles have fewer, while a
  plain glass (70 -> 103) and a falloff of 0.1 (286 -> 379) have more at 0.5, as halving could
  halve the ring at the inner edge of the outline. Tried and dropped: the ring at the inner edge of the outline with
  half of the pieces of the rim (13-16% fewer altogether, but the outline 0.8 pixels off, as
  its vertices don't line up with those of the rim as halving had them), placing rings for the
  pieces of the rim and reducing afterwards (too few reductions), and choosing per ring between
  its fewest pieces and those of the rim by triangles per pixel (more triangles altogether).
- **GPU warps bias positions by 1/32768 of a texel with Nearest sampling,** so that one exactly on
  a border between texels takes the texel after it, as in SoftBackend, rather than either one by
  how the interpolation rounds, which showed as lines where an edge of a Warpmap runs along such
  a border. SoftBackend has positions in steps of 1/65536 of a pixel, so a larger bias moves
  those it has just before a border: 1/1024 made GlBackend differ in more pixels than none,
  1/65536 to 1/32768 in the fewest (gfxdevicetester's Nearest tests from 139, 19 and 19 to 13, 7
  and 1). The GPU backends have it in the fourth value of the third extra, see GpuBackend.
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
  fewer pieces, as few as keeps each within precision, see the next entry. Fewer than 16 pieces
  only within an eighth of the radius. Lenses measure 0.6-0.8 of
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

### GPU backends (2026-11)

- **What doesn't need the graphics API is shared,** where it was written for each backend:
  GpuTools (decoding state changes, blur taps, lines without caps), GpuShapes, TintTools, and
  GpuBackend and GpuEdgemap, which do the whole command stream and the content of edgemap
  buffers through a dozen protected methods a backend implements. Those methods are called per
  draw, not per vertex, so being virtual costs nothing measurable. What does need the API stays
  in each backend, duplicated in structure where the APIs are alike, as that is where they differ
  in ways that matter for reliability: lifetime of resources, synchronization, device loss.
- **Blending and the choice of pipeline are GpuBackend's too.** `_blendState()` works out the
  fixed-function blending of a kind of draw, blend mode and kind of canvas in factors and
  operations of its own, which each backend turns into those of its API with a table. It is the
  only place the blend modes are mapped to the GPU blender, where all four had a copy of it.
  GpuBackend drops draws with Ignore and normalizes the blend mode before it asks the backend to
  bind a pipeline (`_bindPipeline()`), and `_pipelineKey()`, `_vertexShaderName()`,
  `_pixelShaderName()` and `_nbVertexAttributes()` are what the backends tell pipelines, shaders
  and vertex inputs apart by. All four find their shaders by those names, VulkanBackend from a
  table CMake makes of `vulkangfx/spirv`. An alpha only canvas gets the factors of alpha in their
  color form for its colors, which serves the R8 of GL, Metal and Vulkan and the A8 of DX12
  alike. DX12Backend offers all four vertex inputs to every vertex shader, as D3D12 wants each
  one a shader declares, also those it doesn't read. Vulkan and Metal only get those a shader
  reads, as DXC removes the unused one from their shaders. gfxdevicetester gives the same
  results as before the change on all four: DX12Backend and GlBackend on Windows (Arc B580),
  VulkanBackend on Linux (Intel UHD 770 with Mesa, validation layer on and silent) and
  MetalBackend on a Mac (x86_64, Debug), each at bcf58e51 against d155b18a.
- **What GpuBackend tells the error handler of once, it says once for each backend**
  (`_firstReport()`, a bit for each kind in `m_reported`). They were statics of the functions, 17
  of them, so once for the process: a second backend said nothing, nor did one that came after a
  device was replaced. A buffer that is full is told of once for each kind of draw it drops.
- **A session without one of its three frame buffers draws nothing** (`m_bSessionHasBuffers`, set
  by `_reserveBuffers()`, which is what says so). Each draw looked for the buffers it writes to,
  to draw what could be drawn without the missing one. A backend only leaves a buffer out when it
  couldn't make it, and next to nothing draws without all three. A buffer that couldn't grow is
  still there and still draws what fits.
- **One `_addQuad()` writes the six vertices of a quad,** which fills, blits, lines with and
  without caps, edgemaps and shapes each did in a loop of their own. Timed with DX12Backend
  (Arc B580, gfxdevicetester, two runs each): the median of the 262 tests is 0.999 of what it
  was, each between 0.93 and 1.04.
- **Left as it is:** blits, lines and edgemaps step through their part of the command stream
  themselves, as they read it one rectangle or line at a time, where the other draws are handed
  what processCommands() has read.
- **Decided with Tord: edgemaps keep a GPU buffer of their own,** as surfaces have their
  textures. Copying what a frame draws of them into its extras buffer was considered and turned
  down: an edgemap can be drawn many times a frame, and a resource shouldn't fill up the buffers
  of the frame. So each backend keeps its edgemap class, which is what makes that buffer. The
  64 KB a small DX12Edgemap, DX12Warpmap or palette took is solved by shared blocks instead, see
  "DX12Backend: small buffers out of shared blocks" below.
- **DX12Backend and VulkanBackend draw with the same HLSL shaders** (`wax/extensions/hlsl`), see
  its README.md. DX12 gets them as bytecode compiled by FXC, Vulkan as SPIR-V compiled by DXC.
  The constants are declared once, in constants.hlsl, as root constants for DX12 and push
  constants for Vulkan. A local variable may not be named like a constant, as they are defines
  for Vulkan.
- **SPIR-V is checked in** (`vulkangfx/spirv`), as binaries rather than as generated source,
  which was three times the size, and turned into arrays by CMake, so building needs no shader
  compiler.
- **VulkanBackend needs Vulkan 1.3** with dynamic rendering, synchronization2, timeline
  semaphores, demote to helper invocation (how DXC compiles discard) and dual-source blending,
  and VK_KHR_push_descriptor. That is what lets it work like DX12Backend: render targets without
  render pass objects, resources bound per draw like root descriptors, constants like root
  constants.
- **One timeline semaphore counts all submissions to the one queue** that all VulkanBackends
  share. A resource let go of is destroyed once the GPU has passed every submission that may use
  it, including those still being recorded (VulkanContext::destroyLater()), so nothing has to hold
  on to them per command buffer as DX12Backend does.
- **Alpha_8 is R8 in Vulkan,** read as alpha through the swizzle of the view, rather than A8, which
  needs an extension. The shaders write the value of an alpha only canvas to all channels, so the
  blend factors of color do what those of alpha do on A8. XRGB is BGRA read with alpha 1 and
  rendered into without touching alpha, as Vulkan has no BGRX.
- **Windows render into an image of their own,** copied to the swap chain when something was
  drawn, as Wax only draws what changed and swap chain images don't keep their content.
  wappsdl3gl does the same, see WonderApp on Wayland below.
- **Mipmaps are made with linear blits** in Vulkan, rather than the shaders DX12Backend draws them
  with, which average the same four texels.

### Surfaces created without pixels (2026-11)

Decided with Tord: `Surface::Blueprint` has `fillColor`, which a surface created without pixels
is filled with. It defaults to `HiColor::Undefined`, which fills nothing in and leaves the content
unspecified until all of it has been drawn or written. It was unspecified already on SoftSurface
(memory as it came) and GlSurface (a texture given no pixels), while DX12Surface filled its pixels
with zeroes and uploaded them, which cost more than making the surface for something like a
canvas layer that is cleared right after.

- **How it is filled is up to the surface.** For now each fills the pixels it keeps with
  `PixelTools::fillBitmap()`, as `Surface::fill()` does, and sends them to its texture as it sends
  supplied pixels. GlSurface fills a buffer for the upload when it keeps none. A clear on the GPU
  is the faster way for a canvas that keeps no pixels, and is left for when there are such.
- **Ignored when pixels are supplied,** and `Surface::blueprint()` returns it undefined: it says
  how the surface was to start out, not what it is.
- **Indexed surfaces** get the nearest color of their palette.
- **gfxstream:** the CreateSurface chunk ends with the fill color, 48 bytes instead of 40. The
  player reads it when the chunk is that long, so older streams still play, but an older player
  can't read a new stream.
- **C API:** `wax_surfaceBP` has `useFillColor` besides `fillColor`, as a zeroed struct would
  otherwise ask for transparent black. In C# `Surface.Blueprint.FillColor` is null by default.

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

### Line caps on CPUs without FPU (2026-11)

- **The ends of lines are rasterized in 32-bit math,** see `SoftShapes::_rasterizeSmallPart()`.
  Integer math alone wasn't enough: `int64_t` for every coordinate made a 1 pixel wide line take
  about 10 ms on a 16 MHz 68030, most of it in the general rasterizer. Parts up to 128 pixels
  use coordinates relative to the part, with only 32x32 to 64-bit multiplications left, about
  2.2 times faster there. Larger parts stay with the general 64-bit rasterizer, where
  pixel writes dominate anyway.
- **Same pixels as the general rasterizer, by design.** Reciprocals are kept as the bits above
  and below the 5 lowest of 2^37 / n so that every product comes out exactly as in 64 bits,
  which lets the two be compared pixel for pixel.
- **Setup of lines is in 32-bit math too,** see `SoftShapes::prepareLine()`. Thickness is limited
  to 65535 spx, which is all that commands can hold, so profiles of caps fit in 32 bits. Only a
  handful of 64-bit divisions per line remain, one set for the direction and slope and one per
  Vertical or Horizontal cap and per round cap, and lines allocate nothing once buffers have grown.
  Output is the same as before, down to the rounding.

### Build setup (2026-11)

- **`CMakePresets.json` is the shared build setup,** with a configure preset per WonderApp
  framework and platform, read by VS Code, Visual Studio, CLion and `cmake --preset`. All of
  them build in `build/`, as the folder per platform (`tuxbuild/`, `vsbuild/`) was an accident of
  history. Linux builds with Ninja, in Debug or, with the `-release` presets, Release. Windows
  builds with Visual Studio and macOS with Xcode, both multi-config.
  Personal presets go in `CMakeUserPresets.json`, which is git-ignored.
- **`.vscode/` stays out of git.** Developers use different IDEs, and VS Code needs little: one
  launch configuration for the launch target picked in CMake Tools, run from the repository root,
  covers every tool, see `README`. CMake has no target property for a VS Code debugger, as it has
  for Visual Studio and Xcode in `target_working_directory()`.

### WonderApp on Wayland (2026-11)

- **wappsdl3gl windows render into a GlSurface of their own,** flipped into the back buffer and
  swapped in when something was drawn, as Wax only draws what changed and the back buffer doesn't
  keep its content. What is drawn into the front buffer never shows on Wayland. The bytes are
  copied as they are, as the window of an EGL context isn't sRGB-capable.
- **SDL3 windows present again when exposed.** The decorations SDL draws with libdecor only change
  when the window commits a frame, so without that the border of a resizable window keeps an input
  region over all of it and catches the pointer.

### gfxdevicetester timing (2026-11)

Three runs of all 294 tests on DX12Backend (Arc B580, `gfxdevicetester --performance`) are within
10% of each other for every test and within 5% for 289 of them, median 0.6%, after the first four
changes below. Before them two saved results differed by -83% to +90%. On GlBackend two runs are
within 10% for 250 of 257 tests, leaving out four suites drawing shapes that come at two speeds
there, see KNOWN_ISSUES.md.

- **A measurement ends with `Backend::waitForCompletion()`, not with reading back a pixel**
  (`sync_testee()`). The GPU backends read back the whole canvas for one pixel, which took 16-17 ms
  on DX12Backend (Arc B580) against 10 ms for the calls being timed and 0.2 ms for an empty render
  pass. Subtracting it as baseline wasn't the problem, leaving the GPU idle for 2/3 of the time
  was: it lowered its clocks, by how much depended on what had run before, and the same test came
  out anywhere from 1x to 6x its real time (Tile 2.7 us, measured as 7.6-17 us), differing by -83%
  to +90% between two saved results. Whatever is added to a measurement that isn't the test has to
  stay far below `c_perfTargetTime`.
- **A measurement is several frames sent off back to back, waiting only for the last**
  (`time_test()`, `c_perfFrames`). Waiting for every frame left the GPU idle while the next one
  was recorded, and its clocks went up and down in a sawtooth: the same measurement repeated back
  to back took anywhere from 7 to 12 ms. Sent off as an application would, the GPU works on one
  frame while the next is recorded. The time per run is therefore how much longer a frame takes to
  get through, the time of the calls or that of the GPU, whichever is longer, and not their sum
  as before. Results from before this can't be compared with new ones. A change on the side that
  isn't the bottleneck of a test doesn't show, timing the calls and the GPU separately would be
  the way to get at that.
- **A GPU gets a heater before each test** (`heat_testee()`): blended fills of the whole canvas,
  next to nothing to record and a lot to draw, run until they are as fast as we have seen them.
  A test that leaves the GPU with less to do lets it lower its clocks, be it one where recording
  takes longer than drawing or one with a slow `init()` or `exit()`, and the tests after it then
  took up to 9 times as long in all their measurements of a pass (PatchBlitTests, which come
  after CanvasFormatTests). Running the test itself for longer was no way out, since we don't
  know how fast it should be and the clocks come up in steps with plateaus of half a second. The
  heater is the same every time, so its time tells us where the clocks are.
- **Calibration settles for the same number of calls every time** (`calibrate_test()`). The time
  per run changes with the number of calls in a frame (BlitGridInter 180 us with 12 calls, 220 us
  with 100), and the number came from the time of a single call, which is mostly noise for a fast
  test, so it differed by up to 10 times between two runs of the tester. Now it only goes by
  measurements long enough to trust and is rounded to a few values per decade.
- **Performance runs keep to the fastest cores** (`FastCoresScope` in `gfxdevicetester.cpp`, Linux
  only so far). On a hybrid Intel CPU the scheduler moved the timing thread between performance and
  efficiency cores every second or so, which made tests bound by the CPU, like those generating
  Warpmaps, 20-70% slower for seconds at a time.
- **Clocks are left to the user.** Laptops lower their clocks once a power budget averaged over
  tens of seconds is spent, which halved the P-core clock of an i5-1245U after about half a
  minute of runs, at falling temperatures. Runs back to back don't let it recover. For comparable
  results, turn turbo off (`intel_pstate/no_turbo`) or cap the clock while timing.

### Shape shaders on GPUs (2026-11)

On GlBackend, rounded boxes took 1.8 times as long as on the old GlBackend with its own GLSL, on
Intel UHD 770 (Mesa). Rings and pie charts took 1.1 times as long, on Windows (Arc B580) all shapes
3-11 times as long. The shaders work the same, the time went to how the HLSL is turned into GLSL.
Now rounded boxes take 0.7 times as long as on the old GlBackend, rings and pie charts 0.8-0.95
times and line caps 0.25-0.4 times, and VulkanBackend gains about the same. On Windows (Arc B580)
line caps still took up to 4 times as long as on the old GlBackend after the first three changes
below, when the other shapes were level.

- **DXC compiles as HLSL 2018** (`-HV 2018`, all three compile_shaders.py). The default, HLSL
  2021, short-circuits `&&`, `||` and `?:`, so each became a branch, and a branch that reads a
  buffer isn't flattened by the drivers: the corner tests of a rounded box were a chain of
  branches each waiting for a read. FXC, which DX12Backend compiles with, never short-circuits, so
  the shaders already had to work without it. Rounded boxes 1.5 times as fast.
- **A pixel shader for each kind of shape** (`shape.hlsl` and `ps_shape_box.hlsl`,
  `ps_shape_ring.hlsl`, `ps_shape_line.hlsl`, `PipelineKind::ShapeBox`, `ShapeRing` and
  `ShapeLine`), as warps already had. With all three in one, rounded boxes, which need the least,
  took 1.6 times as long, rings and lines 5-10% longer: the driver gives a shader registers and
  threads by its largest part. Drawn boxes alone also get SIMD16 on Intel, all three together
  only SIMD8.
- **Plain min() and max() in the GLSL** (`plain_min_max()` in `openglgfx/compile_shaders.py`). DXC
  makes HLSL's min() and max() NMin and NMax, which SPIRV-Cross can only write in GLSL as two
  isnan() and two selects around each. About 10% for rings and rounded boxes.
- **Lines without arrays indexed at run time** (`ps_shape_line.hlsl`). The part of a pixel inside
  the planes of a line was found by clipping the pixel to each plane in turn, with the corners in
  arrays indexed by how many there were. Mesa read them through chains of selects, 4760
  instructions and only SIMD8, and drivers that can't keep such arrays in registers put them in
  scratch memory. Now the area comes from Green's theorem: each pixel edge and the line of each
  plane is clipped to the others as an interval, and the pixel is half the sum of each interval
  times how far it is from the pixel center. Indices are all constants once loops are unrolled.
  Most pixels are crossed by one plane at most, which `halfPlaneCoverage()` covers, and the
  others mostly by two or three, so only those planes are gathered for the intervals, which take
  about n * n steps. Four or more, in lines thinner than 1.4 pixels and at spears, take a loop
  over all six planes of a line, which turns them around one place per step instead of indexing
  them. Unrolled, that and working out both round ends the general way took too many registers
  for SIMD16 on Intel. Mesa makes about 2170 instructions of the shader now, most of them in
  branches a pixel doesn't take, and runs it as SIMD16. On Intel UHD 770
  (Mesa) line caps take 0.3-0.5 times as long as before on GlBackend and 0.2-0.45 times on
  VulkanBackend. Results are the same as before in all of gfxdevicetester on both: the area is
  exact both ways.
  Lines on top of each other need care: a pixel edge and a plane facing the same way count once,
  and planes facing each other both count, which cancels out. Lines less than 1e-6 apart in
  direction are taken as parallel, with the distance between them worked out the same from both,
  as normals that should be exactly opposite can be a little off, and a multiplication fused into
  an addition leaves the cross product of two of them a little off 0.
- **No pipeline for lines without caps unless there are some** (`GpuBackend::_drawLines()`). A
  line command set the pipeline of lines without caps first, then that of lines with caps, so
  one with only capped lines switched pipelines twice for nothing. On Windows (Arc B580) the line
  caps tests that draw each line in a clip of its own, one command per line, stayed at 4.6 times
  as long as on the old GlBackend after the new line shader, which made the others 1.5-13 times
  as fast. Those tests now take 0.65-0.9 times as long on GlBackend and 0.4-0.9 times on
  VulkanBackend on Intel UHD 770 (Mesa), where the shader takes most of their time.

Not yet timed on Windows, where the shapes were slowest, nor with DX12Backend and MetalBackend.
MetalBackend gets all but the plain min() and max(), which MSL has, DX12Backend only the shader
per kind, as FXC doesn't short-circuit.

### DX12Backend: surfaces (2026-11)

Compared with GlBackend on the same Windows machine (Arc B580), DX12Backend was level in all of
gfxdevicetester (median 1.01) but for tests making D3D12 objects or reading pixels with the CPU:
canvas layers 130-200 times as long, WarpTests making their Warpmaps for each run 5-17 times,
ReadbackTests twice. Timed with the changes below (2026-11-23): canvas layers 5-9 times as long,
ReadbackTests 1.7 times, and with canvases without a pixel buffer, see the section of that name
below, canvas layers 1.5-3 times as long. See "DX12Backend: canvas layers" in KNOWN_ISSUES.md for
what is left.

- **The pixels of a DX12Surface are in CPU cached memory** (`_createDeviceObjects()`, 95e05825),
  a custom heap with the properties of a READBACK heap, created in COMMON. The pixels are read as
  much as they are written: by whoever has a PixelBuffer, by `alpha()`, by the premultiplying copy
  for uploads and the unpremultiplying after readback. In the upload heap, write-combined on a GPU
  of its own, reading took around 60 ns per pixel. The GPU only ever copies from these pixels.
- **Premultiplying writes its buffer once and never reads it** (`GpuTools::convertAlpha()` from
  one buffer into another), as that buffer is in the upload heap. Uploads copy straight from the pixels when
  they are all opaque or transparent black, as a new canvas is, without a buffer of their own.
- **No buffers kept for Warpmaps that might be made.** Each DX12Warpmap makes a committed
  resource, about 250 us. Keeping the buffers of Warpmaps that go away for new ones, up to 32 MB,
  was tried and taken out again: Warpmaps are usually made once and kept for long, and memory
  shouldn't be held for what might come. Shared blocks, which GL drivers carve buffers out of,
  took the cost away since, see below. WarpTests make their Warpmaps in `init()`, as
  the other suites do with what they draw, so they compare drawing. "Cylinders made each run"
  makes them for each run.

### GPU surfaces on GpuSurface (2026-11)

Decided with Tord. DX12Surface and VulkanSurface were the same class twice on the CPU side, and
all four GPU surfaces had their own constructors, palette conversion and premultiplying.

- **GpuSurface (`wax/source/wax_gpusurface.h`) is the base of DX12Surface, VulkanSurface and
  MetalSurface.** It has the pixels the CPU reads and writes, which of pixels and texture is the
  newer and where (`m_dirtyRect`, `m_bBufferNeedsSync`), the palette, the pixels in plain memory
  of a surface the API gives nothing for, and the pixel buffer a canvas gets when its pixels are
  first asked for. A subclass picks the format of the texture, makes what is on the device and
  copies between pixels and texture: `_setTextureFormat()`, `_createDeviceObjects()`,
  `_createPixelBuffer()` and `_readBack()`, and `_waitForUpload()` and `_updatePending()` if its
  uploads need them. That follows the line of GpuBackend: lifetime of resources,
  synchronization and device loss stay with each backend, as does when an upload happens.
- **A surface is made by one sequence** (`GpuSurface::_setup()`), called by the constructor of the
  subclass as it calls what the subclass implements. The four `create()` of each class turn their
  arguments into a `PixelSource`, so each class has one constructor.
- **`create()` returns null for a blueprint that isn't valid or a format without a texture,** on
  all four, as GlSurface and MetalSurface did. DX12Surface and VulkanSurface gave a surface
  without a texture and an error. Pixels in plain memory are for when the device gives nothing:
  none set, lost, or out of memory.
- **Pixels are little endian,** as the texture formats are laid out by byte. DX12Surface and
  MetalSurface said native, which is the same on what they run on.
- **Premultiplying and the palette as the shaders read it are in GpuTools,** used by all four.
  What is sent to a texture gets no premultiplied copy when every pixel is opaque or transparent
  black, which only DX12Surface looked for.
- **MetalSurface sends pixels to its texture when it is used,** where `pullPixels()` committed a
  command buffer and waited for the GPU every time, and creating a surface with pixels did too.
  MetalBackend has it done before the surface is drawn from or into (`syncTexture()`), in a
  command buffer of its own committed there, so ahead of the session in the queue. The copy reads
  the pixels when the GPU gets to it, so `allocPixelBuffer()` waits for the last one. The palette
  is in plain memory, where it was in a Metal buffer nothing on the GPU read.
- **All three backends call `notifyRendered()` when they bind a surface as canvas** (Tord:
  consistency first, then fix what breaks). MetalBackend kept a list of the canvases of a session
  and marked them when the session had been committed, as reading a texture back doesn't wait for
  a session being recorded, which it does with DX12Backend and VulkanBackend. See "MetalSurface:
  pixels asked for during a session" in KNOWN_ISSUES.md for what that leaves. Changed after the
  test runs below, and not compiled.
- **GlSurface isn't built on GpuSurface.** It keeps no pixels unless `buffered`, palette based or
  `Alpha_8`: the texture is the only copy, `allocPixelBuffer()` makes a buffer for the call and
  `alpha()` reads an alpha map of a byte per pixel. That is the leanest of the four and another
  model, so a base class would only have been in its way. It uses the helpers in GpuTools. Making
  the other three as lean, which is what `buffered` says, is a decision of its own, taken since:
  see "GPU surfaces: pixels kept once" below, where GlSurface was put on GpuSurface after all.
- **Left in:** `unload()`, `reload()` and `isLoaded()` of GlSurface and MetalSurface and the two
  pixel counters of GlSurface. Nothing in the repository calls them, but they are public API, so
  taking them out belongs on breaking-changes.

Tested on Windows (Arc B580): gfxdevicetester gives the same results as before on all 262 tests
with DX12Backend and GlBackend, unittester passes, and in a Debug build 51 tests of canvases,
layers, readback, mipmaps, palettes and warps give the same with the device removed and replaced
before they run. VulkanBackend on Linux (Intel UHD 770 with Mesa, validation layer on and silent)
and MetalBackend on a Mac (x86_64, Debug) built as they were written and give the same results
as before on all 262 tests too, VulkanBackend also with garbage written to every new image and
pixel buffer. All at e4e1e9b5 against 50102486.

### DX12Surface: canvases without a pixel buffer (2026-11)

Decided with Tord, after timing where canvas layers spent their time: 2.4 of the 2.55 ms of
gfxdevicetester's CanvasLayerTest1 went to making the four DX12Surfaces of the layers, each a
texture (345 us), a buffer for its pixels (160 us), zeroes written to that buffer (17 us), all of
it read to see if it needed premultiplying (64 us) and then uploaded to the texture, which
DeviceGen2 clears with a fill right after.

- **A canvas that isn't `buffered` and is created without pixels or fill color has no pixel
  buffer** until its pixels are asked for: `allocPixelBuffer()`, `pushPixels()` and `alpha()` make
  it (`GpuSurface::_needPixels()`), marked as older than the texture, so it is read back into before it is
  read. That is what `buffered` of the Blueprint says, which DX12Surface didn't look at. It is
  half the memory of a canvas nobody reads, and with the upload gone CanvasLayerTest1 takes
  0.76 ms where it took 2.45 ms. Other surfaces are written by the CPU and keep their buffer, as
  does an indexed canvas, which can't be drawn to.
- **With a fill color the canvas gets its pixel buffer at once,** filled and uploaded as for any
  surface. Clearing the texture on the GPU instead would save that for a canvas nobody reads, but
  it has to be recorded by a DX12Backend, as uploads are. Left until something needs it: canvas
  layers are created without a fill color and cleared by DeviceGen2.
- **On the device is having a texture,** which `_evacuate()` and `_migrate()` went by the pixel
  buffer for. A canvas without one gets it when a working device is let go of, to read back into.
  When the device is lost it has nothing but what was rendered, which is gone, and it is marked
  as having lost its content like any canvas. Back on a device it has a pixel buffer, as its
  pixels come from plain memory then.
- **Not kept between canvas updates:** what is left of a canvas layer is making its texture,
  about 350 us at 512x512. Placed in a heap that is there already it takes 21 us, but Tord doesn't
  want canvases or memory for them kept behind the scenes. If it is to be solved, by layer
  surfaces that whoever owns the canvas supplies or keeps.
- VulkanSurface and MetalSurface have the same since they were built on GpuSurface, see "GPU
  surfaces on GpuSurface" below. VulkanSurface no longer sends the unspecified pixels of a new
  surface to its texture: an image nothing has been sent to stays in the layout it was created
  in, which VulkanBackend takes it out of like any other.

Tested on Windows (Arc B580): gfxdevicetester gives the same results as before on all 262 tests,
and in a Debug build the same for 43 tests of canvases, layers, readback, mipmaps and warps with
the device removed and replaced between starting up and running them, which takes the surfaces
made by then off the device and back, as without. That took holding the run back some frames, see
SUGGESTIONS.md. Not tested: the device removed while drawing, and a working device being let go
of, which is when a canvas without a pixel buffer gets one to read back into.

### GPU surfaces: staging and plain memory (2026-11)

Step 1 of "GPU surfaces: pixels kept once" below, decided with Tord 2026-11-24. It
replaces what "DX12Backend: surfaces", "GPU surfaces on GpuSurface" and "DX12Surface: canvases
without a pixel buffer" above say of where the pixels of a surface are and of the pixel buffer of
a canvas.

- **GpuSurface has the pixels of every surface but a canvas, in plain memory.** A subclass makes
  staging memory (`_allocStaging()`), has a rectangle of it copied to the texture (`_pull()`) and
  reads a rectangle of the texture back (`_readBack()`). It no longer has a buffer with our
  pixels, nor says how their lines are to be padded.
- **Lines of kept pixels start on four byte boundaries.** With D3D12 they were padded to 512
  bytes, as what a texture is copied from has to be, so a 16 x 16 icon had 8 KB of pixels where it
  now has 1 KB.
- **`buffered` of the Blueprint does nothing** for the three, until it goes.
- **A PixelBuffer of a canvas is staging memory of the rectangle asked for,** which the GPU
  copies from where it is. What needs premultiplying is copied by a second buffer. The surface
  keeps a list of what it has handed out, as `freePixelBuffer()` gets a pointer.
- **A canvas without a device has pixels like any surface,** and lets go of them when they are on
  their way to a texture again and no PixelBuffer points into them.
- **Reading back makes a buffer for the rectangle and lets go of it.** A DX12Surface kept the one
  it read back by, so a canvas that had been read had its pixels three times: texture, pixel
  buffer and that.
- **`alpha()` of a canvas reads a map of a byte for each pixel,** made by reading the canvas back
  when first asked for and again after it has been rendered into or written.
- **DX12Backend stages what a batch of surfaces has to send in one buffer**
  (`_stageUploadBatch()`), up to 64 MB a buffer unless one surface has more. That is one buffer
  made where each surface had one for its pixels, and nothing rounded up to 64 KB. It is in an
  UPLOAD heap, as it is written once. A PixelBuffer of a canvas is in the CPU cached memory the
  pixels of a DX12Surface were in.
- **Nothing waits for an upload with D3D12,** which `allocPixelBuffer()` did when the GPU hadn't
  read the pixels yet. Nothing on the GPU reads our pixels now.
- **A lost device takes nothing of a surface that keeps its pixels.** A canvas is read back if the
  device works, otherwise cleared and marked with `isContentLost()`, also one that was created
  with pixels or a fill color and never rendered into, which had them back before. With the
  device removed and replaced ahead of gfxdevicetester's tests, that is what shows: the three
  canvases of SurfaceFillTests come out cleared.
- **VulkanBackend records the uploads of a surface in its own command buffer**
  (`_recordUploads()`) when it is about to draw from or into it, outside the render pass and
  after having other backends submit what they have recorded. VulkanSurface submitted every
  backend and waited for the GPU before and after each upload. Reading back records what is to
  be uploaded in the command buffer it reads with, so it waits once. A rectangle starting at an
  offset that isn't a multiple of four goes by a buffer of its own, as Vulkan doesn't promise
  such a copy for every queue.
- **MetalSurface commits its uploads as before,** and nothing waits for them either. `unload()`
  reads a canvas back, where what was rendered into it was lost.
- **A PixelBuffer is the pixels of the surface or memory of its own,** so only what is pulled is
  written and only what is pushed has been read. That was so with GlSurface already, and is
  what the documentation of Surface says, but the first version of the test below filled a
  buffer before pushing part of it, which wrote the surface itself with the software renderer.
  For `resources/surfaces.md` of the guide, with step 2.

Tested on Windows (Arc B580): gfxdevicetester gives the same results as before on all 262 tests
with DX12Backend and GlBackend. "Rectangles" of ReadbackTests is new: it writes and reads parts
of canvases through parts of pixel buffers. With DX12Backend it differs where "Readback" does,
at the canvas D3D12 has as XRGB_8 for RGB_565. With GlBackend it differs for the one in
KNOWN_ISSUES.md, as GlSurface can't read part of a surface it has no pixels of. In a Debug build,
63 tests of canvases, layers, readback, mipmaps, palettes, blurs and warps give the same with the
device removed and replaced before they run, but for the three canvases above.

Timed with DX12Backend (two runs each): the median of the 262 tests is 1.000 of what it was.
"Readback" takes 5.3 ms where it took 3.2: each of its six canvases is read back twice a run,
for its pixels and for `alpha()`, and each time a buffer is made. "FillColor" of
SurfaceFillTests takes 0.9 ms where it took 9 us: it reads three canvases that are never
rendered into, which had their pixels at hand. See SUGGESTIONS.md.

Not done: VulkanSurface and MetalSurface are not compiled. What the debug layer of D3D12 says
hasn't been looked at. How long it takes to load the surfaces of an application hasn't been
timed. A working device being let go of was run with the next section.

### GPU surfaces: pixels kept once (2026-11)

Decided with Tord 2026-11-24 and built the same day, in three steps: staging and plain memory
(the section above), the restorer with `buffered` gone, and GlSurface onto GpuSurface. It
replaces what "GPU surfaces on GpuSurface" above says of GlSurface standing by itself and of
what was left in, and what the section above says of `buffered` and of canvases created with
pixels.

**The rule: a surface keeps its pixels unless it is promised them back.** The promise is a
restorer in its Blueprint, which is called when the texture is lost. A canvas is the other case:
the GPU writes it, so pixels kept would be old ones, and whoever rendered into it does that again.

| Surface | Kept in plain memory | After a lost device | Read by the CPU | Written by the CPU |
|---|---|---|---|---|
| Without restorer | All pixels | Sent again | At once, in place | In place, then copied to staging |
| With restorer | An alpha map, if not all is opaque | Restorer called | Read back, waits for the GPU | Straight into staging |
| `Alpha_8`, palette based | All pixels, restorer or not | Sent again | At once, in place | In place, then copied to staging |
| Canvas | Nothing | Restorer called if it has one, else `isContentLost()` | Read back, waits for the GPU | Straight into staging |

At rest an `ARGB_8` surface takes 8 bytes a pixel without restorer, 5 with one (4 if all of it is
opaque) and 4 as a canvas, 5 once `alpha()` has been asked. So for whoever makes a surface: give
a restorer to what is only written, and none to what is read.

Surfaces without pixels of their own:

- **`allocPixelBuffer()` hands out staging memory** of the rectangle asked for, and `pullPixels()`
  has just that region copied to the texture. That is the fast path for a stream: the producer
  writes straight into what the GPU copies from.
- **The whole rectangle must be written if `pushPixels()` is skipped.** In staging memory the
  rest is whatever was there.
- **`pushPixels()` reads the rectangle back and waits for the GPU.** Colors are unpremultiplied
  and lose precision where alpha is low, so what is read regularly should keep its pixels.
- **The alpha map is a byte for each pixel,** written with the pixels and not made as long as
  all that has been written is opaque. A canvas reads its texture back for it.
- **`Alpha_8` and palette based surfaces keep their pixels** with a restorer too, which is never
  called. An alpha map would take as much as the pixels of the 8 bit ones. `Index_16` is with
  them (mine): its alpha comes by the palette, which can change after the pixels were written.
  A side effect is that the glyph cache gets through a lost device by itself.
- **What a surface is created with is written to pixels it has until they are sent,** not
  straight to staging memory (mine). That is a copy more, but DX12Backend stages what a batch of
  surfaces has to send in one buffer, where each would have had a buffer of at least 64 KB made
  for it. A surface that is never drawn from keeps them, as it has nothing in its texture.

The restorer:

- **A `std::function`** that takes the surface and returns a bool, `Surface::Restorer` (Tord,
  2026-11-25). It was a pointer to a function for a day, as `finalizer` is one, which Tord had
  picked over an object with a `restore()` to override. A plain function has the surface to go
  by, with its `identity` and its `baggage`. A lambda captures what it needs, which is what a
  plugin takes, see below, and what the observers of a surface already had. It is a member of
  Surface, so every surface is larger for it on all backends: 64 bytes for a `std::function`
  with MSVC as I recall, not measured, where the pointer was 8.
- **What a restorer captures has to be there as long as the surface,** which can be longer than
  whoever made it: a font or what is about to be rendered may hold the surface.
  `Surface::dropRestorer()` is for that. The surface declines from then on and stays one with a
  restorer, as what it keeps in memory was settled when it was made.
- **Called when the device has been replaced,** between frames, by `DX12Surface::migrateAll()`
  once every surface is on the new device, as a restorer may use other surfaces. The surfaces to
  call are held by strong pointers meanwhile, as a restorer may let go of surfaces. Not when the
  surface is first used: that spreads the work, but runs code of the application from inside
  rendering. An application that wants it spread declines and writes the pixels in its own time.
- **It returns true when the content is back, false when it did nothing.** After false the surface
  is cleared to transparent black and says `isContentLost()`, which is how a canvas without
  restorer ends up too. Clearing is sending zeros, from pixels made for it and let go of when sent.
- **`isContentLost()` goes back to false** when the restorer returns true or all of the surface
  has been written by one `pullPixels()`. A canvas without restorer says it from when the device
  is let go of, a surface with one when it has declined.
- **`Surface::declineRestore()`** is the one ready-made restorer, for streams.
- **A working device that is let go of is as a lost one** to a surface with restorer (Tord).
  Canvases without are read back, into pixels they have until they are on a device again.
- **A canvas with a restorer has it called** in place of being read back or left lost (mine, it
  wasn't talked through). So CanvasCapsule and the like could repaint from one instead of
  looking at `isContentLost()`, which none of them does yet.
- **Without a device, a surface without pixels gets them when first read or written,** zeroed, as
  its content is lost by then. That is no memory for those that are left alone meanwhile.

`buffered` is gone from the Blueprint, the C API (`wax_surfaceBP`, and `WAX_SAVE_BUFFERED_FLAG`,
whose value 8 is left unused), SurfaceWriter, the C# wrapper and the surface factory of the
plugin. The header of `.surf` files keeps the bit as reserved, so files written with it load.
`wax_surfaceBP` got `restorer` and `restorerData`: a `wax_restorer_func` takes the surface and
the data, and the C API wraps the two in a lambda, as it does for the observers of a surface.
`wax_getSurfaceBlueprint()` gives no restorer, as a `std::function` can't be handed back as a
function of C. New calls are `wax_declineSurfaceRestore()`, `wax_dropSurfaceRestorer()` and
`wax_surfaceIsContentLost()`. The layout of `wax_surfaceBP` changed, as it did for `fillColor`,
so plugins have to be built again.

Plugins (2026-11-25). A surface of a plugin is a PluginSurface in the plugin, which passes
every call on to a surface in the host. The host can't call the restorer of the plugin, which
takes the PluginSurface, so:

- **The host gets `PluginSurface::_restoreFromHost()` as restorer and the PluginSurface as its
  data,** which calls the restorer of the Blueprint with the PluginSurface. A table from surfaces
  of the host to PluginSurfaces was the other way, with the restorer still a pointer (Tord asked
  why not a capture).
- **PluginSurfaceFactory makes the PluginSurface first** and attaches it to the surface of the
  host when that is made, as the host needs the pointer with the Blueprint. It was made from
  the host's surface before. A host that fails to make its surface now gives null, where the
  factory went on with a null object.
- **The destructor of a PluginSurface drops the restorer** of the host's surface, which may
  outlive it.
- **`isContentLost()` is virtual** and PluginSurface asks the host, so CanvasCapsule, Glow and
  the charts of a plugin see that their canvases were cleared.
- **`surfaceIsContentLost` and `dropSurfaceRestorer` are new at the end of `wg_surface_calls`,**
  so a plugin built with them doesn't start in a host from before.
- **The restorer of a plugin runs on the thread the host replaces its device on,** between
  frames, as the host calls it. That is where pluginworkbench calls the plugin anyway. Whether
  it is for every host hasn't been looked into.

Tested on Windows (Arc B580, 2026-11-25): gfxdevicetester gives the same results as before on
all 266 tests with DX12Backend and GlBackend, and unittester passes. Plugins were run with
pluginworkbench on wappwin32dx12 in a Debug build, with surfaces added to the test plugin as a
temporary change and the device removed every three seconds:

- The restorer of a surface, a lambda with a capture, was called with the PluginSurface and
  filled it. The surface read back as before the loss and said no lost content.
- A surface that declines and a canvas without restorer said `isContentLost()` and read back
  cleared. A surface without restorer never said it. Its pixels weren't read after the loss.
- A surface with a restorer that was let go of at once never had it called.
- A CanvasCapsule in the plugin didn't crash. Whether it painted again wasn't looked at.
- The debug layer of D3D12 said nothing but that the device was removed.

Not run: a surface of the host that outlives its PluginSurface, which is what dropping the
restorer is for (the call is made, with the surface going away right after), the C# wrapper,
which nothing here builds, and VulkanSurface, MetalSurface and SdlGpuSurface, which take the
restorer from GpuSurface. To get pluginworkbench to run a plugin on Win32, `Win32API` got its
`openLibrary()`, `loadSymbol()` and `closeLibrary()`, which were left to do. It then aborted in
its first frame for another reason, see "A surface with no height gets past
`_isBlueprintValid()`" in KNOWN_ISSUES.md.

GlSurface on GpuSurface:

- **It sends what is written when it is written,** as before, since `glTexSubImage2D()` has
  copied when it returns. GpuSurface gives such a subclass (`m_bPullsAtOnce`) pixels to copy from
  where they are, so nothing goes by staging memory but what has to be premultiplied.
- **Its pixels are little endian** like those of the others, where they were in the byte order
  of the Blueprint. A PixelBuffer says which it is.
- **It keeps its pixels unless it has a restorer,** where it kept none unless `buffered`. That is
  more memory for an application that gives none.
- **Part of a surface is read through a framebuffer,** made when first needed. If GL won't make
  one of the texture, which it doesn't promise for every format, all of it is read and the part
  copied. That fixes "Rectangles" of ReadbackTests.
- **`alpha()` of a canvas reads an alpha map,** read back whole when the canvas has been
  rendered into, where it read one pixel for each call. See SUGGESTIONS.md.
- **A Blob is copied from,** where a `buffered` surface took it over, and its pitch no longer
  has to be a multiple of four.
- **`unload()`, `reload()`, `isLoaded()` and the pixel counters are gone,** from MetalSurface
  the first three.

Tested on Windows (Arc B580): gfxdevicetester gives the same results as before on all 263 tests
with DX12Backend and GlBackend, but for "Rectangles" on GlBackend, which is right now.
ReadbackTests and SurfaceFillTests have a restorer where they had `buffered`. New in
ReadbackTests: "Restorer" writes and reads rectangles of surfaces with a restorer that fills
them again, two of them sRGB, and "WriteAll" and "WriteAllRestorer" write all of a 256 x 256
surface each run. All three give what the software renderer gives on both.

In a Debug build with DX12Backend, with temporary changes that ran the tests, replaced the
device and ran them again: with the device removed, "Restorer" gives the same after, as do the
other tests of ReadbackTests, canvas layers, mipmaps and palettes. Of SurfaceFillTests, the
surface whose restorer declines and the three canvases come back cleared, the first with alpha
0 from `alpha()`, and the surfaces that keep their pixels as they were. With a device that
worked, the two canvases without restorer were read back and came through, and the two surfaces
with a restorer that declines were cleared.

Everything builds for wappwin32dx12 and wappsdl3gl, and unittester passes.

Timed on Windows (Arc B580) with `gfxdevicetester --performance`, each backend against a build
of the commit before all three steps (7842e2ee), two runs of each taken in turn. Run to run, nine
tests of ten are within 1% on DX12Backend, and within 4% before and 14% after on GlBackend, whose
second run after was the uneven one.

- **DX12Backend: the median of the 262 tests is 1.002 of before.** The three blur tests take
  95-100 us where they took 181-187, "A8_Fills" 4.6 us where 7.5 and the four tests of drawing
  into a canvas 1.3 us where 1.6. That was there with step 1 already and taken for noise between
  sessions then. Why hasn't been looked into: all of them draw into canvases that are drawn from
  in turn.
- **Reading canvases costs more with DX12Backend,** as step 1 found: "Readback" 5.2 ms where it
  took 3.2, and "FillColor" 1.15 ms where it took 9 us, as four of its surfaces are read back
  each run where all seven had their pixels at hand.
- **GlBackend: the median is 0.995 of before.** "Readback" takes 2.9 ms where it took 2.2, as
  `alpha()` of a canvas now reads all of it back. The two canvas layer tests and "FillColor" take
  14-23% less, the first most likely as a canvas no longer gets a framebuffer for reading when
  it is made. "GradientBlur" took 13% more in both runs, where the other two blur tests didn't,
  which I take for noise.
- **Writing all of a 256 x 256 surface: "WriteAllRestorer" takes 150 us with DX12Backend and
  80 us with GlBackend.** With D3D12 that is the time it takes to make a buffer, which is made
  for each PixelBuffer of a surface without pixels. "WriteAll", without restorer, shows 5 us with
  DX12Backend and 80 us with GlBackend, but the 5 us is not what a stream pays: the tester runs
  a test many times in one canvas update, and DX12Backend sends what was written once for all of
  them, in a buffer it makes outside what is timed. A surface written once a frame pays for a
  buffer either way, and for one copy more without restorer. So with D3D12 a restorer gains
  memory and a copy, not the buffer, and with OpenGL memory only. Timing one write a frame takes
  another kind of test.

On Linux and on a Mac, at d9508fe6 (2026-11-25, Debug), all of it compiled with no fix:

- **Linux** (Intel UHD 770, GL on llvmpipe): all targets with `linux-wappsdl3vk`,
  `linux-wappsdl3gpu` and `linux-wappsdl3gl`, unittester passes with each. gfxdevicetester:
  VulkanBackend 4 differ and SdlGpuBackend the same 4, GlBackend 3, of 266. What was there before
  is unchanged, and of the new ReadbackTests Restorer, WriteAll and WriteAllRestorer are OK on all
  three. No messages from the validation layer of Vulkan, with VulkanBackend or with SDL_GPU in
  debug mode.
- **Mac** (x86_64): all targets with `macos-wappcocoametal`, `macos-wappsdl3metal` and
  `macos-wappsdl3gpu`, but for two bundle steps, see KNOWN_ISSUES.md. That is the first time
  MetalSurface compiled on GpuSurface. unittester passes with each. gfxdevicetester: MetalBackend
  13 differ of 266 with both frameworks, where the 261 tests of bcf58e51 are unchanged and the
  13th is Rectangles of the new ReadbackTests, and SdlGpuBackend 10, with ReadbackTests sheared
  until uploads on Metal had tight rows, see "SdlGpuBackend" under Decided.

Not done: of applications only pluginworkbench has been run with it, besides gfxdevicetester and
unittester, and that run is the only one where the debug layer of D3D12 was listened to, see
Plugins above. Restorers through a replaced device have only run with DX12Backend.

### GPU backends: no shaders compiled at startup (2026-11)

GlBackend made all its 28 programs when created, and the first DX12Backend compiled all shaders,
which an application drawing with a few of them waited for: 130 ms on GlBackend with the driver's
cache of compiled shaders warm, 2.4 s on DX12Backend (Windows, Arc B580, Release). Small
applications should start fast and not feel heavy. gfxdevicetester gives the same results as
before on both.

- **GlBackend makes a program the first time something is drawn with it** (`_program()`), which
  takes 1.5-17 ms each. `create()` doesn't fail on shaders that don't compile anymore, as it
  compiles none: it checks for OpenGL 3.3, and a program that can't be made is reported once and
  draws nothing.
- **Program binaries are kept per program, as each is compiled** (`s_programBinaries`), where
  those of all programs were fetched at startup. A second GlBackend loads its programs from them.
  `shaderBlob()` puts the ones there are in a blob an application can save, and `setShaderBlob()`
  takes them back the next time it starts. Run with OpenGL 4.1, as binaries need: 23 programs
  loaded in 105 ms, against 408 ms compiling them with a cold driver cache. A blob cut short or
  with binaries the driver turns down ends in compiling. No app framework saves the blob.
- **DX12Backend has its shaders compiled, as bytecode that is checked in**
  (`dx12gfx/wax_dx12shaders.cpp`, made by `dx12gfx/compile_shaders.py` with FXC), so it compiles
  nothing and no longer needs d3dcompiler_47.dll. Compiling each when first needed was tried
  first: fills and blits took 30-35 ms each, but rounded boxes 150 ms, rings 490 ms and lines with
  caps 650 ms, each a pause the first time something of the kind was drawn. A small test run went
  from 3.4 s to 0.95 s. The 37 shaders are 552 KB, 1.7 MB as source, written as 32-bit words to
  keep that down, where the HLSL was 314 KB.
- **Decided with Tord: checked in rather than compiled by FXC when building,** as the shaders
  of the other backends are, so that building needs no shader compiler and DX12Backend builds
  without CMake. The cost is that FXC only runs on Windows: shaders changed elsewhere leave the
  bytecode out of date. `wax_dx12shaders.cpp` says which sources it was made from,
  `compile_shaders.py --check` compares that with the HLSL, and CMake warns when configuring.
- **Debug builds draw with the same optimized shaders** as release builds, where DX12Backend
  compiled them unoptimized with debug information. For debugging a shader, compile it by hand.
- **One list of shaders,** that of `vulkangfx/compile_shaders.py`, which the DX12 script uses as
  the Metal and GL ones do. `wax_dx12shaders.cpp.template` and the Ruby step are gone.
  `GpuBackend::_vertexShaderName()` and `_pixelShaderName()` say which shaders a kind of pipeline
  has, used by all four backends since.


### DX12Backend: small buffers out of shared blocks (2026-11)

Each DX12Edgemap, DX12Warpmap and palette of a DX12Surface had a buffer that was a committed
resource of its own, which D3D12 makes at least 64 KB and takes about 250 us to create. Decided
with Tord: they keep a buffer of their own, but get it from DX12BufferPool
(`dx12gfx/wax_dx12bufferpool.h`), as a range of a block they share. D3D12 binds these buffers by
address, so nothing changes in shaders or in how they are bound.

- **Blocks are upload heap buffers of 64 KB,** mapped for as long as they exist and made when
  needed, so a small application has one block for all of them, what a single edgemap took.
  64 KB is the least D3D12 makes a buffer, so nothing is lost to rounding and a block with one
  buffer left in it holds little. Blocks first doubled in size up to 1 MB, which made fewer of
  them, but then one buffer left could hold 1 MB. Decided with Tord: the pool is there to save
  memory before time.
- **Buffers that don't fit a block get a resource of their own,** rounded up to 64 KB as all of
  them were before the pool. At default precision the Warpmaps of all generators but BallWarp
  stay under 42 KB up to 1024 pixels and most take a few KB, a ball passes 64 KB between 256 and
  512 pixels, and an edgemap takes about width * (segments - 1) * 32 bytes, so one of 1000
  pixels with three segments is at the limit. 128 KB blocks would pack those better and cost a
  small application 64 KB more.
- **A block is released when its last buffer is given back,** unless it is the only empty one.
  One empty block is kept, so a buffer made and let go of over and over doesn't take making a
  block each time, and there is never more than one. An application that makes and lets go of
  more than a block of buffers for each frame makes a block for each 64 KB over that, each frame.
  Accepted: resources are meant to be made once and kept.
- **What is taken of a block is kept track of outside it,** by RangeAllocator
  (`wax/source/wax_rangeallocator.h`): a list of the free ranges in ordinary memory, first fit,
  joined when given back, with unit tests. MemHeap wasn't used, as it keeps its bookkeeping in the
  memory it hands out, and the blocks are write-combined memory, which is very slow to read.
  RangeAllocator knows nothing of D3D12, so VulkanBackend can use it for its memory.
- **The buffers are only ever written.** That is why upload heap memory is right for them, where
  it was wrong for the pixels of surfaces, which are read. Two places did read them and no longer
  do: GpuEdgemap copied the old buffer into the new one when an edgemap got its first tint, and
  now fills the new one in from the samples, and the palette was premultiplied where it lay
  (`p[0] *= p[3]`), in VulkanSurface too, and is now worked out before it is written.
- **A range given back waits for the GPU** before it is handed out again, as draws recorded or
  on their way may still read it. One fence counts the submissions of all DX12Backends
  (`DX12BufferPool::submitted()`), and a range waits for the last one that may read it, which for
  one given back while a backend records is known once nothing is being recorded. This is how
  VulkanContext does it, on the thread that draws, without locks. So DX12Backend no longer holds
  these buffers for each submission, and replacing the buffer of an edgemap at its first tint no
  longer waits for the GPU. The thread pool that releases what DX12Backend holds is still there
  for textures and the buffers of frames.
- **Ranges are whole numbers of 192 bytes,** a multiple of the float4 of edgemaps and palettes,
  of the 24 bytes of a Warpmap vertex and of a cache line, so every kind can start at any range.
  The debug layer of D3D12 has nothing to say about buffers bound that way.
- **A device let go of takes the blocks with it.** Edgemaps and surfaces give their buffers back
  first and get new ones on the next device, as before. A DX12Warpmap, which wasn't moved to a new
  device before and held on to the old one, finds its buffer no longer valid and gets a new one
  when next drawn (`_prepare()`).
- **Surfaces stay as they are.** A small DX12Surface takes at least 128 KB, see KNOWN_ISSUES.md.
  Decided with Tord not to pool them: graphics is meant to be baked into larger surfaces anyway.

On Windows (Arc B580, Release): gfxdevicetester has 134 such buffers of 2.6 MB together, now in
27 blocks and nine resources of their own, 2.9 MB, where they took 8.6 MB or more. (Doubling
blocks took 3.1 MB in six.) A whole accuracy run makes 32 blocks and releases all but the one
kept. "Cylinders made each run" of WarpTests went from 848 us to 15 us, and the other 71 tests
of the edgemap, warp and palette suites take 0.85 to 1.01 times as long as before the pool,
median 1.00, the same with blocks of 64 KB as with doubling ones. All of gfxdevicetester gives
the same results as before on DX12Backend and GlBackend. A debug build, with the debug layer of
D3D12 on, gives the same results for the fill, edgemap, warp and palette suites after two
simulated device losses as without them.

### DX12Backend: the thread pool that releases resources stays (2026-11)

DX12Backend holds the textures and frame buffers that recorded draws use, and hands them to a
thread pool wait at each submission, which releases them when the GPU has passed the fence
(`_holdUntilDone()`, `_releaseWhenDone()`). Replacing it with a release on the thread that draws,
as VulkanContext and DX12BufferPool do, was suggested for being simpler. Decided with Tord to keep
it.

- **It releases without another frame being drawn.** Before it came (e9bf1670, 2026-10, from work
  on a plugin framework) resources were released in `beginRender()`, when their frame slot came
  round again. An editor of a plugin draws only when something changes, so what it let go of
  would stay until whenever that was. The reason wasn't written down then, this is what the diff
  says.
- **The thread that draws never waits for the GPU** or does the releasing.
- **What it costs is for hosts of plugins too:** the callbacks name our module as their library,
  so a host can't unload it while one runs, and the last backend and a lost device wait at most
  two seconds for callbacks still out, rather than hang the host.
- **Not for many instances as such.** Instances of one plugin share the statics of DX12Backend,
  and each plugin has a copy of its own. All DX12Backends of a module are taken to draw on one
  thread: `s_backends` and DX12BufferPool have no locks.


### SdlGpuBackend (2026-11)

A backend on the GPU API of SDL 3 (`wax/extensions/sdlgpugfx`), so that an application on SDL3
can use Wax without picking a backend for each platform: SDL_GPU draws with Vulkan, D3D12 or
Metal as it finds them. WonderApp has it as `wappsdl3gpu`, which renders each window into a canvas
of its own and copies that to the swap chain, as `wappsdl3gl` does. Built on GpuBackend,
GpuSurface and GpuEdgemap like the other GPU backends, with SdlGpuSurface, SdlGpuEdgemap,
SdlGpuWarpmap and their factories. All share one device, set by `SdlGpuBackend::setDevice()`.

- **A session is recorded and played back when it ends,** into a command buffer of its own,
  which is submitted then, as GlBackend plays back its sessions. SDL_GPU has no buffers the CPU
  writes to where the GPU reads them, and copies only in copy passes, outside render passes, so
  what a session writes and what has changed of the surfaces, edgemaps and Warpmaps it draws is
  uploaded in a copy pass in front of its render passes. That is when the Wax design says the
  objects are read. A command buffer per session also puts every session in front of what comes
  after it, so reading back needs no flushing of other backends. It costs about 0.1 ms per session
  more than VulkanBackend on Intel UHD 770, which otherwise is as fast. The render pass of a
  session on the default canvas can go into a command buffer of the application's instead, see
  "Drawing into a frame of the application's".
- **The same HLSL,** compiled to SPIR-V, DXIL and MSL by `sdlgpugfx/compile_shaders.py` and
  checked in, as for the other backends. Each platform has the formats SDL_GPU can have a device
  for there: SPIR-V everywhere but on Apple's, which have MSL, and DXIL too on Windows. SDL_GPU
  binds by slots of its own for each stage, kind and shader, so the script moves the registers to
  them and writes what each shader binds to `wax_sdlgpushaders.cpp`. The constants are the three
  cbuffers of DX12Backend as uniform buffers, as SDL_GPU has no push constants.
- **No dual-source blending:** Replace and Morph draw twice, the first taking away the part of
  the canvas the second color says, the second adding the color, see `hlsl/sdlgpu_dual.hlsl`.
  Multiply puts the two colors together in the shader and draws once: BlendModeMultiply of
  gfxdevicetester takes 18 us on Intel UHD 770, against 34 us on VulkanBackend. Two draws round twice and differ from one where triangles
  of a draw overlap, see KNOWN_ISSUES.md.
- **No swizzles, border colors or sampled integer textures,** so the shaders do what they are
  for on the other backends, see `hlsl/sdlgpu.hlsl`: Alpha_8 is R8 with the value moved to
  alpha, XRGB_8 reads alpha 1, clipping multiplies a texel clamped to the edge by the part of its
  footprint inside the source, which is what a transparent border gives, and palette indexes are
  in R8_UNORM or R16_UNORM, turned back into indexes by the largest index, which SdlGpuSurface
  puts in the header of the palette. The A8 of SDL_GPU wasn't an option: it is R8 with a swizzle
  on Vulkan, which a render target must not have, and Metal can't render into it.
- **Edgemaps and Warpmaps are uploaded whole when they have changed,** cycling the buffer, so
  sessions still in flight keep what they had, unlike on the other GPU backends.
- **Mipmaps** are made by SDL_GenerateMipmapsForGPUTexture(), as VulkanBackend blits them.
- **On Metal, uploads to textures come from rows as long as the region written.** The Metal
  driver of SDL_GPU (3.4.18) ignores `pixels_per_row` of SDL_UploadToGPUTexture() and takes the
  length of a row from the width of the region, which sheared every upload of a rectangle from a
  wider pixel buffer. So with that driver `SdlGpuContext::tightUploadRows()` is true, staging
  memory has no padding at the end of a row, and `SdlGpuSurface::_pull()` copies a rectangle
  narrower than its buffer to a buffer of its own, as it already does for offsets that aren't a
  multiple of four. Downloads are not affected. Other drivers are left as they were, so the copy
  is not made for them. If SDL fixes it, the flag can go.

gfxdevicetester on Linux, Release, with Vulkan: on Intel UHD 770 (Mesa) 266 tests, 4 differ, the
same ones and by the same amounts as on VulkanBackend: Mipmapped_0.4, ReplaceVerticalAndFlat,
Readback and Rectangles of ReadbackTests, see KNOWN_ISSUES.md. The same four on lavapipe. In debug
mode of SDL_GPU, with the validation layer of Vulkan, no messages.

With D3D12 and Metal, 2026-11-25:

- **D3D12** (Windows, Arc B580, Release, e5d6c4ae): 266 tests, 10 differ, none fail. The ten are
  the four above and the six of nearest sampling that DX12Backend and MetalBackend have, by the
  same amounts, see KNOWN_ISSUES.md. In debug mode the debug layer of D3D12 says nothing while
  drawing, only that objects are alive when the process ends. Before "GPU vertices without u and
  v" below it couldn't make the pipelines of shapes, which the Cybersim gen2 application found
  as the first to draw with it on D3D12, and worked around to draw its pages within 4 of 255 of
  SoftBackend.
- **Metal** (a Mac, x86_64, Debug, d9508fe6 and e5d6c4ae): 266 tests, 10 differ, none fail. The
  same ten by name. Readback and Rectangles differed by far more than anywhere else (max 170 and
  251), as uploads of rectangles were sheared, until the rows of uploads were made tight on Metal
  (see above). Since then they differ by max 28 as everywhere else, and no other line changed.
- **Vulkan on Windows** (the same Arc B580, e5d6c4ae): 13 differ, nine of them tests of blending
  that are OK with D3D12 on the same build and with Vulkan on Linux, see KNOWN_ISSUES.md.

### GPU vertices without u and v (2026-11-25)

A vertex of GpuBackend is a position, a color offset and an extras offset, 16 bytes. It had u and
v too, 24 bytes, which only edgemaps read. Decided with Tord 2026-11-25 ("Yes, remove it all"),
after SdlGpuBackend couldn't make the pipelines of shapes on D3D12: `vs_shape.hlsl` declared the
u and v it didn't read, D3D12 takes no pipeline whose layout lacks an input the vertex shader
declares, and shapes are given three inputs. DX12Backend offered all four to every shader, and for
the others the unused input was gone from what DXC and SPIRV-Cross made, so only DXIL showed it.

- **The column and the distance down it are worked out by `vs_segments.hlsl`,** from the
  position, as `vs_blit.hlsl` gets the place in its source: they are the offset from the top left
  corner run through the flip, the same for every patch of a draw. An edgemap draw has three
  extras entries for it where it had two: the two of a blit, and the offsets of its colors with
  the tile size, which had an entry each with half of it unused.
- **What it gives:** every quad is 96 bytes where it was 144, to write and to upload, at 16 bytes
  more for each edgemap drawn. `_addQuad()` takes no u and v, `_nbVertexAttributes()` is 2 for
  plain fills and 3 for the rest, and no vertex shader declares an input another kind lacks.
- **The values are the same:** whole pixels and a half, which a float holds exactly on either
  side. gfxdevicetester gives the same lines as before, the worst pixel aside, with DX12Backend
  and GlBackend on Windows (Arc B580, Release) and with VulkanBackend, SdlGpuBackend and GlBackend
  on Linux (Intel UHD 770 and llvmpipe, Debug), the validation layer of Vulkan silent. On a Mac
  (x86_64, Debug, e5d6c4ae) MetalBackend and SdlGpuBackend with Metal give the same lines as at
  d9508fe6.
- **Compiled in two places:** the shaders of DX12Backend on Windows, which has FXC, and the rest
  on Linux, which has DXC with SPIR-V and SPIRV-Cross (e5d6c4ae). Only segmentsVS changed, and
  shapeVS where the unused input had stayed: in the bytecode of FXC and in the DXIL.
- **Rule for the shaders,** in `hlsl/README.md`: a vertex shader declares only the inputs it
  reads.

### Drawing into a frame of the application's (2026-11-26)

An application with graphics of its own, such as a 3D view, wants its widgets drawn straight onto
its frame in the window, not into a texture that is then copied or blended there. Cybersim, which
renders with SDL_GPU, had to keep a frame texture and copy it to the window every frame, as
SdlGpuBackend couldn't draw into a texture of a swap chain. Decided with Tord 2026-11-26. The
application renders first and then has its root panel rendered with everything dirty, as nothing
of a frame is kept. What each backend has for it:

- **SdlGpuBackend: `setCommandBuffer()`.** A texture of a swap chain is only valid for the
  command buffer it was acquired with, and is presented when that one is submitted, so our draws
  have to go into the command buffer of the application. It is set before a render and forgotten
  by `endRender()`, as `autopresent()` of MetalBackend and the semaphores of VulkanBackend are.
- **Only the render pass of a session on the default canvas goes there** ("uploads kept apart",
  Tord). Its uploads, and everything of sessions on surfaces, are still in command buffers of
  ours, submitted when the session ends. So all that surfaces hold is on its way to the GPU when
  a session has ended, and pixels can be read back at any time. With everything in the
  application's command buffer, a read-back before it was submitted would have got pixels from
  before the frame, and could have put an older upload on top of a newer one. It costs a command
  buffer of ours per frame, about 0.1 ms on Intel UHD 770. What is left for the application to
  know: what it changes of a surface after the render and before it submits is what the window
  gets, a frame early, and we push uniform data to slots 0-3 of its command buffer.
- **Mip levels are made before the render pass,** for all that a session blits from, in every
  session. The pass was ended and begun again for each before. A session has one canvas and
  reads what it draws from when it ends, so nothing changes in between.
- **MetalBackend: `setMetalDevice()` takes the command queue** of the application. Command
  buffers only run in the order they were committed when they are on the same queue, and we had
  one of our own.
- **GlBackend: the default canvas is the framebuffer bound for drawing at `beginRender()`,** not
  always that of the window, for an application that renders its frame into a framebuffer of its
  own.
- **DX12Backend: `setDefaultCanvas()` takes the resting state** of the buffer, PRESENT unless
  given, as VulkanBackend takes a resting layout. With RENDER_TARGET an application that renders
  into the buffer first saves the two transitions.
- **VulkanBackend needed nothing:** it takes an image of the swap chain as default canvas, and
  has `commandBuffer()` and the semaphores.
- **`wappsdl3gpu` with `WAPP_GPU_DIRECT=1`** draws straight into the swap chain this way, with a
  clear where an application has its graphics, to have it run and tested.

On Windows (Arc B580, Release): gfxdevicetester gives the same lines as before with DX12Backend
(13 differ of 266), GlBackend (2) and SdlGpuBackend with D3D12 (10), and unittester passes with
SdlGpuBackend. With Vulkan the same 13 differ, by amounts that vary from run to run, see
KNOWN_ISSUES.md. With `WAPP_GPU_DIRECT=1` widgetbench, editera and helloworld show the same
window as without it, pixel for pixel as PrintWindow gives it, with D3D12 and with Vulkan, and
the debug layer of D3D12 says nothing while drawing. What the application has rendered stays
where the widgets don't draw: with the clear made magenta and only the left half of the window
redrawn each frame, the right half was magenta, with both drivers. GlBackend with a framebuffer
of the application's as default canvas showed the same window as before in widgetbench and
helloworld. The last two were tried with changes to `wappsdl3gpu` and `wappsdl3gl` that weren't
kept.

On Linux (7a50178a, Debug): all of `linux-wappsdl3gl`, `linux-wappsdl3vk` and
`linux-wappsdl3gpu` builds with no fix, unittester passes with each, and gfxdevicetester gives
the same lines as before: VulkanBackend and SdlGpuBackend 4 that differ of 266 (Intel UHD 770),
GlBackend 3 (llvmpipe). With `WAPP_GPU_DIRECT=1` widgetbench shows the same window as without
it, pixel for pixel, on lavapipe under Xvfb, which is the device there that can present, with
the validation layer of Vulkan silent. With GlBackend it shows the same window as at b3de9d77.

On a Mac (7a50178a, x86_64, Debug): `macos-wappcocoametal`, `macos-wappsdl3metal` and
`macos-wappsdl3gpu` build with no fix, but for the two bundle steps in KNOWN_ISSUES.md, and so
do gfxdevicetester and unittester of `macos-wappsdl3gl`. unittester passes with each.
gfxdevicetester gives the same lines as before: MetalBackend 13 that differ of 266 with both
frameworks, SdlGpuBackend with Metal 10. GlBackend has 16, at b3de9d77 too, the same lines but
for three tests that vary from run to run there, see KNOWN_ISSUES.md. With `WAPP_GPU_DIRECT=1`
widgetbench and editera show the same window as without it, pixel for pixel, also after the
window was resized, with the validation of Metal turned on and saying nothing. With a queue of
the application's given to `setMetalDevice()`, in a change to `cocoawindow_metal.mm` that wasn't
kept, `metalCommandQueue()` returned it, gfxdevicetester gave the same lines and the two
applications the same windows.

workbench was compared too, on Windows and Linux, which says nothing: it isn't built on WonderApp
and draws with SoftBackend.

Cybersim, the first application with graphics of its own (859d64aa, Windows, D3D12, Arc B580):
each frame has one command buffer, with the 3D view in a render pass with four samples resolved
into the texture of the swap chain (B8G8R8A8_UNORM_SRGB), then `setDefaultCanvas()`,
`setCommandBuffer()` and the root panel rendered with everything dirty, and the 3D shows where no
widget draws. Its frame texture and the copy to the window are gone. 13 pages and views drawn the
same way into a texture of its own are pixel for pixel what they were before. A second window,
of the debugger, with an SdlGpuBackend of its own on the same device, is drawn between the
acquire and the submit of the first, with fonts and skins on surfaces from the first backend's
factory. The debug mode of SDL_GPU said nothing. Not tried there: Vulkan, and a window that
changes size between the event and the acquire.

Not run: DX12Backend with RENDER_TARGET as resting state, direct mode on a display of an Intel
GPU on Linux, and an application that commits command buffers of its own to the queue it gives
MetalBackend. So of the backends, only SdlGpuBackend has drawn on top of an application's own
graphics.

### How WonderApp's windows get to the screen (2026-11-26)

All thirteen frameworks were read for steps they don't need between Wax and the screen. A window
of WonderApp is redrawn where it has changed only, which takes something to draw into that
keeps what it had. What a swap chain gives back decides how each framework does it:

- **wappwin32dx12 and wappsdl3dx12** draw the dirty rects straight into the back buffer and give
  them to `Present1()`: a flip-sequential swap chain keeps its buffers, and DXGI brings them up
  to date.
- **wappsdl3sw, wappsdl2sw and wappwin32sw** draw into the pixels of the window itself, and
  **wappsdl2gl** into the front buffer of a window with one buffer.
- **wappsdl3vk, the three with Metal, wappsdl3gl and wappsdl3gpu** draw into a canvas of their
  own and copy it to the window. **wappcocoasw** does the same in plain memory, copying what
  changed.

Decided with Tord 2026-11-26, for the canvases:

- **Vulkan keeps its canvas, and copies only what has changed.** An image of a swap chain keeps
  its content while it is presented, by the specification of `vkQueuePresentKHR`, and we are
  told which image we get. Drawing straight into the images would have saved the canvas, but
  each change would have been drawn once for every image ("so we don't have to redraw same
  widgets three times", Tord). So each image has a list of what has changed of the canvas since
  it was last copied to, and that is copied. An image that has never been copied to, or whose
  list has grown past 64 rects, gets all of it, taken from UNDEFINED, the others from
  PRESENT_SRC.
- **wappsdl3metal and wappsdl2metal are not changed.** They copy all of the canvas to a drawable
  for every change, with a Wax session that blits it with the blend mode a session starts with,
  onto a drawable whose content isn't loaded, so it also reads pixels that aren't there where
  the canvas isn't opaque. Tord wanted a plain copy in its place, with a blit encoder. That is
  written and held back: it needs `framebufferOnly` turned off for the layer, which
  CAMetalLayer.h recommends against for most applications and Apple's page says is "at a cost
  to performance". It would still copy everything, as nothing from Apple says that the texture
  of a drawable still has what was presented in it when it comes back.
- **Only documented ways** (Tord): nothing is built on how it happens to work on the machines we
  test on.
- **wappcocoametal draws into buffers of its own,** so that one framework on a Mac sends no more
  than what has changed (Tord: "Yes, build it"). The view has a plain CALayer, and no
  CAMetalLayer. Its contents is one of up to three IOSurfaces, each with a Metal texture on it
  and a list of what has changed since it was last drawn into. A frame takes a buffer that
  nobody reads, copies what it has missed from the one being shown, has Wax draw the dirty rects
  on top as the default canvas, waits for the GPU and gives the buffer to the layer. There is no
  canvas besides the buffers, a change is drawn once, and no more than the change is copied.
  What it stands on is in the headers of the macOS SDK (15.4, read on a Mac): CALayer.h names an
  IOSurfaceRef as what the contents of a layer typically is, MTLDevice makes a texture on one,
  and IOSurfaceRef.h has the use count and `IOSurfaceIsInUse()` for knowing when a surface in a
  pool can be taken again.
- **Two things about it are not in the headers,** and Tord took it with them. That Core Animation
  counts a surface as in use while a layer shows it: the header says CoreVideo "and other APIs"
  do. Were that wrong, a part being redrawn could show half done. And when what Metal has
  rendered can be seen by whoever shows the surface: a completed command buffer is the one point
  where the GPU is known to be done, so the window waits for it, which holds the main thread for
  as long as the GPU takes with the frame. Nothing holds it back to the rate of the display
  either, as `nextDrawable` does.
- **wappsdl3gl has a canvas without alpha,** XRGB_8, which is GL_SRGB8. Copied to the window it
  leaves alpha at 1, so the pass over the whole window that made it opaque after each copy is
  gone. Asking for a window without alpha was tried first and isn't the way: the Arc B580 gave a
  window with 10 bits for each color and 2 of alpha for it.
- **wappsdl3gpu is as it was.** SDL_GPU doesn't tell the textures of a swap chain apart either,
  so it is a canvas and a copy, or all widgets drawn for every change, which
  `WAPP_GPU_DIRECT=1` does and an application with few widgets on graphics of its own wants.

With wappsdl3gl on Windows (Arc B580, Release) widgetbench, helloworld and editera show the same
windows as before, pixel for pixel, gfxdevicetester gives the same lines and unittester passes.

Not compiled: the changes for Vulkan and for wappcocoametal, which Windows has nothing to build
with.

Also seen, and not changed: the main loops wait a fixed time each turn, 10 ms with SDL3, which
keeps a window under 100 frames a second, 1 ms with Win32, and up to 10 ms for an event with
Cocoa, so all three wake many times a second with nothing to do.
