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


#include <wax_vulkanbitmapfactory.h>

namespace wax
{



	const TypeInfo VulkanBitmapFactory::TYPEINFO = { "VulkanBitmapFactory", &BitmapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& VulkanBitmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ________________________________________________________________

	SizeI VulkanBitmapFactory::maxSize() const
	{
		return VulkanBitmap::maxSize();
	}


	//____ createBitmap() __________________________________________________________

	Bitmap_p VulkanBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint)
	{
		return VulkanBitmap::create(blueprint);
	}

	Bitmap_p VulkanBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return VulkanBitmap::create(blueprint, pBlob, pitch);
	}

	Bitmap_p VulkanBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		return VulkanBitmap::create(blueprint, pPixels, format, pitch, pPalette, paletteSize);
	}

	Bitmap_p VulkanBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		return VulkanBitmap::create(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);
	}


} // namespace wax
