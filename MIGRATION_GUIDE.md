# Migrating from breaking-changes to v4

What a project that uses WonderGUI on the branch `breaking-changes` has to change to build and
run on `v4`. v4 contains all of breaking-changes, so only what v4 changed on top of it is here.

v4 is still changing. Every change on v4 that makes existing code fail to build, or behave
differently, is added to this guide in the same commit.

**Contents**

1. [Before you start](#before-you-start)
2. [Checklist](#checklist)
3. [Build and repository layout](#build-and-repository-layout)
4. [Wax: the graphics library on its own](#wax-the-graphics-library-on-its-own)
5. [Colors](#colors)
6. [Renamed terms](#renamed-terms)
7. [Text](#text)
8. [Widgets](#widgets)
9. [Surfaces](#surfaces)
10. [Backends](#backends)
11. [Warpmaps](#warpmaps)
12. [WonderApp](#wonderapp)
13. [C API, plugins and the C# wrapper](#c-api-plugins-and-the-c-wrapper)
14. [Changes that compile but behave differently](#changes-that-compile-but-behave-differently)

## Before you start

**Update to the latest commit of breaking-changes first, and make your code compile there.**
This guide starts from there. Breaking-changes also got changes from v4 along the way (triangle
mesh Warpmaps, for example), and they aren't repeated here.

One change on breaking-changes has to be dealt with before you move: `Color` is no longer an
alias of `Color8` there. Write `Color8` or `HiColor` everywhere your code says `Color` (see
[Colors](#colors)). On v4, `Color` means the 12-bit color, so code that still says `Color` would
compile there and silently mean something else.

Then, on v4:

1. Change the build: CMake root, target names, include paths ([Build](#build-and-repository-layout)).
2. Rename the includes and fix the namespaces ([Wax](#wax-the-graphics-library-on-its-own)).
   This gives most of the compile errors, and the fixes are mechanical.
3. Rename `HiColor` to `Color` ([Colors](#colors)).
4. Move text code to UTF-8 `std::string` ([Text](#text)).
5. Go through [Changes that compile but behave differently](#changes-that-compile-but-behave-differently).
   The compiler doesn't find those.

## Checklist

- [ ] CMake: add the repository root instead of `src/`, link `wax` instead of `wondergfx` and
      `wondergear`, and `wax*` instead of `wg*` for the graphics extensions.
- [ ] Includes: `wg_X.h` → `wax_X.h` for everything in the graphics library; `wondergfx.h` →
      `wax.h`; `wg_gfxdevice.h` → `wax_device.h` and the other `gfx` headers.
- [ ] Namespaces: graphics extension classes (`SoftBackend`, `GlBackend`, `FreeTypeFont`, the
      stream classes, ...) are in `wax`, not `wg`. Forward-declare them in `namespace wax`.
- [ ] `GfxBase` and `GearBase` → `wax::Base` (or `wg::Base` in WonderGUI code); `GfxContext` →
      `wax::Context`.
- [ ] `setDefaultGfxDevice()` → `setDefaultDevice()` and the like; `wg_static_cast` →
      `wax_static_cast`; `WG_IS_LITTLE_ENDIAN` → `WAX_IS_LITTLE_ENDIAN`.
- [ ] `HiColor` → `Color`, `Color8::Red` → `Color::Red`.
- [ ] The renamed terms: `throwError` → `reportError`, `origo` → `origin`, `hotspot` → `pivot`,
      `baggage` → `userData`, in every name they are part of.
- [ ] `String`, `CharSeq`, `CharBuffer` → `std::string` and `std::string_view`, UTF-8.
- [ ] `BasicTextLayout` → `PlainTextLayout`.
- [ ] Per-character styles → `MarkdownTextLayout`, or a `FlowTextLayout` of your own.
- [ ] Text offsets are bytes, not characters.
- [ ] `TextDecoration` is a bitmask: test it with `&`, not `==`.
- [ ] `Surface::Blueprint::buffered` is gone: delete it, or give a `restorer`.
- [ ] Call `pushPixels()` before reading the pixels of a canvas.
- [ ] C API: `wg_` → `wax_` for everything in the graphics library.
- [ ] Rebuild plugins together with their hosts.

## Build and repository layout

### The CMake root is the repository root

`src/` is gone. The CMake root is the root of the repository.

```cmake
# before
add_subdirectory(external/wondergui/src wondergui)
# after
add_subdirectory(external/wondergui wondergui)
```

On the command line, `cmake -S <wondergui>`, or one of the presets in `CMakePresets.json`, e.g.
`cmake --preset linux-wappsdl3gl`. All presets build in `build/`.

`cmake_minimum_required`, the C++ standards (C++17 for the libraries, C++20 for WonderApp and
most tools), `WONDERGUI_FETCH_DEPS` and the default `WAPPLIB` (`wappsdl2gl`) are as before.

### Where things are

| breaking-changes | v4 |
|---|---|
| `src/wondergear/`, `src/wondergfx/` | `wax/source/`, with the subfolders `compression/`, `edgemap/`, `font/`, `surface/`, `warp/` |
| `src/wondergear/c-api/`, `src/wondergfx/c-api/` | `wax/c-api/` |
| `src/extensions/{softgfx,gfxstream,openglgfx,dx12gfx,metalgfx,freetypefont}` | `wax/extensions/` (new: `vulkangfx`, `sdlgpugfx`, and `hlsl` with the shared shaders) |
| `src/wondergui/` | `wondergui/source/`, with `wondergui.h` in `wondergui/` |
| `src/wondergui/c-api/` (`wg_c_hostbridge.h`, `wg_c_types.h`) | `wondergui/extensions/plugin/` |
| `src/extensions/{debugger,legacygui,plugin}` | `wondergui/extensions/` |
| `src/widgetkits/` | `wondergui/widgetkits/` |
| `widgetkit-src/`, `doc/` | `wondergui/widgetkit-src/`, `wondergui/doc/` |
| `src/wonderapp/common/` | `wonderapp/source/` |
| `src/wonderapp/wappsdl2/` | `wonderapp/source/sdl2/` |
| `src/wonderapp/win32-common/`, `win32-dx12/`, `win32-sw/` | `wonderapp/source/win32/`, `win32/dx12/`, `win32/sw/` |
| | new: `wonderapp/source/sdl3/`, `wonderapp/source/cocoa/{sw,metal}/` |
| `src/wrappers/c-sharp/` | `wax/wrappers/c-sharp/` |
| `src/tools/{image2surf,streamgenerator,streamplayer}` | `wax/tools/` |
| `src/tools/{fontgenerator,gfxdevicetester,legacyrunner,pluginworkbench,streamanalyzer,surfviewer,widgetbench,...}` | `wonderapp/tools/` |
| `src/tools/{unittester,workbench,legacyworkbench,embeddedworkbench,streamgendll}` | `wondergui/tools/` |
| `src/tools/helloworld`, `src/examples/example01` | `wonderapp/examples/helloworld`, `wondergui/examples/example01` |
| stb_image and tinyfiledialogs inside the frameworks and tools | `thirdparty/stb/`, `thirdparty/tinyfiledialogs/` |
| `src/tools/{gfxbench,warpmapprototype,softkernelgenerator}`, `dependency-packages/` | removed |

Includes that went through `src/`, such as `<widgetkits/wg_oldskool.h>`, now go through
`wondergui/`, which is in `WONDERGUI_INCDIRS`.

### Targets

| breaking-changes | v4 |
|---|---|
| `wondergear` + `wondergfx` | `wax` |
| `wgsoftgfx`, `wggfxstream`, `wgfreetypefont` | `waxsoftgfx`, `waxgfxstream`, `waxfreetypefont` |
| `wgopenglgfx`, `wgmetalgfx`, `wgdx12gfx` | `waxopenglgfx`, `waxmetalgfx`, `waxdx12gfx` |
| | new: `waxvulkangfx` (needs the Vulkan SDK), `waxsdlgpugfx` (needs SDL3) |
| `wondergui`, `wgdebugger`, `wglegacygui`, `wgplugin`, `wglegacyplugin`, `wgpluginhost` | the same |

`wondergui` links `wax` publicly.

```cmake
# before
target_link_libraries(app PRIVATE wondergui wgsoftgfx wgfreetypefont wondergfx wondergear)
# after
target_link_libraries(app PRIVATE wondergui waxsoftgfx waxfreetypefont wax)
```

### Variables

The include directories of the targets are still private, so the parent scope variables are
still the way to get them. An old name expands to nothing without an error, and the compiler
then doesn't find the headers.

| breaking-changes | v4 |
|---|---|
| `WONDERGEAR_INCDIRS`, `WONDERGFX_INCDIRS` (and `_SOURCES`) | `WAX_INCDIRS`, `WAX_SOURCES` |
| `WGSOFTGFX_*`, `WGGFXSTREAM_*`, `WGFREETYPEFONT_*` | `WAXSOFTGFX_*`, `WAXGFXSTREAM_*`, `WAXFREETYPEFONT_*` |
| `WGOPENGLGFX_*`, `WGMETALGFX_*` | `WAXOPENGLGFX_*`, `WAXMETALGFX_*` |
| | new: `WAXVULKANGFX_*`, `WAXSDLGPUGFX_*` |
| `DX12GFX_*`, `WONDERGUI_*`, `WGDEBUGGER_*`, `WGLEGACYGUI_*`, `WGPLUGIN_*`, `WAPP_INCDIRS`, `WAPP_SOURCES` | the same |

`WONDERGUI_INCDIRS` contains `WAX_INCDIRS`. It no longer has a `c-api` folder: code that
includes `wg_c_types.h` or `wg_c_hostbridge.h` needs `WGPLUGIN_INCDIRS` as well. A build that
sets the include paths by hand must add the subfolders of `wax/source`.

### Export header

Wax and its extensions generate `wax_export.h` with `WAX_EXPORT`. A shared library or plugin of
your own that compiles the Wax sources in and runs `generate_export_header(<target> BASE_NAME wg)`
builds without errors, but exports none of the Wax C API. Generate it with `BASE_NAME wax` too.

### WonderGUI has a C file

The Markdown parser (`wondergui/source/textlayouts/md4c/md4c.c`) is in the `wondergui` target. A
project that adds `wondergui/` directly from a `project(X LANGUAGES CXX)` must enable C:
`project(X LANGUAGES C CXX)`. Adding the repository root is fine, it enables C itself.

### Adding the libraries without the root CMakeLists

In one directory scope, in this order:

```cmake
include(GenerateExportHeader)
set(WONDERGUI_FETCH_DEPS ON)            # or provide Freetype::Freetype
add_subdirectory(<wg>/wax)
add_subdirectory(<wg>/wax/extensions/softgfx)
add_subdirectory(<wg>/wax/extensions/freetypefont)
add_subdirectory(<wg>/wondergui)
```

`freetypefont` finds or fetches FreeType itself now, and stops with an error if it can do
neither. `wgdebugger` needs `WG_ROOT` set to the repository root, where it used to be `src/`.

### SDL3, and no SDL_image

- SDL3 replaces SDL2, except in the SDL2 frameworks. The root looks for SDL3
  (`find_package(SDL3 CONFIG)`) or fetches it, and sets `SDL3_LIBS`. It does so for every
  `WAPPLIB`, as tools and examples link it.
- SDL2 is only found or fetched by the SDL2 frameworks, and only when one of them is selected.
  `SDL2::SDL2` doesn't exist otherwise.
- **SDL_image is gone.** WonderApp loads images with stb_image. `SDL2IMAGE_LIBS` and the
  `SDL2_image` target no longer exist, and the SDL2 frameworks no longer call `IMG_Init()`. An
  application that uses SDL_image itself must find and link it itself.
- On Linux, an SDL3 built from source needs the X11 development packages. Without them, configure
  with `-DSDL_X11_XCURSOR=OFF -DSDL_X11_XINPUT=OFF -DSDL_X11_XFIXES=OFF -DSDL_X11_XRANDR=OFF
  -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XSHAPE=OFF -DSDL_X11_XTEST=OFF`.

### stb_image and tinyfiledialogs are in the frameworks

All WonderApp frameworks compile stb_image (`STB_IMAGE_IMPLEMENTATION`) and tinyfiledialogs, and
`WAPP_INCDIRS` has their folders. An application that has its own implementation of either
will likely get duplicate symbols when linking. Drop yours.

### Shaders

Nothing needs a shader compiler to build. The shaders are checked in.

- DX12 has its bytecode in `wax_dx12shaders.cpp`. If Python is found, configuring warns when the
  bytecode is older than the HLSL.
- Vulkan and SDL GPU turn their checked-in shader files into a `.cpp` when CMake configures.
  **Configure again after pulling changed shaders,** or the old ones stay in the build. A build
  that doesn't use the provided CMakeLists must do that step itself.

### Line endings

All text files are LF, Windows included (`.gitattributes` has `* text=auto eol=lf`). This only
matters for patches you carry against WonderGUI's files.

## Wax: the graphics library on its own

WonderGfx and WonderGear are one library, Wax, in `namespace wax`. WonderGUI uses it, and brings
a list of its names into `wg`.

### What `wg` still has

`wondergui/source/base/wg_wax.h` brings most Wax names into `wg`, so WonderGUI code that uses them
keeps compiling:

- objects and pointers (`Object`, `StrongPtr`, `WeakPtr`, `Blob`, `Finalizer_p`, ...), containers
  and memory classes
- the geometry types (`CoordSPX`, `RectI`, ...), `Patches`, `Spans`
- `Surface`, `SurfaceFactory`, `PixelBuffer`, the surface reader and writer
- `Edgemap` and its factory and shapes, `Warpmap` and its factory, `WarpGenerator` and the warps
- `Font`, `BitmapFont`, `SysFont`, `DummyFont`, `BitmapCache`
- `Printer`, `Tint`, `ColorStop`, the transitions, `Blurbrush`, `CanvasLayers`, `BackendLogger`
- the compressors and decompressors
- `Color`, `Color8`
- all enums and types of `wax_types.h`, and the namespaces `PixelTools`, `GpuShapes`,
  `TintTools`, `Utf8`
- the old names `GfxDevice`, `GfxDeviceGen2`, `GfxDeviceFactory` and `GfxBackend`, as aliases of
  `wax::Device`, `wax::DeviceGen2`, `wax::DeviceFactory` and `wax::Backend`

### What `wg` doesn't have

These need `wax::` or `using namespace wax;`. Old code that uses them in `wg` fails to compile.

| breaking-changes | v4 |
|---|---|
| `wg::GfxBase`, `wg::GearBase` | `wax::Base`. In WonderGUI code `wg::Base`, which derives from `wax::Base` |
| `wg::GfxContext`, `wg::GearContext` (and `_p`, `_wp`) | `wax::Context` |
| all classes and functions of the graphics extensions: `SoftBackend`, `SoftSurface`, `LinearBackend`, `addDefaultSoftKernels()`, `GlBackend`, `DX12Backend`, `MetalBackend`, `FreeTypeFont`, `StreamEncoder`, `StreamPlayer`, `MirrorSurface`, ... | `wax::SoftBackend` and so on |
| `wg_dynamic_cast<>()`, `wg_static_cast<>()` | `wax_dynamic_cast<>()`, `wax_static_cast<>()` |

New in `wax`, and not in `wg` under these names: `Backend`, `Device`, `DeviceGen2`,
`DeviceFactory`, `Base`, `Context`, `Util`, `GpuBackend`, `GpuSurface`.

**With both `using namespace wg;` and `using namespace wax;`,** `Base` and `Util` are ambiguous, as
they are different things in the two. Write `wg::Base::init()` or `wax::Base::...`. Every other
name they share is the same entity in both. With `using namespace wax;`, names such as `Backend`,
`Device` and `Context` can also clash with names of your own.

**Forward declarations of graphics classes go in `namespace wax`.** `namespace wg { class
Surface; }` conflicts with the using-declaration (an error in clang, accepted by GCC), and
`namespace wg { class SoftBackend; }` silently declares another, incomplete class.

```cpp
// before
namespace wg { class Surface; class SoftBackend; }
// after
namespace wax { class Surface; class SoftBackend; }
```

### Headers

The rule is `wg_X.h` → `wax_X.h` for everything that is in Wax now, extensions included. There
are no headers with the old names. Where the name had `gfx` or `gear` in it, that part is gone:

| breaking-changes | v4 |
|---|---|
| `wondergfx.h`, `wondergear.h` | `wax.h` (includes the warp headers too) |
| `wg_gfxbase.h`, `wg_gearbase.h` | `wax_base.h` |
| `wg_gfxtypes.h`, `wg_geartypes.h` | `wax_types.h` |
| `wg_gfxenumextras.h`, `wg_gearenumextras.h` | `wax_enumextras.h` |
| `wg_gfxbackend.h` | `wax_backend.h` |
| `wg_gfxdevice.h`, `wg_gfxdevice_gen2.h`, `wg_gfxdevicefactory.h` | `wax_device.h`, `wax_device_gen2.h`, `wax_devicefactory.h` |
| `wg_gfxutil.h` | `wax_util.h` |
| `wg_softgfxdevicefactory.h`, `wg_glgfxdevicefactory.h`, `wg_metalgfxdevicefactory.h`, `wg_streamgfxdevicefactory.h` | `wax_softdevicefactory.h`, `wax_gldevicefactory.h`, `wax_metaldevicefactory.h`, `wax_streamdevicefactory.h` |
| `wondergfxstream.h` | `waxgfxstream.h` |
| `wg_color.h` | `wax_color.h` |

`wondergui.h` includes `wax.h`, but no extensions.

### Classes and methods without the Gfx prefix

| breaking-changes | v4 | Still in `wg` under the old name? |
|---|---|---|
| `GfxBackend` | `wax::Backend` | yes, alias |
| `GfxDevice`, `GfxDeviceGen2` | `wax::Device`, `wax::DeviceGen2` | yes, alias |
| `GfxDeviceFactory` | `wax::DeviceFactory` | yes, alias |
| `GfxDeviceFactory::createGfxDevice()` | `DeviceFactory::createDevice()` | no |
| `SoftGfxDeviceFactory`, `GlGfxDeviceFactory`, `MetalGfxDeviceFactory`, `StreamGfxDeviceFactory` | `wax::SoftDeviceFactory`, `wax::GlDeviceFactory`, `wax::MetalDeviceFactory`, `wax::StreamDeviceFactory` | no |
| `Base::setDefaultGfxDevice()`, `defaultGfxDevice()` | `setDefaultDevice()`, `defaultDevice()` | no |
| `Base::setDefaultGfxDeviceFactory()`, `defaultGfxDeviceFactory()` | `setDefaultDeviceFactory()`, `defaultDeviceFactory()` | no |
| `Printer::setGfxDevice()`, `gfxDevice()` | `Printer::setDevice()`, `device()` | no |
| `wg::Util` (from `wg_gfxutil.h`) | `wax::Util`; `wg::Util` has `using namespace wax::Util` | yes |

`RootPanel::setGfxDevice()` and `gfxDevice()` are unchanged. A device factory of your own must
override `createDevice()`. With `override` on the old name it fails to compile; without it the
class stays abstract.

### Macros

| breaking-changes | v4 |
|---|---|
| `WGBP`, `WGOVR`, `WGCREATE`, `WGTRACK` | still there for `wg`. For classes only in `wax`: `WAXBP`, `WAXOVR`, `WAXCREATE`, `WAXTRACK` |
| `WG_IS_BIG_ENDIAN`, `WG_IS_LITTLE_ENDIAN`, `WG_IS_64_BITS` | `WAX_IS_BIG_ENDIAN`, `WAX_IS_LITTLE_ENDIAN`, `WAX_IS_64_BITS` |
| `LINK_METHODS`, `PROTECTED_LINK_METHODS` | `WAX_LINK_METHODS`, `WAX_PROTECTED_LINK_METHODS` |
| `WG_EXPORT`, `wg_export.h` | `WAX_EXPORT`, `wax_export.h` |

`WGBP` expands to `wg::T`, so `WGCREATE(SoftBackend, ...)` no longer compiles. Write
`WAXCREATE(SoftBackend, ...)`.

### Initialization

A WonderGUI application calls `wg::Base::init()` and `wg::Base::exit()` as before.

An application that uses only the graphics library:

```cpp
// before
wg::GfxBase::init();
...
wg::GfxBase::exit();
// after
wax::Base::init();
...
wax::Base::exit();
```

`wax::Base` has what `GearBase` and `GfxBase` had, with one `wax::Context`
(`Base::context()`, `Base::setContext()`). Init is counted: code that called both
`GearBase::init()` and `GfxBase::init()` keeps one pair of `init()` and `exit()`.

### Smaller changes

- **`EdgemapWave`:** `setPos()`, `pos()`, `Blueprint::pos` and `setFlip()` are removed. The wave
  is at 0,0: place it with the offset you draw it at. The flip is only set through
  `Blueprint::flip`; create a new wave to change it.
- **`TintTools`:** `positionAt()`, `layerColorAt()` with a float position, the float `buildLUT()`
  and `TintLayer::stopPos[]` are removed. Use `stopPos16[i] / 65535.f` where you need a float.
- **SoftBackend kernels:** `addDefaultSoftKernels()` and the other preset functions are as before,
  in `wax`. `softkernelgenerator` is gone: a kernel set of your own is a `constexpr
  SoftKernelSpec` (`wax_softkernelspec.h`), added with `addSoftKernels<spec>()`
  (`wax_softkernelset.impl.h`). `wax/extensions/softgfx/wax_softkernels_tinylinear.cpp` is a
  template to copy.

  ```cpp
  #include <wax_softkernelset.impl.h>

  constexpr wax::SoftKernelSpec c_myKernels = {
      wax::maskOf(TintMode::None, TintMode::Flat),                      // tint modes
      wax::maskOf(BlendMode::Replace, BlendMode::Blend),                // blend modes
      wax::maskOf(SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear), // source formats
      wax::maskOf(SoftFormat::ARGB_8_linear),                           // destination formats
      { { wax::maskOf(SoftBlitType::StraightBlit), wax::maskOf(TintMode::None),
          wax::maskOf(BlendMode::Blend), wax::maskOf(SoftFormat::ARGB_8_linear),
          wax::maskOf(SoftFormat::ARGB_8_linear) } }                    // one-pass blits
  };

  wax::addSoftKernels<c_myKernels>(pSoftBackend);
  ```
- **`SoftSurface::tileMaskX()` and `tileMaskY()`** are replaced by `wrapTilePos(CoordI)`.
- **Class names:** the `TYPEINFO` names follow the renames: "Device", "Backend",
  "DeviceFactory", "SoftDeviceFactory" and so on, where they were "GfxDevice", "GfxBackend", ...
  Errors from `Base` give the class name "Base", where it was "GearBase", and messages say "Wax"
  where they said "WonderGUI". This only matters to code that compares these strings.
- **`SurfaceReader`** without a `factory` in its Blueprint uses `Base::defaultSurfaceFactory()`,
  where it crashed.
- **`Object::isInstanceOf()`** reports an error for a circular `TYPEINFO` chain, where it hung.

## Colors

### `HiColor` is now `Color`, and `Color` is no longer `Color8`

On breaking-changes, `Color` was a typedef of `Color8` (8-bit channels) until it was removed. On
v4, `HiColor` (12-bit channels, 0-4096) is renamed `Color`, and there is no `HiColor` left.

Do it in two steps, so that every mistake is a compile error:

1. **On breaking-changes,** at its latest commit, make your code compile. Every `Color` it still
   has is the old alias and fails, so write `Color8` where you mean 8-bit and `HiColor` where you
   mean the 12-bit color.
2. **On v4,** replace the word `HiColor` with `Color`, and `Color8::Red` and the other named
   colors with `Color::Red`.

| breaking-changes | v4 |
|---|---|
| `Color` (alias of `Color8`, removed) | `Color8` |
| `HiColor` | `Color` |
| `HiColor::Black`, `HiColor::Transparent`, ... | `Color::Black`, `Color::Transparent`, ... |
| `Color8::Cyan` and the rest of the web palette | `Color::Cyan` |
| `Color8::LemonCiffon` | `Color::LemonChiffon` |

### The named colors are in `Color`, at 12 bits

`Color8` has no named colors any more. `Color` has all of them: the web palette, and
`Undefined`, `ShowError`, `Transparent`, `TransparentBlack` and `TransparentWhite` as before.
Their channels are `round(value / 255 * 4096)`, the values a conversion of the old `Color8`
constant gives, so what is drawn doesn't change.

Where you need 8 bits, convert: `Color8 c = Color::Red;`.

**Watch for constants at namespace scope.** The named colors of `Color` are constant-initialized,
so they can be used before `Base::init()`. A conversion between `Color` and `Color8` can't: it
looks the channels up in tables that `Base::init()` fills in, and gives transparent black before
that, without a warning.

```cpp
const Color paper = Color::Linen;         // fine, a copy of a constant
const Color8 paper8 = Color::Linen;       // transparent black if this runs before Base::init()
const Color cream = Color8(0xF3F3F3);     // the same
```

Make such constants `Color`, or create them after `Base::init()`.

**`auto` changes type.** `auto c = Color8::Red;` gave a `Color8`. `auto c = Color::Red;` gives a
`Color`, with 12-bit `r`, `g`, `b`, `a` and a 64-bit `argb`. Code that reads them as 8-bit values
compiles and gets other numbers.

### LegacyGUI: `WgColor` keeps its named colors

`WgColor` is a class derived from `Color8` instead of a typedef. It has the web palette as 8-bit
constants under the old names, `WgColor::LemonCiffon` included, so `WgColor::Black` and the like
compile as before. It converts to and from `Color8`, and from `Color`.

## Renamed terms

Some words are replaced by others in every name they are part of: class, method, field,
parameter, enum and C API names, and the text of messages and documentation. There are no
aliases with the old names, so code with them fails to compile. A search and replace of the old
word, keeping its case, is enough for most code.

LegacyGUI keeps its own names (`WgOrigo`, `SetHotspot()`, ...). Types that are renamed in
WonderGUI and Wax get aliases with their old names in LegacyGUI, so legacy code keeps compiling:
`wg::SampleOrigo`. Renamed methods and fields can't get such aliases.

### Blueprint fields in alphabetical order

Blueprint fields are in alphabetical order, so renamed fields have moved: `userData` is now
near the end of each Blueprint, where `baggage` was first, and `origin` comes before `pivot` in
`FlexPanelSlot::Blueprint`. C++20 code with designated initializers must list them in that
order, or it fails to compile:

```cpp
// before
pPanel->slots.pushBack(pWidget, { .baggage = pData, .hotspot = Placement::Center, .origo = Placement::Center });
// after
pPanel->slots.pushBack(pWidget, { .origin = Placement::Center, .pivot = Placement::Center, .userData = pData });
```

Blueprints filled in with `WGBP()` or `_.userData = ...` don't depend on the order. The C API
structs keep the order they had.

### `throwError` → `reportError`

| Before | After |
|---|---|
| `Base::throwError()` | `Base::reportError()` |

### `origo` → `origin`

| Before | After |
|---|---|
| `SampleOrigo`, `SampleOrigo_min`, `SampleOrigo_max`, `SampleOrigo_size` | `SampleOrigin`, `SampleOrigin_min`, ... |
| `EdgemapWave::Blueprint::origo` and the `SampleOrigo origo` parameters of the edgemaps and their factories | `origin` |
| `FlexPos::origo` | `FlexPos::origin` |
| `FlexPanelSlot::origo()`, `setOrigo()`, `Blueprint::origo` | `origin()`, `setOrigin()`, `Blueprint::origin` |
| `ModalOverlay::Slot::origo()`, `setOrigo()`, `Blueprint::origo` | `origin()`, `setOrigin()`, `Blueprint::origin` |
| `Printer::setCursorOrigo()`, `cursorOrigo()` | `setCursorOrigin()`, `cursorOrigin()` |
| C: `wax_sampleOrigo`, `WAX_SAMPLEORIGO_TOP`, `_MIDDLE_UP`, `_MIDDLE_DOWN`, `_BOTTOM` | `wax_sampleOrigin`, `WAX_SAMPLEORIGIN_TOP`, ... |
| C: `wax_setPrinterOrigo()`, `wax_printerOrigo()` | `wax_setPrinterOrigin()`, `wax_printerOrigin()` |
| C#: `Printer.SetCursorOrigo()`, `CursorOrigo()` | `SetCursorOrigin()`, `CursorOrigin()` |

### `hotspot` → `pivot`

| Before | After |
|---|---|
| `FlexPanelSlot::hotspot()`, `setHotspot()`, `Blueprint::hotspot` | `pivot()`, `setPivot()`, `Blueprint::pivot` |
| `PickMsg::hotspot()`, `setHotspot()` | `pivot()`, `setPivot()` |

### `baggage` → `userData`

| Before | After |
|---|---|
| `Widget::baggage()`, `setBaggage()`, `hasBaggage()` | `userData()`, `setUserData()`, `hasUserData()` |
| `Surface::baggage()`, `setBaggage()` | `userData()`, `setUserData()` |
| `Blueprint::baggage` of every widget, the surfaces and the other objects that have one | `Blueprint::userData` |
| C: `wax_setSurfaceBaggage()`, `wax_getSurfaceBaggage()` | `wax_setSurfaceUserData()`, `wax_getSurfaceUserData()` |
| Plugin interface: `setSurfaceBaggage`, `getSurfaceBaggage` | `setSurfaceUserData`, `getSurfaceUserData`, at the same place |

## Text

### UTF-8 `std::string` instead of `String` and `CharSeq`

`String`, `CharSeq`, `CharBuffer`, `Char` and `TextTool` are gone from WonderGUI. Their headers
(`wg_string.h`, `wg_charseq.h`, `wg_charbuffer.h`, `wg_char.h`, `wg_texttool.h`) are in LegacyGUI
now, built only into `wglegacygui`, and `wondergui.h` doesn't include them.

Text is UTF-8. Setters take `std::string_view`, getters return `std::string`, mostly as a
`const std::string&`. Invalid UTF-8 becomes U+FFFD when it is set.

| Where | breaking-changes | v4 |
|---|---|---|
| `DynamicText` (e.g. `TextDisplay::display`, `TextEditor::editor`) | `setText(const char*)`, `setText(const CharSeq&)`, `setText(const CharBuffer*)`, `setText(const String&)` | `setText(std::string_view)` |
| `DynamicText` | `append()`, `insert()`, `replace()` taking `const CharSeq&` | taking `std::string_view` |
| `StaticText` | `String text() const` | `const std::string& text() const` |
| `EditableText` | `caretPut(const CharSeq&)`, `caretPut(uint16_t)` | `caretPut(std::string_view)`, `caretPut(char32_t)` |
| `EditableText` | `String getSelection()` | `std::string getSelection()` |
| Blueprints of the text components, `ColumnHeader`, `SelectBoxEntry` | `String text` | `std::string text` |
| `ColumnHeader`, `SelectBoxEntry` | `setText(...)`, `String text()` | `setText(std::string_view)`, `const std::string& text()` |
| `GridLine` of charts | `String label`, `const String& label()` | `std::string label`, `const std::string& label()` |
| `Widget` and all Blueprints | `setTooltip(const String&)`, `virtual String tooltip() const`, `String tooltip` | `setTooltip(std::string_view)`, `virtual std::string tooltip() const`, `std::string tooltip` |
| `Base` | `setClipboardText(const String&)`, `String getClipboardText()` | `setClipboardText(std::string_view)`, `std::string getClipboardText()` |
| `InputHandler` | `putText(const CharSeq&)` | `putText(std::string_view)` |
| `TextInputMsg` | `String text()` | `const std::string& text()` |
| `BasicNumberLayout::Blueprint` | `String prefix, prefixNegative, suffix, suffixNegative`; `uint16_t decimalSign, integerGroupSeparator` | `std::string ...`; `char32_t ...`; new `prefixStyle`, `suffixStyle` |
| Debugger: `DebugWindow`, `InspectorView` | `String label`, `setLabel(String)`, `String title()` | `std::string label`, `setLabel(std::string_view)`, `std::string title()` |
| Debugger: `InfoSection` helpers for sections of your own | `const CharSeq&` | `std::string_view` |

String literals and `std::string` passed to these keep compiling.

```cpp
// before
wg::String s = pDisplay->display.text();
std::string u8 = wg::CharSeq(s).getStdString();
pEditor->editor.caretPut(uint16_t('x'));
wg::String MyWidget::tooltip() const { return wg::String("hi"); }
// after
const std::string& u8 = pDisplay->display.text();
pEditor->editor.caretPut(U'x');
std::string MyWidget::tooltip() const { return "hi"; }
```

Text in other encodings is converted with `wg::Utf8` (`wax_utf8.h`): `Utf8::fromUtf16()`,
`fromUtf32()` and `fromLatin1()`. A `std::wstring` on Windows goes through
`fromUtf16(reinterpret_cast<const uint16_t*>(ws.c_str()))`. Characters above U+FFFF work now.

### Offsets are bytes

**This compiles, and gives other results for text that isn't ASCII.** Every offset into text is a
byte offset into the UTF-8 string, where it used to count characters:

- `insert()`, `replace()`, `erase()` and what they return
- `StaticText::length()`, and `text().length()`
- `EditableText::select()`, `selectionBegin()`, `selectionEnd()`, `selectionSize()`,
  `setCaretOfs()`, `caretOfs()`

Offsets are clamped and moved back to the start of a character. Convert with `Utf8`:

```cpp
// before: erase the third character
editor.erase(2, 1);
// after
const std::string& t = editor.text();
int ofs = wg::Utf8::offsetOfCodepoint(t, 2);
editor.erase(ofs, wg::Utf8::next(t, ofs) - ofs);
```

Use `Utf8::countCodepoints(text)` where you want a number of characters. `maxChars` of
`EditableText` counts characters (codepoints).

### No per-character styles

`DynamicText::setCharStyle()` and `clearCharStyle()` are removed, and so are styles carried by
`String` and `CharBuffer`. Styles come from markup, which the text layout interprets:

- **`MarkdownTextLayout`** (new) has a `TextStyle_p` in its Blueprint for each role: `strong`,
  `emphasis`, `code`, `codeBlock`, `heading[6]`, `link`, `listItem`, `quote`, `strikethrough` and
  `markup`. They are added on top of the style of the widget. Use
  `MarkdownTextLayout::escape()` on text from users that shouldn't be read as Markdown.

  ```cpp
  // before
  text.setText("Hello world");
  text.setCharStyle(pBold, 6, 5);
  // after
  auto pLayout = MarkdownTextLayout::create(WGBP(MarkdownTextLayout, _.strong = pBold));
  text.setLayout(pLayout);
  text.setText("Hello **world**");
  ```
- **A `FlowTextLayout` of your own,** overriding `_parse(const std::string& text, Markup& markup)`,
  for markup of your own. It fills `markup.runs`: begin, end, up to six styles, hidden, a
  substitute character and a link.

Links found in markup are reported through `TextLayout::linkAt()`. The messages a `TextLink`
sends are as before.

### `BasicTextLayout` is `PlainTextLayout`

`BasicTextLayout` (`wg_basictextlayout.h`) is renamed `PlainTextLayout` (`wg_plaintextlayout.h`),
with the same Blueprint. It interprets no markup. It is the default text layout.

```cpp
// before
auto pLayout = BasicTextLayout::create(WGBP(BasicTextLayout, _.wrap = true));
// after
auto pLayout = PlainTextLayout::create(WGBP(PlainTextLayout, _.wrap = true));
```

It draws differently from `BasicTextLayout` in a few ways:

- `TextDecoration::Underline` is drawn. `BasicTextLayout` ignored it.
- Moving the caret by word and selecting a word with a double click work. They were stubs.
- Lines are measured again when a change of state changes the size of the font.

**Text layouts of your own:** `onCharStyleChanged()` is gone, `tooltip()` returns `std::string`,
`linkAt()` is new, and `_chars()` is `_text()`. In `TextItem`, `_textBegin()` and `_textLength()`
are one `_text()` returning `const std::string&`. **Number layouts of your own:**
`_renderString()` is replaced by `_renderSegments()` and `_segmentsSizeBaseline()`.

### `TextStyle`

- New: `Blueprint::sizeScale` (default 1), which scales the inherited size when `size` isn't set.
- Fixed: a style with an undefined `backColor` wrote the back color of the default style into
  `color`. Text color can change for styles that leave `backColor` undefined.

### `TextDecoration` is a bitmask

```cpp
// breaking-changes
enum class TextDecoration { Undefined, None, Underline };
// v4
enum class TextDecoration : uint8_t { None = 0, Underline = 1, Strikethrough = 2, Undefined = 0x80 };
```

`|`, `&` and `|=` combine them, and `Strikethrough` is new. What compiles and changes:

- The numbers are different. Stored or serialized values mean something else, and
  `TextDecoration{}` is `None`, where it was `Undefined`.
- Styles stacked on each other combine their decorations, where the last one used to win. `None`
  clears what is inherited, `Undefined` inherits it.
- `==` is false for combinations:

  ```cpp
  // before
  if (attr.decoration == TextDecoration::Underline) ...
  // after
  if ((attr.decoration & TextDecoration::Underline) != TextDecoration::None) ...
  ```

### Number layouts

Styles inside the prefix and suffix strings of `BasicNumberLayout` were used. They are now set
with `prefixStyle` and `suffixStyle` in its Blueprint.

## Widgets

### `ScrollCapsule`

No change in its API, but in its layout:

- `scrollX` and `scrollY` alone decide whether it scrolls along an axis. It scrolls with the wheel
  and `setViewOffset()` also without a scrollbar that can be shown.
- **Its minimum size along an axis it scrolls along is only that of the scrollbar,** or 0 without
  one, where it used to be at least the minimum of the child. A ScrollCapsule can now be much
  smaller than its content. Give it or its slot a minimum size if you relied on the old one.
- Scrollbars that can't be shown take no space, also when they have a track skin.
- With overlay scrollbars on both axes, the child gets a size. It used to stay 0x0.

### `InputHandler::setDoubleClickTresholds()`

It was declared inline and defined in the `.cpp`, so it couldn't be called from outside. It can
now. Its first argument is in **microseconds**: pass milliseconds × 1000.

### Debugger

`DebugFrontend::deactivate()` takes the capsules out of select mode, and `activate()` puts them
back. Workarounds for clicks that went missing after closing the debugger can go.

### LegacyGUI

- `LegacyRootCapsule::ReleaseAllChildren()` always returns true. `SetChild(nullptr)` and
  `DeleteChild()` on an empty capsule no longer crash.
- Code that still uses `TextTool` or `CharSeq` must include and link LegacyGUI.
- `Base::init()` no longer calls `TextTool::setDefaultBreakRules()`. `TextTool` sets up its own.

## Surfaces

### `Blueprint::buffered` is replaced by a restorer

`Surface::Blueprint::buffered` is removed (a compile error), and `restorer` is new:

```cpp
typedef std::function<bool(Surface* pSurface)> Restorer;
```

A surface of a GPU backend keeps a copy of its pixels in memory, **unless it has a restorer.**
With one, the copy isn't kept, and the restorer writes the content again when the backend has
lost it. A canvas never keeps a copy.

- `buffered = true` is now what you get by default. Delete the line.
- With the default `buffered = false`, GlSurface kept no copy. **A GL application that gives no
  restorer now uses about twice the memory for its surfaces.** Give a restorer to surfaces whose
  content you can write again: images from files, for example.

```cpp
// before
auto pSurf = pFactory->createSurface(WGBP(Surface, _.size = {256,256}, _.format = PixelFormat::ARGB_8, _.buffered = true));
// after, keeps its pixels
auto pSurf = pFactory->createSurface(WGBP(Surface, _.size = {256,256}, _.format = PixelFormat::ARGB_8));
// after, keeps no copy, writes them again when needed
auto pSurf = pFactory->createSurface(WGBP(Surface, _.size = {256,256}, _.format = PixelFormat::ARGB_8,
    _.restorer = [path](Surface* p) { /* write all pixels, e.g. allocPixelBuffer() + pullPixels() */ return true; }));
// after, content that is written every frame anyway
    _.restorer = Surface::declineRestore
```

A restorer returns true when it has written the whole content, or false to decline: then the
surface is transparent black and `isContentLost()` is true. What it captures must live as long as
the surface, or until `dropRestorer()`, which makes the surface decline from then on.

`isContentLost()` is now virtual, and goes back to false once the restorer has returned true or
the whole surface has been written by one `pullPixels()`. A surface made from the Blueprint of
another, with `convert()` for example, gets its restorer too.

Restorers are only called by DX12Backend, when it replaces its device. VulkanBackend stops on a
lost device, SdlGpuBackend isn't told of one, and GL and Metal don't lose textures. On those, a
restorer only decides what is kept in memory.

What a surface of a GPU backend keeps, by kind:

| Surface | Kept in memory | After a lost device | Reading pixels |
|---|---|---|---|
| no restorer | all pixels | sent again | in place |
| with a restorer | an alpha map, none if it is opaque | the restorer is called | read back from the GPU, waits for it |
| `Alpha_8`, `Index_8`, `Index_16` | all pixels, restorer or not | sent again | in place |
| canvas | nothing | its restorer, or cleared with `isContentLost()` | read back from the GPU, waits for it |

`SurfaceWriter::SaveInfo::buffered` is gone too. `.surf` files with the old flag set still load.

### Reading pixels needs `pushPixels()`

On a canvas, and on a surface with a restorer, `allocPixelBuffer()` gives memory for the rectangle
whose content is undefined until `pushPixels()` reads the texture back. `pullPixels()` copies it
into the texture: if you skipped `pushPixels()`, write all of the rectangle.

On breaking-changes, DX12Surface and MetalSurface gave pointers into pixels of their own, canvases
included, so reading without `pushPixels()` worked there. **It reads garbage now, without an
error.**

```cpp
auto pixbuf = pCanvas->allocPixelBuffer(rect);
pCanvas->pushPixels(pixbuf);            // required before reading
/* read pixbuf.pixels */
pCanvas->freePixelBuffer(pixbuf);
```

`alpha()` of a canvas reads an alpha map back after each render into it, so it costs more.

### Other changes

- `DX12Surface::create()`, and `VulkanSurface`, return null for a Blueprint they can't make a
  texture for. DX12Surface used to return a surface without a texture. Check what `create()` and
  the factories return.
- A `Blob` given to `create()` is copied, not taken over, and its pitch no longer has to be a
  multiple of 4.
- `fillColor` of the Blueprint is used by every GPU surface, VulkanSurface and SdlGpuSurface
  included. With `Color::Undefined`, the content is unspecified.
- **GlSurface:** `unload()`, `isLoaded()`, `reload()`, `totalTexturePixels()`,
  `totalBackingPixels()`, `paletteTextureWidth()` and `paletteTextureHeight()` are removed.
  `getPaletteTexture()` names a buffer texture of float4 now. Pixels are little-endian, as on the
  other GPU surfaces.
- **MetalSurface:** `unload()`, `isLoaded()`, `reload()` and `getPaletteTexture()` are removed.
  Use `sampleTexture()` and `paletteColors()`.
- **DX12Surface:** `paletteBuffer()` is removed. `isIndexed()`, `mipLevels()`, `isMipmapStale()`,
  `clearMipmapStale()` and `notifyRendered()` are in the base class `GpuSurface` now, and can be
  called as before.

## Backends

All GPU backends (OpenGL, DX12, Metal, Vulkan, SDL GPU) derive from `GpuBackend`, which derives
from `Backend`. That changes nothing for code that uses them. Errors that came once a process come
once per backend, so a warning can come once per window.

### GlBackend

- **The default canvas is the framebuffer that is bound for drawing when `beginRender()` runs.**
  It used to bind framebuffer 0, the window. An application that leaves a framebuffer of its own
  bound when it renders now gets the widgets drawn into that. Bind 0 first if you want the window:
  `glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);`. `endRender()` binds again what was bound at
  `beginRender()`.
- To draw widgets on top of a frame of your own: bind your framebuffer, draw, mark the root panel
  dirty all over (`addDirtyPatch()`), and render it.
- No shaders are compiled by `create()`. Each program is compiled the first time it is needed,
  which can be a small hitch. `create()` only fails below OpenGL 3.3.
- `shaderBlob()` builds a blob of the programs compiled so far, so call it at shutdown, not after
  `create()`. It needs program binaries (OpenGL 4.1). Blobs saved on breaking-changes are
  rejected, and the programs are compiled again.
- The argument of `create(int uboBindingPoint)` isn't used.

### DX12Backend

- The shaders are compiled bytecode. **An application that set up DX12Backend itself no longer
  needs `D3DCompiler.lib` and `d3dcompiler_47.dll`.** Startup no longer waits for shaders.
- `setDefaultCanvas()` takes a resting state of the buffer, after the size and scale, defaulting
  to `D3D12_RESOURCE_STATE_PRESENT`. Pass `D3D12_RESOURCE_STATE_RENDER_TARGET` if you render into
  the back buffer first and present from that state. That has not been run.
- Replacing the device works as before: `setDevice(nullptr, nullptr)`, then `setDevice(device,
  queue)`, then `setDefaultCanvas()` on every backend, and redraw everything. New: the second
  `setDevice()` calls the restorers.
  - A surface with a restorer gets it called even if the old device still worked.
  - A canvas without a restorer is read back if the old device works, else cleared with
    `isContentLost()`.
  - **A canvas created with pixels or a `fillColor` and never rendered into comes back cleared,**
    with `isContentLost()`. It used to get its pixels back.
  - A surface whose restorer declined is cleared, with `isContentLost()`: write it again, as a
    lost canvas.

  `_recoverDevice()` in `wonderapp/source/sdl3/backend_dx12.cpp` does all of it.

### MetalBackend

- `setMetalDevice()` takes the command queue you commit to, optionally:
  `setMetalDevice(device, queue)`. Pass it if you draw into the same drawable, so the order of the
  command buffers is defined. `metalCommandQueue()` is new.
- The shaders are compiled when first needed, as Metal Shading Language 2.3. **That needs macOS 11
  or later when the application runs.**
- A MetalSurface sends its pixels to its texture when it is used. An application that writes
  surfaces every frame hasn't been tried.

### SoftBackend and LinearBackend

What they draw changed in places, fixing what was wrong:

- Surfaces of any size tile correctly. Before, sizes that weren't a power of two tiled at the
  nearest smaller power of two, with a warning.
- Straight tiles were wrong when the width wasn't a multiple of the tile, and off by a pixel for
  negative offsets.
- Flipped, rotated and scaled blits with `SampleMethod::Nearest` no longer read the wrong pixel at
  exact pixel borders.

### VulkanBackend (new)

`wax/extensions/vulkangfx/wax_vulkanbackend.h`, target `waxvulkangfx`. It needs Vulkan 1.3 with
`dynamicRendering`, `synchronization2`, `shaderDemoteToHelperInvocation`, `timelineSemaphore` and
`dualSrcBlend` enabled, and the extensions `VulkanBackend::requiredDeviceExtensions()` returns.

```cpp
VulkanBackend::setDevice(instance, physicalDevice, device, queue, queueFamily);   // before any backend or surface
auto pBackend = VulkanBackend::create();
Base::setDefaultSurfaceFactory(pBackend->surfaceFactory());                        // and the edgemap and warpmap factories

pBackend->setDefaultCanvas(image, view, format, restingLayout, sizeSPX, scale);   // per window and resize

// Between beginRender() and endRender(), which RootPanel::render() calls:
//   pBackend->commandBuffer() to add commands of your own after the last session,
//   pBackend->addWaitSemaphore(sem, stage), pBackend->addSignalSemaphore(sem).

// At shutdown, when everything is released:
VulkanBackend::setDevice(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 0);
```

Images of a swap chain don't keep their content, and only what changed is drawn: render into an
image of your own and copy it to the swap chain, as `wappsdl3vk` does, or draw everything each
frame. A lost device stops it for good. It has only run on Linux.

### SdlGpuBackend (new)

`wax/extensions/sdlgpugfx/wax_sdlgpubackend.h`, target `waxsdlgpugfx`, on the GPU API of SDL3,
which picks Vulkan, D3D12 or Metal.

```cpp
SDL_GPUDevice* pDev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, debug, nullptr);
SdlGpuBackend::setDevice(pDev);              // before any backend or surface, setDevice(nullptr) when all are gone
auto pBackend = SdlGpuBackend::create();
pBackend->setDefaultCanvas(texture, format, sizeSPX, scale);
```

A swap chain must be sRGB (`SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR`): UNORM is taken as linear
and comes out too dark.

By default, each session goes in a command buffer of the backend's own, so the default canvas
must be a texture of yours, which you then copy to the window. To draw straight into the window,
or on top of a frame of your own, give it your command buffer:

```cpp
SDL_GPUCommandBuffer* pCmd = SDL_AcquireGPUCommandBuffer(pDev);
SDL_WaitAndAcquireGPUSwapchainTexture(pCmd, pWindow, &pTex, &w, &h);
// your own render passes, all ended
pBackend->setDefaultCanvas(pTex, SDL_GetGPUSwapchainTextureFormat(pDev, pWindow), {w*64, h*64}, scale);
pBackend->setCommandBuffer(pCmd);
pRoot->addDirtyPatch(size);                  // a swap chain texture keeps nothing, draw it all
pRoot->render();
pBackend->setDefaultCanvas(nullptr, format, size, scale);
SDL_SubmitGPUCommandBuffer(pCmd);
```

Only the pass on the default canvas goes in your command buffer. The backend uses uniform slots 0
to 3 of both stages: push yours again if you draw after it. SDL GPU has no dual source blending,
so `Replace` and `Morph` are drawn in two passes, and pixels where triangles meet can differ a
little. `WAPP_GPU_DIRECT=1` in `wappsdl3gpu` shows all of it working.

## Warpmaps

Breaking-changes already has the triangle mesh Warpmaps, the warp generators and the precision
that is a quality factor, so a project at its latest commit only needs the renames above for warp
code: `wax_` headers, `wax::Base::setDefaultWarpmapFactory()` where it said `GfxBase::`, and the
C API.

New on v4: `PerspectiveWarp` (the source on a plane seen in perspective, with
`PerspectiveWarp::rotated()` for a rotated rectangle), `BallWarp` (a ball covered by the source)
and `LensWarp` (a round lens magnifying what is under it). SdlGpuBackend and VulkanBackend draw
Warpmaps too.

## WonderApp

How an application links a framework is unchanged: `${WAPP_INCDIRS}` and `${WAPPLIB}`. The API in
`wonderapp.h`, `wappwindow.h` and `wappwindowapi.h` is the same, apart from the renames of Wax.
`Blob`, `Surface` and `SurfaceFactory` are forward-declared in `namespace wax` in `wappapi.h`.

### Frameworks

New values of `WAPPLIB` (none were removed):

| `WAPPLIB` | Renders with | Notes |
|---|---|---|
| `wappsdl3sw`, `wappsdl3gl` | software, OpenGL | `wappsdl3gl` runs on Wayland, `wappsdl2gl` shows nothing there |
| `wappsdl3vk` | Vulkan | needs the Vulkan SDK |
| `wappsdl3dx12` | DX12 | Windows only, recovers from a lost device |
| `wappsdl3metal` | Metal | macOS only |
| `wappsdl3gpu` | SDL GPU | `SDL_GPU_DRIVER` picks Vulkan, D3D12 or Metal |
| `wappcocoasw`, `wappcocoametal` | software, Metal | AppKit without SDL, macOS only |

The SDL3 frameworks size windows in points, with a canvas the size of the window in pixels, so a
window can get another size than on `wappsdl2*` on a scaled display.

### Behaviour changes

- **`loadSurface()` uses stb_image on every framework.** Images with a palette (8-bit PNG, GIF)
  come back as `XRGB_8` or `ARGB_8`, where SDL_image gave `Index_8`. Grey images do too. Formats
  are those of stb_image: PNG, JPEG, BMP, GIF, TGA, PSD, HDR, PIC and PNM. QOI, SVG, TIFF, WebP,
  AVIF and others that SDL_image read are gone.
- **Win32:** `programArguments()` returns the arguments, where it returned nothing.
  `openLibrary()`, `loadSymbol()` and `closeLibrary()` work, where they were stubs. The default
  edgemap and warpmap factories are set, where creating an edgemap or a warp without a factory
  crashed.
- `loadSurface()` gives no restorer, so loaded images keep their pixels on all backends.

Known limitations on v4: the SDL frameworks ignore SIGTERM and SDL_QUIT, so an application only
quits when its windows are closed or `update()` returns false, and `Window::setSize()`,
`setPos()` and `setGeo()` don't change the window.

## C API, plugins and the C# wrapper

### C API

Every `wg_` function and type of the graphics library is `wax_`, and every `WG_` constant is
`WAX_`. The headers are `wax_c_X.h`, in `wax/c-api`. Besides that rule:

| breaking-changes | v4 |
|---|---|
| `wondergfx_c.h`, `wondergear_c.h` | `wax_c.h` |
| `wg_c_gfxbase.h`, `wg_c_gearbase.h` | `wax_c_base.h` |
| `wg_c_gfxtypes.h`, `wg_c_geartypes.h` | `wax_c_types.h` |
| `wg_c_gfxbackend.h`, `wg_c_gfxdevice.h`, `wg_c_gfxutil.h` | `wax_c_backend.h`, `wax_c_device.h`, `wax_c_util.h` |
| `wondergfxstream_c.h` | `waxgfxstream_c.h` |
| `wg_initGfxBase()`, `wg_initGearBase()` | `wax_initBase()` |
| `wg_exitGfxBase()`, `wg_exitGearBase()` | `wax_exitBase()` |
| `wg_isGfxBaseInitialized()`, `wg_isGearBaseInitialized()` | `wax_isBaseInitialized()` |
| `wg_setDefaultGfxDevice()`, `wg_defaultGfxDevice()` | `wax_setDefaultDevice()`, `wax_defaultDevice()` |
| `wg_createGfxDevice()` | `wax_createDevice()` |
| `wg_setPrinterGfxDevice()`, `wg_printerGfxDevice()` | `wax_setPrinterDevice()`, `wax_printerDevice()` |
| `wg_surfaceBP.buffered`, `WG_SAVE_BUFFERED_FLAG` | removed; `wax_surfaceBP` has `restorer` and `restorerData` |

New: `wax_surfaceIsContentLost()`, `wax_declineSurfaceRestore()`, `wax_dropSurfaceRestorer()`.

Functions that create objects return NULL on failure, where they crashed: creating a FreeType
font without a blob, a surface from a missing glyph, reading a surface from bad data, and creating
edgemaps, surfaces, mirror surfaces and canvas layers from an invalid Blueprint. Check for NULL.

WonderGUI's own C API (`wg_c_types.h`, `wg_c_hostbridge.h`, in `wondergui/extensions/plugin`)
keeps its `wg_` prefix, and uses `wax_obj`.

### Plugins

`wax_surfaceBP` has another layout, and the surface calls of the plugin interface have two more
at the end (`surfaceIsContentLost`, `dropSurfaceRestorer`). **Hosts and plugins must be rebuilt
together.** A new plugin refuses to start in an old host.

- A restorer in the Blueprint of a plugin's surface is passed on, and the host calls it.
  `PluginSurface::isContentLost()` asks the host.
- `PluginSurfaceFactory` returns null when the host can't create the surface. It returned a
  surface around null.
- `userData` and `finalizer` in the Blueprint of a plugin's surface are lost. Set them on the
  surface afterwards.

### C# wrapper

It is in `wax/wrappers/c-sharp/Wax/Wax.csproj`, `namespace Wax`. `GfxBase` is `Base`, `GfxBackend`
is `Backend`, `GfxDevice` is `Device`, `Printer.SetGfxDevice()` is `SetDevice()` and
`SetDefaultGfxDevice()` is `SetDefaultDevice()`. `Surface.IsContentLost()` and
`Surface.DropRestorer()` are new. It doesn't build on v4, nor on breaking-changes (see
KNOWN_ISSUES.md).

## Changes that compile but behave differently

The compiler finds none of these. Go through them after your code compiles.

| Change | Effect | Section |
|---|---|---|
| Text offsets are bytes | wrong positions in text that isn't ASCII | [Text](#offsets-are-bytes) |
| `text().length()` and `StaticText::length()` are bytes | counts are too high for text that isn't ASCII | [Text](#offsets-are-bytes) |
| `TextDecoration` is a bitmask | stored values change meaning, `==` fails for combinations, stacked styles combine | [Text](#textdecoration-is-a-bitmask) |
| `PlainTextLayout` draws underlines and moves by word | text looks and behaves differently | [Text](#basictextlayout-is-plaintextlayout) |
| `TextStyle` fix for an undefined `backColor` | text color can change | [Text](#textstyle) |
| `Color8` from a named `Color` before `Base::init()` | transparent black | [Colors](#the-named-colors-are-in-color-at-12-bits) |
| `auto c = Color::Red` is a `Color` | 12-bit channels read as 8-bit | [Colors](#the-named-colors-are-in-color-at-12-bits) |
| ScrollCapsule's minimum size | layouts shrink | [Widgets](#scrollcapsule) |
| GPU surfaces keep their pixels without a restorer | more memory, about twice for GL | [Surfaces](#blueprintbuffered-is-replaced-by-a-restorer) |
| Reading a canvas without `pushPixels()` | garbage pixels | [Surfaces](#reading-pixels-needs-pushpixels) |
| DX12: a canvas never rendered into, after a lost device | comes back cleared | [Backends](#dx12backend) |
| DX12Surface and VulkanSurface return null for a bad Blueprint | a crash if not checked | [Surfaces](#other-changes) |
| GlBackend draws into the framebuffer that is bound | widgets in the wrong place | [Backends](#glbackend) |
| GL programs compiled when first needed | a small hitch the first time | [Backends](#glbackend) |
| MetalBackend needs macOS 11 | pipelines fail on older macOS | [Backends](#metalbackend) |
| SoftBackend tiling and nearest blits fixed | output changes where it was wrong | [Backends](#softbackend-and-linearbackend) |
| `loadSurface()` with stb_image | no `Index_8` surfaces, fewer formats | [WonderApp](#behaviour-changes) |
| SDL3 frameworks size windows in points | other window sizes on scaled displays | [WonderApp](#frameworks) |
| C API creation functions return NULL | a crash if not checked | [C API](#c-api) |
| Class names of `TYPEINFO`: `Device`, `Backend`, `Base`, ...; error messages say "Wax" | code comparing these strings | [Wax](#classes-and-methods-without-the-gfx-prefix) |
| `wax::Base::exit()` warns only about real leaks | no warning on every exit | [Wax](#initialization) |
