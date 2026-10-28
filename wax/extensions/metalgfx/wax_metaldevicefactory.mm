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


#include "wax_metalbackend.h"
#include "wax_metaldevicefactory.h"

#include "wax_device_gen2.h"


namespace wax
{

	const TypeInfo MetalDeviceFactory::TYPEINFO = { "MetalDeviceFactory", &DeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

	MetalDeviceFactory::MetalDeviceFactory(const Blueprint& bp)
	{
		m_device 						= bp.device;
		m_pDefaultCanvasRenderPassDesc	= bp.defaultCanvasRenderPassDesc;
		m_defaultCanvasSize				= bp.defaultCanvasSize;
		m_defaultCanvasPixelFormat		= bp.defaultCanvasPixelFormat;
		m_defaultCanvasColorSpace		= bp.defaultCanvasColorSpace;
		m_defaultCanvasScale			= bp.defaultCanvasScale;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createDevice() ________________________________________________________

	Device_p MetalDeviceFactory::createDevice()
	{
		auto pBackend = MetalBackend::create();
		pBackend->setDefaultCanvas( m_pDefaultCanvasRenderPassDesc, m_defaultCanvasSize, m_defaultCanvasPixelFormat, m_defaultCanvasScale, m_defaultCanvasColorSpace );

		auto pDevice = DeviceGen2::create(pBackend);
		return pDevice;
	}


} // namespace wax
