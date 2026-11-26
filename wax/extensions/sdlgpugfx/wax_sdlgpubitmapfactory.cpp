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


#include <wax_sdlgpubitmapfactory.h>

namespace wax
{



	const TypeInfo SdlGpuBitmapFactory::TYPEINFO = { "SdlGpuBitmapFactory", &BitmapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SdlGpuBitmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ________________________________________________________________

	SizeI SdlGpuBitmapFactory::maxSize() const
	{
		return SdlGpuBitmap::maxSize();
	}


	//____ createBitmap() __________________________________________________________

	Bitmap_p SdlGpuBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint)
	{
		return SdlGpuBitmap::create(blueprint);
	}

	Bitmap_p SdlGpuBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return SdlGpuBitmap::create(blueprint, pBlob, pitch);
	}

	Bitmap_p SdlGpuBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		return SdlGpuBitmap::create(blueprint, pPixels, format, pitch, pPalette, paletteSize);
	}

	Bitmap_p SdlGpuBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		return SdlGpuBitmap::create(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);
	}


} // namespace wax
