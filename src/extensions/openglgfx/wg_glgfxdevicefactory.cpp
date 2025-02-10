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


#include "wg_glbackend.h"
#include "wg_glgfxdevicefactory.h"

#include "wg_gfxdevice_gen2.h"


namespace wg
{

	const TypeInfo GlGfxDeviceFactory::TYPEINFO = { "GlGfxDeviceFactory", &GfxDeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

	GlGfxDeviceFactory::GlGfxDeviceFactory(const Blueprint& bp)
	{
		m_defaultCanvasSize				= bp.defaultCanvasSize;
		m_defaultCanvasScale			= bp.defaultCanvasScale;
		m_uboBindingPoint				= bp.uboBindingPoint;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlGfxDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createGfxDevice() ________________________________________________________

	GfxDevice_p GlGfxDeviceFactory::createGfxDevice()
	{
		auto pBackend = GlBackend::create(m_uboBindingPoint);
		pBackend->setDefaultCanvas( m_defaultCanvasSize, m_defaultCanvasScale );

		auto pDevice = GfxDeviceGen2::create(pBackend);
		return pDevice;
	}


} // namespace wg
