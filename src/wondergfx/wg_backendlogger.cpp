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

#include <wg_backendlogger.h>

namespace wg
{
	const TypeInfo BackendLogger::TYPEINFO = { "BackendLogger", &GfxBackend::TYPEINFO };

	const TypeInfo s_unspecifiedSurfaceType = { "SurfaceType Unspecified, real backend missing", &Surface::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BackendLogger::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________

	void BackendLogger::beginRender()
	{
		*m_pStream << "BEGIN RENDER" << std::endl;

		if (m_pBackend)
			m_pBackend->beginRender();
	}

	//____ endRender() _______________________________________________

	void BackendLogger::endRender()
	{
		*m_pStream << "END RENDER" << std::endl;

		if( m_pBackend )
			m_pBackend->endRender();
	}

	//____ beginSession() ______________________________________________________

	void BackendLogger::beginSession(const SessionInfo* pSession)
	{
		*m_pStream << "BEGIN SESSION" << std::endl;

		*m_pStream << "    StateChanges:  " << pSession->nStateChanges << std::endl;
		*m_pStream << "    Plots:         " << pSession->nPlots << std::endl;
		*m_pStream << "    Fills:         " << pSession->nFill << std::endl;
		*m_pStream << "    Lines:         " << pSession->nLines << std::endl;
		*m_pStream << "    Blits:         " << pSession->nBlit << std::endl;
		*m_pStream << "    Blurs:         " << pSession->nBlur << std::endl;
		*m_pStream << "    EdgemapDraws:  " << pSession->nEdgemapDraws << std::endl;

		*m_pStream << "    Points:        " << pSession->nPoints << std::endl;
		*m_pStream << "    LineCoords:    " << pSession->nLineCoords << std::endl;
		*m_pStream << "    LineClipRects: " << pSession->nLineClipRects << std::endl;
		*m_pStream << "    Rects:         " << pSession->nRects << std::endl;
		*m_pStream << "    Colors:        " << pSession->nColors << std::endl;
		*m_pStream << "    Transforms:    " << pSession->nTransforms << std::endl;
		*m_pStream << "    Objects:       " << pSession->nObjects << std::endl;


		if (m_pBackend)
			m_pBackend->beginSession(pSession);
	}

	//____ endSession() ________________________________________________________

	void BackendLogger::endSession()
	{
		*m_pStream << "END SESSION" << std::endl;

		if (m_pBackend)
			m_pBackend->endSession();
	}

	//____ setCanvas() _______________________________________________

	void BackendLogger::setCanvas(Surface* pSurface)
	{
		*m_pStream	<< "SET CANVAS: ptr = " << pSurface << " id = " << pSurface->identity() 
					<< " pixelSize = " << pSurface->pixelSize().w  << ", " << pSurface->pixelSize().h 
					<< " format = " << toString(pSurface->pixelFormat()) << std::endl;

		if (m_pBackend)
			m_pBackend->setCanvas(pSurface);
	}

	void BackendLogger::setCanvas(CanvasRef ref)
	{

		*m_pStream << "SET CANVAS: Ref = " << toString(ref) << std::endl;

		if (m_pBackend)
			m_pBackend->setCanvas(ref);
	}

	//____ setObjects() _______________________________________________

	void BackendLogger::setObjects(Object** pBeg, Object** pEnd)
	{
		*m_pStream << "SET OBJECTS: Amount = " << int(pEnd -pBeg) << std::endl;

		Object** p = pBeg;

		while (p < pEnd)
		{
			auto pObj = *p++;
			if( pObj )
				*m_pStream << "    " << pObj << " (" << pObj->typeInfo().className << ")" << std::endl;
			else
				*m_pStream << "    nullptr" << std::endl;

		}

		if (m_pBackend)
			m_pBackend->setObjects(pBeg,pEnd);
	}

	//____ setCoords() _______________________________________________

	void BackendLogger::setCoords(spx* pBeg, spx* pEnd)
	{
		*m_pStream << "SET COORDS: Amount = " << int(pEnd - pBeg);

		m_pCoordsBeg = pBeg;
		m_pCoordsEnd = pEnd;
		m_pCoordsPtr = pBeg;

		spx * p = pBeg;

		int rows = 0;

		while (p < pEnd)
		{
			rows %= 8;

			if (rows == 0)
				*m_pStream << std::endl << "   ";
			else
				*m_pStream << ", ";

			*m_pStream << *p++;

			rows++;
		}

		*m_pStream << std::endl;

		if (m_pBackend)
			m_pBackend->setCoords(pBeg, pEnd);
	}

	//____ setColors() ___________________________________________________

	void BackendLogger::setColors(HiColor* pBeg, HiColor* pEnd)
	{
		*m_pStream << "SET COLORS: Amount = " << int(pEnd - pBeg) << std::endl;

		m_pColorsBeg = pBeg;
		m_pColorsEnd = pEnd;
		m_pColorsPtr = pBeg;

		HiColor* p = pBeg;

		while (p < pEnd)
		{
			*m_pStream << "    " << p->r << ", " << p->g << ", " << p->b << ", " << p->a << std::endl;
			p++;
		}

		if (m_pBackend)
			m_pBackend->setColors(pBeg, pEnd);
	}



	//____ setTransforms() _______________________________________________

	void BackendLogger::setTransforms(Transform* pBeg, Transform* pEnd)
	{
		*m_pStream << "SET TRANSFORMS: Amount = " << int(pEnd - pBeg) << std::endl;

		Transform * p = pBeg;

		while (p < pEnd)
		{
			*m_pStream << "    " << p->xx << ", " << p->xy << std::endl;
			*m_pStream << "    " << p->yx << ", " << p->yy << std::endl;
			*m_pStream << std::endl;

			p++;
		}

		if (m_pBackend)
			m_pBackend->setTransforms(pBeg, pEnd);
	}

	//____ processCommands() _______________________________________________

	void BackendLogger::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
		*m_pStream << "PROCESS COMMANDS:" << std::endl;

		HiColor* pColors = m_pColorsPtr;
		spx* pCoords = m_pCoordsPtr;

		auto p = pBeg;
		while (p < pEnd)
		{
			auto cmd = Command(*p++);
			switch (cmd)
			{
			case Command::None:
				*m_pStream << "None (this should not be present, something is probably wrong!)" << std::endl;
				break;

			case Command::StateChange:
			{
				*m_pStream << "    StateChange" << std::endl;

				int32_t statesChanged = *p++;

				if (statesChanged & uint8_t(StateChange::BlitSource))
				{
					int32_t objectOfs = *p++;

					*m_pStream << "        BlitSource: " << objectOfs << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					BlendMode mode = (BlendMode)*p++;

					*m_pStream << "        BlendMode: " << toString(mode) << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					HiColor& col = *pColors++;

					*m_pStream << "        TintColor: " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{
					int32_t objectOfs = *p++;
					int32_t	x = *p++;
					int32_t	y = *p++;
					int32_t	w = *p++;
					int32_t	h = *p++;

					*m_pStream << "        TintMap: " << objectOfs << " rect: " << x << ", " << y << ", " << w << ", " << h << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					float morphFactor = (*p++) / 4096.f;

					*m_pStream << "        MorphFactor: " << morphFactor << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					HiColor& col = *pColors++;

					*m_pStream << "        FixedBlendColor: " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					int32_t objectOfs = *p++;

					*m_pStream << "        BlurBrush: " << objectOfs << std::endl;					
				}

				break;
			}

			case Command::Fill:
			{
				int32_t nRects = *p++;

				HiColor& col = *pColors++;

				*m_pStream << "    Fill: " << nRects << " rects with color: " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;

				_printRects(*m_pStream, nRects, reinterpret_cast<RectSPX*>(pCoords) );
				pCoords += nRects * 4;
				break;
			}

			case Command::Plot:
			{
				int32_t nPlots = *p++;

				*m_pStream << "    Plot: " << nPlots << " points.";	

				for (int i = 0; i < nPlots; i++)
				{
					if ((i % 16) == 0)
						*m_pStream << std::endl << "        ";

					*m_pStream << pCoords[i];
				}

				pColors += nPlots;
				pCoords += nPlots;
				break;
			}

			case Command::Line:
			{
				spx thickness = *p++;
				int32_t nRects = *p++;
				int32_t nLines = *p++;

				HiColor col = * pColors++;

				*m_pStream << "    Draw " << nLines << " lines with color : " << col.r << ", " << col.g << ", " << col.b << ", " << col.a
					<< " and thickness: " << thickness
					<< ". Clipped by " << nRects << " rectangles:" << std::endl;

				_printRects(*m_pStream, nRects, reinterpret_cast<RectSPX*>(pCoords));
				pCoords += nRects * 4;

				for (int i = 0; i < nLines; i++)
				{
					CoordSPX beg = { *pCoords++, *pCoords++ };
					CoordSPX end = { *pCoords++, *pCoords++ };

					*m_pStream << "        from (" << beg.x << ", " << beg.y << ") to(" << end.x << ", " << end.y << ")" << std::endl;
				}

				break;
			}

			case Command::DrawEdgemap:
			{
				int32_t objectOfs = *p++;
				spx destX = *p++;
				spx destY = *p++;
				int32_t transform = *p++;
				int32_t	nRects = *p++;

				*m_pStream << "    DrawEdgemap: " << objectOfs << " at: " << destX << ", " << destY << ", with transform: " << transform 
					<< " split into " << nRects << " rectangles." << std::endl;

				_printRects(*m_pStream, nRects, reinterpret_cast<RectSPX*>(pCoords));
				pCoords += nRects * 4;
				break;
			}

			case Command::Blur:
			case Command::Tile:
			case Command::Blit:
			case Command::ClipBlit:
			{
				if (cmd == Command::Blur)
					*m_pStream << "    Blur: ";
				else if (cmd == Command::Tile)
					*m_pStream << "    Tile: ";
				else if (cmd == Command::Blit)
					*m_pStream << "    Blit: ";
				else if (cmd == Command::ClipBlit)
					*m_pStream << "    ClipBlit: ";

				int32_t nRects = *p++;
				int32_t transform = *p++;

				int32_t srcX = *p++;
				int32_t srcY = *p++;
				int32_t dstX = *p++;
				int32_t dstY = *p++;

				*m_pStream << nRects << " rects with transform: " << transform << ", src: " << srcX << ", " << srcY << " dest: " << dstX << ", " << dstY << std::endl;

				_printRects(*m_pStream, nRects, reinterpret_cast<RectSPX*>(pCoords));
				pCoords += nRects * 4;
				break;
			}

			default:
				*m_pStream << "ERROR: Unknown command (" << int(cmd) << ")" << std::endl;
				break;
			}
		}

		m_pCoordsPtr = pCoords;
		m_pColorsPtr = pColors;

		if (m_pBackend)
			m_pBackend->processCommands(pBeg, pEnd);
	}

	//____ canvasInfo() __________________________________________________

	const CanvasInfo* BackendLogger::canvasInfo(CanvasRef ref) const
	{
		const CanvasInfo * pCanvasInfo = nullptr;

		if (m_pBackend)
			pCanvasInfo = m_pBackend->canvasInfo(ref);


		*m_pStream << "Called canvasInfo() for CanvasRef: " << toString(ref);

		if (pCanvasInfo)
		{
			*m_pStream << " which succeeded.";
		}
		else
		{
			*m_pStream << " which failed.";
		}

		*m_pStream << std::endl;

		return pCanvasInfo;
	}

	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p BackendLogger::surfaceFactory()
	{
		SurfaceFactory_p pFactory = nullptr;

		if (m_pBackend)
			pFactory = m_pBackend->surfaceFactory();

		*m_pStream << "Called surfaceFactory(). Returned: " << pFactory.rawPtr() << std::endl;

		return pFactory;
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p BackendLogger::edgemapFactory()
	{
		EdgemapFactory_p pFactory = nullptr;

		if (m_pBackend)
			pFactory = m_pBackend->edgemapFactory();

		*m_pStream << "Called edgemapFactory(). Returned: " << pFactory.rawPtr() << std::endl;

		return pFactory;
	}

	//____ maxEdges() ____________________________________________________________

	int BackendLogger::maxEdges() const
	{
		int maxEdges = 0;

		if (m_pBackend)
			maxEdges = m_pBackend->maxEdges();

		*m_pStream << "Called maxEdges(). Returned: " << maxEdges << std::endl;

		return maxEdges;

	}

	//____ surfaceType() ______________________________________________________

	const TypeInfo& BackendLogger::surfaceType(void) const
	{
		const TypeInfo& ref = m_pBackend ? m_pBackend->surfaceType() : s_unspecifiedSurfaceType;

		*m_pStream << "Called surfaceType(). Returned: " << ref.className << std::endl;

		return ref;
	}

	//____ _printRects() _______________________________________________________

	void BackendLogger::_printRects(std::ostream& stream, int nRects, RectSPX* pRects)
	{
		for (int i = 0; i < nRects; i++)
		{
			stream << "        " << pRects[i].x << ", " << pRects[i].y << ", " << pRects[i].w << ", " << pRects[i].h << std::endl;
		}
	}



} // namespace wg
