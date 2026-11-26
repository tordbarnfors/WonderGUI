---
title: 'ColorSkin'
weight: 10
---


# ColorSkin

ColorSkin is the most simple StateSkin. It fills the whole are with a single color, which can be redefined for each state.

## Properties

| Property  | Set in state | Description                 |
| --------- | ----------- | --------------------------- |
| blendMode | no          | BlendMode to use for fill.  |
| color     | yes | Tint color for the surface. |

## Examples

```c++
auto pColorSkin1 = ColorSkin::create( { .color = Color::White } );

auto pColorSkin2 = ColorSkin::create( { 
    .states ={ { State::Default, { .color = Color::Green }},
               { State::Hovered, { .color = Color::Yellow }},
               { State::Pressed, { .color = Color::Red, .contentShift = {1,1} }},
               { State::Disabled, { .color = Color::Gray }}
});

```

There are shorthand versions for specifying just a single color and for single parameter state specifications:

```c++
auto pColorSkin1 = ColorSkin::create( Color::White );

auto pColorSkin2 = ColorSkin::create( { 
    .states ={ { State::Default, Color::Green },
               { State::Hovered, Color::Yellow },
               { State::Pressed, { .color = Color::Red, .contentShift = {1,1}  },
               { State::Disabled, Color::Gray }
});
```

