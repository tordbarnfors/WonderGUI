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

#include <wg_softbackend.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>


namespace wg
{
	const TypeInfo SoftBackend::TYPEINFO = { "SoftBackend", &GfxBackend::TYPEINFO };

	//____ create() _____________________________________________

	SoftBackend_p SoftBackend::create()
	{
		return SoftBackend_p(new SoftBackend());
	}

	//____ constructor _____________________________________________

	SoftBackend::SoftBackend()
	{

	}

	//____ destructor _____________________________________________

	SoftBackend::~SoftBackend()
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ () _____________________________________________

	void SoftBackend::beginRender()
	{
	}

	//____ () _____________________________________________

	void SoftBackend::endRender()
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setCanvas(Surface* pSurface)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setCanvas(CanvasRef ref)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setObjects(Object** pBeg, Object** pEnd)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setCoords(spx* pBeg, spx* pEnd)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setTransforms(Transform* pBeg, Transform* pEnd)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
	}

	//____ defineCanvas() _____________________________________________

	bool SoftBackend::defineCanvas(CanvasRef ref, SoftSurface* pSurface)
	{

		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](CanvasInfo& entry) { return (ref == entry.ref); });

		if (it == m_definedCanvases.end())
		{
			if (pSurface)
				m_definedCanvases.push_back(CanvasInfo(ref, pSurface, pSurface->pixelSize() * 64, pSurface->pixelFormat(), pSurface->scale()));
		}
		else
		{
			if (pSurface)
			{
				it->pSurface = pSurface;
				it->size = pSurface->pixelSize() * 64;
				it->scale = pSurface->scale();
				it->format = pSurface->pixelFormat();
			}
			else
			{
				m_definedCanvases.erase(it);
			}
		}

		return true;
	}

	//____ canvasInfo() _____________________________________________

	const CanvasInfo* SoftBackend::canvasInfo(CanvasRef ref) const
	{
		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](const CanvasInfo& entry) { return (ref == entry.ref); });

		if (it != m_definedCanvases.end())
			return &*it;

		return nullptr;
	}


	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p SoftBackend::surfaceFactory()
	{
		if (!m_pSurfaceFactory)
			m_pSurfaceFactory = SoftSurfaceFactory::create();

		return m_pSurfaceFactory;
	}


	//____ edgemapFactory() ___________________________________________________

	EdgemapFactory_p SoftBackend::edgemapFactory()
	{
		if (!m_pEdgemapFactory)
			m_pEdgemapFactory = SoftEdgemapFactory::create();

		return m_pEdgemapFactory;
	}


	//____ () _____________________________________________

	int SoftBackend::maxEdges() const
	{

	}



} // namespace wg
