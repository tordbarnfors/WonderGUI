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
#include "wax_softbackend.h"
#include "wax_softdevicefactory.h"

#include "wax_device_gen2.h"


namespace wax
{

	const TypeInfo SoftDeviceFactory::TYPEINFO = { "SoftDeviceFactory", &DeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

	SoftDeviceFactory::SoftDeviceFactory(const Blueprint& bp)
	{
		m_pDefaultCanvasSurface			= bp.defaultCanvasSurface;
		m_kernels						= bp.kernels;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createDevice() ________________________________________________________

	Device_p SoftDeviceFactory::createDevice()
	{
		auto pBackend = SoftBackend::create();

		for( auto func : m_kernels )
			func(pBackend);

		if( m_pDefaultCanvasSurface )
			pBackend->defineCanvas( CanvasRef::Default, m_pDefaultCanvasSurface);

		auto pDevice = DeviceGen2::create(pBackend);
		return pDevice;
	}

} // namespace wax

