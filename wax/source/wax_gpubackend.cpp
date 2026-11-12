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

#include <wax_gpubackend.h>
#include <wax_gpuedgemap.h>
#include <wax_warpmap.h>
#include <wax_base.h>
#include <wax_util.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace wax
{
	const TypeInfo GpuBackend::TYPEINFO = { "GpuBackend", &Backend::TYPEINFO };

	//____ constructor ___________________________________________________________

	GpuBackend::GpuBackend()
	{
	}

	//____ destructor ____________________________________________________________

	GpuBackend::~GpuBackend()
	{
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& GpuBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxEdges() ____________________________________________________________

	int GpuBackend::maxEdges() const
	{
		return c_maxSegments - 1;
	}

	//____ _hasPartialCoverage() _________________________________________________

	bool GpuBackend::_hasPartialCoverage( PipelineKind kind )
	{
		return kind == PipelineKind::FillAA || kind == PipelineKind::Line || kind == PipelineKind::ShapeBox || kind == PipelineKind::ShapeRing || kind == PipelineKind::ShapeLine || _isWarp(kind);
	}

	//____ _isWarp() _____________________________________________________________

	bool GpuBackend::_isWarp( PipelineKind kind )
	{
		return kind == PipelineKind::WarpEdgemap || kind == PipelineKind::WarpBlit || kind == PipelineKind::WarpPaletteBlit;
	}

	//____ _canvasKind() _________________________________________________________

	GpuBackend::CanvasKind GpuBackend::_canvasKind( const Surface * pSurface )
	{
		if( pSurface->pixelFormat() == PixelFormat::Alpha_8 )
			return CanvasKind::AlphaOnly;

		auto pDesc = pSurface->pixelDescription();

		if( pDesc->type != PixelType::Index && pDesc->A_mask == 0 )
			return CanvasKind::WithoutAlpha;

		return CanvasKind::WithAlpha;
	}

	//____ _isOfType() ___________________________________________________________
	//
	// TypeInfo compares by address, so a plain == would turn away anything derived from a type.
	// Walk the chain instead.

	bool GpuBackend::_isOfType(const TypeInfo& type, const TypeInfo& base)
	{
		for (const TypeInfo* pType = &type; pType; pType = pType->pSuperClass)
		{
			if (pType == &base)
				return true;
		}

		return false;
	}

	//____ _reportBufferFull() ___________________________________________________

	void GpuBackend::_reportBufferFull( const char * pBuffer, const char * pWhat, bool& bReported )
	{
		if( bReported || !_blameSessionInfo() )
			return;

		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%s buffer full, %s dropped. SessionInfo underestimated what the session needs.", pBuffer, pWhat);
		Base::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, buffer, this, &typeInfo(), __func__, __FILE__, __LINE__);
		bReported = true;
	}

	//____ beginSession() ________________________________________________________

	void GpuBackend::beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo)
	{
		Surface * pCanvasSurface = nullptr;

		if (pCanvas)
		{
			if (_isNativeSurface(pCanvas))
				pCanvasSurface = pCanvas;
			else
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Canvas is not a surface of this backend, rendering to default canvas instead.",
					this, &typeInfo(), __func__, __FILE__, __LINE__);
		}

		if (!_isRecording())
			return;					// Nothing can be recorded: device lost, beginRender() never called or recording failed.

		m_bInSession = true;

		_beginSessionOnDevice(pCanvasSurface);

		// Sessions start with default state, changes arrive as StateChange commands.

		_resetStates();

		// Room for everything the session may write.

		_reserveBuffers(pInfo);

		// We must not touch a single pixel outside the update rects, the window may present only
		// those. The rects we get for the draw commands are already clipped against them by
		// DeviceGen2, so no extra clipping is needed here.

		m_pActiveCanvas = nullptr;
		_setCanvas(pCanvasSurface);
	}

	//____ endSession() __________________________________________________________

	void GpuBackend::endSession()
	{
		if (!m_bInSession)
			return;

		_checkSessionUsage();

		_endSessionOnDevice();

		m_pActiveCanvas = nullptr;
		m_bCanvasBound = false;
		m_pBlitSource = nullptr;
		m_bInSession = false;
	}

	//____ setCanvas() ___________________________________________________________
	//
	// Called between processCommands() within a session, when a render layer has a canvas of its
	// own.

	void GpuBackend::setCanvas(Surface* pSurface)
	{
		if (pSurface && !_isNativeSurface(pSurface))
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Canvas is not a surface of this backend.",
				this, &typeInfo(), __func__, __FILE__, __LINE__);
			return;
		}

		if (!m_bInSession || !_isRecording())
			return;					// Nothing to render into it with, see processCommands().

		_resetStates();
		_setCanvas(pSurface);
	}

	void GpuBackend::setCanvas(CanvasRef ref)
	{
		if (ref == CanvasRef::Default)
		{
			if (m_bInSession && _isRecording())
			{
				_resetStates();
				_setCanvas(nullptr);
			}
		}
		else
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only CanvasRef::Default is supported.",
				this, &typeInfo(), __func__, __FILE__, __LINE__);
	}

	//____ _setCanvas() __________________________________________________________

	void GpuBackend::_setCanvas(Surface* pCanvas)
	{
		// We can't read from what we paint into.

		if (pCanvas && pCanvas == m_pBlitSource)
			_setBlitSource(nullptr);

		m_pActiveCanvas = pCanvas;

		SizeI size;
		CanvasKind kind = CanvasKind::WithAlpha;

		m_bCanvasBound = _bindCanvas(pCanvas, size, kind);

		if (!m_bCanvasBound)
		{
			size = SizeI();
			kind = CanvasKind::WithAlpha;
		}

		m_activeCanvasSize = size;
		m_activeCanvasKind = kind;

		// Vertex positions are in canvas pixels, the vertex shaders need the canvas size to bring
		// them into clip space.

		m_canvasConstants.canvasScale[0] = size.w > 0 ? 2.f / size.w : 0.f;
		m_canvasConstants.canvasScale[1] = size.h > 0 ? 2.f / size.h : 0.f;

		_setConstants(ConstantBlock::Canvas);

		m_bBlitSourceBound = false;
		_updatePixelInfo();				// The pixel shaders write differently to canvases without alpha.
	}

	//____ _resetStates() ________________________________________________________
	//
	// States start over with each session and each canvas, as DeviceGen2 sets them again for it,
	// like SoftBackend does. It encodes a state change only when a value differs from these
	// defaults, so getting them out of step means drawing with the values of the previous session
	// or canvas.

	void GpuBackend::_resetStates()
	{
		m_tintColor = HiColor::White;
		m_pixelConstants.tintOfs = -1;
		m_activeBlendMode = BlendMode::Blend;
		m_morphFactor = 0.5f;

		// No brush until a StateChange says otherwise. Should one ever be missing, this makes a
		// blur come out as a plain blit rather than as something built from whatever was left
		// behind.

		m_blurRadius = 0;

		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 4; j++)
				m_blurColorMtx[i][j] = 0.f;

		m_blurColorMtx[4][0] = 1.f;
		m_blurColorMtx[4][1] = 1.f;
		m_blurColorMtx[4][2] = 1.f;
		m_blurColorMtx[4][3] = 1.f;

		_setBlitSource(nullptr);
	}

	//____ setObjects() __________________________________________________________

	void GpuBackend::setObjects(Object* const* pBeg, Object* const* pEnd)
	{
		m_pObjectsBeg = pBeg;
		m_pObjectsEnd = pEnd;
	}

	//____ setRects() ____________________________________________________________

	void GpuBackend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
		m_pRectsBeg = pBeg;
		m_pRectsEnd = pEnd;
		m_pRectsPtr = pBeg;
	}

	//____ setColors() ___________________________________________________________

	void GpuBackend::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
		m_pColorsBeg = pBeg;
		m_pColorsEnd = pEnd;
		m_pColorsPtr = pBeg;
	}

	//____ setTransforms() _______________________________________________________

	void GpuBackend::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
		m_pTransformsBeg = pBeg;
		m_pTransformsEnd = pEnd;
	}

	//____ _object() _____________________________________________________________

	Object* GpuBackend::_object(uint32_t index) const
	{
		return (m_pObjectsBeg && index < uint32_t(m_pObjectsEnd - m_pObjectsBeg)) ? m_pObjectsBeg[index] : nullptr;
	}

	//____ processCommands() _____________________________________________________

	void GpuBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version)
	{
		// Nothing can be recorded: the device is lost, beginRender() was never called, or
		// recording broke. Every draw would fail on its own and many would say so, so the commands
		// are dropped here, quietly. The streams are set again before the next call, so there is
		// no need to step through them.

		if (!m_bInSession || !_isRecording())
			return;

		const RectSPX*	pRects = m_pRectsPtr;
		const HiColor*	pColors = m_pColorsPtr;

		auto p = pBeg;
		while (p < pEnd)
		{
			auto cmd = Command(*p++);
			switch (cmd)
			{
				case Command::None:
					break;

				case Command::StateChange:
				{
					GpuTools::StateChangeParams state;
					p = GpuTools::decodeStateChange(p, pColors, state);

					_applyStateChange(state);
					break;
				}

				case Command::Fill:
				{
					int32_t nRects = *p++;
					HiColor col = *pColors++;

					_drawFillRects(pRects, nRects, col);
					pRects += nRects;
					break;
				}

				case Command::Blit:
				case Command::ClipBlit:
				case Command::Tile:
				case Command::Blur:
				{
					// All four carry the same payload and cover the same geometry. Whether the
					// source is clamped or tiled is a property of the surface, and the rects have
					// already been clipped by DeviceGen2, so only a blur draws differently - it
					// reads nine texels per pixel instead of one.

					int32_t nRects = *p++;

					_drawBlitRects(p, pRects, nRects, version,
								   cmd == Command::Blur ? PipelineKind::Blur : PipelineKind::Blit, cmd == Command::ClipBlit);
					break;
				}

				case Command::Line:
				{
					int32_t nClipRects = *p++;
					int32_t nLines = *p++;
					p++;								// padding

					_drawLines(p, pRects, pColors, nClipRects, nLines);
					break;
				}

				case Command::DrawShape:
				{
					auto kind = ShapeKind(p[0]);
					int nRects = p[1];
					int nColors = p[2];
					int nWords = p[3];
					const uint16_t* pPayload = p + c_drawShapeHeaderWords - 1;

					_drawShape(kind, pPayload, pRects, nRects, pColors);

					p = pPayload + nWords;
					pRects += nRects;
					pColors += nColors;
					break;
				}

				case Command::WarpBlit:
				case Command::WarpDrawEdgemap:
				{
					WarpParams params;
					p = decodeWarp(cmd, p, params);

					_drawWarp(cmd, params, pRects);
					pRects += params.nRects;
					break;
				}

				case Command::DrawEdgemap:
				case Command::TileEdgemap:
				{
					int32_t nRects = *p++;

					_drawEdgemap(p, pRects, nRects, cmd == Command::TileEdgemap);
					break;
				}

				default:
				{
					// Every command we know of is either drawn or stepped over above, so this is a
					// command that didn't exist when this was written. We don't know how big its
					// payload is, so there is no way to find the next one and the rest of the
					// session has to go.

					static bool bReported = false;
					if (!bReported)
					{
						char buffer[128];
						snprintf(buffer, sizeof(buffer), "Command %d is unknown, rest of session dropped.", (int)cmd);
						Base::throwError(ErrorLevel::Error, ErrorCode::Other, buffer, this, &typeInfo(), __func__, __FILE__, __LINE__);
						bReported = true;
					}

					p = pEnd;
					break;
				}
			}
		}

		m_pRectsPtr = pRects;
		m_pColorsPtr = pColors;
	}

	//____ _applyStateChange() ___________________________________________________

	void GpuBackend::_applyStateChange( const GpuTools::StateChangeParams& state )
	{
		if (GpuTools::hasChanged(state, StateChange::BlitSource))
		{
			Object * pObject = _object(state.blitSource);

			_setBlitSource( _isNativeSurface(pObject) ? static_cast<Surface*>(pObject) : nullptr, state.blitSource );
		}

		// Tint color and tint are independent. The tint color goes into every color _addColor()
		// writes, the tint is evaluated per pixel by the pixel shaders and multiplied on top.

		if (GpuTools::hasChanged(state, StateChange::TintColor))
			m_tintColor = state.tintColor;

		if (GpuTools::hasChanged(state, StateChange::Tint))
		{
			if (state.tint.nLayers == 0)
				_clearTint();
			else
				_setTint(state.tint);
		}

		if (GpuTools::hasChanged(state, StateChange::BlendMode))
			m_activeBlendMode = state.blendMode;

		if (GpuTools::hasChanged(state, StateChange::MorphFactor))
			m_morphFactor = GpuTools::morphFactor(state.morphFactor);

		if (state.changed & (uint8_t(StateChange::BlendMode) | uint8_t(StateChange::MorphFactor)))
			_updatePixelInfo();

		// Blending against a known background (FixedBlendColor) is an optimization for software
		// rendering. We render it as an ordinary Blend, which gives the same result, so the color
		// is of no use to us.

		if (GpuTools::hasChanged(state, StateChange::Blur))
		{
			m_blurRadius = state.blurRadius;
			GpuTools::blurColorMatrix(state.pBlurWeights, m_blurColorMtx);
		}
	}

	//____ _drawFillRects() ______________________________________________________
	//
	// A rectangle that lands on whole pixels is just a quad. One that doesn't needs the coverage
	// of the pixels along its edges worked out, which is a different pipeline and an entry in the
	// extras buffer. A single command can hold both kinds, so runs are drawn as the kind changes.

	void GpuBackend::_drawFillRects(const RectSPX* pRects, int nRects, HiColor color)
	{
		if (nRects <= 0 || !m_buffers.pVertexPtr)
			return;

		// One color for the whole command, the vertices only carry its offset.

		int colorOfs = _addColor(color);
		if (colorOfs < 0)
			return;

		PipelineKind kind = PipelineKind::Fill;

		int firstVertex = int(m_buffers.pVertexPtr - m_buffers.pVertexBeg);
		int nRectsInRun = 0;

		for (int i = 0; i < nRects; i++)
		{
			const RectSPX& rect = pRects[i];

			spx x2spx = rect.x + rect.w;
			spx y2spx = rect.y + rect.h;

			PipelineKind rectKind = (((rect.x | rect.y | x2spx | y2spx) & 63) == 0) ? PipelineKind::Fill : PipelineKind::FillAA;

			if (rectKind != kind)
			{
				_drawFillRun(kind, firstVertex, nRectsInRun);

				kind = rectKind;
				firstVertex = int(m_buffers.pVertexPtr - m_buffers.pVertexBeg);
				nRectsInRun = 0;
			}

			if (m_buffers.pVertexEnd - m_buffers.pVertexPtr < 6)
			{
				static bool bReported = false;
				_reportBufferFull("Vertex", "fills", bReported);
				break;
			}

			int extrasOfs = 0;

			float x1, y1, x2, y2;

			if (kind == PipelineKind::Fill)
			{
				x1 = float(rect.x >> 6);
				y1 = float(rect.y >> 6);
				x2 = float(x2spx >> 6);
				y2 = float(y2spx >> 6);
			}
			else
			{
				// Center and radius of the rectangle, in pixels, for the shader.

				float radiusX = rect.w / 128.f;
				float radiusY = rect.h / 128.f;

				Extras centerAndRadius = { rect.x / 64.f + radiusX, rect.y / 64.f + radiusY, radiusX, radiusY };

				extrasOfs = _addExtras(centerAndRadius);
				if (extrasOfs < 0)
					break;

				// The quad has to cover every pixel the rectangle touches, so it reaches out to
				// whole pixels in both directions.

				x1 = float(rect.x >> 6);
				y1 = float(rect.y >> 6);
				x2 = float((x2spx + 63) >> 6);
				y2 = float((y2spx + 63) >> 6);
			}

			const float coords[6][2] = { {x1,y1}, {x2,y1}, {x2,y2},
										 {x1,y1}, {x2,y2}, {x1,y2} };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				Vertex * pV = m_buffers.pVertexPtr++;

				pV->x = coords[vertex][0];
				pV->y = coords[vertex][1];
				pV->colorOfs = (uint32_t) colorOfs;
				pV->extrasOfs = (uint32_t) extrasOfs;
				pV->u = 0.f;
				pV->v = 0.f;
			}

			nRectsInRun++;
		}

		_drawFillRun(kind, firstVertex, nRectsInRun);
	}

	//____ _drawFillRun() ________________________________________________________

	void GpuBackend::_drawFillRun(PipelineKind kind, int firstVertex, int nRects)
	{
		if (nRects <= 0)
			return;

		if (!_setPipeline(kind))
			return;

		_draw(firstVertex, nRects * 6);
	}

	//____ _drawLines() __________________________________________________________
	//
	// Lines are quads that cover everything the line might touch, with the line itself handed to
	// the pixel shader as an offset, a half width and a slope, see GpuTools::lineQuad(). It works
	// out the coverage of each pixel from those, which is what antialiases the edges.

	void GpuBackend::_drawLines(const uint16_t*& pCmd, const RectSPX*& pRects, const HiColor*& pColors, int nClipRects, int nLines)
	{
		bool bDraw = m_buffers.pVertexPtr != nullptr && _setPipeline(PipelineKind::Line);

		const RectSPX* pClipRects = pRects;
		pRects += nClipRects;

		int firstVertex = bDraw ? int(m_buffers.pVertexPtr - m_buffers.pVertexBeg) : 0;
		int nLinesDrawn = 0;

		for (int i = 0; i < nLines; i++)
		{
			// Even when we can't draw we have to step through the command's data, or we lose track
			// of where we are in the command stream.

			HiColor col = *pColors++;

			auto p32 = (const spx*) pCmd;

			CoordSPX beginSpx, endSpx;

			beginSpx.x = *p32++;
			beginSpx.y = *p32++;
			endSpx.x = *p32++;
			endSpx.y = *p32++;

			pCmd = (const uint16_t*) p32;

			spx thicknessSPX = *pCmd++;
			uint16_t capsWord = *pCmd++;

			if (capsWord != 0)
			{
				m_cappedLines.push_back({ beginSpx, endSpx, thicknessSPX, capsWord, col });
				continue;
			}

			if (!bDraw)
				continue;

			if (m_buffers.pVertexEnd - m_buffers.pVertexPtr < 6)
			{
				static bool bReported = false;
				_reportBufferFull("Vertex", "lines", bReported);
				continue;
			}

			GpuTools::LineQuad quad;
			if (!GpuTools::lineQuad(beginSpx, endSpx, thicknessSPX, quad))
				continue;

			Extras lineInfo = { quad.offset, quad.w, quad.slope, quad.bSteep ? 1.f : 0.f };

			int extrasOfs = _addExtras(lineInfo);
			if (extrasOfs < 0)
				continue;

			int colorOfs = _addColor(col);
			if (colorOfs < 0)
				continue;

			const int corners[6] = { 0, 1, 2, 0, 2, 3 };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				Vertex * pV = m_buffers.pVertexPtr++;

				pV->x = float(quad.corners[corners[vertex]].x);
				pV->y = float(quad.corners[corners[vertex]].y);
				pV->colorOfs = (uint32_t) colorOfs;
				pV->extrasOfs = (uint32_t) extrasOfs;
				pV->u = 0.f;
				pV->v = 0.f;
			}

			nLinesDrawn++;
		}

		// The clip rects are ours to apply. Unlike fills and blits, the rects of a line command
		// are not the geometry, they are what it may be drawn inside, so the same lines are drawn
		// once per rect with the scissor set to it.

		if (nLinesDrawn > 0)
			_drawScissored(pClipRects, nClipRects, firstVertex, nLinesDrawn * 6);

		if (!m_cappedLines.empty())
		{
			_drawCappedLines(pClipRects, nClipRects);
			m_cappedLines.clear();
		}
	}

	//____ _drawScissored() ______________________________________________________
	//
	// Draws vertices once for each clip rectangle, with the scissor set to it, then puts the
	// scissor back to the whole canvas.

	void GpuBackend::_drawScissored(const RectSPX* pClipRects, int nClipRects, int firstVertex, int nVertices)
	{
		for (int i = 0; i < nClipRects; i++)
		{
			const RectSPX& clip = pClipRects[i];

			RectI rect;
			rect.x = clip.x >> 6;
			rect.y = clip.y >> 6;
			rect.w = ((clip.x + clip.w) >> 6) - rect.x;
			rect.h = ((clip.y + clip.h) >> 6) - rect.y;

			_setScissor(rect);
			_draw(firstVertex, nVertices);
		}

		_resetScissor();
	}

	//____ _drawCappedLines() ____________________________________________________
	//
	// Lines with caps, drawn by the shape shaders in a quad around each, see
	// GpuShapes::writeLine(). As for shapes, their colors are written as given and the tint color
	// comes with the white color of the vertices.

	void GpuBackend::_drawCappedLines(const RectSPX* pClipRects, int nClipRects)
	{
		if (!m_buffers.pVertexPtr || !m_buffers.pColorPtr || !m_buffers.pExtrasPtr || !_setPipeline(PipelineKind::ShapeLine))
			return;

		int colorOfs = _addColor(HiColor::White);
		if (colorOfs < 0)
			return;

		int firstVertex = int(m_buffers.pVertexPtr - m_buffers.pVertexBeg);
		int nLinesDrawn = 0;

		for (const GpuTools::CappedLine& line : m_cappedLines)
		{
			if (m_buffers.pExtrasEnd - m_buffers.pExtrasPtr < GpuShapes::c_lineEntries || m_buffers.pVertexEnd - m_buffers.pVertexPtr < 6
				|| m_buffers.pColorPtr == m_buffers.pColorEnd)
			{
				static bool bReported = false;
				_reportBufferFull("A", "lines", bReported);
				break;
			}

			Extras* pEntries = m_buffers.pExtrasPtr;
			CoordF quad[4];

			int nEntries = GpuShapes::writeLine(line.begin, line.end, line.thickness, decodeLineCaps(line.caps), (float*) pEntries, quad);
			if (nEntries == 0)
				continue;

			pEntries[0].w = float(m_buffers.pColorPtr - m_buffers.pColorBeg);
			line.color.toLinearFloat(&m_buffers.pColorPtr->r);
			m_buffers.pColorPtr++;

			int extrasOfs = int(pEntries - m_buffers.pExtrasBeg);
			m_buffers.pExtrasPtr += nEntries;

			const int corners[6] = { 0, 1, 2, 0, 2, 3 };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				const CoordF& corner = quad[corners[vertex]];
				Vertex * pV = m_buffers.pVertexPtr++;

				pV->x = corner.x;
				pV->y = corner.y;
				pV->colorOfs = (uint32_t) colorOfs;
				pV->extrasOfs = (uint32_t) extrasOfs;
				pV->u = 0.f;
				pV->v = 0.f;
			}

			nLinesDrawn++;
		}

		if (nLinesDrawn == 0 || !_setPipeline(PipelineKind::ShapeLine))
			return;

		_drawScissored(pClipRects, nClipRects, firstVertex, nLinesDrawn * 6);
	}

	//____ _addColor() ___________________________________________________________

	int GpuBackend::_addColor(HiColor color)
	{
		if (!m_buffers.pColorPtr)
			return -1;

		if (m_buffers.pColorPtr == m_buffers.pColorEnd)
		{
			static bool bReported = false;
			_reportBufferFull("Color", "draws", bReported);
			return -1;
		}

		// Colors are sRGB, but we blend in linear. Tint is a plain multiplication, in linear.

		float col[4], tint[4];
		color.toLinearFloat(col);
		m_tintColor.toLinearFloat(tint);

		Color * pColor = m_buffers.pColorPtr;

		pColor->r = col[0] * tint[0];
		pColor->g = col[1] * tint[1];
		pColor->b = col[2] * tint[2];
		pColor->a = col[3] * tint[3];

		return int(m_buffers.pColorPtr++ - m_buffers.pColorBeg);
	}

	//____ _setTint() ____________________________________________________________
	//
	// Writes the tint block into the color buffer and points the pixel shaders at it. Its stop
	// colors are not multiplied by the tint color, _addColor() has already put that into the colors
	// the tint is multiplied with.

	void GpuBackend::_setTint(const TintTools::DecodedTint& tint)
	{
		int blockSize = TintTools::gpuTintBlockSize(tint);

		if (!m_buffers.pColorPtr || m_buffers.pColorEnd - m_buffers.pColorPtr < blockSize)
		{
			static bool bReported = false;
			_reportBufferFull("Color", "tints", bReported);

			_clearTint();
			return;
		}

		static_assert(sizeof(Color) == 4 * sizeof(float), "Tint blocks are written as float4 entries.");

		m_pixelConstants.tintOfs = int32_t(m_buffers.pColorPtr - m_buffers.pColorBeg);
		TintTools::writeGpuTintBlock(tint, (float*) m_buffers.pColorPtr);
		m_buffers.pColorPtr += blockSize;

		_setConstants(ConstantBlock::Pixel);
	}

	//____ _clearTint() __________________________________________________________

	void GpuBackend::_clearTint()
	{
		if (m_pixelConstants.tintOfs < 0)
			return;

		m_pixelConstants.tintOfs = -1;
		_setConstants(ConstantBlock::Pixel);
	}

	//____ _updatePixelInfo() ____________________________________________________
	//
	// How the pixel shaders hand what they draw to the blender, see output.hlsl. It follows blend
	// mode, morph factor and the kind of canvas.

	void GpuBackend::_updatePixelInfo()
	{
		BlendMode mode = _normalizeBlendMode(m_activeBlendMode);
		CanvasKind canvasKind = m_activeCanvasKind;

		uint32_t outputMode = 0;

		if (mode == BlendMode::Replace || mode == BlendMode::Morph || (mode == BlendMode::Multiply && canvasKind == CanvasKind::AlphaOnly))
			outputMode = 1;
		else if (mode == BlendMode::Min)
			outputMode = 2;
		else if ((mode == BlendMode::Add || mode == BlendMode::Invert || mode == BlendMode::Max) && canvasKind != CanvasKind::AlphaOnly)
			outputMode = 3;

		m_pixelConstants.outputMode = outputMode;
		m_pixelConstants.canvasKind = uint32_t(canvasKind);
		m_pixelConstants.morphFactor = (mode == BlendMode::Morph) ? m_morphFactor : 1.f;

		_setConstants(ConstantBlock::Pixel);
	}

	//____ _addExtras() __________________________________________________________

	int GpuBackend::_addExtras(const Extras& extras)
	{
		if (!m_buffers.pExtrasPtr || m_buffers.pExtrasPtr == m_buffers.pExtrasEnd)
		{
			static bool bReported = false;
			if (m_buffers.pExtrasPtr)
				_reportBufferFull("Extras", "draws", bReported);

			return -1;
		}

		int ofs = int(m_buffers.pExtrasPtr - m_buffers.pExtrasBeg);

		*m_buffers.pExtrasPtr++ = extras;

		return ofs;
	}

	int GpuBackend::_addExtras(const Extras& first, const Extras& second)
	{
		if (!m_buffers.pExtrasPtr)
			return -1;

		if (m_buffers.pExtrasEnd - m_buffers.pExtrasPtr < 2)
		{
			static bool bReported = false;
			_reportBufferFull("Extras", "blits", bReported);
			return -1;
		}

		int ofs = int(m_buffers.pExtrasPtr - m_buffers.pExtrasBeg);

		*m_buffers.pExtrasPtr++ = first;
		*m_buffers.pExtrasPtr++ = second;

		return ofs;
	}

	//____ _addBlurExtras() ______________________________________________________

	int GpuBackend::_addBlurExtras()
	{
		const int nEntries = 18;			// Nine color matrices, then nine offsets.

		if (!m_buffers.pExtrasPtr || m_buffers.pExtrasEnd - m_buffers.pExtrasPtr < nEntries)
		{
			static bool bReported = false;
			if (m_buffers.pExtrasPtr)
				_reportBufferFull("Extras", "blurs", bReported);

			return -1;
		}

		int ofs = int(m_buffers.pExtrasPtr - m_buffers.pExtrasBeg);

		for (int i = 0; i < 9; i++)
			*m_buffers.pExtrasPtr++ = { m_blurColorMtx[i][0], m_blurColorMtx[i][1], m_blurColorMtx[i][2], m_blurColorMtx[i][3] };

		// The offsets are in texture coordinates, so they are measured against the source.

		float offsets[9][2];
		GpuTools::blurOffsets(m_blurRadius, m_blitSourceSize, offsets);

		for (int i = 0; i < 9; i++)
			*m_buffers.pExtrasPtr++ = { offsets[i][0], offsets[i][1], 0.f, 0.f };

		return ofs;
	}

	//____ _setBlitSource() ______________________________________________________

	void GpuBackend::_setBlitSource(Surface* pSurface, uint32_t objectIndex)
	{
		m_pBlitSource = pSurface;
		m_blitSourceObject = objectIndex;
		m_bBlitSourceBound = false;
		m_bBlitSourceIndexed = false;

		if (!pSurface)
			return;

		// Reading from the surface we are painting into can't work, the texture can't be render
		// target and shader resource at the same time.

		if (pSurface == m_pActiveCanvas)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"Can't blit from the surface we are rendering into.", this, &typeInfo(), __func__, __FILE__, __LINE__);
			m_pBlitSource = nullptr;
			return;
		}

		// Make sure the texture holds everything written to the surface.

		_prepareBlitSource(pSurface);

		m_blitSourceSize = pSurface->pixelSize();
		m_bBlitSourceAlphaOnly = (pSurface->pixelFormat() == PixelFormat::Alpha_8);
		m_bBlitSourceIndexed = (pSurface->pixelDescription()->type == PixelType::Index);
	}

	//____ _bindBlitSource() _____________________________________________________
	//
	// Binds the texture of the blit source, its sampler and the constants about it. Only needed
	// when the source has changed or our bindings have been invalidated.

	bool GpuBackend::_bindBlitSource()
	{
		if (!_isRecording() || !m_pBlitSource)
			return false;

		if (m_bBlitSourceBound)
			return true;

		if (!_bindBlitTexture(m_pBlitSource))
			return false;

		// The shaders need the source size to get from pixels to texture coordinates.

		m_canvasConstants.textureSize[0] = (float) m_blitSourceSize.w;
		m_canvasConstants.textureSize[1] = (float) m_blitSourceSize.h;

		_bindBlitSamplerAndFlags();			// Sets the canvas constants.

		m_bBlitSourceBound = true;
		return true;
	}

	//____ _bindBlitSamplerAndFlags() ____________________________________________
	//
	// A ClipBlit must not read outside the source: the transform can put parts of the destination
	// beyond the source's edges, and those should be left alone, not filled with the edge texels a
	// clamping sampler gives. So a clipping draw of a non-tiling source uses a sampler that returns
	// transparent black outside. Tiling sources wrap either way.

	void GpuBackend::_bindBlitSamplerAndFlags()
	{
		bool bClip = m_bBlitClip && !m_pBlitSource->isTiling();

		_bindBlitSampler(m_pBlitSource, bClip);

		// Bits 0 and 4 are for the ordinary shaders. The palette shaders fetch texels themselves, so
		// they need to know what the sampler would have done. A source without alpha (bit 4) reads
		// alpha 1 also from the transparent border, so the ordinary blit shader works out its alpha
		// itself when clipping.

		auto pDesc = m_pBlitSource->pixelDescription();
		bool bNoAlpha = pDesc->type != PixelType::Index && pDesc->A_mask == 0 && !m_bBlitSourceAlphaOnly;

		m_canvasConstants.flags = (m_bBlitSourceAlphaOnly ? 1 : 0) |
								  (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear ? 2 : 0) |
								  (m_pBlitSource->isTiling() ? 4 : 0) |
								  (bClip ? 8 : 0) |
								  (bNoAlpha ? 16 : 0);

		_setConstants(ConstantBlock::Canvas);

		m_bBoundBlitClip = m_bBlitClip;
	}

	//____ _drawBlitRects() ______________________________________________________

	void GpuBackend::_drawBlitRects(const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, int version, PipelineKind kind, bool bClip)
	{
		m_bBlitClip = bClip;

		// Transforms below this index are the standard ones, the rest were handed to us through
		// setTransforms().

		int customTransformStart = (version == 1) ? GfxFlip_size : NbStandardTransforms;

		bool bBilinear = (m_pBlitSource && m_pBlitSource->sampleMethod() == SampleMethod::Bilinear);

		// Custom transforms sample bilinear sources at the center of the texel, but nearest ones at
		// the corner, so that stretched pixels are evenly distributed. A corner on a texel edge
		// belongs to the texel after it, but rounding could tip it into the one before. Corners land
		// exactly on texel edges when an axis aligned stretch has a simple ratio, so those are
		// nudged forward by a fraction of a texel. Rotated ones practically never do, and a nudge
		// would only move samples that weren't on an edge. Blurs always sample the center, so their
		// fractional sample offsets round to the nearest pixel either way. Standard transforms are
		// dealt with per rectangle below.

		bool bCenter = bBilinear || kind == PipelineKind::Blur;

		// Even when we can't draw we have to step through the command's data, or we lose track of
		// where we are in the stream.

		bool bDraw = m_buffers.pVertexPtr != nullptr && _bindBlitSource();

		// The source may be bound for a different kind of blit.

		if (bDraw && m_bBoundBlitClip != m_bBlitClip)
			_bindBlitSamplerAndFlags();

		// A palette based source has shaders of its own.

		if (m_bBlitSourceIndexed)
			kind = (kind == PipelineKind::Blur) ? PipelineKind::PaletteBlur : PipelineKind::PaletteBlit;

		// A blur needs its brush where the pixel shader can reach it. It goes in the extras buffer,
		// which the shader indexes into from a constant. That constant is set again for every
		// blur draw.

		if (bDraw && (kind == PipelineKind::Blur || kind == PipelineKind::PaletteBlur))
		{
			int blurOfs = _addBlurExtras();

			if (blurOfs < 0)
				bDraw = false;
			else
			{
				m_canvasConstants.blurOfs = (uint32_t) blurOfs;
				_setConstants(ConstantBlock::Canvas);
			}
		}

		if (bDraw)
			bDraw = _setPipeline(kind);

		// _addColor() multiplies by the tint, so what a blit wants here is white. Passing the tint
		// would apply it twice.

		int colorOfs = bDraw ? _addColor(HiColor::White) : -1;

		if (colorOfs < 0)
			bDraw = false;

		int firstVertex = bDraw ? int(m_buffers.pVertexPtr - m_buffers.pVertexBeg) : 0;
		int nRectsDrawn = 0;

		for (int i = 0; i < nRects; i++)
		{
			auto p32 = (const spx*) pCmd;

			int		srcX = *p32++;			// Source coordinates are in 1/1024 pixels.
			int		srcY = *p32++;
			spx		dstX = *p32++;
			spx		dstY = *p32++;

			pCmd = (const uint16_t*) p32;

			int32_t transform = *pCmd++;
			pCmd++;							// padding

			const RectSPX& dest = *pRects++;

			if (!bDraw)
				continue;

			if (m_buffers.pVertexEnd - m_buffers.pVertexPtr < 6)
			{
				static bool bReported = false;
				_reportBufferFull("Vertex", "blits", bReported);
				continue;
			}

			// Source and destination origin plus the transform is all the vertex shader needs to
			// work out texture coordinates for each corner.

			auto& mtx = (transform < customTransformStart) ? s_standardTransforms[transform] : m_pTransformsBeg[transform - customTransformStart];

			// The source coordinate is where the corner of the first destination pixel lands.
			// Standard transforms (blits, tiles and blurs, possibly flipped) map pixels 1:1, so they
			// sample the center of each source pixel, half a pixel along the transformed axes,
			// whichever way those point.

			float srcOfsX, srcOfsY;

			if (transform < customTransformStart)
			{
				srcOfsX = (mtx.xx + mtx.yx) * 0.5f;
				srcOfsY = (mtx.xy + mtx.yy) * 0.5f;
			}
			else if (bCenter)
				srcOfsX = srcOfsY = 0.5f;
			else if ((mtx.xy == 0.f && mtx.yx == 0.f) || (mtx.xx == 0.f && mtx.yy == 0.f))
				srcOfsX = srcOfsY = 1.f / 4096;
			else
				srcOfsX = srcOfsY = 0.f;

			Extras srcDst;

			srcDst.x = srcX / 1024.f + srcOfsX;
			srcDst.y = srcY / 1024.f + srcOfsY;
			srcDst.z = float(dstX >> 6) + 0.5f;
			srcDst.w = float(dstY >> 6) + 0.5f;

			Extras matrix = { mtx.xx, mtx.xy, mtx.yx, mtx.yy };

			int extrasOfs = _addExtras(srcDst, matrix);
			if (extrasOfs < 0)
				continue;

			float x1 = float(dest.x >> 6);
			float y1 = float(dest.y >> 6);
			float x2 = x1 + float(dest.w >> 6);
			float y2 = y1 + float(dest.h >> 6);

			const float coords[6][2] = { {x1,y1}, {x2,y1}, {x2,y2},
										 {x1,y1}, {x2,y2}, {x1,y2} };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				Vertex * pV = m_buffers.pVertexPtr++;

				pV->x = coords[vertex][0];
				pV->y = coords[vertex][1];
				pV->colorOfs = (uint32_t) colorOfs;
				pV->extrasOfs = (uint32_t) extrasOfs;
				pV->u = 0.f;
				pV->v = 0.f;
			}

			nRectsDrawn++;
		}

		if (nRectsDrawn > 0)
			_draw(firstVertex, nRectsDrawn * 6);
	}

	//____ _drawEdgemap() ________________________________________________________
	//
	// An edgemap is drawn as one quad per patch, with the work done in the pixel shader: it walks
	// down a column of edges and works out how much of the pixel each segment covers. The vertices
	// carry where in the edgemap each corner sits, which is what lets the same quad be flipped or
	// rotated.

	void GpuBackend::_drawEdgemap(const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, bool bTile)
	{
		int32_t flip = *pCmd++;
		pCmd++;								// padding

		Object * pObject = _object(readObjectIndex(pCmd));

		auto p32 = (const spx*) pCmd;

		spx destX = *p32++;
		spx destY = *p32++;

		pCmd = (const uint16_t*) p32;

		GpuEdgemap * pEdgemap = _isNativeEdgemap(pObject) ? static_cast<GpuEdgemap*>(pObject) : nullptr;

		if (!pEdgemap || !pEdgemap->m_pGpuData || !m_buffers.pVertexPtr || flip < 0 || flip >= GfxFlip_size)
		{
			static bool bReported = false;
			if (!bReported && m_buffers.pVertexPtr)
			{
				const char * pReason = !pEdgemap ? "it is not an edgemap of this backend" :
									   !pEdgemap->m_pGpuData ? "it has no buffer" : "the flip value is out of range";

				char buffer[256];
				snprintf(buffer, sizeof(buffer), "Can't draw edgemap, %s.", pReason);
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &typeInfo(), __func__, __FILE__, __LINE__);
				bReported = true;
			}

			pRects += nRects;
			return;
		}

		int nSegments = pEdgemap->m_nbRenderSegments;

		if (nSegments < 1 || nRects <= 0)
		{
			pRects += nRects;
			return;
		}

		auto& mtx = s_standardTransforms[flip];

		// Where the edgemap lands, before any patch is taken out of it.

		RectSPX destIn = {
			destX,
			destY,
			pEdgemap->m_size.w * 64 * int(std::abs(mtx.xx)) + pEdgemap->m_size.h * 64 * int(std::abs(mtx.yx)),
			pEdgemap->m_size.w * 64 * int(std::abs(mtx.xy)) + pEdgemap->m_size.h * 64 * int(std::abs(mtx.yy))
		};

		RectI dest = Util::roundToPixels(destIn);

		int uIncX = int(mtx.xx);
		int vIncX = int(mtx.xy);
		int uIncY = int(mtx.yx);
		int vIncY = int(mtx.yy);

		// We may have been given room for more columns than the edgemap has.

		int maxCol = pEdgemap->m_size.w;

		if (uIncX != 0)								// Columns run horizontally.
		{
			if (dest.w > maxCol)
			{
				if (uIncX < 0)
					dest.x += dest.w - maxCol;

				dest.w = maxCol;
			}
		}
		else										// Columns run vertically.
		{
			if (dest.h > maxCol)
			{
				if (uIncY < 0)
					dest.y += dest.h - maxCol;

				dest.h = maxCol;
			}
		}

		// Where the top left corner sits in the edgemap, once it has been flipped.

		int uTopLeft = 0;
		int vTopLeft = 0;

		if (uIncX + uIncY < 0)
			uTopLeft = maxCol;

		if (vIncX < 0)
			vTopLeft = dest.w;
		else if (vIncY < 0)
			vTopLeft = dest.h;

		// _addColor() multiplies by the tint, so white is what gets the tint applied exactly once.
		// Passing the tint itself would square it.

		int colorOfs = _addColor(HiColor::White);

		// Where the segment colors are in the edgemap's buffer: flat colors and the table of tint
		// blocks, see GpuEdgemap. Then the size of the edgemap when tiled, in columns and pixels,
		// which the shader wraps positions outside of it by. Zero when not.

		Extras tileSize = { bTile ? float(pEdgemap->m_size.w) : 0.f, bTile ? float(pEdgemap->m_size.h) : 0.f, 0.f, 0.f };

		int extrasOfs = _addExtras({ float(pEdgemap->_flatColorsOfs()), float(pEdgemap->_tintTableOfs()), 0.f, 0.f }, tileSize);

		if (colorOfs < 0 || extrasOfs < 0)
		{
			pRects += nRects;
			return;
		}

		if (m_buffers.pVertexEnd - m_buffers.pVertexPtr < nRects * 6)
		{
			static bool bReported = false;
			_reportBufferFull("Vertex", "edgemaps", bReported);

			pRects += nRects;
			return;
		}

		int firstVertex = int(m_buffers.pVertexPtr - m_buffers.pVertexBeg);

		for (int i = 0; i < nRects; i++)
		{
			const RectSPX& patchSpx = *pRects++;

			RectI patch = Util::roundToPixels(patchSpx);		// Rounded like the position of the edgemap.

			int dx1 = patch.x;
			int dy1 = patch.y;
			int dx2 = patch.x + patch.w;
			int dy2 = patch.y + patch.h;

			// U is which column of the edgemap, V how far down that column. Half a pixel back, so
			// that the shader works from pixel centers.

			float u1 = float(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
			float v1 = float(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

			float u2 = float(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
			float v2 = float(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

			float u3 = float(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
			float v3 = float(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

			float u4 = float(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
			float v4 = float(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

			const float uv[4][2] = { { u1, v1 - 0.5f }, { u2, v2 - 0.5f }, { u3, v3 - 0.5f }, { u4, v4 - 0.5f } };

			const float coords[4][2] = { { float(dx1), float(dy1) }, { float(dx2), float(dy1) },
										 { float(dx2), float(dy2) }, { float(dx1), float(dy2) } };

			const int corners[6] = { 0, 1, 2, 0, 2, 3 };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				int corner = corners[vertex];
				Vertex * pV = m_buffers.pVertexPtr++;

				pV->x = coords[corner][0];
				pV->y = coords[corner][1];
				pV->colorOfs = (uint32_t) colorOfs;
				pV->extrasOfs = (uint32_t) extrasOfs;
				pV->u = uv[corner][0];
				pV->v = uv[corner][1];
			}
		}

		if (!_setPipeline(PipelineKind::Segments))
			return;

		// The edgemap's own buffer, and how to walk it. The number of edges we draw and the number
		// stored per column are not always the same, so the shader is told both.

		if (!_bindEdgemap(pEdgemap))
			return;

		m_bBlitSourceBound = false;			// A palette based blit source keeps its palette where the edgemap goes.

		m_canvasConstants.edgemapEdges = uint32_t(nSegments - 1);
		m_canvasConstants.edgemapPitch = uint32_t(pEdgemap->m_nbSegments - 1);
		_setConstants(ConstantBlock::Canvas);

		_draw(firstVertex, nRects * 6);
	}

	//____ _drawShape() __________________________________________________________
	//
	// A shape, drawn by the shape shaders from the entries GpuShapes makes of it, with the colors
	// and tint blocks of its segments in the color buffer. Its segment colors are written as given,
	// the tint color comes with the white color of the vertices, as for edgemaps.

	void GpuBackend::_drawShape(ShapeKind kind, const uint16_t* pPayload, const RectSPX* pRects, int nRects, const HiColor* pColors)
	{
		int maxEntries, nSegments;
		GpuShapes::measure(kind, pPayload, maxEntries, nSegments);

		if (maxEntries == 0)
		{
			Base::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, "Failed DrawShape operation. Unsupported kind of shape.",
				this, &typeInfo(), __func__, __FILE__, __LINE__);
			return;
		}

		if (!m_buffers.pVertexPtr || !m_buffers.pColorPtr || !m_buffers.pExtrasPtr || !_setPipeline(kind == ShapeKind::Ring ? PipelineKind::ShapeRing : PipelineKind::ShapeBox))
			return;

		static bool bReported = false;

		if (m_buffers.pExtrasEnd - m_buffers.pExtrasPtr < maxEntries)
			return _reportBufferFull("Extras", "shapes", bReported);

		if (m_buffers.pVertexEnd - m_buffers.pVertexPtr < nRects * 6)
			return _reportBufferFull("Vertex", "shapes", bReported);

		// _addColor() multiplies by the tint color, so white is what gets it applied exactly once.

		int colorOfs = _addColor(HiColor::White);
		if (colorOfs < 0)
			return;

		if ((int)m_shapeSegments.size() < nSegments)
			m_shapeSegments.resize(nSegments);

		Extras* pEntries = m_buffers.pExtrasPtr;
		int nEntries = GpuShapes::write(kind, pPayload, pColors, (float*) pEntries, m_shapeSegments.data());

		static_assert(sizeof(Extras) == 4 * sizeof(float), "Shape entries are written as float4.");

		// Colors and tint blocks of drawn segments, their offsets in the entries of the segments.

		for (int i = 0; i < nSegments; i++)
		{
			const GpuShapes::Segment& seg = m_shapeSegments[i];

			if (!seg.bDrawn)
				continue;

			if (m_buffers.pColorPtr == m_buffers.pColorEnd)
				return _reportBufferFull("Color", "shapes", bReported);

			Extras& entry = pEntries[seg.entry];

			entry.x = float(m_buffers.pColorPtr - m_buffers.pColorBeg);
			seg.color.toLinearFloat(&m_buffers.pColorPtr->r);
			m_buffers.pColorPtr++;

			if (seg.pTint)
			{
				TintTools::DecodedTint tint;
				const HiColor* pTintColors = seg.pTintColors;
				TintTools::decodeTint(seg.pTint, pTintColors, tint);

				int blockSize = TintTools::gpuTintBlockSize(tint);
				if (m_buffers.pColorEnd - m_buffers.pColorPtr < blockSize)
					return _reportBufferFull("Color", "shapes", bReported);

				entry.y = float(m_buffers.pColorPtr - m_buffers.pColorBeg);
				TintTools::writeGpuTintBlock(tint, (float*) m_buffers.pColorPtr);
				m_buffers.pColorPtr += blockSize;
			}
		}

		int extrasOfs = int(pEntries - m_buffers.pExtrasBeg);
		m_buffers.pExtrasPtr += nEntries;

		// A quad for each clip rectangle.

		int firstVertex = int(m_buffers.pVertexPtr - m_buffers.pVertexBeg);

		for (int i = 0; i < nRects; i++)
		{
			RectI patch = pRects[i] / 64;

			const float coords[4][2] = { { float(patch.x), float(patch.y) }, { float(patch.x + patch.w), float(patch.y) },
										 { float(patch.x + patch.w), float(patch.y + patch.h) }, { float(patch.x), float(patch.y + patch.h) } };

			const int corners[6] = { 0, 1, 2, 0, 2, 3 };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				int corner = corners[vertex];
				Vertex * pV = m_buffers.pVertexPtr++;

				pV->x = coords[corner][0];
				pV->y = coords[corner][1];
				pV->colorOfs = (uint32_t) colorOfs;
				pV->extrasOfs = (uint32_t) extrasOfs;
				pV->u = 0.f;
				pV->v = 0.f;
			}
		}

		_draw(firstVertex, nRects * 6);
	}

	//____ _drawWarp() ___________________________________________________________
	//
	// A WarpBlit or WarpDrawEdgemap. The triangles of the Warpmap are drawn from its own buffer once
	// for each clip rectangle, with the part of it covered by the Warpmap as scissor, see warp.hlsl.
	// Three extras entries tell the pixel shaders what the Warpmap maps onto, WarpConstants tell the
	// vertex shader where to draw it. pRects are the clip rectangles, which the caller steps past.

	void GpuBackend::_drawWarp(Command cmd, const WarpParams& params, const RectSPX* pRects)
	{
		bool bEdgemap = (cmd == Command::WarpDrawEdgemap);

		Object * pWarpObject = _object(params.warpmap);
		Object * pEdgemapObject = bEdgemap ? _object(params.edgemap) : nullptr;

		Warpmap * pWarpmap = _isNativeWarpmap(pWarpObject) ? static_cast<Warpmap*>(pWarpObject) : nullptr;
		GpuEdgemap * pEdgemap = _isNativeEdgemap(pEdgemapObject) ? static_cast<GpuEdgemap*>(pEdgemapObject) : nullptr;

		if (!pWarpmap || (bEdgemap && (!pEdgemap || !pEdgemap->m_pGpuData)))
		{
			static bool bReported = false;
			if (!bReported)
			{
				const char * pReason = !pWarpmap ? "the warpmap is not one of this backend" :
									   !pEdgemap ? "the edgemap is not one of this backend" : "the edgemap has no buffer";

				char buffer[256];
				snprintf(buffer, sizeof(buffer), "Can't draw warp, %s.", pReason);
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &typeInfo(), __func__, __FILE__, __LINE__);
				bReported = true;
			}

			return;
		}

		if (!m_buffers.pVertexPtr || !m_buffers.pExtrasPtr)
			return;

		// A blit reads around the source rather than clipping to it, the shaders take care of its
		// borders. A palette based source has a shader of its own.

		PipelineKind kind = PipelineKind::WarpEdgemap;

		if (!bEdgemap)
		{
			m_bBlitClip = false;

			if (!_bindBlitSource())
				return;

			if (m_bBoundBlitClip != m_bBlitClip)
				_bindBlitSamplerAndFlags();

			kind = m_bBlitSourceIndexed ? PipelineKind::WarpPaletteBlit : PipelineKind::WarpBlit;
		}

		if (!_setPipeline(kind))
			return;

		if (m_buffers.pExtrasEnd - m_buffers.pExtrasPtr < 3)
		{
			static bool bReported = false;
			return _reportBufferFull("Extras", "warps", bReported);
		}

		// _addColor() multiplies by the tint color, so white is what gets it applied exactly once.

		int colorOfs = _addColor(HiColor::White);
		if (colorOfs < 0)
			return;

		// u and v are taken relative to the first vertex by the vertex shader, which is added back
		// to the source offset, brought into the periods.

		CoordSPX ref;
		if (pWarpmap->vertexCount() > 0)
			ref = { pWarpmap->vertexData()[0].u, pWarpmap->vertexData()[0].v };

		spx uPeriod = pWarpmap->uPeriod(), vPeriod = pWarpmap->vPeriod();
		spx refU = uPeriod > 0 ? ((ref.x % uPeriod) + uPeriod) % uPeriod : ref.x;
		spx refV = vPeriod > 0 ? ((ref.y % vPeriod) + vPeriod) % vPeriod : ref.y;

		// Extras, see warp.hlsl.

		CoordI dest = Util::roundToPixels(params.dest);
		SizeI warpSize = pWarpmap->pixelSize();

		float srcW = pEdgemap ? float(pEdgemap->m_size.w) : params.source.w / 64.f;
		float srcH = pEdgemap ? float(pEdgemap->m_size.h) : params.source.h / 64.f;

		int extrasOfs = int(m_buffers.pExtrasPtr - m_buffers.pExtrasBeg);

		*m_buffers.pExtrasPtr++ = { srcW, srcH, pEdgemap ? float(pEdgemap->m_nbRenderSegments - 1) : 0.f, 0.f };
		*m_buffers.pExtrasPtr++ = { (params.sourceOffset.x + refU) / 64.f, (params.sourceOffset.y + refV) / 64.f, uPeriod / 64.f, vPeriod / 64.f };

		if (pEdgemap)
			*m_buffers.pExtrasPtr++ = { float(pEdgemap->m_nbSegments - 1), float(pEdgemap->_flatColorsOfs()), float(pEdgemap->_tintTableOfs()), float(pEdgemap->_rawSamplesOfs()) };
		else
		{
			// A surface that isn't tiling is read as if the edge pixels of the source rectangle
			// continued outside it, see warpBlitCoverage() in warp.hlsl.

			float clampInset = m_pBlitSource->isTiling() ? 0.f : m_pBlitSource->sampleMethod() == SampleMethod::Bilinear ? 0.5f : 1.f / 256;

			// Positions exactly on a border between texels take the texel after it, as in
			// SoftBackend, rather than either one by how floats round.

			float nearestBias = m_pBlitSource->sampleMethod() == SampleMethod::Nearest ? 1.f / 32768 : 0.f;

			*m_buffers.pExtrasPtr++ = { params.source.x / 64.f, params.source.y / 64.f, clampInset, nearestBias };
		}

		if (pWarpmap->triangleCount() == 0)
			return;

		// What the vertex shader needs, see WarpInfo in constants.hlsl.

		m_warpConstants = { float(dest.x), float(dest.y), uint32_t(colorOfs), uint32_t(extrasOfs), ref.x, ref.y, { 0, 0 } };
		_setConstants(ConstantBlock::Warp);

		if (pEdgemap)
		{
			if (!_bindEdgemap(pEdgemap))
				return;

			m_bBlitSourceBound = false;			// A palette based blit source keeps its palette there too.
		}

		// The part of each clip rectangle the Warpmap covers.

		RectI patches[16];
		int nPatches = 0;

		RectI warpRect(dest, warpSize);

		for (int i = 0; i < params.nRects; i++)
		{
			RectI patch = RectI::overlap(Util::roundToPixels(pRects[i]), warpRect);

			if (patch.w <= 0 || patch.h <= 0)
				continue;

			patches[nPatches++] = patch;

			if (nPatches == 16)
			{
				_drawWarpmap(pWarpmap, patches, nPatches);
				nPatches = 0;
			}
		}

		if (nPatches > 0)
			_drawWarpmap(pWarpmap, patches, nPatches);
	}

	//____ _normalizeBlendMode() _________________________________________________

	BlendMode GpuBackend::_normalizeBlendMode(BlendMode blendMode)
	{
		switch (blendMode)
		{
			case BlendMode::Undefined:
				return BlendMode::Blend;

			case BlendMode::BlendFixedColor:
				// Blending against a known background is an optimization for software rendering,
				// and an ordinary Blend is an allowed stand-in. Keeping them as one mode also keeps
				// one pipeline instead of two identical ones.

				return BlendMode::Blend;

			default:
				return blendMode;
		}
	}

	//____ _reserveBuffers() _____________________________________________________
	//
	// Makes sure what is left of this frame's vertex, color and extras buffers can hold everything
	// the session may write, going by its SessionInfo. The counts are upper bounds: we can't tell
	// ahead how many fills are subpixel or which rects are blits, so we assume the worst.
	//
	// Per session it can use at most:
	//
	//   Vertices: six per rect of any kind, six per line.
	//   Colors:   those in the color stream - fills, lines and tints copy theirs in - plus a white
	//             one per blit, blur, edgemap, shape and warp draw. A tint block also has a header,
	//             geometry and stop positions besides its colors, at most c_tintBlockOverhead
	//             entries, and any state change and shape segment may carry a tint.
	//   Extras:   two per rect (blits need two, subpixel fills and patches fewer), the entries of a
	//             line with caps per line, 18 per blur for its brush, two per edgemap draw, three
	//             per warp and the entries of shapes, see GpuShapes.

	void GpuBackend::_reserveBuffers(const SessionInfo* pInfo)
	{
		int nVertices, nColors, nExtras;

		if (pInfo)
		{
			int nLines = pInfo->nLineCoords / 2;

			nVertices = pInfo->nRects * 6 + nLines * 6;

			// Overhead of a tint block besides the stop colors, which are in nColors.

			const int c_maxTintStops = Tint::c_maxMixComponents * Tint::c_maxStops;
			const int c_tintBlockOverhead = TintTools::gpuTintBlockMaxSize(Tint::c_maxMixComponents, c_maxTintStops) - c_maxTintStops;

			nColors = pInfo->nColors + pInfo->nBlit + pInfo->nBlur + pInfo->nEdgemapDraws + pInfo->nShapeDraws + pInfo->nWarpDraws + pInfo->nLines
					+ (pInfo->nStateChanges + pInfo->nShapeTints) * c_tintBlockOverhead;
			nExtras = pInfo->nRects * 2 + nLines * GpuShapes::c_lineEntries + pInfo->nBlur * 18 + pInfo->nEdgemapDraws * 2 + pInfo->nWarpDraws * 3 + GpuShapes::maxSessionEntries(*pInfo);
		}
		else
		{
			nVertices = c_defaultVertices;
			nColors = c_defaultColors;
			nExtras = c_defaultExtras;
		}

		bool bOK = _reserveFrameBuffers(nVertices, nColors, nExtras, pInfo ? pInfo->nObjects : 0);

		// If a buffer couldn't grow, the draws that don't fit are dropped, and the messages for
		// that would blame SessionInfo. It is the buffer's fault, so say so, once.

		m_bSessionBuffersShort = !bOK;

		if (!bOK && _isRecording())
		{
			static bool bReported = false;
			if (!bReported)
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer), "Couldn't make room for %d vertices, %d colors and %d extras, draws that don't fit are dropped.",
						 nVertices, nColors, nExtras);
				Base::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, buffer, this, &typeInfo(), __func__, __FILE__, __LINE__);
				bReported = true;
			}
		}

		// What _checkSessionUsage() measures the session against.

		m_bSessionInfoGiven = (pInfo != nullptr);
		m_sessionVertices = nVertices;
		m_sessionColors = nColors;
		m_sessionExtras = nExtras;
		m_pSessionVertexBeg = m_buffers.pVertexPtr;
		m_pSessionColorBeg = m_buffers.pColorPtr;
		m_pSessionExtrasBeg = m_buffers.pExtrasPtr;
	}

	//____ _checkSessionUsage() __________________________________________________
	//
	// Called by endSession(). _reserveBuffers() only makes sure there is room for what SessionInfo
	// says, and the buffers have usually grown well beyond that, so a session that uses more mostly
	// gets away with it - until the one time it doesn't, and draws go missing. This catches it
	// every time, while it is still harmless.
	//
	// Without a SessionInfo the defaults are a guess, not a promise, so there is nothing to check.

	void GpuBackend::_checkSessionUsage()
	{
		if (!m_bSessionInfoGiven || !_isRecording())
			return;

		struct { const char* pWhat; int used; int reserved; } usage[] = {
			{ "vertices",	m_pSessionVertexBeg ? int(m_buffers.pVertexPtr - m_pSessionVertexBeg) : 0,		m_sessionVertices },
			{ "colors",		m_pSessionColorBeg ? int(m_buffers.pColorPtr - m_pSessionColorBeg) : 0,			m_sessionColors },
			{ "extras",		m_pSessionExtrasBeg ? int(m_buffers.pExtrasPtr - m_pSessionExtrasBeg) : 0,		m_sessionExtras } };

		static bool bReported[3] = { false, false, false };

		for (int i = 0; i < 3; i++)
		{
			if (usage[i].used > usage[i].reserved && !bReported[i])
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer), "SessionInfo underestimated what the session needs: room was made for %d %s, the session wrote %d.",
						 usage[i].reserved, usage[i].pWhat, usage[i].used);
				Base::throwError(ErrorLevel::Warning, ErrorCode::Internal, buffer, this, &typeInfo(), __func__, __FILE__, __LINE__);
				bReported[i] = true;
			}
		}

		m_bSessionInfoGiven = false;
	}

} // namespace wax
