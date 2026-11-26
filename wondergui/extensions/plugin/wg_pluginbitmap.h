/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#ifndef	WG_PLUGINBITMAP_DOT_H
#define	WG_PLUGINBITMAP_DOT_H
#pragma once


#include <wax_bitmap.h>
#include <wg_types.h>

typedef void* wax_obj;
typedef struct wax_bitmapBP_struct wax_bitmapBP;

namespace wg
{

	class PluginBitmap;
	typedef	StrongPtr<PluginBitmap>	PluginBitmap_p;
	typedef	WeakPtr<PluginBitmap>	    PluginBitmap_wp;

	//____ PluginBitmap _____________________________________________________________

	class PluginBitmap : public Bitmap
	{
		friend class PluginBitmapFactory;

	public:

		//.____ Creation __________________________________________

		static PluginBitmap_p	create( wax_obj object );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ______________________________________________________

		int					scale() const override;

        //.____ Content _______________________________________________________

		int					alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		const PixelBuffer	allocPixelBuffer(const RectI& rect) override;
		bool				pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true) override;
		void				freePixelBuffer(const PixelBuffer& buffer) override;

		//.____  Rendering ____________________________________________________

		bool				fill(Color col) override;
		bool				fill(const RectI& region, Color col ) override;

//		bool				copy(CoordI dest, Bitmap* pSrcBitmap, const RectI& srcRect) override;		//The default implementation should work through the CABI-wall ...
//		bool				copy(CoordI dest, Bitmap* pSrcBitmap) override;								//The default implementation should work through the CABI-wall ...

		//.____ Misc _________________________________________________________

		int					addObserver(const std::function<void(int nRects, const RectI* pRects)>& func) override;
		bool				removeObserver(int observerId) override;

		bool				isContentLost() const override;

		inline wax_obj		cObject() { return m_cBitmap; }

	private:

		// A PluginBitmap is made ahead of the bitmap of the host and attached to it after,
		// as the host gets a pointer to it with the Blueprint when there is a restorer.

		PluginBitmap( const Restorer& restorer );
		~PluginBitmap();

		void			_attach( wax_obj object );
		static uint8_t	_restoreFromHost( wax_obj bitmap, void * pData );

		wax_obj		m_cBitmap = nullptr;
		int			m_cObserverId = 0;

	};
} // namespace wg
#endif //WG_PLUGINBITMAP_DOT_H

