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
#ifndef	WAX_METALBITMAPFACTORY_DOT_H
#define	WAX_METALBITMAPFACTORY_DOT_H
#pragma once

#include <wax_bitmapfactory.h>

namespace wax
{

	class MetalBitmapFactory;
	typedef	StrongPtr<MetalBitmapFactory>	MetalBitmapFactory_p;
	typedef	WeakPtr<MetalBitmapFactory>	MetalBitmapFactory_wp;

	//____ MetalBitmapFactory _____________________________________________________

	class MetalBitmapFactory : public BitmapFactory
	{
	public:
		//.____ Creation __________________________________________

		static MetalBitmapFactory_p	create() { return MetalBitmapFactory_p(new MetalBitmapFactory()); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint) override;
		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch) override;

		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
								  const PixelDescription& pixelDescription, int pitch = 0,
								  const Color8 * pPalette = nullptr, int paletteSize = 0) override;

		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
								  PixelFormat format = PixelFormat::Undefined, int pitch = 0,
								  const Color8 * pPalette = nullptr, int paletteSize = 0) override;
        
	protected:
		MetalBitmapFactory() {}
		~MetalBitmapFactory() {}

	};

}


#endif //WAX_METALBITMAPFACTORY_DOT_H

