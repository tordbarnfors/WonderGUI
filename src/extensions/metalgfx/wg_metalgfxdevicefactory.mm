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


#include "wg_metalbackend.h"
#include "wg_metalgfxdevicefactory.h"

#include "wg_gfxdevice_gen2.h"


namespace wg
{

	const TypeInfo MetalGfxDeviceFactory::TYPEINFO = { "MetalGfxDeviceFactory", &GfxDeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

	MetalGfxDeviceFactory::MetalGfxDeviceFactory(const Blueprint& bp)
	{
		m_device 						= bp.device;
		m_pDefaultCanvasRenderPassDesc	= bp.defaultCanvasRenderPassDesc;
		m_defaultCanvasSize				= bp.defaultCanvasSize;
		m_defaultCanvasPixelFormat		= bp.defaultCanvasPixelFormat;
		m_defaultCanvasScale			= bp.defaultCanvasScale;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalGfxDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createGfxDevice() ________________________________________________________

	GfxDevice_p MetalGfxDeviceFactory::createGfxDevice()
	{
		auto pBackend = MetalBackend::create();
		pBackend->setDefaultCanvas( m_pDefaultCanvasRenderPassDesc, m_defaultCanvasSize, m_defaultCanvasPixelFormat, m_defaultCanvasScale );

		auto pDevice = GfxDeviceGen2::create(pBackend);
		return pDevice;
	}


} // namespace wg
