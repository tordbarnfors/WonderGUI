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
#include <wg_pluginbitmap.h>
#include <wg_pluginbitmapfactory.h>


namespace wg
{

	const TypeInfo PluginBitmapFactory::TYPEINFO = { "PluginBitmapFactory", &BitmapFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	PluginBitmapFactory::PluginBitmapFactory( wax_obj cFactory) : m_cFactory(cFactory)
	{
		/*
		 TODO: Solve this in a more correct way:
		 
		 We skip retaining/releasing the hosts BitmapFactory used by us to make curent
		 Softube close down sequence work where PluginBitmapFactory (due to a static pointer)
		 is destroyed AFTER the PluginCalls-structure has been cleared.
		 We therefore can not call release() in our destructor.

		 We therefore rely on the hosts real BitmapFactory not being deleted until
		 this class is not used by us anymore. This is safe to assume in the Softube GUI
		 framework, but could cause issues with other future uses of WonderGUI plugins
		 implementations.
		 */
		
//		PluginCalls::object->retain(cFactory);
	}

	//____ destructor ________________________________________________________

	PluginBitmapFactory::~PluginBitmapFactory()
	{
		//TODO: Solve this in a more correct way. See commen in constructor.
		
//		PluginCalls::object->release(m_cFactory);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginBitmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI PluginBitmapFactory::maxSize() const
	{			
		wax_sizeI sz = PluginCalls::bitmapFactory->maxBitmapSize(m_cFactory);
		return { sz.w, sz.h };
	}

	//____ createBitmap() ________________________________________________________

	Bitmap_p PluginBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint)
	{
		wax_bitmapBP	bp;
		PluginBitmap_p pBitmap = _prepareBitmap(&bp, &blueprint);

		return _attachBitmap( pBitmap, PluginCalls::bitmapFactory->createBitmap(m_cFactory, &bp) );
	}

	Bitmap_p PluginBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		wax_bitmapBP	bp;
		PluginBitmap_p pBitmap = _prepareBitmap(&bp, &blueprint);

		return _attachBitmap( pBitmap, PluginCalls::bitmapFactory->createBitmapFromBlob(m_cFactory, &bp, static_cast<Object*>(pBlob), pitch ) );
	}


	Bitmap_p PluginBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		wax_bitmapBP	bp;
		PluginBitmap_p pBitmap = _prepareBitmap(&bp, &blueprint);

		return _attachBitmap( pBitmap, PluginCalls::bitmapFactory->createBitmapFromBitmap(m_cFactory, &bp, pPixels, wax_pixelFormat(format), pitch, (const wax_color8*) pPalette, paletteSize) );
	}

	Bitmap_p PluginBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		wax_bitmapBP	bp;
		PluginBitmap_p pBitmap = _prepareBitmap(&bp, &blueprint);

		return _attachBitmap( pBitmap, PluginCalls::bitmapFactory->createBitmapFromRawData(m_cFactory, &bp, pPixels,
					(const wax_pixelDescription*) &pixelDescription, pitch, (const wax_color8*)pPalette, paletteSize) );
	}

	//____ _prepareBitmap() __________________________________________________

	// Makes the PluginBitmap and the Blueprint for the host. The host can't call the
	// restorer of a Blueprint, which takes a bitmap of ours, so it gets a function that
	// does, with the PluginBitmap to call it for.

	PluginBitmap_p PluginBitmapFactory::_prepareBitmap(wax_bitmapBP* pDest, const Bitmap::Blueprint* pSource)
	{
		PluginBitmap_p pBitmap = new PluginBitmap(pSource->restorer);

		_convertBlueprint(pDest, pSource);

		if( pSource->restorer )
		{
			pDest->restorer = PluginBitmap::_restoreFromHost;
			pDest->restorerData = pBitmap.rawPtr();
		}

		return pBitmap;
	}

	//____ _attachBitmap() ___________________________________________________

	Bitmap_p PluginBitmapFactory::_attachBitmap(PluginBitmap* pBitmap, wax_obj obj)
	{
		if( !obj )
			return nullptr;

		pBitmap->_attach(obj);
		PluginCalls::object->release(obj);
		return pBitmap;
	}

	//____ _convertBlueprint() ________________________________________________

	void PluginBitmapFactory::_convertBlueprint(wax_bitmapBP* pDest, const Bitmap::Blueprint* pSource)
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
		pDest->restorer = nullptr;				// Set by _prepareBitmap().
		pDest->restorerData = nullptr;
		pDest->sampleMethod = (wax_sampleMethod)pSource->sampleMethod;
		pDest->scale = pSource->scale;
		pDest->size.w = pSource->size.w;
		pDest->size.h = pSource->size.h;
		pDest->tiling = pSource->tiling;
	}

} // namespace wg
