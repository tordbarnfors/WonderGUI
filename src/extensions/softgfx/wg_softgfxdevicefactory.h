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

#ifndef	WG_SOFTGFXDEVICEFACTORY_DOT_H
#define	WG_SOFTGFXDEVICEFACTORY_DOT_H
#pragma once

#include <wg_softbackend.h>
#include <wg_gfxdevicefactory.h>
#include <wg_geo.h>

namespace wg
{

	class SoftGfxDeviceFactory;
	typedef	StrongPtr<SoftGfxDeviceFactory>		SoftGfxDeviceFactory_p;
	typedef	WeakPtr<SoftGfxDeviceFactory>		SoftGfxDeviceFactory_wp;

	//____ SoftGfxDeviceFactory _____________________________________________________

	class SoftGfxDeviceFactory : public GfxDeviceFactory
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			//TODO: Replace defaultCanvasSurface with flexible canvas definitions.

			SoftSurface_p								defaultCanvasSurface;
			std::vector<bool(*)(SoftBackend*)>			kernels;
		};


		//.____ Creation __________________________________________

		static SoftGfxDeviceFactory_p	create(const Blueprint& bp) { return SoftGfxDeviceFactory_p(new SoftGfxDeviceFactory(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		GfxDevice_p	createGfxDevice() override;

	protected:
		SoftGfxDeviceFactory(const Blueprint& bp);
		virtual ~SoftGfxDeviceFactory() {}

		SoftSurface_p	m_pDefaultCanvasSurface;

		std::vector<bool(*)(SoftBackend*)>	m_kernels;
	};



}


#endif //WG_SOFTGFXDEVICEFACTORY_DOT_H

