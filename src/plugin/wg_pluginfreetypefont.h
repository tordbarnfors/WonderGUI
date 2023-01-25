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


#ifndef WG_PLUGINFREETYPEFONT_DOT_H
#define WG_PLUGINFREETYPEFONT_DOT_H
#pragma once


#include <wg_pluginfont.h>
#include <wg_pluginbitmapcache.h>

namespace wg
{
	class	PluginFreeTypeFont;
	typedef	StrongPtr<PluginFreeTypeFont>	PluginFreeTypeFont_p;
	typedef	WeakPtr<PluginFreeTypeFont>		PluginFreeTypeFont_wp;

	class PluginFreeTypeFont : public PluginFont
	{
	public:


		//.____ Creation __________________________________________

		static PluginFreeTypeFont_p	create(wg_obj cFreeTypeFont, PluginBitmapCache* pCache);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ______________________________________________________

		void		getGlyphWithBitmap(uint16_t chr, Glyph& glyph) override;


	private:
		PluginFreeTypeFont( wg_obj cFont, PluginBitmapCache * pCache );
		~PluginFreeTypeFont();
		
		PluginBitmapCache_p m_pCache;
		
	};



} // namespace wg

#endif //WG_CABIFREETYPEFONT_DOT_H
