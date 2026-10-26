---
title: 'SpinMeterSkin'
weight: 30
---


# SpinMeterSkin

Displays a surface that is rotated according to the widget provided single value and properties containing the center of rotation, angles to rotate between and size and placement within area to render.

## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| angleBegin | The rotation angle in degrees for value 0.0. Defaults to 0. |
| angleEnd | The rotation angle in degrees for value 1.0. Defaults to 360. |
| blendMode | BlendMode used. |
| color | Tint color for the surface. |
| defaultSize | Default size for this skin. |
| gfxPadding | Like contentPadding, but for the graphics of the skin. |
| gradient | Tint gradient for the frames. |
| pivot | Center of rotation in the surface, expressed as coordinates in range 0.0 -> 1.0. Defaults to {0.5, 0.5}, placing the pivot in the center of the surface. |
| placement | Where the pivot of the surface should be placed within the skin, expressed as coordinates in range 0.0 -> 1.0. Defaults to {0.5, 0.5}, placing the pivot of the surface in the center of the render rectangle. |
| surface | The surface to rotate. |
| zoom | A zoom factor for the surface. 1.0 means that it will be stretched so that it nice fits the destination when not rotated. Defaults to 1.0. |

## Designing graphics to rotate

The surface containing the graphics to rotate should have transparent pixels around the edges to avoid certain kind of smearing effects on certain graphics backends.

## Center of rotation

The center of rotation is controlled by two properties: *pivot* and *placement*. 

Imagine that your surface and the render rectangle are cardboard cutouts. The pivot marks the position in the surface where you stick through a needle and placement marks the position on the render rectangle where you attach it.

Both pivot and placement are float coordinates in the range 0.0 - 1.0 to make things simple.

This way you can not only rotate a large surface around its center, you can e.g. make a clock hand that is standing tall and thin, set the pivot to the bottom of the hand and set placement at the center.

## Controlling rotation

Rotation for values 0.0 and 1.0 are expressed in degrees and defined by properties *angleBegin* and *angleEnd*.  To rotate the surface anti-clockwise you simply give angleEnd a larger value than angleBegin. Values below 0 and above 360 are allowed to facilitate this.

A distance between angleEnd and angleBegin larger than 360 degrees is also allowed and will make the surface spin more than full circle over the full value range.

## Zooming

The *zoom* property can be used to make your graphics larger or smaller. A zoom factor or 1.0 means that the surface will be scaled to nicely fit the render rectangle when not rotated.



## Examples

