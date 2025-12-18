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
#ifndef	WG_PRINTER_DOT_H
#define	WG_PRINTER_DOT_H
#pragma once

#include <wg_pointers.h>
#include <wg_gfxtypes.h>
#include <wg_gfxdevice.h>
#include <wg_font.h>

namespace wg
{

	class Printer;
	typedef	StrongPtr<Printer>	Printer_p;
	typedef	WeakPtr<Printer>	Printer_wp;


	/**
	 * @brief Object for low-level printing of text on screen.
	 *
	 * A Printer in WG is a low-level object for printing text on screen (it performs 'prints') and has
	 * nothing to do with printing on paper.
	 *
	 * It exists outside the normal text display system and doesn't use TextStyle, Text-objects, Strings
	 * or TextLayout objects. It just prints UTF-8 strings of text onto the current canvas of its specified
	 * GfxDevice using the specified font.
	 *
	 * All print-methods are meant to be used within a GfxDevice render-cycle, where things such as Canvas,
	 * BlendMode and TintColor has already been setup.
	 *
	 * To change the size of the font between prints you simply change the size in the Font-object itself. If you
	 * want to print in another color you change tint color of the GfxDevice.
	 *
	 * At a minimum you need to set GfxDevice and Font before you can start printing. You should also set
	 * line width if you want to do aligned printing (since line width needs to be known for alignment).
	 *
	 */

	class Printer : public Object
	{
	public:
		//.____ Creation __________________________________________

		static Printer_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ________________________________________________

		void		setGfxDevice(GfxDevice * pDevice);
		GfxDevice_p	gfxDevice() const { return m_pGfxDevice;  }

		void		setFont(Font * pFont);
		Font_p		font() const { return m_pFont;  }
		
		void		setCursorOrigo( CoordSPX origo );
		CoordSPX	cursorOrigo() const { return m_cursorOrigo;  }

		void		setLineWidth( spx width );		// Needed for printJustifiedLine()
		spx			lineWidth() const { return m_lineWidth; }

		void		setTabSize(int nbWhiteSpace);
		int			tabSize() const { return m_tabSize;  }

		void		resetCursor();
		void		setCursor( CoordSPX pos );
		CoordSPX	cursor() const { return m_cursorPos;  }
		void		tab();
		void		crlf();
		void		crlf( spx maxFontSizeThisLine, spx maxFontSizeNextLine );
		
		void		print( const char * pText );
		void		printAligned( Placement xAlignment, const char * pText );
		void		printWrapping( const char * pText, spx wrappedLinesIndent = 0 );

		CoordSPX	printAt( CoordSPX pos, const char * pText );
		CoordSPX	printInBox( const RectSPX& box, Placement alignment, const char * pText );

		spx			lineHeight();
		SizeSPX		textSize( const char * pText );
		SizeSPX		wrappingTextSize( const char * pText, spx wrappedLinesIndent = 0 );


	protected:
		Printer();
		~Printer() {}
	
		
		uint32_t 	_parseChar( const char *& pStr );
		CoordSPX	_print( CoordSPX pos, const char * pBegin, const char * pEnd, spx origoX, bool bWrap = false, spx wrappedLinesIndent = 0 );
		CoordSPX	_printAligned( CoordSPX pos, spx lineWidth, Placement xAlignment, const char * pBegin );

		const char * _lineEnd( const char * pPos );
		SizeSPX		_textSize( const char * pBegin, const char * pEnd, bool bWrap = false, spx wrappedLinesIndent = 0 );
		
		GfxDevice_p	m_pGfxDevice;
		Font_p		m_pFont;
		
		CoordSPX	m_cursorPos;
		CoordSPX	m_cursorOrigo;
		
		spx			m_lineWidth = 100*64;
		int			m_tabSize = 8;

	};


} // namespace wg
#endif //WG_PRINTER_DOT_H
