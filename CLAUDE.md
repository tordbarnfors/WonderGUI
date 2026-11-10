# WonderGUI

## Keep the project's memory up to date

- **KNOWN_ISSUES.md** lists bugs and limitations we know about. Read it before working in
  an area. Add an entry when you find a bug you don't fix, and remove the entry in the
  same commit as the fix.
- **ROADMAP.md** lists planned changes and the design decisions behind existing code.
  Add decisions made in a session there, so they don't only exist in that chat.

## Documentation

- The manual is in `wondergui/doc/guide/content/docs/` (Hugo): `basics/` for the object model,
  blueprints, screen geometry and surfaces, `widgets/` for states, containers and layout,
  `resources/` for skins, surfaces and warpmaps. Read the page for an area before working in it.
- `basics/blending_and_transparency.md` is the definition all graphics backends must follow,
  not just a description. Change it together with the backends.
- Parts of the guide were written before the Wax split and may still lag behind, so trust the
  code where they differ.
- Read `wondergui/widgetkit-src/PIPELINE.md` before touching widget kits.

## Layout

- `wax/` - graphics base library and its extensions (soft, OpenGL, DX12, Metal, Vulkan backends, FreeType fonts, streaming). The GPU backends (OpenGL, DX12, Metal, Vulkan) share the HLSL shaders in `wax/extensions/hlsl` and the API independent part of a GPU backend in `wax/source/wax_gpubackend.h` (and `wax_gpuedgemap.h`). `c-api/` is compiled into the `wax` library, so changing the Wax API usually means updating it too. `wrappers/` has the C# wrapper.
- `wondergui/` - the UI toolkit: `source/` (widgets, components, text layouts), `extensions/` (legacy GUI, debugger, plugin), `tools/`, `widgetkits/`.
- `wonderapp/` - application frameworks (SDL2/SDL3, Win32, Cocoa) and tools built on them.
- `thirdparty/` - vendored third-party code.

## Building and testing

- CMake root is the repository root. Pick the app framework with `-DWAPPLIB=<name>`
  (see the list in `CMakeLists.txt`), e.g. `wappsdl2sw` for software rendering on Linux.
- In a Linux container without X11 development packages, SDL3 needs
  `-DSDL_X11_XCURSOR=OFF -DSDL_X11_XINPUT=OFF -DSDL_X11_XFIXES=OFF -DSDL_X11_XRANDR=OFF
  -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XSHAPE=OFF -DSDL_X11_XTEST=OFF` to configure.
- `unittester` runs from the repository root (it loads `resources/`). Pass part of a
  collection name to run only those tests, e.g. `unittester TextSystem`; the full suite
  includes some slow shape tests.
- Tools and examples also load `resources/`, so run them from the repository root too.
- `wappsdl3vk` renders with Vulkan. Headless under `xvfb-run` it presents with lavapipe only, as
  Xvfb has no DRI3; `WAPP_VK_DEVICE=Intel` renders on the GPU without presenting, which is enough
  for `gfxdevicetester --accuracy`. `WAPP_VK_VALIDATION=1` turns on the validation layer.
- After changing a shader in `wax/extensions/hlsl`, run `wax/extensions/vulkangfx/compile_shaders.py`
  (needs DXC), `wax/extensions/metalgfx/compile_shaders.py` and `wax/extensions/openglgfx/compile_shaders.py`
  (also need spirv-cross, and glslangValidator checks the GLSL), and regenerate
  `wax/extensions/dx12gfx/wax_dx12shaders.cpp`, see `wax/extensions/hlsl/README.md`.
- `gfxdevicetester` compares the native device of the selected `WAPPLIB` against the software
  reference. Use it when changing a graphics backend or blending. `widgetbench` is for looking at
  widgets, skins and layouts (see its `README.md` for adding a test), `workbench` for trying things out.
- Source files are listed explicitly in each `CMakeLists.txt`, so new files must be added there.
  New public headers also go in `wax/source/wax.h` or `wondergui/wondergui.h`.
- `tuxbuild/` is the usual local build directory and is git-ignored.

## Conventions

- Files should use LF line endings. Make corrections as you discover.
- Tab width is 4 and we use tabs for indentation. Make corrections as you discover.
- Blueprint struct members are in alphabetical order.
- Wax, WonderGUI and their extensions are C++17. WonderApp and most tools are C++20, so designated initializers only work there. Library code fills in blueprints with `WAXBP()` (Wax) or `WGBP()` (WonderGUI).
- Files are named `wax_` or `wg_` plus the lower case class name.
- Members are prefixed `m_`, with `m_b` for bools and `m_p` for pointers. Pointer variables are prefixed `p`. Methods starting with `_` are internal, even when public.
- Objects are created through a static `create()`, with constructors protected. Each class has `Foo_p`/`Foo_wp` typedefs for `StrongPtr`/`WeakPtr` and a `TYPEINFO`. Never point `std::shared_ptr` or similar at an `Object`.
- Don't edit generated files: `wondergui/widgetkits/wg_glossyblue.h` and `resources/glossyblue_skinblocks.png` come from `wondergui/widgetkit-src/`.

- LegacyGUI is only there to make old code downstream still compile. No new features goes here unless specifically requested.



## WAX Design

- Meant to run on anything from 68030-based machines with 16-bit graphics, a few megabytes of memory and no FPU, up to modern high end machines with GPU acceleration.
- spx (short for subpixel) is the primary measure unit for the API and is 1/64 pixel. Pixel for surface, edgemap and warpmap sizes.
- All code should be big/little endian agnostic.
- Avoid floating point calculations and only use floats in API where it makes a lot of sense and costs little. Floats in range 0.0-1.0 are more acceptable.
- Minimal reliance on external libraries. Some std libraries are already accepted.
- Avoid memory trashing.
- Content of Bitmaps, Edgemaps and Warpmaps can be modified while being in flight for render, with consequences. This is accepted per design.
- origin for drawing operations are top/left corner.
- Drawing is deferred: draws are rendered when the canvas update ends and read their Edgemaps and Warpmaps then.
- Colors given to the API are sRGB, pixels are stored with straight alpha and blending happens in linear light. `ReferenceBackend` (`wax/extensions/softgfx`) implements the rules as directly as possible and is what other backends are tested against.
- Pixel formats only describe the channels, named in register order starting with the most significant bits. Color space and byte order are separate properties of a surface.
- Each backend has its own Surface, Edgemap and Warpmap subtypes. Code that shouldn't depend on the backend creates them through the factories of the device.
- Surfaces, Edgemaps and Warpmaps are semi-mutable: their properties are fixed when created, but their content can change.

## WonderGUI Design

- pts (short for points) is the primary measure unit for the external API and is by default float. Plan is to swap it to integers on low end systems where we don't use scaling and pts = pixels.
- Skins, TextLayouts, TextStyles are immutable objects once created.
- A widget provides the functionality, but the look and interaction feedback is decided by chosen Skin, TextStyle and TextLayout.
- Low end target is the same as WAX, so no reliance on FPU and be cautious with memory in the base system. Individual non-essential widgets and other classes are allowed to break this rule.
- `wondergui/source/base/wg_wax.h` decides which Wax names are part of `wg`, with some renamed (`wax::Device` is `wg::GfxDevice`). Wax extensions stay in `wax`. Forward-declare Wax classes in namespace `wax`, never in `wg`.
- Widget internals use spx, including the virtuals to override (`_render()`, `_resize()`, `_defaultSize()`, `_receive()`, `_setState()`, `_alphaTest()`). Scale is the number of spx per pt, 64 by default. It is set on the root and can be changed for a branch with a ScaleCapsule.
- Other immutable resources, like PackLayout, Font, Caret, NumberLayout and Transition, work the same way. Resources are meant to be created once and shared, not created per widget or on the fly. There is also a hard limit of 65535 TextStyles.
- Widgets can't have their state set directly, only disabled and flagged are controlled from outside. There are 8 state properties forming 72 valid states, and a state without its own value falls back by word order in its name (see `widgets/widget_states.md` in the guide). Pressed implies Hovered.
- Containers expose their slots as public members (`slot`, `slots`, `mainSlot`). Slots can't be created on their own, they are added by pushing a widget, optionally with a slot Blueprint.

