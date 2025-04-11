/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_streamtrimbackend.h>

namespace wg
{
	const TypeInfo StreamTrimBackend::TYPEINFO = { "StreamTrimBackend", &GfxBackend::TYPEINFO };

	const TypeInfo s_unspecifiedSurfaceType = { "SurfaceType Unspecified, real backend missing", &Surface::TYPEINFO };

	//____ destructor __________________________________________________

	StreamTrimBackend::~StreamTrimBackend()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamTrimBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________

	void StreamTrimBackend::beginRender()
	{
		if( m_pBackend )
			m_pBackend->beginRender();
	}

	//____ endRender() _______________________________________________

	void StreamTrimBackend::endRender()
	{
		if( m_pBackend )
			m_pBackend->endRender();
	}

	//____ beginSession() ______________________________________________________

	void StreamTrimBackend::beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo )
	{
		if (!m_pBackend)
			return;

		if (m_trimLevel == 0 || m_masks.empty())
		{
			m_pBackend->beginSession(canvasRef, pCanvas, nUpdateRects, pUpdateRects, pInfo);
		}
		else
		{
			m_pUpdateRectsBeg = pUpdateRects;
			m_pUpdateRectsEnd = pUpdateRects + nUpdateRects;

			m_trimmedUpdateRects.clear();

			int trimLevel = std::min(m_trimLevel, int(m_masks.size()));

			auto itMaskEnd = m_masks.begin() + trimLevel;

			for (auto p = m_pUpdateRectsBeg; p < m_pUpdateRectsEnd; p++)
			{

				m_trimmedUpdateRects.push_back(*p);
				auto pToTrim = &m_trimmedUpdateRects.back();

				if( pToTrim->isEmpty() )
					continue;

				for (auto pMask = m_masks.begin(); pMask < itMaskEnd; pMask++)
				{
					if( pMask->canvasRef == canvasRef && pMask->pCanvas == pCanvas )
					{
						_trim(pToTrim, pMask->rects.data(), pMask->rects.data() + pMask->rects.size());
					}

					if (pToTrim->isEmpty())
						break;
				}
			}

			m_pBackend->beginSession(canvasRef, pCanvas, (int) m_trimmedUpdateRects.size(), m_trimmedUpdateRects.data(), pInfo);
		}
	}

	//____ endSession() ________________________________________________________

	void StreamTrimBackend::endSession()
	{
		if( m_pBackend )
			m_pBackend->endSession();

		if( !m_masks.empty() )
			m_masks.pop_front();
	}

	//____ setCanvas() _______________________________________________

	void StreamTrimBackend::setCanvas(Surface* pSurface)
	{
		if (m_pBackend)
			m_pBackend->setCanvas(pSurface);
	}

	void StreamTrimBackend::setCanvas(CanvasRef ref)
	{
		if (m_pBackend)
			m_pBackend->setCanvas(ref);
	}

	//____ setObjects() _______________________________________________

	void StreamTrimBackend::setObjects(Object* const * pBeg, Object* const * pEnd)
	{
		if (m_pBackend)
			m_pBackend->setObjects(pBeg, pEnd);
	}

	//____ setRects() _______________________________________________

	void StreamTrimBackend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
		if (!m_pBackend)
			return;

		int nRects = int(pEnd - pBeg);

		if (m_trimLevel == 0 || m_masks.empty())
		{
			m_pBackend->setRects(pBeg, pEnd);
		}
		else
		{
			m_renderingRects.clear();

			for (auto p = pBeg; p < pEnd; p++)
			{
				for (auto p2 = m_pUpdateRectsBeg; p2 < m_pUpdateRectsEnd; p2++)
				{
					if (p2->contains(*p) )
					{
						// Found updateRect containing our renderingRect
						// Now clip renderingRect against corresponding 
						// clippedUpdateRect.

						m_renderingRects.push_back(RectSPX::overlap(*p, m_trimmedUpdateRects[p2 - m_pUpdateRectsBeg]));
						break;
					}
				}
			}

			m_pBackend->setRects(m_renderingRects.data(), m_renderingRects.data() + m_renderingRects.size());
		}
	}

	//____ setColors() ___________________________________________________

	void StreamTrimBackend::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
		if (m_pBackend)
			m_pBackend->setColors(pBeg, pEnd);
	}

	//____ setTransforms() _______________________________________________

	void StreamTrimBackend::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
		if (m_pBackend)
			m_pBackend->setTransforms(pBeg, pEnd);
	}

	//____ processCommands() _______________________________________________

	void StreamTrimBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd)
	{
		if (m_pBackend)
			m_pBackend->processCommands(pBeg, pEnd);
	}

	//____ canvasInfo() __________________________________________________

	const CanvasInfo* StreamTrimBackend::canvasInfo(CanvasRef ref) const
	{
		if( !m_pBackend)
			return nullptr;

		return m_pBackend->canvasInfo(ref);
	}

	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p StreamTrimBackend::surfaceFactory()
	{
		if( !m_pBackend)
			return nullptr;

		return m_pBackend->surfaceFactory();
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p StreamTrimBackend::edgemapFactory()
	{
		if( !m_pBackend)
			return nullptr;

		return m_pBackend->edgemapFactory();
	}

	//____ maxEdges() ____________________________________________________________

	int StreamTrimBackend::maxEdges() const
	{
		if( !m_pBackend)
			return 0;

		return m_pBackend->maxEdges();
	}

	//____ surfaceType() ______________________________________________________

	const TypeInfo& StreamTrimBackend::surfaceType(void) const
	{
		if( !m_pBackend)
			return s_unspecifiedSurfaceType;

		return m_pBackend->surfaceType();
	}

	//____ addNonMaskingSession() ________________________________________________

	void StreamTrimBackend::addNonMaskingSession()
	{
		m_masks.emplace_back();
		auto& mask = m_masks.back();

		mask.canvasRef = CanvasRef::None;
		mask.pCanvas = nullptr;
	}

	//____ addFullyMaskingSession() ______________________________________________

	void StreamTrimBackend::addFullyMaskingSession( CanvasRef canvasRef, Surface * pCanvas )
	{
		m_masks.emplace_back();
		auto& mask = m_masks.back();

		mask.canvasRef = canvasRef;
		mask.pCanvas = pCanvas;
	}

	//____ addMaskingSession() _______________________________________________

	void StreamTrimBackend::addMaskingSession( CanvasRef canvasRef, Surface * pCanvas, int nMaskingRects, const RectSPX * pMaskingRects )
	{
		m_masks.emplace_back();
		auto& mask = m_masks.back();

		mask.canvasRef = canvasRef;
		mask.pCanvas = pCanvas;
		mask.rects.insert(mask.rects.end(), pMaskingRects, pMaskingRects + nMaskingRects);
	}

	//____ clearSessionMasks() ______________________________________________

	void StreamTrimBackend::clearSessionMasks()
	{
		m_masks.clear();
	}

	//____ setTrimLevel() ________________________________________________________

	void StreamTrimBackend::setTrimLevel(int level)
	{
		m_trimLevel = level;
	}

	//____ _trim() ____________________________________________________________

	void StreamTrimBackend::_trim( RectSPX * pTrim, RectSPX * pMaskBeg, RectSPX * pMaskEnd )
	{
		RectSPX& trim = * pTrim;

		for( RectSPX * pMask = pMaskBeg ; pMask < pMaskEnd ; pMask++ )
		{
			if( pMask->x <= pTrim->x && pMask->x + pMask->w >= pTrim->x + pTrim->w )
			{
				int maskY1 = pMask->y;
				int maskY2 = pMask->y + pMask->h;

				if( maskY1 >= (trim.y + trim.h) || maskY2 <= trim.y )
					continue;

				if( maskY1 <= trim.y && maskY2 >= (trim.y + trim.h) )
				{
					trim.w = trim.h = 0;
					break;
				}
				else if( maskY1 <= trim.y )
				{
					int cut = maskY2 - trim.y;
					trim.y += cut;
					trim.h -= cut;
				}
				else if( maskY2 >= (trim.y + trim.h) )/////////////////////////////////////////////
				{
					trim.h = maskY2 - trim.y;
				}
			}
			else if( pMask->y <= pTrim->y && pMask->y + pMask->h >= pTrim->y + pTrim->h )
			{
				int maskX1 = pMask->x;
				int maskX2 = pMask->x + pMask->w;

				if( maskX1 >= (trim.x + trim.w) || maskX2 <= trim.x )
					continue;

				if( maskX1 <= trim.x && maskX2 >= (trim.x + trim.w) )
				{
					trim.w = trim.h = 0;
					break;
				}
				else if( maskX1 <= trim.x )
				{
					int cut = maskX2 - trim.x;
					trim.x += cut;
					trim.w -= cut;
				}
				else if( maskX2 >= (trim.x + trim.w) )
				{
					trim.w = maskX2 - trim.x;
				}
			}
		}
	}


} // namespace wg
