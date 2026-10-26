---
title: 'CircleSkin'
weight: 50
---


# CircleSkin

CircleSkin simply draws a circle in the center of the area to be skinned. The circle has a color, size (relative to the size of the surrounding box) and thickness that can be set. It can also have an outline of any thickness and color.

The outline is also drawn on the inside of the circle, unless the circle is filled. Color, size and thickness can be set individually per state.

[image]



## Properties

| Property         | Set in state | Description                                                  |
| ---------------- | ----------- | ------------------------------------------------------------ |
| blendMode        | no          | BlendMode used for circle and its outline.                   |
| color            | yes | Color of circle. Defaults to White.                          |
| outlineColor     | yes | Color of outline of the circle. Defaults to Black.           |
| outlineThickness | yes | Thickness (in pts) of outline. Defaults to 0.                |
| size             | yes | Size of circle relative to skin box in the range 0.0 -> 1.0. Includes the outlines. Deaults to 1.0. |
| thickness        | yes | Thickness of the circle in pts. Setting this to more than the radius of the circle fills the circle. |

## Examples

Creating a static, white circle with a pink border:

```c++
auto pCircleSkin = CircleSkin::create({ 
  .color = Color::White, 
  .outlineThickness = 2,
  .outlineColor = Color::Pink,
  .thickness = 10 });
```

A Green circle that turns yellow and grows slightly when hovered and turns red when pressed:

```c++
auto pCircleSkin = CircleSkin::create({
  .outlineThickness = 2,
  .size = 0.9f,
  .states ={{ State::Default, { .color = Color::Green, 
                                .outlineColor = Color::DarkGreen }},
            { State::Hovered, { .color = Color::Yellow, 
                                .outlineColor = Color::DarkYellow,
                              	.size = 1.f }},
            { State::Pressed, { .color = Color::Red, 
                                .outlineColor = Color::DarkRed }},
            { State::Disabled,{ .color = Color::Gray, 
                                .outlineColor = Color::DarkGray }},
  .thickness = 10
});
```



There are shorthand versions for setting states with only one parameter or just color and outlineColor. The previous example can therefore be shortened to:

```c++
auto pCircleSkin = CircleSkin::create({
  .outlineThickness = 2,
  .size = 0.9f,
  .states ={{ State::Default, Color::Green, Color::DarkGreen },
            { State::Hovered, { .color = Color::Yellow, 
                                .outlineColor = Color::DarkYellow,
                              	.size = 1.f }},
            { State::Pressed, Color::Red, Color::DarkRed },
            { State::Disabled, Color::Gray, Color::DarkGray },
  .thickness = 10
});
```
