/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free Ioware; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Ioware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_PLUGINSURFACE_DOT_H
#define	WG_PLUGINSURFACE_DOT_H
#pragma once


#include <wg_surface.h>

typedef void* wg_obj;
typedef struct wg_surfaceBP_struct wg_surfaceBP;

namespace wg
{

	class PluginSurface;
	typedef	StrongPtr<PluginSurface>	PluginSurface_p;
	typedef	WeakPtr<PluginSurface>	    PluginSurface_wp;

	//____ PluginSurface _____________________________________________________________

	class PluginSurface : public Surface
	{
		friend class PluginSurfaceFactory;

	public:

		//.____ Creation __________________________________________

		static PluginSurface_p	create( wg_obj object );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ______________________________________________________

		void				setScale(int scale) override;
		int					scale() const override;

        //.____ Content _______________________________________________________

		int					alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		const PixelBuffer	allocPixelBuffer(const RectI& rect) override;
		bool				pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				pullPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				freePixelBuffer(const PixelBuffer& buffer) override;

		//.____  Rendering ____________________________________________________

		bool				fill(HiColor col) override;
		bool				fill(const RectI& region, HiColor col ) override;

//		bool				copy(CoordI dest, Surface* pSrcSurf, const RectI& srcRect) override;		//The default implementation should work through the CABI-wall ...
//		bool				copy(CoordI dest, Surface* pSrcSurf) override;								//The default implementation should work through the CABI-wall ...

		//.____ Misc _________________________________________________________
		 
		int					addObserver(const std::function<void(int nRects, const RectSPX* pRects)>& func) override;
		bool				removeObserver(int observerId) override;

		inline wg_obj		cObject() { return m_cSurface; }

	private:
		PluginSurface( wg_obj object );
		~PluginSurface();

		wg_obj		m_cSurface;
		int			m_cObserverId = 0;

	};
} // namespace wg
#endif //WG_PLUGINSURFACE_DOT_H

