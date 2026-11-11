# Known issues

Bugs and limitations we know about but haven't fixed yet. Add an entry when you
find one, and remove it in the same commit that fixes it.

Each entry says what happens, where in the code, and anything known about the cause.

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

### Text system rewrite on macOS

The UTF-8 text system was built and tested on Linux (SDL2 software and OpenGL) and has
been run on Win32/DX12. The macOS frameworks (Cocoa software/Metal, SDL2 Metal) haven't
been built with it yet.

### GlBackend on GpuBackend: palettes on the 2013 Mac Pro and speed on GPUs

GlBackend on GpuBackend and the generated GLSL runs gfxdevicetester on Windows, on macOS with an
AMD GPU and on Mesa's llvmpipe. The old GLSL read palettes with texture(), as texelFetch() gave
random groups of black pixels on a 2013 Mac Pro (AMD, macOS OpenGL 4.1). The generated GLSL reads
the indexes of palette based surfaces with texelFetch() from an integer texture, and the palette
from a buffer texture, as colors and extras always were. Run PaletteBlitTests on that Mac before
relying on it there. If it shows, a `WAX_GLSL` path in `ps_blit_palette.hlsl` can sample instead.

Timed with gfxdevicetester on llvmpipe, which runs shaders on the CPU and so shows every
instruction, it is faster overall than the old GlBackend (483 ms against 518 ms for one run of
each test), but blurs are about 20% slower, tiled blits 20%, palette blits up to 35% and rounded
boxes 15%: the shared shaders branch on constants where the old GLSL had a program for each case,
and the GLSL from SPIRV-Cross reads a buffer element again wherever it is used, which llvmpipe
doesn't merge, see `openglgfx/compile_shaders.py`. Not timed on a GPU yet.

### VulkanBackend on Windows and on other GPUs

Only run on Linux, with lavapipe and Intel UHD 770 (Mesa).

## Graphics

### gfxdevicetester: tests that differ on GlBackend and VulkanBackend alike

SegmentTests Circles, MipmapTests Mipmapped_0.4 and EdgemapTintTests ReplaceVerticalAndFlat differ
from ReferenceBackend by the same amounts on GlBackend (llvmpipe) and VulkanBackend (lavapipe and
Intel UHD 770), so it is likely something they have in common with each other, and maybe with the
other GPU backends, rather than a bug of one backend. Not looked into yet.

### gfxdevicetester's ReadbackTests differs on VulkanBackend

The RGB_565 canvas is kept as XRGB_8, as on DX12Backend and MetalBackend, which keeps the dark
colors a Replace leaves in a linear canvas, where RGB_565 rounds them to black. Expected, see the
comment in `readbacktests.h`.

### GPU backends stall when pixels of a surface are uploaded

DX12Surface and VulkanSurface submit what every backend has recorded and wait for the GPU before
and after copying pixels to a texture. Fine for surfaces filled while loading, slow for one
updated every frame.

`syncTexture()` in `wax_dx12surface.cpp` and `wax_vulkansurface.cpp`

### Edgemaps and Warpmaps of GPU backends have one buffer for two frames in flight

Changing an edgemap or a Warpmap writes into the buffer the GPU may still be reading for the
previous frame, so it can show the new content early. DX12, Metal and Vulkan alike.

### VulkanBackend: a lost device stops rendering for good

It is reported once, and nothing renders after that. Replacing the device, as DX12Backend
does, is planned, see ROADMAP.md.

### VulkanBackend: one memory allocation per buffer and image

Drivers can limit allocations to 4096, which an app with that many surfaces would hit.

`VulkanContext::createBuffer()` and `createImage()` in `wax/extensions/vulkangfx/wax_vulkancontext.cpp`

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
