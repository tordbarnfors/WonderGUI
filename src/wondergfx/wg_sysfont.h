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
#ifndef	WG_SYSFONT_DOT_H
#define	WG_SYSFONT_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_font.h>
#include <wg_surfacefactory.h>

namespace wg
{

	class SysFont;
	typedef	StrongPtr<SysFont>	SysFont_p;
	typedef	WeakPtr<SysFont>	SysFont_wp;

	//____ SysFont _____________________________________________________________

	class SysFont : public Font
	{
	public:
		//.____ Creation __________________________________________

		static SysFont_p	create( SurfaceFactory * pFactory, Font * pBackupFont = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ______________________________________________________

		inline bool		setSize(spx size) override { return (size == 14); };						// SizeI is just a dummy for SysFont...
		inline spx		size() override { return 14; }

		void			getGlyphWithoutBitmap(uint16_t chr, Glyph& glyph) override;
		void			getGlyphWithBitmap(uint16_t chr, Glyph& glyph) override;
		spx				kerning(Glyph& leftGlyph, Glyph& rightGlyph) override;

		inline spx		lineGap() override { return 2*64; }
		inline spx		whitespaceAdvance() override { return 14*64; }
		inline spx		maxAdvance() override { return 14*64; }
		inline spx 		maxAscend() override { return 12*64; }
		inline spx 		maxDescend() override { return 2*64; }

		//.____ Misc ___________________________________________________________


		inline int		nbGlyphs() override { return 32*3; }
		inline bool		hasGlyphs() override { return true; }
		bool			hasGlyph(uint16_t chr) override;
		inline bool		isMonospace() override { return true; }
		inline bool		isMonochrome() override { return true; }

	protected:
		SysFont(SurfaceFactory * pFactory, Font * pBackupFont);
		~SysFont();

		Surface_p		m_pSurface;
	};



} // namespace wg
#endif // WG_SYSFONT_DOT_H
