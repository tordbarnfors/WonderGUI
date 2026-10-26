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
#ifndef WG_PLUGINFONT_DOT_H
#define WG_PLUGINFONT_DOT_H
#pragma once


#include <wg_c_types.h>
#include <wax_font.h>
#include <wg_types.h>


namespace wg
{
	class	PluginFont;
	typedef	StrongPtr<PluginFont>	PluginFont_p;
	typedef	WeakPtr<PluginFont>		PluginFont_wp;

	class PluginFont : public Font
	{
	public:


		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ______________________________________________________

		bool		setSize( spx size ) override;
		spx			size() override;

		void		getGlyphWithoutBitmap(char32_t chr, Glyph& glyph) override;
		spx			kerning(Glyph& leftGlyph, Glyph& rightGlyph) override;

		spx			lineGap() override;
		spx			whitespaceAdvance() override;
		spx			maxAdvance() override;
		spx			maxAscend() override;
		spx			maxDescend() override;

		//.____ Misc ___________________________________________________________

		int			nbGlyphs() override;
		bool		hasGlyphs() override;
		bool		hasGlyph( char32_t chr ) override;
		bool		isMonospace() override;

		inline wax_obj	cObject() { return m_cFont; }


	protected:
		PluginFont( wax_obj cFont );
		~PluginFont();

		wax_obj		m_cFont;
	};



} // namespace wg

#endif //WG_PLUGINFONT_DOT_H
