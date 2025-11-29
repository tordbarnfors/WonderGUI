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
#include <memory.h>

#include <wg_plugincalls.h>
#include <wg_pluginsurface.h>
#include <wg_pluginsurfacefactory.h>


namespace wg
{

	const TypeInfo PluginSurfaceFactory::TYPEINFO = { "PluginSurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	PluginSurfaceFactory::PluginSurfaceFactory( wg_obj cFactory) : m_cFactory(cFactory)
	{
		/*
		 TODO: Solve this in a more correct way:
		 
		 We skip retaining/releasing the hosts SurfaceFactory used by us to make curent
		 Softube close down sequence work where PluginSurfaceFactory (due to a static pointer)
		 is destroyed AFTER the PluginCalls-structure has been cleared.
		 We therefore can not call release() in our destructor.

		 We therefore rely on the hosts real SurfaceFactory not being deleted until
		 this class is not used by us anymore. This is safe to assume in the Softube GUI
		 framework, but could cause issues with other future uses of WonderGUI plugins
		 implementations.
		 */
		
//		PluginCalls::object->retain(cFactory);
	}

	//____ destructor ________________________________________________________

	PluginSurfaceFactory::~PluginSurfaceFactory()
	{
		//TODO: Solve this in a more correct way. See commen in constructor.
		
//		PluginCalls::object->release(m_cFactory);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginSurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI PluginSurfaceFactory::maxSize() const
	{			
		wg_sizeI sz = PluginCalls::surfaceFactory->maxSurfaceSize(m_cFactory);
		return { sz.w, sz.h };
	}

	//____ createSurface() ________________________________________________________

	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		wg_surfaceBP	bp;
		_convertBlueprint(&bp, &blueprint);

		auto obj = PluginCalls::surfaceFactory->createSurface(m_cFactory, &bp);
		auto pSurf = PluginSurface::create(obj);
		PluginCalls::object->release(obj);
		return pSurf;
	}

	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		wg_surfaceBP	bp;
		_convertBlueprint(&bp, &blueprint);

		auto obj = PluginCalls::surfaceFactory->createSurfaceFromBlob(m_cFactory, &bp, static_cast<Object*>(pBlob), pitch );
		auto pSurf = PluginSurface::create(obj);
		PluginCalls::object->release(obj);
		return pSurf;
	}


	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		wg_surfaceBP	bp;
		_convertBlueprint(&bp, &blueprint);

		auto obj = PluginCalls::surfaceFactory->createSurfaceFromBitmap(m_cFactory, &bp, pPixels, wg_pixelFormat(format), pitch, (const wg_color8*) pPalette, paletteSize);
		auto pSurf = PluginSurface::create(obj);
		PluginCalls::object->release(obj);
		return pSurf;
	}

	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		wg_surfaceBP	bp;
		_convertBlueprint(&bp, &blueprint);

		auto obj = PluginCalls::surfaceFactory->createSurfaceFromRawData(m_cFactory, &bp, pPixels, 
					(const wg_pixelDescription*) &pixelDescription, pitch, (const wg_color8*)pPalette, paletteSize);
		auto pSurf = PluginSurface::create(obj);
		PluginCalls::object->release(obj);
		return pSurf;
	}

	//____ _convertBlueprint() ________________________________________________

	void PluginSurfaceFactory::_convertBlueprint(wg_surfaceBP* pDest, const Surface::Blueprint* pSource)
	{
		pDest->buffered = pSource->buffered;
		pDest->canvas = pSource->canvas;
		pDest->palette = (wg_color8*)pSource->palette;
		pDest->paletteSize = pSource->paletteSize;
		pDest->paletteCapacity = pSource->paletteCapacity;
		pDest->dynamic = pSource->dynamic;
		pDest->format = (wg_pixelFormat)pSource->format;
		pDest->identity = pSource->identity;
		pDest->mipmap = pSource->mipmap;
		pDest->sampleMethod = (wg_sampleMethod)pSource->sampleMethod;
		pDest->scale = pSource->scale;
		pDest->size.w = pSource->size.w;
		pDest->size.h = pSource->size.h;
		pDest->tiling = pSource->tiling;
	}

} // namespace wg
