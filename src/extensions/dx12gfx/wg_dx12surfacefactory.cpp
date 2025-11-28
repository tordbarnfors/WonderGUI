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

#include <wg_dx12surfacefactory.h>

namespace wg
{



	const TypeInfo DX12SurfaceFactory::TYPEINFO = { "DX12SurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12SurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ________________________________________________________________

	SizeI DX12SurfaceFactory::maxSize() const
	{
		return DX12Surface::maxSize();
	}


	//____ createSurface() __________________________________________________________

	Surface_p DX12SurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		return DX12Surface::create(blueprint);
	}

	Surface_p DX12SurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return DX12Surface::create(blueprint, pBlob, pitch);
	}

	Surface_p DX12SurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		return DX12Surface::create(blueprint, pPixels, format, pitch, pPalette, paletteSize);
	}

	Surface_p DX12SurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		return DX12Surface::create(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);
	}


} // namespace wg
