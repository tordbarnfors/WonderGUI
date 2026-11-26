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
#ifndef WAX_BITMAPFACTORY_DOT_H
#define	WAX_BITMAPFACTORY_DOT_H
#pragma once
//==============================================================================

#include <wax_bitmap.h>
#include <wax_blob.h>


namespace wax
{


	class BitmapFactory;
	typedef	StrongPtr<BitmapFactory>	BitmapFactory_p;
	typedef	WeakPtr<BitmapFactory>	BitmapFactory_wp;


	//____ BitmapFactory _______________________________________________________
	/**
	 * @brief Factory class for creating bitmaps.
	 *
	 * BitmapFactory is the base class for all bitmap factories. The bitmap factories
	 * are used by Wax objects that needs to dynamically create bitmaps as
	 * part of their operation, like FreeTypeFont.
	 *
	 **/
	class BitmapFactory : public Object
	{
	public:

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		virtual SizeI		maxSize() const = 0;

		//.____ Misc _______________________________________________________

		// Pixels given to createBitmap() are in the color space of the blueprint. Byte order is the
		// one of the blueprint or, when a PixelDescription is given, of the PixelDescription.

		virtual Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint) = 0;
		virtual Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch = 0) = 0;
		virtual Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
										  const PixelDescription& pixelDescription, int pitch = 0, const Color8 * pPalette = nullptr, int paletteSize = 0) = 0;
		virtual Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										  const Color8 * pPalette = nullptr, int paletteSize = 0) = 0;

	protected:
		virtual ~BitmapFactory() {}
	};


	//==============================================================================

} // namespace wax
#endif // WAX_BITMAPFACTORY_DOT_H
