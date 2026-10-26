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

#include "wg_metalsurface.h"
#include "wg_metalsurfacefactory.h"
#include <wg_gfxutil.h>



namespace wg
{

	const TypeInfo MetalSurfaceFactory::TYPEINFO = { "MetalSurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalSurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI MetalSurfaceFactory::maxSize() const
	{
		return MetalSurface::maxSize();
	}

	//____ createSurface() ________________________________________________________

	Surface_p MetalSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		return MetalSurface::create(blueprint);
	}

	Surface_p MetalSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return MetalSurface::create(blueprint, pBlob, pitch);
	}

	Surface_p MetalSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return MetalSurface::create(blueprint, pPixels, format, pitch, pPalette, paletteSize);
	}

	Surface_p MetalSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return MetalSurface::create(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);
	}

} // namespace wg
