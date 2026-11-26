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
#ifndef	WG_PLUGINBITMAPFACTORY_DOT_H
#define	WG_PLUGINBITMAPFACTORY_DOT_H
#pragma once

#include <wax_bitmapfactory.h>
#include <wg_pluginbitmap.h>
#include <wg_types.h>

#include <vector>

typedef void* wax_obj;
typedef struct wax_bitmapBP_struct wax_bitmapBP;

namespace wg
{

	class PluginBitmapFactory;
	typedef	StrongPtr<PluginBitmapFactory>	PluginBitmapFactory_p;
	typedef	WeakPtr<PluginBitmapFactory>		PluginBitmapFactory_wp;

	//____ PluginBitmapFactory _____________________________________________________

	class PluginBitmapFactory : public BitmapFactory
	{
	public:
		//.____ Creation __________________________________________

		static PluginBitmapFactory_p	create(wax_obj cFactory) { return PluginBitmapFactory_p(new PluginBitmapFactory(cFactory)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Bitmap_p	createBitmap(	const Bitmap::Blueprint& blueprint) override;
		Bitmap_p	createBitmap(	const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch) override;

		Bitmap_p	createBitmap(	const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
									const PixelDescription& pixelDescription, int pitch = 0, 
									const Color8* pPalette = nullptr, int paletteSize = 0) override;

		Bitmap_p	createBitmap(	const Bitmap::Blueprint& blueprint, const uint8_t* pPixels, 
									PixelFormat format = PixelFormat::Undefined, int pitch = 0,
									const Color8* pPalette = nullptr, int paletteSize = 0) override;

	protected:
		PluginBitmapFactory( wax_obj cFactory );
		~PluginBitmapFactory();

		void			_convertBlueprint(wax_bitmapBP* pDest, const Bitmap::Blueprint* pSource);
		PluginBitmap_p	_prepareBitmap(wax_bitmapBP* pDest, const Bitmap::Blueprint* pSource);
		Bitmap_p		_attachBitmap(PluginBitmap* pBitmap, wax_obj obj);


		wax_obj	m_cFactory;
	};
}


#endif //WG_PLUGINBITMAPFACTORY_DOT_H

