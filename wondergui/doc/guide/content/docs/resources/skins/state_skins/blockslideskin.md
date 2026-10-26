---
title: 'BlockSlideSkin'
weight: 60
---


# BlockSlideSkin

BlockSlideSkin is a special StateSkin where most state changes result in switching between blocks in a bitmap, but one specified state change results in sliding the block in a specified direction.

This skin was specially made to make iOS-style toggle buttons possible, where switching to and from  state *selected* results in a handle sliding left or right. To make that kind of toggle button you need to layer a skin providing a foreground on top of a *BlockSlideSkin* or use layered rendering with the BlockSlideSkin below a background with transparent cutouts for the BlockSlideSkins.



## Properties

| Property       | Set in state | Description                                                  |
| -------------- | ------------ | ------------------------------------------------------------ |
| blendMode      | no           | BlendMode used for blending the blocks to the background.    |
| blockLength    | no           | Length in (pts) of block to be displayed.                    |
| color          | yes          | Tint color for the blocks.                                   |
| firstLane      | no           | Rectangle for first block lane. Other lanes are lined up next to it (down or to the right). Mandatory. |
| slideDirection | no           | The direction the block will slide along the lane when widget enters slideState. |
| slideDistance  | no           | The distance (in pts) graphics will slide when entering slideState. |
| slideDuration  | no           | Duration of slide in milliseconds. Defaults to 250.          |
| slideState     | no           | The state that triggers sliding. Defaults to StateBits::Selected. |
| spacing        | no           | Spacing between lanes in the surface.                        |
| surface        | no           | Surface containing the lanes.                                |
| blockless      | only         | Specified state has no block lane, just other properties.    |



## Concept

The concept of BlockSlideSkin is similar to BlockSkin in that you have a surface with multiple blocks of graphics lined up next to each other and switch between them depending on the state of the widget.

Now instead of thinking of these as blocks, think about them as lanes that a block that has the same width, but shorter length than the slide can slide along. The lanes are switched depending on the state of the widget, just like blocks are switched in BlockSkin, except for one specific state where we instead slide the block across the lane to the other side. This is called the *slideState* and is by default set to *Selected*, making the block slide along the lane to switch side when widget enters or leaves state Selected.

A BlockSlideSkin can be used on its own if you have very square graphics, but typically you need to put a cut-out on top of it to cover parts of the lane that should not be seen. This can be done by placing the BlockSlideSkin in a DoubleSkin with a normal BlockSkin that has a transparent cut-out.

Alternatively you can use *CanvasLayers* to render your BlockSlideSkin in a lower layer and put a background with cut-outs for all your sliders in an upper layer. That is an advanced, experimental feature and a separate subject.



## Configuration

Let's configure a typical iOS-style toggle button. We start with a surface that looks like this:

![sliding_statebutton](/docs/manual/sliding_statebutton.png)

This surface has three "lanes" lined up vertically. The top one is for state *Default*, the second for *Hovered* and the third for *Disabled*. These needs to be lined up vertically since we will slide the block horizontally. If we were to create sliders that would slide vertically, we would need to line them up horizontally. In other words, the lanes needs to be placed next to each other, not after each other.

The size of this particular surface is 132x108 points, i.e. the three lanes are 132x35 points each.

We start by defining the lanes in the blueprint. We do so by defining a rectangle for the first lane, any spacing between the lanes and which states we have lanes for:

```c++
.firstLane = {0,0,132,35},
.spacing = 0,
.states = { State::Default, State::Hovered, State::Disabled }
```

Actually, in this case we don't really need to specify firstLane since BlockSlideSkin will calculate size of firstLane if not defined, assuming that the lanes covers the whole surface.

Secondly we define the length of the block and which direction it should slide when widget enters the state set in *slideState*:

```c++
.blockLength = 84,
.slideDirection = Direction::Left,
```

Assuming that we are in *Default* state, we now have a block that is 84 points long at the right edge of the first lane (so that it can slide to the left):

[image]

By default, slideState is set to *Selected*, which means that once the widget is selected, the block starts sliding to the left for *slideDuration* milliseconds before it reaches the left edge. This will make the grey knob of the widget appear to move to the ***right*** and the red background section move out while the green section moves in.

Other states can be changed as a slide is in progress. The blit source rectangle will simply "switch lanes" and continue sliding on another lane. 



## Examples

```c++
auto pSliderSkin = BlockSlideSkin::create({
	.blockLength = 84,
	.slideDirection = Direction::Left,
	.slideDuration = 100,
	.slideState = StateBits::Selected,
	.spacing = 0,
	.states = { State::Default, {}, State::Hovered, {}, State::Disabled, {} },
	.surface = pSliderSurf 
});
```

