---
title: 'Warpmaps'
weight: 35
---

# Warpmaps

A Warpmap bends a surface or an edgemap into another shape when it is drawn: a gauge or a progress ring from a straight bar, text along an arc, a waving flag. The source is drawn as it is and the Warpmap decides where each part of it ends up.

A Warpmap covers a rectangle of pixels with a grid of nodes, one node every *cellSize* pixels in both directions plus one at the right and bottom edge. Each node holds the position in the source that is drawn at that point, normalized so that 0.0 -> 1.0 covers the source: u along its width and v along its height. Positions between nodes are interpolated. Where a position falls outside the source nothing is drawn, so the shape of what is drawn is given by the nodes, with anti-aliased borders.

Warpmaps are *semi-mutable*: size and cell size are set when created, nodes and periods can be changed. Each graphics backend has its own Warpmap subtype, created by its WarpmapFactory (`GfxDevice::warpmapFactory()`), or through a generator such as CircularWarp.



## Drawing

```c++
	pDevice->warpDrawEdgemap( dest, pWarpmap, pEdgemap );

	pDevice->setBlitSource( pSurface );
	pDevice->warpBlit( dest, pWarpmap );						// The whole surface.
	pDevice->warpBlit( dest, pWarpmap, sourceRect );			// A rectangle of it.
```

The Warpmap is placed with its top-left corner at *dest*, rounded to whole pixels, and nothing is drawn outside its rectangle. Blend mode, tint color and tint apply as for any other drawing, the tint in canvas space. Surfaces are read with their sample method. Nothing outside the source rectangle is read, as with stretchBlit(): along its borders the surface is read as if the edge pixels of the rectangle continued outside it. A tiling surface wraps around instead. There is no supersampling when a Warpmap shrinks a surface, use a mipmapped surface for that.

All methods take a *source offset* (in spx of the source) that is added to the position before reading, so a ring can be turned or a pattern scrolled without changing the Warpmap.



## Periods

An axis can be periodic (*uPeriod* or *vPeriod* above 0), as when u is the angle of a ring. Positions are then read modulo the period and interpolation between nodes across the seam takes the short way round.

With a period of 1.0 the source wraps around, which for an edgemap or surface that is tiling is seamless. A period larger than 1.0 leaves a gap between the copies of the source, as a 270 degree arc has uPeriod = 360 / 270. A pixel in the gap takes its colors from the nearest copy, so they don't jump where the copies meet.



## CircularWarp

CircularWarp makes and keeps a Warpmap that bends the source around a circle: its width along the circle, from *startAngle* and *sweep* degrees on, its height from *topRadius* to *bottomRadius*.

```c++
	auto pGauge = CircularWarp::create( WGBP(CircularWarp,
										_.size = { 200, 200 },
										_.topRadius = 98*64,
										_.bottomRadius = 70*64,
										_.startAngle = -135.f,
										_.sweep = 270.f ), pDevice->warpmapFactory() );

	pDevice->warpDrawEdgemap( { 10*64, 10*64 }, pGauge->refresh(), pEdgemap );
```

refresh() updates the Warpmap after settings have changed and returns it. The cell size decides how closely the Warpmap follows the circle and is calculated from *precision* (largest position error in pixels) unless set. A radius of 0 gives a disc, which only works for content that doesn't change from left to right close to the center.



## Making your own

```c++
	auto pWarpmap = pDevice->warpmapFactory()->createWarpmap( WGBP(Warpmap, _.size = { 160, 120 }, _.cellSize = 8) );

	SizeI nodes = pWarpmap->nodes();
	std::vector<CoordF> positions( nodes.w * nodes.h );

	for( int row = 0 ; row < nodes.h ; row++ )
		for( int col = 0 ; col < nodes.w ; col++ )
		{
			float u = pWarpmap->nodeX(col) / 160.f;
			float v = pWarpmap->nodeY(row) / 120.f;
			positions[row*nodes.w + col] = { u, v + 0.05f * std::sin(u * 12.f) };		// A wave.
		}

	pWarpmap->importNodes( { 0, 0, nodes.w, nodes.h }, positions.data() );
```

A new Warpmap maps its rectangle straight onto the source. Smaller cells follow curved shapes more closely, a circle of radius r is off by roughly cellSize² / (8 * r) pixels.



## How it is drawn

The exact rules all backends follow are given in the documentation of the Warpmap class. In short: the part of a pixel covered is the part of its area whose position is inside the source (or, for an edgemap, inside each segment), which is then blended as any anti-aliased edge, see [Blending and transparency](../basics/blending_and_transparency). Colors are taken at the position of the center of the pixel.

ReferenceBackend measures coverage by sampling each pixel at many points. The other backends estimate it from the position at the center of the pixel and how fast it changes, taking each border and edge as straight over the pixel. That is off by a few percent where an edge bends sharply within a pixel.
