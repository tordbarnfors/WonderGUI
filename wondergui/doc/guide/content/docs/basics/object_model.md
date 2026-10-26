---
title: 'Object model'
weight: 10
---


# The object model

*WonderGUI uses its own system of reference counted objects with smart pointers. From a user perspective It is in many ways similar to, STL's shared_ptr and weak_ptr but it is very different under the hood and more tightly integrated with WonderGUI.*



The reason for WonderGUI having its own object model is partly because WonderGUI predates C++11 and partly because the tighter integration offers a number of features and benefits not available from STL's counterpart.

Although you can use STL's smart pointers in software using WonderGUI, you can **not** use them to point at WonderGUI objects. That would cause confusion and most certainly crashes. All other kinds of mixing, like including WonderGUI smart pointers in objects managed by STL smart pointers or the other way around should be fine.



## Object

wg::Object is the base class for all reference counted objects. It contains the reference counter and methods for increasing, decreasing and reading the reference count. It also contains some limited means of introspection and allows for a "finalizer" callback that is called right before the object is destroyed.



## Creating objects

Every class derived from Object that can be instantiated has a static method called create(). To create a Button (one of the simplest widgets) you simply call Button::create(). A shared pointer to the object is returned if object could be created. An error is sent to the error handler (registered in wg::Base) and a nullptr is returned if the object could not be created.

Object creation in WonderGUI will fail more often than when using new, because WonderGUI will rather fail and return a nullptr than return a broken object due to invalid construction parameters.



## Shared and weak pointers

WonderGUI supports shared and weak pointers. There is no pointer equivalent to stl::unique_ptr, we found them to be of little use in WonderGUI and prone to cause errors when used casually. WonderGUI shared pointers have less overhead than stl::shared_ptr so the benefit of a unique_ptr would be small.

The shared pointer is called StrongPtr and the weak pointer is called WeakPtr. Both are templates and can be used as follows:

```c++
StrongPtr<Button>	pMyButton;
WeakPtr<Button>		pMyButton2;
```

However, for convenience every Object-derived class has typedefs where the StrongPtr gets the name of the class followed by "\_p" and the WeakPtr gets the extension "\_wp". So you can more easily write:

```c++
Button_p	pMyButton;
Button_wp	pMyButton2;
```

The only overhead of a StrongPtr compared to a raw C-pointer is the increment or "decrement with compare" of the reference count when creating, copying and destroying pointers. The StrongPtr is the same size as a normal C-pointer and there is no extra data allocated.

A WeakPtr on the other hand adds some extra overhead. A separate, small struct is allocated and linked to the object, containing  count of weak pointers and a pointer to the Object. Having more than one WeakPtr pointing to the same Object doesn't cost extra, they share the same struct.



## Raw C-style pointers

Contrary to STL's smart pointers, a raw pointer to a WonderGUI Object can be converted back to a StrongPtr again, since the reference counter is located in the Object itself.

Using raw pointers to Objects is totally fine as long as you have at least one StrongPtr pointing at the Object or you have manually increased the reference count by calling the Objects retain() method. In fact, most methods taking an Object pointer as input takes a raw pointer since StrongPtr is implicitly cast to raw pointer without cost.

So, you can actually opt to use just raw pointers with manual reference counting once the object has been created as the following example illustrates:

```c++

Button * pRaw;
{
	Button_p	pMyButton = Button::create();// pMyButton assigned, increasing ref count.
	pRaw = pMyButton;
    pRaw->retain();			// Manually increase ref count.
}							// pMyButton goes out of scope, decreasing ref count.

// Do whatever you want with the object here...



pRaw->release();	// Decrease the ref count once you are done. This will destroy 
					// the object unless WonderGUI holds an internal reference.

```

Not that this is not anything we recommend since it negates the advantages of smart pointers, but it is a good trick to know since it might come in handy in special cases, like when writing wrappers for other languages.



## Casting

A StrongPtr (or WeakPtr) is implicitly cast to smart pointers of a base classes:

```c++
Button_p	pMyButton = Button::create();
Widget_p	p2 = pMyButton;			// StrongPtr to base class Widget.
Object_wp	myWeakPtr = pMyButton;	// WeakPtr to base class Object.
```

Just as with raw pointers, any casting to a subclass needs to be made explicit. There are two template methods to help you with this:

* wg_static_cast
* wg_dynamic_cast

These are equivalent to standard static_cast and dynamic_cast in the way they are used, but operates on StrongPtr. Use them like this:

```c++
Button_p	pMyButton = wg_static_cast<Button_p>(pWidget);
Button_p	pMyButton2 = wg_dynamic_cast<Button_p>(pWidget);
```

The difference between them is, like in normal C++, that wg_dynamic_cast makes a safe attempt to cast and returns nullptr if it fails, while static_cast assumes that you know what you are doing and do it the fast way.

Finally there is one more way to cast. Since we can assign to a StrongPtr from a raw C-style pointer, we can simply get the raw pointer, cast it using normal C++ pointer casting and then assign it to the StrongPtr, like this:

```c++
	Panel_p p = static_cast<Panel*>(pWidget.rawPtr());
```

Which is exactly what wg_static_cast does.



## Finalizers

A finalizer is a function that is called when an Objects reference count has reached zero, right before its destructor is called.

Each Object can only have one finalizer and it can't be a complex lambda with captures since it is stored as a function pointer, not an std::function.

A finalizer-pointer is stored in the same struct used by weak pointers, so they cause the same overhead.

Here is an example on how to set a finalizer for a widget using a Lambda:

```c++
auto pWidget = Button::create();

pWidget->setFinalizer([](Object * p) { /* Do something here */ });
//                     ^
// Note:       Capture | must be empty!

```



## Introspection

All Objects have an associated TypeInfo structure which allows you to check the class-type of the object and what class it inherits from. The name of the class is also accessible as a printable c-string.

The method typeInfo() returns a reference to the type-info for the object and the TypeInfo-struct for any class can be accessed by <classname>::TYPEINFO. 

Here are a few examples of what you can use that for:

```c++
// Determine if an object is a Widget, i.e. a subclass of the Widget-class:

bool isWidget = pObject->isInstanceOf(Widget::TYPEINFO);

// Determine if an object is a Button-widget specifically (not a subclass thereof):

bool isButton = (pObject->typeInfo == Button::TYPEINFO);

// Get the class-name of an object:

const char * pClassName = pObject->typeInfo().className;

// Get TypeInfo-struct for the class inherited from:

TypeInfo * pSuperClass = pObject->typeInfo().pSuperClass;

```



## Components and component pointers

The term *Component* is used in WonderGUI to refer to a public member of an Object with its own API. Components are used to easily add functionality to Objects, particularly Widgets, through composition instead of inheritance such as a text-field or a container for child widgets. Typically (but not always) they contain a reference to the object containing them so they can notify the object of changes.

Sometimes it might be useful to point to a specific component inside an object directly. This can naturally be done using raw pointers like this:

```c++
Text * pLabel = pMyButton->label;
```

However, that only works as long as you keep a strong pointer to the object around, so it isn't destroyed. In order to make sure the object isn't destroyed under your feet, you can use a *ComponentPtr* which points at the component but keeps a reference to the Object, guaranteeing that it isn't destroyed while the pointer is being used. It works like this:

```c++
auto pLabel = ComponentPtr( pMyButton, pMyButton->label );
```

First parameter is a pointer to the object and the second is a reference to the component.

If you need to declare it, it can be declared like this:

```c++
ComponentPtr<Text>	pLabel;
```

But in most cases the component has a typedef:ed pointer that can be used for brevity and readability:

```c++
Text_p	pLabel;
```

A weak-pointer version currently does not exist but can easily be added if needed.

