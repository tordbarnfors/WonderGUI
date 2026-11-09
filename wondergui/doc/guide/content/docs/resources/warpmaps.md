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

The Warpmap is a ring of sectors, split into parts across the ring, which goes all the way around and a pixel beyond both radii whatever the sweep, so that the ring can be turned by an offset. How many sectors and parts it takes is calculated from *precision* (see [WarpGenerators](#warpgenerators)). A radius of 0 gives a disc, where the sectors meet in the center, which only works for content that doesn't change much from left to right close to the center.

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

Positions along the outline are those of the nearest point on it. The source keeps its width along the straight sides and is spread or squeezed around the corners, more the further from the outline, so placing the outline in the middle of the band gives the most even result. Corners stay round on the outside. On the inside they become mitered like a picture frame where the band goes deeper than *cornerRadius*. *cornerRadius* is limited to half the shorter side of *rect*, so a square with corners of half its side is a circle.

The Warpmap follows the outline: a quad across the band for each straight side, which is exact, and sectors split into parts across the band around the corners, as many as *precision* takes. It goes all the way around and a pixel beyond the band, whatever the sweep.

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

`rotationOffset()` turns the cylinder by a number of degrees as if *startAngle* was that much larger, so a spinning reel is generated once. The Warpmap is a row of strips along the axis, where positions change linearly. Across it strips are as wide as *precision* allows, wide in the middle and narrower towards the rims, where the cylinder curves away the most. A *span* close to 180 degrees therefore takes many strips. The view is straight from the side, without perspective, and the cylinder isn't shaded: draw it with a tint that gets darker towards the rims for that.



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

The Warpmap is the shape of the corners, made about a pixel larger so that its borders are antialiased, and cut off by the borders of the Warpmap and just before the horizon of the plane. It is split into a fan of at most 7 triangles, with weights for perspective, so positions are exact everywhere, however steep the angle. Precision doesn't apply to a PerspectiveWarp. Near the horizon the source is shrunk a lot, which a mipmapped surface draws better on backends that support mipmaps. Showing something else on the back side, and hiding it, is left to the caller: the corners turn counter clockwise when the back faces the viewer.



## BallWarp

BallWarp makes and keeps a Warpmap that wraps the source around a ball, such as a globe from a map of the world. The ball is centered in the Warpmap, as large as fits with its antialiased outline, and is seen from far away, straight on and without perspective, so half of it shows. The source is a map of the whole ball in equirectangular projection: its width goes all the way around, from 180 degrees west at the left edge to 180 degrees east at the right, and its height from the north pole at the top to the south pole at the bottom. The middle of the map faces the viewer. The width is periodic, so the map wraps around seamlessly.

```c++
	auto pGlobe = BallWarp::create( WGBP(BallWarp,
										_.size = { 200, 200 },
										_.sourceSize = pWorldMap->pixelSize() * 64,
										_.tilt = 23.5f ), pDevice->warpmapFactory() );

	degrees += speed * seconds;
	pDevice->setBlitSource( pWorldMap );
	pDevice->warpBlit( { 10*64, 10*64 }, pGlobe->generate(), pGlobe->rotationOffset(degrees) );
```

*tilt* leans the north pole towards the viewer (-90 to 90 degrees, 90 looking straight down on it) and *roll* turns the axis of the ball clockwise on screen, both set with `setOrientation()`. `rotationOffset()` spins the ball around its axis, from west to east for positive degrees as the Earth turns, so a spinning globe is generated once. `radius()` gives the radius of the ball in spx.

The Warpmap is a grid of meridians and parallels, between which positions are linear, cut where the ball turns away from the viewer and with a vertex of its own at the poles for each piece. Pieces are as large as *precision* allows, the same all over the ball, with points added along the outline where that keeps it within half a pixel. The outline isn't a border of the source, so it is antialiased by the coverage of the vertices: a row of triangles from full coverage half a pixel inside it to none half a pixel outside. The number of vertices and triangles only depends on size and precision, so changing tilt and roll updates the Warpmap in place. The ball isn't shaded: draw it with a tint for that, such as one getting darker away from where the light comes from.



## LensWarp

LensWarp makes and keeps a Warpmap of a round lens that magnifies what is under it, such as a magnifying glass held over some graphics. The lens is centered in the Warpmap, as large as fits with its antialiased outline. Its source is what is under the lens: normally a source rectangle of the same size as the Warpmap, at the same place in the surface that holds the graphics, so the source is as large as the Warpmap where nothing is magnified.

```c++
	auto pLens = LensWarp::create( WGBP(LensWarp,
										_.size = { 120, 120 },
										_.sourceSize = { 120*64, 120*64 },
										_.magnification = 2.f,
										_.falloff = 0.3f ), pDevice->warpmapFactory() );

	// pScene holds the graphics, drawn 1:1 at the top-left corner of the canvas.

	pDevice->setBlitSource( pScene );
	pDevice->warpBlit( pos, pLens->generate(), { pos.x, pos.y, 120*64, 120*64 } );
```

In the middle of the lens the source is magnified by *magnification*, 1 or more. Over the outer part of its radius given by *falloff*, 0 to 1, the magnification falls off to none at the rim, where the source meets what is around the lens without a seam, squeezed more the shorter the falloff is:

* *falloff* 0 magnifies all of the lens evenly, like a magnifying glass, with a sharp edge.
* *falloff* 0.2 to 0.4 magnifies the middle evenly and squeezes the source towards the rim, like thick glass.
* *falloff* 1 magnifies the most in the middle and less and less out to the rim, like a bubble.

Move the lens by drawing it somewhere else with the source rectangle moved along, which doesn't take generating again. A source offset moves the source within the lens in steps finer than a pixel. Changing magnification or falloff does take generating again. Without falloff the Warpmap is then only updated, as only positions in the source change; with falloff it can get more or fewer rings.

A lens is drawn over what is already there, so graphics with transparency in them are blended twice where the lens is. Draw the lens from a surface with the finished graphics on it, such as the one a CanvasCapsule renders its children into. An edgemap seen through a lens stays sharp however much it is magnified, and a surface with *SampleMethod::Nearest* makes a loupe that shows its pixels. A frame, a highlight or a shadow are drawn separately.

The Warpmap is the middle of the lens as one polygon, split into a strip from top to bottom, where positions are linear, rings through the falloff as close as *precision* allows, every other one turned by half a piece and each with as few pieces as *precision* allows, fewer close to the middle, and a ring of triangles around the outline from full coverage of the vertices half a pixel inside it to none half a pixel outside, which antialiases it. Pieces along the rim are as long as keeps the outline within half a pixel. Without falloff the Warpmap is about 150 triangles for a lens 240 pixels across and draws about as fast as stretching the same area. A falloff takes rings, one to two and a half thousand triangles, which a software backend draws 2 to 3 times slower than without falloff; a GPU doesn't notice. Rings are at least two pixels apart, so where the falloff squeezes the source into each pixel, and over its first pixels, positions follow the falloff a bit less closely than *precision* asks for, which doesn't show.



## WarpGenerators

CircularWarp, RoundedRectWarp, CylinderWarp, PerspectiveWarp, BallWarp and LensWarp are WarpGenerators: they make a Warpmap from a few settings and keep it. Size, source size, precision and generate() are the same for all WarpGenerators, though precision doesn't apply to PerspectiveWarp, which is exact. Each generator chooses the size of its pieces from precision, as suits its shape, so there is no setting for that.

*precision* means the same for all of them, and higher precision gives more triangles. Positions are kept within 1 / *precision* pixels (*WarpGenerator::c_tolerance* / *precision*): the largest distance on screen between where a pixel shows the source and where it would show it exactly. It is 1 by default, a pixel, which suits most GUIs; 2 (half a pixel) is about as high as shows, and 0.5 (two pixels) makes generating, memory and streaming cheaper where that matters, while it hardly changes the time to draw, which mostly goes by the pixels. Curved outlines are kept within half a pixel (*WarpGenerator::c_outlineTolerance*) whatever precision is set, as an outline off by a pixel or more shows as corners where it should be round, while the inside of a shape off as much hardly shows: the radii of rings and rounded corners, the outline of a ball and the rim of a lens. Higher precision takes more triangles, how many more depending on the shape: a cylinder only gets more strips towards its rims, while a ball gets more pieces both ways. Pieces are sized for a little less than that distance, to leave room for rounding the vertices to spx. They are never narrower than two pixels, as narrower ones would only be needed where the source is squeezed into each pixel, such as the rims of a cylinder seen almost to its sides or a steep falloff of a lens. Positions there can be off by more than precision asks for, which doesn't show, as the source is still sampled once per pixel, and is cheaper to draw. Positions in the source are whole spx too, so where the source is magnified many times, such as a small surface wrapped around a large cylinder, positions closer than about the magnification / 128 pixels can't be kept, whatever the precision.

generate() updates the Warpmap after settings have changed and returns it. A Warpmap that is held elsewhere, such as by a draw that isn't rendered yet, is never changed: generate() then makes a new one, as it does when size or the number of vertices and triangles has changed. Settings that change the shape, such as the radii of a CircularWarp, can change that, while angles and offsets don't. So draw what generate() returns rather than keeping it, and one generator can be drawn several times with different settings within the same canvas update.

To make your own, subclass WarpGenerator and implement _meshSize(), which gives the number of vertices and triangles, and _generateMesh(), which fills them in, sizing the pieces from *_pieceTolerance()* and *_outlineTolerance()*, which give the distances in pixels that *precision* allows. Override _uPeriod() and _vPeriod() if an axis is periodic, and set *m_bDirty* when one of your settings changes. WarpGenerator has integer maths for that, for CPUs without an FPU: fixed point factors, CORDIC angles and sines, and integer square roots.



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

Triangles take their corners by index in the vertices, 16 bits each, so a Warpmap has at most *Warpmap::maxVertices* (65536) vertices and *Warpmap::maxTriangles* (twice that) triangles. Generators keep within that by making their pieces larger where a shape is very large or *precision* very high.

A new Warpmap created without vertices and triangles maps its rectangle straight onto the source, pixel for pixel, with its first four vertices and two triangles, and leaves the rest of its triangles with all corners in the first vertex, which draws nothing. Unused triangles can be left like that, so a Warpmap can be made with room for the most triangles a shape needs. `importVertices()`, `importTriangles()` and `importMesh()` change the content later.

Some things to keep in mind:

* **Edges between triangles** don't show, as long as neighbouring triangles share the vertices along their common edge. A pixel is drawn by the one triangle that its center is inside of, and isn't antialiased along the edges of triangles.

* **The border of what is drawn** is where the position leaves the source, which can cross the inside of a triangle. That is where it is antialiased, so let the triangles reach about a pixel beyond the part of the source that is drawn, with u and v going on outside the source. Otherwise the border is cut off without antialiasing along the edge of the mesh.

* **Triangles may not overlap.** What is drawn where they do is undefined. The order of the corners doesn't matter, clockwise or counter clockwise.

* **Positions are linear within a triangle,** unless it is in perspective. Smaller triangles follow curved shapes more closely, a circle of radius r drawn through triangles with sides of length l is off by roughly l² / (8 * r) pixels.

* **Perspective** is given by the weights. A plane seen in perspective, such as a card turned in 3D, is exact with a single quad of two triangles when the weight of each corner is proportional to 1 / its distance from the eye, and positions are the ones at the corners. Only the ratios between the weights of a triangle matter. *Warpmap::weightOne* is 1.0, and is the weight of vertices that don't set one.

* **Outlines that aren't borders of the source** are antialiased by the coverage of the vertices, *Warpmap::coverageOne* unless set. It is interpolated linearly over a triangle and multiplies the coverage of each pixel, so a row of triangles from coverageOne half a pixel inside an outline to 0 half a pixel outside it antialiases the outline, as BallWarp does around a ball covered by a map. Triangles with all corners at coverageOne cost nothing extra.



## How it is drawn

The exact rules all backends follow are given in the documentation of the Warpmap class. In short: a pixel is drawn by the triangle its center is inside of, with a rule for centers exactly on an edge so that no pixel is drawn twice. The position at a point is interpolated from the corners of that triangle, with perspective:

	u = (b0*w0*u0 + b1*w1*u1 + b2*w2*u2) / (b0*w0 + b1*w1 + b2*w2)

b0, b1 and b2 being the barycentric coordinates of the point in the triangle, and the same for v. That triangle gives the positions over the whole pixel, also where it reaches outside the triangle. The part of a pixel covered is the part of its area whose position is inside the source (or, for an edgemap, inside each segment), which is then blended as any anti-aliased edge, see [Blending and transparency](../basics/blending_and_transparency). Colors are taken at the position of the center of the pixel.

ReferenceBackend measures coverage by sampling each pixel at many points and divides for perspective at each point. The other backends estimate coverage from the position at the center of the pixel and how fast it changes, taking each border and edge as straight over the pixel. That is off by a few percent where an edge bends sharply within a pixel. They may also divide for perspective only once every few pixels, up to 16, and interpolate linearly in between, keeping within about a quarter of a pixel.

The DX12 and Metal backends draw Warpmaps as the OpenGL backend does.
