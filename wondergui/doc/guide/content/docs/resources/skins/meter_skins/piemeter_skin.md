---
title: 'PieMeterSkin'
weight: 40
---


# PieMeterSkin

PieMeterSkin can draw pie charts and donut charts. This is more useful than it sounds since a donut chart can be used for rounded meters or progress indicators in a GUI.

PieMeterSkin therefore have many settings which don't make much sense for a normal pie chart but makes it more useful for animated meters.



## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| backColor | Color for background of pie-meter. Defaults to transparent. |
| blendMode | BlendMode used for all rendering, including slices, hub and background. |
| defaultSize | Default size for this skin. |
| emptyColor | Color for the "empty" slice of the piechart where there are no slices. Defaults to DarkBlue. |
| gfxPadding | Like contentPadding but for the pie-chart graphics. |
| hubColor | Color for the hole at the center of a donut chart. Defaults to transparent. |
| hubSize | Size of the hole at the center of a donut chart, expressed as a fraction of the radius in the range of 0.0 - 1.0. Defaults to 0.75. Set to 0.0 to create a pie chart. |
| length | Length in degrees of the meter. 360 is a full pie. |
| min | Fraction of meter filled when value is at its lowest. 0.0 - 1.0. |
| movingSlices | Set to true if slices should move as the pie grows. Otherwise they represent static areas, gradually being filled as the pie grows. |
| square | Set to true to make outside of pie/donut rectangular. |
| rotation | Rotation of pie/donut in degrees. |
| slices | Array of slices for the donut, including their relative sizes and colors. |

Each slice has the following properties:

| Property | Description                                                  |
| -------- | ------------------------------------------------------------ |
| size     | Size of this slice relative to the rest. The size of all slices is summed up to become a total for a full circle pie. |
| minColor | Color of this slice when value of widget is 0.0.             |
| maxColor | Color of this slice when value of widget is 1.0.             |



## Designing the pie

The pie always occupies a square area. If the skin covers an area that isn't square, the largest possible square in the center of the area is used for the pie.

The pie can be made to look either like a normal pie or be donut shaped simply by setting the *hubSize*, which determines the size of the hole in the middle of the pie.

The pie can be made square instead of circular by setting *square* to true. That only affects the outer edge, not the hub which always remains circular. 

The pie does not need to be a full circle. The *max* value determines the fraction of the pie that will be filled for a widget provided value of 1.0 and the min value determines the fraction of the pie that is always filled. This way you can easily make a meter that covers 1/3 of a circle.

You might find it useful to rotate the pie, this can be done through the *rotation* property. By default the pie starts at 0 degrees.

Except for the color of the slices, there are  three colors that can be set, *backColor*, *hubColor* and *emptyColor*.

The *hubColor* is the color used for the hub (if any). This makes it possible to have a clearly visible hub instead of just a hole for the background. 

The *emptyColor* is the color to use for the section of the pie that would contain slices at maximum value but currently isn't. This makes it possible to clearly show how far a meter goes.

The *backColor* is used for every part of the skin that isn't covered by anything else. It also fills the hub and empty section if hubColor and emptyColor is set to transparent.



## Setting up slices

The slices of the pie defines differently colored sections. Each section has a *size*, which is its relative size compared to all the other slices when the piechart is at its max and two colors, minColor and maxColor, which are the colors for the section when pie is at its minimum size and maximum size respectively.

Since size is a relative value, you can use any range of positive values as you want. A slice with twice the value of another is twice as big, but usual convention is to set values in either the range 0.0 - 1.0 or in degrees.

The first slice in the *slices* array is the first to fill up as value starts at min and the last slice then represents the top values.

The parameter *movingSlices* determines the behavior as the pie grows. By default slices are not moving, which means that the different sections of colors they represent fills upp one by one, starting with the first. 

If *movingSlices* is set, everything looks the same for a pie chart at its full size, but if the pie shrinks, the sections starts to move backwards instead of disappearing from the top. This results in the last pie in the slice being fully visible the longest while earlier entries gradually disappears.



## Examples



The following example makes a 120 degree donut-style meter:

```c++
auto pSkin = PieMeterSkin::create({
				.backColor = Color::Black,
				.emptyColor = Color::DarkGrey,
				.hubColor = Color::Transparent,
				.hubSize = 0.75f,
				.length = 120,
				.min = 0.1f,
				.rotation = -60,
				.slices = { 1.f, Color::DarkBlue, Color::Blue }
			});
```

It results in the following output for widget values 0.0, 0.5 and 1.0:

![piemeter-example-1](/docs/manual/piemeter-example-1.png)

Please note that for value 0.5 it doesn't point straight up since 0 is at 10% of the length, which results in 0.5 being at 55% of the length. That is something that has to be taken into consideration in the design.

The following example is identical except that it divides up the meter into three slices of one third each:

```c++
auto pSkin = PieMeterSkin::create({
				.backColor = Color::Black,
				.emptyColor = Color::DarkGrey,
				.hubColor = Color::Transparent,
				.hubSize = 0.75f,
				.length = 120,
				.min = 0.1f,
				.rotation = -60,
				.slices = { {1.f, Color::Green, Color::Green},
							{1.f, Color::Yellow, Color::Yellow},
							{1.f,Color::Red,Color::Red} }
			});
```

Which results in the following output:

![piemeter-example-2](/docs/manual/piemeter-example-2.png)

Lastly, this example creates a fan-like meter with a colored hub where the slices are defined so it has a "head" of brighter slices in the front using the *movingSlices* property:

```c++
auto pSkin = PieMeterSkin::create({
                .backColor = Color::Black,
                .emptyColor = Color::DarkGrey,
                .hubColor = Color::Yellow,
                .hubSize = 0.25f,
                .length = 120,
                .min = 0.1,
                .movingSlices = true,
                .rotation = -60,
                .slices = { {0.7f,Color::DarkBlue,Color::DarkBlue},
                            {0.2f,Color::Blue,Color::Blue},
                            {0.1f,Color::LightBlue,Color::LightBlue} }
    });
```

Which results in:

![piemeter-example-3](/docs/manual/piemeter-example-3.png)
