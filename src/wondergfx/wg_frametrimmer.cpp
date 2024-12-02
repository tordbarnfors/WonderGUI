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

#include <wg_frametrimmer.h>

namespace wg
{
	const TypeInfo FrameTrimmer::TYPEINFO = { "FrameTrimmer", &GfxBackend::TYPEINFO };

	const TypeInfo s_unspecifiedSurfaceType = { "SurfaceType Unspecified, real backend missing", &Surface::TYPEINFO };

	//____ destructor __________________________________________________

	FrameTrimmer::~FrameTrimmer()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FrameTrimmer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________

	void FrameTrimmer::beginRender()
	{
		if( m_pBackend )
			m_pBackend->beginRender();
	}

	//____ endRender() _______________________________________________

	void FrameTrimmer::endRender()
	{
		if( m_pBackend )
			m_pBackend->endRender();
	}

	//____ beginSession() ______________________________________________________

	void FrameTrimmer::beginSession(const SessionInfo* pSession)
	{
		if (!m_pBackend)
			return;

		if (m_trimLevel == 0 || m_masks.empty())
		{
			m_pBackend->beginSession(pSession);
		}
		else
		{
			m_pUpdateRectsBeg = pSession->pUpdateRects;
			m_pUpdateRectsEnd = pSession->pUpdateRects + pSession->nUpdateRects;

			m_trimmedUpdateRects.clear();

			int trimLevel = std::min(m_trimLevel, int(m_masks.size()));

			auto itMaskEnd = m_masks.begin() + trimLevel;

			for (auto p = m_pUpdateRectsBeg; p < m_pUpdateRectsEnd; p++)
			{
				m_trimmedUpdateRects.push_back(*p);
				auto pToTrim = &m_trimmedUpdateRects.back();

				if (pToTrim->isEmpty())
					break;

				for (auto pMask = m_masks.begin(); pMask < itMaskEnd; pMask++)
				{
					_trim(pToTrim, pMask->data(), pMask->data() + pMask->size());

					if (pToTrim->isEmpty())
						break;
				}
			}

			// We create a new session struct so we can supply our trimmed update rects

			SessionInfo newSessionInfo = *pSession;
			newSessionInfo.pUpdateRects = m_trimmedUpdateRects.data();
			newSessionInfo.nUpdateRects = m_trimmedUpdateRects.size();

			m_pBackend->beginSession(&newSessionInfo);
		}
	}

	//____ endSession() ________________________________________________________

	void FrameTrimmer::endSession()
	{
		if( m_pBackend )
			m_pBackend->endSession();
	}

	//____ setCanvas() _______________________________________________

	void FrameTrimmer::setCanvas(Surface* pSurface)
	{
		if (m_pBackend)
			m_pBackend->setCanvas(pSurface);
	}

	void FrameTrimmer::setCanvas(CanvasRef ref)
	{
		if (m_pBackend)
			m_pBackend->setCanvas(ref);
	}

	//____ setObjects() _______________________________________________

	void FrameTrimmer::setObjects(Object** pBeg, Object** pEnd)
	{
		if (m_pBackend)
			m_pBackend->setObjects(pBeg, pEnd);
	}

	//____ setRects() _______________________________________________

	void FrameTrimmer::setRects(RectSPX* pBeg, RectSPX* pEnd)
	{
		if (!m_pBackend)
			return;

		int nRects = pEnd - pBeg;

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
						// Found updateRect containg our renderingRect
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

	void FrameTrimmer::setColors(HiColor* pBeg, HiColor* pEnd)
	{
		if (m_pBackend)
			m_pBackend->setColors(pBeg, pEnd);
	}

	//____ setTransforms() _______________________________________________

	void FrameTrimmer::setTransforms(Transform* pBeg, Transform* pEnd)
	{
		if (m_pBackend)
			m_pBackend->setTransforms(pBeg, pEnd);
	}

	//____ processCommands() _______________________________________________

	void FrameTrimmer::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
		if (m_pBackend)
			m_pBackend->processCommands(pBeg, pEnd);
	}

	//____ canvasInfo() __________________________________________________

	const CanvasInfo* FrameTrimmer::canvasInfo(CanvasRef ref) const
	{
		if( !m_pBackend)
			return nullptr;

		return m_pBackend->canvasInfo(ref);
	}

	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p FrameTrimmer::surfaceFactory()
	{
		if( !m_pBackend)
			return nullptr;

		return m_pBackend->surfaceFactory();
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p FrameTrimmer::edgemapFactory()
	{
		if( !m_pBackend)
			return nullptr;

		return m_pBackend->edgemapFactory();
	}

	//____ maxEdges() ____________________________________________________________

	int FrameTrimmer::maxEdges() const
	{
		if( !m_pBackend)
			return 0;

		return m_pBackend->maxEdges();
	}

	//____ surfaceType() ______________________________________________________

	const TypeInfo& FrameTrimmer::surfaceType(void) const
	{
		if( !m_pBackend)
			return s_unspecifiedSurfaceType;

		return m_pBackend->surfaceType();
	}

	//____ pushMask() _______________________________________________

	void FrameTrimmer::pushMask(RectSPX* pBeg, RectSPX* pEnd)
	{
		m_masks.emplace_back();

		auto& vec = m_masks.back();

		vec.insert(vec.end(), pBeg, pEnd);
	}

	//____ popMaskRects() ________________________________________________

	void FrameTrimmer::popMask()
	{
		m_masks.pop_front();
	}

	//____ clearMasks() ______________________________________________

	void FrameTrimmer::clearMasks()
	{
		m_masks.clear();
	}

	//____ setTrimLevel() ________________________________________________________

	void FrameTrimmer::setTrimLevel(int level)
	{
		m_trimLevel = level;
	}

	//____ _trim() ____________________________________________________________

	void FrameTrimmer::	_trim( RectSPX * pTrim, RectSPX * pMaskBeg, RectSPX * pMaskEnd )
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
