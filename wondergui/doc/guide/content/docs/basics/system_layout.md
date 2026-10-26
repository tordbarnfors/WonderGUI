---
title: 'System Layout'
weight: 5
---

# System Layout

*In this section we take a look at WonderGUI as a whole and how it is divided into separate modules.*



## Introduction

WonderGUI has a modular foundation, allowing you to include only parts of the system in your project. There is a base system and extensions that are compiled into separate libraries, providing additional functionality.



## A layered base system

The base system is built as four layers that can be compiled into separate libraries, but usually they are all built into one single WonderGUI-library. Each layer only depends on lower layers so the top layers can be shaved off if their functionality is not desired. That way the core aspects of WonderGUI, such as the object model or the rendering engine can be used in projects without including the full GUI-system.

The layers, from bottom to top are the following:

1. **WonderGear** - This contains the fundamental object model with smart pointers and reference counted objects. It also contains some classes for memory management. It's a tiny library for any project that wants to use the object model.
2. **WonderGfx** - This layer deals with all the low-level aspects of drawing and buffer graphics, as well as classes for loading and saving images (in the few formats supported directly by WonderGfx), the font system and a simple system for printing text.  If you are not interested in a GUI, but wants a cross-platform, efficient library for drawing simple 2D-graphics and manipulating images, you can skip the rest or build it yourself on top of WonderGfx.
4. **WonderGUI** - The GUI library itself, providing all the rest such as widgets, skins, advanced text handling, event handling and user input.

All layers use the same namespace 'wg' (hence the naming of them), so you won't notice this layered approach in your code even when you use the full stack. Initializing one layer will automatically initialize the layers below so you can remain oblivious to this layout in your code. You will only notice it when you look at the file structure of WonderGUI since each layer is compartmentalized into its own folder.

## Extensions

Any part of WonderGUI that is not necessary for the rest to work or is platform dependent is provided as an extension. These are built as separate libraries and will need to be linked to your project individually.

Here is a short description of some of the extensions:

- **MetalGfx** - A WonderGfx extension that provides hardware accelerated rendering through Apple's Metal API.
- **OpenGLGfx** - A WonderGfx extension that provides hardware accelerated rendering through the OpenGL API.
- **GfxStream** - A WonderGfx extension that enables you to encode drawing commands into a binary stream that can be stored and transmitted to another device for rendering.
- **SoftGfx** - A WonderGfx extension for CPU-based software rendering. Slow and can be quite memory hungry (depending on configuration) but highly portable and reliable and works almost anywhere.
- **FreetypeFont** - A WonderGfx extension for using vector fonts through the FreeType library.
- **LegacyGUI** - A WonderGUI extension that provides support for building legacy software written for an earlier, deprecated version of WonderGUI. Should not be used in any new projects.

