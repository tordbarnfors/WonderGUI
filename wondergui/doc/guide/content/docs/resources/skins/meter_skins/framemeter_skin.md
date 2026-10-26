---
title: 'FrameMeterSkin'
weight: 20
---


# FrameMeterSkin

FrameMeterSkin contains a bitmap animation consisting of two or more frames. All the frames must come from the same surface and have the same size.

The widget provided single value determines the offset into the animation to display as a fraction of the animations total length. The length of each frame can be set individually to make animations that lingers longer on certain frames. Frames can also be rotated in steps of 90 degrees and flipped, making it possible to reuse the same pixels for multiple frames in certain kinds of animations.

## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| blendMode | BlendMode used. |
| color | Tint color for the frames. |
| frames | A vector with the frames of the animation. |
| gfxPadding | Like contentPadding, but for the animation. |
| gradient | Tint gradient for the frames. |
| size | Size of the animation frames (pts). |
| surface | Surface containing the frames for the animation |

The individual frames have the following properties:

| Property | Description                                                  |
| -------- | ------------------------------------------------------------ |
| pos      | Position (pts) of the top left corner of pixels in the surface to use for this frame. |
| duration | Relative duration of this frame. Must be >= 1, defaults to 1. |
| flip     | The rotation/flipping of the frame. Defaults to None.        |



## Examples

Let's say we have a surface of 3 animation frames all 16x16 pixels and lined up horizontally with no spacing between them. The easiest way to create them would look like this:

```c++
auto pSkin = FrameMeterSkin::create({
    .frames = { {{0,0}} , {{0,16}}, {{0,32}} },
    .size = {16,16}, 
    .surface = pSurface 
});
```

We here use the shorthand form for the frames, just specifying the coordinate for each frame. This can be followed by duration and flip.

However if we have many frames it is usually easier to create the blueprint outside the create function and fill in the frames using a loop:

```c++
FrameMeterSkin::Blueprint bp = { .size = {16,16}, .surface = pSurface };

for (int i = 0; i < 20; i++)
		bp.frames.push_back({ .pos = {0,i*16.f} });

auto pSkin = FrameMeterSkin::create(bp);
```

There is shorthand for specifying the frames with coordinate and optionally duration and flip:

```c++
bp.frames.push_back({ {0,i*16.f}, 1, GfxFlip::Rot90 });
```

