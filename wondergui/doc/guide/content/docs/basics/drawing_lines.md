---
title: 'Drawing lines'
weight: 27
---

# Drawing lines

GfxDevice draws straight lines of any thickness and direction with drawLine(). Each end of a line can be given a cap: flat, square, round, cut along x or y, or pointed like a spear.

```c++
	pDevice->drawLine( begin, end, color, 2*64 );										// No caps, as before.

	pDevice->drawLine( begin, end, color, 4*64, LineCap::Round );						// Both ends.
	pDevice->drawLine( begin, end, color, 4*64, { LineCap::Flat, LineCap::Spear(300) } );	// Begin, end.

	pDevice->drawLine( begin, Direction::Down, 100*64, color, 3*64, LineCap::Square );
```

Positions, lengths and thickness are in spx, see *Understanding screen geometry*. Blend mode, tint color and tint apply as for any other drawing.



## Lines without caps

Without caps (LineCap::None at both ends) a line is drawn as it always has been. Its ends are rounded to whole pixels and it runs through the middle of the pixels at begin and end. It is drawn one column of pixels at a time, from begin up to but not including end, along the axis the line mainly follows. The ends are cut along the other axis: vertically if the line runs mostly along x, horizontally otherwise. A line from a pixel to itself draws nothing.

This is the fastest way to draw a line. A line drawn from a position in a direction without caps is the exception: it is a fill of the rectangle it covers, placed exactly.



## Lines with caps

As soon as one end has another cap, the line is drawn as an exact shape:

- Its ends are exact positions on the canvas, like the corners of a rectangle given to fill(). They are not rounded to whole pixels. A line from (10,10) to (100,10) runs along the border between pixel rows 9 and 10, so a line 1 pixel thick there covers half of each. Place it at y = 10.5 to fill row 10, as you would place a fill.
- The line is the band of points within half its thickness of the line through begin and end, cut at each end by its cap.
- Pixels are covered by the exact area of the line within them, also at the ends.

| Cap              | The end of the line                                                                     |
| ---------------- | --------------------------------------------------------------------------------------- |
| `Flat`           | Cut square to the line, through the end.                                                |
| `Square`         | Cut square to the line, half the thickness beyond the end.                              |
| `Round`          | A half circle with the diameter of the thickness, centered on the end.                  |
| `Vertical`       | Cut along a vertical line through the end.                                              |
| `Horizontal`     | Cut along a horizontal line through the end.                                            |
| `Spear(length)`  | Pointed, with the tip on the end, reaching full thickness *length* percent of the thickness from the tip. |
| `None`           | When the other end has a cap: cut along the axis the line doesn't mainly follow, through the end. |

The length of a spear is 25 to 800 percent of the thickness and rounded to the nearest 25. LineCap::Spear() without a length is 200 percent. Spears of a line shorter than they are meet and cut each other short.

Vertical and horizontal cuts let the ends of lines meet without gaps or overlaps whatever their directions, which suits line charts: draw the segments of the chart with `LineCap::Vertical` and each starts where the one before ended. The more a line runs along such a cut, the further its end reaches beyond the endpoint: a line 5 degrees off a horizontal cut reaches about 6 thicknesses beyond it, as a long point. A vertical cut on a line that has no horizontal extent, or a horizontal cut on one with no vertical extent, leaves nothing to draw.

Where two lines meet inside a pixel rather than on its border, each covers part of the pixel and they are blended one after the other. That leaves a faint seam, more visible the more transparent the lines are. Let lines meet on whole pixels to avoid it, as the segments of a chart can by joining at whole pixels along x.

A line with no length runs along x, so round caps make a dot and square caps a square.

Lines meeting at round or square caps overlap, which shows where they are semi-transparent. There are no joins between lines, use a polyline for lines that are to be joined.



## Polylines

drawPolyline() draws a line through a number of points as one shape, with the same thickness all the way:

```c++
	CoordSPX points[4] = { {10*64,10*64}, {40*64,10*64}, {40*64,60*64}, {90*64,60*64} };

	pDevice->drawPolyline( 4, points, color, 3*64 );
	pDevice->drawPolyline( 4, points, color, 3*64, { LineCap::Round, LineCap::Spear(300) } );
```

Its points are exact positions on the canvas, as the ends of a line with caps are, and it is drawn as exactly. The segments from one point to the next end flat at the points in between and are joined there by a round join: the wedge left open between two segments on the outside of a turn is filled by that part of a disc around the point, with the diameter of the thickness. The caps are for the two ends of the polyline. LineCap::None, the default, cuts an end along the axis its segment doesn't mainly follow, through the endpoint.

What sets a polyline apart from lines drawn one after another is that its segments and joins are drawn together: each pixel is blended once, with the part of it they cover. A polyline in a semi-transparent color is as transparent at its joints as everywhere else, and no background shows through between two segments. Wires between nodes are what it was made for.

That holds for segments joined to each other, not for a polyline that crosses itself or comes back to within its thickness of itself, as in a zigzag closing in or a turn sharper than the segments are long enough for. There segments that aren't joined cover the same pixels, which a backend may blend once or several times, so they can come out darker in a semi-transparent color, and GPU backends can leave slivers of the tightest turns out. Draw such polylines in opaque colors, where it hardly shows.

A polyline has at most GfxDevice::maxPolylinePoints() points. Points that are the same as the one before them are left out, a polyline with one segment is drawn by drawLine(), and an end is only cut out of its own segment.

A curve is drawn as a polyline through points along it, close enough for the segments not to show: a few pixels apart for a curve that bends gently.



## Speed and accuracy

Caps cost a little extra for each line, more the thicker it is. On SoftBackend a capped line takes about 1 to 5 microseconds more than one without caps on a desktop CPU. GPU backends draw them at about the same speed.

Polylines cost more than lines: a polyline of two segments takes about four times as long as a line with caps, on SoftBackend and GPU backends alike. SoftBackend and LinearBackend draw all of a polyline as an exact shape, also its long straight segments. GPU backends work out what three segments and two joins cover for the pixels close to a joint, and visit a pixel once for every segment within about half the thickness of it, so segments much shorter than a polyline is thick are costly there. Keep the points of a curve a few pixels apart.

SoftBackend and LinearBackend draw the parts of a line around its ends as exact shapes, and the whole columns of pixels between them with the same code as lines without caps. That code estimates the coverage of the pixels along the edges of the line from the middle of each column, which can be off by up to an eighth of a pixel for lines near 45 degrees, as for lines without caps. The GPU backends work out the exact coverage of straight edges for every pixel, but estimate the round caps of thin lines and small dots.
