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
#include <wg_softsurfacefactory.h>

namespace wg
{



	const TypeInfo SoftSurfaceFactory::TYPEINFO = { "SoftSurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftSurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ________________________________________________________________

	SizeI SoftSurfaceFactory::maxSize() const
	{
		return SoftSurface::maxSize();
	}


	//____ createSurface() __________________________________________________________

	Surface_p SoftSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		return SoftSurface::create(blueprint);
	}

	Surface_p SoftSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return SoftSurface::create(blueprint, pBlob, pitch);
	}

	Surface_p SoftSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return SoftSurface::create(blueprint, pPixels, format, pitch, pPalette, paletteSize);
	}

	Surface_p SoftSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return SoftSurface::create(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);
	}


} // namespace wg
