---
title: 'MeterSkins'
weight: 20
BookToC: false
---


# MeterSkins

MeterSkins are skins that change their appearance depending on the value of the widget and can be used for things such as progress indicators, value indicators, knobs etc.

Skins are provided two values by the widget at the time of rendering in the range 0.0 - 1.0. Widgets such as *Slider*, *Knob* and *Timer* use this value to communicate their value relative to their max and min values.

A MeterSkin simply draws itself differently depending on the values. Most Widgets only submits one value and most MeterSkins only listens to the first value. Among the exceptions are *RangeSlider* that submits begin and end values for the range and *FillMeterSkin* that fills an area between the two values if two are presented.



