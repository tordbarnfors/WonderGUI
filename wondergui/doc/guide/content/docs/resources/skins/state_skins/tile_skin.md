---
title: 'TileSkin'
weight: 40
---


# TileSkin

TileSkin renders graphics by tiling a Surface over the widgets area. Due to technical reasons the whole surface is tiled, you can not select just a portion of it to tile.

A gradient can be set for the whole skin. The surface to tile and a color used for tint can be set per state.



## Properties

| Property  | Set in state | Description                    |
| --------- | ----------- | ------------------------------ |
| blendMode | no          | BlendMode to use for the skin. |
| color     | yes | Tint color for the surface.    |
| gradient  | no          | Tint gradient for the surface. |
| surface   | yes | Surface to tile.               |



## Examples

```c++
auto pTileSkin1 = TileSkin::create({ .surface = pSurface });

auto pTileSkin2 = TileSkin::create({
  .states ={{ State::Default, { .color = Color::LightGray, .surface = pDefaultSurface },
            { State::Hovered, { .surface = pDefaultSurface },
            { State::Pressed, { .contentShift = { 1, 1 }, 
                                .surface = pPressedSurface }},
            { State::Disabled,{ .surface = pDefaultSurface }}
});

```



There are shorthand versions for specifying just a single surface with an optional color and for single parameter state specifications:

```c++
auto pTileSkin1 = TileSkin::create(pSurface);

auto pTileSkin2 = TileSkin::create({
  .states ={{ State::Default, { .color = Color::LightGray, .surface = pDefaultSurface },
            { State::Hovered, pDefaultSurface },
            { State::Pressed, { .contentShift = { 1, 1 }, 
                                .surface = pPressedSurface }},
            { State::Disabled, pDefaultSurface }
});
```
