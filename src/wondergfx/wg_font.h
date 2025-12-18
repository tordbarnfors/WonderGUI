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
#ifndef	WG_FONT_DOT_H
#define	WG_FONT_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_pointers.h>
#include <wg_surface.h>
#include <wg_surfacefactory.h>

namespace wg
{

	class	Font;

	//____ Glyph Info _________________________________________________________

	struct Glyph
	{
		spx			advance = 0;			// Set to zero if glyph do not exist.
		uint32_t	kerningIndex = 0;		//
		void*		fontRef = nullptr;		// Needed for kerning in backup font.

		// These below are not updated by getGlyphWithoutBitmap().

		Surface_p 	pSurface;		// Set to null if no bitmap for glyph could be found.
		RectSPX		rect;
		spx			bearingX = 0;		// x offset when rendering the glyph (negated offset to glyph origo)
		spx			bearingY = 0;		// y offset when rendering the glyph (negated offset to glyph origo)
	};


	class Font;
	typedef	StrongPtr<Font>		Font_p;
	typedef	WeakPtr<Font>		Font_wp;

	//____ Font _____________________________________________________________
	/**
	 * @class Font
	 * @file wg_font.h
	 * @brief Base class for various types of fonts.
	 *
	 * The Font class is the base class from which all fonts are subclassed,
	 * providing the functionality to get metrics for the font and metrics
	 * and bitmaps of all its character glyphs.
	 */


	class Font : public Object
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ____________________________________________

		virtual bool			setSize( spx size ) = 0;	///@brief Set the font size for subsequent calls.
		virtual spx				size() = 0;
		virtual void			getGlyphWithoutBitmap(uint16_t chr, Glyph& glyph) = 0;
		virtual void			getGlyphWithBitmap(uint16_t chr, Glyph& glyph) = 0;
		virtual spx				kerning( Glyph& leftGlyph, Glyph& rightGlyph ) = 0;

		virtual spx				lineGap() = 0;					///@brief Returns distance between two lines of this font and current size.
		virtual spx				whitespaceAdvance() = 0;		///@brief Returns width of a whitespace for current size.
		virtual spx				maxAdvance() = 0;				///@brief Returns largets occupied width of a character for current size.
		virtual spx				maxAscend() = 0;				///@brief Returns largets height from baseline of a character for current size.
		virtual spx 			maxDescend() = 0;				///@brief Returns largets depth from baseline of a character for current size.

		//.____ Misc ___________________________________________________________

		virtual int				nbGlyphs() = 0;					///@brief Return number of distinct glyphs in the font.
		virtual bool			hasGlyphs() = 0;				///@brief Check if font has any glyphs at all.
		virtual bool			hasGlyph( uint16_t chr ) = 0;	///@brief Check if font provides a glyph for specified character.
		virtual bool			isMonospace() = 0;				///@brief Check if font is monospaced.
		virtual bool			isMonochrome();					///@brief Check if font is monochrome or multi-colored.

        inline Font_p           backupFont() const;             ///@brief Font used when glyph can't be found in our font.

		Surface_p				getGlyphAsSurface(uint16_t chr, const Surface::Blueprint& blueprint = Surface::Blueprint() , SurfaceFactory * pFactory = nullptr );

		//.____ Deprecated __________________________________________________________
		
		inline void				setBackupFont( Font * pFont ) { m_pBackupFont = pFont; }

		
	protected:
		Font( Font * pBackupFont ) : m_pBackupFont(pBackupFont) {}
		virtual ~Font() {}

        Font_p         m_pBackupFont;
	};


    //____ backupFont() _____________________________________________________

    inline Font_p Font::backupFont() const
    {
        return  m_pBackupFont;
    }


} // namespace wg
#endif // WG_FONT_DOT_H
