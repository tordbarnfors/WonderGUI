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

	//____ beginRender() ______________________________________________________

	void BackendLogger::beginRender()
	{
		if( m_pOStream )
			*m_pOStream << "BEGIN RENDER" << std::endl;

		if (m_pBackend)
			m_pBackend->beginRender();
	}

	//____ endRender() ________________________________________________________

	void BackendLogger::endRender()
	{
		if( m_pOStream )
			*m_pOStream << "END RENDER" << std::endl;

		if( m_pBackend )
			m_pBackend->endRender();
	}

	//____ beginSession() _____________________________________________________

	void BackendLogger::beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo )
	{
		if( m_pOStream )
		{
			*m_pOStream << "BEGIN SESSION" << std::endl;

			*m_pOStream << "    CanvasRef:   " << toString(canvasRef) << std::endl;
			*m_pOStream << "    CanvasPtr:   " << pCanvas << std::endl;
			*m_pOStream << "    UpdateRects:   " << nUpdateRects << std::endl;

			if( pInfo )
			{
				*m_pOStream << "    CanvasChanges: " << pInfo->nSetCanvas << std::endl;
				*m_pOStream << "    StateChanges:  " << pInfo->nStateChanges << std::endl;
				*m_pOStream << "    Fills:         " << pInfo->nFill << std::endl;
				*m_pOStream << "    Lines:         " << pInfo->nLines << std::endl;
				*m_pOStream << "    Blits:         " << pInfo->nBlit << std::endl;
				*m_pOStream << "    Blurs:         " << pInfo->nBlur << std::endl;
				*m_pOStream << "    EdgemapDraws:  " << pInfo->nEdgemapDraws << std::endl;

				*m_pOStream << "    LineCoords:    " << pInfo->nLineCoords << std::endl;
				*m_pOStream << "    LineClipRects: " << pInfo->nLineClipRects << std::endl;
				*m_pOStream << "    Rects:         " << pInfo->nRects << std::endl;
				*m_pOStream << "    Colors:        " << pInfo->nColors << std::endl;
				*m_pOStream << "    Transforms:    " << pInfo->nTransforms << std::endl;
				*m_pOStream << "    Objects:       " << pInfo->nObjects << std::endl;
			}

			*m_pOStream << "UPDATE RECTS" << std::endl;
			_printRects(* m_pOStream, nUpdateRects, pUpdateRects);
		}

		if (m_pBackend)
			m_pBackend->beginSession(canvasRef, pCanvas, nUpdateRects, pUpdateRects, pInfo);
	}

	//____ endSession() _______________________________________________________

	void BackendLogger::endSession()
	{
		if( m_pOStream )
			*m_pOStream << "END SESSION" << std::endl;

		if (m_pBackend)
			m_pBackend->endSession();
	}

	//____ setCanvas() ________________________________________________________

	void BackendLogger::setCanvas(Surface* pSurface)
	{
		if( m_pOStream )
		{
			*m_pOStream	<< "SET CANVAS: ptr = " << pSurface << " id = " << pSurface->identity()
			<< " pixelSize = " << pSurface->pixelSize().w  << ", " << pSurface->pixelSize().h
			<< " format = " << toString(pSurface->pixelFormat()) << std::endl;
		}

		if (m_pBackend)
			m_pBackend->setCanvas(pSurface);
	}

	void BackendLogger::setCanvas(CanvasRef ref)
	{
		if( m_pOStream )
			*m_pOStream << "SET CANVAS: Ref = " << toString(ref) << std::endl;

		if (m_pBackend)
			m_pBackend->setCanvas(ref);
	}

	//____ setObjects() _______________________________________________________

	void BackendLogger::setObjects(Object* const * pBeg, Object* const * pEnd)
	{
		if( m_pOStream )
		{
			*m_pOStream << "SET OBJECTS: Amount = " << int(pEnd -pBeg) << std::endl;

			m_pObjectsBeg = pBeg;
			m_pObjectsEnd = pEnd;
			m_pObjectsPtr = pBeg;

			Object* const * p = pBeg;

			while (p < pEnd)
			{
				auto pObj = *p++;
				if( pObj )
					*m_pOStream << "    " << pObj << " (" << pObj->typeInfo().className << ")" << std::endl;
				else
					*m_pOStream << "    nullptr" << std::endl;
			}
		}

		if (m_pBackend)
			m_pBackend->setObjects(pBeg,pEnd);
	}

	//____ setRects() _________________________________________________________

	void BackendLogger::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
		if( m_pOStream )
		{
			*m_pOStream << "SET RECTS: Amount = " << int(pEnd - pBeg);

			m_pRectsBeg = pBeg;
			m_pRectsEnd = pEnd;
			m_pRectsPtr = pBeg;

			const RectSPX * p = pBeg;

			int rows = 0;

			while (p < pEnd)
			{
				rows %= 4;

				if (rows == 0)
					*m_pOStream << std::endl << "   ";

				else
					*m_pOStream << ", ";

				RectSPX rect = *p++;

				*m_pOStream << "(" << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << ")";

				rows++;
			}

			*m_pOStream << std::endl;
		}

		if (m_pBackend)
			m_pBackend->setRects(pBeg, pEnd);
	}

	//____ setColors() ________________________________________________________

	void BackendLogger::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
		if( m_pOStream )
		{
			*m_pOStream << "SET COLORS: Amount = " << int(pEnd - pBeg) << std::endl;

			m_pColorsBeg = pBeg;
			m_pColorsEnd = pEnd;
			m_pColorsPtr = pBeg;

			const HiColor* p = pBeg;

			while (p < pEnd)
			{
				*m_pOStream << "    " << p->r << ", " << p->g << ", " << p->b << ", " << p->a << std::endl;
				p++;
			}
		}

		if (m_pBackend)
			m_pBackend->setColors(pBeg, pEnd);
	}

	//____ setTransforms() ____________________________________________________

	void BackendLogger::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
		if( m_pOStream )
		{
			*m_pOStream << "SET TRANSFORMS: Amount = " << int(pEnd - pBeg) << std::endl;

			const Transform * p = pBeg;

			while (p < pEnd)
			{
				*m_pOStream << "    " << p->xx << ", " << p->xy << std::endl;
				*m_pOStream << "    " << p->yx << ", " << p->yy << std::endl;
				*m_pOStream << std::endl;

				p++;
			}
		}

		if (m_pBackend)
			m_pBackend->setTransforms(pBeg, pEnd);
	}

	//____ processCommands() __________________________________________________

	void BackendLogger::processCommands(const uint16_t* pBeg, const uint16_t* pEnd)
	{
		if( m_pOStream )
		{
			*m_pOStream << "PROCESS COMMANDS:" << std::endl;

			const HiColor* pColors	= m_pColorsPtr;
			const RectSPX* pRects	= m_pRectsPtr;
			Object* const * pObjects = m_pObjectsPtr;

			auto p = pBeg;
			while (p < pEnd)
			{
				auto cmd = Command(*p++);
				switch (cmd)
				{
				case Command::None:
					*m_pOStream << "None (this should not be present, something is probably wrong!)" << std::endl;
					break;

				case Command::StateChange:
				{
					*m_pOStream << "    StateChange" << std::endl;

					int32_t statesChanged = *p++;

					if (statesChanged & uint8_t(StateChange::BlitSource))
					{
						Object* pObj = *pObjects++;
						*m_pOStream << "        BlitSource: " << pObj << std::endl;
					}

					if (statesChanged & uint8_t(StateChange::TintColor))
					{
						const HiColor& col = *pColors++;

						*m_pOStream << "        TintColor: " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;
					}

					if (statesChanged & uint8_t(StateChange::TintMap))
					{
						auto p32 = (const spx *) p;
						int32_t	x = *p32++;
						int32_t	y = *p32++;
						int32_t	w = *p32++;
						int32_t	h = *p32++;

						int32_t	nHorrColors = *p32++;
						int32_t	nVertColors = *p32++;
						p = (const uint16_t*) p32;

						*m_pOStream << "        TintMap: rect: " << x << ", " << y << ", " << w << ", " << h << " horr colors: " << nHorrColors << " vert colors: , " << nVertColors << std::endl;
					}

					if (statesChanged & uint8_t(StateChange::BlendMode))
					{
						BlendMode mode = (BlendMode)*p++;

						*m_pOStream << "        BlendMode: " << toString(mode) << std::endl;
					}

					if (statesChanged & uint8_t(StateChange::MorphFactor))
					{
						float morphFactor = (*p++) / 4096.f;

						*m_pOStream << "        MorphFactor: " << morphFactor << std::endl;
					}

					if (statesChanged & uint8_t(StateChange::FixedBlendColor))
					{
						const HiColor& col = *pColors++;

						*m_pOStream << "        FixedBlendColor: " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;
					}

					if (statesChanged & uint8_t(StateChange::Blur))
					{
						spx size = *p++;

						auto pRed = p;
						auto pGreen = p+9;
						auto pBlue = p + 18;
						p += 27;

						*m_pOStream << "        Blur: size: " << size << std::endl;

						*m_pOStream << "            Red matrix: ";
						for (int i = 0; i < 8; i++)
							*m_pOStream << pRed[i] / float(32768) << ", ";
						*m_pOStream << pRed[8] / float(32768) << std::endl;

						*m_pOStream << "            Green matrix: ";
						for (int i = 0; i < 8; i++)
							*m_pOStream << pGreen[i] / float(32768) << ", ";
						*m_pOStream << pGreen[8] / float(32768) << std::endl;

						*m_pOStream << "            Blue matrix: ";
						for (int i = 0; i < 8; i++)
							*m_pOStream << pBlue[i] / float(32768) << ", ";
						*m_pOStream << pBlue[8] / float(32768) << std::endl;
					}

					// Take care of alignment

					if( (uintptr_t(p) & 0x2) == 2 )
						p++;

					break;
				}

				case Command::Fill:
				{
					int32_t nRects = *p++;

					const HiColor& col = *pColors++;

					*m_pOStream << "    Fill: " << nRects << " rects with color: " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;

					_printRects(*m_pOStream, nRects, pRects);
					pRects += nRects;
					break;
				}

				case Command::Line:
				{
					int32_t nRects = *p++;
					int32_t nLines = *p++;
					p++;


					*m_pOStream << "    Draw " << nLines << " clipped by " << nRects << " rectangles:" << std::endl;

					_printRects(*m_pOStream, nRects, pRects);
					pRects += nRects;


					for (int i = 0; i < nLines; i++)
					{
						auto p32 = (const spx *) p;
						CoordSPX beg = { *p32++, *p32++ };
						CoordSPX end = { *p32++, *p32++ };
						p = (const uint16_t*) p32;

						spx thickness = *p++;
						p++;

						HiColor col = * pColors++;

						*m_pOStream << "        from (" << beg.x << ", " << beg.y << ") to (" << end.x << ", " << end.y << ") with thickness " << thickness
									<< " and color " << col.r << ", " << col.g << ", " << col.b << ", " << col.a << std::endl;
					}

					break;
				}

				case Command::DrawEdgemap:
				{
					int32_t	nRects = *p++;
					int32_t transform = *p++;

					p++;		// padding

					auto p32 = (const spx *) p;
					spx destX = *p32++;
					spx destY = *p32++;
					p = (const uint16_t*) p32;

					Object * pObj = *pObjects++;

					*m_pOStream << "    DrawEdgemap: " << pObj << " at: " << destX << ", " << destY << ", with transform: " << transform
						<< " split into " << nRects << " rectangles." << std::endl;

					_printRects(*m_pOStream, nRects, pRects);
					pRects += nRects;
					break;
				}

				case Command::Blur:
				case Command::Tile:
				case Command::Blit:
				case Command::ClipBlit:
				{
					if (cmd == Command::Blur)
						*m_pOStream << "    Blur: ";
					else if (cmd == Command::Tile)
						*m_pOStream << "    Tile: ";
					else if (cmd == Command::Blit)
						*m_pOStream << "    Blit: ";
					else if (cmd == Command::ClipBlit)
						*m_pOStream << "    ClipBlit: ";

					int32_t nRects = *p++;


					*m_pOStream << nRects << " rects." << std::endl;

					for( int i = 0 ; i < nRects ; i++ )
					{
						const RectSPX& rect = * pRects++;


						auto p32 = (const spx *) p;
						int32_t srcX = *p32++;
						int32_t srcY = *p32++;
						int32_t dstX = *p32++;
						int32_t dstY = *p32++;
						p = (const uint16_t*) p32;

						int32_t transform = *p++;
						p++;						// padding

						* m_pOStream 	<< "        rect = (" << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h
								<< "), source = (" << srcX << ", " << srcY
								<< "), dest = (" << dstX << ", " << dstY
								<< "), transform = " << transform
								<< std::endl;
					}
					break;
				}

				default:
					*m_pOStream << "ERROR: Unknown command (" << int(cmd) << ")" << std::endl;
					break;
				}
			}

			m_pRectsPtr = pRects;
			m_pColorsPtr = pColors;
			m_pObjectsPtr = pObjects;
		}

		if (m_pBackend)
			m_pBackend->processCommands(pBeg, pEnd);
	}

	//____ canvasInfo() _______________________________________________________

	const CanvasInfo* BackendLogger::canvasInfo(CanvasRef ref) const
	{
		const CanvasInfo * pCanvasInfo = nullptr;

		if (m_pBackend)
			pCanvasInfo = m_pBackend->canvasInfo(ref);

		if( m_pOStream )
		{
			*m_pOStream << "Called canvasInfo() for CanvasRef: " << toString(ref);

			if (pCanvasInfo)
			{
				*m_pOStream << " which succeeded.";
			}
			else
			{
				*m_pOStream << " which failed.";
			}

			*m_pOStream << std::endl;
		}

		return pCanvasInfo;
	}

	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p BackendLogger::surfaceFactory()
	{
		SurfaceFactory_p pFactory = nullptr;

		if (m_pBackend)
			pFactory = m_pBackend->surfaceFactory();

		if( m_pOStream )
			*m_pOStream << "Called surfaceFactory(). Returned: " << pFactory.rawPtr() << std::endl;

		return pFactory;
	}

	//____ edgemapFactory() ___________________________________________________

	EdgemapFactory_p BackendLogger::edgemapFactory()
	{
		EdgemapFactory_p pFactory = nullptr;

		if (m_pBackend)
			pFactory = m_pBackend->edgemapFactory();

		if( m_pOStream )
			*m_pOStream << "Called edgemapFactory(). Returned: " << pFactory.rawPtr() << std::endl;

		return pFactory;
	}

	//____ maxEdges() _________________________________________________________

	int BackendLogger::maxEdges() const
	{
		int maxEdges = 0;

		if (m_pBackend)
			maxEdges = m_pBackend->maxEdges();

		if( m_pOStream )
			*m_pOStream << "Called maxEdges(). Returned: " << maxEdges << std::endl;

		return maxEdges;

	}

	//____ canBeBlitSource() __________________________________________________

	bool BackendLogger::canBeBlitSource(const TypeInfo& type) const
	{
		bool can = false;
		if (m_pBackend)
			can = m_pBackend->canBeBlitSource(type);
		if( m_pOStream )
			*m_pOStream << "Called canBeBlitSource(" << type.className << "). Returned: " << (can ? "true" : "false") << std::endl;
		return can;
	}

	//____ canBeCanvas() ______________________________________________________

	bool BackendLogger::canBeCanvas(const TypeInfo& type) const
	{
		bool can = false;
		if (m_pBackend)
			can = m_pBackend->canBeCanvas(type);
		if( m_pOStream )
			*m_pOStream << "Called canBeCanvas(" << type.className << "). Returned: " << (can ? "true" : "false") << std::endl;
		return can;
	}

	//____ waitForCompletion() ___________________________________________________

	void BackendLogger::waitForCompletion()
	{
		if (m_pBackend)
			m_pBackend->waitForCompletion();

		if( m_pOStream )
			*m_pOStream << "Called waitForCompletion()" << std::endl;
	}



	//____ setOStream() _______________________________________________________

	void BackendLogger::setOStream( std::ostream * pStream )
	{
		m_pOStream = pStream;
	}

	//____ _printRects() ______________________________________________________

	void BackendLogger::_printRects(std::ostream& stream, int nRects, const RectSPX* pRects)
	{
		for (int i = 0; i < nRects; i++)
		{
			stream << "        " << pRects[i].x << ", " << pRects[i].y << ", " << pRects[i].w << ", " << pRects[i].h << std::endl;
		}
	}



} // namespace wg
