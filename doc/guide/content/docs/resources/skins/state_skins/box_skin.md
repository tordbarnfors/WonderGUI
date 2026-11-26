---
title: 'BoxSkin'
weight: 20
---


# BoxSkin

BoxSkin is slightly more advanced than ColorSkin. With BoxSkin your filled rectangle has an outline of configurable thickness and color. The thickness of the outline is set for the whole skin, while the color of the outline can be changed per state.

## Properties

| Property         | Set in state | Description                                      |
| ---------------- | ----------- | ------------------------------------------------ |
| color            | yes | Color of interior of the box. Defaults to White. |
| cornerRadius     | no  | Radius (in pts) of the corners, either one value for all corners or four values clockwise from top-left. Defaults to 0 (square corners). |
| outlineColor     | yes | Color of outline of the box. Defaults to Black.  |
| outlineThickness | no          | Thickness (in pts) of outline. Defaults to 1.    |

Rounded corners are drawn with subpixel precision and antialiased, so a rounded BoxSkin looks the same at any scale. Radii that don't fit the box are scaled down together, so a radius of at least half the height of the box gives it round ends (a pill). The inner corners of the outline get the radius of the corner minus the thicker of the two sides meeting there, or are square if that side is thicker than the radius.

## Examples

```c++
auto pBoxSkin1 = BoxSkin::create({ 
  .color = Color8::White, 
  .outlineThickness = 2,
  .outlineColor = Color8::Pink });

auto pBoxSkin2 = BoxSkin::create({
  .outlineThickness = 2,
  .states ={{ State::Default, { .color = Color8::Green, 
                                .outlineColor = Color8::DarkGreen }},
            { State::Hovered, { .color = Color8::Yellow, 
                                .outlineColor = Color8::DarkYellow }},
            { State::Pressed, { .color = Color8::Red, 
                               .contentShift = { 1, 1 }, 
                               .outlineColor = Color8::DarkRed }},
            { State::Disabled, { .color = Color8::Gray, 
                                 .outlineColor = Color8::DarkGray }}
});

```

A button with rounded corners, a thicker outline to the right and below for a hint of depth:

```c++
auto pButtonSkin = BoxSkin::create({
  .color = Color8::LightBlue,
  .cornerRadius = 6,
  .outlineColor = Color8::DarkBlue,
  .outlineThickness = { 1, 2, 2, 1 } });
```

There are shorthand versions for creating a skin with all parameters (+ optional padding) and states with color and outline color:

```c++
auto pBoxSkin1 = BoxSkin::create( 2, Color8::White, Color8::Pink );

auto pBoxSkin2 = BoxSkin::create({
  .outlineThickness = 2,
  .states ={{ State::Default, Color8::Green, Color8::DarkGreen },
            { State::Hovered, Color8::Yellow, Color8::DarkYellow },
            { State::Pressed, {.color = Color8::Red, 
                               .contentShift = { 1, 1 }, 
                               .outlineColor = Color8::DarkRed }},
            { State::Disabled,Color8::Gray, Color8::DarkGray }
});
```
