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

#ifndef	WG_PLUGINSURFACEFACTORY_DOT_H
#define	WG_PLUGINSURFACEFACTORY_DOT_H
#pragma once

#include <wg_surfacefactory.h>

#include <vector>

typedef void* wg_obj;
typedef struct wg_surfaceBP_struct wg_surfaceBP;

namespace wg
{

	class PluginSurfaceFactory;
	typedef	StrongPtr<PluginSurfaceFactory>	PluginSurfaceFactory_p;
	typedef	WeakPtr<PluginSurfaceFactory>		PluginSurfaceFactory_wp;

	//____ PluginSurfaceFactory _____________________________________________________

	class PluginSurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static PluginSurfaceFactory_p	create(wg_obj cFactory) { return PluginSurfaceFactory_p(new PluginSurfaceFactory(cFactory)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Surface_p	createSurface(	const Surface::Blueprint& blueprint) override;
		Surface_p	createSurface(	const Surface::Blueprint& blueprint, Blob* pBlob, int pitch) override;

		Surface_p	createSurface(	const Surface::Blueprint& blueprint, const uint8_t* pPixels,
									const PixelDescription2& pixelDescription, int pitch = 0, 
									const Color8* pPalette = nullptr) override;

		Surface_p	createSurface(	const Surface::Blueprint& blueprint, const uint8_t* pPixels, 
									PixelFormat format = PixelFormat::Undefined, int pitch = 0,
									const Color8* pPalette = nullptr) override;

	protected:
		PluginSurfaceFactory( wg_obj cFactory );
		~PluginSurfaceFactory();

		void _convertBlueprint(wg_surfaceBP* pDest, const Surface::Blueprint* pSource);


		wg_obj	m_cFactory;
	};
}


#endif //WG_PLUGINSURFACEFACTORY_DOT_H

