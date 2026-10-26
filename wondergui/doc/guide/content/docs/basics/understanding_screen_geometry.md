---
title: 'Screen geometry'
weight: 20
---

# Understanding screen geometry



## Coordinate system

The WG global coordinate system starts in the upper left corner of the screen with coordinate (0,0). The horizontal axis goes to the right and the vertical axis goes down.

Widgets however resides in local coordinate systems that goes in the same direction but where coordinate (0,0) is the upper left corner of the container widget that is its parent.



## Points, subpixels and scale

WonderGUI very rarely measures anything in pixels. Instead two complimentary units of measurement are used - *points* and *subpixels*.

### Points (pts)

Points is the canonical unit used for specifying position and size of widgets and what you will use almost all the time. Points are independent of the screens pixel resolution and therefore scales with the specified scale. The type ***pts*** is used for points and by default resolves to a *float*, but can be set to *int* if performance of floating point math is a concern and limitations in scaling capabilities can be accepted.

The basic geometry classes *Coord, Size, Rect* and *Border* all use *pts* as units, allowing geometry to be specified with great precision.

WonderGUI however currently limits the placement of widgets to pixel boundaries with appropriate rounding of coordinates.

### Subpixels (spx)

Subpixels is a high-precision integer measurement unit that measures everything in 1/64th of a pixel. This is used extensively inside WonderGUI but is rarely seen through the API with one exception - the graphics subsystem. All calls to GfxDevice including setting of clip-rectangles, drawing of lines and blitting of graphics use subpixels.

Any look inside a widget will reveal that nearly all internal methods uses spx as their format and proper understanding of spx is needed for writing your own widgets.

The type ***spx*** is defined for subpixels and resolves to an *int*. A set of geometry classes for handling geometry in subpixel format exists in the form of *CoordSPX, SizeSPX, RectSPX* and *BorderSPX*.

It is worth noting that although everything is expressed in a resolution of 1/64th of a pixel, a lot of methods currently operates on a pixel boundary, requiring values to be rounded or truncated.

### Scale

The scale specifies the relation between points and subpixels and is defined as *the number of subpixels corresponding to one point*. The default scale is 64, resulting in one point corresponding to one pixel.

A scale of 128 means that there are 128 subpixels per point, i.e. 2 pixels per point.

Scale is set on the Root-widget, but can be changed for a branch in the widget hierarchy by inserting a ScaleCapsule, through which you can set a new scale for all its descendants. 
