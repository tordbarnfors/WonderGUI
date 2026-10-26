---
title: 'Blueprints'
weight: 30
---

# Blueprints

*Blueprints is a central concept in WonderGUI and provides the default way to initialize an object. By using blueprints we can create and initialize complex objects in one call without long lists of parameters that needs to be set in a specific and hard to remember order. The code also gets easier to read and understand since initialization gets more descriptive.*


## Introduction

Technically, a blueprint is just a plain C-style struct (also called POD, Plain Old Data object) that is used as an input parameter to the create() method for a class. It has no methods and all data members are public and directly accessible.

Every class that supports blueprints has a struct simply named "Blueprint" within its name scope, so TextStyle uses TextStyle::Blueprint while BlockSkin has BlockSkin::Blueprint. Clean and simple. 

A blueprint also has the advantage that it can be created once and used several times to create multiple objects. It can also be modified between uses, so you can set some common variables for a group of objects once and then just change the parameters that differ.

You don't need to set all parameters in a blueprint, we provide sane defaults for most of them.



## Creating blueprints

There are three ways to create and use a Blueprint depending on conditions and personal preferences. Let's quickly go through them:



1. ### Creating and setting members explicitly

   This is the simplest way to do it, but also results in the most cluttered and verbose code. You simply create the blueprint and set its members one by one before use:

   ```c++
   TextStyle::Blueprint	style;
   style.font = pMyFont;
   style.size = 12;
   style.color = Color::Black;
   auto pMyStyle = TextStyle::create(style);
   ```

   This works in any C++ environment and can easily be understood by any programmer with no prior exposure to WonderGUI or C++20 designated initializers.

   It also lets you reuse the blueprint further down the code, with or without modifications.

   

2. ### Using C++20 designated initializers

   If you are allowed to use C++20 features you can use *designated initializers* to create the blueprint and set all parameters directly in the create call.

   ```c++
   auto pMyStyle = TextStyle::create({	.color = Color::Black, 
   									.font = pMyFont,
                                       .size = 12 });
   ```

   This is short, sweet and very clean, but designated initializers has one big drawback - *you must set the parameters in the order they are presented in the struct!*

   Some compilers have extended C++ to allow out or order initialization, in which case you can ignore this problem and happily use this method without restrictions.

   For the rest of you we have made it as easy as possible by ordering all parameters in alphabetical order. This is not an ideal solution, for example will .maxValue appear before .minValue which is counter-intuitive and you will have to do some extra work to make sure the order is right, but it is the best we can do for the moment. 

   This restriction is also the main reason we provide the third method.

   

3. ### Using the WGBP macro

   By some clever usage of a lambda inside a macro we have created something similar to designated intializers that work on C++11 and avoids the need to order parameters alphabetically. It looks like this:

   ```c++
    auto pMyStyle = TextStyle::create( WGBP(TextStyle,
   									_.font = pMyFont,
   									_.size = 12,
   									_.color = Color::Black ));
   ```

   This is looks very similar to designated initializers, but is a bit more convoluted. The changes needed are:

   * We need to add **WGBP** (which is the name of the macro and stands for *WonderGUI BluePrint*) and use parentheses instead of curly brackets.

   * We need to repeat the name of the class which the blueprint is for as the first parameter of the macro.

   * We need to preceed each parameter name not only with a period but also an underscore.

     

   The code generated includes a lambda that is called to initialize the object, but compiler optimizations (when enabled) will remove this lambda and make the code just as efficient as the alternatives.

   This is the method we generally recommend since it sidesteps the issue of ordering the parameters and also works on C++11. However, it has a few drawbacks that we want to make clear:

   1. It results in more typing and text than using designated initializers.
   2. It uses a macro, which is very short and simple but might be against project/company policy.
   3. It is a WonderGUI-specific construct, developers not familiar with WonderGUI might be confused the first time they see it.
   4. Error messages will get more confusing and intellisense/autocomplete-features of your IDE might not list the name of the parameters due to the macro.



It is worth pointing out that you can use designated initializers or the WGBP macro also when creating a blueprint for repeated use. Like:

```c++
TextStyle::Blueprint bp1 = { .color = Color::Black, 
							 .font = pMyFont,
                             .size = 12 };

TextStyle::Blueprint bp2 = WGBP(TextStyle,
								_.font = pMyFont,
								_.size = 12,
								_.color = Color::Black );
```



## Hierarchical blueprints

The examples above are sweet and simple, but you will likely end up creating more complex, hierarchical blueprints when adding state-specific data to Skins and TextStyles. Let's take a look at how that works, using BoxSkin as an example:

A BoxSkin is a simple skin displaying a filled rectangle with an outline. You can specify the color of the rectangle and the color of the frame individually and also the size of the frame. You can also alter the colors depending on the state of the widget using the skin, such that a dark box gets brighter or gets a different outline when the mouse pointer enters it. There are more parameters, but let's ignore them.

### The basic way - step by step

Let's say we want a green box with a thin black outline. We can create a Blueprint for it like this:

```c++
BoxSkin::Blueprint bp = WGBP(BoxSkin, 
                             _.color = Color::Green, 
                             _.outline = 1.f, 
                             _.outlineColor = Color::Black );
```

Let's say that we want the box to turn red when the mouse pointer enters it. In WG terminology we say that the box enters state Hovered, so we need to provide an alternative color for state Hovered.

State specific data is added to a vector named "states" in the blueprint. Each entry in the vector contains a struct containing two members, like this:

```c++
struct StateBP
{
	State		state;
	StateData	data;
}
```

This pattern is identical for all blueprints supporting states, only the format of the StateData differs. For BoxSkin it looks as follows:

```c++
struct StateData
{
	HiColor		color = HiColor::Undefined;
	Coord		contentShift = {0,0};
	HiColor		outlineColor = HiColor::Undefined;
};
```

Knowing this, we can add our color for state Hovered to our blueprint. Let's start by doing it in the most explainatory step-by-step way before showing how to do it with less code:

```c++
BoxSkin::StateBP hovered;
hovered.state = State::Hovered;
hovered.data.color = Color::Red;

bp.states.push_back(hovered);
```



### The designated initializer way (C++20)

Above we simply create a struct containing the state-specific information and push it onto the stack. That is however quite much code. If we use C++20 designated initializers we can make it much shorter:

```c++
bp.states.push_back( {State::Hovered, {.color = Color::Red}} );
```



### The WGBP-compatible alternative

Unfortunately we can not reuse the designated initializer pattern with the WGBP macro. The reason is that the macro only works with the Blueprint structs themselves, not the structs contained within such as StateBP. Instead we have opted to take a step back from the POD-principle and add a bunch of constructors to our StateBP-structs. For BoxSkin::StateBP we have the following constructors:

```c++
StateBP( State state, StateData data )
StateBP( State state, Color color )
StateBP( State state, Coord contentShift )
StateBP( State state, Color color, Color outlineColor )
```

These cover the most common use cases and allows us to write as follows:

```c++
bp.states.push_back( {State::Hovered, Color::Red} );
```

This is actually briefer and more compact than the designated initializers example above. It comes with one drawback though: we don't (and can't have) constructors for all combinations of parameters. We therefore have to resort to setting parameters explicitly in some cases, but in ~90% of typical use cases we can use this short and clear way.



Also, we can use initializer lists to fill the vector instead of one or more push_back() calls:

```c++
bp.states = {{State::Hovered, Color::Red}};
```

This makes adding several states very clean and simple:

```c++
bp.states = {	{State::Hovered, Color::Yellow},
				{State::Pressed, Color::Red} }
```

Intializer lists also have one big advantage over push_back() - they can be inlined in the blueprint intialization. This in turn allows us to inline everyting into the call to BoxSkin::create() like this:

```c++
auto pMySkin = BoxSkin::create( WGBP(BoxSkin, 
                             _.color = Color::Green, 
                             _.outline = 1.f, 
                             _.outlineColor = Color::Black,
                             _.states = {	{State::Hovered, Color::Yellow},
											{State::Pressed, Color::Red} }
								));
```



## More about blueprints

There are a few more things related to blueprints that is worth pointing out. 

* Since states are specified in a vector the same state can be specified more than once. That is allowed and in that case they are processed in order so that parameters set for the state early in the vector are overridden by parameters set for the state later. Please note that this doesn't mean that earlier entries are fully ignored, Color specifications in Blueprints typically default to Undefined, in which case a color set by an earlier state remains. The contrary is also true, just because you don't specify the value for a parameter it might still default to a value in the StateData-structure that overrides any previously set value. 

* You might notice that you can set values for state Normal in the states-vector just like any other state although the values for state Normal already are defined in the blueprint (color and outlineColor in BoxSkin are for normal state). This is allowed and the two following examples yields the same result. Which one to choose is mainly a matter of taste:

  ```c++
  TextStyle::Blueprint bp1 = WGBP(TextStyle,
  								_.color = Color::White,
  								_.states = {{State::Hovered, Color::Pink}}
  								);
  																
  TextStyle::Blueprint bp2 = WGBP(TextStyle,
  								_.states = {{State::Normal, Color::White}
  											{State::Hovered, Color::Pink}}
  								);					
  ```

* All objects that can be created using a blueprint are meant to have a method called blueprint() which returns a blueprint for it, but currently only TextStyle supports that. This is meant as a way to clone an object or make a modified version of it by extracting a blueprint, modifying it and using it to create a new object. The blueprint returned is generated from the objects data and can therefore be different from the one used to create it.
