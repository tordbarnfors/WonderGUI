/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/

#include <wg_referencebackend.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softwarpmapfactory.h>
#include <wg_softwarpmap.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>

namespace wg
{
	const TypeInfo ReferenceBackend::TYPEINFO = { "ReferenceBackend", &GfxBackend::TYPEINFO };

	// Small helpers for the math.

	static double _clamp01(double value) { return std::clamp(value, 0.0, 1.0); }

	static double _mix(double from, double to, double fraction) { return from + (to - from) * fraction; }

	// Length of the overlap between the ranges begin1 -> end1 and begin2 -> end2.

	static double _overlap(double begin1, double end1, double begin2, double end2)
	{
		return std::max(0.0, std::min(end1, end2) - std::max(begin1, begin2));
	}

	// Moves a position onto the closest pixel boundary if it is within tolerance from it.

	static double _snapToPixelBoundary(double position, double tolerance)
	{
		double boundary = std::round(position);
		return std::abs(position - boundary) <= tolerance ? boundary : position;
	}

	// Modulo that is positive also for negative values, for wrapping around tiles.

	static int _wrap(int value, int size)
	{
		int result = value % size;
		return result < 0 ? result + size : result;
	}

	// Reads a 32-bit value from the command stream, where it is stored as two 16-bit words.

	static int32_t _read32(const uint16_t*& p)
	{
		int32_t value;
		std::memcpy(&value, p, sizeof(value));
		p += 2;
		return value;
	}


	//____ create() _____________________________________________

	ReferenceBackend_p ReferenceBackend::create()
	{
		return ReferenceBackend_p(new ReferenceBackend());
	}

	//____ constructor _____________________________________________

	ReferenceBackend::ReferenceBackend()
	{
		_resetStates();
	}

	//____ destructor _____________________________________________

	ReferenceBackend::~ReferenceBackend()
	{
		if (m_pBlitSource)
			m_pBlitSource->freePixelBuffer(m_blitSourceBuffer);

		if (m_pCanvas)
			m_pCanvas->freePixelBuffer(m_canvasBuffer);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ReferenceBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _____________________________________________

	void ReferenceBackend::beginRender()
	{
	}

	//____ endRender() _____________________________________________

	void ReferenceBackend::endRender()
	{
		setCanvas((Surface*) nullptr);
	}

	//____ beginSession() _______________________________________________________

	void ReferenceBackend::beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo )
	{
		// We don't need the update rects. Everything we draw is within them anyway.

		if( canvasRef != CanvasRef::None )
			setCanvas(canvasRef);
		else
			setCanvas(pCanvas);
	}

	//____ endSession() _______________________________________________________

	void ReferenceBackend::endSession()
	{
	}

	//____ setCanvas() _____________________________________________

	void ReferenceBackend::setCanvas(CanvasRef ref)
	{
		auto pInfo = canvasInfo(ref);

		if (!pInfo)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Canvas reference has not been defined.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		setCanvas(pInfo->pSurface);
	}

	void ReferenceBackend::setCanvas(Surface* pSurface)
	{
		// Give the pixels of the previous canvas back.

		if (m_pCanvas)
		{
			m_pCanvas->pullPixels(m_canvasBuffer, false);
			m_pCanvas->freePixelBuffer(m_canvasBuffer);
			m_pCanvas = nullptr;
		}

		// Start using the new canvas, if we can.

		if (pSurface)
		{
			PixelFormat format = pSurface->pixelFormat();

			if (format != PixelFormat::ARGB_8 && format != PixelFormat::XRGB_8 && format != PixelFormat::RGB_565 &&
				format != PixelFormat::BGR_565 && format != PixelFormat::Alpha_8)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "ReferenceBackend can't render to surfaces of this pixel format.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}
			else
			{
				m_pCanvas = pSurface;
				m_canvasBuffer = pSurface->allocPixelBuffer();
				m_bAlphaOnlyCanvas = (format == PixelFormat::Alpha_8);
				m_bCanvasHasAlpha = (format == PixelFormat::ARGB_8 || format == PixelFormat::Alpha_8);
			}
		}

		// All states are reset when a canvas is set.

		_resetStates();
	}

	//____ _resetStates() ________________________________________________________

	void ReferenceBackend::_resetStates()
	{
		if (m_pBlitSource)
		{
			m_pBlitSource->freePixelBuffer(m_blitSourceBuffer);
			m_pBlitSource = nullptr;
		}

		m_blendMode = BlendMode::Blend;
		m_tintColor = { 1, 1, 1, 1 };
		m_tint.nLayers = 0;
		m_morphFactor = 0.5;

		m_blurRadius = s_defaultBlurRadius / 64.0;
		for (int color = 0; color < 3; color++)
			for (int i = 0; i < 9; i++)
				m_blurWeights[color][i] = s_defaultBlur[i] / 65536.0;
	}

	//____ setObjects() _____________________________________________

	void ReferenceBackend::setObjects(Object* const * pBeg, Object* const * pEnd)
	{
		m_pObjectsBeg = pBeg;
	}

	//____ setRects() _____________________________________________

	void ReferenceBackend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
		m_pRectsPtr = pBeg;
	}

	//____ setColors() _____________________________________________

	void ReferenceBackend::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
		m_pColorsPtr = pBeg;
	}

	//____ setTransforms() _____________________________________________

	void ReferenceBackend::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
		m_pTransformsBeg = pBeg;
	}

	//____ processCommands() _____________________________________________
	/*
		Reads the commands and their parameters. Rects, colors and objects are read from their
		buffers as the commands need them, in the order the commands come.
	*/

	void ReferenceBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version)
	{
		// Transforms after the standard ones are custom transforms from the transform buffer.

		int customTransformStart = (version == 1) ? GfxFlip_size : NbStandardTransforms;

		auto p = pBeg;
		while (p < pEnd)
		{
			auto cmd = Command(*p++);
			switch (cmd)
			{
				case Command::None:
					break;

				case Command::StateChange:
					p = _changeStates(p);
					break;

				case Command::Fill:
				{
					int nRects = *p++;
					Color color = _toLinear(*m_pColorsPtr++);

					for (int i = 0; i < nRects; i++)
						_fill(*m_pRectsPtr++, color);
					break;
				}

				case Command::Line:
				{
					int nClipRects = *p++;
					int nLines = *p++;
					p++;										// padding

					const RectSPX* pClipRects = m_pRectsPtr;
					m_pRectsPtr += nClipRects;

					for (int i = 0; i < nLines; i++)
					{
						Color color = _toLinear(*m_pColorsPtr++);

						CoordSPX begin, end;
						begin.x = _read32(p);
						begin.y = _read32(p);
						end.x = _read32(p);
						end.y = _read32(p);

						spx thickness = *p++;
						uint16_t caps = *p++;

						if (caps == 0)
							_drawLine(begin, end, thickness, color, nClipRects, pClipRects);
						else
							_drawCappedLine(begin, end, thickness, decodeLineCaps(caps), color, nClipRects, pClipRects);
					}
					break;
				}

				case Command::Blit:
				case Command::ClipBlit:
				case Command::Tile:
				case Command::Blur:
				{
					int nRects = *p++;

					for (int i = 0; i < nRects; i++)
					{
						CoordI source;							// In 1/1024 pixels.
						source.x = _read32(p);
						source.y = _read32(p);

						CoordSPX dest;
						dest.x = _read32(p);
						dest.y = _read32(p);

						int transform = *p++;
						p++;									// padding

						bool bCustomTransform = (transform >= customTransformStart);
						const Transform& mtx = bCustomTransform ? m_pTransformsBeg[transform - customTransformStart] : s_standardTransforms[transform];

						_blit(cmd, source, dest, mtx, bCustomTransform, *m_pRectsPtr++);
					}
					break;
				}

				case Command::DrawShape:
				{
					auto kind = ShapeKind(p[0]);
					int nRects = p[1];
					int nColors = p[2];
					int nWords = p[3];
					const uint16_t* pPayload = p + c_drawShapeHeaderWords - 1;

					_drawShape(kind, pPayload, nRects);

					p = pPayload + nWords;
					m_pRectsPtr += nRects;
					m_pColorsPtr += nColors;
					break;
				}

				case Command::WarpBlit:
				case Command::WarpDrawEdgemap:
				{
					WarpParams params;
					p = decodeWarp(cmd, p, params);

					auto pWarpmap = static_cast<SoftWarpmap*>(m_pObjectsBeg[params.warpmap]);
					auto pEdgemap = (cmd == Command::WarpDrawEdgemap) ? static_cast<SoftEdgemap*>(m_pObjectsBeg[params.edgemap]) : nullptr;

					if (pEdgemap)
						_decodeSegmentTints(pEdgemap);

					for (int i = 0; i < params.nRects; i++)
						_warp(params, pWarpmap, pEdgemap, *m_pRectsPtr++);
					break;
				}

				case Command::DrawEdgemap:
				case Command::TileEdgemap:
				{
					int nRects = *p++;
					GfxFlip flip = GfxFlip(*p++);
					p++;										// padding

					auto pEdgemap = static_cast<SoftEdgemap*>(m_pObjectsBeg[readObjectIndex(p)]);

					CoordSPX origo;
					origo.x = _read32(p);
					origo.y = _read32(p);

					for (int i = 0; i < nRects; i++)
						_drawEdgemap(pEdgemap, cmd == Command::TileEdgemap, flip, origo, *m_pRectsPtr++);
					break;
				}

				default:
					break;
			}
		}
	}

	//____ _changeStates() ________________________________________________________
	/*
		Reads a StateChange command. Each state that has changed has its parameters in the stream
		(or its color in the color buffer), in the order of the StateChange bits.
	*/

	const uint16_t* ReferenceBackend::_changeStates(const uint16_t* p)
	{
		int statesChanged = *p++;

		if (statesChanged & int(StateChange::BlitSource))
		{
			if (m_pBlitSource)
				m_pBlitSource->freePixelBuffer(m_blitSourceBuffer);

			m_pBlitSource = static_cast<Surface*>(m_pObjectsBeg[readObjectIndex(p)]);

			if (m_pBlitSource)
				m_blitSourceBuffer = m_pBlitSource->allocPixelBuffer();
		}

		if (statesChanged & int(StateChange::TintColor))
			m_tintColor = _toLinear(*m_pColorsPtr++);

		if (statesChanged & int(StateChange::Tint))
			p = TintTools::decodeTint(p, m_pColorsPtr, m_tint);

		if (statesChanged & int(StateChange::BlendMode))
			m_blendMode = BlendMode(*p++);

		if (statesChanged & int(StateChange::MorphFactor))
			m_morphFactor = *p++ / 4096.0;

		if (statesChanged & int(StateChange::FixedBlendColor))
			m_pColorsPtr++;						// Not needed, BlendFixedColor is drawn as Blend.

		if (statesChanged & int(StateChange::Blur))
		{
			// Radius in spx, followed by nine weights each for red, green and blue, where 32768 is 1.0.

			m_blurRadius = *p++ / 64.0;

			for (int color = 0; color < 3; color++)
				for (int i = 0; i < 9; i++)
					m_blurWeights[color][i] = *p++ / 32768.0;
		}

		// The next command starts on a 32-bit boundary.

		if ((uintptr_t(p) & 0x2) == 2)
			p++;

		return p;
	}

	//____ _fill() ________________________________________________________________
	/*
		Fills a rectangle, which can have edges anywhere within pixels. Each pixel is covered
		by the overlap of the pixel and the rectangle.
	*/

	void ReferenceBackend::_fill(const RectSPX& rect, const Color& color)
	{
		double left = rect.x / 64.0;
		double top = rect.y / 64.0;
		double right = (rect.x + rect.w) / 64.0;
		double bottom = (rect.y + rect.h) / 64.0;

		for (int y = int(std::floor(top)); y < int(std::ceil(bottom)); y++)
		{
			for (int x = int(std::floor(left)); x < int(std::ceil(right)); x++)
			{
				double coverage = _overlap(x, x + 1, left, right) * _overlap(y, y + 1, top, bottom);

				_plotPixel(x, y, color * _deviceTint(x, y), coverage);
			}
		}
	}

	//____ _drawLine() ____________________________________________________________
	/*
		Draws a straight line without caps (LineCap::None at both ends), as defined by the
		GfxBackend contract:

		Begin and end are rounded to whole pixels. A line that mainly runs along the x-axis is
		drawn as a column of color for every pixel from begin.x up to (but not including) end.x.
		Each column is centered on the line through the centers of the begin and end pixels and is
		thick enough to make the line 'thickness' thick, measured at right angle to the line. Lines
		that mainly run along the y-axis are drawn the same way with x and y swapped. A line from a
		pixel to itself has no length and draws nothing.

		Each pixel is covered by the overlap of the pixel and the column of color.

		Lines are tinted like everything else, by the tint color and the Tint at the center of
		each pixel.
	*/

	void ReferenceBackend::_drawLine(CoordSPX _begin, CoordSPX _end, spx thickness, const Color& color, int nClipRects, const RectSPX* pClipRects)
	{
		CoordI begin = Util::roundToPixels(_begin);
		CoordI end = Util::roundToPixels(_end);

		// We draw along the 'major' axis, which is the axis the line mainly runs along,
		// placing a column of color across the 'minor' axis at each pixel.

		bool bAlongX = std::abs(end.x - begin.x) > std::abs(end.y - begin.y);

		int majorBegin = bAlongX ? begin.x : begin.y;
		int majorEnd = bAlongX ? end.x : end.y;
		int minorBegin = bAlongX ? begin.y : begin.x;
		int minorEnd = bAlongX ? end.y : end.x;

		if (majorBegin > majorEnd)
		{
			std::swap(majorBegin, majorEnd);
			std::swap(minorBegin, minorEnd);
		}

		int length = majorEnd - majorBegin;
		if (length == 0)
			return;

		double slope = double(minorEnd - minorBegin) / length;
		double columnThickness = (thickness / 64.0) * std::sqrt(1.0 + slope * slope);

		for (int major = majorBegin; major < majorEnd; major++)
		{
			double center = minorBegin + 0.5 + slope * (major - majorBegin);
			double columnBegin = center - columnThickness / 2;
			double columnEnd = center + columnThickness / 2;

			for (int minor = int(std::floor(columnBegin)); minor < int(std::ceil(columnEnd)); minor++)
			{
				int x = bAlongX ? major : minor;
				int y = bAlongX ? minor : major;

				// Only pixels within the clip rectangles are drawn.

				bool bInside = false;
				for (int i = 0; i < nClipRects && !bInside; i++)
					bInside = _pixelRect(pClipRects[i]).contains(CoordI(x, y));

				if (bInside)
					_plotPixel(x, y, color * _deviceTint(x, y), _overlap(minor, minor + 1, columnBegin, columnEnd));
			}
		}
	}

	//____ _blit() ________________________________________________________________
	/*
		Blits the part of a Blit, ClipBlit, Tile or Blur that is within rect. Rect and dest are
		always whole pixels.

		Each destination pixel reads the source at a position given by the transform, which tells
		how far we move over the source for each pixel we move along x and y in the destination.
		The GfxBackend contract defines the position differently for the standard and the custom
		transforms:

		Standard transforms (blits 1:1 with flips and rotations): source is the position of the
		corner of the source rectangle that ends up at dest. Every pixel reads the source at the
		center of the pixel, transformed to the source.

		Custom transforms (scaling and arbitrary transforms): source is where the first destination
		pixel samples the source. Nearest sampling reads the pixel the position is in, while
		Bilinear sampling has the centers of the source pixels on whole positions.

		Blit reads within the source, ClipBlit reads transparent black outside the source, Tile
		repeats the source and Blur blends nine samples around the position.
	*/

	void ReferenceBackend::_blit(Command cmd, CoordI source, CoordSPX dest, const Transform& mtx, bool bCustomTransform, const RectSPX& rect)
	{
		if (!m_pBlitSource)
			return;

		bool bBilinear = (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear);

		EdgeMode edgeMode = (cmd == Command::Tile) ? EdgeMode::Tile : (cmd == Command::ClipBlit) ? EdgeMode::Clip : EdgeMode::Clamp;

		CoordI destPixel = Util::roundToPixels(dest);
		RectI pixels = _pixelRect(rect);

		for (int y = pixels.y; y < pixels.y + pixels.h; y++)
		{
			for (int x = pixels.x; x < pixels.x + pixels.w; x++)
			{
				// Our offset from dest, at the corner or the center of the pixel.

				double dx = x - destPixel.x;
				double dy = y - destPixel.y;

				if (!bCustomTransform)
				{
					dx += 0.5;
					dy += 0.5;
				}

				// Position in the source, where pixel (x,y) covers x -> x+1 and y -> y+1.

				double srcX = source.x / 1024.0 + dx * mtx.xx + dy * mtx.yx;
				double srcY = source.y / 1024.0 + dx * mtx.xy + dy * mtx.yy;

				if (bCustomTransform)
				{
					// Custom transforms are floats, which can't describe steps like 1/3 exactly, so a position
					// that should end up exactly on a pixel boundary might end up a tiny bit before it. Positions
					// that are within the rounding error of the transform from a pixel boundary are moved onto it.

					srcX = _snapToPixelBoundary(srcX, (std::abs(dx * mtx.xx) + std::abs(dy * mtx.yx)) * FLT_EPSILON);
					srcY = _snapToPixelBoundary(srcY, (std::abs(dx * mtx.xy) + std::abs(dy * mtx.yy)) * FLT_EPSILON);

					if (bBilinear)
					{
						srcX += 0.5;
						srcY += 0.5;
					}
				}

				Color texel = (cmd == Command::Blur) ? _sampleBlurred(srcX, srcY) : _sampleSource(srcX, srcY, edgeMode);

				_plotPixel(x, y, texel * _deviceTint(x, y), 1.0);
			}
		}
	}

	//____ _drawEdgemap() _________________________________________________________
	/*
		Draws the part of an edgemap that is within rect, flipped/rotated by flip, with the
		top-left corner of the flipped edgemap at origo. When tiling, the edgemap is repeated
		over all of rect, with one of the copies at origo.

		The GfxBackend contract places edgemaps on whole pixels, so origo is rounded to nearest
		pixel. Each pixel of the canvas therefore covers exactly one pixel of the edgemap.
	*/

	void ReferenceBackend::_drawEdgemap(SoftEdgemap* pEdgemap, bool bTile, GfxFlip flip, CoordSPX _origo, const RectSPX& rect)
	{
		if (m_blendMode == BlendMode::Ignore)
			return;

		SizeI mapSize = pEdgemap->pixelSize();
		CoordI origo = Util::roundToPixels(_origo);

		// The standard transforms tell how we move over the edgemap when we move over the canvas,
		// just like for blits. The corner of the edgemap we start from is the one that makes us
		// move into the edgemap.

		const Transform& mtx = s_standardTransforms[int(flip)];

		double cornerX = (mtx.xx < 0 || mtx.yx < 0) ? mapSize.w : 0;
		double cornerY = (mtx.xy < 0 || mtx.yy < 0) ? mapSize.h : 0;

		_decodeSegmentTints(pEdgemap);

		// Draw the pixels

		RectI pixels = _pixelRect(rect);

		for (int y = pixels.y; y < pixels.y + pixels.h; y++)
		{
			for (int x = pixels.x; x < pixels.x + pixels.w; x++)
			{
				// Center of the canvas pixel, transformed to the edgemap.

				double dx = x - origo.x + 0.5;
				double dy = y - origo.y + 0.5;

				double mapX = cornerX + dx * mtx.xx + dy * mtx.yx;
				double mapY = cornerY + dx * mtx.xy + dy * mtx.yy;

				int column = int(std::floor(mapX));
				int row = int(std::floor(mapY));

				if (bTile)
				{
					column = _wrap(column, mapSize.w);
					row = _wrap(row, mapSize.h);
				}
				else if (column < 0 || row < 0 || column >= mapSize.w || row >= mapSize.h)
					continue;

				_drawEdgemapPixel(pEdgemap, column, row, { x, y });
			}
		}
	}

	//____ _decodeSegmentTints() __________________________________________________
	/*
		Decodes the tints of the segments of an edgemap, placed over the edgemap.
	*/

	void ReferenceBackend::_decodeSegmentTints(SoftEdgemap* pEdgemap)
	{
		SizeI mapSize = pEdgemap->pixelSize();
		int nSegments = pEdgemap->renderSegments();

		m_segmentTints.resize(nSegments);
		for (int seg = 0; seg < nSegments; seg++)
		{
			Tint* pTint = pEdgemap->tints()[seg].rawPtr();
			m_segmentTints[seg].nLayers = 0;

			if (pTint)
			{
				uint16_t	words[TintTools::c_maxEncodedTintWords];
				HiColor		colors[TintTools::c_maxEncodedTintColors];
				int			nColors;

				TintTools::encodeTint(pTint, RectSPX(0, 0, mapSize.w * 64, mapSize.h * 64), words, colors, nColors);

				const HiColor* pColors = colors;
				TintTools::decodeTint(words, pColors, m_segmentTints[seg]);
			}
		}
	}

	//____ _drawEdgemapPixel() ____________________________________________________
	/*
		Draws the pixel at column, row of the edgemap onto the canvas pixel. The pixel gets the
		average of what each segment would give the whole pixel, weighted by how much of the pixel
		the segment covers and averaged as premultiplied colors.
	*/

	void ReferenceBackend::_drawEdgemapPixel(SoftEdgemap* pEdgemap, int column, int row, CoordI canvasPixel)
	{
		int nSegments = pEdgemap->renderSegments();
		int nEdges = nSegments - 1;

		// Samples are stored column by column, with the samples of all edges of a column together.
		// There is always one more column of samples than columns of pixels.

		int samplePitch = pEdgemap->m_nbSegments - 1;
		const spx* pLeftSamples = pEdgemap->m_pSamples + column * samplePitch;
		const spx* pRightSamples = pLeftSamples + samplePitch;

		EdgeLine edges[Edgemap::maxSegments];

		for (int edge = 0; edge < nEdges; edge++)
			edges[edge] = { pLeftSamples[edge] / 64.0, pRightSamples[edge] / 64.0 };

		double coverage[Edgemap::maxSegments];
		_segmentCoverage(edges, nEdges, row, coverage);

		//

		if (!_isInsideCanvas(canvasPixel.x, canvasPixel.y))
			return;

		Color dst = _canvasPixel(canvasPixel.x, canvasPixel.y);
		Color colors[Edgemap::maxSegments];

		for (int seg = 0; seg < nSegments; seg++)
			colors[seg] = _segmentColor(pEdgemap, seg, column, row) * _deviceTint(canvasPixel.x, canvasPixel.y);

		// Effect modes apply the segments together, see _effectSource(). Paint modes average what each
		// segment gives its part of the pixel.

		if (_isEffectMode())
		{
			_setCanvasPixel(canvasPixel.x, canvasPixel.y, _blend(_effectSource(nSegments, colors, coverage), dst));
			return;
		}

		Color results[Edgemap::maxSegments];

		for (int seg = 0; seg < nSegments; seg++)
			results[seg] = _blend(colors[seg], dst);

		_setCanvasPixel(canvasPixel.x, canvasPixel.y, _averageColors(nSegments, results, coverage, true));
	}

	//____ _segmentCoverage() _____________________________________________________
	/*
		Calculates how much of the pixel at row in a column of an edgemap that each segment covers.
		Segment 0 is above edge 0, segment n is between edge n-1 and edge n and the last segment is
		below the last edge.

		Edges are not supposed to cross, but if one goes above an edge before it, it is pushed down
		to that edge. So the top of a segment is the lowest of all the edges above it.
	*/

	void ReferenceBackend::_segmentCoverage(const EdgeLine* pEdges, int nEdges, int row, double* pCoverage)
	{
		double areaBelowTop = 1.0;					// No edges above the first segment, all of the pixel is below its top.

		for (int seg = 0; seg <= nEdges; seg++)
		{
			double areaBelowBottom = (seg < nEdges) ? _areaBelow(pEdges, seg + 1, row) : 0.0;

			pCoverage[seg] = std::max(0.0, areaBelowTop - areaBelowBottom);
			areaBelowTop = areaBelowBottom;
		}
	}

	//____ _areaBelow() ___________________________________________________________
	/*
		Area of the pixel at row (0.0 -> 1.0) that is below the lowest of the given edges.

		At position x across the column, the height of the pixel below the edges is:

			height(x) = clamp( row + 1 - lowest edge at x, 0.0, 1.0 )

		Height is linear except where two edges cross (the lowest one changes) and where the lowest
		edge enters or leaves the pixel (the clamp starts or stops). We find all those points and
		integrate exactly between them, where height is linear.
	*/

	double ReferenceBackend::_areaBelow(const EdgeLine* pEdges, int nEdges, int row)
	{
		auto height = [&](double x)
		{
			double lowest = -std::numeric_limits<double>::infinity();
			for (int i = 0; i < nEdges; i++)
				lowest = std::max(lowest, pEdges[i].at(x));

			return _clamp01(row + 1 - lowest);
		};

		// Points where height might stop being linear.

		std::vector<double> points = { 0.0, 1.0 };

		auto addPoint = [&points](double x)
		{
			if (x > 0.0 && x < 1.0)
				points.push_back(x);
		};

		for (int i = 0; i < nEdges; i++)
		{
			const EdgeLine& edge = pEdges[i];
			double rise = edge.y1 - edge.y0;

			// Where the edge passes the top and bottom of the pixel.

			if (rise != 0.0)
			{
				addPoint((row - edge.y0) / rise);
				addPoint((row + 1 - edge.y0) / rise);
			}

			// Where the edge crosses the edges after it.

			for (int j = i + 1; j < nEdges; j++)
			{
				const EdgeLine& other = pEdges[j];
				double riseDiff = rise - (other.y1 - other.y0);

				if (riseDiff != 0.0)
					addPoint((other.y0 - edge.y0) / riseDiff);
			}
		}

		std::sort(points.begin(), points.end());

		// Integrate, height is linear between the points.

		double area = 0.0;

		for (size_t i = 1; i < points.size(); i++)
		{
			double x0 = points[i - 1];
			double x1 = points[i];

			area += (height(x0) + height(x1)) / 2 * (x1 - x0);
		}

		return area;
	}

	//____ _warp() _________________________________________________________________
	/*
		Draws the part of a WarpBlit (pEdgemap is null) or WarpDrawEdgemap that is within rect,
		following the definition in Warpmap:

		The Warpmap is placed with its top-left corner at dest, rounded to nearest pixel. The
		position in the source of a point of the Warpmap is interpolated from the corners of its
		cell (Warpmap::positionAt()), offset by the source offset, scaled to pixels of the source
		and wrapped on periodic axes.

		The part of a pixel that is covered (by the source or by each segment of the edgemap) is
		measured by sampling the pixel at the points of a Fibonacci lattice, which are spread evenly
		and all have different x and different y, so that edges in any direction, also straight
		along x or y, are measured to within a sample. Pixels are first sampled at c_warpQuickSamples
		points, and again at c_warpSamples if not all in the same segment or all outside.

		Colors are taken at the center of the pixel: the blit source is sampled there by its sample
		method, as if the edge pixels of the source rectangle continued outside it (or wrapping around
		the surface if it is tiling), and the tints of edgemap segments are evaluated there. A center
		in the gap between copies on a periodic axis is taken in the nearest copy.
	*/

	static const int c_warpQuickSamples = 233;			// Fibonacci numbers, lattice steps are the ones before them.
	static const int c_warpQuickStep = 144;
	static const int c_warpSamples = 4181;
	static const int c_warpStep = 2584;

	void ReferenceBackend::_warp(const WarpParams& params, SoftWarpmap* pWarpmap, SoftEdgemap* pEdgemap, const RectSPX& rect)
	{
		if (m_blendMode == BlendMode::Ignore)
			return;

		if (!pEdgemap && !m_pBlitSource)
			return;

		CoordI dest = Util::roundToPixels(params.dest);
		SizeI warpSize = pWarpmap->pixelSize();

		// Source rectangle, in pixels of the source.

		double srcX = 0, srcY = 0, srcW, srcH;

		if (pEdgemap)
		{
			srcW = pEdgemap->pixelSize().w;
			srcH = pEdgemap->pixelSize().h;
		}
		else
		{
			srcX = params.source.x / 64.0;
			srcY = params.source.y / 64.0;
			srcW = params.source.w / 64.0;
			srcH = params.source.h / 64.0;
		}

		double offsetX = params.sourceOffset.x / 64.0;
		double offsetY = params.sourceOffset.y / 64.0;

		double periodX = pWarpmap->uPeriod() * srcW;
		double periodY = pWarpmap->vPeriod() * srcH;

		const int cellSize = pWarpmap->cellSize();
		const SizeI nodes = pWarpmap->nodes();

		// Position in the source (relative to the source rectangle) at point x,y of the Warpmap,
		// which is in the cell with the given corners (as Warpmap::positionAt()). Pixels are
		// always within one cell.

		struct Cell
		{
			double	x0, y0, w, h;
			CoordF	corners[4];
		};

		auto cellOf = [&](int x, int y)
		{
			Cell cell;
			int column = std::clamp(x / cellSize, 0, nodes.w - 2);
			int row = std::clamp(y / cellSize, 0, nodes.h - 2);

			pWarpmap->cellCorners(column, row, cell.corners);
			cell.x0 = pWarpmap->nodeX(column);
			cell.y0 = pWarpmap->nodeY(row);
			cell.w = pWarpmap->nodeX(column + 1) - cell.x0;
			cell.h = pWarpmap->nodeY(row + 1) - cell.y0;
			return cell;
		};

		auto sourcePos = [&](const Cell& cell, double x, double y, double& sx, double& sy)
		{
			const CoordF* c = cell.corners;

			double fx = (x - cell.x0) / cell.w;
			double fy = (y - cell.y0) / cell.h;

			double u = (c[0].x * (1 - fx) + c[1].x * fx) * (1 - fy) + (c[2].x * (1 - fx) + c[3].x * fx) * fy;
			double v = (c[0].y * (1 - fx) + c[1].y * fx) * (1 - fy) + (c[2].y * (1 - fx) + c[3].y * fx) * fy;

			sx = u * srcW + offsetX;
			sy = v * srcH + offsetY;

			if (periodX > 0)
				sx -= periodX * std::floor(sx / periodX);
			if (periodY > 0)
				sy -= periodY * std::floor(sy / periodY);
		};

		// Position of a center in the gap between copies moved to the nearest copy, for colors.

		auto nearestCopy = [](double& s, double size, double period)
		{
			if (period > 0 && 2 * s > std::min(size, period) + period)
				s -= period;
		};

		auto isInside = [&](double sx, double sy)
		{
			return sx >= 0 && sy >= 0 && sx < srcW && sy < srcH;
		};

		int nSegments = pEdgemap ? pEdgemap->renderSegments() : 1;

		EdgeMode edgeMode = (m_pBlitSource && m_pBlitSource->isTiling()) ? EdgeMode::Tile : EdgeMode::Clamp;

		RectI pixels = RectI::overlap(_pixelRect(rect), RectI(dest, warpSize));

		for (int y = pixels.y; y < pixels.y + pixels.h; y++)
		{
			for (int x = pixels.x; x < pixels.x + pixels.w; x++)
			{
				if (!_isInsideCanvas(x, y))
					continue;

				Cell cell = cellOf(x - dest.x, y - dest.y);

				// Coverage of the source or of each segment, -1 for outside.

				double coverage[Edgemap::maxSegments];
				double covered = 0;

				auto measure = [&](int nSamples, int step)
				{
					for (int seg = 0; seg < nSegments; seg++)
						coverage[seg] = 0;
					covered = 0;

					double sampleArea = 1.0 / nSamples;
					int first = -2;
					bool bSame = true;

					for (int i = 0; i < nSamples; i++)
					{
						double px = x - dest.x + (i + 0.5) / nSamples;
						double py = y - dest.y + (((int64_t(i) * step) % nSamples) + 0.5) / nSamples;

						double posX, posY;
						sourcePos(cell, px, py, posX, posY);

						int seg = -1;

						if (isInside(posX, posY))
						{
							seg = pEdgemap ? _edgemapSegmentAt(pEdgemap, posX, posY) : 0;
							coverage[seg] += sampleArea;
							covered += sampleArea;
						}

						if (first == -2)
							first = seg;
						else if (seg != first)
							bSame = false;
					}

					return bSame;
				};

				if (!measure(c_warpQuickSamples, c_warpQuickStep))
					measure(c_warpSamples, c_warpStep);

				if (covered <= 0.0)
					continue;

				// Colors at the center of the pixel.

				double centerX, centerY;
				sourcePos(cell, x - dest.x + 0.5, y - dest.y + 0.5, centerX, centerY);
				nearestCopy(centerX, srcW, periodX);
				nearestCopy(centerY, srcH, periodY);

				Color tint = _deviceTint(x, y);

				if (!pEdgemap)
				{
					// A non-tiling surface is read as if the edge pixels of the source rectangle
					// continued outside it: positions are kept between the centers of its edge pixels
					// when bilinear, inside it when nearest.

					double readX = srcX + centerX;
					double readY = srcY + centerY;

					if (edgeMode == EdgeMode::Clamp)
					{
						auto keepInside = [&](double pos, double begin, double size)
						{
							double inset = m_pBlitSource->sampleMethod() == SampleMethod::Bilinear ? 0.5 : 1.0 / 65536;
							double lo = begin + inset;
							double hi = begin + size - inset;
							return lo > hi ? begin + size / 2 : std::clamp(pos, lo, hi);
						};

						readX = keepInside(readX, srcX, srcW);
						readY = keepInside(readY, srcY, srcH);
					}

					Color texel = _sampleSource(readX, readY, edgeMode);
					_plotPixel(x, y, texel * tint, covered);
					continue;
				}

				Color colors[Edgemap::maxSegments];

				for (int seg = 0; seg < nSegments; seg++)
				{
					Color segColor = pEdgemap->tints()[seg] ? _tintColorAt(m_segmentTints[seg], centerX, centerY)
															: _toLinear(pEdgemap->flatColors()[seg]);
					colors[seg] = segColor * tint;
				}

				Color dst = _canvasPixel(x, y);

				// Effect modes apply the segments together, see _effectSource(). Parts of the pixel
				// outside the source are transparent, which has no effect.

				if (_isEffectMode())
				{
					_setCanvasPixel(x, y, _blend(_effectSource(nSegments, colors, coverage), dst));
					continue;
				}

				// Paint modes average what each segment gives its part of the pixel, with the
				// part outside the source keeping the canvas.

				Color results[Edgemap::maxSegments + 1];
				double weights[Edgemap::maxSegments + 1];

				for (int seg = 0; seg < nSegments; seg++)
				{
					results[seg] = _blend(colors[seg], dst);
					weights[seg] = coverage[seg];
				}

				results[nSegments] = dst;
				weights[nSegments] = std::max(0.0, 1.0 - covered);

				_setCanvasPixel(x, y, _averageColors(nSegments + 1, results, weights, true));
			}
		}
	}

	//____ _edgemapSegmentAt() ______________________________________________________
	/*
		Segment of an edgemap at position x,y (in pixels of the edgemap, inside it). Edges are
		linear across each column of pixels. An edge that goes above an edge before it is pushed
		down to that edge, as in _segmentCoverage(). A point exactly on an edge is below it.
	*/

	int ReferenceBackend::_edgemapSegmentAt(SoftEdgemap* pEdgemap, double x, double y) const
	{
		int nEdges = pEdgemap->renderSegments() - 1;
		int samplePitch = pEdgemap->m_nbSegments - 1;

		int column = std::clamp(int(std::floor(x)), 0, pEdgemap->pixelSize().w - 1);
		double frac = x - column;

		const spx* pLeft = pEdgemap->m_pSamples + column * samplePitch;
		const spx* pRight = pLeft + samplePitch;

		double lowest = -std::numeric_limits<double>::infinity();
		int segment = 0;

		for (int edge = 0; edge < nEdges; edge++)
		{
			double edgeY = std::max(lowest, (pLeft[edge] + (pRight[edge] - pLeft[edge]) * frac) / 64.0);
			lowest = edgeY;

			if (y >= edgeY)
				segment = edge + 1;
		}

		return segment;
	}

	//____ _isInsideCanvas() ______________________________________________________

	bool ReferenceBackend::_isInsideCanvas(int x, int y) const
	{
		if (!m_pCanvas)
			return false;

		const RectI& r = m_canvasBuffer.rect;
		return (x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h);
	}

	//____ _canvasPixel() _________________________________________________________

	ReferenceBackend::Color ReferenceBackend::_canvasPixel(int x, int y) const
	{
		if (!_isInsideCanvas(x, y))
			return Color();

		return _readPixel(m_canvasBuffer, x, y);
	}

	//____ _setCanvasPixel() ______________________________________________________

	void ReferenceBackend::_setCanvasPixel(int x, int y, const Color& color)
	{
		if (_isInsideCanvas(x, y))
			_writePixel(m_canvasBuffer, x, y, color);
	}

	//____ _plotPixel() ___________________________________________________________
	/*
		Blends color into a pixel of the canvas, where it covers coverage (0.0 -> 1.0) of the pixel.

		Paint modes draw a partially covered pixel as if the canvas had a much higher resolution, the
		shape was drawn there without anti-aliasing and the subpixels were then averaged. That is the
		average of the blended color for the covered part and the original color for the rest,
		weighted by their areas and averaged as premultiplied colors.

		Effect modes are applied with a strength scaled by the coverage, see _effectSource().
	*/

	void ReferenceBackend::_plotPixel(int x, int y, const Color& color, double coverage)
	{
		if (m_blendMode == BlendMode::Ignore || coverage <= 0.0 || !_isInsideCanvas(x, y))
			return;

		Color dst = _canvasPixel(x, y);

		if (coverage < 1.0 && _isEffectMode())
		{
			_setCanvasPixel(x, y, _blend(_effectSource(1, &color, &coverage), dst));
			return;
		}

		Color result = _blend(color, dst);

		if (coverage < 1.0)
		{
			Color colors[2] = { dst, result };
			double areas[2] = { 1.0 - coverage, coverage };

			result = _averageColors(2, colors, areas, true);
		}

		_setCanvasPixel(x, y, result);
	}

	//____ _isEffectMode() ________________________________________________________

	bool ReferenceBackend::_isEffectMode() const
	{
		switch (m_blendMode)
		{
			case BlendMode::Add:
			case BlendMode::Subtract:
			case BlendMode::Multiply:
			case BlendMode::Invert:
			case BlendMode::Min:
			case BlendMode::Max:
				return true;
			default:
				return false;
		}
	}

	//____ _effectSource() ________________________________________________________
	/*
		Effect modes don't average what the covered and uncovered parts of a pixel get. Coverage
		scales the strength of the effect instead, just like the alpha of the source does. Colors that
		cover parts of the pixel, like the segments of an edgemap, are first combined into one source:
		their premultiplied average weighted by coverage, which has the alpha of them all together.
		Where nothing covers the pixel the source is transparent, which has no effect.

		On an alpha-only canvas the alpha of the source is the value, so the parts not covered get the
		value that has no effect instead, which is 1.0 for Multiply and Min.
	*/

	ReferenceBackend::Color ReferenceBackend::_effectSource(int nColors, const Color* pColors, const double* pCoverage) const
	{
		Color src = _averageColors(nColors, pColors, pCoverage, true);

		if (m_bAlphaOnlyCanvas && (m_blendMode == BlendMode::Multiply || m_blendMode == BlendMode::Min))
		{
			double covered = 0.0;
			for (int i = 0; i < nColors; i++)
				covered += pCoverage[i];

			src.a += std::max(1.0 - covered, 0.0);
		}

		return src;
	}

	//____ _averageColors() _______________________________________________________
	/*
		Weighted average of colors. If bPremultiplied is set, colors are averaged as premultiplied
		colors: each color also counts by its alpha, so the more transparent it is, the less its color
		counts and a fully transparent color only makes the result more transparent. Otherwise the
		color channels are averaged as they are.
	*/

	ReferenceBackend::Color ReferenceBackend::_averageColors(int nColors, const Color* pColors, const double* pWeights, bool bPremultiplied)
	{
		Color sum;
		double colorWeightSum = 0.0;

		for (int i = 0; i < nColors; i++)
		{
			double colorWeight = bPremultiplied ? pWeights[i] * pColors[i].a : pWeights[i];

			sum.r += pColors[i].r * colorWeight;
			sum.g += pColors[i].g * colorWeight;
			sum.b += pColors[i].b * colorWeight;
			sum.a += pColors[i].a * pWeights[i];

			colorWeightSum += colorWeight;
		}

		if (colorWeightSum > 0.0)
		{
			sum.r /= colorWeightSum;
			sum.g /= colorWeightSum;
			sum.b /= colorWeightSum;
		}

		return sum;
	}

	//____ _blend() _______________________________________________________________
	/*
		Blends a source color with the destination color of the canvas, according to the blend mode.
		These are the definitions of the blend modes, which are explained in the guide under
		"Blending and transparency".

		Paint modes (Replace, Blend, BlendFixedColor and Morph) put paint on the canvas.
		BlendFixedColor is Blend, made faster where the canvas is known to be the fixed blend
		color, so we draw it as Blend.

		Effect modes (Add, Subtract, Multiply, Invert, Min and Max) change the light that is already
		on the canvas, see _blendEffect().
	*/

	ReferenceBackend::Color ReferenceBackend::_blend(const Color& src, const Color& dst) const
	{
		if (m_bAlphaOnlyCanvas)
			return { 1.0, 1.0, 1.0, _blendAlphaOnly(src.a, dst.a) };

		Color result;

		switch (m_blendMode)
		{
			case BlendMode::Ignore:
				result = dst;
				break;

			case BlendMode::Replace:
				result = src;
				break;

			case BlendMode::Undefined:					// Undefined is treated as Blend.
			case BlendMode::Blend:
			case BlendMode::BlendFixedColor:
				result = _over(src, dst);
				break;

			case BlendMode::Morph:
			{
				// Cross-fade from the canvas to the source, as premultiplied colors.

				Color colors[2] = { dst, src };
				double weights[2] = { 1.0 - m_morphFactor, m_morphFactor };

				result = _averageColors(2, colors, weights, true);
				break;
			}

			case BlendMode::Add:
			case BlendMode::Subtract:
			case BlendMode::Multiply:
			case BlendMode::Invert:
			case BlendMode::Min:
			case BlendMode::Max:
				result = _blendEffect(src, dst);
				break;

			default:
				result = dst;
				break;
		}

		// A canvas without alpha just doesn't store the alpha of the result, its pixels stay opaque.
		// Only paint modes can give a result that isn't opaque here.

		if (!m_bCanvasHasAlpha)
			result.a = 1.0;

		return result;
	}

	//____ _blendEffect() _________________________________________________________
	/*
		Effect modes change the light that a pixel of the canvas gives, which is its premultiplied
		color P (its color times its alpha). Think of the canvas as glass with paint on it: an effect
		is applied to the light that comes from the paint, not to the light that passes through the
		glass from behind.

		The alpha of the source is the strength of the effect. The source is applied as its color
		times its alpha (s), which fades it towards black, the color that has no effect for Add,
		Subtract, Invert and Max. Multiply and Min have no effect with white, so they apply the source
		faded towards white by its alpha (t).

		Add, Invert and Max can give more light than the paint on the glass can hold. They raise the
		alpha of the canvas by the strongest channel of s (m), Add by adding it, Max by taking the
		larger of the two and Invert by putting it over the alpha of the canvas. That way they can
		light up transparent parts of the canvas. The other effect modes keep the alpha.

		These are the operations that GPUs can do with fixed-function blending of premultiplied
		colors in a single pass. On an opaque canvas they are the familiar ones, applied to the color.
	*/

	ReferenceBackend::Color ReferenceBackend::_blendEffect(const Color& src, const Color& dst) const
	{
		double back[3] = { dst.r * dst.a, dst.g * dst.a, dst.b * dst.a };
		double light[3] = { src.r * src.a, src.g * src.a, src.b * src.a };
		double source[3] = { src.r, src.g, src.b };
		double strongest = std::max({ light[0], light[1], light[2] });

		double result[3];

		for (int i = 0; i < 3; i++)
		{
			double P = back[i];
			double s = light[i];
			double t = _mix(1.0, source[i], src.a);

			switch (m_blendMode)
			{
				case BlendMode::Add:		result[i] = P + s;						break;
				case BlendMode::Subtract:	result[i] = P - s;						break;
				case BlendMode::Multiply:	result[i] = P * t;						break;
				case BlendMode::Invert:		result[i] = P + s * (1.0 - 2.0 * P);	break;
				case BlendMode::Min:		result[i] = std::min(P, t);				break;
				case BlendMode::Max:		result[i] = std::max(P, s);				break;
				default:					result[i] = P;							break;
			}
		}

		double alpha = dst.a;

		if (m_blendMode == BlendMode::Add)
			alpha = dst.a + strongest;
		else if (m_blendMode == BlendMode::Invert)
			alpha = dst.a + strongest * (1.0 - dst.a);
		else if (m_blendMode == BlendMode::Max)
			alpha = std::max(dst.a, strongest);

		alpha = _clamp01(alpha);

		if (alpha <= 0.0)
			return { dst.r, dst.g, dst.b, 0.0 };			// Still transparent, the color doesn't matter.

		return { std::min(_clamp01(result[0]) / alpha, 1.0), std::min(_clamp01(result[1]) / alpha, 1.0),
				 std::min(_clamp01(result[2]) / alpha, 1.0), alpha };
	}

	//____ _over() ________________________________________________________________
	/*
		Porter-Duff 'over': src placed on top of dst. Where dst is transparent, more of src shows
		through, so drawing onto a transparent surface and then blending that surface onto a
		background gives the same result as drawing directly onto the background.

		This is the same as averaging the colors of src and dst as premultiplied colors, with src
		weighted by its alpha and dst by its alpha times what src leaves uncovered. When dst is
		opaque it is the familiar src * src.a + dst * (1 - src.a).
	*/

	ReferenceBackend::Color ReferenceBackend::_over(const Color& src, const Color& dst)
	{
		Color colors[2] = { src, dst };
		double weights[2] = { 1.0, 1.0 - src.a };

		return _averageColors(2, colors, weights, true);
	}

	//____ _blendAlphaOnly() ______________________________________________________
	/*
		Blends the alpha of a source with the alpha of a canvas with alpha only (Alpha_8). Returns
		the new alpha of the canvas.

		Such a canvas is a one-channel image and its alpha is its value. Paint modes work on alpha
		just like on any other canvas. Effect modes would have nothing to work on if they kept
		alpha. Instead they work on the alpha of the canvas with the alpha of the source as their
		value.
	*/

	double ReferenceBackend::_blendAlphaOnly(double src, double dst) const
	{
		switch (m_blendMode)
		{
			case BlendMode::Ignore:
				return dst;
			case BlendMode::Replace:
				return src;
			case BlendMode::Undefined:
			case BlendMode::Blend:
			case BlendMode::BlendFixedColor:
				return src + dst * (1.0 - src);
			case BlendMode::Morph:
				return _mix(dst, src, m_morphFactor);
			case BlendMode::Add:
				return _clamp01(dst + src);
			case BlendMode::Subtract:
				return _clamp01(dst - src);
			case BlendMode::Multiply:
				return dst * src;
			case BlendMode::Invert:
				return src * (1.0 - dst) + dst * (1.0 - src);
			case BlendMode::Min:
				return std::min(dst, src);
			case BlendMode::Max:
				return std::max(dst, src);
			default:
				return dst;
		}
	}

	//____ _sampleSource() ________________________________________________________
	/*
		Reads the blit source at position x, y, where pixel (x,y) covers x -> x+1 and y -> y+1.

		Nearest sampling gives the pixel the position is in. Bilinear sampling interpolates between
		the four pixels with their centers closest to the position, as premultiplied colors so that
		the color of transparent pixels doesn't bleed into their neighbours.
	*/

	ReferenceBackend::Color ReferenceBackend::_sampleSource(double x, double y, EdgeMode edgeMode) const
	{
		if (m_pBlitSource->sampleMethod() != SampleMethod::Bilinear)
			return _sourcePixel(int(std::floor(x)), int(std::floor(y)), edgeMode);

		// Position relative to the pixel centers.

		double cx = x - 0.5;
		double cy = y - 0.5;

		int x0 = int(std::floor(cx));
		int y0 = int(std::floor(cy));

		double fracX = cx - x0;
		double fracY = cy - y0;

		Color pixels[4] = { _sourcePixel(x0, y0, edgeMode), _sourcePixel(x0 + 1, y0, edgeMode),
							_sourcePixel(x0, y0 + 1, edgeMode), _sourcePixel(x0 + 1, y0 + 1, edgeMode) };

		double weights[4] = { (1.0 - fracX) * (1.0 - fracY), fracX * (1.0 - fracY),
							  (1.0 - fracX) * fracY, fracX * fracY };

		return _averageColors(4, pixels, weights, true);
	}

	//____ _sampleBlurred() _______________________________________________________
	/*
		Blurs the blit source around position x, y by sampling it at the position and at eight
		positions around it, the radius of the blur away (straight up, down, left and right) or
		0.7 times the radius away in both directions (diagonally). The red, green and blue channels
		are weighted sums of the samples, with weights from the blur state. Alpha is the alpha of the
		center sample. Samples outside the source read the pixels on its edge.
	*/

	ReferenceBackend::Color ReferenceBackend::_sampleBlurred(double x, double y) const
	{
		double r = m_blurRadius;
		double d = m_blurRadius * 0.7;

		const double offsets[9][2] = {	{ -d, -d }, { 0, -r }, { d, -d },
										{ -r,  0 }, { 0,  0 }, { r,  0 },
										{ -d,  d }, { 0,  r }, { d,  d } };

		Color result;

		for (int i = 0; i < 9; i++)
		{
			Color sample = _sampleSource(x + offsets[i][0], y + offsets[i][1], EdgeMode::Clamp);

			result.r += sample.r * m_blurWeights[0][i];
			result.g += sample.g * m_blurWeights[1][i];
			result.b += sample.b * m_blurWeights[2][i];

			if (i == 4)
				result.a = sample.a;
		}

		result.r = _clamp01(result.r);
		result.g = _clamp01(result.g);
		result.b = _clamp01(result.b);

		return result;
	}

	//____ _sourcePixel() _________________________________________________________

	ReferenceBackend::Color ReferenceBackend::_sourcePixel(int x, int y, EdgeMode edgeMode) const
	{
		SizeI size = m_pBlitSource->pixelSize();

		bool bOutside = (x < 0 || y < 0 || x >= size.w || y >= size.h);

		if (bOutside)
		{
			switch (edgeMode)
			{
				case EdgeMode::Clip:
					return Color();

				case EdgeMode::Tile:
					x = _wrap(x, size.w);
					y = _wrap(y, size.h);
					break;

				case EdgeMode::Clamp:
					x = std::clamp(x, 0, size.w - 1);
					y = std::clamp(y, 0, size.h - 1);
					break;
			}
		}

		return _readPixel(m_blitSourceBuffer, x, y);
	}

	//____ _deviceTint() __________________________________________________________
	/*
		The color the device tints pixel x,y of the canvas with: the tint color multiplied with
		the Tint (if any) at the center of the pixel.
	*/

	ReferenceBackend::Color ReferenceBackend::_deviceTint(int x, int y) const
	{
		if (m_tint.nLayers == 0)
			return m_tintColor;

		return m_tintColor * _tintColorAt(m_tint, x + 0.5, y + 0.5);
	}

	//____ _segmentColor() ________________________________________________________
	/*
		Color of a segment of an edgemap at the given pixel of the edgemap, from its Tint at
		the center of the pixel or its flat color.
	*/

	ReferenceBackend::Color ReferenceBackend::_segmentColor(SoftEdgemap* pEdgemap, int segment, int column, int row) const
	{
		if (pEdgemap->tints()[segment])
			return _tintColorAt(m_segmentTints[segment], column + 0.5, row + 0.5);

		return _toLinear(pEdgemap->flatColors()[segment]);
	}

	//____ _tintColorAt() _________________________________________________________
	/*
		Evaluates a Tint at position x, y (in pixels of the space the Tint is placed in).

		Each layer of the Tint is a gradient. The position along the gradient is calculated from its
		geometry and adjusted by its spread. The color is interpolated in sRGB between the two color
		stops around the position (or taken from the first or last stop if outside them). The
		resulting color is converted to linear and the layers are mixed by their weights.
	*/

	ReferenceBackend::Color ReferenceBackend::_tintColorAt(const TintTools::DecodedTint& tint, double x, double y)
	{
		if (tint.nLayers == 0)
			return { 1, 1, 1, 1 };

		Color result;

		for (int l = 0; l < tint.nLayers; l++)
		{
			const TintTools::TintLayer& layer = tint.layers[l];

			if (layer.weight == 0 || layer.nStops == 0)
				continue;

			// Position along the gradient. See TintTools::CanvasGeometry.

			double position;

			if (layer.shape == TintShape::Linear)
				position = layer.geo[0] * x + layer.geo[1] * y + layer.geo[2];
			else
			{
				double u = (x - layer.geo[0]) * layer.geo[2];
				double v = (y - layer.geo[1]) * layer.geo[3];
				position = std::sqrt(u * u + v * v);
			}

			// Spread

			switch (layer.spread)
			{
				default:
				case TintSpread::Pad:
					position = _clamp01(position);
					break;

				case TintSpread::Repeat:
					position = position - std::floor(position);
					break;

				case TintSpread::Reflect:
					position = position - 2.0 * std::floor(position / 2.0);			// 0.0 -> 2.0
					if (position > 1.0)
						position = 2.0 - position;
					break;
			}

			// Find the last stop at or before position. At a hard edge (two stops at the same position)
			// that is the second stop, so we get the color after the edge.

			const float* pStopPos = layer.stopPos;
			const HiColor* pStopColors = layer.stopColors;

			HiColor from, to;
			double fraction = 0.0;

			int stop = layer.nStops - 1;
			while (stop >= 0 && pStopPos[stop] > position)
				stop--;

			if (stop < 0)
				from = to = pStopColors[0];						// Before the first stop.
			else if (stop == layer.nStops - 1)
				from = to = pStopColors[stop];					// At or after the last stop.
			else
			{
				from = pStopColors[stop];
				to = pStopColors[stop + 1];
				fraction = (position - pStopPos[stop]) / (pStopPos[stop + 1] - pStopPos[stop]);
			}

			// Interpolate in sRGB, then convert to linear.

			Color color;
			color.r = _sRGBToLinear(_mix(from.r, to.r, fraction) / 4096.0);
			color.g = _sRGBToLinear(_mix(from.g, to.g, fraction) / 4096.0);
			color.b = _sRGBToLinear(_mix(from.b, to.b, fraction) / 4096.0);
			color.a = _mix(from.a, to.a, fraction) / 4096.0;

			result = result + color * (layer.weight / 4096.0);
		}

		return result;
	}

	//____ _toLinear() ____________________________________________________________
	/*
		Converts a HiColor, which has sRGB color channels and linear alpha, all 0 -> 4096.
	*/

	ReferenceBackend::Color ReferenceBackend::_toLinear(HiColor color)
	{
		return { _sRGBToLinear(color.r / 4096.0), _sRGBToLinear(color.g / 4096.0), _sRGBToLinear(color.b / 4096.0), color.a / 4096.0 };
	}

	//____ _sRGBToLinear() ________________________________________________________
	/*
		The sRGB transfer functions, exactly as specified.
	*/

	double ReferenceBackend::_sRGBToLinear(double value)
	{
		value = _clamp01(value);
		return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
	}

	//____ _linearToSRGB() ________________________________________________________

	double ReferenceBackend::_linearToSRGB(double value)
	{
		value = _clamp01(value);
		return value <= 0.0031308 ? value * 12.92 : 1.055 * std::pow(value, 1.0 / 2.4) - 0.055;
	}

	//____ _readPixel() ___________________________________________________________
	/*
		Reads pixel x, y of a pixel buffer and converts it to a linear color.

		Channels are stored as integers from 0 up to the largest value that fits their bits, which
		is 0.0 -> 1.0. Color channels are in the color space of the pixels, alpha is always linear.
		Formats without color channels are white, formats without alpha are opaque.
	*/

	ReferenceBackend::Color ReferenceBackend::_readPixel(const PixelBuffer& buffer, int x, int y)
	{
		int bytesPerPixel = Util::pixelFormatToDescription(buffer.format).bits / 8;
		const uint8_t* p = buffer.pixels + (y - buffer.rect.y) * buffer.pitch + (x - buffer.rect.x) * bytesPerPixel;

		bool bLinear = (buffer.colorSpace == ColorSpace::Linear);			// Undefined is treated as sRGB.

		auto decode = [bLinear](int value, int maxValue)
		{
			double encoded = double(value) / maxValue;
			return bLinear ? encoded : _sRGBToLinear(encoded);
		};

		switch (buffer.format)
		{
			case PixelFormat::ARGB_8:
			case PixelFormat::XRGB_8:
			{
				// Channels are A, R, G, B from most to least significant byte.

				uint32_t v = buffer.bigEndian ? (uint32_t(p[0]) << 24) | (p[1] << 16) | (p[2] << 8) | p[3]
											  : (uint32_t(p[3]) << 24) | (p[2] << 16) | (p[1] << 8) | p[0];

				double alpha = (buffer.format == PixelFormat::ARGB_8) ? (v >> 24) / 255.0 : 1.0;

				return { decode((v >> 16) & 0xFF, 255), decode((v >> 8) & 0xFF, 255), decode(v & 0xFF, 255), alpha };
			}

			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
			{
				// Five bits of red (RGB_565) or blue (BGR_565) at the top, six bits of green in the middle
				// and five bits of the other one at the bottom.

				uint16_t v = buffer.bigEndian ? (p[0] << 8) | p[1] : (p[1] << 8) | p[0];

				double high = decode(v >> 11, 31);
				double green = decode((v >> 5) & 0x3F, 63);
				double low = decode(v & 0x1F, 31);

				if (buffer.format == PixelFormat::RGB_565)
					return { high, green, low, 1.0 };
				else
					return { low, green, high, 1.0 };
			}

			case PixelFormat::Alpha_8:
				return { 1.0, 1.0, 1.0, p[0] / 255.0 };

			case PixelFormat::Index_8:
			case PixelFormat::Index_16:
			{
				int index = (buffer.format == PixelFormat::Index_8) ? p[0] : buffer.bigEndian ? (p[0] << 8) | p[1] : (p[1] << 8) | p[0];

				if (!buffer.palette)
					return Color();

				Color8 c = buffer.palette[index];
				return { decode(c.r, 255), decode(c.g, 255), decode(c.b, 255), c.a / 255.0 };
			}

			default:
				return Color();
		}
	}

	//____ _writePixel() __________________________________________________________
	/*
		Writes a linear color to pixel x, y of a pixel buffer. This is where colors are rounded,
		to nearest value the pixel can hold. Padding bits are left as they are.
	*/

	void ReferenceBackend::_writePixel(const PixelBuffer& buffer, int x, int y, const Color& color)
	{
		int bytesPerPixel = Util::pixelFormatToDescription(buffer.format).bits / 8;
		uint8_t* p = buffer.pixels + (y - buffer.rect.y) * buffer.pitch + (x - buffer.rect.x) * bytesPerPixel;

		bool bLinear = (buffer.colorSpace == ColorSpace::Linear);			// Undefined is treated as sRGB.

		auto encode = [bLinear](double linear, int maxValue)
		{
			double encoded = bLinear ? _clamp01(linear) : _linearToSRGB(linear);
			return uint32_t(std::floor(encoded * maxValue + 0.5));
		};

		auto encodeAlpha = [](double alpha, int maxValue)
		{
			return uint32_t(std::floor(_clamp01(alpha) * maxValue + 0.5));
		};

		switch (buffer.format)
		{
			case PixelFormat::ARGB_8:
			case PixelFormat::XRGB_8:
			{
				// Channels are A, R, G, B from most to least significant byte. XRGB_8 keeps its padding byte.

				uint8_t& top = buffer.bigEndian ? p[0] : p[3];
				uint32_t a = (buffer.format == PixelFormat::ARGB_8) ? encodeAlpha(color.a, 255) : top;

				uint32_t v = (a << 24) | (encode(color.r, 255) << 16) | (encode(color.g, 255) << 8) | encode(color.b, 255);

				for (int i = 0; i < 4; i++)
				{
					int shift = buffer.bigEndian ? 24 - i * 8 : i * 8;
					p[i] = uint8_t(v >> shift);
				}
				break;
			}

			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
			{
				uint32_t red = encode(color.r, 31);
				uint32_t green = encode(color.g, 63);
				uint32_t blue = encode(color.b, 31);

				uint32_t high = (buffer.format == PixelFormat::RGB_565) ? red : blue;
				uint32_t low = (buffer.format == PixelFormat::RGB_565) ? blue : red;

				uint16_t v = uint16_t((high << 11) | (green << 5) | low);

				p[buffer.bigEndian ? 0 : 1] = uint8_t(v >> 8);
				p[buffer.bigEndian ? 1 : 0] = uint8_t(v);
				break;
			}

			case PixelFormat::Alpha_8:
				p[0] = uint8_t(encodeAlpha(color.a, 255));
				break;

			default:
				break;						// We can't render to other formats, setCanvas() makes sure we don't try.
		}
	}

	//____ _pixelRect() ___________________________________________________________
	/*
		The pixels of a rectangle in spx, e.g. the pixels with their centers inside it.
	*/

	RectI ReferenceBackend::_pixelRect(const RectSPX& rect)
	{
		int left = Util::roundToPixels(rect.x);
		int top = Util::roundToPixels(rect.y);
		int right = Util::roundToPixels(rect.x + rect.w);
		int bottom = Util::roundToPixels(rect.y + rect.h);

		return RectI(left, top, right - left, bottom - top);
	}

	//____ Shapes ________________________________________________________________
	/*
		Shapes of Command::DrawShape are made of segments that are drawn together: fill and
		outline of a rounded box, back, outline, hub and slices of a ring.

		Each kind of shape is a class that decodes its payload and defines the shape by
		segmentAt(), which tells what segment a point of the canvas is in. That is written as
		plainly as the definition of the shape in GfxDevice.

		To work out how much of a pixel each segment covers we also need the edges of the shape,
		the straight lines and circles along which one segment can turn into another. They are
		given whole: a line or circle may go on where the shape has no edge, which does no harm.
		See _shapeCoverage().
	*/

	static const double c_pi = 3.14159265358979323846;

	//____ ShapeEdge ______________________________________________________________
	// A straight line or a circle.

	struct ReferenceBackend::ShapeEdge
	{
		bool	bCircle = false;
		double	x = 0, y = 0;					// A point on the line, or the center of the circle.
		double	dirX = 0, dirY = 0;				// Direction of the line, of length 1.
		double	radius = 0;						// Of the circle.

		static ShapeEdge line(double x, double y, double dirX, double dirY)
		{
			double length = std::sqrt(dirX * dirX + dirY * dirY);
			return { false, x, y, dirX / length, dirY / length, 0 };
		}

		static ShapeEdge circle(double x, double y, double radius)
		{
			return { true, x, y, 0, 0, radius };
		}

		// Tells if the edge passes through the inside of the pixel at x, y.

		bool passesThrough(int px, int py) const
		{
			const double corners[4][2] = { { double(px), double(py) }, { px + 1.0, double(py) }, { double(px), py + 1.0 }, { px + 1.0, py + 1.0 } };

			if (bCircle)
			{
				// The circle passes through if it is farther from the center than the nearest
				// point of the pixel and closer than the farthest.

				double nearX = std::clamp(x, double(px), px + 1.0) - x;
				double nearY = std::clamp(y, double(py), py + 1.0) - y;
				double nearest = std::sqrt(nearX * nearX + nearY * nearY);

				double farthest = 0.0;
				for (auto& corner : corners)
					farthest = std::max(farthest, std::hypot(corner[0] - x, corner[1] - y));

				return nearest < radius && radius < farthest;
			}

			// The line passes through if corners of the pixel are on both sides of it.

			double lowest = DBL_MAX, highest = -DBL_MAX;
			for (auto& corner : corners)
			{
				double side = (corner[0] - x) * dirY - (corner[1] - y) * dirX;
				lowest = std::min(lowest, side);
				highest = std::max(highest, side);
			}

			return lowest < 0.0 && highest > 0.0;
		}

		// Adds y of the points where the edge crosses the vertical line at u.

		void crossingsAtX(double u, std::vector<double>& ys) const
		{
			if (bCircle)
			{
				double dx = u - x;
				if (std::abs(dx) < radius)
				{
					double dy = std::sqrt(radius * radius - dx * dx);
					ys.push_back(y - dy);
					ys.push_back(y + dy);
				}
			}
			else if (dirX != 0.0)
				ys.push_back(y + (u - x) * dirY / dirX);
		}

		// Adds x of the points where the edge crosses the horizontal line at v.

		void crossingsAtY(double v, std::vector<double>& xs) const
		{
			if (bCircle)
			{
				double dy = v - y;
				if (std::abs(dy) < radius)
				{
					double dx = std::sqrt(radius * radius - dy * dy);
					xs.push_back(x - dx);
					xs.push_back(x + dx);
				}
			}
			else if (dirY != 0.0)
				xs.push_back(x + (v - y) * dirX / dirY);
		}

		// Adds x of the points where the edge crosses another.

		void intersections(const ShapeEdge& other, std::vector<double>& xs) const
		{
			if (!bCircle && !other.bCircle)
			{
				// Where x, y + t * dir is on the other line.

				double det = dirX * other.dirY - dirY * other.dirX;
				if (det != 0.0)
				{
					double t = ((other.x - x) * other.dirY - (other.y - y) * other.dirX) / det;
					xs.push_back(x + t * dirX);
				}
			}
			else if (bCircle && other.bCircle)
			{
				// Along the line between the centers, and across it.

				double dx = other.x - x;
				double dy = other.y - y;
				double d = std::sqrt(dx * dx + dy * dy);

				if (d > 0.0 && d <= radius + other.radius && d >= std::abs(radius - other.radius))
				{
					double along = (radius * radius - other.radius * other.radius + d * d) / (2 * d);
					double across = std::sqrt(std::max(0.0, radius * radius - along * along));
					double middleX = x + along * dx / d;

					xs.push_back(middleX - across * dy / d);
					xs.push_back(middleX + across * dy / d);
				}
			}
			else
			{
				// Where the point line.x, line.y + t * line.dir is at radius from the center.

				const ShapeEdge& line = bCircle ? other : *this;
				const ShapeEdge& circle = bCircle ? *this : other;

				double px = line.x - circle.x;
				double py = line.y - circle.y;
				double half = px * line.dirX + py * line.dirY;
				double discriminant = half * half - (px * px + py * py - circle.radius * circle.radius);

				if (discriminant >= 0.0)
				{
					double root = std::sqrt(discriminant);
					xs.push_back(line.x + (-half - root) * line.dirX);
					xs.push_back(line.x + (-half + root) * line.dirX);
				}
			}
		}
	};

	//____ Shape __________________________________________________________________

	class ReferenceBackend::Shape
	{
	public:
		virtual ~Shape() = default;

		// The segment a point is in, -1 for none.

		virtual int		segmentAt(double x, double y) const = 0;

		struct Segment
		{
			bool					bDrawn = false;
			Color					color;					// Linear.
			TintTools::DecodedTint	tint = {};				// Placed on the canvas, nLayers is 0 if none.
		};

		std::vector<Segment>	segments;
		std::vector<ShapeEdge>	edges;

	protected:

		// Sets up a segment: its color from pColors if drawn and its tint from pTint if tinted,
		// stepping both forward. The colors of the tints follow the colors of all segments.

		void setupSegment(int index, bool bDrawn, const HiColor*& pColors, bool bTinted, const uint16_t*& pTint, const HiColor*& pTintColors)
		{
			Segment& segment = segments[index];
			segment.bDrawn = bDrawn;

			if (bDrawn)
				segment.color = _toLinear(*pColors++);

			if (bTinted)
				pTint = TintTools::decodeTint(pTint, pTintColors, segment.tint);
		}
	};

	//____ RoundedBoxShape ________________________________________________________
	/*
		A box with rounded corners and an outline inside it, see GfxDevice::drawRoundedBox().

		The inner edge of the outline is a box of its own: rect inset by the outline, with the
		radius of each corner reduced by the thicker of the two sides meeting there. Where an
		uneven outline makes it reach outside the outer edge, the outline just gets thinner.
	*/

	class ReferenceBackend::RoundedBoxShape : public ReferenceBackend::Shape
	{
	public:

		static constexpr int c_fill = 0;
		static constexpr int c_outline = 1;

		// A box with circular corners. Radii are top-left, top-right, bottom-right and
		// bottom-left, 0 for a square corner.

		struct Box
		{
			double	left = 0, top = 0, right = 0, bottom = 0;
			double	radius[4] = { 0, 0, 0, 0 };

			// Center of the circle of a corner, which touches both sides meeting there.

			double	centerX(int corner) const { return (corner == 0 || corner == 3) ? left + radius[corner] : right - radius[corner]; }
			double	centerY(int corner) const { return (corner == 0 || corner == 1) ? top + radius[corner] : bottom - radius[corner]; }

			bool contains(double x, double y) const
			{
				if (x < left || x >= right || y < top || y >= bottom)
					return false;

				// Points in the corner of the box beyond the center of its circle are only inside
				// if they are inside the circle.

				for (int corner = 0; corner < 4; corner++)
				{
					double cx = centerX(corner);
					double cy = centerY(corner);

					bool bBeyondX = (corner == 0 || corner == 3) ? x < cx : x > cx;
					bool bBeyondY = (corner == 0 || corner == 1) ? y < cy : y > cy;

					if (bBeyondX && bBeyondY && std::hypot(x - cx, y - cy) > radius[corner])
						return false;
				}

				return true;
			}

			void addEdges(std::vector<ShapeEdge>& edges) const
			{
				edges.push_back(ShapeEdge::line(left, top, 1, 0));
				edges.push_back(ShapeEdge::line(left, bottom, 1, 0));
				edges.push_back(ShapeEdge::line(left, top, 0, 1));
				edges.push_back(ShapeEdge::line(right, top, 0, 1));

				for (int corner = 0; corner < 4; corner++)
				{
					if (radius[corner] > 0)
						edges.push_back(ShapeEdge::circle(centerX(corner), centerY(corner), radius[corner]));
				}
			}
		};

		Box		outer;
		Box		inner;							// Inner edge of the outline.
		bool	bOutline = false;				// Has an outline, otherwise inner isn't used.

		//____ create() _________________________________________________________

		static std::unique_ptr<Shape> create(const uint16_t* pPayload, const HiColor* pColors)
		{
			using Flags = GfxBackend::RoundedBoxFlags;

			const RoundedBoxPayload box = decodeRoundedBox(pPayload);
			auto has = [&box](Flags flag) { return (box.flags & uint16_t(flag)) != 0; };

			auto pShape = std::make_unique<RoundedBoxShape>();

			// Geometry, from spx to pixels.

			spx radius[4] = { box.radius[0], box.radius[1], box.radius[2], box.radius[3] };
			Util::clampCornerRadii(box.rect.size(), radius);

			BorderSPX outline = { std::max(box.outline.top, 0), std::max(box.outline.right, 0),
								  std::max(box.outline.bottom, 0), std::max(box.outline.left, 0) };

			spx innerRadius[4];
			Util::innerCornerRadii(radius, outline, innerRadius);

			const RectSPX& rect = box.rect;
			Box& outer = pShape->outer;
			Box& inner = pShape->inner;

			outer.left = rect.x / 64.0;
			outer.top = rect.y / 64.0;
			outer.right = (int64_t(rect.x) + rect.w) / 64.0;
			outer.bottom = (int64_t(rect.y) + rect.h) / 64.0;

			inner.left = outer.left + outline.left / 64.0;
			inner.top = outer.top + outline.top / 64.0;
			inner.right = outer.right - outline.right / 64.0;
			inner.bottom = outer.bottom - outline.bottom / 64.0;

			for (int i = 0; i < 4; i++)
			{
				outer.radius[i] = radius[i] / 64.0;
				inner.radius[i] = innerRadius[i] / 64.0;
			}

			pShape->bOutline = (outline.top | outline.right | outline.bottom | outline.left) != 0;

			outer.addEdges(pShape->edges);
			if (pShape->bOutline)
				inner.addEdges(pShape->edges);

			// Segments. Colors come first, then the colors of the tints in the same order.

			const HiColor* pTintColors = pColors + (has(Flags::HasFill) ? 1 : 0) + (has(Flags::HasOutline) ? 1 : 0);
			const uint16_t* pTint = box.pTints;

			pShape->segments.resize(2);
			pShape->setupSegment(c_fill, has(Flags::HasFill), pColors, has(Flags::HasFillTint), pTint, pTintColors);
			pShape->setupSegment(c_outline, has(Flags::HasOutline), pColors, has(Flags::HasOutlineTint), pTint, pTintColors);

			return pShape;
		}

		//____ segmentAt() ______________________________________________________

		int segmentAt(double x, double y) const override
		{
			if (!outer.contains(x, y))
				return -1;

			if (bOutline && !inner.contains(x, y))
				return c_outline;

			return c_fill;
		}
	};

	//____ RingShape ______________________________________________________________
	/*
		A ring, pie, donut or disc, see GfxDevice::drawRing().

		It is made of four circles around the center of rect. Circle 0 is the outer edge, the
		largest circle that fits in rect, and circle 3 the inner edge, the edge of the hole.
		The outline lies along the outer edge, between circles 0 and 1, and along the inner
		edge, between circles 2 and 3. Slices lie between circles 1 and 2, the hub inside
		circle 3. Without outline circle 1 is circle 0 and circle 2 is circle 3, and without
		hole circles 2 and 3 have no radius.

		The back segment covers the rest of rect outside circle 0 and the part between circles
		1 and 2 that the slices don't reach.

		A square ring has rect itself as outer edge and rect inset by the outline as the inside
		of the outer outline, in place of circles 0 and 1.
	*/

	class ReferenceBackend::RingShape : public ReferenceBackend::Shape
	{
	public:

		static constexpr int c_back = 0;
		static constexpr int c_outline = 1;
		static constexpr int c_hub = 2;
		static constexpr int c_firstSlice = 3;

		double	left = 0, top = 0, right = 0, bottom = 0;						// Rect.
		double	centerX = 0, centerY = 0;
		double	radius[4] = { 0, 0, 0, 0 };										// Of the circles.

		bool	bSquare = false;
		double	insetLeft = 0, insetTop = 0, insetRight = 0, insetBottom = 0;	// Rect inset by the outline.

		bool	bSlices = false;			// There is room for slices, otherwise it is all outline.
		double	start = 0;					// Where the first slice starts, in turns clockwise from 12 o'clock.
		std::vector<double>	sliceSizes;		// In turns.

		//____ create() _________________________________________________________

		static std::unique_ptr<Shape> create(const uint16_t* pPayload, const HiColor* pColors)
		{
			using Flags = GfxBackend::RingFlags;
			using SliceFlags = GfxBackend::RingSliceFlags;

			const RingPayload ring = decodeRing(pPayload);
			auto has = [&ring](Flags flag) { return (ring.flags & uint16_t(flag)) != 0; };
			auto sliceHas = [&ring](int slice, SliceFlags flag) { return (ring.sliceFlags(slice) & uint16_t(flag)) != 0; };

			auto pShape = std::make_unique<RingShape>();
			RingShape& s = *pShape;

			// Geometry, from spx to pixels.

			s.left = ring.rect.x / 64.0;
			s.top = ring.rect.y / 64.0;
			s.right = (int64_t(ring.rect.x) + ring.rect.w) / 64.0;
			s.bottom = (int64_t(ring.rect.y) + ring.rect.h) / 64.0;

			s.centerX = (s.left + s.right) / 2;
			s.centerY = (s.top + s.bottom) / 2;

			double outerRadius = std::min(s.right - s.left, s.bottom - s.top) / 2;
			double thickness = ring.thickness / 64.0;

			bool bHole = thickness > 0 && thickness < outerRadius;
			double holeRadius = bHole ? outerRadius - thickness : 0.0;
			double outline = std::clamp(ring.outline / 64.0, 0.0, bHole ? thickness / 2 : outerRadius);

			s.radius[0] = outerRadius;
			s.radius[1] = outerRadius - outline;
			s.radius[2] = bHole ? holeRadius + outline : 0.0;
			s.radius[3] = holeRadius;

			s.bSquare = has(Flags::Square);
			s.insetLeft = s.left + outline;
			s.insetTop = s.top + outline;
			s.insetRight = s.right - outline;
			s.insetBottom = s.bottom - outline;

			// Slices need room between the outer outline and the inner one.

			double sliceRadius = s.bSquare ? std::min(s.centerX - s.insetLeft, s.centerY - s.insetTop) : s.radius[1];
			s.bSlices = sliceRadius > s.radius[2];

			s.start = ring.start / double(c_shapeFullTurn);

			for (int i = 0; i < ring.nSlices; i++)
				s.sliceSizes.push_back(std::min(ring.sliceSize(i), c_shapeFullTurn) / double(c_shapeFullTurn));

			// Edges: sides of rect, the circles, the inset rect of a square ring and the boundaries
			// between slices, as lines through the center.

			s.edges.push_back(ShapeEdge::line(s.left, s.top, 1, 0));
			s.edges.push_back(ShapeEdge::line(s.left, s.bottom, 1, 0));
			s.edges.push_back(ShapeEdge::line(s.left, s.top, 0, 1));
			s.edges.push_back(ShapeEdge::line(s.right, s.top, 0, 1));

			for (int i = s.bSquare ? 2 : 0; i < 4; i++)
			{
				if (s.radius[i] > 0)
					s.edges.push_back(ShapeEdge::circle(s.centerX, s.centerY, s.radius[i]));
			}

			if (s.bSquare)
			{
				s.edges.push_back(ShapeEdge::line(s.insetLeft, s.insetTop, 1, 0));
				s.edges.push_back(ShapeEdge::line(s.insetLeft, s.insetBottom, 1, 0));
				s.edges.push_back(ShapeEdge::line(s.insetLeft, s.insetTop, 0, 1));
				s.edges.push_back(ShapeEdge::line(s.insetRight, s.insetTop, 0, 1));
			}

			auto addBoundary = [&s](double position)
			{
				double angle = (s.start + position) * 2 * c_pi;
				s.edges.push_back(ShapeEdge::line(s.centerX, s.centerY, std::sin(angle), -std::cos(angle)));
			};

			addBoundary(0.0);

			double end = 0.0;
			for (double size : s.sliceSizes)
			{
				end += size;
				if (end >= 1.0)
					break;

				addBoundary(end);
			}

			// Segments. Colors come first, then the colors of the tints in the same order:
			// outline, hub, back and the slices.

			const HiColor* pTintColors = pColors + (has(Flags::HasOutline) ? 1 : 0) + (has(Flags::HasHub) ? 1 : 0) + (has(Flags::HasBack) ? 1 : 0);
			for (int i = 0; i < ring.nSlices; i++)
				if (sliceHas(i, SliceFlags::Drawn))
					pTintColors++;

			const uint16_t* pTint = ring.pTints;

			s.segments.resize(c_firstSlice + ring.nSlices);
			s.setupSegment(c_outline, has(Flags::HasOutline), pColors, has(Flags::HasOutlineTint), pTint, pTintColors);
			s.setupSegment(c_hub, has(Flags::HasHub), pColors, has(Flags::HasHubTint), pTint, pTintColors);
			s.setupSegment(c_back, has(Flags::HasBack), pColors, has(Flags::HasBackTint), pTint, pTintColors);

			for (int i = 0; i < ring.nSlices; i++)
				s.setupSegment(c_firstSlice + i, sliceHas(i, SliceFlags::Drawn), pColors, sliceHas(i, SliceFlags::Tinted), pTint, pTintColors);

			return pShape;
		}

		//____ segmentAt() ______________________________________________________

		int segmentAt(double x, double y) const override
		{
			if (x < left || x >= right || y < top || y >= bottom)
				return -1;

			double dx = x - centerX;
			double dy = y - centerY;
			double distance = std::sqrt(dx * dx + dy * dy);

			bool bInsideOuterEdge = bSquare || distance < radius[0];
			bool bInsideOuterOutline = bSquare ? (x >= insetLeft && x < insetRight && y >= insetTop && y < insetBottom) : distance < radius[1];

			if (!bInsideOuterEdge)
				return c_back;

			if (!bInsideOuterOutline)
				return c_outline;

			if (distance < radius[3])
				return c_hub;

			if (distance < radius[2] || !bSlices)
				return c_outline;

			return sliceAt(dx, dy);
		}

		//____ sliceAt() ________________________________________________________
		// The slice in direction dx, dy from the center, or the back segment in the gap after the slices.

		int sliceAt(double dx, double dy) const
		{
			// Angle clockwise from 12 o'clock with y pointing down, in turns. The center itself
			// counts as 12 o'clock.

			double angle = (dx == 0.0 && dy == 0.0) ? 0.0 : std::atan2(dx, -dy) / (2 * c_pi);

			double position = angle - start;
			position -= std::floor(position);

			double end = 0.0;
			for (int i = 0; i < (int) sliceSizes.size(); i++)
			{
				end += sliceSizes[i];
				if (position < end)
					return c_firstSlice + i;
			}

			return c_back;
		}
	};

	//____ LineShape ______________________________________________________________
	/*
		A line with caps, see LineCap. Its endpoints are exact positions on the canvas, like the
		corners of a rectangle given to fill(), not rounded to whole pixels as for lines without caps.

		The line is the band of points within half the thickness of the infinite line through begin
		and end, cut by a region for each end. Seen from an end, with 'along' measured from the end
		towards the other end and 'across' at right angle to the line:

		  Flat        along >= 0.
		  Square      along >= -thickness/2.
		  Round       along >= 0, or within thickness/2 of the end.
		  Vertical    On the side of the other end of the vertical line through the end. Empty if
		              the line has no horizontal extent.
		  Horizontal  The same with a horizontal line. Empty if the line has no vertical extent.
		  Spear       along >= 0 and |across| <= thickness/2 * along / length, or along >= length,
		              where length is the spear length.
		  None        Vertical if the line runs mostly along x (more along x than y), otherwise
		              Horizontal.

		A line of zero length runs along x.
	*/

	class ReferenceBackend::LineShape : public ReferenceBackend::Shape
	{
	public:

		struct End
		{
			LineCapShape	shape;
			double			x, y;				// The endpoint.
			double			dirX, dirY;			// Towards the other end, of length 1.
			double			spearLength;		// Pixels.

			bool contains(double px, double py, double along, double across, double r) const
			{
				switch (shape)
				{
					case LineCapShape::Flat:		return along >= 0.0;
					case LineCapShape::Square:		return along >= -r;
					case LineCapShape::Round:		return along >= 0.0 || std::hypot(px - x, py - y) <= r;
					case LineCapShape::Vertical:	return dirX != 0.0 && (px - x) * dirX >= 0.0;
					case LineCapShape::Horizontal:	return dirY != 0.0 && (py - y) * dirY >= 0.0;
					case LineCapShape::Spear:		return along >= spearLength || (along >= 0.0 && std::abs(across) * spearLength <= r * along);
					default:						return false;
				}
			}

			bool isEmpty() const
			{
				return (shape == LineCapShape::Vertical && dirX == 0.0) || (shape == LineCapShape::Horizontal && dirY == 0.0);
			}

			// How far the region reaches beyond the end along the line, within the band.

			double reach(double r) const
			{
				switch (shape)
				{
					case LineCapShape::Square:
					case LineCapShape::Round:		return r;
					case LineCapShape::Vertical:	return r * std::abs(dirY) / std::abs(dirX);
					case LineCapShape::Horizontal:	return r * std::abs(dirX) / std::abs(dirY);
					default:						return 0.0;
				}
			}

			void addEdges(std::vector<ShapeEdge>& edges, double normalX, double normalY, double r) const
			{
				switch (shape)
				{
					case LineCapShape::Flat:
						edges.push_back(ShapeEdge::line(x, y, normalX, normalY));
						break;
					case LineCapShape::Square:
						edges.push_back(ShapeEdge::line(x - dirX * r, y - dirY * r, normalX, normalY));
						break;
					case LineCapShape::Round:
						edges.push_back(ShapeEdge::line(x, y, normalX, normalY));
						edges.push_back(ShapeEdge::circle(x, y, r));
						break;
					case LineCapShape::Vertical:
						edges.push_back(ShapeEdge::line(x, y, 0, 1));
						break;
					case LineCapShape::Horizontal:
						edges.push_back(ShapeEdge::line(x, y, 1, 0));
						break;
					case LineCapShape::Spear:
						edges.push_back(ShapeEdge::line(x, y, dirX * spearLength + normalX * r, dirY * spearLength + normalY * r));
						edges.push_back(ShapeEdge::line(x, y, dirX * spearLength - normalX * r, dirY * spearLength - normalY * r));
						edges.push_back(ShapeEdge::line(x + dirX * spearLength, y + dirY * spearLength, normalX, normalY));
						break;
					default:
						break;
				}
			}
		};

		End		begin, end;
		double	dirX, dirY;						// From begin to end, of length 1.
		double	normalX, normalY;
		double	length;
		double	r;								// Half the thickness.

		LineShape(CoordSPX _begin, CoordSPX _end, spx thickness, LineCaps caps, const Color& color)
		{
			double dx = (_end.x - _begin.x) / 64.0;
			double dy = (_end.y - _begin.y) / 64.0;

			length = std::sqrt(dx * dx + dy * dy);
			dirX = length > 0.0 ? dx / length : 1.0;
			dirY = length > 0.0 ? dy / length : 0.0;
			normalX = -dirY;
			normalY = dirX;
			r = thickness / 128.0;

			bool bAlongX = length == 0.0 || std::abs(_end.x - _begin.x) > std::abs(_end.y - _begin.y);

			auto setupEnd = [&](End& e, LineCap cap, CoordSPX pos, double towardsX, double towardsY)
			{
				e.shape = cap.shape == LineCapShape::None ? (bAlongX ? LineCapShape::Vertical : LineCapShape::Horizontal) : cap.shape;
				e.x = pos.x / 64.0;
				e.y = pos.y / 64.0;
				e.dirX = towardsX;
				e.dirY = towardsY;
				e.spearLength = cap.spearLength / 100.0 * thickness / 64.0;
			};

			setupEnd(begin, caps.begin, _begin, dirX, dirY);
			setupEnd(end, caps.end, _end, -dirX, -dirY);

			segments.resize(1);
			segments[0].bDrawn = true;
			segments[0].color = color;

			edges.push_back(ShapeEdge::line(begin.x + normalX * r, begin.y + normalY * r, dirX, dirY));
			edges.push_back(ShapeEdge::line(begin.x - normalX * r, begin.y - normalY * r, dirX, dirY));
			begin.addEdges(edges, normalX, normalY, r);
			end.addEdges(edges, normalX, normalY, r);
		}

		bool isEmpty() const
		{
			return r <= 0.0 || begin.isEmpty() || end.isEmpty();
		}

		// Pixels that might be touched, clipped to clip.

		RectI bounds(const RectI& clip) const
		{
			double left = DBL_MAX, top = DBL_MAX, right = -DBL_MAX, bottom = -DBL_MAX;

			for (const End* pEnd : { &begin, &end })
			{
				double reach = std::min(pEnd->reach(r), 1e6);
				for (double side : { -r, r })
				{
					double x = pEnd->x - pEnd->dirX * reach + normalX * side;
					double y = pEnd->y - pEnd->dirY * reach + normalY * side;

					left = std::min(left, x);
					top = std::min(top, y);
					right = std::max(right, x);
					bottom = std::max(bottom, y);
				}
			}

			int x1 = std::max(clip.x, int(std::floor(std::max(left, double(clip.x)))));
			int y1 = std::max(clip.y, int(std::floor(std::max(top, double(clip.y)))));
			int x2 = std::min(clip.x + clip.w, int(std::ceil(std::min(right, double(clip.x + clip.w)))));
			int y2 = std::min(clip.y + clip.h, int(std::ceil(std::min(bottom, double(clip.y + clip.h)))));

			return RectI(x1, y1, std::max(0, x2 - x1), std::max(0, y2 - y1));
		}

		int segmentAt(double x, double y) const override
		{
			double along = (x - begin.x) * dirX + (y - begin.y) * dirY;
			double across = (x - begin.x) * normalX + (y - begin.y) * normalY;

			if (std::abs(across) > r)
				return -1;

			if (begin.contains(x, y, along, across, r) && end.contains(x, y, length - along, across, r))
				return 0;

			return -1;
		}
	};

	//____ _drawCappedLine() ______________________________________________________
	/*
		Draws a line with caps, see LineShape, within the clip rectangles.
	*/

	void ReferenceBackend::_drawCappedLine(CoordSPX begin, CoordSPX end, spx thickness, LineCaps caps, const Color& color, int nClipRects, const RectSPX* pClipRects)
	{
		if (m_blendMode == BlendMode::Ignore)
			return;

		LineShape shape(begin, end, thickness, caps, color);

		if (shape.isEmpty())
			return;

		for (int i = 0; i < nClipRects; i++)
		{
			RectI pixels = shape.bounds(_pixelRect(pClipRects[i]));

			for (int y = pixels.y; y < pixels.y + pixels.h; y++)
				for (int x = pixels.x; x < pixels.x + pixels.w; x++)
					_drawShapePixel(shape, x, y);
		}
	}

	//____ _drawShape() ___________________________________________________________
	/*
		Draws a shape from the payload of a DrawShape command, within the next nRects clip
		rectangles. Its colors are the next ones in the color buffer.
	*/

	void ReferenceBackend::_drawShape(ShapeKind kind, const uint16_t* pPayload, int nRects)
	{
		if (m_blendMode == BlendMode::Ignore)
			return;

		std::unique_ptr<Shape> pShape;

		if (kind == ShapeKind::RoundedBox)
			pShape = RoundedBoxShape::create(pPayload, m_pColorsPtr);
		else if (kind == ShapeKind::Ring)
			pShape = RingShape::create(pPayload, m_pColorsPtr);
		else
			return;

		for (int i = 0; i < nRects; i++)
		{
			RectI pixels = _pixelRect(m_pRectsPtr[i]);

			for (int y = pixels.y; y < pixels.y + pixels.h; y++)
				for (int x = pixels.x; x < pixels.x + pixels.w; x++)
					_drawShapePixel(*pShape, x, y);
		}
	}

	//____ _drawShapePixel() ______________________________________________________
	/*
		Draws a pixel of a shape. With a paint mode the pixel gets the average of what each segment
		gives the part of the pixel it covers and the original pixel for the part not covered by any
		drawn segment, weighted by their areas and averaged as premultiplied colors. Effect modes
		apply the drawn segments together, see _effectSource().
	*/

	void ReferenceBackend::_drawShapePixel(const Shape& shape, int x, int y)
	{
		if (!_isInsideCanvas(x, y))
			return;

		std::vector<double> coverage;
		_shapeCoverage(shape, x, y, coverage);

		Color dst = _canvasPixel(x, y);

		std::vector<Color> sources;
		std::vector<double> sourceAreas;

		for (int seg = 0; seg < (int) shape.segments.size(); seg++)
		{
			const Shape::Segment& segment = shape.segments[seg];

			if (!segment.bDrawn || coverage[seg] <= 0.0)
				continue;

			Color color = segment.color;
			if (segment.tint.nLayers > 0)
				color = color * _tintColorAt(segment.tint, x + 0.5, y + 0.5);

			sources.push_back(color * _deviceTint(x, y));
			sourceAreas.push_back(coverage[seg]);
		}

		if (sources.empty())
			return;													// Not covered by any drawn segment.

		if (_isEffectMode())
		{
			_setCanvasPixel(x, y, _blend(_effectSource(int(sources.size()), sources.data(), sourceAreas.data()), dst));
			return;
		}

		std::vector<Color> colors = { dst };
		std::vector<double> areas = { 1.0 };

		for (size_t i = 0; i < sources.size(); i++)
		{
			colors.push_back(_blend(sources[i], dst));
			areas.push_back(sourceAreas[i]);
			areas[0] -= sourceAreas[i];
		}

		// A pixel fully covered by one segment gets what it gives, as in _plotPixel().

		if (colors.size() == 2 && areas[1] == 1.0)
		{
			_setCanvasPixel(x, y, colors[1]);
			return;
		}

		areas[0] = std::max(areas[0], 0.0);
		_setCanvasPixel(x, y, _averageColors(int(colors.size()), colors.data(), areas.data(), true));
	}

	//____ _shapeCoverage() _______________________________________________________
	/*
		How much of the pixel at x, y each segment of a shape covers, 0.0 -> 1.0.

		The area of a segment within the pixel is the integral across the pixel of length(u), how
		much of the vertical line at u within the pixel lies in the segment. length(u) is exact:
		along the line the segment can only change where an edge crosses it, so we cut the line at
		those crossings and ask the shape which segment each piece is in.

		length(u) is smooth, made of straight lines and circular arcs, except where an edge enters
		or leaves the pixel through its top or bottom, where two edges cross, at the left and right
		ends of circles and at vertical edges. We cut the pixel at all those u and integrate each
		part with adaptive Gauss-Legendre quadrature, which converges quickly on smooth functions,
		to an error far below what any canvas can show.
	*/

	// A function of u giving a value for each segment.

	using SegmentFunction = std::function<void(double u, std::vector<double>& values)>;

	// Integral of f from u0 to u1 by the 5-point Gauss-Legendre rule.

	static void _gaussLegendre(const SegmentFunction& f, double u0, double u1, std::vector<double>& result)
	{
		static const double c_nodes[5] = { 0.0, -0.5384693101056831, 0.5384693101056831, -0.9061798459386640, 0.9061798459386640 };
		static const double c_weights[5] = { 0.5688888888888889, 0.4786286704993665, 0.4786286704993665, 0.2369268850561891, 0.2369268850561891 };

		double middle = (u0 + u1) / 2;
		double half = (u1 - u0) / 2;

		std::vector<double> values;
		std::fill(result.begin(), result.end(), 0.0);

		for (int i = 0; i < 5; i++)
		{
			f(middle + c_nodes[i] * half, values);
			for (size_t seg = 0; seg < result.size(); seg++)
				result[seg] += values[seg] * c_weights[i] * half;
		}
	}

	// Integrates f from u0 to u1, adding the result to sum. whole is _gaussLegendre() of the
	// interval, which is compared to the same over its halves. Halves that don't agree with it
	// within tolerance are integrated on their own, until depth runs out.

	static void _integrate(const SegmentFunction& f, double u0, double u1, const std::vector<double>& whole, double tolerance, int depth, std::vector<double>& sum)
	{
		double middle = (u0 + u1) / 2;

		std::vector<double> left(sum.size()), right(sum.size());
		_gaussLegendre(f, u0, middle, left);
		_gaussLegendre(f, middle, u1, right);

		double difference = 0.0;
		for (size_t seg = 0; seg < sum.size(); seg++)
			difference = std::max(difference, std::abs(left[seg] + right[seg] - whole[seg]));

		if (difference <= tolerance || depth == 0)
		{
			for (size_t seg = 0; seg < sum.size(); seg++)
				sum[seg] += left[seg] + right[seg];
		}
		else
		{
			_integrate(f, u0, middle, left, tolerance, depth - 1, sum);
			_integrate(f, middle, u1, right, tolerance, depth - 1, sum);
		}
	}

	void ReferenceBackend::_shapeCoverage(const Shape& shape, int x, int y, std::vector<double>& coverage)
	{
		const double top = y;
		const double bottom = y + 1;

		// Only edges that pass through the pixel matter here.

		std::vector<const ShapeEdge*> edges;
		for (const ShapeEdge& edge : shape.edges)
		{
			if (edge.passesThrough(x, y))
				edges.push_back(&edge);
		}

		// A pixel no edge passes through is all in one segment.

		int nSegments = int(shape.segments.size());
		coverage.assign(nSegments, 0.0);

		if (edges.empty())
		{
			int segment = shape.segmentAt(x + 0.5, y + 0.5);
			if (segment >= 0)
				coverage[segment] = 1.0;
			return;
		}

		// Where across the pixel length(u) might not be smooth.

		std::vector<double> cuts;

		for (size_t i = 0; i < edges.size(); i++)
		{
			const ShapeEdge& edge = *edges[i];

			if (edge.bCircle)
			{
				cuts.push_back(edge.x - edge.radius);
				cuts.push_back(edge.x + edge.radius);
			}
			else if (edge.dirX == 0.0)
				cuts.push_back(edge.x);

			edge.crossingsAtY(top, cuts);
			edge.crossingsAtY(bottom, cuts);

			for (size_t j = i + 1; j < edges.size(); j++)
				edge.intersections(*edges[j], cuts);
		}

		cuts.erase(std::remove_if(cuts.begin(), cuts.end(), [x](double u) { return u <= x || u >= x + 1; }), cuts.end());
		cuts.push_back(x);
		cuts.push_back(x + 1);
		std::sort(cuts.begin(), cuts.end());

		// length(u) for each segment.

		auto lengths = [&](double u, std::vector<double>& result)
		{
			std::vector<double> crossings;
			for (const ShapeEdge* pEdge : edges)
				pEdge->crossingsAtX(u, crossings);

			crossings.erase(std::remove_if(crossings.begin(), crossings.end(), [=](double v) { return v <= top || v >= bottom; }), crossings.end());
			crossings.push_back(top);
			crossings.push_back(bottom);
			std::sort(crossings.begin(), crossings.end());

			result.assign(nSegments, 0.0);

			for (size_t i = 1; i < crossings.size(); i++)
			{
				int segment = shape.segmentAt(u, (crossings[i - 1] + crossings[i]) / 2);
				if (segment >= 0)
					result[segment] += crossings[i] - crossings[i - 1];
			}
		};

		// Integrate across the pixel, part by part.

		std::vector<double> whole(nSegments);

		for (size_t i = 1; i < cuts.size(); i++)
		{
			if (cuts[i] - cuts[i - 1] < 1e-12)
				continue;

			_gaussLegendre(lengths, cuts[i - 1], cuts[i], whole);
			_integrate(lengths, cuts[i - 1], cuts[i], whole, 1e-11, 40, coverage);
		}

		// Rounding errors aside, a segment covering all or nothing of the pixel covers exactly that.

		for (double& value : coverage)
		{
			if (value < 1e-9)
				value = 0.0;
			else if (value > 1.0 - 1e-9)
				value = 1.0;
		}
	}

	//____ defineCanvas() _____________________________________________

	bool ReferenceBackend::defineCanvas(CanvasRef ref, SoftSurface* pSurface)
	{
		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](CanvasInfo& entry) { return (ref == entry.ref); });

		if (it != m_definedCanvases.end())
			m_definedCanvases.erase(it);

		if (pSurface)
			m_definedCanvases.push_back(CanvasInfo(ref, pSurface, pSurface->pixelSize() * 64, pSurface->pixelFormat(), pSurface->scale(),
												   pSurface->colorSpace(), pSurface->isBigEndian()));
		return true;
	}

	//____ canvasInfo() _____________________________________________

	const CanvasInfo* ReferenceBackend::canvasInfo(CanvasRef ref) const
	{
		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](const CanvasInfo& entry) { return (ref == entry.ref); });

		if (it != m_definedCanvases.end())
			return &*it;

		return nullptr;
	}

	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p ReferenceBackend::surfaceFactory()
	{
		if (!m_pSurfaceFactory)
			m_pSurfaceFactory = SoftSurfaceFactory::create();

		return m_pSurfaceFactory;
	}

	//____ edgemapFactory() ___________________________________________________

	EdgemapFactory_p ReferenceBackend::edgemapFactory()
	{
		if (!m_pEdgemapFactory)
			m_pEdgemapFactory = SoftEdgemapFactory::create();

		return m_pEdgemapFactory;
	}

	//____ warpmapFactory() ___________________________________________________

	WarpmapFactory_p ReferenceBackend::warpmapFactory()
	{
		if (!m_pWarpmapFactory)
			m_pWarpmapFactory = SoftWarpmapFactory::create();

		return m_pWarpmapFactory;
	}

	//____ maxEdges() _____________________________________________

	int ReferenceBackend::maxEdges() const
	{
		return Edgemap::maxSegments - 1;
	}

	//____ canBeBlitSource() ___________________________________________

	bool ReferenceBackend::canBeBlitSource(const TypeInfo& type) const
	{
		return (type == SoftSurface::TYPEINFO);
	}

	//____ canBeCanvas() ______________________________________________

	bool ReferenceBackend::canBeCanvas(const TypeInfo& type) const
	{
		// Any surface that can give us a pixel buffer of a format we know.

		return true;
	}

	//____ waitForCompletion() ___________________________________________________

	void ReferenceBackend::waitForCompletion()
	{
	}

} // namespace wg
