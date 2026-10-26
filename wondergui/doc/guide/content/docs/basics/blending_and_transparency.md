---
title: 'Blending and transparency'
weight: 25
---

# Blending and transparency

*This section defines how GfxDevice combines what you draw with what is already on the canvas: what each BlendMode does, how transparency and anti-aliasing work, and what to expect when you draw onto transparent surfaces. All graphics backends are meant to follow these definitions. ReferenceBackend implements them as directly as possible and is used to test the other backends against them.*



## Colors

Blending happens in **linear light**, with every channel going from 0.0 to 1.0. Colors given to GfxDevice (HiColor) are sRGB and pixels of sRGB surfaces are converted to linear before blending and back again afterwards. Mixing of colors in Tint gradients is the exception, it happens in sRGB before the result is converted to linear.

Pixels are stored with **straight alpha**, e.g. the color channels are not multiplied with alpha.

- A missing color is white. An Alpha_8 surface used as a blit source is white with the alpha of the surface.
- A missing alpha is opaque. A surface without alpha (XRGB_8, RGB_565, BGR_565) is always opaque.
- **The color of a fully transparent pixel never matters.** It can't be seen and it doesn't bleed into anything else, whether through blending, filtering or anti-aliasing. The one exception is Replace onto a canvas without alpha, see below.

In the formulas below:

| Symbol | Meaning |
|---|---|
| Cb, αb | Color and alpha of the canvas pixel (b as in backdrop). |
| Cs, αs | Color and alpha of what we draw (s as in source), with the tint color and Tint already multiplied in. |
| mix(a, b, t) | a + (b − a) · t |

Color formulas apply to each of the color channels separately. All results are clamped to 0.0 -> 1.0.



## The glass model

Think of a transparent canvas as a perfect, non-reflective sheet of glass that we paint on.

**Paint modes** put paint on the glass. Where paint is semi-transparent, more paint makes the glass more opaque. Blend is the normal way of painting, Replace scrapes off what was there and paints anew.

**Effect modes** don't add any paint. They change the light that comes from the paint already on the glass, like shining a light on it (Add), filtering it (Multiply) or inverting it (Invert). Light that passes through the glass from behind isn't affected. Modes that take light away (Subtract, Multiply and Min) leave the glass as transparent as it was. Modes that add light (Add, Invert and Max) make the glass glow where its paint can't hold the light they add, which makes it more opaque.

In both groups, the alpha of the source always means **how much**: a fully transparent source has no effect at all.



## Paint modes

| BlendMode | out.rgb | out.α |
|---|---|---|
| **Replace** | Cs | αs |
| **Blend** | (Cs · αs + Cb · αb · (1 − αs)) / out.α | αs + αb · (1 − αs) |
| **BlendFixedColor** | As Blend. | |
| **Morph** | (Cb · αb · (1 − m) + Cs · αs · m) / out.α | αb · (1 − m) + αs · m |
| **Ignore** | Cb | αb |
| **Undefined** | Treated as Blend. | |

**Blend** is Porter-Duff *over*. On an opaque canvas pixel it is the familiar `Cs · αs + Cb · (1 − αs)`. The division matters where the canvas is transparent: drawing onto a transparent surface and blending that surface onto a background gives the same result as drawing directly onto the background.

**Replace** writes the source as it is, alpha included. It can make the canvas transparent.

**BlendFixedColor** is an optimization of Blend for software rendering where the canvas already is a flat area of one color, the fixed blend color: the source is blended onto that color instead of the canvas pixels being read. Any backend may draw it as Blend, which gives the same result there.

**Morph** cross-fades from the canvas to the source by the morph factor m, alpha included. It mixes premultiplied colors, so when one side is transparent the other side simply fades out instead of being darkened by the color of the transparent side.



## Effect modes

Effect modes work on the light that a pixel of the canvas gives, which is its premultiplied color **P = Cb · αb**. On an opaque canvas that is simply its color.

The alpha of the source is the strength of the effect. It fades the source towards the color that has no effect: black for Add, Subtract, Invert and Max, white for Multiply and Min. So the source is applied as the light it gives, **s = αs · Cs**, or for Multiply and Min as **t = mix(1, Cs, αs)**. **m** is the strongest channel of s.

| BlendMode | out.rgb · out.α | out.α | Meaning |
|---|---|---|---|
| **Add** | P + s | αb + m | Light shining on the paint. |
| **Subtract** | P − s | αb | Light taken away. |
| **Multiply** | P · t | αb | A filter. A transparent source is a clear filter. |
| **Invert** | P + s · (1 − 2 · P) | αb + m · (1 − αb) | Inverts the canvas, each channel by the amount given by Cs. |
| **Min** | min(P, t) | αb | Limits the canvas to at most Cs. |
| **Max** | max(P, s) | max(αb, m) | Raises the canvas to at least Cs. |

out.rgb is the first column divided by out.α. On an opaque canvas these are the familiar operations on its color, like `Cb + αs · Cs` for Add and `max(Cb, αs · Cs)` for Max.

Add, Invert and Max can give more light than the paint on the glass can hold. They then raise its alpha by the strongest channel of s: Add adds it, Max takes the larger of the two and Invert puts it over the alpha of the canvas. Unlike the other effect modes they therefore light up transparent parts of the canvas. Add with white at half strength onto a transparent pixel makes it white at half opacity.

Subtract, Multiply and Min only take light away and keep the alpha of the canvas. They have no effect on transparent parts of it, and they affect semi-transparent paint only as far as it gives light: Min with mid grey doesn't darken a pixel that is red at 50% alpha, since its light already is no brighter than mid grey.

An alpha-only surface with a soft round shape, blitted with Add and a tint color, works as a point light shining on the canvas. Blitted with Min and a dark tint color it caps the brightness like a shadow, with Max and a light tint color it raises the brightness like a fill light.

These definitions are what GPUs can do with fixed-function blending of premultiplied colors in a single pass, see Exactness below.



## Anti-aliasing

Edges of fills, lines, edgemaps, shapes and warps are anti-aliased by the exact area of each pixel that is covered.

With a paint mode, **a partially covered pixel is drawn as if the canvas had a much higher resolution, the shape was drawn there without anti-aliasing and the subpixels were then averaged.** That is the average of what the blend mode gives the covered part and the original pixel for the rest, weighted by their areas:

```
out = average( op(source, canvas) weighted by coverage,  canvas weighted by (1 − coverage) )
```

A pixel crossed by several segments of an edgemap (also when warped) or a shape gets the average of what each segment gives its part of the pixel. The segments of a shape (fill and outline of a rounded box; back, outline, hub and slices of a ring) are drawn together, so the canvas doesn't show through where they meet.

Averages of colors are taken as **premultiplied colors**: each color counts by its alpha as well as by its area, so that a transparent part only makes the pixel more transparent and doesn't darken it.

For example: a quarter of an opaque red pixel is covered by a Replace with black at 50% alpha. Three quarters of the pixel stay opaque red and one quarter becomes half transparent black, which averages to alpha 0.875 and a color of 86% red.

For Blend this is the same as multiplying the alpha of the source by the coverage.

With an effect mode, **a partially covered pixel gets the effect with a strength scaled by the area covered**, just like the alpha of the source scales it:

```
out = op(source with its alpha times coverage,  canvas)
```

Where several segments of an edgemap or a shape share a pixel, they are combined into one source first: their premultiplied average weighted by the area each covers, with the alpha of them all together. Parts of the pixel not covered by any segment count as transparent, which has no effect.

For Multiply and Invert, and for Add and Subtract as long as they don't saturate, this is the same as averaging subpixels. For Min and Max, and where Add and Subtract saturate, it means that partial coverage gives a partial effect: a half covered pixel of a Max with white is raised to at least mid grey, rather than half of it to white.



## Sampling

Bilinear sampling interpolates premultiplied colors, so transparent pixels don't bleed their color into their neighbours. Mipmaps should be generated the same way.

Outside the source of a ClipBlit is transparent. That has no effect in any effect mode and doesn't darken the edges of a bilinear blit.



## Canvases without alpha

A canvas without alpha simply doesn't store the alpha of the result, its pixels stay opaque. That only concerns paint modes that can give a result that isn't opaque: Replace writes the color of the source, whatever its alpha, and so does Morph with the color it results in.

This is the one exception to the rule that the color of a fully transparent pixel never matters: a transparent Replace onto a canvas without alpha writes the color the transparent pixel has.

Anti-aliased edges are not affected, they are mixed with what was there as on any other canvas.



## Alpha-only canvases

An Alpha_8 canvas is the one exception to the glass model. It has no color, so it is a one-channel image and its alpha is its value.

- **Paint modes** work on alpha as on any other canvas.
- **Effect modes** would do nothing if they kept alpha. Instead they work on alpha with the alpha of the source as the value: Add grows the mask, Subtract cuts holes in it, Multiply and Min intersect and Max unites.

Anti-aliasing works as on any other canvas: paint modes average the result and the original value by area, effect modes scale the value of the source by the area covered. For Multiply and Min, whose value with no effect is 1, the parts of the pixel not covered count as 1.

| BlendMode | out.α |
|---|---|
| Add | αb + αs |
| Subtract | αb − αs |
| Multiply | αb · αs |
| Invert | αs · (1 − αb) + αb · (1 − αs) |
| Min, Max | min(αb, αs), max(αb, αs) |



## Layers

Since Blend is Porter-Duff over, content drawn with paint modes onto a transparent layer (a CanvasLayers layer or a surface of your own) and then blended onto the canvas looks exactly as if it was drawn directly onto the canvas.

Add, Invert and Max leave their light on a transparent layer, so a single one of them drawn onto a transparent layer, with the layer then blended onto the canvas with the same mode, gives the same result as drawing directly onto the canvas. Subtract, Multiply and Min only change light that is already there, and on a transparent layer there is none. If the content of a layer should work as one of them on what is below the layer, draw it onto the layer with Blend and blend the layer with the effect mode, using `blendFunc` in CanvasLayers::LayerBP.



## Relation to other standards

*Porter-Duff compositing* (1984) defines how the coverage of two images combines. Blend is its *over* operator. Subtract, Multiply and Min resemble its *atop* operator, which keeps the alpha of the destination, but change the destination instead of placing the source on it.

*W3C Compositing and Blending* (used by CSS, SVG, PDF and HTML canvas) treats blend modes as paint: the blended color is composited with *over*, so a blend mode paints normally onto transparent parts of the canvas and makes the canvas more opaque. Multiply, Min, Max and Invert correspond to W3C's multiply, darken, lighten and exclusion, applied as effects instead of paint. Add corresponds most closely to the compositing operator plus-lighter, which also adds alpha, but by the alpha of the source instead of by its strongest channel. Invert on premultiplied colors is the same as exclusion, apart from alpha.

Neither standard has a mode that takes away alpha by the alpha of the source, like Porter-Duff *destination-in* does. That could be added as a separate paint mode if it is needed for masking.



## Exactness

ReferenceBackend and the software backends can follow these definitions exactly everywhere.

GPU backends can follow them exactly with fixed-function blending in a single pass, provided that canvases with alpha store premultiplied colors and dual-source blending is used (available in OpenGL 3.3, Metal and DirectX 12). The effect modes, and how coverage scales them, are defined to be what such blending can do: none of them needs the alpha of the canvas as a blend factor, which Invert, Min and Max can't use. Dual-source blending is only needed for the paint modes.

The area of a pixel covered by a shape is exact in ReferenceBackend. The software backends are exact along straight edges and follow circles within 1/64 pixel. GPU backends estimate it from the distance between the pixel center and the edges.

The area of a pixel covered by a warp (warpBlit() and warpDrawEdgemap()) is measured by ReferenceBackend at thousands of points in the pixel. The other backends estimate it from the position at the center of the pixel and how fast it changes, taking each border of the source and each edge of an edgemap as straight over the pixel, which is off by a few percent where an edge bends sharply within a pixel.



## Implementation status

| | ReferenceBackend | SoftBackend, LinearBackend | OpenGL, Metal, DirectX 12 |
|---|---|---|---|
| Blend as Porter-Duff over | Yes | Yes | Yes |
| Source alpha as strength of effect modes | All | All | All |
| Effect modes on the light of the canvas | Yes | Yes | Yes |
| Paint modes: anti-aliasing as average of subpixels | Yes | Yes | Yes |
| Effect modes: anti-aliasing as strength scaled by coverage | Yes | Yes | Yes |
| Subpixels averaged as premultiplied colors | Yes | Yes | Yes |
| Morph as premultiplied cross-fade | Yes | Yes | Yes |
| Premultiplied bilinear sampling | Yes | Yes | Yes |
| Transparent outside of ClipBlit has no effect | Yes | Yes | Yes |
| Warps (warpBlit, warpDrawEdgemap) | Yes | Yes | Yes |

The software backends draw BlendFixedColor onto the fixed color, as described under Paint modes.

The GPU backends keep canvases and other surfaces with alpha premultiplied and use dual-source blending, as described under Exactness. Colors of nearly transparent pixels therefore lose some precision when read back from them.

GfxDeviceTester has tests for these rules in BlendRuleTests and LayerRoundTripTests, and for warps in WarpTests.
