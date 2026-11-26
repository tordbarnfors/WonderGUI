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
#ifndef	WAX_SYSFONT_DOT_H
#define	WAX_SYSFONT_DOT_H
#pragma once

#include <wax_geo.h>
#include <wax_font.h>
#include <wax_bitmapfactory.h>

namespace wax
{

	class SysFont;
	typedef	StrongPtr<SysFont>	SysFont_p;
	typedef	WeakPtr<SysFont>	SysFont_wp;

	//____ SysFont _____________________________________________________________

	class SysFont : public Font
	{
	public:
		//.____ Creation __________________________________________

		static SysFont_p	create( BitmapFactory * pFactory, Font * pBackupFont = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ______________________________________________________

		inline bool		setSize(spx size) override { return (size == 14); };						// SizeI is just a dummy for SysFont...
		inline spx		size() override { return 14; }

		void			getGlyphWithoutBitmap(char32_t chr, Glyph& glyph) override;
		void			getGlyphWithBitmap(char32_t chr, Glyph& glyph) override;
		spx				kerning(Glyph& leftGlyph, Glyph& rightGlyph) override;

		inline spx		lineGap() override { return 2*64; }
		inline spx		whitespaceAdvance() override { return 14*64; }
		inline spx		maxAdvance() override { return 14*64; }
		inline spx 		maxAscend() override { return 12*64; }
		inline spx 		maxDescend() override { return 2*64; }

		//.____ Misc ___________________________________________________________


		inline int		nbGlyphs() override { return 32*3; }
		inline bool		hasGlyphs() override { return true; }
		bool			hasGlyph(char32_t chr) override;
		inline bool		isMonospace() override { return true; }
		inline bool		isMonochrome() override { return true; }

	protected:
		SysFont(BitmapFactory * pFactory, Font * pBackupFont);
		~SysFont();

		Bitmap_p		m_pBitmap;
	};



} // namespace wax
#endif // WAX_SYSFONT_DOT_H
