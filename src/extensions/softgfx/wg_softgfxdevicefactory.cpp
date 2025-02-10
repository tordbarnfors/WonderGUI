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


#include "wg_softbackend.h"
#include "wg_softgfxdevicefactory.h"

#include "wg_gfxdevice_gen2.h"


namespace wg
{

	const TypeInfo SoftGfxDeviceFactory::TYPEINFO = { "SoftGfxDeviceFactory", &GfxDeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

	SoftGfxDeviceFactory::SoftGfxDeviceFactory(const Blueprint& bp)
	{
		m_pDefaultCanvasSurface			= bp.defaultCanvasSurface;
		m_kernels						= bp.kernels;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftGfxDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createGfxDevice() ________________________________________________________

	GfxDevice_p SoftGfxDeviceFactory::createGfxDevice()
	{
		auto pBackend = SoftBackend::create();

		for( auto func : m_kernels )
			func(pBackend);

		if( m_pDefaultCanvasSurface )
			pBackend->defineCanvas( CanvasRef::Default, m_pDefaultCanvasSurface);

		auto pDevice = GfxDeviceGen2::create(pBackend);
		return pDevice;
	}

} // namespace wg

