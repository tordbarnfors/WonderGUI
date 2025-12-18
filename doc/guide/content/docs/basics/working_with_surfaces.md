---
title: 'Working with Surfaces'
weight: 40
---

# Working with Surfaces

*This section deals with how to work with surfaces beyond creating them and using them as resources for GfxDevice. Different ways of copying, converting and modifying content of the surface are explained.*



**Important:** The methods discussed here all use the CPU to copy and manipulate data. This is way slower than using a hardware accelerated GfxDevice to do the copying and in many cases also slower than using SoftGfxDevice, which has been optimized for fast operations.

These methods are implemented in a generic fashion, making them small and always available independent of graphics backend in use. They are handy for converting surfaces between surface subtypes and manipulate surfaces with pixel formats not supported by the current backend, but shouldn't be used for anything time critical.

Accessing pixels directly through PixelBuffers are the exception. Using pixel buffers and manipulate pixels directly can be faster than making many calls to a GfxDevice to achieve the same effect.



## Clearing and filling

To clear a surface you simply fill it with the color you want, using the fill() method. A rectangle can be specified in which case only that rectangle will be filled.

```c++
pMySurface->fill( Color::Black );

pMySurface->fill( {10,10,200,100}, Color::White );
```

The alpha channel is written as well as the color components. No blending takes place. To clear the alpha channel, simply specify a color with 0 alpha or just use Color::Transparent.



## Copying graphics between surfaces

The copy() method is an easy way to copy graphics between surfaces. The following example copies  a rectangular block of 100 x 40 pixels from top-left corner of the specified source surface to position (50,50) of our surface:

```c++
pMySurface->copy( {50,50}, pOtherSurface, {0,0,100,40} );
```

The source rectangle can be left out, in which case the whole source surface is copied to the specified destination.

This copy operation works between any source and destination format, pixels are converted on the fly. No blending takes place, the alpha channel is copied as is, if both source and destination has alpha channel. If source is lacking an alpha channel, the destination pixels are made opaque. If destination lacks an alpha channel, the alpha channel of the source is simply discarded.

### Considerations when copying to palette based surfaces

When copying to an indexed surface, there is a lot of overhead which might make the operation very slow and the operation might even fail. The reason is that the color of each pixel copied needs to be checked against the color of the destination palette. If the color doesn't exist in the destination palette it will be added if there is unused capacity in the palette and thus grow the palette. If the palette can not grow, the operation might fail midway, with some pixels copied and possibly an extended palette.

This is when copying TO palette based surfaces. Copying from a palette based surface always works  and have no performance penalty.

### sRGB and Linear formats

This is unlikely to affect you since sRGB is so prevalent nowadays, but it might be worth mentioning that pixels are converted between sRGB and linear formats if not both surfaces use the same. So copying between BGRA_8_linear and BGRA_8_sRGB will modify the pixels although they have the same number of bits for each component.



## Converting surfaces

Converting a surface is just as easy as copying blocks of graphics between them. You need to specify a Blueprint for the new surface and the surface factory to use.  This example converts a surface to 16-bit from whatever format it was before:

```
auto pNewSurface = pMySurface->convert( { .format = PixelFormat::BGR_565_linear }, pSurfaceFactory );
```

The size-property of the blueprint is ignored, the converted surface is always the same format as the original surface.

One thing to keep in mind is that the supplied blueprint has defaults that might differ from the original surface. Flags such as canvas or buffered might be set which are lost in the process. To avoid this you can copy the original surfaces blueprint and just modify the properties intended:

```c++
auto pNewSurface = pMySurface->convert( WGOVR(pMySurface->blueprint(),
	_.format = PixelFormat::BGR_565_linear
));
```

However, be aware that  this will copy the content of the **whole** blueprint, including things you might not have intended such as id, baggage and finalizers.





## Accessing pixels directly

To access the pixels in a surface we use something called *PixelBuffers*. Conceptually a PixelBuffer is a buffer in memory to which a rectangular block of pixels from the buffer is copied. The pixels can then be read and optionally modified and sent back to the Surface. In reality the PixelBuffer is often just a window to the pixels in the surface to avoid unnecessary copying. If it can be a window or pixels needs to be copied depends on the graphics subsystem in use and might also depend on the hardware it runs on.

The process is fairly straight forward and divided into 5 steps:

1. Allocate the PixelBuffer.
2. Push pixels from the surface to the PixelBuffer.
3. Read and/or modify the pixels.
4. Pull pixels back into the Surface.
5. Free the PixelBuffer.

Step 2 can be skipped if you intend to fill the whole PixelBuffer with new data, not reading a single pixel value.

Step 4 can (and should) be skipped if you only read the pixels.

Let's look into the different steps and what they do.

### Allocating the PixelBuffer

Allocating the PixelBuffer either "opens a window" to the pixels or allocates a buffer in memory large enough to hold the pixels. It then returns a struct containing all the information you need about the buffer and the pixels. By supplying a rectangle to the call you limit the buffer/window to those pixels, which can both be convenient and save time and memory if a buffer is involved.

The following example allocates a pixel buffer mapping the top 64x64 pixels of the surface:

```c++
PixelBuffer bufferInfo = pMySurface->allocPixelBuffer( {0,0,64,64} );
```

After the call, the struct called *bufferInfo* contains the following information:

| Member  | Type         | Description                                                  |
| ------- | ------------ | ------------------------------------------------------------ |
| format  | PixelFormat  | The format of the pixels in the buffer (same as the Surface). |
| pixels  | uint8_t*     | A pointer to the top-left pixel in the buffer.               |
| palette | const Color* | Pointer at palette if format uses a palette.                 |
| rect    | RectI        | The area of the Surface the buffer maps to, which is the same as the rectangle provided the allocPixelBuffer() call (or the whole surface if no rectangle provided). |
| pitch   | int          | Distance in bytes from beginning of one row of pixels to the next. |

The buffer is tied to the specific surface is was created from. You may not pull or push pixels to another surface using the same buffer.

Once allocated the buffer should be considered to be empty. Before reading any pixels from the surface you should first push pixels to the buffer.

### Pushing pixels to the buffer

Pushing pixels is done through one simple call to the surface using the buffer struct as parameter:

```
pMySurface->pushPixels(bufferInfo);
```

Optionally you can push only some pixels to the buffer by supplying a rectangle:

```
pMySurface->pushPixels(bufferInfo, {5,5,16,20});
```

The rectangle supplied is relative to *the buffer rectangle and not the surface*, meaning that if the buffer maps an area starting 10 pixels into the buffer, a rectangle starting at offset 0 also starts 10 pixels into the buffer.

By pushing pixels to the buffer we are guaranteed that those pixels in the buffer are identical to the ones in the surface. If we have a window into the surface and no separate buffer, the pushPixels() call will do nothing and cause no overhead. Either way we should always call pushPixels() before we read pixels.

The only time you don't need to call pushPixels() is when you intend to fully replace *all* the pixels of the buffer, without reading any.

We can now start to read and/or write the content of the buffer.

### Reading and writing pixels

Reading and writing pixels is simply a matter of reading and writing the data pointed to by the pixels-member of the PixelBuffer struct. Typically you do know the format of the pixels in the surface beforehand since you have allocated it, but the format member can be read if you don't. 

In the example below we draw a red line across the 64x64 buffer allocated above. The pixel format is known to be BGRA_8_sRGB, meaning one byte each for blue, green, red and alpha in that order.

```c++
for( int y = 0 ; y < 64 ; y++ )
{
	// Set pointer at pixel. Y * pitch moves pointer to next line, 
	
    int x = y;
	uint8_t * p = bufferInfo.pixels + y * bufferInfo.pitch + x* 4;

	// Fill in values for blue, green red and alpha

	p[0] = 0;
	p[1] = 0;
	p[2] = 255;
	p[3] = 255;
}
```

This is a very naive implementation, but it serves as a good example.

Once we have written our pixels we need to pull them back into the surface.

### Pulling pixels to the surface

Once we are done writing pixels we need to pull the pixels back into the surface. If you are modifying pixels, this is an important step. Even if you know that you have direct access to the pixels in the surface you should call pull, since it tells WonderGUI what areas of the surface that have been modified.

Pulling pixels works exactly like pushing, just moves them the other direction. The parameters are the same as when pushing. You can pull all pixels of the buffer or a subset represented by a rectangle.

```c++
pMySurface->pullPixels(bufferInfo);

pMySurface->pullPixels(bufferInfo, {5,5,16,20});
```

Once you are done with your buffer you should free it.

### Freeing the PixelBuffer

The PixelBuffer is freed by this simple call:

```c++
pMySurface->freePixelBuffer(bufferInfo);	
```

Once the buffer is freed it should not be used any more.

Besides returning any allocated memory it will also notify the surface that there will be no more accessing the pixels until a new buffer is allocated.

### Summary

Putting it all together we get the following example:

```c++
PixelBuffer bufferInfo = pMySurface->allocPixelBuffer( {0,0,64,64} );

pMySurface->pushPixels(bufferInfo);

// Modify pixels here...

pMySurface->pullPixels(bufferInfo);
pMySurface->freePixelBuffer(bufferInfo);

```

Some additional things to point out:

* It is perfectly fine to allocate several small PixelBuffers for the same surface at the same time, but they should not overlap.
* Even though you only write pixels to the PixelBuffer you need to pullPixels() first if you don't change ALL pixels in the buffer. Otherwise you might pull dirt into the buffer alongside your written pixels.
* It is absolutely forbidden to write pixels outside the buffer.
* A PixelBuffer should only be alive for a short amount of time. Depending on graphics backend it's existence can hamper or even prevent rendering operations.
* If the PixelFormat is unknown or not fully understood you can use Util::pixelFormatToDescription() to get detailed information about the pixel format, such as the size of a pixel, size and arrangement of R, G, B and A components etc.



## Modifying the palette

Although the palette is intended to be modifiable we currently do not provide any API for that.

