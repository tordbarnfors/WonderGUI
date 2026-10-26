---
title: 'DoubleSkin'
weight: 50
---


# DoubleSkin

DoubleSkin combines two skins into one, rendering one ontop of the other.

## Properties

| Property  | Description                 |
| --------- | --------------------------- |
| skinInSkin | Set to true if front skin should be drawn inside back skins content area. Defaults to false. |
| skins | Array of two Skin_p. The first entry is front skin, second entry is back skin. |

## Layout

The two skins contained in the DoubleSkin are here referred to as *frontSkin* and *backSkin*. The *frontSkin* is the first in the array of the blueprint and *backSkin* is the second.

The *skinInSkin* property decides wether frontSkin should be placed inside the backSkins content area or just be placed over the whole of backSkin.

Normally any content is placed inside frontSkins content area, but if *padding* is set in DoubleSkin, that value overrides and specifies a content area separated from the two contained skins.

Setting *margin* in DoubleSkin works as normal, adding to the margin of the contained skins.

Setting *layer* changes layer for both contained skins, but any of them will override it with their own *layer* if set.

## Examples

