---
title: 'Surfaces'
weight: 30
---

# Surfaces

Surfaces are containers of bitmap graphics (or textures) and are a central part of WonderGUI.

They are used both as sources of graphics fragments that builds up the GUI and as canvases to which the GUI (or parts thereof) is rendered.

Each graphics backend (OpenGL, Metal, software etc) has its own Surface subtype which needs to be used with that backend.

Typically you use a *GfxDevic*e to copy graphics from one surface to another where the source can be stretched, rotated and tinted before being blended onto the destination, but there are also a few simple methods for clearing a surface, copy graphics between surfaces and directly manipulating pixels in a surface. Besides being convenient, these methods are also useful when you want to convert between pixel formats or surface subtypes. See [working with surfaces](../basics/working_with_surfaces) for more information about these methods. 

Surfaces are *semi-mutable* in the sense that the properties of the surface (like width, height and pixel format) can not be changed once created, but the content of the surface can. This includes the palette if surface is indexed.

Surfaces can be created either directly through their create() methods or by using a *SurfaceFactory*. The advantage of using a SurfaceFactory is that you don't need to know the specific subtype of Surface to create where you create it, otherwise it is essentially the same.

You can get a callback when a surface is modified by adding an Observer.



## Properties

| Property        | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| baggage         | Any object associated with the surface. Can be retrieved using baggage(). |
| bigEndian       | Byte order of the pixels. Defaults to the byte order of the system. |
| buffered        | Set to true if a copy in local system memory should be kept to allow for quick reading and writing of pixel data. Might affect performance depending on surface subtype. Defaults to false. |
| canvas          | Must be set to true if surface might be used as canvas by the GfxDevice. This might affect performance. Defaults to false. |
| colorSpace      | Color space of the pixels, sRGB or Linear. Defaults to sRGB. Colors given to WonderGUI are always sRGB, while blending always is done in linear space. |
| finalizer       | A function that is called right before the Skin is destroyed. |
| palette         | A pointer at an array of Color8 objects used as the palette if pixel format is indexed or bitplanes. Mandatory if pixel format is indexed or bitplanes. Defautls to nullptr. |
| paletteCapacity | Allows for setting the size of the palette to a larger value than paletteSize for later adding of colors. No pixel index may have a larger value than paletteCapacity. Defaults to the value of paletteSize. |
| paletteSize     | Number of color entries initially specified for the palette. Defaults to 0, in which case max for format (256 or 65536) is used. |
| dynamic         | Optimization hint used by some Surface subtypes. A dynamic surface is one that is updated very often. Might speed up CPU access to the surface such as PixelBuffer operations and Surfaces own copy operations but slow down blit operatons. Defaults to false. |
| format          | The pixel format for the surface. Default format is decided by the surface subtype but ARGB_8 is typical. |
| identity        | An integer value that can be set to whatever and used to identify the surface at a later time. Defaults to 0. |
| mipmap          | Set to true in order to enable mipmapping of the surface. Mipmapping allows for far better down-scaling but takes almost 50% more memory and slows down modifications of the surface. Defaults to false. |
| sampleMethod    | Sample method used when scaling graphics. Set to Nearest or Bilinear. Nearest gives blocky scaling with no interpolation between pixels but is way faster when using software rendering. Defaults to Bilinear on graphics subsystems where this doesn't slow down operations. |
| scale           | Scale of the surface. Defaults to 64.                        |
| size            | Size in pixels of the surface. Bitplane based formats must have a width that is a multiple of 16 pixels and some graphics subsystems (and hardware platforms) might place other limitations. |
| tiling          | Must be set to true if GfxDevice::tile() operations should be used to blit graphics from the surface. Please not that some surface subtypes like SoftSurface puts limitations on the size of the surface when tiling. Defaults to false. |



## Pixel formats

Each surface has a specific pixel format and several different pixel formats are supported by WonderGUI. This includes common formats such as 32 bit ARGB but also 8 and 16 bit indexed palette formats as well as legacy bitplane formats as used on 16-bit computers from the 90's.

Pixel formats only describe how the channels are organized, named in register order starting with the most significant bits. Color space and byte order are separate properties of the surface.

All graphics backends have limitations and therefore only supports surfaces of certain pixel formats. Only Surface subtype to support all pixel formats is SoftSurface. This so that all formats can be loaded and converted to appropriate format for the graphics subsystem in use. SoftGfxDevice does only support a subset of pixel formats.



## Points, pixels and scale factor

A surface has a scale factor that specifies the relation between pixels and points, just like the rest of WonderGUI, but when manipulating a surface directly through its methods you work with pixel units and don't have to care about the scale factor.

The scale is used when you create skins based on surfaces such as BlockSkin. Since surface coordinates are specified in points when creating a skin, you can have a set of surfaces containing the same graphics in different sizes and scale factors and choose which one to load based on the host computers scale factor without having to change the coordinates used.

The scale factor of a surface is also used by RootPanel to determine the scale factor of the GUI when setting a surface as its canvas.



## Creating surfaces

Surfaces can be created either directly through their create() methods or by using a *SurfaceFactory*. The advantage of using a SurfaceFactory is that you don't need to know the specific subtype of Surface to create where you create it, otherwise it is essentially the same:

```c++
// Create a surface directly

auto p1 = SoftGfxSurface::create({ 
	.format = PixelFormat::ARGB_8, 
	.size = {640,480} 
});

// Create a surface using a factory

auto p2 = pFactory->createSurface({ 
	.format = PixelFormat::ARGB_8, 
	.size = {640,480} 
});
```



A SurfaceFactory can either be created directly or retrieved from its corresponding GfxDevice:

```
// Create a SoftSurfaceFactory specifically.

auto pFactory1 = SoftSurfaceFactory::create();

// Get a SurfaceFactory from the GfxDevice in use.

auto pFactory2 = pMyGfxDevice->surfaceFactory();
```





