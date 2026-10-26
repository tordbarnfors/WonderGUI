---
title: 'Overview'
weight: 10
---


# Widget overview

*This section gives you a quick overview of the widgets available and some fundamental understanding of the different widget categories.*



## Widget categories

At the most basic level, widgets are divided into three categories: *roots*, containers and *normal widgets*. 

### Roots

Roots are special widgets from which a hierarch of displayed widgets originate. They do not inherit from the Widget-class, instead they inherit from a special Root-class and deal with rendering of the hierarchy. They all accept one widget as their child an can not be inserted into a widget hierarchy, just be the root of one.

The most commonly used Root is called RootPanel and that is the one you are likely to use.

### Containers

Containers are widgets that accepts one or more other widgets as children. They are used for organizing the widgets hierarchically and controls the layout of your user interface. If you think about the widget hierarchy as a tree, the containers are the branches of the tree, while normal widgets such as buttons and text-fields are the leafs.

Containers typically plays a behind-the-curtain role in the user interface, organizing other widgets on the screen without being visible to the user, but they can also be made visible since they can be skinned as any other widgets, providing frames and backgrounds for the widgets they contain.

There are four sub-groups of containers:

- **Panels**: These are the normal container widgets. Think of them as a physical panel or a plate onto which you place other widgets. The kind of Panel decides how you can organize and move the widgets on the panel. By default the panels are transparent as glass, but can be skinned. By organizing panels within panels you can make the most elaborate layout from  simple panel widgets.
- **Lists**: These are similar to panels, but provides the added functionality of selecting widgets with the mouse and keyboard. These are useful for views where you can select one or more items.
- **Overlays**: Overlays are meant to be placed close to the root and provide functionality for overlaying widgets such as popup menus and dialogs, tooltips and notifications onto the rest of the hierarchy. They have one main child, through which the normal hierarchy continues, and can have several children, with special abilities that are freely placed above the hierarchy. They need to be present in the hierarchy for certain things to work, e.g. tooltips won't be displayed unless you have a TooltipLayer.
- **Capsules**: Capsules are tiny containers that only takes one child and overrides some of its characteristics or how it is seen by its parent widget. You could say that it encapsulates the widget so it looks different to its parent. A typical Capsule is SizeCapsule which overrides the methods used by the parent to decide its geometry with values set in the capsule.

### Normal widgets

Normal widgets are simply widgets that are not containers and therefore can only be leafs in the widget hierarchy. They are meant to be displays or controls that are seen and manipulated by the user.



