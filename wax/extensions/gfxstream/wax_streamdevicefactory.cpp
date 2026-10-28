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
#include "wax_streambackend.h"
#include "wax_streamdevicefactory.h"

#include "wax_device_gen2.h"


namespace wax
{

	const TypeInfo StreamDeviceFactory::TYPEINFO = { "StreamDeviceFactory", &DeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

StreamDeviceFactory::StreamDeviceFactory(const Blueprint& bp)
	{
		m_pEncoder = bp.encoder;
		m_maxEdges = bp.maxEdges;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createDevice() ________________________________________________________

	Device_p StreamDeviceFactory::createDevice()
	{
		auto pBackend = StreamBackend::create(m_pEncoder, m_maxEdges);

		//TODO: Define canvases

		pBackend->encodeCanvasList();

		auto pDevice = DeviceGen2::create(pBackend);
		return pDevice;
	}

} // namespace wax

