---
title: 'System Layout'
weight: 5
---

# System Layout

*In this section we take a look at WonderGUI as a whole and how it is divided into separate modules.*



## Introduction

WonderGUI has a modular foundation, allowing you to include only parts of the system in your project. It is built on top of a graphics library called Wax and both have extensions that are compiled into separate libraries, providing additional functionality.



## Two base libraries

The base system is built as two separate libraries, neither of which needs any external libraries. WonderGUI depends on Wax, but Wax doesn't depend on WonderGUI, so the core aspects of the system, such as the object model or the rendering engine can be used in projects without including the GUI-system.

The libraries, from bottom to top are the following:

1. **Wax** - This contains the fundamental object model with smart pointers and reference counted objects and some classes for memory management. It also deals with all the low-level aspects of drawing and buffer graphics, as well as classes for loading and saving surfaces, the font system and a simple system for printing text. If you are not interested in a GUI, but want a cross-platform, efficient library for drawing simple 2D-graphics and manipulating images, you can use Wax on its own or build your own system on top of it.
2. **WonderGUI** - The GUI library itself, providing all the rest such as widgets, skins, advanced text handling, event handling and user input.

Wax uses the namespace 'wax' and WonderGUI the namespace 'wg'. The Wax classes that are part of WonderGUI's API are also brought into the wg namespace, some of them under a different name (wax::Device is wg::GfxDevice), so you can work with wg::Surface, wg::GfxDevice and so on without having to think about which library they come from. The header wg_wax.h lists them. Initializing WonderGUI with wg::Base::init() will automatically initialize Wax as well.

## Extensions

Any part of the system that is not necessary for the rest to work or is platform dependent is provided as an extension. These are built as separate libraries and will need to be linked to your project individually.

The extensions to Wax use the namespace 'wax' and are not brought into the wg namespace, so they are referred to as wax::SoftBackend and so on.

Here is a short description of some of the extensions:

- **DX12Gfx** - A Wax extension that provides hardware accelerated rendering through the DirectX 12 API.
- **MetalGfx** - A Wax extension that provides hardware accelerated rendering through Apple's Metal API.
- **OpenGLGfx** - A Wax extension that provides hardware accelerated rendering through the OpenGL API.
- **GfxStream** - A Wax extension that enables you to encode drawing commands into a binary stream that can be stored and transmitted to another device for rendering.
- **SoftGfx** - A Wax extension for CPU-based software rendering. Slow and can be quite memory hungry (depending on configuration) but highly portable and reliable and works almost anywhere.
- **FreetypeFont** - A Wax extension for using vector fonts through the FreeType library.
- **Debugger** - A WonderGUI extension that provides a debug window for inspecting a running application.
- **LegacyGUI** - A WonderGUI extension that provides support for building legacy software written for an earlier, deprecated version of WonderGUI. Should not be used in any new projects.
- **Plugin** - A WonderGUI extension for building plugins as shared libraries and hosting them in an application.

