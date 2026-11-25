# Suggestions

Things that work as intended but could be nicer: paper cuts, small conveniences, places
where the first try goes wrong. Bugs and limitations go in KNOWN_ISSUES.md, plans and
decisions in ROADMAP.md. Add an entry when you run into something, and remove it when it
has been done or decided against.

Each entry says what came up, where, and what might help. They are observations from using
the library, not decisions.

## Using WonderGUI without WonderApp

Cybersim draws its own 3D view and puts WonderGUI on top of it, so it drives a `RootPanel`
itself as `example01` does. Most of these come from that.

### One call to set up the software renderer

A host needs `SoftBackend::create()`, `addDefaultSoftKernels()`, `GfxDeviceGen2::create()`, and
then `Base::setDefaultDevice()`, `setDefaultSurfaceFactory()`, `setDefaultEdgemapFactory()` and
`setDefaultWarpmapFactory()`. Leaving the last four out works until the first text is drawn
(see KNOWN_ISSUES.md). `example01` leaves them out, and the only place that shows the whole
list is `wonderapp/source/sdl3/backend_software.cpp`. `SoftDeviceFactory` may already be most
of this. A documented one-liner, or `example01` doing it, would save the search.

### A `HostBridge` that does nothing, as the default

`Base::init(nullptr)` is what `example01` shows, and it crashes once a `LineEditor` is added
(see KNOWN_ISSUES.md). If `Base` used a do-nothing bridge when given none, callers of
`Base::hostBridge()` wouldn't need to check, and small hosts wouldn't need to write one.

### SDL3 input translation outside WonderApp

The table from `SDLK_*` to `Key`, the mouse button translation and the event switch are in
`wonderapp/source/sdl3/main.cpp`, and a host that bypasses WonderApp writes them again.
Cybersim's copy is `src/gui/GuiInput.cpp`. A small extension with `mapSDLKeys()` and
`translateSDLEvent(event, root)`, and a `HostBridge` for SDL's clipboard and cursor, would let
both use the same code.

### Loading a file into a `Blob`, and a picture into a `Surface`

`loadBlob()` and `wappLoadSurface()` belong to WonderApp. Without it, reading a font file into
a `Blob` and a PNG into a `Surface` is the first thing a host writes. `Blob::load(path)` in Wax
would cover fonts. Pictures need a decoder, so maybe an extension next to `freetypefont` that
uses the stb_image already in `thirdparty`.

### `example01` points at examples that aren't there

Its header says "there are better examples to use as a template" and "See other examples for
a more full initialization", but it is the only one in `wondergui/examples`. A second example
with a font, a text field and keyboard input would be the template most hosts need.

### Errors go nowhere unless a handler is set

`Base::throwError()` does nothing when no error handler has been set, so a host that hasn't
called `Base::setErrorHandler()` never sees the warnings WonderGUI gives. Printing to stderr
by default, at least in debug builds, would make first-time mistakes visible.

## Building

### Library targets that carry their include directories

`wax`, `waxsoftgfx`, `waxfreetypefont` and `wondergui` add their include directories as
`PRIVATE` and hand them to the parent in variables (`WONDERGUI_INCDIRS` and so on). Inside this
repository that works, but a project that adds the libraries has to wire the variables up
itself. Cybersim wraps them in an interface target in its `cmake/WonderGUI.cmake`. `PUBLIC`
include directories, or one `WonderGUI::wondergui` style target per library, would make
`target_link_libraries()` enough.

### A way to build only the libraries

The root `CMakeLists.txt` always builds a WonderApp framework, the tools and the examples, and
by default wants SDL2, OpenGL and GLEW for `wappsdl2gl`. A product that only needs the
libraries adds `wax`, `wax/extensions/softgfx`, `wax/extensions/freetypefont` and `wondergui`
with `add_subdirectory()` one by one, and has to `include(GenerateExportHeader)` and set
`WONDERGUI_FETCH_DEPS` first, since those come from the root. An option such as
`WONDERGUI_BUILD_TOOLS=OFF`, on by default, would let products use the root as it is.

### CMake 4.2.3 as the minimum

The root asks for CMake 4.2.3 or newer, which is newer than what current Linux distributions
ship. The library folders don't state a minimum of their own, and Cybersim, whose own minimum
is 3.28, adds them directly. That has only been run with CMake 4.4, so whether the libraries
need anything from 4.x is not known. If they don't, a lower minimum for them would help
products that build with a distribution's own CMake.

### New SDL GPU shaders need CMake to configure again

`wax/extensions/sdlgpugfx/CMakeLists.txt` turns the shaders in `shaders/` into arrays in
`wax_sdlgpushadercode.cpp` of the build directory when CMake configures. The `CONFIGURE_DEPENDS`
glob only notices shaders that are added or removed, so after a change to a shader, such as
a pull that brings new ones, a build in an existing build directory (from Xcode, say) keeps the
old ones until `cmake --preset ...` is run again. On the Mac, the new vertex layout of
`vertices-without-uv` with the old `segmentsVS` failed with "Vertex attribute
in_var_TEXCOORD0(3) is missing from the vertex descriptor", and 60 tests of
`gfxdevicetester --accuracy` differed. Adding the shader files to the `CMAKE_CONFIGURE_DEPENDS`
directory property would make the build configure again when one changes. Making the source
file in an `add_custom_command()` that depends on the shaders would leave configuring out of it.

## API

### `Size`, `Border` and `Corners` can't be `constexpr`

`CoordT` has `constexpr` constructors but `SizeT`, `BorderT` and `CornersT` don't, so
`constexpr wg::Size buttonSize{250, 90};` doesn't compile while the same with `wg::Coord` does.
Layout constants are a natural use.

### A transparent canvas takes a trick

To draw a GUI onto a transparent surface that is then laid over something else, the areas
`RootPanel` redraws must first be cleared, or widgets that go away stay visible. What works is
a root skin of `ColorSkin` with `BlendMode::Replace` and `HiColor::Transparent`. A sentence in
the guide, or a `RootPanel` blueprint member saying the canvas is an overlay, would save
working it out.

### Reading the pixels of a canvas waits for the GPU each time

A canvas of a GPU backend has no pixels of its own, so each `pushPixels()` reads the texture back
and waits for the GPU, also when nothing has been rendered into it since the last one, and
`alpha()` reads it back again after each time it has been rendered into. Reading a canvas and
then asking for its alpha is two round trips. A widget that is rendered into a canvas every frame
and marked against its alpha has the whole canvas read back for each frame the pointer moves in,
where GlSurface read the one pixel until 2026-11-24. Things that would help, none of them needed
by anything we have: bringing an alpha map that is there up to date when all of the canvas is
read back anyway, reading just the pixel asked for when the map is stale, and keeping the buffer
a canvas was last read back by for as long as it isn't rendered into, which is memory held for
what might come. Timings in "GPU surfaces: staging and plain memory" in ROADMAP.md.

### Members of `Surface::Blueprint` are not in alphabetical order

`dynamic`, `format`, `identity` and `mipmap` come after `paletteSize`, where the convention has
them before `finalizer` and `palette`. Moving them breaks every designated initializer that sets
one of them together with a palette member, in the order they have now, so it is a change for
breaking-changes. `restorer` was put where it belongs either way.

`wax/source/surface/wax_surface.h`

### Shrinking a picture well

There is no way to get a good small version of a large picture: SoftBackend doesn't use
mipmaps (see KNOWN_ISSUES.md) and a bilinear blit to an eighth of the size skips most pixels.
Cybersim averages the pixels itself, in linear light with premultiplied colors as
`blending_and_transparency.md` asks of mipmaps, before making the surface (`resized()` in its
`src/image/Bitmap.cpp`). The same as a `Surface` method or utility in Wax would do for icons
and pictures shown at a size that depends on the scale.

## Widgets

From building Cybersim's report page: a table of figures, bar charts and curves.

### Column and row weights of a TablePanel do nothing without a layout

A `TablePanel` given a width and `setColumnWeight()` keeps its columns at the width of their
content, packed to the left. Weights only come into play with a `columnLayout` (a `PackLayout`)
in the Blueprint, which nothing at `setColumnWeight()` hints at. Falling back on
`Base::defaultPackLayout()`, or a line in the header, would help. Cybersim wanted fixed column
widths anyway and uses `columns[i].setMinWidth()`.

### No cell that spans columns

The report's table has figures per part in its upper rows and per part and side below, so the
upper cells should span two columns. `TablePanel` has no span, so Cybersim stacks two tables
and gives their columns matching minimum widths (`ReportPage::summaryTable()`).

### A bar chart

There are `AreaChart` and `PlotChart` but nothing for bars. Cybersim's reaction chart, one
horizontal bar per sign going left or right from a middle line, is built from a `Filler` per bar
and grid line and a `TextDisplay` per label in a `FlexPanel` (`makeReactionChart()` in its
`src/gui/ReportCharts.cpp`): about 60 widgets per chart. A bar chart sharing `Chart`'s grid lines
and labels would replace that.

### A curve is an AreaChart with a transparent fill

A line chart is made by giving an `AreaChart` entry `color = HiColor::Transparent`,
`bottomOutlineThickness = 0` and only top samples. That works well, but the only place that
shows it is `wonderapp/tools/pluginworkbench/testplugin.cpp`. Worth a sentence in the guide's
widget gallery.

### `xLines` and `yLines` of a Chart read the other way around

`xLines` are the horizontal lines, placed by value between `displayFloor` and `displayCeiling`.
`yLines` are the vertical ones, placed by a fraction of the width from 0 to 1, since a chart has
no range along its width. The first guess is that `xLines` are lines at x positions, and that
both are placed by value. Comments at the two members would settle it. A range for the width
(seconds, in Cybersim's case) would let vertical lines be placed by value too.

## Documentation

### The guide says FlexPanel clips what sticks out, the default is to move it

`widget_layout_fundamentals.md` says slots placed partly outside a FlexPanel "will be clipped
when rendering" and that `setConfineWidgets()` changes it. The Blueprint's default is
`EdgePolicy::Confine`, which moves the widget back inside: a text box that was 70 points too
tall ended up 70 points higher than it was placed, over the heading above it. The guide should
describe `edgePolicy` and its default.

### `FreeTypeFont` glyph cache entry in KNOWN_ISSUES.md may be out of date

The entry says sizes that aren't a whole number of pixels hit the cache mismatch.
`FreeTypeFont::setSize()` now starts with `align(_size)`, which rounds to whole pixels, and both
`_findGlyph()` and `_addGlyph()` are called with `m_size`. If that is so, the mismatch can no
longer happen and the entry can go, though the two functions still round differently.

## Tools

### A simulated device loss doesn't happen in an unattended gfxdevicetester run on Win32

`WAPP_DX12_SIMULATE_DEVICE_LOSS=<seconds>` removes the device between frames, counted from the
first time it is looked at, which the Win32 main loop does after the update of the application.
`gfxdevicetester --accuracy` runs all its tests in its first update, so the run is over before
the device can be removed: with 1 and 2 seconds a print showed no loss in a run of 92 seconds
(Debug, 2026-11-23). Results that are the same with and without the variable then say nothing.
Removing the device ahead of the run says little as well: what surfaces were created with hasn't
been sent to their textures before something draws from them, so nothing is lost. What tells is
running the tests, removing the device and running them again, which was done as temporary
changes (2026-11-24): the batch run twice with a frame in between, and the device removed at the
first look. A value that means at once and an option to run the tests twice would make it
something to just run, and replacing a device that works (`_recoverDevice()` without a loss) has
no way to be asked for at all.

`_simulateDeviceLoss()` in `wonderapp/source/win32/dx12/win32gfxbackend.cpp` and
`wonderapp/source/sdl3/backend_dx12.cpp`, `GfxDeviceTester::update()`
