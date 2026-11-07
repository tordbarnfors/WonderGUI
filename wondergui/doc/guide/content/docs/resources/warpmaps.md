---
title: 'Warpmaps'
weight: 35
---

# Warpmaps

A Warpmap bends a surface or an edgemap into another shape when it is drawn: a gauge or a progress ring from a straight bar, text along an arc, a waving flag. The source is drawn as it is and the Warpmap decides where each part of it ends up.

A Warpmap covers a rectangle of pixels with a mesh of triangles. Each vertex has a position in the Warpmap (*x* and *y*, in spx from its top-left corner), the position in the source that is drawn there (*u* along the width of the source and *v* along its height, in spx of the source: 64 per pixel of the edgemap or surface, relative to the source rectangle of a blit) and a weight *w* for perspective. Positions within a triangle are interpolated from its corners. Where a position falls outside the source nothing is drawn, so the shape of what is drawn is given by the vertices, with anti-aliased borders. Nothing is drawn outside the triangles either.

A Warpmap is made for a source of a certain size. It isn't stretched to fit a source of another size, that takes other vertices.

Warpmaps are *semi-mutable*: size and the number of vertices and triangles are set when created, while vertices, triangles and periods can be changed. Each graphics backend has its own Warpmap subtype, created by its WarpmapFactory (`GfxDevice::warpmapFactory()`), or through a WarpGenerator such as CircularWarp.



## Drawing

```c++
	pDevice->warpDrawEdgemap( dest, pWarpmap, pEdgemap );

	pDevice->setBlitSource( pSurface );
	pDevice->warpBlit( dest, pWarpmap );						// The whole surface.
	pDevice->warpBlit( dest, pWarpmap, sourceRect );			// A rectangle of it.
```

The Warpmap is placed with its top-left corner at *dest*, rounded to whole pixels, and nothing is drawn outside its rectangle. Blend mode, tint color and tint apply as for any other drawing, the tint in canvas space. Surfaces are read with their sample method. Nothing outside the source rectangle is read, as with stretchBlit(): along its borders the surface is read as if the edge pixels of the rectangle continued outside it. A tiling surface wraps around instead. There is no supersampling when a Warpmap shrinks a surface, use a mipmapped surface for that.

Draws are rendered when the canvas update ends and read the Warpmap then. Changing the content of a Warpmap changes all of its draws that aren't rendered yet, so use a Warpmap for each shape drawn within the same canvas update.

All methods take a *source offset* that is added to the position before reading, so a ring can be turned or a pattern scrolled without changing the Warpmap. It is in spx of the source, like u and v.



## Periods

An axis can be periodic (*uPeriod* or *vPeriod* above 0, in spx of the source), as when u is the angle of a ring. Positions are then read modulo the period, after they have been interpolated, so the corners of a triangle keep values close to each other also where they go past the period: a triangle across the seam of a ring has corners with u just below the period and just above it, rather than just above 0.

With a period of the size of the source it wraps around, which for an edgemap or surface that is tiling is seamless. A longer period leaves a gap between the copies of the source, as a 270 degree arc of a source *w* spx wide has uPeriod = w * 360 / 270. A pixel in the gap takes its colors from the nearest copy, so they don't jump where the copies meet.

Periods are given in the blueprint when created, and can be changed with `setPeriods()` or together with all vertices and triangles with `importMesh()`.



## CircularWarp

CircularWarp makes and keeps a Warpmap that bends the source around a circle: its width along the circle, from *startAngle* and *sweep* degrees on, its height from *topRadius* to *bottomRadius*. *sourceSize* is the size of the source it is made for, in spx.

```c++
	auto pGauge = CircularWarp::create( WGBP(CircularWarp,
										_.size = { 200, 200 },
										_.sourceSize = pEdgemap->pixelSize() * 64,
										_.topRadius = 98*64,
										_.bottomRadius = 70*64,
										_.startAngle = -135.f,
										_.sweep = 270.f ), pDevice->warpmapFactory() );

	pDevice->warpDrawEdgemap( { 10*64, 10*64 }, pGauge->generate(), pEdgemap );
```

The cell size decides how closely the Warpmap follows the circle and is calculated from *precision* (largest position error in pixels) at the smaller radius unless set, see [WarpGenerators](#warpgenerators). For a disc, precision is kept from half its radius and outwards. A radius of 0 gives a disc, which only works for content that doesn't change from left to right close to the center.

To turn the ring, draw it with the offset from `rotationOffset()` rather than generating it again. It turns the ring by a number of degrees, clockwise if positive, as if *startAngle* was that much larger, which costs nothing but the draw:

```c++
	angle += speed * seconds;
	pDevice->warpDrawEdgemap( { 10*64, 10*64 }, pGauge->generate(), pEdgemap, pGauge->rotationOffset(angle) );
```




## RoundedRectWarp

RoundedRectWarp makes and keeps a Warpmap that bends the source around the outline of a rounded rectangle: *rect* with corners of *cornerRadius*. The width of the source goes along the outline, clockwise from *start* and covering *sweep* of it, both as parts of the whole outline with 0 at the middle of the top side. Its height goes from *topDistance* to *bottomDistance*, distances from the outline that are positive outwards.

```c++
	auto pFocusRing = RoundedRectWarp::create( WGBP(RoundedRectWarp,
										_.size = { 140, 60 },
										_.sourceSize = pEdgemap->pixelSize() * 64,
										_.rect = { 10*64, 10*64, 120*64, 40*64 },
										_.cornerRadius = 8*64,
										_.topDistance = 3*64,
										_.bottomDistance = -1*64 ), pDevice->warpmapFactory() );

	pDevice->warpDrawEdgemap( { 10*64, 10*64 }, pFocusRing->generate(), pEdgemap, pFocusRing->shiftOffset(part) );		// Marching ants.
```

Positions along the outline are those of the nearest point on it. The source keeps its width along the straight sides and is spread or squeezed around the corners, more the further from the outline, so placing the outline in the middle of the band gives the most even result. Corners stay round on the outside. On the inside they become mitered like a picture frame where the band goes deeper than *cornerRadius*, and since a Warpmap interpolates across the miter, that takes small cells to look clean. *cornerRadius* is limited to half the shorter side of *rect*, so a square with corners of half its side is a circle.

`shiftOffset()` gives the offset that moves the source along the outline by a part of it, clockwise if positive, as if *start* was that much larger. Both offsets go by the sweep and source size of the generated Warpmap.



## CylinderWarp

CylinderWarp makes and keeps a Warpmap that wraps the source around a cylinder seen from the side, such as the reel of a slot machine. The cylinder fills the Warpmap: with *axis* X it lies down and turns up and down, with *axis* Y it stands up and turns sideways. Along the axis the source is stretched to the length of the cylinder. Across it, *span* degrees of the cylinder are seen, centered on the side facing the viewer, and the source is squeezed more and more towards the rims. Around the cylinder the source covers *sweep* degrees from *startAngle*, angles counted from the side facing the viewer, downwards or to the right. That axis of the source is periodic, so with a sweep of 360 it wraps around the cylinder seamlessly.

```c++
	auto pReel = CylinderWarp::create( WGBP(CylinderWarp,
										_.size = { 80, 200 },
										_.sourceSize = pReelStrip->pixelSize() * 64,
										_.span = 110.f ), pDevice->warpmapFactory() );

	auto pMap = pReel->generate();

	angle += speed * seconds;
	pDevice->setBlitSource( pReelStrip );
	pDevice->warpBlit( { 10*64, 10*64 }, pMap, pReel->rotationOffset(angle) );
```

`rotationOffset()` turns the cylinder by a number of degrees as if *startAngle* was that much larger, so a spinning reel is generated once. Positions change linearly along the axis, so cells span the length of the cylinder unless set, and across it the cell size is calculated from *precision* at the rims, where the cylinder curves away the most. A *span* close to 180 degrees therefore takes small cells. The view is straight from the side, without perspective, and the cylinder isn't shaded: draw it with a tint that gets darker towards the rims for that.



## PerspectiveWarp

PerspectiveWarp makes and keeps a Warpmap that places the source on a flat plane seen in perspective: a floor leaning into the distance, text scrolling away like movie credits, a card or a door turning around an axis, or a picture pinned onto a sign seen at an angle. The further away a part of the plane is, the smaller it gets.

The plane is given by its four *corners*: where the top-left, top-right, bottom-right and bottom-left corners of the source end up, in spx from the top-left corner of the Warpmap. Any convex shape with four corners can be given, and the source is drawn between them as a plane seen in perspective. Corners in the reverse order (counter clockwise) mirror the source, as when the back of a turning card faces the viewer. Parts outside the Warpmap are cut off, so make it large enough to hold the corners.

`rotated()` gives the corners of a rectangle on screen after rotating it in 3D and seeing it from an eye in front of the screen, so that is what you usually set *corners* with:

```c++
	RectSPX cardRect = { 20*64, 30*64, 64*64, 96*64 };

	auto pCard = PerspectiveWarp::create( WGBP(PerspectiveWarp,
										_.size = { 110, 160 },
										_.sourceSize = pCardFace->pixelSize() * 64,
										_.corners = PerspectiveWarp::rotated( cardRect, 0.f, 30.f, 0.f, 300*64 ) ),
										pDevice->warpmapFactory() );

	turn += speed * seconds;
	pCard->setCorners( PerspectiveWarp::rotated( cardRect, 0.f, turn, 0.f, 300*64 ) );

	pDevice->setBlitSource( pCardFace );
	pDevice->warpBlit( { 10*64, 10*64 }, pCard->generate() );
```

`rotated( rect, tilt, turn, roll, viewDistance, pivot, eye )` rotates the rectangle around its *pivot* (relative to the rectangle, its center by default): first *roll* degrees clockwise within the screen, then *turn* degrees around the vertical axis, the right side moving away, then *tilt* degrees around the horizontal axis, the top moving away. It is then seen from an eye *viewDistance* spx in front of the screen, straight in front of the pivot unless *eye* is given (in spx, in the same space as the rectangle). A shorter distance gives stronger perspective, around two to three times the size of the rectangle looks natural, and 0 gives none. Rectangles seen from the same eye share their vanishing points, as for a row of cards. An eye above a rectangle tilted around its bottom edge sees it as a floor going towards a horizon:

```c++
	// A floor 300 x 150 pixels, seen from above its middle at the top of the Warpmap.

	_.corners = PerspectiveWarp::rotated( { 0, 0, 300*64, 150*64 }, 70.f, 0.f, 0.f, 150*64, { 0.5f, 1.f }, CoordSPX(150*64, 0) )
```

Changing the angles takes generating again, which is cheap: a few dozen calculations for the whole Warpmap. Moving the source along the plane, such as scrolling credits or a moving floor, is done with a source offset when drawing instead, as for any Warpmap. A tiling surface with a source rectangle larger than the surface makes a floor of many tiles.

The Warpmap is the shape of the corners, made about a pixel larger so that its borders are antialiased, and cut off by the borders of the Warpmap and just before the horizon of the plane. It is split into a fan of at most 7 triangles, with weights for perspective, so positions are exact everywhere, however steep the angle. Cell size and precision don't apply to a PerspectiveWarp. Near the horizon the source is shrunk a lot, which a mipmapped surface draws better on backends that support mipmaps. Showing something else on the back side, and hiding it, is left to the caller: the corners turn counter clockwise when the back faces the viewer.



## WarpGenerators

CircularWarp, RoundedRectWarp, CylinderWarp and PerspectiveWarp are WarpGenerators: they make a Warpmap from a few settings and keep it. Size, source size and generate() are the same for all WarpGenerators, and cell size and precision for those that make the Warpmap from a grid.

generate() updates the Warpmap after settings have changed and returns it. A Warpmap that is held elsewhere, such as by a draw that isn't rendered yet, is never changed: generate() then makes a new one, as it does when size, cell size or the number of triangles has changed. So draw what generate() returns rather than keeping it, and one generator can be drawn several times with different settings within the same canvas update.

For now, generators other than PerspectiveWarp calculate the position in the source at the nodes of a grid, one node every *cellSize.w* pixels horizontally and *cellSize.h* pixels vertically plus one at the right and bottom edge, and make the Warpmap from that with two triangles for each cell of the grid. Unless set, the cell size is calculated from *precision* and the shape: within 4 to 64 pixels along curves, and up to the size of the Warpmap along axes where positions change linearly. Width and height can be set separately, a 0 for either has it calculated. Positions are linear within each triangle, so a circle of radius r is off by roughly cellSize² / (4 * r) pixels, along the diagonals of the cells.

To make your own, subclass WarpGenerator and implement _generateNodes(), which fills in the positions of all nodes, and _calcCellSize(), which picks the width and height of cells from *precision*. _cellSizeForRadius() gives the cell size for a curve of a given radius. Override _uPeriod() and _vPeriod() if an axis is periodic, and set *m_bDirty* when one of your settings changes. A generator can also make its own triangles rather than a grid, as PerspectiveWarp does, by overriding _meshSize() and _generateMesh().



## Making your own

```c++
	// A wave, 6 pixels high, from a strip of 21 x 2 vertices and 40 triangles.

	const int columns = 20;
	SizeI size = { 160, 120 };

	std::vector<Warpmap::Vertex> vertices;
	std::vector<Warpmap::Triangle> triangles;

	for( int col = 0 ; col <= columns ; col++ )
	{
		spx x = col * size.w * 64 / columns;
		spx wave = spx(std::lround(6 * 64 * std::sin(x / 64 * 0.1)));

		vertices.push_back( { x, 0, x, wave } );
		vertices.push_back( { x, size.h * 64, x, size.h * 64 + wave } );

		if( col > 0 )
		{
			int i = col * 2;
			triangles.push_back( { i - 2, i, i - 1 } );
			triangles.push_back( { i, i + 1, i - 1 } );
		}
	}

	auto pWarpmap = pDevice->warpmapFactory()->createWarpmap( WGBP(Warpmap, _.size = size, _.triangles = int(triangles.size()), _.vertices = int(vertices.size())),
															  vertices.data(), triangles.data() );
```

A new Warpmap created without vertices and triangles maps its rectangle straight onto the source, pixel for pixel, with its first four vertices and two triangles, and leaves the rest of its triangles with all corners in the first vertex, which draws nothing. Unused triangles can be left like that, so a Warpmap can be made with room for the most triangles a shape needs. `importVertices()`, `importTriangles()` and `importMesh()` change the content later.

Some things to keep in mind:

* **Edges between triangles** don't show, as long as neighbouring triangles share the vertices along their common edge. A pixel is drawn by the one triangle that its center is inside of, and isn't antialiased along the edges of triangles.

* **The border of what is drawn** is where the position leaves the source, which can cross the inside of a triangle. That is where it is antialiased, so let the triangles reach about a pixel beyond the part of the source that is drawn, with u and v going on outside the source. Otherwise the border is cut off without antialiasing along the edge of the mesh.

* **Triangles may not overlap.** What is drawn where they do is undefined. The order of the corners doesn't matter, clockwise or counter clockwise.

* **Positions are linear within a triangle,** unless it is in perspective. Smaller triangles follow curved shapes more closely, a circle of radius r drawn through triangles with sides of length l is off by roughly l² / (8 * r) pixels.

* **Perspective** is given by the weights. A plane seen in perspective, such as a card turned in 3D, is exact with a single quad of two triangles when the weight of each corner is proportional to 1 / its distance from the eye, and positions are the ones at the corners. Only the ratios between the weights of a triangle matter. *Warpmap::weightOne* is 1.0, and is the weight of vertices that don't set one.



## How it is drawn

The exact rules all backends follow are given in the documentation of the Warpmap class. In short: a pixel is drawn by the triangle its center is inside of, with a rule for centers exactly on an edge so that no pixel is drawn twice. The position at a point is interpolated from the corners of that triangle, with perspective:

	u = (b0*w0*u0 + b1*w1*u1 + b2*w2*u2) / (b0*w0 + b1*w1 + b2*w2)

b0, b1 and b2 being the barycentric coordinates of the point in the triangle, and the same for v. That triangle gives the positions over the whole pixel, also where it reaches outside the triangle. The part of a pixel covered is the part of its area whose position is inside the source (or, for an edgemap, inside each segment), which is then blended as any anti-aliased edge, see [Blending and transparency](../basics/blending_and_transparency). Colors are taken at the position of the center of the pixel.

ReferenceBackend measures coverage by sampling each pixel at many points and divides for perspective at each point. The other backends estimate coverage from the position at the center of the pixel and how fast it changes, taking each border and edge as straight over the pixel. That is off by a few percent where an edge bends sharply within a pixel. They may also divide for perspective only once every few pixels, up to 16, and interpolate linearly in between, keeping within about a quarter of a pixel.

**For now, only the software backends draw Warpmaps.** The GPU backends are still to be changed over from the grid of nodes that Warpmaps had before.
