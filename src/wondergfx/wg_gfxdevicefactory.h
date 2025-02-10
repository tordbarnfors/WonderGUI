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

#ifndef WG_GFXDEVICEFACTORY_DOT_H
#define	WG_GFXDEVICEFACTORY_DOT_H
#pragma once
//==============================================================================

#include <wg_gfxdevice.h>


namespace wg
{


	class GfxDeviceFactory;
	typedef	StrongPtr<GfxDeviceFactory>	GfxDeviceFactory_p;
	typedef	WeakPtr<GfxDeviceFactory>	GfxDeviceFactory_wp;


	//____ GfxDeviceFactory _______________________________________________________
	/**
	 * @brief Factory class for creating gfxdevices.
	 *
	 * GfxDeviceFactory is the base class for all gfxdevice factories. The gfxdevice factories
	 * can be used to create a gfxdevice without knowing which one or the details of creating one.
	 *
	 **/
	class GfxDeviceFactory : public Object
	{
	public:

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Misc _______________________________________________________

		virtual GfxDevice_p	createGfxDevice() = 0;

	protected:
		virtual ~GfxDeviceFactory() {}
	};


	//==============================================================================

} // namespace wg
#endif // WG_GFXDEVICEFACTORY_DOT_H
