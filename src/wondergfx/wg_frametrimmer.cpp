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
		for( auto pObject : m_objects )
			pObject->release();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FrameTrimmer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________

	void FrameTrimmer::beginRender()
	{
		if( m_trimLevel == 0 && m_pBackend )
			m_pBackend->beginRender();
	}

	//____ endRender() _______________________________________________

	void FrameTrimmer::endRender()
	{
		if( m_trimLevel == 0 && m_pBackend )
			m_pBackend->endRender();
	}

	//____ beginSession() ______________________________________________________

	void FrameTrimmer::beginSession(const SessionInfo* pSession)
	{
		if( m_trimLevel == 0 )
		{
			if( m_pBackend )
				m_pBackend->beginSession(pSession);
		}
		else
		{
			m_sessions.emplace_back();

			auto& session = m_sessions.back();

			session.info = * pSession;

			session.updateRectsBeg = m_rects.size();
			m_rects.insert( m_rects.end(), pSession->pUpdateRects, pSession->pUpdateRects + pSession->nUpdateRects );
			session.updateRectsEnd = m_rects.size();

			session.commandRectsBeg = m_rects.size();
			session.commandRectsEnd = m_rects.size();

			m_pCanvas		= nullptr;
			m_canvasRef 	= CanvasRef::None;

			m_objectsBeg 	= m_objectsEnd		= m_objects.size();
			m_rectsBeg 		= m_rectsEnd		= m_rects.size();
			m_colorsBeg 	= m_colorsEnd		= m_colors.size();
			m_transformsBeg = m_transformsEnd	= m_transforms.size();
		}
	}

	//____ endSession() ________________________________________________________

	void FrameTrimmer::endSession()
	{
		if( m_trimLevel == 0 )
		{
			if( m_pBackend )
				m_pBackend->beginSession(pSession);
		}
		else
		{
			auto& session = m_sessions.back();

			session.updateRectsEnd = m_rects.size();
			session.commandRectsEnd = m_rects.size();
		}
	}

	//____ setCanvas() _______________________________________________

	void FrameTrimmer::setCanvas(Surface* pSurface)
	{
		m_pCanvas = pSurface;
		m_canvasRef = CanvasRef::None;
	}

	void FrameTrimmer::setCanvas(CanvasRef ref)
	{
		m_pCanvas = nullptr;
		m_canvasRef = ref;
	}

	//____ setObjects() _______________________________________________

	void FrameTrimmer::setObjects(Object** pBeg, Object** pEnd)
	{
		m_objectsBeg = m_objects.size();

		auto it = m_objects.insert( m_objects.end(), pBeg, pEnd );
		while( it < m_objects.end() )
			(*it++)->retain();

		m_objectsEnd = m_objects.size();
	}

	//____ setRects() _______________________________________________

	void FrameTrimmer::setRects(RectSPX* pBeg, RectSPX* pEnd)
	{
		m_rectsBeg = m_rects.size();
		m_rects.insert( m_rects.end(), pBeg, pEnd );
		m_rectsEnd = m_rects.size();
	}

	//____ setColors() ___________________________________________________

	void FrameTrimmer::setColors(HiColor* pBeg, HiColor* pEnd)
	{
		m_colorsBeg = m_colors.size();
		m_colors.insert( m_colors.end(), pBeg, pEnd );
		m_colorsEnd = m_colors.size();
	}

	//____ setTransforms() _______________________________________________

	void FrameTrimmer::setTransforms(Transform* pBeg, Transform* pEnd)
	{
		m_transformsBeg = m_transforms.size();
		m_transforms.insert( m_transforms.end(), pBeg, pEnd );
		m_transformsEnd = m_transforms.size();
	}

	//____ processCommands() _______________________________________________

	void FrameTrimmer::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
		m_sessions.back().commandSets.emplace_back();
		auto& set = m_sessions.back().commandSets.back();

		set.pCanvas = m_pCanvas;
		set.canvasRef = m_canvasRef;

		set.commandsBeg = m_commands.size();
		m_commands.insert( m_commands.end(), pBeg, pEnd );
		set.commandsEnd = m_commands.size();

		set.objectsBeg		= m_objectsBeg;
		set.objectsEnd		= m_objectsEnd;
		set.rectsBeg		= m_rectsBeg;
		set.rectsEnd		= m_rectsEnd;
		set.colorsBeg		= m_colorsBeg;
		set.colorsEnd		= m_colorsEnd;
		set.transformsBeg	= m_transformsBeg;
		set.transformsEnd	= m_transformsEnd;
	}

	//____ flush() _______________________________________________________________

	void FrameTrimmer::flush()
	{
		_trimFrames();
		_renderFrames();

		// Cleanup

		for( auto pObject : m_objects )
			pObject->release();

		m_sessions.clear();
		m_commands.clear();
		m_rects.clear();
		m_colors.clear();
		m_objects.clear();
		m_transforms.clear();
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

	//____ setTrimLevel() ________________________________________________________

	void FrameTrimmer::setTrimLevel( int level )
	{
		m_trimLevel = level;
	}

	//____ _trimFrames() _________________________________________________________

	void FrameTrimmer::_trimFrames()
	{
		for( auto itTrimFrame = m_sessions.begin() ; itTrimFrame != m_sessions.end() ; itTrimFrame++ )
		{

			for( auto itMaskFrame = itTrimFrame+1 ; itMaskFrame != m_sessions.end() ; itMaskFrame++ )
			{
				if( itTrimFrame->commandSets.front().canvasRef == itMaskFrame->commandSets.front().canvasRef &&
					itTrimFrame->commandSets.front().pCanvas == itMaskFrame->commandSets.front().pCanvas )
				{
					auto itTrimEnd = m_rects.begin() + itTrimFrame->updateRectsEnd;
					for( auto itTrim = m_rects.begin() + itTrimFrame->updateRectsBeg ; itTrim < itTrimEnd ; itTrim++ )
					{
						if( !itTrim->isEmpty() )
						{
							_trim(&(*itTrim), m_rects.data() + itMaskFrame->updateRectsBeg, m_rects.data() + itMaskFrame->updateRectsEnd,
								  m_rects.data() + itMaskFrame->commandRectsBeg, m_rects.data() + itMaskFrame->commandRectsEnd );
						}
					}
				}
			}
		}
	}


	//____ _trim() ____________________________________________________________

	void FrameTrimmer::	_trim( RectSPX * pTrim, RectSPX * pMaskBeg, RectSPX * pMaskEnd, RectSPX * pDrawBeg, RectSPX * pDrawEnd )
	{
		RectSPX trim = * pTrim;

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
				else if( maskY2 >= (trim.y + trim.h) )
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

		// Clip the draw rects as well.

		if( trim.x != pTrim->x || trim.y != pTrim->y || trim.w != pTrim->w || trim.h != pTrim->h )
		{
			for( RectSPX * p = pDrawBeg ; p < pDrawEnd ; p++ )
			{
				if( pTrim->contains(p->pos()) )
					* p = RectSPX::overlap( * p, trim );
			}

			* pTrim = trim;
		}
	}


	//____ _renderFrames() _______________________________________________________

	void FrameTrimmer::_renderFrames()
	{
		if( !m_pBackend )
			return;

		Surface * pCanvas	= nullptr;
		CanvasRef canvasRef	= CanvasRef::None;

		size_t		objectsEnd = 0;
		size_t		rectsEnd = 0;
		size_t		colorsEnd = 0;
		size_t		transformsEnd = 0;

		m_pBackend->beginRender();

		for( auto& session : m_sessions )
		{
			session.info.pUpdateRects = m_rects.data() + session.updateRectsBeg;
			m_pBackend->beginSession(&session.info);

			for( auto& set : session.commandSets )
			{
				if( canvasRef != set.canvasRef || pCanvas != set.pCanvas )
				{
					canvasRef = set.canvasRef;
					pCanvas = set.pCanvas;

					if( canvasRef != CanvasRef::None )
						m_pBackend->setCanvas(canvasRef);
					else
						m_pBackend->setCanvas(pCanvas);
				}

				if( objectsEnd != set.objectsEnd )
				{
					objectsEnd = set.objectsEnd;
					m_pBackend->setObjects(m_objects.data() + set.objectsBeg, m_objects.data() + set.objectsEnd);
				}

				if( rectsEnd != set.rectsEnd )
				{
					rectsEnd = set.rectsEnd;
					m_pBackend->setRects(m_rects.data() + set.rectsBeg, m_rects.data() + set.rectsEnd);
				}

				if( colorsEnd != set.colorsEnd )
				{
					colorsEnd = set.colorsEnd;
					m_pBackend->setColors(m_colors.data() + set.colorsBeg, m_colors.data() + set.colorsEnd);
				}

				if( transformsEnd != set.transformsEnd )
				{
					transformsEnd = set.transformsEnd;
					m_pBackend->setTransforms(m_transforms.data() + set.transformsBeg, m_transforms.data() + set.transformsEnd);
				}

				m_pBackend->processCommands(m_commands.data() + set.commandsBeg, m_commands.data() + set.commandsEnd);
			}
		}

		m_pBackend->endRender();
	}

} // namespace wg
