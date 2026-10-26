---
title: 'Gallery'
weight: 40
---


# Widget gallery



## Panels

*Panels are used for managing the layout of widgets.*

### RootPanel

RootPanel is not technically a panel, but reminiscent of one. It accepts exactly one child and needs to be at the root of any widget hierarchy that should be displayed and interacted with.

### PackPanel

Children are lined up horizontally or vertically, depending on the setting of the Panel. The size of each child is determined by some simple rules provided by a PackLayout-object. Most complex layouts can easily be achieved by simply putting PackPanels inside PackPanels and carefully selecting the rules.

### FlexPanel

Children can either have their corners pinned to locations of the panel (such as topleft corner or bottom center) and stretch with the panel, or be placed and moved freely. Children can overlap and their overlap-order can be changed. FlexPanel's are very flexible in the layout of their children but should mainly be used at the very top of a hierarchy since they don't propagate the need to resize from their children to their parent.

### LambaPanel

The geometry of each child is decided through a callback that is called whenever the parent is resized or requests a resize. This allows for very flexible rule-based layouts. It has the same downside as FlexPanel in that childrens need to resize isn't propagated to parents.

### StackPanel

Children are stacked, placed ontop of each other in the same area, partially or fully covering each other. Some layout rules allows for the placement of smaller children and rearranging the order.

### ScrollPanel

Allows for content that is bigger than the panel itself by having one child that can be scrolled around inside the panel. Optionally scrollbars are included.

### SplitPanel

The area is split, either horizontally or vertically, between two children. A handle is placed between the two areas through which the user can adjust the size of the children.

### TablePanel

Arranges children in rows and columns. Rows and columns can be resized using PackLayout-objects, just like PackPanels.

### TwoSlotPanel

Simpler version of PackPanel where only two slots are present. Faster with less overhead.



## Lists

*Lists provides layouts similar to panels, but also provides functionality for the user to select/unselect children using mouse and keyboard.*

### PackList

Children are lined up horizontally or vertically, depending of the setting of the Panel. The list can include a clickable header with sort-functionality.



## Overlays

*Overlays provide for overlaying the hierarchy with certain widgets that exists outside the hierarchy, such as pop-up menus, modal dialogs, tooltips and notifications. They are typically placed near the root in the hierarchy and are necessary for certain aspects of the GUI to work. They can also be placed further down the hierarchy, e.g. putting a ModalOverlay over a window in a GUI that looks like a desktop can provide dialog boxes that are modal only to the content of that window.*

### DragNDropOverlay

Provides drag-n-drop functionality and is necessary for drag-n-drop to work in the GUI. The widget actually being dragged around before being dropped is drawn in the overlay.

### ModalOverlay

Putting a widget into a ModalOverlay blocks all the widgets under the ModalOverlay from receiving any keyboard or mouse input until the widget has disappeared.

### PopupOverlay

Any widget opening a popup (e.g. SelectBox, PopupOpener) will place the popup in the closest ancestor PopupOverlay. A PopupOverlay is necessary for these widgets to work.

### TooltipOverlay

Any widget displaying a tooltip will do so in the closest ancestor TooltipOverlay. 

### DesignOverlay

A work in progress. Intended to become a simple GUI-designer in an overlay. Allowing for manipulation of the GUI in realtime.



## Capsules

*A Capsule only has one child and is placed in the widget hierarchy to modify the characteristics of it, thus affect the branch below it in various ways.*

### BlockingCapsule

Allows for fine control of which widgets or rectangular areas accepts mouse interaction. Can be used for interactive guides where user is only allowed to manipulate some widgets.

### CanvasCapsule

The child and the rest of the branch is rendered into a separate canvas Surface, from which graphics is copied when widgets need to be redrawn. This can significantly speed up rendering of complex and detailed (parts of) GUI when these widgets needs to be redrawn more often than they change, e.g. when they are above animated backgrounds or moved around a lot.

### LabelCapsule

Used to display a frame with a label around a group of widgets.

### RenderLayerCapsule

Allows for switching the renderlayer so that the rest of the branch is rendered into a different layer.

### ReorderCapsule

A special capsule that allows for easy rearranging of children in a PackPanel or moving children between PackPanels through drag-n-drop. It takes a single PackPanel as its child and handles movement of children within the PackPanel through drag-n-drop and transitions. The PackPanel's children can also be moved to another PackPanel inside a ReorderCapsule.

### ScaleCapsule

Allows for setting a different scale for the widgets in the rest of the branch, making them bigger or smaller.

### SizeCapsule

Overrides the widgets default, min and max size with specific values. Can be used to force or limit the widget to certain sizes, depending on its parent.





## Buttons

### Button

Basic push-button. 

### PopupOpener

Opens a pop-up widget next to itself when pressed.

### ToggleButton

Toggles between selected/unselected when pressed. Used as Checkboxes and Radiobuttons.



## Text widgets

### TextDisplay

Displays multi-line text with styles and layout. Doesn't scroll text that doesn't fit. Put inside a ScrollPanel if you want that.

### TextEditor

Like TextDisplay, but with cursor that allows editing of text. Works great with simple, non-styled text, but can also display different styles, although it doesn't provide any controls for editing style. Need to be complemented with other widgets for that.

### LineEditor

Displays a single line of text that can be edited. Scrolls text as necessary to keep cursor in view, therefore doesn't need any ScrollPanel.

### NumberDisplay

Displays a value in textual form, using a NumberLayout object to format the value into text, can therefore be set to display just a numeric value, a roman value or a date and time depending on the NumberLayout-class used.



## Knobs & sliders

### Knob

Press and modify a value by moving the mouse. Widget uses a meter skin, so widget can look anything like a wheel, piechart, VU-meter, slider etc.

### Slider

Simple slider widget. Moves a handle along a horizontal or vertical line. 

### RangeSlider

A slider where you set the beginning and end of a range using two handles.

### Scrollbar

Typical scrollbar with optional buttons at the ends.



## Select widgets

### SelectBox

Displays a text-entry with a drop-down menu from which to select entries.



## Displays

### CanvasDisplay

Similar to SurfaceDisplay but displays the content of a CanvasCapsule instead. Useful for live thumbnails and such.

### SurfaceDisplay

Displays the content of a Surface. Display is automatically updated when content of surface is modified. Surface can be zoomed and panned.



## Simple, non-interactive widgets

### Filler

A minimal widget with no functionality. You can set its default size and skin. Used mainly to create spacing between other widgets in PackPanels.

### Image

Simple display of an image. Useful for icons and such.

### Timer

A widget that simply keeps track of time. Combine with Animation-skins to create animations.



## Chart widgets

*There are two kinds of charts in WonderGUI normal charts and scrolling charts. Scrolling charts move constantly from right to left and you fill in new values on the right.* 

### PlotChart

Displays data from samples as a series of plotted circles.

### AreaChart 

Displays data from samples as one or two lines from left to right where the samples are the height of the lines along the chart and the are between them is filled.

### AreaScrollChart

Similar to AreaChart, but the content is scrolled and new samples are added on the right side.



## Special widgets

### FPSDisplay

Displays the number of times Base::update() is called per second.

### AnimPlayer

Plays a bitmap animation.

### PianoKeyboard

Displays a piano keyboard with pressable keys.

### SideCanvas

Internally used by certain widgets (e.g. SelectBox when the menu is opened) to open a secondary rectangle on screen for drawing. Not to be used explicitly.
