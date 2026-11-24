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

### GPU surfaces: pixels kept once

Decided with Tord 2026-11-24, not started. DX12Surface, VulkanSurface and MetalSurface keep the
pixels of every surface besides the texture, and GlSurface only when asked to, see "GPU surfaces
keep their pixels twice" in KNOWN_ISSUES.md. The second set is there for the CPU to read and write
and to get the texture back after a lost device. Most surfaces are never read or written once
made, and many can get their pixels back in a cheaper way.

**The rule: a surface keeps its pixels unless it is promised them back.** The promise is a
restorer in its Blueprint, which is called when the texture is lost. A canvas is the other case:
the GPU writes it, so pixels kept would be old ones, and whoever rendered into it does that again.

| Surface | Kept in plain memory | After a lost device | Read by the CPU | Written by the CPU |
|---|---|---|---|---|
| Without restorer | All pixels | Sent again | At once, in place | In place, then copied to staging |
| With restorer | An alpha map, if it has alpha | Restorer called | Read back, waits for the GPU | Straight into staging |
| `Alpha_8`, `Index_8` | All pixels, restorer or not | Sent again | At once, in place | In place, then copied to staging |
| Canvas | Nothing | `isContentLost()` | Read back, waits for the GPU | Straight into staging |

At rest an `ARGB_8` surface takes 8 bytes a pixel without restorer, as on three of the backends
today, 5 with one (4 if all of it is opaque) and 4 as a canvas, 5 once `alpha()` has been asked.

So for whoever makes a surface: give a restorer to what is only written, and none to what is read.

#### Pixels and staging

- **Pixels that are kept are in plain memory,** on all backends. They survive a lost device as
  they are, so the copying out of and back into device memory of `DX12Surface::_evacuate()` and
  `_migrate()` goes. The price is one copy more for each upload of a surface without alpha: the
  GPU copies straight from the pixels today, as they are in a buffer of the device.
- **The GPU copies from staging memory,** which each backend makes when asked for and releases
  when it has been given back and the GPU has passed the copy, told by the fence DX12BufferPool
  and VulkanContext have. GlSurface needs none of that, as `glTexSubImage2D()` has copied when it
  returns.
- **No staging memory is kept to be used again** (Tord, who first chose keeping it until a frame
  had passed without use). A stream makes a buffer for each frame, about 160 us with D3D12. Small
  ones can share blocks that are released when empty, as the buffers of DX12BufferPool do, so
  that a glyph doesn't take the 64 KB that D3D12 makes a buffer at least.
- **Several surfaces are staged and sent in one go,** as DX12Backend does it today and what Tord
  asked for to stay: surfaces with pixels to send wait on a list, and their copies are recorded
  together in front of the session that first needs one, or earlier by `submitUploads()`. The
  staging memory of each lives until the GPU has passed that.
- **Staging memory is CPU cached,** the kind the pixels of a DX12Surface are in today, not
  write-combined. A PixelBuffer may be read, by whoever has it and by us for premultiplying and
  the alpha map, and reading write-combined memory took 60 ns a pixel, see "DX12Backend:
  surfaces" below. Nothing is measured to be gained from writing it.
- **Uploads never wait for the GPU.** VulkanSurface does today, see "VulkanBackend stalls when
  pixels of a surface are uploaded" in KNOWN_ISSUES.md, which has to go with this. When the copy
  is recorded stays with each backend, as now.
- **A surface that keeps its pixels collects what is to be sent as today** and has it copied to
  staging, premultiplied if need be, when the texture is next used.
- **A surface made while there is no device** keeps what it was given in plain memory until there
  is one, as today, also with a restorer.

#### Surfaces without pixels of their own

- **`allocPixelBuffer()` hands out staging memory** of the rectangle asked for, and `pullPixels()`
  has just that region copied to the texture. A pull is a copy of its own, where a surface with
  pixels sends one rectangle around all that has changed.
- **That is the fast path for streaming:** the producer writes straight into what the GPU copies
  from, also from another thread, as it is memory like any other between the two calls. Without
  alpha that is one pass over the pixels on the CPU, the producer's own. With alpha it is two: the
  pixels are premultiplied into a second staging buffer, in the pass that writes the alpha map. A
  surface that keeps its pixels takes two as well: into the pixels, and from there to staging.
- **The whole rectangle must be written if `pushPixels()` is skipped.** That is what the
  documentation says, but with the PixelBuffer pointing into the pixels of the surface, writing
  part of it works today. In staging memory the rest is whatever was there.
- **`pushPixels()` reads the rectangle back and waits for the GPU,** which takes about as long for
  a few pixels as for all. Alpha is exact, from the alpha map. Colors are unpremultiplied and lose
  precision where alpha is low, so reading, changing and writing back can shift such pixels. What
  is read regularly should keep its pixels. `Surface::copy()`, `convert()` and SurfaceWriter read
  this way too.
- **The alpha map is a byte for each pixel,** as GlSurface has it, since skins have a mark
  threshold of their own. It is written with the pixels, when the surface is made and by each
  pull, and not made as long as all that has been written is opaque. `Alpha_8` and `Index_8` keep
  their pixels instead, which take as much, so their restorer is never called. A side effect is
  that the glyph cache gets through a lost device by itself.
- **A canvas gets its alpha map by reading back** at the first `alpha()`, and again at the next
  one after it has been rendered into. That is what the pixel buffer of a canvas does today, at a
  quarter of the memory.
- **Mipmaps are made anew for the whole surface** after a write, as today, so what is written
  every frame shouldn't have them.

#### The restorer

- **A pointer to a function** that takes the surface and returns a bool, as `finalizer` is one
  (Tord, over an object with a `restore()` to override). What it has to go by is the surface, with
  its `identity` and its `baggage`, so a loader is found through one of them or is global. The C
  API passes the pointer on as it is.
- **Called when the device has been replaced,** between frames, from where `DX12Backend::setDevice()`
  puts surfaces back on a device today. Not when the surface is first used: that spreads the work,
  but runs code of the application from inside rendering. An application that wants it spread
  declines and writes the pixels in its own time.
- **It gets the surface and fills it** with what Surface has for writing. No buffer is made for it
  beforehand, so declining costs nothing.
- **It returns true when the content is back, false when it did nothing.** After false the surface
  is transparent black and says `isContentLost()`, which is how a canvas without restorer ends up
  too. A decoder with threads of its own declines and writes when it is done, and a stream
  declines as its next frame replaces everything.
- **It gives back what the surface held, not what it was made with.** One that loads a file again
  takes a surface that was changed since back to what the file has.
- **One ready-made restorer,** which declines, for streams.
- **`loadSurface()` of WonderApp gives no restorer,** so what it loads keeps its pixels (Tord,
  who plans to change its API). Until then, what an application loads through it takes twice its
  pixels on all backends, which on GlBackend is more than today, once GlSurface follows the rule.
- **`isContentLost()` goes back to false** when the restorer returns true or the whole surface has
  been written. A canvas is replaced by its owner, as CanvasCapsule, Glow, PlotChart and
  ScrollChart do.
- **A working device that is let go of is as a lost one** to a surface with restorer: the
  restorer is called on the next device (Tord). Every surface is read back before the device goes
  today, which for these would be all textures at once, with colors of low alpha losing precision
  on the way. Canvases are still read back, into plain memory they keep until they are on a
  device again, as nobody else has what was rendered into them.
- **Only DX12Backend replaces a device today,** so that is where restorers are called. On the
  others a restorer says the same about what to keep, and is called once they can replace theirs.

#### `buffered` goes

Without restorer the pixels are kept, which is what `buffered` asked for. With one, `buffered`
would say to not use it, as leaving it out does. For a canvas it only decides whether the pixel
buffer is made with the canvas or when first read. It leaves the Blueprint, the C API, the C#
wrapper and the surface factory of the plugin. The header of `.surf` files keeps the field as
reserved, so files written with it load.

#### Steps

1. **Staging and plain memory,** for DX12Surface, VulkanSurface and MetalSurface, without change
   to the API: staging memory, kept pixels in plain memory, uploads that don't wait, and
   canvases without pixels of their own, which is all of a surface with restorer but the restorer.
   On gl-shape-performance.
2. **The restorer in the Blueprint and `buffered` out of it,** `isContentLost()` going back to
   false, the restorer that declines, the C API, the C# wrapper and `resources/surfaces.md` of the
   guide. On breaking-changes, merged into v4, which needs breaking-changes to build and work
   first.
3. **GlSurface onto GpuSurface,** after step 2, as it would keep pixels it doesn't today in
   between. Its staging is plain memory let go of when sent, and it reads a rectangle back through
   a framebuffer. `unload()`, `reload()` and `isLoaded()` of GlSurface and MetalSurface and the
   pixel counters of GlSurface go here.

To test with: gfxdevicetester giving the same results on all four, its ReadbackTests and
SurfaceFillTests with and without restorer where they have `buffered` today, new tests writing
and reading rectangles of surfaces with restorer, one timing a surface written in full each run,
and a device removed and replaced with restorers that restore and that decline.

#### Not part of this

- **Alpha without the second pass:** a flag for pixels the producer has premultiplied, or a
  texture with straight alpha that the shader multiplies, which is right at 1:1 and with nearest
  sampling but lets the color of transparent texels in at edges when scaled. Either can be added
  later.
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
  see "GPU surfaces: pixels kept once" under Planned.
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
