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

- Files use both LF and CRLF line endings. Keep each file's existing endings when editing.
- Blueprint struct members are in alphabetical order.
