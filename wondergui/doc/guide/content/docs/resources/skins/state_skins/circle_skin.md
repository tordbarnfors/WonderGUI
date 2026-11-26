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
  .color = Color8::White, 
  .outlineThickness = 2,
  .outlineColor = Color8::Pink,
  .thickness = 10 });
```

A Green circle that turns yellow and grows slightly when hovered and turns red when pressed:

```c++
auto pCircleSkin = CircleSkin::create({
  .outlineThickness = 2,
  .size = 0.9f,
  .states ={{ State::Default, { .color = Color8::Green, 
                                .outlineColor = Color8::DarkGreen }},
            { State::Hovered, { .color = Color8::Yellow, 
                                .outlineColor = Color8::DarkYellow,
                              	.size = 1.f }},
            { State::Pressed, { .color = Color8::Red, 
                                .outlineColor = Color8::DarkRed }},
            { State::Disabled,{ .color = Color8::Gray, 
                                .outlineColor = Color8::DarkGray }},
  .thickness = 10
});
```



There are shorthand versions for setting states with only one parameter or just color and outlineColor. The previous example can therefore be shortened to:

```c++
auto pCircleSkin = CircleSkin::create({
  .outlineThickness = 2,
  .size = 0.9f,
  .states ={{ State::Default, Color8::Green, Color8::DarkGreen },
            { State::Hovered, { .color = Color8::Yellow, 
                                .outlineColor = Color8::DarkYellow,
                              	.size = 1.f }},
            { State::Pressed, Color8::Red, Color8::DarkRed },
            { State::Disabled, Color8::Gray, Color8::DarkGray },
  .thickness = 10
});
```
