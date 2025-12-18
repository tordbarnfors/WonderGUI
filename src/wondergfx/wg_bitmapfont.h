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
#ifndef	WG_BITMAPFONT_DOT_H
#define	WG_BITMAPFONT_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_font.h>
#include <wg_surface.h>

namespace wg
{

	class BitmapFont;
	typedef	StrongPtr<BitmapFont>	BitmapFont_p;
	typedef	WeakPtr<BitmapFont>		BitmapFont_wp;

	//____ BitmapFont _____________________________________________________________

	class BitmapFont : public Font
	{
	public:
		//.____ Creation __________________________________________

		static BitmapFont_p	create( Surface * pSurf, const char * pGlyphSpec, Font * pBackupFont = nullptr ) { return BitmapFont_p(new BitmapFont(pSurf,pGlyphSpec,pBackupFont)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ______________________________________________________

		inline bool			setSize( spx size ) override { return (size == m_size);};						// SizeI is just a dummy for BitmapFont...
		inline spx			size() override { return m_size; }

		void				getGlyphWithoutBitmap(uint16_t chr, Glyph& glyph) override;
		void				getGlyphWithBitmap(uint16_t chr, Glyph& glyph) override;
		spx					kerning(Glyph& leftGlyph, Glyph& rightGlyph) override;

		inline spx			lineGap() override { return m_lineGap; }
		inline spx			whitespaceAdvance() override { return m_spaceAdvance; }
		inline spx			maxAdvance() override { return m_maxAdvance; }
		inline spx 			maxAscend() override { return m_maxAscend; }
		inline spx 			maxDescend() override { return m_maxDescend; }

		//.____ Misc ___________________________________________________________


		inline int			nbGlyphs() override { return m_nGlyphs; }
		inline bool			hasGlyphs() override { return m_nGlyphs?true:false; }
		bool				hasGlyph( uint16_t chr ) override;
		inline bool			isMonospace() override { return m_bMonospace; }
		inline bool			isMonochrome() override { return m_bMonochrome; }

		inline Surface_p	surface() { return m_pSurface;}
		
	protected:
		BitmapFont( Surface * pSurf, const char * pGlyphSpec, Font * pBackupFont );
		~BitmapFont();

		void _insertGlyphs(Surface* pSurf, const char* pGlyphSpec);

		class MyGlyph
		{
		public:
//			MyGlyph();
//			MyGlyph( int advance, int16_t bearingX, int16_t bearingY, uint32_t kerningIndex, Font * pFont, Surface * pSurf, const RectSPX& rect );

			spx			advance = 0;
			uint32_t	kerningIndex = 0;		//

			Surface_p 	pSurface;			// Set to null if no bitmap for glyph could be found.
			RectSPX		rect;
			spx			bearingX = 0;			// x offset when rendering the glyph (negated offset to glyph origo)
			spx			bearingY = 0;			// y offset when rendering the glyph (negated offset to glyph origo)
		};

		Surface_p	m_pSurface;
		
		MyGlyph * 	m_glyphTab[256];

		int			m_nKerningGlyphs;
		int16_t*	m_pKerningTable;

		int			m_nGlyphs;
		bool		m_bMonospace;
		bool		m_bMonochrome;
		spx			m_lineGap;
		spx			m_spaceAdvance;
		spx			m_maxAdvance;
		spx			m_maxAscend;
		spx			m_maxDescend;
		spx			m_size;						// Fontsize for this font.
	};



} // namespace wg
#endif // WG_BITMAPFONT_DOT_H
