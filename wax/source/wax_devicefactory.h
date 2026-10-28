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
#ifndef WAX_DEVICEFACTORY_DOT_H
#define	WAX_DEVICEFACTORY_DOT_H
#pragma once
//==============================================================================

#include <wax_device.h>


namespace wax
{


	class DeviceFactory;
	typedef	StrongPtr<DeviceFactory>	DeviceFactory_p;
	typedef	WeakPtr<DeviceFactory>	DeviceFactory_wp;


	//____ DeviceFactory _______________________________________________________
	/**
	 * @brief Factory class for creating gfxdevices.
	 *
	 * DeviceFactory is the base class for all gfxdevice factories. The gfxdevice factories
	 * can be used to create a gfxdevice without knowing which one or the details of creating one.
	 *
	 **/
	class DeviceFactory : public Object
	{
	public:

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Misc _______________________________________________________

		virtual Device_p	createDevice() = 0;

	protected:
		virtual ~DeviceFactory() {}
	};


	//==============================================================================

} // namespace wax
#endif // WAX_DEVICEFACTORY_DOT_H
