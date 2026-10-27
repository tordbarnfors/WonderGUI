---
title: 'Resources'
weight: 20
BookToC: true
---

# Resources

*Objects that are used by the widgets but not widgets themselves are collectively referred to as resources. They are reference counted and often immutable.*



## Immutable vs modifiable

Most resources in WonderGUI are *immutable*, meaning that once they have been created they can not be modified. This is by design since allowing common resources shared by many widgets to be updated would require expensive book-keeping to make sure any widget affected is updated accordingly.

Some objects, i.e. Surfaces, are *semi-mutable* in that you  can change their content (i.e. their pixel values) while their  attributes are immutable (e.g. size, pixel format etc). These should be  treated with care, understanding that changing the pixel data of a  surface will not automatically update any widget or other resource  depending on it.

Widgets are never immutable or semi-mutable, they can all be manipulated at all times.



## Handling of immutable resources

Although immutable resources can be created anytime during the program cycle, the  intended use is to create them once (like during program start) and have widgets share them. Widgets can swap one for another when needed, but should rarely have new ones created specifically for them.

A small example to clarify how resource should be handled:

```c++
*** RIGHT WAY! ***

	// At initialization
	auto pWhiteSkin = ColorSkin::create( Color::White );					
	auto pBlackSkin = ColorSkin::create( Color::Black );
										
	// Later on changing color of a widget.
	pWidget->setSkin( bIsWhite ? pWhiteSkin : pBlackSkin );
	
*** WRONG WAY! ***

	pWidget->setSkin( bIsWhite ? ColorSkin::create( bIsWhite ? Color::White : Color::Black ) );
```

Doing it the wrong way will create and destroy resources repeatedly. The cost in this case is negligible but doing the same in other places could result in the unnecessary creation of a large amount of resources, increasing memory consumption and decreasing performance. 

There is also a hard limit on some resources, e.g. there can be no more than 65535 TextStyle objects at any given time. Creating a new TextStyle for each entry in a table could make you quickly reach this limit.

Creating immutable resources on the fly might be the right thing to do in some cases, but you should always stop and think before doing it.



## List of resource classes

The following table lists the classes that are considered resources with a short description to give you a quick overview.

| Class                | Modifiable   | Description                                                  |
| :------------------- | ------------ | :----------------------------------------------------------- |
| Blob                 | Content only | A blob is for storing arbitrary binary data as a WonderGUI object with smart pointers and referrence counting. |
| Caret                | No           | A text cursor. Basic implementation of a blinking straight line cursor, but can be subclassed to custom designs. |
| CircularWarp         | Yes          | Generator of a Warpmap that bends the source around a circle, such as a gauge or progress ring from a straight bar. See [Warpmaps](warpmaps). |
| Dataset              | Content only | Template class for wrapping any class into a WonderGUI object with smart pointers and reference counting. |
| Edgemap              | Content only | Custom class storing graphics as a number of edges between fields of different colors. Used to store simple shapes such as graphs, charts, circles etc in a generic format for rendering. Edgemap is a base class which is subclassed depending on rendering output such as Software, OpenGL, Metal and Streaming. |
| EdgemapFactory       | No           | A simple factory class for creating edgemaps. Subclassed into factories for various rendering outputs. |
| Font                 | No           | Stores a font used for rendering text. Subclasses exist for simple bitmap fonts and vector fonts using FreeType for rendering. |
| NumberLayout         | No           | NumberLayout manages the layout and rendering of the textual representation of a value. Like TextLayout but for numerical values instead of strings of text. |
| Printer              | Yes          | A low-level class for printing text to the screen as part of the wondgfx library. Meant for situations where WonderGUI's more sophisticated text layout system would be overkill. Not used by any WonderGUI widgets, but is available nevertheless. |
| Skin                 | No           | Defines the look and feel of a widget (or a part thereof) and draws it. Contains information about margins, padding, graphics and colors and any changes to appearance when interacting with it. |
| Surface              | Content only | Bitmap surfaces containing graphics resources for rendering or serving as rendering output. Surface is a base class which is subclassed depending on rendering output such as Software, OpenGL, Metal och Streaming. |
| SurfaceFactory       | No           | A simple factory class for creating surfaces. Subclassed into factories for various rendering outputs. |
| SurfaceFileInspector | No           | Class for reading header information from a .wbm-file, containing graphics in WonderGUI specific format. |
| SurfaceReader        | No           | Class for reading surfaces from WonderGUI's custom graphics file format. |
| SurfaceWriter        | No           | Class for writing surfaces to WonderGUI's custom graphics file format. |
| TextLayout           | No           | TextLayouts manages the layout and rendering of text, such as the texts placement and alignment within a rectangle. They also manage text-wrapping and can theoretically handle animated text, rotated text and bi-directional text, although just a basic TextLayout subclass has been implemented so far. |
| TextStyle            | No           | TextStyle objects specify the font, size, color and decorations to use for some text or parts thereof. |
| Transition           | No           | Specifies a transition from one color, position size or such to another. Subclassed into classes for dealing specifically with colors and arrays of values. |
| WarpGenerator        | Yes          | Base class for generators that make and keep a Warpmap from a few settings, such as CircularWarp. See [Warpmaps](warpmaps). |
| Waveform             | Content only | Class for storing a "waveform", a set of samples that represents the curve of a graph, oscilloscope or similar. Waveforms are turned into edgemaps for rendering. |
| Warpmap              | Content only | Bends a surface or an edgemap into another shape when drawn, by a grid of positions in the source. Subclassed depending on rendering output, like Edgemap. See [Warpmaps](warpmaps). |
| WarpmapFactory       | No           | A simple factory class for creating warpmaps. Subclassed into factories for various rendering outputs. |

