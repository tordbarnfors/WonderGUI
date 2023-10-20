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
| outlineColor     | yes | Color of outline of the box. Defaults to Black.  |
| outlineThickness | no          | Thickness (in pts) of outline. Defaults to 1.    |

## Examples

```c++
auto pBoxSkin1 = BoxSkin::create({ 
  .color = Color::White, 
  .outlineThickness = 2,
  .outlineColor = Color::Pink });

auto pBoxSkin2 = BoxSkin::create({
  .outlineThickness = 2,
  .states ={{ State::Default, { .color = Color::Green, 
                                .outlineColor = Color::DarkGreen }},
            { State::Hovered, { .color = Color::Yellow, 
                                .outlineColor = Color::DarkYellow }},
            { State::Pressed, { .color = Color::Red, 
                               .contentShift = { 1, 1 }, 
                               .outlineColor = Color::DarkRed }},
            { State::Disabled, { .color = Color::Gray, 
                                 .outlineColor = Color::DarkGray }}
});

```

There are shorthand versions for creating a skin with all parameters (+ optional padding) and states with color and outline color:

```c++
auto pBoxSkin1 = BoxSkin::create( 2, Color::White, Color::Pink );

auto pBoxSkin2 = BoxSkin::create({
  .outlineThickness = 2,
  .states ={{ State::Default, Color::Green, Color::DarkGreen },
            { State::Hovered, Color::Yellow, Color::DarkYellow },
            { State::Pressed, {.color = Color::Red, 
                               .contentShift = { 1, 1 }, 
                               .outlineColor = Color::DarkRed }},
            { State::Disabled,Color::Gray, Color::DarkGray }
});
```
