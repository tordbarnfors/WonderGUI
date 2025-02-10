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

#ifndef WG_METALGFXDEVICEFACTORY_DOT_H
#define	WG_METALGFXDEVICEFACTORY_DOT_H
#pragma once
//==============================================================================

#include <wg_gfxdevicefactory.h>


namespace wg
{
	class MetalGfxDeviceFactory;
	typedef	StrongPtr<MetalGfxDeviceFactory>	MetalGfxDeviceFactory_p;
	typedef	WeakPtr<MetalGfxDeviceFactory>		MetalGfxDeviceFactory_wp;


	//____ MetalGfxDeviceFactory _______________________________________________________

	class MetalGfxDeviceFactory : public GfxDeviceFactory
	{
	public:

		//.____ Blueprint _________________________________________

		struct Blueprint
		{
			PixelFormat					defaultCanvasPixelFormat = PixelFormat::Undefined;
			MTLRenderPassDescriptor *	defaultCanvasRenderPassDesc = nullptr;
			int 						defaultCanvasScale = 64;
			SizeI						defaultCanvasSize;
			id<MTLDevice> 				device = nil;
		};

		//.____ Creation __________________________________________

		static MetalGfxDeviceFactory_p	create( const Blueprint& blueprint ) { return MetalGfxDeviceFactory_p(new MetalGfxDeviceFactory(blueprint)); }

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Misc _______________________________________________________

		GfxDevice_p	createGfxDevice() override;

	protected:
		MetalGfxDeviceFactory(const Blueprint& bp);
		virtual ~MetalGfxDeviceFactory() {}

		id<MTLDevice>				m_device = nil;
		MTLRenderPassDescriptor * 	m_pDefaultCanvasRenderPassDesc = nullptr;
		SizeI						m_defaultCanvasSize;
		PixelFormat					m_defaultCanvasPixelFormat = PixelFormat::Undefined;
		int							m_defaultCanvasScale = 64;

	};


	//==============================================================================

} // namespace wg
#endif // WG_METALGFXDEVICEFACTORY_DOT_H
