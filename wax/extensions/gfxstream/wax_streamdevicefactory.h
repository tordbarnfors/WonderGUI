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
#ifndef	WAX_STREAMDEVICEFACTORY_DOT_H
#define	WAX_STREAMDEVICEFACTORY_DOT_H
#pragma once

#include <wax_streambackend.h>
#include <wax_devicefactory.h>
#include <wax_geo.h>

namespace wax
{

	class StreamDeviceFactory;
	typedef	StrongPtr<StreamDeviceFactory>		StreamDeviceFactory_p;
	typedef	WeakPtr<StreamDeviceFactory>		StreamDeviceFactory_wp;

	//____ StreamDeviceFactory _____________________________________________________

	class StreamDeviceFactory : public DeviceFactory
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			//TODO: Add canvas definitions.

			StreamEncoder_p encoder;
			int 			maxEdges = 15;
		};


		//.____ Creation __________________________________________

		static StreamDeviceFactory_p	create(const Blueprint& bp) { return StreamDeviceFactory_p(new StreamDeviceFactory(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Device_p	createDevice() override;

	protected:
		StreamDeviceFactory(const Blueprint& bp);
		virtual ~StreamDeviceFactory() {}

		StreamEncoder_p		m_pEncoder;
		int					m_maxEdges = 15;

	};


}


#endif //WAX_SOFTDEVICEFACTORY_DOT_H

