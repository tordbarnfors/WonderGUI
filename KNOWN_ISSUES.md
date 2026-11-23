# Known issues

Bugs and limitations we know about but haven't fixed yet. Add an entry when you
find one, and remove it in the same commit that fixes it.

Each entry says what happens, where in the code, and anything known about the cause.

## Starting up

Found when Cybersim set WonderGUI up on its own, without WonderApp. None of them gives an
error message.

### A `HiColor` made from a `Color8` before `Base::init()` is transparent black

`HiColor::HiColor(Color8)` looks the channels up in `unpackLinearTab`, which `_initTables()`
fills in when `Base::init()` runs. Before that the table is all zeros, so a color converted
earlier, such as a constant at namespace scope (`const HiColor paper = Color8(0xF3F3F3);`),
silently becomes transparent black and stays that way. Skins made with it draw nothing.

`wax/source/wax_color.cpp`

### Without a `HostBridge`, the first change of pointer style crashes

`InputHandler` calls `Base::hostBridge()->setPointerStyle()` without checking for null, while
`lockHidePointer()`, the clipboard calls in `Base` and the focus calls in `RootPanel` do check.
With `Base::init(nullptr)`, as `example01` does, it crashes as soon as the pointer comes over a
widget with another pointer style, such as a `LineEditor`.

`wondergui/source/base/wg_inputhandler.cpp`

### Without a default `SurfaceFactory`, drawing text with a `FreeTypeFont` crashes

A `FreeTypeFont` that isn't given a cache uses `Base::defaultBitmapCache()`, and
`BitmapCache::_addCacheSurface()` calls `createSurface()` on `Base::defaultSurfaceFactory()`
without checking for null. A host that creates a `RootPanel` with its own device and never
calls `Base::setDefaultSurfaceFactory()` works until the first text is drawn.
`Font::getGlyphAsSurface()` does check. Seen as a crash at the first render with text; the null
factory is from reading the code, since the fix that was tried set the default device and all
three factories at once.

`wax/source/font/wax_bitmapcache.cpp`

## Text system

### Markdown: code fence lines show as empty lines

In `MarkdownTextLayout`'s rendered mode the ```` ``` ```` lines of a fenced code block are
hidden, but their newline isn't, so each fence leaves an empty line above and below the code.
Hidden runs may not span a newline (see `FlowTextLayout::Run`), so fixing this needs a way to
hide a whole line.

`wondergui/source/textlayouts/wg_markdowntextlayout.cpp`

### Markdown: code background only covers the text

The `codeBlock` style's back color is drawn behind the characters only, not across the
width of the block. `FlowTextLayout` has no block backgrounds yet.

`wondergui/source/textlayouts/wg_flowtextlayout.cpp` (`_renderBack()`)

### Markdown: tables, images and raw HTML are not supported

Tables display as plain text with their pipes, images display their alt text, and raw
HTML is displayed as text (`MD_FLAG_NOHTML`).

### Text layout works on codepoints, not graphemes

There is no shaping, grapheme clustering or bidirectional text. Combining marks and emoji
sequences (ZWJ, skin tones, flags) are laid out and stepped over one codepoint at a time,
and right-to-left text is displayed left-to-right. All positions are byte offsets and the
layout decides where the caret may go, so this can be added without changing the API.

### `FreeTypeFont` glyph cache misses non-integer pixel sizes

`_findGlyph()` picks the size slot with `(size+32)/64` while `_addGlyph()` uses `size/64`.
For sizes that aren't a whole number of pixels, like 13 pt at 150% scale (19.5 px), a glyph
is stored in one slot and looked for in another, so it is never found and gets added again
each time, hitting the `assert` in `_addGlyph()` in debug builds.

`wax/extensions/freetypefont/wax_freetypefont.h` and `.cpp`

### `StaticText::_setSize()` ignores a scale change if the size is the same

It returns early when `size == m_size`, even if the scale changed, so the `TextLayout`
isn't told about the new scale.

`wondergui/source/components/wg_statictext.cpp`

## Widgets

### The tab key doesn't move keyboard focus, and `tabLock` has no effect

Nothing moves focus from one widget to the next. `Widget` has `tabLock` in its Blueprint with
`setTabLock()` and `isTabLocked()`, described as preventing focus from shifting away with tab,
but only the debugger reads it. `TextEditor` handles the tab key itself through its
`tabKeyAction`, `LineEditor` types nothing for it. A form with several fields has to move focus
itself with `grabFocus()`.

`wondergui/source/widgets/wg_widget.h`

### `ScrollCapsule::viewOffset()` returns the negated offset

`viewOffset()` returns `childCanvas.pos - viewRegion.pos`, the opposite sign of what
`setViewOffset()` takes, so `setViewOffset(viewOffset())` jumps instead of staying.

`wondergui/source/widgets/capsules/wg_scrollcapsule.h`

### `TablePanel` reports another minimum when asked about a scale it isn't at yet

`_minSize()` adds up row and column minimums at its own scale, but the minimums of the cells at
any other scale. When a scale change reaches a `PackPanel`, it lays out its children before they
have the new scale, and nothing corrects the result until the next resize. A table with 276
padded rows reports 1112 pts instead of 8. `_defaultSize()` has a separate path for other scales
too and should be checked for the same kind of mismatch.

`wondergui/source/widgets/panels/wg_tablepanel.cpp`

## Layout

### `PackLayout`: `Factor::Ordered` expands far too much

The expand step uses `std::max(itemAdd, spaceLeft)` where it should use `std::min`, so the first
item grows to its maximum regardless of the space left. Shrinking with `Factor::Ordered` isn't
implemented.

`wondergui/source/base/wg_packlayout.cpp` (`getItemSizes()`)

### `PackLayout`: hide settings and weight sources look broken

Found by reading the code, not confirmed by running it. In `getItemSizes()`:

- `HideSetting::Restrained` sums its minimums from a pointer that doesn't advance, and subtracts
  a single byte instead of an `spx`.
- `HideSetting::Aggressive` compares every item against the first item's minimum.
- `pWeightSpx` is declared again inside the `if`, so the outer one stays `nullptr` and
  `SizeSource::Weight` doesn't get its values.
- `PackPanel` doesn't handle the -1 lengths that mark hidden items.

`wondergui/source/base/wg_packlayout.cpp`, `wondergui/source/widgets/panels/wg_packpanel.cpp`

## WonderApp

### `Window::setSize()`, `setPos()` and `setGeo()` don't change the window

`_updateWindowGeo()` is an unimplemented stub that returns what it's given, so the window keeps
its geometry while `size()` reports the new one.

`wonderapp/source/wappwindow.cpp`

### SDL frameworks ignore SIGTERM and other requests to quit

The handling of `SDL_EVENT_QUIT` (`SDL_QUIT` for SDL2) is commented out in `process_system_events()`,
so an app only quits when its windows are closed or `update()` says so. `timeout` and Ctrl+C in a
terminal don't stop it, which matters for scripted runs: kill such a process with SIGKILL. Found
with gfxdevicetester, which behaves the same with OpenGL and Vulkan.

`wonderapp/source/sdl3/main.cpp`, `wonderapp/source/sdl2/main.cpp`

### wappsdl2gl draws into the front buffer of its windows

It never swaps, which shows nothing on Wayland, where a window isn't mapped until it has swapped
in a buffer. SDL2 uses X11 by default, so this only hits with `SDL_VIDEODRIVER=wayland`.
wappsdl3gl renders into a surface per window and swaps it in, which wappsdl2gl could copy.

`wonderapp/source/sdl2/backend_opengl.cpp`

## Documentation and scripts

### The guide doesn't build with Hugo 0.169

The vendored hugo-book theme calls `resources.ToCSS`, which Hugo 0.169 doesn't have, so every
page fails to render. Updating the theme or changing the call should fix it.

`wondergui/doc/guide/themes/hugo-book/layouts/partials/docs/html-head.html`

### `build-wg-superheader.sh` and `build-package.sh` use paths that no longer exist

They expect `src/baselibrary/` and `build/gnumake/` from older repository layouts.

`scripts/`

## Not yet verified

### API reference generation

`make_docs.bat` was updated for the current layout. Copying the sources and running
`predoxy.rb` has been tested, but not `doxygen` and `postdoxy.rb` (which needs the Ruby
module nokogiri), since neither was available on the machine.

### `TablePanel::setMinRowHeight()` had no visible effect

In Cybersim's report page a `TablePanel` of 4 rows of `TextDisplay` cells, with no `rowLayout`
and with `setMinRowHeight(0, 4, 30)` called right after creating it, drew its rows at the height
of the text, about 23 points. `columns[i].setMinWidth()` on the same table did work. Not looked
into further: the rows got their spacing from `rowSpacing` instead.

`wondergui/source/widgets/panels/wg_tablepanel.cpp`

### Text system rewrite on macOS

The UTF-8 text system was built and tested on Linux (SDL2 software and OpenGL) and has
been run on Win32/DX12. The macOS frameworks (Cocoa software/Metal, SDL2 Metal) haven't
been built with it yet.

### GlBackend on GpuBackend timed on few GPUs

GlBackend on GpuBackend and the generated GLSL runs gfxdevicetester on Windows, on a 2013 Mac Pro
(AMD, macOS OpenGL 4.1) and on Mesa's llvmpipe, with PaletteBlitTests passing on the Mac Pro too,
where texelFetch() of palettes once gave black pixels.

Timed with gfxdevicetester on llvmpipe, which runs shaders on the CPU and so shows every
instruction, it is faster overall than the old GlBackend (483 ms against 518 ms for one run of
each test), but blurs are about 20% slower, tiled blits 20%, palette blits up to 35% and rounded
boxes 15%: the shared shaders branch on constants where the old GLSL had a program for each case,
and the GLSL from SPIRV-Cross reads a buffer element again wherever it is used, which llvmpipe
doesn't merge, see `openglgfx/compile_shaders.py`. Timed on Intel UHD 770 (Mesa) since, where it is
faster than the old GlBackend overall (0.78 times as long for one run of each test), with blurs as
the exception (2.6 times as long), and on an Arc B580 on Windows, see "GlBackend: drawn shapes are
slow on Windows" below.

### VulkanBackend on Windows and on other GPUs

Only run on Linux, with lavapipe and Intel UHD 770 (Mesa).

## Graphics

### gfxdevicetester: tests that differ on GlBackend and VulkanBackend alike

SegmentTests Circles, MipmapTests Mipmapped_0.4 and EdgemapTintTests ReplaceVerticalAndFlat differ
from ReferenceBackend by the same amounts on GlBackend (llvmpipe) and VulkanBackend (lavapipe and
Intel UHD 770), so it is likely something they have in common with each other, and maybe with the
other GPU backends, rather than a bug of one backend. Not looked into yet. At bcf58e51 (2026-11)
Circles was no longer among those that differ on VulkanBackend (Intel UHD 770) or MetalBackend,
the other two were, on both.

### gfxdevicetester's ReadbackTests differs on VulkanBackend

The RGB_565 canvas is kept as XRGB_8, as on DX12Backend and MetalBackend, which keeps the dark
colors a Replace leaves in a linear canvas, where RGB_565 rounds them to black. Expected, see the
comment in `readbacktests.h`. MetalBackend differs the same way (max 28, mean 0.796).

### gfxdevicetester: tests that differ on DX12Backend and MetalBackend alike

On a Mac (x86_64, Debug, 2026-11-23) twelve tests differ from ReferenceBackend, the same at
bcf58e51 as at d155b18a, so not from the blending moved to GpuBackend. Three are those of the
two entries above. The other nine differ by exactly the same amounts on DX12Backend (Windows,
Arc B580), so it is something the two have in common and not the GPU. They don't differ on
VulkanBackend (Intel UHD 770), and on GlBackend (Arc B580) the three of palettes and BlitGridNear
are OK and the other five have specks:

    BlitConsistencyTests  BlitGridNear             max 255  mean 0.314  over  1293
    SamplingTests         RotScaleNearestAlpha     max 151  mean 0.068  over   665
    SamplingTests         RotScaleNearestRGBX      max 148  mean 0.060  over   420
    PaletteCapacityTests  Palette8                 max 215  mean 0.058  over    65
    PaletteCapacityTests  Palette8Capacity16       max 215  mean 0.058  over    65
    PaletteCapacityTests  Palette256               max 232  mean 0.047  over    71
    WarpTests             Blit nearest wave        max 199  mean 0.097  over   404
    WarpTests             Blit source rectangle    max 199  mean 0.051  over   465
    WarpTests             Tiling not power of two  max 199  mean 0.062  over   344

Few pixels each, far off. Several are of nearest sampling by their names, so a guess is which
texel D3D12 and Metal pick at the border between two, where GL and Vulkan pick the other. Not
looked into.

### VulkanBackend stalls when pixels of a surface are uploaded

VulkanSurface submits what every backend has recorded and waits for the GPU before and after
copying pixels to a texture. Fine for surfaces filled while loading, slow for one updated every
frame. DX12Backend records the uploads into its own command list instead, see
`DX12Backend::submitUploads()`.

`syncTexture()` in `wax_vulkansurface.cpp`

### DX12Backend: canvas layers take 1.5-3 times as long as on GlBackend

On Windows (Arc B580), gfxdevicetester's tests drawing into canvas layers took 130-200 times as
long as on GlBackend, then 5-9 times with the pixels of a DX12Surface in CPU cached memory
(95e05825). A canvas layer no longer gets a pixel buffer, nor an upload of transparent black, see
"DX12Surface: canvases without a pixel buffer" in ROADMAP.md, which leaves this (gfxdevicetester,
Release, two runs of each, 2026-11-23):

    Test                                        at first   95e05825      now       GlBackend
    CanvasLayerTests     CanvasLayerTest1       54.9 ms    2.45 ms       0.76 ms   0.27 ms
    FlattenLayersTests   WithCanvasLayers       84.1 ms    3.1 ms        1.16 ms   0.59 ms
    LayerRoundTripTests  ViaCanvasLayer         26.6 ms    0.82-0.87 ms  0.23 ms   0.15 ms
    ReadbackTests        Readback                4.7 ms    3.3 ms        3.2 ms    2.0 ms

Without canvas layers the same tests are faster on DX12Backend (NoCanvasLayers 18 us against
210 us, ViaSurface 15 us against 100 us).

What is left is making the texture of each layer, a committed resource, which DeviceGen2 does for
every canvas update (`_beginCanvasUpdate()`). Timed by itself it grows with the size, less than
the pixel buffer did (ARGB_8, two runs of 20 each):

    Size         Texture          Pixel buffer
    64x64        150-250 us       65 us
    512x512      160-280 us       95 us
    1024x1024    210-410 us       205 us
    2048x2048    390-1050 us      630 us
    3840x2160    740-1860 us      1210 us

Tried: D3D12_HEAP_FLAG_CREATE_NOT_ZEROED makes no difference. A texture placed in a heap that is
there already takes 21 us, but needs a heap kept between updates, which is what GL drivers do.
Decided with Tord not to keep canvases or memory for them behind the scenes: if anything, layer
surfaces are to be supplied or kept by whoever owns the canvas.

`_createDeviceObjects()` in `wax/extensions/dx12gfx/wax_dx12surface.cpp`

### DeviceGen2 makes a surface for every canvas layer, used or not, of the size of the canvas

`CanvasLayers` says that the surface of a layer is made when something first draws to the layer,
so that layers not used cost nothing. DeviceGen2 makes one for every layer with a format or a
function of its own when the canvas update begins, clears it, and blends it onto the canvas when
the update ends, also when nothing was drawn to it. The surfaces are of the size of the whole
canvas, however small the rectangles to update are. RootPanel and CanvasCapsule begin a canvas
update with their layers for every render, so a window with layers pays for all of them each time
something in it changes. All backends.

`DeviceGen2::_beginCanvasUpdate()` and `_doFlattenLayers()` in `wax/source/wax_device_gen2.cpp`,
`wax/source/wax_canvaslayers.h`

### DX12Backend: shaders changed outside Windows leave its bytecode out of date

DX12Backend draws with bytecode that is checked in (`wax_dx12shaders.cpp`), compiled by FXC,
which only runs on Windows. After a shader in `wax/extensions/hlsl` is changed on Linux or macOS,
DX12Backend keeps drawing with the old one until `dx12gfx/compile_shaders.py` is run on Windows.
CMake warns about it when a DX12 build is configured, if it finds a Python, and
`compile_shaders.py --check` tells anywhere.

`wax/extensions/dx12gfx/compile_shaders.py`

### DX12Surface: a small surface takes at least 128 KB

The texture of a DX12Surface and the buffer with its pixels are a committed resource each, which
D3D12 makes at least 64 KB, and there are one or two descriptor heaps besides. A 32x32 icon has 4 KB of
pixels. A canvas that isn't buffered has no pixel buffer until its pixels are asked for, so half. Edgemaps, Warpmaps and palettes had the same and get their buffers from DX12BufferPool
since, see ROADMAP.md, but that is for buffers that are only written: the pixels are read by the
CPU, so they would need blocks of cached memory, and textures a heap to be placed in.

Accepted as it is (Tord, 2026-11): graphics is meant to be baked into larger surfaces, for
performance too, so nothing is planned for this.

`_createDeviceObjects()` in `wax/extensions/dx12gfx/wax_dx12surface.cpp`

### DX12BufferPool keeps a block for as long as one buffer is left in it

Buffers aren't moved, so a block that had many buffers and has one left stays, 64 KB of it. New
buffers go to the oldest block with room, which lets newer ones empty.

`wax/extensions/dx12gfx/wax_dx12bufferpool.cpp`

### DX12BufferPool releases nothing until something is drawn or allocated

What is given back while the GPU may still read it waits for `collect()`, which runs at the next
`beginRender()` of any DX12Backend and at the next `alloc()`. An application that stops drawing,
like the editor of a plugin, keeps a buffer too big for a block that it let go of, or a block
that emptied, until then. Textures and frame buffers don't have this, the thread pool of DX12Backend releases
them when the GPU is done, see ROADMAP.md.

`DX12BufferPool::free()` and `collect()` in `wax/extensions/dx12gfx/wax_dx12bufferpool.cpp`

Having the callbacks of that thread pool release these too was looked at and left: one only runs
for a submission that held a texture or an outgrown buffer, and it runs on another thread, so the
pool would need a lock and its own wait. A buffer given back when the GPU is done already, as
when a widget goes between frames, is released at once, and its block with it if it emptied
and another is empty already.

### GlBackend: drawn shapes are slow on Windows, and come at two speeds

Timed with gfxdevicetester on an Arc B580 (driver 32.0.101.9033, wappsdl3gl), the tests drawing
pie charts, rings, rounded boxes, line caps, circles and ellipses take 3-11 times as long as on
DX12Backend on the same GPU, where all tests taken together are level (median ratio 1.01). The
thread calling us mostly waits during them, no core is loaded more than 20%.

The shape shaders have been made faster since, which on Intel UHD 770 (Mesa) made rounded boxes
2.6 times as fast and the other shapes 10-20%, see "Shape shaders on GPUs" in ROADMAP.md. Timed
on Windows after that, the shapes were level with the old GlBackend but for line caps, which
took up to 4 times as long. The line shader has been rewritten since, without arrays indexed at
run time, which made line caps 2-3 times as fast on Intel UHD 770. On Windows that made them 1.5-13
times as fast, but for the tests drawing each line in a clip of its own, which stayed 4.6 times
as slow. Those switched pipelines twice per line, which is gone now, not timed on Windows since.

DX12Backend has run the new line shader, compiled by FXC, in all of gfxdevicetester on Windows
(Arc B580, 2026-11-23): LineCapTests pass, not timed. MetalBackend has run the MSL made from it
(a Mac, x86_64, 2026-11-23), not timed: LineCapTests come out as on DX12Backend and GlBackend,
eight OK and Round solaris and Dots with specks (max 21 and 24). It is the same shader as
GlBackend and VulkanBackend have, where it gives the same results as before.

They also come out at one of two or three speeds for a whole run of the tester, the same in all
its passes (PieChartTests 126, 167 or 187 us per run), which makes them the tests that still
differ by 25-40% between two runs. Running on fewer cores doesn't change it. Not looked into what
makes the driver wait.

### Edgemaps and Warpmaps of GPU backends have one buffer for two frames in flight

Changing an edgemap or a Warpmap writes into the buffer the GPU may still be reading for the
previous frame, so it can show the new content early. DX12, Metal and Vulkan alike.

### VulkanBackend: a lost device stops rendering for good

It is reported once, and nothing renders after that. Replacing the device, as DX12Backend
does, is planned, see ROADMAP.md.

### VulkanBackend: one memory allocation per buffer and image

Drivers can limit allocations to 4096, which an app with that many surfaces would hit.
RangeAllocator (`wax/source/wax_rangeallocator.h`), which DX12BufferPool hands out ranges of its
blocks with, is there to do the same for blocks of Vulkan memory.

`VulkanContext::createBuffer()` and `createImage()` in `wax/extensions/vulkangfx/wax_vulkancontext.cpp`
### SoftBackend doesn't use the `mipmap` flag of a surface

`Surface` keeps `mipmap` from its Blueprint, but nothing in `wax/extensions/softgfx` reads it:
no smaller levels are made and blits always sample the full-size pixels. A blit to a much
smaller size, like an `Image` showing a 1173 pixel wide picture 150 wide, should therefore alias.
That hasn't been measured: Cybersim shrinks its pictures itself before making surfaces of them.

`wax/extensions/softgfx/wax_softsurface.cpp`, `wax_softbackend.cpp`

### SoftBackend draws thick rings slower than the grid of nodes did

Linear triangles follow a ring less closely than the bilinear cells of the grid of nodes did, so
for the same precision a ring takes pieces about half the size, around 8 x 15 pixels for a ring
of radius 90 to 148. SoftBackend spends about as much on each piece on each row as on drawing
it, so such a ring takes 1.25 times as long as before, and 1.6 times as long when it is a blit,
which reads the surface in a kernel call for each piece on each row. Thin rings, rounded frames,
focus rings, cylinders and planes in perspective take as long as before or less, down to 0.67.
Measured on x86-64 against commit 2614ea3.

`wax/source/warp/wax_warpgenerator.cpp` (`_bandLayout()`), `wax/extensions/softgfx/wax_softbackend_warp.cpp`

### gfxdevicetester's Perspective warp test differs on SoftBackend

SoftBackend divides for perspective every few pixels and keeps within a quarter of a pixel in
between, as Warpmap allows, while ReferenceBackend is exact. Along sharp edges inside the
textures of the cards that shows in a few hundred pixels, by up to about a third of the color
range. Borders of the source and edges of edgemaps are within 1/64 of a pixel. See the ROADMAP
entry on dividing for perspective for the trade-off.

### MeshCompressor can miss a short stretch of a sequence

MeshCompressor chooses the lag of each sequence by trying lags on 8 windows of 16 values spread
over it. A stretch unlike the rest that is shorter than the space between the windows can fall
between them, and is then coded with a lag that suits the rest. The coverage of a BallWarp of
300 pixels is such a case: all full over the grid, then alternating full and none over a few
hundred vertices along the outline. It takes 818 bytes rather than 19, 5% of the update. Smaller
balls are caught by the windows. Only sent when tilt or roll changes, not when spinning.

`MeshCompressor::compress()` in `wax/source/compression/wax_meshcompression.cpp`

## C API

### `wax_getFreeTypeFontBitmapCache()` returns a retained cache

The other C API getters, such as `wax_getBitmapFontSurface()` and `wax_getDeviceSurfaceFactory()`,
return a borrowed object, but this one retains the cache, so a caller that treats it like the
others leaks a reference. The header doesn't say which is meant. Dropping the retain is a change
of contract for callers that already release it, so it is left until someone decides.

`wax/extensions/freetypefont/wax_c_freetypefont.cpp`
