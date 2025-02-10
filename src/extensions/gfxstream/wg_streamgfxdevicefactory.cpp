
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


#include "wg_streambackend.h"
#include "wg_streamgfxdevicefactory.h"

#include "wg_gfxdevice_gen2.h"


namespace wg
{

	const TypeInfo StreamGfxDeviceFactory::TYPEINFO = { "StreamGfxDeviceFactory", &GfxDeviceFactory::TYPEINFO };

	//____ constructor ___________________________________________________________

StreamGfxDeviceFactory::StreamGfxDeviceFactory(const Blueprint& bp)
	{
		m_pEncoder = bp.encoder;
		m_maxEdges = bp.maxEdges;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamGfxDeviceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createGfxDevice() ________________________________________________________

	GfxDevice_p StreamGfxDeviceFactory::createGfxDevice()
	{
		auto pBackend = StreamBackend::create(m_pEncoder, m_maxEdges);

		//TODO: Define canvases

		pBackend->encodeCanvasList();

		auto pDevice = GfxDeviceGen2::create(pBackend);
		return pDevice;
	}

} // namespace wg

