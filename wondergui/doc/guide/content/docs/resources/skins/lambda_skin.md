---
title: 'LambdaSkin'
weight: 40
---


# LambdaSkin

LambdaSkin is a simple skin where rendering and mark testing is performed by supplied lambda expressions.

This makes it easy to make a simple skin with custom graphics. it doesn't support animation, but StateSkin and MeterSkin functionality is supported.

## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| ignoreState | Set to true if skin looks the same regardless of state. Defaults to false. |
| ignoreValue | Set to false if skin looks the same regardless of value parameters. Defaults to false. |
| markTestFunc | Lambda expression for testing skin for mouse hovering. |
| opaque | Set to true if skin will be totally opaque under all circumstances. Used for rendering optimisations and mouse pointer testing. |
| renderFunc | Lambda expression for rendering skin. |
| defaultSize | Default size for this skin. |

## Rendering expression

The rendering expression has the following declaration:

```c++
void myRenderingFunction(GfxDevice* pDevice, const RectSPX& canvas, 
                         int scale, State state, float value1, float value2)
```

* pDevice - The device that should be used for drawing.
* canvas - The rectangle covered by the skin, measured in subpixels. Margin has already been deducted.
* scale - Scale of the widget. Number of pts for each subpixel. 64 = one point is one pixel.
* state - State of the widget.
* value1 - First value supplied by the widget. Range 0.0 - 1.0. Used by knobs, sliders, meters and such. Defaults to 1.0 for widgets that don't supply a value.
* Value2 - Second value supplied by widget and should be in range 0.0 - 1.0, OR -1 if not supplied by the widget.



When the function is executed, any specified layer has already been switched to and the GfxDevice is ready for draw-calls.

The function is not allowed to draw anything outside *canvas* since clip-rectangles are not guaranteed to be limited to that area.

Any change of state such as blendMode, tintColor etc should be returned to the previous state. Only blitSource may be left in a changed state.



## Mark test expression

Mark tests are done to determine if the pointer hovers above a certain widget or not.

The mark test expression has the following declaration:

```c++
bool myMarkTestgFunction(const CoordSPX& pos, const RectSPX& canvas, int scale, 
                         State state, float value1, float value2, int minAlpha )
```

* pos - The coordinate to markTest.
* canvas - The rectangle covered by the skin, measured in subpixels. Margin has already been deducted.
* scale - Scale of the widget. Number of pts for each subpixel. 64 = one point is one pixel.
* state - State of the widget.
* value1 - First value supplied by the widget. Range 0.0 - 1.0. Used by knobs, sliders, meters and such. Defaults to 1.0 for widgets that don't supply a value.
* Value2 - Second value supplied by widget and should be in range 0.0 - 1.0, OR -1 if not supplied by the widget.
* minAlpha - The minimum alpha value the marked position of the graphics should have in order to mark this skin. Range 1-4096 where 1 means that everything but completely transparent areas of the skin should be marked and 4096 means that only completely opaque areas should be marked.

If mark test expression is not specified, the *opaque* property determines if mark test should succeed or fail.

## Performance

By default a LambdaSkin triggers a full redraw of its area for any change in widgets state or values. To avoid this and increase performance you should set the *ignoreState* and *ignoreValue* properties to true when they are not used when rendering.

## Examples

Create a static white skin with a 6 pts thick green ring with 1 pts thick black outlines:

```c++
auto pMySkin = LambdaSkin::create({
    .opaque = true,
	.renderFunc = [](GfxDevice* pDevice,const RectSPX& canvas, 
                     int scale, State state, float value1, float value2)
	{
		pDevice->fill( canvas, Color::White );

		RingSlice slice = { .size = 1.f, .color = Color::Green };

		pDevice->drawRing({ .rect = canvas, .thickness = scale*6, .outline = scale*1,
		                    .nSlices = 1, .pSlices = &slice, .outlineColor = Color::Black });
	}
    .ignoreState = true,
    .ignoreValue = true
});

```

Create a meter skin similar to FillMeterSkin that gradually fills its canvas from left to right and where we only mark test the filled area (which doesn't make much sense, but makes a good example):

```c++
auto pMySkin = LambdaSkin::create({
	.markTestFunc = [](const CoordSPX& pos, const RectSPX& canvas, int scale,
                       State state, float value1, float value2, int minAlpha )
	{
		RectSPX canv = canvas;
		canv.w = canvas.w * value1;
		return canv.contains(pos);
	},
	.renderFunc = [](GfxDevice* pDevice,const RectSPX& canvas,
					 int scale, State state, float value1, float value2)
	{
		RectSPX canv = canvas;
		canv.w = canvas.w * value1;
		pDevice->fill( canv, Color::Green );
	},
	.ignoreState = true
});

```

