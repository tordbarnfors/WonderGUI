---
title: 'Skins'
weight: 40
---


# Skins

Skins are a very important part of WonderGUI and defines the look and (to some extent) the feel of a widget. A typical widget doesn't draw anything at all on the screen unless you apply a skin to it. All widgets have at least one skin, which is considered the background skin, covering the whole area of the widget. This is enough for most widgets, but some widgets have additional skins that covers different parts of the widget. A slider for example has two skins, one for the background and one for the handle.

Skins are very generic in that the same skin can be applied to widgets of all different kinds, we don't have a specific type of skin for say Buttons, although some skins have features that are only useful for certain categories of widgets. 

There are many kinds of skins to choose from. The most simple skin is just a filled rectangle and is suitable for backgrounds and such. Other skins draws bitmaps, boxes, circles and even pie-charts. More advanced skins change their look depending on the state of the widget (like a pressed Button) or the value of the widget (like the progress of a progress-bar or the rotation of a knob). Skins can be animated and layered to create advanced effects.

Skins are *Immutable Resource Objects* and are therefore created once, never modified and then used by any number of Widgets.



## Categories

Most skins falls into one of four different categories:

* **State skins:** Change their appearance depending on the state of the widget, such as if it is normal, pressed, selected, disabled, has keyboard focus etc. These are most commonly used.
* **Meter skins:** Change their appearance depending on the value of the widget and can be used for things such as progress indicators, value indicators, knobs etc.
* **Animation skins:** Skins that are animated and continuously change their appearance independently of state and value.
* **Static skins:** Skins that are not animated at all. 

A skin isn't limited to any of these categories and some falls entirely outside, such as CustomSkin, DoubleSkin and BakeSkin.



## Properties available in all skins

There are six properties that are available in all skins:

| Property  | Set in state | Description                                                  |
| --------- | ------------ | ------------------------------------------------------------ |
| finalizer | no           | A function that is called right before the Skin is destroyed. |
| layer     | no           | CanvasLayer that skin is draw in. Defaults to 0.             |
| margin    | no           | Border around skin.                                          |
| markAlpha | no           | Lowest alpha value that results in hovering and press.       |
| overflow  | no           | Allows for skin to be drawn outside its normal area.         |
| padding   | no           | Border around skin content inside skin.                      |

### *margin* and *padding*

Margin and padding both specifies *Borders* and deals with the geometry of the widget. Margin is an empty area around the skin that is not rendered while padding specifies the distance from the edge of the skin to the area where the content of the widget is drawn. In the case of a button, the content is the text and/or icon. For a container the content is where all child widgets are drawn. Although a widget can place content outside the content area and even in the margin, all current widgets refrain from doing so.

![skin-borders](/docs/manual/skin-borders.png)

### overflow

Overflow is the opposite of margin and allows the  skin to be rendered partly outside the widget. This is useful for certain effects such as a glow around the widget or a shadow under it. The overflowing areas of a widget is only for visual effects and cans not be interacted with using the mouse.

### *markAlpha*

MarkAlpha is simply a value that determines what alpha-value a pixel of the skin should have in order to be clickable. Setting this to 0 makes the whole skin (except for the margin) opaque to the mouse. Setting it to higher than 4096 makes it transparent to the mouse.

### *layer*

Layer simply specifies what layer to render the skin into when rendering to multiple layers. Layered rendering is both an experimental feature and a large topic on its own and will therefore not be covered here.



## Properties available in most skins

The following properties are available in most skins and works the same across them all.

### *blendMode*

The *blendMode* property specifies which blend mode to use when rendering the skin. Default is always *Blend*. Change this for various graphical effects. This is usually set for the whole skin and can't be  change with state or value.

### *color*

The color used for drawing or tinting the skin (for bitmap-based skins). This can usually be set per state in a StateSkin.

### *gradient*

Color gradient uses to tint the skin. If both color and gradient is defined, these are combined. This is usually set for the whole skin and can't be changed with state or value.



