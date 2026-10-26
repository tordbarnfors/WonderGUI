---
title: 'SpinAnimSkin'
weight: 10
---


# SpinAnimSkin

SpinAnimSkins plays a continously looping animation where a surface is rotated.

It's very similar to SpinMeterSkin in properties and behavior, except that it rotates continuously and not sets rotation based on widgets value.



## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| angleBegin | Rotation angle in degrees for the start of a rotation cycle. Default 0. |
| angleEnd | Rotation angle in degrees for the end of a rotation cycle. Default 360. |
| blendMode | BlendMode used. |
| color | Tint color for the surface. |
| defaultSize | Default size for this skin n pts. |
| gfxPadding | Like contentPadding, but for the graphics of the skin.s |
| gradient | Gradient used for bar and its background. |
| pivot | Center of rotation in the surface, expressed as coordinates in range 0.0 -> 1.0. Defaults to {0.5, 0.5}, placing the pivot in the center of  the surface. |
| placement | Where the pivot of the surface should be placed within the skin,  expressed as coordinates in range 0.0 -> 1.0. Defaults to {0.5, 0.5}, placing the pivot of the surface in the center of the render rectangle. |
| returnTransition | Transition to use for return animation from *angleEnd* to *angleBegin*. |
| surface | The surface to rotate. |
| transition | Transition to use for animation from *angleBegin* to *angleEnd*. |
| zoom | A zoom factor for the surface. 1.0 means that it will be stretched so  that it nice fits the destination when not rotated. Defaults to 1.0. |

## Center of rotation

The center of rotation is controlled by two properties: *pivot* and *placement*. 

Imagine that your surface and the render rectangle are cardboard cutouts. The pivot marks the position in the surface where you stick through a needle and placement marks the position on the render rectangle where you attach it.

Both pivot and placement are float coordinates in the range 0.0 - 1.0 to make things simple.

This way you can not only rotate a large surface around its center, you can e.g. make a clock hand that is standing tall and thin, set the pivot to the bottom of the hand and set placement at the center.

## Controlling rotation

A rotation cycle starts at *angleBegin*, goes to *angleEnd* and returns back at *angleBegin*. The *transition* property decides the speed and any ease-in/ease-out from angleBegin to angleEnd. The *returnTransition* property decides the same for the retun to angleBegin.

If *transition* is not specified, a one second linear transition is performed. If returnTransition is  not specified, the animation will jump back to angleBegin and restart *transition* again once it reaches *angleEnd*. 

To create a continously rotating animation you set a linear *transition* of desired length, make sure distance between *angleBegin* and *angleEnd* is 360 and leave out *returnTransition*.

To rotate the surface anti-clockwise you simply give angleEnd a smaller value than angleBegin. Values below 0 and above 360 are allowed to facilitate this and animations that rotates several rounds before returning.

## Zooming

The *zoom* property can be used to make your graphics larger or smaller. A zoom factor or 1.0 means that the surface will be scaled to nicely fit the render rectangle when not rotated.



## Examples

Continous clockwise rotation of a surface at one revolution per two seconds:

```c++
auto pTransition = ValueTransition::create( 2000000, TransitionCurve::Linear );

auto pAnimSkin = SpinAnimSkin::create({
	.angleBegin = 0.f,
	.angleEnd = 360.f,
	.surface = pWheelSurf,
	.transition = pTransition
});
```

Animation of a hammer that rotates 90 degrees to hit an imaginary nail and then returns back for another round:

```c++
auto pTransition = ValueTransition::create( 500000, TransitionCurve::EaseIn );
auto pReturnTransition = ValueTransition::create( 500000, TransitionCurve::EaseOut );

auto pAnimSkin = SpinAnimSkin::create({
	.angleBegin = 0.f,
	.angleEnd = 90.f,
	.returnTransition = pReturnTransition,
	.surface = pHammerSurf,
	.transition = pTransition
});
```

