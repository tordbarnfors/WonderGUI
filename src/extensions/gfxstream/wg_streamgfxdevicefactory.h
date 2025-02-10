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

#ifndef	WG_STREAMGFXDEVICEFACTORY_DOT_H
#define	WG_STREAMGFXDEVICEFACTORY_DOT_H
#pragma once

#include <wg_streambackend.h>
#include <wg_gfxdevicefactory.h>
#include <wg_geo.h>

namespace wg
{

	class StreamGfxDeviceFactory;
	typedef	StrongPtr<StreamGfxDeviceFactory>		StreamGfxDeviceFactory_p;
	typedef	WeakPtr<StreamGfxDeviceFactory>		StreamGfxDeviceFactory_wp;

	//____ StreamGfxDeviceFactory _____________________________________________________

	class StreamGfxDeviceFactory : public GfxDeviceFactory
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			//TODO: Add canvas definitions.

			StreamEncoder_p encoder;
			int 			maxEdges = 15;
		};


		//.____ Creation __________________________________________

		static StreamGfxDeviceFactory_p	create(const Blueprint& bp) { return StreamGfxDeviceFactory_p(new StreamGfxDeviceFactory(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		GfxDevice_p	createGfxDevice() override;

	protected:
		StreamGfxDeviceFactory(const Blueprint& bp);
		virtual ~StreamGfxDeviceFactory() {}

		StreamEncoder_p		m_pEncoder;
		int					m_maxEdges = 15;

	};


}


#endif //WG_SOFTGFXDEVICEFACTORY_DOT_H

