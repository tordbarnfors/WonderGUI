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

#include <memory.h>

#include "wax_metalbitmap.h"
#include "wax_metalbitmapfactory.h"
#include <wax_util.h>



namespace wax
{

	const TypeInfo MetalBitmapFactory::TYPEINFO = { "MetalBitmapFactory", &BitmapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalBitmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI MetalBitmapFactory::maxSize() const
	{
		return MetalBitmap::maxSize();
	}

	//____ createBitmap() ________________________________________________________

	Bitmap_p MetalBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint)
	{
		return MetalBitmap::create(blueprint);
	}

	Bitmap_p MetalBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return MetalBitmap::create(blueprint, pBlob, pitch);
	}

	Bitmap_p MetalBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
												PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return MetalBitmap::create(blueprint, pPixels, format, pitch, pPalette, paletteSize);
	}

	Bitmap_p MetalBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return MetalBitmap::create(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);
	}

} // namespace wax
