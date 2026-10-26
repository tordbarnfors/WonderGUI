---
title: 'FillMeterSkin'
weight: 10
---


# FillMeterSkin

FillMeterSkin simply fills an area with a solid color depending on the values from the widget and the properties set. You could say it draws a filled bar whose length and start point depends on the values from the widget.

If the widget provides two values, they are used as the begin and end positions of the bar, otherwise the single value is used as the length of the bar.

The bar can start from any side of the skins rectangular area and thus grow in any of the four directions. As a special case, the bar can also be set to start from the center and grow towards the sides or top and bottom.

The bar color can be set to fade from one value for its lowest value to another for its highest value and there are some properties to control defaultSize and tweak padding and minimum length.

## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| backColor | Color for area not covered by the bar. Default is Transparent. |
| blendMode | BlendMode used for bar and its background. |
| color | Color used for the bar. |
| direction | Direction for the bar to grow in. |
| gfxPadding | Padding for the area for the bar. |
| gradient | Gradient used for bar and its background. |
| maxColor | Color of bar when bar is full. |
| minColor | Color of bar when bar is at its minimum. |
| defaultSize | Default size of the skin in pts. |
| startFromCenter | Set to true to make the bar start from the center and grow to the sides or top and bottom depending on direction. |
| startLength | Length of bar when length from widget is 0. |

## Layout

In the normal case the bar starts from one of the four sides of the rectangle and grows toward the opposite as defined by the *direction* property.

However, there is often a need to tweak the area the bar occupies and set a start length so it doesn't go invisible at the widgets minimum value. For this purpose the properties *gfxPadding* and *startLength* are provided. See the examples section for a good example on how these values can be used.

## Coloring

Typically the bar is given a color defined by the *color* property, but if both *minColor* and *maxColor* are declared, the color property is overridden and the bar gets a color that is interpolated between minColor and maxColor depending on the length of the bar.

A background color can also be set through the *backColor* property, but is by default transparent.

Finally a gradient can be applied. The gradient covers the whole area of the bar and affects both the bar and the background if not transparent.





## Examples

Let's say we have a Slider widget where a handle can be moved left to right and that we want the area of the background left of the handle to have a bright blue color, like this:

[image]

The rounded shape of the background will be achieved by adding a mask ontop of the FillMeterSkin by placing our FillMeterSkin and a normal BlockSkin with the mask in a DoubleSkin. Our FillMeterSkin only  needs to draw a rectangular blue bar of the right size and the black background.

This might seem easy and straight forward, but there is some padding to take into consideration, otherwise it will look wrong. If we just let it go from left edge to right edge of the widget it will start smaller and grow faster than it should since it will cover a longer distance than the center of the handle:

[images]

The result would be that the bar would start too small, catch up to the handle in the middle and then grow faster.

To correct this we need to make the bar start with the width from the left edge to the center of the handle at its leftmost position and then grow by the same amount the handle is moved. This is achieved by setting *gfxPadding* and *startLength*:

[image]

Now the slider background will look correct. and we would have a blueprint looking something like this:

[code]
