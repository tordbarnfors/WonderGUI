---
title: 'TileSkin'
weight: 40
---


# TileSkin

TileSkin renders graphics by tiling a Bitmap over the widgets area. Due to technical reasons the whole bitmap is tiled, you can not select just a portion of it to tile.

A gradient can be set for the whole skin. The bitmap to tile and a color used for tint can be set per state.



## Properties

| Property  | Set in state | Description                    |
| --------- | ----------- | ------------------------------ |
| blendMode | no          | BlendMode to use for the skin. |
| color     | yes | Tint color for the bitmap.    |
| gradient  | no          | Tint gradient for the bitmap. |
| bitmap   | yes | Bitmap to tile.               |



## Examples

```c++
auto pTileSkin1 = TileSkin::create({ .bitmap = pBitmap });

auto pTileSkin2 = TileSkin::create({
  .states ={{ State::Default, { .color = Color::LightGray, .bitmap = pDefaultBitmap },
            { State::Hovered, { .bitmap = pDefaultBitmap },
            { State::Pressed, { .contentShift = { 1, 1 }, 
                                .bitmap = pPressedBitmap }},
            { State::Disabled,{ .bitmap = pDefaultBitmap }}
});

```



There are shorthand versions for specifying just a single bitmap with an optional color and for single parameter state specifications:

```c++
auto pTileSkin1 = TileSkin::create(pBitmap);

auto pTileSkin2 = TileSkin::create({
  .states ={{ State::Default, { .color = Color::LightGray, .bitmap = pDefaultBitmap },
            { State::Hovered, pDefaultBitmap },
            { State::Pressed, { .contentShift = { 1, 1 }, 
                                .bitmap = pPressedBitmap }},
            { State::Disabled, pDefaultBitmap }
});
```
