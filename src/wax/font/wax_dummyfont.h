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
#ifndef	WAX_DUMMYFONT_DOT_H
#define	WAX_DUMMYFONT_DOT_H
#pragma once

#include <wax_geo.h>
#include <wax_font.h>



namespace wax
{
	class DummyFont;
	typedef	StrongPtr<DummyFont>	DummyFont_p;
	typedef	WeakPtr<DummyFont>		DummyFont_wp;

	class DummyFont : public Font
	{
	public:
		//.____ Creation __________________________________________

		static DummyFont_p	create() { return DummyFont_p(new DummyFont()); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ____________________________________________

		bool			setSize( spx size ) override;
		spx				size() override;

		void				getGlyphWithoutBitmap(char32_t chr, Glyph& glyph) override;
		void				getGlyphWithBitmap(char32_t chr, Glyph& glyph) override;
		spx					kerning(Glyph& leftGlyph, Glyph& rightGlyph) override;

		spx				lineGap() override;
		spx				whitespaceAdvance() override;
		spx 			maxAdvance() override;
		spx				maxAscend() override;
		spx				maxDescend() override;

		//.____ Misc ___________________________________________________________

		int				nbGlyphs() override;
		bool			hasGlyphs() override;
		bool			isMonospace() override;
		bool			isMonochrome() override;
		bool			hasGlyph( char32_t chr ) override;

	protected:
		DummyFont();
		~DummyFont();

	};

}

#endif // WAX_DUMMYFONT_DOT_H
