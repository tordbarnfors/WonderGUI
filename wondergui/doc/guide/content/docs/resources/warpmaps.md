---
title: 'Warpmaps'
weight: 35
---

# Warpmaps

A Warpmap bends a surface or an edgemap into another shape when it is drawn: a gauge or a progress ring from a straight bar, text along an arc, a waving flag. The source is drawn as it is and the Warpmap decides where each part of it ends up.

A Warpmap covers a rectangle of pixels with a grid of nodes, one node every *cellSize* pixels in both directions plus one at the right and bottom edge. Each node holds the position in the source that is drawn at that point, in spx of the source (64 per pixel of the edgemap or surface, relative to the source rectangle of a blit): u along its width and v along its height. Positions between nodes are interpolated. Where a position falls outside the source nothing is drawn, so the shape of what is drawn is given by the nodes, with anti-aliased borders.

A Warpmap is made for a source of a certain size. It isn't stretched to fit a source of another size, that takes other nodes.

Warpmaps are *semi-mutable*: size and cell size are set when created, nodes and periods can be changed. Each graphics backend has its own Warpmap subtype, created by its WarpmapFactory (`GfxDevice::warpmapFactory()`), or through a WarpGenerator such as CircularWarp.



## Drawing

```c++
	pDevice->warpDrawEdgemap( dest, pWarpmap, pEdgemap );

	pDevice->setBlitSource( pSurface );
	pDevice->warpBlit( dest, pWarpmap );						// The whole surface.
	pDevice->warpBlit( dest, pWarpmap, sourceRect );			// A rectangle of it.
```

The Warpmap is placed with its top-left corner at *dest*, rounded to whole pixels, and nothing is drawn outside its rectangle. Blend mode, tint color and tint apply as for any other drawing, the tint in canvas space. Surfaces are read with their sample method. Nothing outside the source rectangle is read, as with stretchBlit(): along its borders the surface is read as if the edge pixels of the rectangle continued outside it. A tiling surface wraps around instead. There is no supersampling when a Warpmap shrinks a surface, use a mipmapped surface for that.

Draws are rendered when the canvas update ends and read the Warpmap then. Changing the nodes or periods of a Warpmap changes all of its draws that aren't rendered yet, so use a Warpmap for each shape drawn within the same canvas update.

All methods take a *source offset* that is added to the position before reading, so a ring can be turned or a pattern scrolled without changing the Warpmap. It is in spx of the source, like the nodes.



## Periods

An axis can be periodic (*uPeriod* or *vPeriod* above 0, in spx of the source), as when u is the angle of a ring. Positions are then read modulo the period and interpolation between nodes across the seam takes the short way round.

With a period of the size of the source it wraps around, which for an edgemap or surface that is tiling is seamless. A longer period leaves a gap between the copies of the source, as a 270 degree arc of a source *w* spx wide has uPeriod = w * 360 / 270. A pixel in the gap takes its colors from the nearest copy, so they don't jump where the copies meet.



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

The cell size decides how closely the Warpmap follows the circle and is calculated from *precision* (largest position error in pixels) at the smaller radius unless set. For a disc, precision is kept from half its radius and outwards. A radius of 0 gives a disc, which only works for content that doesn't change from left to right close to the center.




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

	pDevice->warpDrawEdgemap( { 10*64, 10*64 }, pFocusRing->generate(), pEdgemap, { offset, 0 } );		// Scroll offset for marching ants.
```

Positions along the outline are those of the nearest point on it. The source keeps its width along the straight sides and is spread or squeezed around the corners, more the further from the outline, so placing the outline in the middle of the band gives the most even result. Corners stay round on the outside. On the inside they become mitered like a picture frame where the band goes deeper than *cornerRadius*, and since a Warpmap interpolates across the miter, that takes small cells to look clean. *cornerRadius* is limited to half the shorter side of *rect*, so a square with corners of half its side is a circle.



## WarpGenerators

CircularWarp and RoundedRectWarp are WarpGenerators: they make a Warpmap from a few settings and keep it. Size, source size, cell size, precision and generate() are the same for all WarpGenerators.

generate() updates the Warpmap after settings have changed and returns it. A Warpmap that is held elsewhere, such as by a draw that isn't rendered yet, is never changed: generate() then makes a new one, as it does when size or cell size has changed. So draw what generate() returns rather than keeping it, and one generator can be drawn several times with different settings within the same canvas update. Unless set, the cell size is calculated from *precision* and the shape, within 4 to 64 pixels.

To make your own, subclass WarpGenerator and implement _generateNodes(), which fills in the positions of all nodes, and _calcCellSize(), which picks a cell size from *precision*. _cellSizeForRadius() gives the cell size for a curve of a given radius. Override _uPeriod() and _vPeriod() if an axis is periodic, and set *m_bDirty* when one of your settings changes.



## Making your own

```c++
	auto pWarpmap = pDevice->warpmapFactory()->createWarpmap( WGBP(Warpmap, _.size = { 160, 120 }, _.cellSize = 8) );

	SizeI nodes = pWarpmap->nodes();
	std::vector<CoordSPX> positions( nodes.w * nodes.h );

	for( int row = 0 ; row < nodes.h ; row++ )
		for( int col = 0 ; col < nodes.w ; col++ )
		{
			int x = pWarpmap->nodeX(col);
			int y = pWarpmap->nodeY(row);
			positions[row*nodes.w + col] = { x * 64, y * 64 + spx(std::lround(6 * 64 * std::sin(x * 0.1))) };		// A wave, 6 pixels high.
		}

	pWarpmap->importNodes( { 0, 0, nodes.w, nodes.h }, positions.data() );
```

A new Warpmap maps its rectangle straight onto the source, pixel for pixel. Smaller cells follow curved shapes more closely, a circle of radius r is off by roughly cellSize² / (8 * r) pixels.



## How it is drawn

The exact rules all backends follow are given in the documentation of the Warpmap class. In short: the part of a pixel covered is the part of its area whose position is inside the source (or, for an edgemap, inside each segment), which is then blended as any anti-aliased edge, see [Blending and transparency](../basics/blending_and_transparency). Colors are taken at the position of the center of the pixel.

ReferenceBackend measures coverage by sampling each pixel at many points. The other backends estimate it from the position at the center of the pixel and how fast it changes, taking each border and edge as straight over the pixel. That is off by a few percent where an edge bends sharply within a pixel.
