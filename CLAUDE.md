# WonderGUI

## Keep the project's memory up to date

- **KNOWN_ISSUES.md** lists bugs and limitations we know about. Read it before working in
  an area. Add an entry when you find a bug you don't fix, and remove the entry in the
  same commit as the fix.
- **ROADMAP.md** lists planned changes and the design decisions behind existing code.
  Add decisions made in a session there, so they don't only exist in that chat.

## Layout

- `wax/` - graphics base library and its extensions (soft, OpenGL, DX12, Metal backends, FreeType fonts, streaming).
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

## Conventions

- Files should use LF line endings. Make corrections as you discover.
- Tab width is 4 and we use tabs for indentation. Make corrections as you discover.
- Blueprint struct members are in alphabetical order.

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

## WonderGUI Design

- pts (short for points) is the primary measure unit for the external API and is by default float. Plan is to swap it to integers on low end systems where we don't use scaling and pts = pixels.
- Skins, TextLayouts, TextStyles are immutable objects once created.
- A widget provides the functionality, but the look and interaction feedback is decided by chosen Skin, TextStyle and TextLayout.
- Low end target is the same as WAX, so no reliance on FPU and be cautious with memory in the base system. Individual non-essential widgets and other classes are allowed to break this rule.

