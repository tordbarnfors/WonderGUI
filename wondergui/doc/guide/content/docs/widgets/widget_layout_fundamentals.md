---
title: 'Layout fundamentals'
weight: 30
---


# Layout fundamentals



## Introduction

Essentially there are three widgets that can be used to build the main layout of an application, these are:

* PackPanel
* FlexPanel
* LambdaPanel

All of these provides the flexibility and control needed to make flexible layouts that can adapt to the size of the window. New users tend to instinctively go for FlexPanel or LambdaPanel, since these provide a more direct way to position widgets in fewer steps, while with PackPanels you need to build a hierarchy of PackPanels inside PackPanels and apply rules for resizing through PackLayout objects. That seems more complex, but  is often a better choice in the long term since it allows for a more responsive design where the layout better adapts to the needs of individual widgets.

In this chapter we will implement the same basic layout using either only PackPanels, FlexPanels or LambdaPanels and point out the strengths and weaknesses of each approach.

## The design to implement

The design we have chosen for our exercise is a simple two pane layout with a menubar on top and a column of buttons between the left and right pane. Think of a file utility similar to Norton Commander (PC DOS), Midnight Commander (Linux), Directory Opus (Amiga) or Kobold (Atari). if you are familiar with those old applications from the 80's and 90's. If not, don't worry and let us explain.

![2-pane-view](/docs/manual/2-pane-view.png)


The two main sections are the left and right pane. These are used to display the content of two directories as a list of files. Between them is a column of buttons containing actions such as copy, move and delete. Which will copy or move any selected files from one pane or the other or simply delete them.

The content of the menubar is irrelevant for this exercise, so is the content of the panes. We just need to acknowledge that we want a Slot on the top for the menubar, one to the left and one to the right for the panes and three in a column in the middle for the buttons. We should also acknowledge the padding between the slots and recreate this. We should also allow for the resizing of the window with some decent results, that is the panes should get all the extra space while the height of the menubar and size of the buttons remains the same.

## Implementing the design with a FlexPanel

The whole design can be implemented in one single FlexPanel. In FlexPanel you can specify position and size of individual Slots in both absolute positions and sizes as well as positions and sizes that are relative to the size of the FlexPanel. You can even combine relative positions with absolute offsets for adjustments which makes positioning very flexible and powerful.

### Pinned and movable

FlexPanels provide two modes for controlling the position and size for of a slot called *Pinned* and *Movable*. You can mix them and switch between them, but each individual slot is either Pinned or Movable at any given moment.

The best way to understand Pinned mode is to think of the FlexPanel as a rubber sheet that can be stretched along its X- and Y-axies. You then attach the slot, which also is a rubber sheet, to the FlexPanel with two pins, one in the top left corner of the slot and the other in the bottom right. Now when you stretch the FlexPanel the pinned slot also stretches.

In Movable mode you simply specify the position and size of the Slot in absolute terms, they don't change when you resize the FlexPanel and you can then move it around by updating the position. However, the origo from which the position is calculated can be specified with a pin. That way you can e.g. place a slot relative to the center or bottom-right of the FlexPanel, making it move appropriately when FlexPanel is resized, while size of the slot is not affected.

### Specifying a FlexPos

To make things slightly more complicated and much more flexible, the pin also contains an offset (in absolute points) that modifies its position. The position of the pin is specified through a tiny class called a *FlexPos* and can be created like this:

```c++
FlexPos({0.5f, 0.f}, {6,10} );
```

The first two values specify a relative position along the X- and Y-axis of the FlexPanel in the range 0.0 -> 1.0. The second two values specifies a coordinate (measured in points) that is added to the coordinate resulting from the relative positions and the size of the FlexPanel. The result in this case is a position that is 6 points down and 10 points to the left of the top center of the FlexPanel, no matter what size the FlexPanel is.

There are more ways to specify the position, like replacing the relative position with a Placement like this:

```c++
FlexPos( Placement::North, {6,10} );
```

But they translates to the same result, one relative coordinate in the range 0.0 -> 1.0 and one absolute coordinate for adjustment.

So, to create a slot that starts 10 pixels from the top-left corner of the FlexPanel and ends at the center you can do the following:

```c++
pFlexPanel->slots.pushBack( pMyWidget, {.pin1 = FlexPos( {0.f,0.f}, {10,10} ),
                                        .pin2 = FlexPos( {0.5f,0.5f},{0,0} ) });
```

Or, express the same in a more readable syntax, taking advantage of the Placement enum, parameter defaults and type deducation:

```c++
pFlexPanel->slots.pushBack( pMyWidget, {.pin1 = { Placement::NorthWest, {10,10} },
                                        .pin2 = { Placement::Center } });
```

### Adding the menubar

So now we know enough to correctly create a slot for the menubar:

```c++
pFlexPanel->slots.pushBack( pMyMenubar, {.pin1 = { Placement::NorthWest },
                                         .pin2 = { Placement::NorthEast, {0,20} } });
```

We pin the menubar to the top-left corner and a position 20 points down from the top-right corner. This give us a menubar that is always 20 points high but stretches from the left to the right of the window.

### Adding the panes

The two panes can be placed using the same technique:

```c++
pFlexPanel->slots.pushBack( pPane1, {.pin1 = { Placement::NorthWest, { 5, 20+5 } },
                                     .pin2 = { Placement::South, { -50-5,-5 } } });
										 
pFlexPanel->slots.pushBack( pPane2, {.pin1 = { Placement::North, { 5+50, 20+5 } },
                                     .pin2 = { Placement::SouthEast, { -5,-5 } } });
```

This requires some explanation. We have here decided to make the padding around the panes 5 points and leave 100 points in the middle for the buttons. We therefore specify the top-left corner of pane 1 to be 5 points from the left edge of our FlexPanel while being 25 points down from the top since the top panel is 20 points and then we have 5 points of padding. That is specified through pin1.

Similarly, the bottom right corner of the pane should be placed 5 points in from the bottom-center of the FlexPanel due to padding and also moved left 50 by points, in order to make room for half the space needed for the buttons.

Pane 2 is specified in the same way, with a relative position and adjustments in absolute points.

This places the two panes so that they irregardless of the FlexPanels size takes up half the window each after having left room for  a 20 points high menubar, a 100 points wide middle section for the buttons and a padding of 5 points each.

### Adding the buttons

The buttons should not be resized as the size of the FlexPanel is changed. Neither should they move, they should be clustered around the center with one button straight in the center and the others directly above and below with some padding inbetween.

We had already decided that they should be 100 points wide, so let's now also decide that they should be 30 points high each with a 5 points padding between. This could be achieved in pinned mode if we just put the relative placement of all pins in the center of the FlexPanel and use the absolute adjustment to move the corners of all three buttons to their correct offsets. For the top button of the three it would look something like this:

```c++
pFlexPanel->slots.pushBack( pCopyButton, { .pin1 = { Placement::Center, {-50, -(15+5+30) }},
                                           .pin2 = { Placement::Center, {50, -(15+5) }} });
```

Top-left corner being moved up half the center buttons size + padding + size of our button, while bottom right corner being 30 points lower.

However, that is an awkward way to express a slot of a fixed size being placed at a fixed distance from the center. Instead we can set the slot as movable. If we skip setting the *pin1* and *pin2* members of the slots blueprint the slot will be created as *movable* and we can instead set *orgio*, *pos* and *size*:

```c++
pFlexPanel->slots.pushBack( pCopyButton, { .origo = Placement::Center, 
                                           .pos = {0, -(15+5+30)}, 
                                           .size = {100,30} } );
```

This first sets the size of the slot, then places it at the specified origo (which is a FlexPos, so we can set relative values instead of a Placement-enum) and adjusts the position with pos.

Attentive readers might ask, can't we just include the position adjustment in the origo since it is a FlexPos and skip the pos-variable? Yes, you can, but it we think that becomes less readable and goes against the established convention.

### The result

Lets sum it all together and also take advantage of the fact that we can define and push all the slots in one operation as an initializer list:

```c++
pFlexPanel->slots.pushBack({
	{ pMyMenubar, {	.pin1 = { Placement::NorthWest }, 
                  .pin2 = { Placement::NorthEast, {0,20} }
	}},
	{ pPane1,     {	.pin1 = { Placement::NorthWest, {5,20+5} }, 
                  .pin2 = { Placement::South, {-50-5,-5} }
	}},
	{ pPane2,     {	.pin1 = { Placement::North, {5+50,20+5} }, 
                  .pin2 = { Placement::SouthEast, {-5,-5} }
	}},
	{ pCopyButton,{ .origo = Placement::Center, 
                  .pos = {0, -(15+5+30)}, 
                  .size = {100,30}
	}},								  
	{ pMoveButton, { .origo = Placement::Center, 
                   .pos = {0,-15}, 
                   .size = {100,30} 
	}},
	{ pDeleteButton, { .origo = Placement::Center, 
                     .pos = {0, (15+5)}, 
                     .size = {100,30}
	}} 
});
```

The result is a short, very readable structure, specifying the children and their geometry.



### A few more things to know about FlexPanels

1. The slots of a FlexPanel are allowed to overlap each other. The first among the slots (the one you access with slots.front() is the topmost while slots.back() returns the bottom one). FlexPanel checks a widgets opacity and a few other tricks to minimize amount of redraw of the same pixels, but it has limited understanding of the shape and opacity of widgets in the slots.
2. If you skip the size-parameter when placing a movable widget (or set size to {0,0}) the slot will get the occupying widgets default size. If the default size changes (like when adding text to a TextDisplay-widget) the slot will be resized as well. Setting size to zero works individually for the axis, so if you want a slot that is 200 points wide but gets the height from the widget you simply set it to {200,0}.
3. Slots in a movable state may be placed fully or partly outside FlexPanels geometry, in which case they will be clipped when rendering. This behavior can be changed with setConfineWidgets().
4. Movable slots have a fourth parameter called "hotspot". This is a secondary FlexPos determining the point inside the slot that should be placed at its origo in the FlexPanel. It defaults to the same value as the origo, making placement feel natural with e.g. the center of the slot being placed at the center of the FlexPanel when setting origo to center. Typically you ignore it, but in some cases it can be useful, like when you want to put a slots top-left corner or lower-right corner at the center of the FlexPanel.



### Advantages and disadvantages

The clear advantage of FlexPanels is that you can create complex, dynamic layouts with just one FlexPanel where you would need several nested PackPanels otherwise. You can also overlap slots anyway you see fit and move child widgets freely inside and outside the PackPanels geometry. This makes FlexPanel a natural fit for a desktop-like layer with several free-moving and overlapping child widgets.

It's also the best choice when you have a panel where children are spread out and you know the exact placement and size for each of them, like when you make a GUI mimicking a physical object.

However, it has one great disadvantage: It doesn't move other slots when the size of one slot changes. Imagine a list where you need to insert an object. If you implement that as a FlexPanel you would need to move every following slot individually. Neither can you easily create a row of buttons where the length of one button decides the starting place of the next or a GUI where space is allocated dynamically as individual widgets grow or shrink in size.

In this example we made several assumptions such that 20 points was the right height for the menubar and 100 points was enough for the button with the longest text. But what if the user interface is translated and the text will no longer fit? Or the font needs to be changed? That would require manual tweaking of the code, especially if the layout needs to respond to such changes dynamically.

Neither can FlexPanel calculate its own defaultSize() from what is needed for its slots. It allows too much flexibility to be able to do that. Therefore we strongly advice against putting a FlexPanel inside PackPanel or any other Container that takes the childs preferred size and relations between width and height into account. It wouldn't get the right value and the layout would probably break. Therefore it is strongly recommended to only use FlexPanels at a top-level layout for the layout of a section that should have a rigid, fixed size. For the later case you can use setDefaultSize() to set the size that the FlexPanel present its parent when asked.

So, let's look at an alternative way to implement the same layout, that becomes more dynamic.



## Implementing the design with PackPanels

The PackPanel is far simpler and more limited than the FlexPanel. Instead of allowing free placement of children, it simply lines them up (or packs them) side by side, either along the X or Y axis (horizontally or vertically). However, by combining several PackPanels into a hierarchy and assigning rules for how available space should be divided among the slots you can create complex, dynamic structures.

### Dividing the design into boxes

When implementing a design using PackPanels you start by dividing the design into boxes either horizontally or vertically. Then you recursively divide each box until you have the slots that you need. Let's see how this applies to our two-pane design:

* Step 1 - We divide the design into two vertical boxes: The menubar and the main box.
* Step 2 - We divide the main box into three horizontal boxes: pane 1, button column and pane 2.
* Step 3 - We divide the button column into 5  vertical boxes: empty top box + one for each button + empty bottom box.

When we have done this we are done. Each box either nicely contains one of the widgets that we want (with or without some padding) or no widget at all. Looking like this:

![2-pane-packpanels](/docs/manual/2-pane-packpanels.png)

Once we have divided the design into boxes like this, we can implement it with PackPanels. Each box will be implemented as a slot in a PackPanel. Since PackPanels can't have any empty slots, we use a special dummy Widget for the empty areas called Filler.

However, there is one more thing we need to understand before we can implement it: How the space of a PackPanel is divided between the boxes it contains. 

By default the slots in a PackPanel are given the space along their axis that their widgets request. So, for a horizontal PackPanel each slot is given the width returned by defaultSize() of the widget it contains and lined up from left to right, while a vertical PackPanel gives the height returned by defaultSize() and lines them up from top to bottom. If not all widgets fit in the PackPanel they are simply clipped at the end. The secondary axis, i.e. the height of a horizontal PackPanel or the width of a vertical PackPanel is used fully by the slots that all gets the full height or width of the panel.

This is usually not what you want. Usually you want the slots to fill out the area of the PackPanel but not extend beyond it so they get clipped and you also want some way to control how much space is assigned to what slot. For this we use a PackLayout-object.

### The PackLayout object

A PackLayout Object is created as any other Object, a create method taking a Blueprint as input. PackLayout objects are immutable, once created you can't change their content. You need to replace it with a new object if you want to change things.

The Blueprint for the PackLayout Object contains what rules that should apply when allocating the size of the PackPanel to the slots it contains. It can be configured in many ways that is beyond the scope of this chapter. However, one particular configuration has turned out to be very useful and is the only one we need for typical layouts like this:

```c++
PackLayout_p pLayout = PackLayout::create( {.expandFactor = PackLayout::Factor::Weight, 
                                            .shrinkFactor = PackLayout::Factor::Weight });
```

 A PackLayout configured like this, when applied to a horizontal PackPanel, will do the following:

1. It starts with giving each slot the width that corresponds to the default size of the widget it holds.
2. If there is space left, it will assign that space based on each slots weight. The default weight of a slot is 1.0. If all slots have the same weight, the space left is distributed evenly. If one slot has twice the weight it gets a twice as big slice of the space left. If a slot has the weight 0, it doesn't get any part of the space left. If a slot reaches the maximum width of the widget it contains it stops there.
3. If on the other hand the space has been exhausted and slots needs to be shrunk in order for all of them to fit in, it will shrink them based on the weight. A slot is not shrunk more than the minimum width of the widget it contains.

In cases where the default, minimum or maximum width of the widget is larger or smaller than you would like, you can limit it by wrapping it in a SizeCapsule before putting it in the slot.



### Creating and configuring the PackPanels

We start with creating the layout and three panels we need, two vertical and one horizontal.

```c++

PackLayout_p pLayout = PackLayout::create( {.expandFactor = PackLayout::Factor::Weight, 
                                            .shrinkFactor = PackLayout::Factor::Weight });

PackPanel_p pWindowPanel = PackPanel::create( { .axis = Axis::Y, .layout = pLayout } );

PackPanel_p pMainSection = PackPanel::create({
    .axis = Axis::X, .layout = pLayout, 
    .skin = ColorSkin::create( { .color = Color::Transparent, .padding = 5 } ), 
    .spacing = 5 }
);

PackPanel_p pButtonColumn = PackPanel::create( { .axis = Axis::Y, .layout = pLayout });
```

They all use the same PackLayout since a basic use of weights fulfills all our requirements. The only difference is that the one for the main section is horizontal while the others are vertical.

The main section is somewhat different since we want to create some spacing around our panes. We do  that by first adding an invisible skin with a 5 point padding, which pushes the content of the PackPanel 5 points in from each side. This creates top, bottom, left and right spacing for its content. We also need to add some spacing between the panes and the buttons, this is done by setting the spacing attribute of the PackPanel.



### Building the button column

We now need to populate our PackPanels with content, thus building the hierarchy. Hierarchies like these are best built from the bottom up, so we start with adding the slots to the innermost PackPanel, the one for the bottom column. To create the 5 boxes we need to add five slots. Since we are not allowed to have empty slots we use Filler-widgets for the top and bottom slots:

```c++
pButtonColumn->slots.pushBack({ {Filler::create(), { .weight = 1.f }},
                                {pCopyButton, { .weight = 0.f }},
                                {pMoveButton, { .weight = 0.f }},
                                {pDeleteButton, { .weight = 0.f }},
                                {Filler::create(), { .weight = 1.f }}
														 });
```

Let's go through what we do here. We add five slots to the PackPanel for the button column. The three middle gets the buttons we want to display while we create Fillers for the top and bottom one. We create the Fillers in place since we don't need any pointer to them anyway, they are just there to fill out the slots. For the Filler slots we set a weight of 1, while we set the weight to 0 for the button slots. 

With the PackLayout we use this means that the buttons will always be displayed in their default size, which should be the right size for a nice looking button. The Fillers will stretch though and since they have the same value they will evenly split all the extra space between them, thus forcing the buttons to the middle of the PackLayout. Fillers have by default a size of 0 points, so they won't take any space if the window shrinks down so only the buttons fit.

We could have skipped setting the weight of the Filler-slots since weight defaults to 1, but we added it to make the example more clear.

### Building the main section

The main section is what contains the left pane, the button column and the right pane. That is three slots where the left and right ones should have a padding of 5 pixels. The middle slot should have the size needed for the buttons while the left and right ones should share the rest of the space. Setting this up is easy:

```c++
pMainSection->slots.pushBack({ {pPane1, { .weight = 1.f} },
                               {pButtonColumn, { .weight = 0.f } },
                               {pPane2, { .weight = 1.f} }
                            });
```

Given all we know already this is quite self-explanatory.

### Adding the final piece

Now that we have the main section completed we just need to add the menubar. We want the menubar to have the height it needs and the main section to get all the rest of it:

```c++
pWindowPanel->slots.pushBack({ {pMyMenubar, { .weight = 0.f}},
                               {pMainSection, { .weight = 1.f }}
                            });
```

Now the layout is complete and can be added to the Root-widget of the window. It will adjust accordingly to the size of the window, giving just enough space to the menubar and button column while assigning the rest to the two panes.

### The result

Let's put it all together to see the whole picture. This time we remove unnecessary setting of slot properties where default values are correct for us:

```c++
PackLayout_p pLayout = PackLayout::create( {.expandFactor = PackLayout::Factor::Weight, 
                                            .shrinkFactor = PackLayout::Factor::Weight });

PackPanel_p pWindowPanel = PackPanel::create( { .axis = Axis::Y, .layout = pLayout } );
PackPanel_p pMainSection = PackPanel::create( { .axis = Axis::X, .layout = pLayout, .skin = ColorSkin::create( { .color = Color::Transparent, .padding = 5 }), .spacing = 5 });
PackPanel_p pButtonColumn = PackPanel::create( { .axis = Axis::Y, .layout = pLayout });

pButtonColumn->slots.pushBack({ Filler::create(),
								{pCopyButton, { .weight = 0.f }},
								{pMoveButton, { .weight = 0.f }},
								{pDeleteButton, { .weight = 0.f }},
								Filler::create()
							  });

pMainSection->slots.pushBack({ pPane1,
                               {pButtonColumn, { .weight = 0.f } },
                               pPane2
                             });

pWindowPanel->slots.pushBack({ {pMyMenubar, { .weight = 0.f}},
                               pMainSection
                             });
```

This is a several step process so it doesn't look as elegant as the FlexPanel example. However, it has several advantages, so lets discuss those.

### Advantages and disadvantages

The advantages of building the design with PackPanels is that it adapts automatically to changes and therefore is easy to modify. What if we change the font size so that the space needed for the menubar and buttons increase? It just works. What if we want to add a fourth button to the column? Just add it, the placement of the other buttons will be adjusted automatically. What if we want the buttons not centered but at the top? Just remove the top filler. With PackPanels you get a high level of maintainability since later changes often can be done easily without breaking anything.

There are some disadvantages though. The first one is that you are restricted to a rigid grid layout. You can not place a widget at an exact point or move it around. You can't have widgets that overlap or move outside the window.

As mentioned before, the code can also be harder to read since it doesn't look as clean and it is a several step process that you need to read through to understand the layout.

## Conclusion

Hopefully this example has given you a better idea of when to use what layout mechanism. In reality you shouldn't stick to just one that we did in this example. You might have sections of the GUI where a rigid, adaptive, box-layout is preferable while other sections needs the flexibility.

The main warning I have for you is to **not insert FlexPanels into a rigid PackPanel layout** since they don't report default size or adapt to resize in the same way. Sometimes it makes sense, but often there is a better alternative.

