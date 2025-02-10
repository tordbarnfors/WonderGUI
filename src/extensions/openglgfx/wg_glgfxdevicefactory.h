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

#ifndef	WG_GLGFXDEVICEFACTORY_DOT_H
#define	WG_GLGFXDEVICEFACTORY_DOT_H
#pragma once

#include <wg_gfxdevicefactory.h>
#include <wg_geo.h>

namespace wg
{

	class GlGfxDeviceFactory;
	typedef	StrongPtr<GlGfxDeviceFactory>		GlGfxDeviceFactory_p;
	typedef	WeakPtr<GlGfxDeviceFactory>	GlGfxDeviceFactory_wp;

	//____ GlGfxDeviceFactory _____________________________________________________

	class GlGfxDeviceFactory : public GfxDeviceFactory
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			int		defaultCanvasScale = 64;
			SizeSPX	defaultCanvasSize;
			int 	uboBindingPoint = 0;
		};


		//.____ Creation __________________________________________

		static GlGfxDeviceFactory_p	create(const Blueprint& bp) { return GlGfxDeviceFactory_p(new GlGfxDeviceFactory(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		GfxDevice_p	createGfxDevice() override;

	protected:
		GlGfxDeviceFactory(const Blueprint& bp);
		virtual ~GlGfxDeviceFactory() {}

		int			m_defaultCanvasScale = 64;
		SizeSPX		m_defaultCanvasSize;
		int			m_uboBindingPoint = 0;
	};



}


#endif //WG_GLGFXDEVICEFACTORY_DOT_H

