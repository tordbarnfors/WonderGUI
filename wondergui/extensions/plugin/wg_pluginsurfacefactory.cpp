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

	PluginSurfaceFactory::PluginSurfaceFactory( wax_obj cFactory) : m_cFactory(cFactory)
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
		wax_sizeI sz = PluginCalls::surfaceFactory->maxSurfaceSize(m_cFactory);
		return { sz.w, sz.h };
	}

	//____ createSurface() ________________________________________________________

	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		wax_surfaceBP	bp;
		PluginSurface_p pSurf = _prepareSurface(&bp, &blueprint);

		return _attachSurface( pSurf, PluginCalls::surfaceFactory->createSurface(m_cFactory, &bp) );
	}

	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		wax_surfaceBP	bp;
		PluginSurface_p pSurf = _prepareSurface(&bp, &blueprint);

		return _attachSurface( pSurf, PluginCalls::surfaceFactory->createSurfaceFromBlob(m_cFactory, &bp, static_cast<Object*>(pBlob), pitch ) );
	}


	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		wax_surfaceBP	bp;
		PluginSurface_p pSurf = _prepareSurface(&bp, &blueprint);

		return _attachSurface( pSurf, PluginCalls::surfaceFactory->createSurfaceFromBitmap(m_cFactory, &bp, pPixels, wax_pixelFormat(format), pitch, (const wax_color8*) pPalette, paletteSize) );
	}

	Surface_p PluginSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		wax_surfaceBP	bp;
		PluginSurface_p pSurf = _prepareSurface(&bp, &blueprint);

		return _attachSurface( pSurf, PluginCalls::surfaceFactory->createSurfaceFromRawData(m_cFactory, &bp, pPixels,
					(const wax_pixelDescription*) &pixelDescription, pitch, (const wax_color8*)pPalette, paletteSize) );
	}

	//____ _prepareSurface() __________________________________________________

	// Makes the PluginSurface and the Blueprint for the host. The host can't call the
	// restorer of a Blueprint, which takes a surface of ours, so it gets a function that
	// does, with the PluginSurface to call it for.

	PluginSurface_p PluginSurfaceFactory::_prepareSurface(wax_surfaceBP* pDest, const Surface::Blueprint* pSource)
	{
		PluginSurface_p pSurf = new PluginSurface(pSource->restorer);

		_convertBlueprint(pDest, pSource);

		if( pSource->restorer )
		{
			pDest->restorer = PluginSurface::_restoreFromHost;
			pDest->restorerData = pSurf.rawPtr();
		}

		return pSurf;
	}

	//____ _attachSurface() ___________________________________________________

	Surface_p PluginSurfaceFactory::_attachSurface(PluginSurface* pSurf, wax_obj obj)
	{
		if( !obj )
			return nullptr;

		pSurf->_attach(obj);
		PluginCalls::object->release(obj);
		return pSurf;
	}

	//____ _convertBlueprint() ________________________________________________

	void PluginSurfaceFactory::_convertBlueprint(wax_surfaceBP* pDest, const Surface::Blueprint* pSource)
	{
		pDest->byteOrder = pSource->bigEndian ? WAX_BYTEORDER_BIG_ENDIAN : WAX_BYTEORDER_LITTLE_ENDIAN;
		pDest->canvas = pSource->canvas;
		pDest->colorSpace = (wax_colorSpace)pSource->colorSpace;
		pDest->useFillColor = !pSource->fillColor.isUndefined();
		pDest->fillColor.b = pSource->fillColor.b;
		pDest->fillColor.g = pSource->fillColor.g;
		pDest->fillColor.r = pSource->fillColor.r;
		pDest->fillColor.a = pSource->fillColor.a;
		pDest->palette = (wax_color8*)pSource->palette;
		pDest->paletteSize = pSource->paletteSize;
		pDest->paletteCapacity = pSource->paletteCapacity;
		pDest->dynamic = pSource->dynamic;
		pDest->format = (wax_pixelFormat)pSource->format;
		pDest->identity = pSource->identity;
		pDest->mipmap = pSource->mipmap;
		pDest->restorer = nullptr;				// Set by _prepareSurface().
		pDest->restorerData = nullptr;
		pDest->sampleMethod = (wax_sampleMethod)pSource->sampleMethod;
		pDest->scale = pSource->scale;
		pDest->size.w = pSource->size.w;
		pDest->size.h = pSource->size.h;
		pDest->tiling = pSource->tiling;
	}

} // namespace wg
