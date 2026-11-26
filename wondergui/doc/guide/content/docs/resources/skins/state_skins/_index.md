---
title: 'StateSkins'
weight: 30
BookToC: false
---


# StateSkins

StateSkins change their appearance depending on the state of the widget, such as if it is normal, pressed, selected, disabled, has keyboard focus etc. These are most commonly used.

## Common properties for all state skins

There are two properties that are shared between all StateSkins: content shifting and state specific info in Blueprints.

### Content shifting

Content shifting is the ability to change the position of the content rectangle depending on state. This can be used to create a 3D effect when e.g. a button is pressed. Shifting the content slightly down and to the right will move the text and icon on the button, which along with changes to the skins graphics can improve the sense of the button actually being pushed down.

### State specific blueprint info

 All StateSkin blueprints have a member called "states", which is a vector containing state-specific data.  This state-specific data varies from skin to skin but always includes the member *contentShift*. Using a blueprint with state specific data can look like this:

```c++
auto pMySkin = ColorSkin::create( 
{ 
    .margin = 2,
    .padding = 2,
    .states = { {State::Default, {.color = Color::Green}},
                {State::Hovered, {.color = Color::Yellow}},
                {State::Pressed, {.contentShift = {2,2}, .color = Color::Red}}
              }
});
```

As you can see, you provide a list with one entry for each state. The entry provides the state specification followed by the data for each state. The order of the states specified is unimportant. 

Specifying the default state is not necessary since all settings for default state are also included as separate parameters in the blueprint. Therefore the same specification as the one above can also be written as:

```c++
auto pMySkin = ColorSkin::create( 
{
    .color = Color::Green,
    .margin = 2,
    .padding = 2,
    .states = { {State::Hovered, {.color = Color::Yellow}},
                {State::Pressed, {.contentShift = {2,2}, .color = Color::Red}}
              }
});
```

This saves some typing and looks cleaner, but it becomes harder to see how state Default differs from the rest. If both specifications are included, the values in .states overrides the top-level values for default state.
