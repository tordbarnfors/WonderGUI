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
#ifndef	WG_METALSURFACEFACTORY_DOT_H
#define	WG_METALSURFACEFACTORY_DOT_H
#pragma once

#include <wg_surfacefactory.h>

namespace wg
{

	class MetalSurfaceFactory;
	typedef	StrongPtr<MetalSurfaceFactory>	MetalSurfaceFactory_p;
	typedef	WeakPtr<MetalSurfaceFactory>	MetalSurfaceFactory_wp;

	//____ MetalSurfaceFactory _____________________________________________________

	class MetalSurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static MetalSurfaceFactory_p	create() { return MetalSurfaceFactory_p(new MetalSurfaceFactory()); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Surface_p	createSurface(const Surface::Blueprint& blueprint) override;
		Surface_p	createSurface(const Surface::Blueprint& blueprint, Blob* pBlob, int pitch) override;

		Surface_p	createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
								  const PixelDescription& pixelDescription, int pitch = 0,
								  const Color8 * pPalette = nullptr, int paletteSize = 0) override;

		Surface_p	createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
								  PixelFormat format = PixelFormat::Undefined, int pitch = 0,
								  const Color8 * pPalette = nullptr, int paletteSize = 0) override;
        
	protected:
		MetalSurfaceFactory() {}
		~MetalSurfaceFactory() {}

	};

}


#endif //WG_METALSURFACEFACTORY_DOT_H

