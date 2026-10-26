---
title: 'Containers, slots and children'
weight: 20
---

# Containers, slots and children

*The layout of a user interfaces is built up by placing widgets inside widgets. A widget that can hold one or more other widgets is called a Container. The are several categories of containers, see the chapter "Widget Overview" to learn more about those.*



## What is a container?

A container is simply a Widget which can contain one or more other widgets. The contained widgets are called children and the container are the childrens parent.



## What is a slot?

A Slot is simply an object in which the Container stores the pointer to a widget and all information associated with it, such as its position, size and other parameters needed to properly place and display it.

There is a Slot base class containing a pointer to the widget and methods for reading its size and position within the container. Most containers extends this Slot class and adds methods for manipulating the slot in various ways.

One way to think about this is that the container is a wodden panel that you can hang on a wall. This panel has several rectangular holes (slots) into which you can put various objects (widgets). It is the size and position of the slot that determines the size and position of the widget you put into it. Slots are added, removed, resized and moved around and the widget follows it around and is resized to fit nicely.

Some containers have a fixed amount of slots while others can add and remove slots dynamically. Some containers have slots that can overlap each other while others enforce a flat layout. Widgets are allowed to "overflow" which means that they can draw graphics outside their slots, which is useful for effects such as glows and shadows, but the interactive part of a widget, where it accepts mouse clicks and such, is restricted to the slot. Some containers though (such as Scrollpanel and FlexPanel) have slots that can extend or be moved outside its own boundaries in which case the child is clipped.



## Accessing slots

A container typically presents its slots as public members that can be accessed directly. A container which has strictly one slot (like any *Capsule* or the *ScrollPanel*) has a member simply named *slot*. Accessing the slot and any member of it is easy:

```c++
Widget_p pWidget = pContainer->slot.widget();	// Get a pointer to the widget in the slot.
Size size = pContainer->slot.size();			// Get the size of the slot (and therefore the widget).
Coord pos = pContainer->slot.pos(); 			// Get the position of the widget inside the container.
Rect geo = pContainer->slot.geo();				// Get the geometry (position and size) of widget.
```

The four methods above are available for all slots irregardless of container.

A slot can be either *static* or *dynamic*. Most slots are dynamic, which means that you can change the widget inside.

```c++
pContainer->slot.setWidget( pNewWidget );		// Replace widget in slot with a new one.
pContainer->slot = pNewWidget;					// Shorthand expression for above.
```



Most containers have more than one slot though. These are typically arranged in an array (fixed amount of slots) or vector (dynamic amount of slots) which is named *slots*. These can be accessed like the content of a normal std::vector:

```c++
Widget_p pWidget = pContainer->slots[0].widget();		// Get a pointer to the widget in the first slot.
Widget_p pWidget = pContainer.slots.front().widget();	// Same result as above.

Widget_p pWidget = pContainer.slots.back().widget();	// Get a pointer to the widget in the last slot.
Widget_p pWidget = pContainer->slots[pContainer->slots.size()-1].widget();	// Same as above.
```



Standard iterators can also be used:

```c++
for( auto it = pContainer->slots.begin() ; it != pContainer->slots.end() ; it++ )
{
  Rect geo = it->geo();
  
  // Do something
}
```



There are two types of SlotVectors, *dynamic* and *static*. The difference is that a *DynamicSlotVector* allows you to add and remove slots while a *StaticSlotVector* has the amount of slots controlled by its container and you can only manipulate the content of the slots (unless the slots themselves also are static, in which case you can only read them).

DynamicSlotVector is similar to std::vector and operates in a similar fashion but not identical. It is lacking some members that simply have not been added yet but also have some useful additions. One of those is that besides using iterators, you can always also use indexes for all methods:

```c++
// These two calls yields identical results.

pContainer->slots.insert( pContainer->slots.begin() + 10, pWidgetToInsert );
pContainer->slots.insert( 10, pWidgetToInsert );
```

One thing to be aware of is that when you specify a range using index you use start-index and amount, not end-index.

```c++
// Erase two slots, starting from the third slot

pContainer->slots.erase( pContainer->slots.begin() + 3, pContainer->slots.begin() + 5 );
pContainer->slots.erase( 3, 2 );
```

Another feature is that DynamicSlotVector can push to both the front and end of the vector and reserve space at both ends. This is useful since you often end up wanting to add widgets to the front instead since the order of children carries meaning.

```c++
pContainer->slots.reserveFront(10);		// Grow container so we have space for 10 new slots at the front.
pContainer->slots.reserveBack(5);			// Similar to std::vector::reserve(), reserves space at the back.

pContainer->slots.pushFront( pWidget );
pContainer->slots.pushBack( pAnotherWidget );
```

Another notable difference is that DynamicSlotVector follows WonderGUI's naming convention, using camelCase instead of snake_case and some methods differ slightly in naming (such as isEmpty() instead of  empty()).



Not all containers have all their slots in one public member though. Overlays in particular always have two sets of slots. One is what is called the *mainSlot*, which is a single slot for the normal hierarchy of widgets continuing below the overlay. A second slot, or vector of slots, contains the widgets that resides in the overlay. A good example is the ModalOverlay, which besides the mainSlot has a DynamicSlotVector named *modalSlots*, in which you should place any widget that should become modal (prevent access to any widget below it).

```c++
pModalOverlay->mainSlot.setWidget( pMyWidgetHierarchy );		// Set the main GUI below the modal widgets
pModalOverlay->modalSlots.pushFront( pMyModalWidget );			// Add a modal widget.

pModalOverlay->modalSlots.clear();		// Remove any modal slot, make main GUI accessible again.

```



Many container widgets also offer special convenience methods for operating on several slots at once. E.g. PackPanel has the hideSlots() methods for hiding several slots at once:

```c++
pContainer->hideSlots( beginInterator, endIterator );
pContainer->hideSlots( startIndex, amount );
```

This is both convenient and improves performance since only one re-calculation of geometry is performed once all slots have been hidden.



One more thing worth mentioning is that you can set a raw pointer directly to any public member of widget such as a single slot or a slot vector for increased code brevity when accessing it multiple times:

```c++
auto p = &pContainer->slots;

p->reserveFront(3);
p->pushFront( pChild1 );
p->pushFront( pChild2 );
p->pushFront( pChild3 );
```

This is however a raw pointer, so you should only keep it around for as long as you keep the original smartpointer or might end up with a dangling pointer. A way to avoid this is to use a component pointer instead, which is a smartpointer that keeps a reference to the object while pointing at one of its public member:

```c++
auto p = ComponentPtr<DynamicSlotVector>(pContainer, pContainer->slots);

p->reserveFront(3);
p->pushFront( pChild1 );
p->pushFront( pChild2 );
p->pushFront( pChild3 );
```

This keeps the object alive but adds some slight overhead.



## Adding slots

When you add a slot to a DynamicSlotVector, you do not create a Slot object and push it. The slots are internal to the containers and can only exist within containers. Instead you either just push a widget to the slot vector, in which case a new slot with default values and the specifed widget is added, or you push the widget together with a Blueprint for the slot. Here is an example for PackPanel:

```c++
// Add a widget to the back, slot is intialized with default settings.

pPackPanel->slots.pushBack( pMyWidget );

// Add a widget to the back, but provide a blueprint for the slot.

pPackPanel->slots.pushBack( pMyWidget, { .padding = {0,5,0,5}, .weight = 2.5f } );

// Same as above, but using the WGBP macro (for C++17 and older)

pPackPanel->slots.pushBack( pMyWidget, WGBP(PackPanel::Slot, _.padding = {0,5,0,5}, _.weight = 2.5f) );

```

Each time you add or insert a slot, you get an iterator to the slot returned that you can use to access slot:

```c++
pPackPanel->pushBack( pMyWidget )->setWeight(2.5f);
```

This is however less efficient than providing a blueprint.

Another way to add widgets to a DynamicSlotVector is to use the left-shift operator (<<) like you do with strings:

```c++
pPackPanel->slots << pMyWidget;
```

These can also be stringed together for brevity:

```c++
pPackPanel->slots << pWidget1 << pWidget2 << pWidget3;
```

These are however added one by one, which is not optimal since each widget added might trigger a costly re-calculation of geometry. It also has the drawback that you can't include a blueprint due to limitations in C++.



A better way to add multiple widgets is therefore to add all at once using an initializer_list.

```c++
pPackPanel->slots.pushBack( { pWidget1, pWidget2, pWidget3 });
```

You can also include slot Blueprints by creating an intializer_list with tuples of pointers and Blueprints, like this:

```c++
pPackPanel->slots.pushBack( { {pWidget1, { .weight = 0.f }},
                              {pWidget2, { .weight = 1.f }},
                              {pWidget3, { .weight = 2.f, .visible = false}} });
```

Remember that Blueprints are just structs with parameters. You can reuse the same Blueprint to increase efficiency, save yourself some typing and making the code cleaner:

```c++
PackPanel::Slot::Blueprint light( .weight = 0.5f );
PackPanel::Slot::Blueprint heavy( .weight = 2.f );

pPackPanel->slots.pushBack( { {pWidget1, light },
                              {pWidget2, light },
                              {pWidget3, heavy } });
```



The widgets can also be added using a begin/end pair of iterators to a container or array containing Widget_p's:

```c++
// Use a vector

std::vector<Widget_p>	myVector = { pWidget1, pWidget2, pWidget3 };
pPackPanel->slots.pushBack( myVector.begin(), myVector.end() );

// Or an array

Widget_p		myArray[4] = { pWidget1, pWidget2, pWidget3 };
pPackPanel->slots.pushBack( myArray, myArray+3);
```

Adding with a begin and end iterator actually also works with raw pointers if needed:

```c++
Widget*		myArray[4] = { pWidget1, pWidget2, pWidget3 };
pPackPanel->slots.pushBack( myArray, myArray+3);
```

Unfortunately you can not use begin/end-iterators with Blueprints yet. It can probably be done, but is a little bit tricky to solve.



## Clearing and removing slots



## Moving widgets around

