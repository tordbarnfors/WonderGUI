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
#ifndef	WAX_SOFTDEVICEFACTORY_DOT_H
#define	WAX_SOFTDEVICEFACTORY_DOT_H
#pragma once

#include <wax_softbackend.h>
#include <wax_devicefactory.h>
#include <wax_geo.h>

namespace wax
{

	class SoftDeviceFactory;
	typedef	StrongPtr<SoftDeviceFactory>		SoftDeviceFactory_p;
	typedef	WeakPtr<SoftDeviceFactory>		SoftDeviceFactory_wp;

	//____ SoftDeviceFactory _____________________________________________________

	class SoftDeviceFactory : public DeviceFactory
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

		static SoftDeviceFactory_p	create(const Blueprint& bp) { return SoftDeviceFactory_p(new SoftDeviceFactory(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Device_p	createDevice() override;

	protected:
		SoftDeviceFactory(const Blueprint& bp);
		virtual ~SoftDeviceFactory() {}

		SoftSurface_p	m_pDefaultCanvasSurface;

		std::vector<bool(*)(SoftBackend*)>	m_kernels;
	};



}


#endif //WAX_SOFTDEVICEFACTORY_DOT_H

