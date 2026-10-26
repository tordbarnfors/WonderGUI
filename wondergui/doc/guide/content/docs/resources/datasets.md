---
title: 'Datasets'
weight: 20
---


# Datasets

*Dataset is a template class for wrapping plain old data objects (PODs) into WonderGUI objects with smart pointers and reference counting, but can also be used for more advanced objects as long as they have empty constructors.*

## Creation and use

Creating a Dataset object from your own data class is easy:

```c++
class myData
{
    int foo;
    int bar;
}

auto p = Dataset<myData>::create();
```

The Object pointed to by p has a public member named data, which is an instance of myData and can be accessed without restrictions:

```c++
p->data.foo = 123;
int x = p->data.bar;
```

Creation with initialization works just as fine as empty initialization:

```c++
auto p2 = Dataset<myData>::create( {123,456} );
auto p3 = Dataset<myData>::create( { .foo = 123, .bar = 456 } );	// Since C++20
```

## Purpose

The main purpose of Dataset is to allow for an easy way to attach arbitrary information to any widget or surface through their baggage() methods and have that data automatically destroyed along with the widget.

