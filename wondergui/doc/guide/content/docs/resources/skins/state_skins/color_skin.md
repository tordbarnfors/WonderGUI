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
auto pColorSkin1 = ColorSkin::create( { .color = Color8::White } );

auto pColorSkin2 = ColorSkin::create( { 
    .states ={ { State::Default, { .color = Color8::Green }},
               { State::Hovered, { .color = Color8::Yellow }},
               { State::Pressed, { .color = Color8::Red, .contentShift = {1,1} }},
               { State::Disabled, { .color = Color8::Gray }}
});

```

There are shorthand versions for specifying just a single color and for single parameter state specifications:

```c++
auto pColorSkin1 = ColorSkin::create( Color8::White );

auto pColorSkin2 = ColorSkin::create( { 
    .states ={ { State::Default, Color8::Green },
               { State::Hovered, Color8::Yellow },
               { State::Pressed, { .color = Color8::Red, .contentShift = {1,1}  },
               { State::Disabled, Color8::Gray }
});
```

