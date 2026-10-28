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
#ifndef	WAX_GLDEVICEFACTORY_DOT_H
#define	WAX_GLDEVICEFACTORY_DOT_H
#pragma once

#include <wax_devicefactory.h>
#include <wax_geo.h>

namespace wax
{

	class GlDeviceFactory;
	typedef	StrongPtr<GlDeviceFactory>		GlDeviceFactory_p;
	typedef	WeakPtr<GlDeviceFactory>	GlDeviceFactory_wp;

	//____ GlDeviceFactory _____________________________________________________

	class GlDeviceFactory : public DeviceFactory
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			int		defaultCanvasScale = 64;
			SizeSPX	defaultCanvasSize;
			ColorSpace defaultCanvasColorSpace = ColorSpace::sRGB;	// sRGB needs an sRGB-capable default framebuffer.
			int 	uboBindingPoint = 0;
		};


		//.____ Creation __________________________________________

		static GlDeviceFactory_p	create(const Blueprint& bp) { return GlDeviceFactory_p(new GlDeviceFactory(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Device_p	createDevice() override;

	protected:
		GlDeviceFactory(const Blueprint& bp);
		virtual ~GlDeviceFactory() {}

		int			m_defaultCanvasScale = 64;
		SizeSPX		m_defaultCanvasSize;
		ColorSpace	m_defaultCanvasColorSpace = ColorSpace::sRGB;
		int			m_uboBindingPoint = 0;
	};



}


#endif //WAX_GLDEVICEFACTORY_DOT_H

