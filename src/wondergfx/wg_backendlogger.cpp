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

	//____ beginRender()_______________________________________________

	void BackendLogger::beginRender()
	{
		*m_pStream << "BEGIN RENDER" << std::endl;

		if (m_pBackend)
			m_pBackend->beginRender();
	}

	//____ endRender()_______________________________________________

	void BackendLogger::endRender()
	{
		*m_pStream << "END RENDER" << std::endl;

		if( m_pBackend )
			m_pBackend->endRender();
	}

	//____ setCanvas()_______________________________________________

	void BackendLogger::setCanvas(Surface* pSurface)
	{
		*m_pStream	<< "SET CANVAS: Surface = id: " << pSurface->identity() 
					<< " pixelSize:  " << pSurface->pixelSize().w  << ", " << pSurface->pixelSize().h 
					<< " format: = " << toString(pSurface->pixelFormat()) << std::endl;

		if (m_pBackend)
			m_pBackend->setCanvas(pSurface);
	}

	void BackendLogger::setCanvas(CanvasRef ref)
	{

		*m_pStream << "SET CANVAS: Ref = " << toString(ref) << std::endl;

		if (m_pBackend)
			m_pBackend->setCanvas(ref);
	}

	//____ setObjects()_______________________________________________

	void BackendLogger::setObjects(Object** pBeg, Object** pEnd)
	{
		*m_pStream << "SET OBJECTS: Amount = " << int(pEnd -pBeg) << std::endl;

		Object** p = pBeg;

		while (p < pEnd)
		{
			auto pObj = *p++;
			*m_pStream << pObj << " (" << pObj->typeInfo().className << ")" << std::endl;
		}

		if (m_pBackend)
			m_pBackend->setObjects(pBeg,pEnd);
	}

	//____ setCoords()_______________________________________________

	void BackendLogger::setCoords(spx* pBeg, spx* pEnd)
	{
		*m_pStream << "SET COORDS: Amount = " << int(pEnd - pBeg) << std::endl;

		spx * p = pBeg;

		int rows = 0;

		while (p < pEnd)
		{
			rows %= 10;

			if (rows == 0)
				*m_pStream << std::endl;
			else
				*m_pStream << ", ";

			*m_pStream << *p++;

			rows++;
		}

		*m_pStream << std::endl;

		if (m_pBackend)
			m_pBackend->setCoords(pBeg, pEnd);
	}

	//____ setTransforms()_______________________________________________

	void BackendLogger::setTransforms(Transform* pBeg, Transform* pEnd)
	{
		*m_pStream << "SET TRANSFORMS: Amount = " << int(pEnd - pBeg) << std::endl;

		Transform * p = pBeg;

		while (p < pEnd)
		{
			*m_pStream << p->xx << ", " << p->xy << std::endl;
			*m_pStream << p->yx << ", " << p->yy << std::endl;
			*m_pStream << std::endl;
		}

		if (m_pBackend)
			m_pBackend->setTransforms(pBeg, pEnd);
	}

	//____ processCommands()_______________________________________________

	void BackendLogger::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
		*m_pStream << "PROCESS COMMANDS:" << std::endl;

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
				*m_pStream << "StateChange" << std::endl;

				int32_t statesChanged = *p++;

				if (statesChanged & uint8_t(StateChange::BlitSource))
				{
					int32_t objectOfs = *p++;

					*m_pStream << "BlitSource: " << objectOfs << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					BlendMode mode = (BlendMode)*p++;

					*m_pStream << "BlendMode: " << toString(mode) << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					int32_t r = *p++;
					int32_t g = *p++;
					int32_t b = *p++;
					int32_t a = *p++;

					*m_pStream << "TintColor: " << r << ", " << g << ", " << b << ", " << a << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{
					int32_t objectOfs = *p++;
					int32_t	x = *p++;
					int32_t	y = *p++;
					int32_t	w = *p++;
					int32_t	h = *p++;

					*m_pStream << "TintMap: " << objectOfs << " rect: " << x << ", " << y << ", " << w << ", " << "h" << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					float morphFactor = (*p++) / 4096.f;

					*m_pStream << "MorphFactor: " << morphFactor << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					int32_t r = *p++;
					int32_t g = *p++;
					int32_t b = *p++;
					int32_t a = *p++;

					*m_pStream << "FixedBlendColor: " << r << ", " << g << ", " << b << ", " << a << std::endl;
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					//TODO: Implement!!!	

					assert(false);
				}

				break;
			}

			case Command::Fill:
			{
				int32_t nRects = *p++;

				int32_t r = *p++;
				int32_t g = *p++;
				int32_t b = *p++;
				int32_t a = *p++;

				*m_pStream << "Fill: " << nRects << " rects with: " << r << ", " << g << ", " << b << ", " << a << std::endl;

				break;
			}

			case Command::FillSubPixel:
			case Command::Plot:
			case Command::Line:
			case Command::Blur:
			case Command::DrawEdgemap:
			{
				//TODO: Implement!!!	

				assert(false);
				break;
			}

			case Command::Blit:
			{
				int32_t nRects = *p++;
				int32_t transform = *p++;

				int32_t srcX = *p++;
				int32_t srcY = *p++;
				int32_t dstX = *p++;
				int32_t dstY = *p++;

				*m_pStream << "Blit: " << nRects << " rects with transform: " << transform << ", src: " << srcX << ", " << srcY << " dest: " << dstX << ", " << dstY << std::endl;

				break;
			}

			default:
				*m_pStream << "ERROR: Unknown command (" << int(cmd) << ")" << std::endl;
				break;
			}
		}


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




} // namespace wg
