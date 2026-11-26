---
title: 'Bitmaps'
weight: 30
---

# Bitmaps

Bitmaps are containers of bitmap graphics (or textures) and are a central part of WonderGUI.

They are used both as sources of graphics fragments that builds up the GUI and as canvases to which the GUI (or parts thereof) is rendered.

Each graphics backend (OpenGL, Metal, software etc) has its own Bitmap subtype which needs to be used with that backend.

Typically you use a *GfxDevic*e to copy graphics from one bitmap to another where the source can be stretched, rotated and tinted before being blended onto the destination, but there are also a few simple methods for clearing a bitmap, copy graphics between bitmaps and directly manipulating pixels in a bitmap. Besides being convenient, these methods are also useful when you want to convert between pixel formats or bitmap subtypes. See [working with bitmaps](../basics/working_with_bitmaps) for more information about these methods. 

Bitmaps are *semi-mutable* in the sense that the properties of the bitmap (like width, height and pixel format) can not be changed once created, but the content of the bitmap can. This includes the palette if bitmap is indexed.

Bitmaps can be created either directly through their create() methods or by using a *BitmapFactory*. The advantage of using a BitmapFactory is that you don't need to know the specific subtype of Bitmap to create where you create it, otherwise it is essentially the same.

You can get a callback when a bitmap is modified by adding an Observer.



## Properties

| Property        | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| userData        | Any object associated with the bitmap. Can be retrieved using userData(). |
| bigEndian       | Byte order of the pixels. Defaults to the byte order of the system. |
| canvas          | Must be set to true if bitmap might be used as canvas by the GfxDevice. This might affect performance. Defaults to false. |
| colorSpace      | Color space of the pixels, sRGB or Linear. Defaults to sRGB. Colors given to WonderGUI are always sRGB, while blending always is done in linear space. |
| fillColor       | The color a bitmap created without pixels is filled with. Defaults to Color::Undefined, in which case nothing is filled in and the content of the bitmap is unspecified until you have drawn or written to all of it. Ignored when pixels are supplied. |
| finalizer       | A function that is called right before the Skin is destroyed. |
| palette         | A pointer at an array of Color8 objects used as the palette if pixel format is indexed or bitplanes. Mandatory if pixel format is indexed or bitplanes. Defautls to nullptr. |
| paletteCapacity | Allows for setting the size of the palette to a larger value than paletteSize for later adding of colors. No pixel index may have a larger value than paletteCapacity. Defaults to the value of paletteSize. |
| paletteSize     | Number of color entries initially specified for the palette. Defaults to 0, in which case max for format (256 or 65536) is used. |
| dynamic         | Optimization hint used by some Bitmap subtypes. A dynamic bitmap is one that is updated very often. Might speed up CPU access to the bitmap such as PixelBuffer operations and Bitmaps own copy operations but slow down blit operatons. Defaults to false. |
| format          | The pixel format for the bitmap. Default format is decided by the bitmap subtype but ARGB_8 is typical. |
| identity        | An integer value that can be set to whatever and used to identify the bitmap at a later time. Defaults to 0. |
| mipmap          | Set to true in order to enable mipmapping of the bitmap. Mipmapping allows for far better down-scaling but takes almost 50% more memory and slows down modifications of the bitmap. Defaults to false. |
| restorer        | A function that puts the content of the bitmap back when a GPU has lost it, which lets the bitmap go without a copy of its pixels in system memory. See [below](#keeping-the-pixels-and-getting-them-back). Defaults to nullptr, in which case the pixels are kept. |
| sampleMethod    | Sample method used when scaling graphics. Set to Nearest or Bilinear. Nearest gives blocky scaling with no interpolation between pixels but is way faster when using software rendering. Defaults to Bilinear on graphics subsystems where this doesn't slow down operations. |
| scale           | Scale of the bitmap. Defaults to 64.                        |
| size            | Size in pixels of the bitmap. Bitplane based formats must have a width that is a multiple of 16 pixels and some graphics subsystems (and hardware platforms) might place other limitations. |
| tiling          | Must be set to true if GfxDevice::tile() operations should be used to blit graphics from the bitmap. Please not that some bitmap subtypes like SoftBitmap puts limitations on the size of the bitmap when tiling. Defaults to false. |



## Keeping the pixels and getting them back

A bitmap of a graphics backend with a GPU (OpenGL, DirectX 12, Metal, Vulkan) has its pixels in a texture on the GPU. The texture can be lost, as when the graphics driver is restarted or updated, and reading it takes waiting for the GPU. So such a bitmap also keeps its pixels in system memory, *unless it is promised them back*:

| Bitmap                      | Kept in system memory                         | When the texture is lost                            | Reading pixels              |
| ---------------------------- | --------------------------------------------- | --------------------------------------------------- | --------------------------- |
| Without restorer             | All pixels                                    | Sent again, nothing for you to do                   | At once                     |
| With restorer                | The alpha channel, if not all of it is opaque | The restorer is called                              | Read back, waits for the GPU |
| Alpha_8 and palette based    | All pixels, restorer or not                   | Sent again, nothing for you to do                   | At once                     |
| Canvas                       | Nothing                                       | The restorer is called if there is one, see below   | Read back, waits for the GPU |

So a bitmap in ARGB_8 takes 8 bytes for each pixel without restorer, 5 with one (4 if it is opaque) and 4 as a canvas.

The rule of thumb: give a restorer to bitmaps that you only write, such as graphics loaded from a file that can be loaded again or anything that is replaced all the time, and none to bitmaps that you read.

### The restorer

The restorer is a function that takes the bitmap and returns a bool. It is called between frames, when the GPU device has been replaced, and writes the content the bitmap is to have, using any of the methods of the bitmap:

```c++
bool restoreMyLogo( Bitmap * pBitmap )
{
	auto pOriginal = loadMyLogoIntoSoftBitmap();
	return pBitmap->copy( {0,0}, pOriginal );
}

auto pLogo = pFactory->createBitmap({ .restorer = restoreMyLogo, .size = {256,64} }, pPixels );
```

It is held as a std::function, so it can also be a lambda that captures what it needs, such as the path of the file to load:

```c++
auto pIcon = pFactory->createBitmap({ .restorer = [path]( Bitmap * pBitmap ) { return loadInto( pBitmap, path ); }, .size = {64,64} }, pPixels );
```

A plain function only gets the bitmap, and can use its identity or user data to tell bitmaps sharing a restorer apart.

Whatever a restorer captures or uses has to be there for as long as the bitmap is, which can be longer than you think: a font or a frame being rendered may hold on to the bitmap after you let go of it. If a restorer points at something that is going away before the bitmap, call dropRestorer() on the bitmap first. The bitmap then behaves as if its restorer returned false.

* Return true when the content is back. It has to be what the bitmap held when it was lost, which isn't what it was created with if you have changed it since.
* Return false when you did nothing. The bitmap is then transparent black and its isContentLost() returns true until all of it has been written. Use this when restoring takes time you don't want to spend in one go, and write the pixels when you have them.
* Bitmap::declineRestore is a restorer that just returns false, for bitmaps where everything is replaced all the time anyway, such as one showing video.

A canvas without restorer says isContentLost() when what was rendered into it is lost, and whoever rendered into it needs to do that again. The widgets of WonderGUI that keep content in canvases do so themselves.

Backends that have no GPU ignore the restorer, as they have nothing to lose. Of those with a GPU only the DirectX 12 backend can currently replace its device, but a restorer saves memory on all of them.

## Pixel formats

Each bitmap has a specific pixel format and several different pixel formats are supported by WonderGUI. This includes common formats such as 32 bit ARGB but also 8 and 16 bit indexed palette formats as well as legacy bitplane formats as used on 16-bit computers from the 90's.

Pixel formats only describe how the channels are organized, named in register order starting with the most significant bits. Color space and byte order are separate properties of the bitmap.

All graphics backends have limitations and therefore only supports bitmaps of certain pixel formats. Only Bitmap subtype to support all pixel formats is SoftBitmap. This so that all formats can be loaded and converted to appropriate format for the graphics subsystem in use. SoftBackend does only support a subset of pixel formats.



## Points, pixels and scale factor

A bitmap has a scale factor that specifies the relation between pixels and points, just like the rest of WonderGUI, but when manipulating a bitmap directly through its methods you work with pixel units and don't have to care about the scale factor.

The scale is used when you create skins based on bitmaps such as BlockSkin. Since bitmap coordinates are specified in points when creating a skin, you can have a set of bitmaps containing the same graphics in different sizes and scale factors and choose which one to load based on the host computers scale factor without having to change the coordinates used.

The scale factor of a bitmap is also used by RootPanel to determine the scale factor of the GUI when setting a bitmap as its canvas.



## Creating bitmaps

Bitmaps can be created either directly through their create() methods or by using a *BitmapFactory*. The advantage of using a BitmapFactory is that you don't need to know the specific subtype of Bitmap to create where you create it, otherwise it is essentially the same:

```c++
// Create a bitmap directly

auto p1 = SoftBitmap::create({ 
	.format = PixelFormat::ARGB_8, 
	.size = {640,480} 
});

// Create a bitmap using a factory

auto p2 = pFactory->createBitmap({ 
	.format = PixelFormat::ARGB_8, 
	.size = {640,480} 
});
```



A BitmapFactory can either be created directly or retrieved from its corresponding GfxDevice:

```
// Create a SoftBitmapFactory specifically.

auto pFactory1 = SoftBitmapFactory::create();

// Get a BitmapFactory from the GfxDevice in use.

auto pFactory2 = pMyGfxDevice->bitmapFactory();
```





