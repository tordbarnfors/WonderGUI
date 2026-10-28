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
#ifndef WAX_METALDEVICEFACTORY_DOT_H
#define	WAX_METALDEVICEFACTORY_DOT_H
#pragma once
//==============================================================================

#include <wax_devicefactory.h>


namespace wax
{
	class MetalDeviceFactory;
	typedef	StrongPtr<MetalDeviceFactory>	MetalDeviceFactory_p;
	typedef	WeakPtr<MetalDeviceFactory>		MetalDeviceFactory_wp;


	//____ MetalDeviceFactory _______________________________________________________

	class MetalDeviceFactory : public DeviceFactory
	{
	public:

		//.____ Blueprint _________________________________________

		struct Blueprint
		{
			PixelFormat					defaultCanvasPixelFormat = PixelFormat::Undefined;	// ARGB_8, XRGB_8 or Alpha_8. Undefined means ARGB_8.
			ColorSpace					defaultCanvasColorSpace = ColorSpace::sRGB;
			MTLRenderPassDescriptor *	defaultCanvasRenderPassDesc = nullptr;
			int 						defaultCanvasScale = 64;
			SizeI						defaultCanvasSize;
			id<MTLDevice> 				device = nil;
		};

		//.____ Creation __________________________________________

		static MetalDeviceFactory_p	create( const Blueprint& blueprint ) { return MetalDeviceFactory_p(new MetalDeviceFactory(blueprint)); }

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Misc _______________________________________________________

		Device_p	createDevice() override;

	protected:
		MetalDeviceFactory(const Blueprint& bp);
		virtual ~MetalDeviceFactory() {}

		id<MTLDevice>				m_device = nil;
		MTLRenderPassDescriptor * 	m_pDefaultCanvasRenderPassDesc = nullptr;
		SizeI						m_defaultCanvasSize;
		PixelFormat					m_defaultCanvasPixelFormat = PixelFormat::Undefined;
		ColorSpace					m_defaultCanvasColorSpace = ColorSpace::sRGB;
		int							m_defaultCanvasScale = 64;

	};


	//==============================================================================

} // namespace wax
#endif // WAX_METALDEVICEFACTORY_DOT_H
