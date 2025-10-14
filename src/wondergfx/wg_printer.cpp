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
#include <wg_printer.h>

namespace wg
{

	const TypeInfo Printer::TYPEINFO = { "Printer", &Object::TYPEINFO };

	const static int g_placementMulX[Placement_size] = { 0, 0, 1, 2, 2, 2, 1, 0, 0, 1 };
	const static int g_placementMulY[Placement_size] = { 0, 0, 0, 0, 1, 2, 2, 2, 1, 1 };


	//____ Create _________________________________________________________________

	Printer_p Printer::create()
	{
		Printer * pPrinter = new Printer();
		return Printer_p(pPrinter);
	}

	//____ constructor ____________________________________________________________

	Printer::Printer()
	{
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& Printer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setGfxDevice() ______________________________________________________
	/**
	* @brief Set GfxDevice used for printing
	* 
	* A GfxDevice is needed for printing. Printer uses the GfxDevice as is
	* and issues individual blit-calls for copying characters to the screen.
	* 
	* Therefore the GfxDevice needs to be in a mode where beginRender() and beginCanvasUpdate()
	* has been called before any of the print-methods can be called.
	* 
	* No states of the GfxDevice is changed by Printer (except for changing blitSource) so
	* blend modes, tint settings etc are used as they are.
	* 
	* To change the color of the printed text you should change the tint color of the GfxDevice.
	* 
	* @param pDevice	Pointer at GfxDevice to use when printing.
	*/

	void Printer::setGfxDevice(GfxDevice * pDevice)
	{
		m_pGfxDevice = pDevice;
	}

	//____ setFont() _____________________________________________________________
	/**
	* @brief Set font used for printing.
	* 
	* This sets the font used for printing. To change the size of text being printed
	* you should call the fonts setSize() method before printing.
	* 
	* The font can be changed between individual print calls.
	* 
	* @pFont	The font to use.
	*/
	void Printer::setFont(Font * pFont)
	{
		m_pFont = pFont;
	}

	//____ setCursorOrigo() ____________________________________________________________
	/**
	* @brief Set the origo for the cursor.
	* 
	* Sets the origo for the cursor on the canvas. The origo is the top-left position for the cursor
	* and the position the cursor returns to when resetCursor() is called.
	* 
	* When the cursor begins a new line it returns to the x-position of origo.
	* 
	* Current cursor position is not modified when setting origo. So you might
	* want to call resetCursor() right after.
	* 
	* @param origo	Position of origo in subpixel coordinates.
	*/
	void Printer::setCursorOrigo( CoordSPX origo )
	{
		m_cursorOrigo = origo;
	}

	//____ setLineWidth() ________________________________________________________
	/**
	* @brief Set line width used for aligning text.
	* 
	* This sets the line width, which is needed for calculating the right position
	* when centering or right-aligning text.
	* 
	* @param width	Line width measured in subpixels.
	*/
	void Printer::setLineWidth( spx width )
	{
		m_lineWidth = width;
	}

	//____ setTabSize() __________________________________________________________

	void Printer::setTabSize(int nbWhiteSpace)
	{
		m_tabSize = nbWhiteSpace;
	}


	//____ resetCursor() _________________________________________________________
	/**
	* @brief Move cursor to origo.
	* 
	* Move the cursor to origo.
	*/

	void Printer::resetCursor()
	{
		m_cursorPos = m_cursorOrigo;
	}

	//____ setCursor() ___________________________________________________________
	/**
	* @brief Move cursor to specified position.
	* 
	* Moves the cursor to the specified position on the canvas.
	* 
	* @param origo	Position of cursor in subpixel coordinates.
	*
	* Please note that characters are printed with the cursor position as their baseline,
	* not their top left coordinate. Setting cursor to {0,0} will make all uppercase
	* characters of the first line disappear above the canvas.
	* 
 	* @param position	New position of cursor in subpixel coordinates.
	*/
	void Printer::setCursor( CoordSPX pos )
	{
		m_cursorPos = pos;
	}

	//____ tab() ______________________________________________________________
	/**
	* @brief Move the cursor to next tab position.
	* 
	* Moves the cursor to the next tab position.
	*/

	void Printer::tab()
	{
		spx tablen = m_pFont->whitespaceAdvance() * m_tabSize;
		m_cursorPos.x = m_cursorPos.x - (m_cursorPos.x % tablen) + tablen;
	}

	//____ crlf() ________________________________________________________________
	/**
	* @brief Move cursor to beginning of next line.
	* 
	* Moves the cursor to the beginning of the next line.
	* 
	* The beginning of the next line has the X-position of cursor origo. The new 
	* Y-position is calculated under the assumption that the current font and font size
	* has been used for printing the current line and will be used for printing the next line.
	* 
	* If that assumption is incorrect, you need to specify the max font size used for this line
	* and the next line.
	*/

	void Printer::crlf()
	{
		if( m_pFont )
		{
			m_cursorPos.x = m_cursorOrigo.x;
			m_cursorPos.y += m_pFont->maxAscend() + m_pFont->maxDescend() + m_pFont->lineGap();
		}
	}

	void Printer::crlf( spx maxFontSizeThisLine, spx maxFontSizeNextLine )
	{
		if( m_pFont )
		{
			m_cursorPos.x = m_cursorOrigo.x;
			spx oldSize = m_pFont->size();
			m_pFont->setSize(maxFontSizeThisLine);
			m_cursorPos.y += m_pFont->maxDescend() + m_pFont->lineGap();
			m_pFont->setSize(maxFontSizeNextLine);
			m_cursorPos.y += m_pFont->maxAscend();
			m_pFont->setSize(oldSize);
			
		}
	}

	//____ printAligned() __________________________________________________
	/**
	* @brief Prints horizontally aligned text.
	* 
	* Prints the text with the specified horizontal alignment.
	* 
	* @param xAlignment	Should be either West, Center or East since only horizontal alignment is made.
	*					Specifying other alignment such as SouthWest will also work, but the vertical
	*					part of the alignment is ignored.
	* 
	* @param pText		Text to be printed. Zero terminated UTF8 string of characters. The string
	*                   can be a multi-line string with line-feed characters. Each line is aligned
	*					individually.
	* 
	* The X-position of the cursor is ignored as the text is aligned as specified.
	* The cursor is then moved to the position after the last printed character.
	* 
	*/
	void Printer::printAligned( Placement xAlignment, const char * pText )
	{
		m_cursorPos = _printAligned(m_cursorPos, m_lineWidth, xAlignment, pText);
	}

	//____ print() ________________________________________________________________
	/**
	* @brief Prints text at current cursor position.
	* 
	* Prints the specified text starting at the current cursor position.
	* 
	* @param pText		Text to be printed. Zero terminated UTF8 string of characters. The string
	*                   can be a multi-line string with tab and line-feed characters.
	* 
	* The cursor is moved to the position after the last printed character.
	* 
	*/
	void Printer::print( const char * pText )
	{
		m_cursorPos = _print(m_cursorPos, pText, nullptr, m_cursorOrigo.x );
	}

	//____ printWrapping() ________________________________________________________________
	/**
	* @brief Prints text at current cursor position with raw line wrapping.
	*
	* Prints the specified text starting at the current cursor position. Any line longer than current lineWidth
	* is wrapped to next line. The wrapping does not consider word boundaries, it just wraps where is needed, including
	* in the middle of a word.
	* Optionally, the wrapped lines can be indented.
	*
	* @param pText				Text to be printed. Zero terminated UTF8 string of characters. The string
	*                   				can be a multi-line string with tab and line-feed characters.
	* @param wrappedLinesIndent	Distance to indent wrapped lines, measured in subpixels.
	*
	* The cursor is moved to the position after the last printed character.
	*
	*/
	void Printer::printWrapping( const char * pText, spx wrappedLinesIndent )
	{
		m_cursorPos = _print(m_cursorPos, pText, nullptr, m_cursorOrigo.x, true, wrappedLinesIndent );
	}


	//____ lineHeight() __________________________________________________________
	/**
	* @brief Height of a line using the current font and font size.
	* 
	* Gets the height of a line when using the current font and font size.
	* This line height includes the spacing between lines specified as lineGap
	* in the font, it therefore corresponds to the distance the cursor needs
	* to move downwards after ending a line and beginning a new one and is not
	* the same as height needed for printing a single line.
	* 
	* @return Height of the line in subpixels or 0 if no font specified.
	*/
	spx Printer::lineHeight()
	{
		if( m_pFont )
			return m_pFont->maxAscend() + m_pFont->maxDescend() + m_pFont->lineGap();
		else
			return 0;
	}

	//____ textSize() _____________________________________________________________
	/**
	* @brief Size on canvas needed to print text.
	* 
	* Gets the size needed on canvas for printing the specified text, using the current
	* font and font size.
	* 
	* @param	Zero-terminated string of characters to calculate size of.
	*
	* @return	Size needed in subpixels.
	*/

	SizeSPX Printer::textSize( const char * pText )
	{
		return _textSize(pText, nullptr);
	}

	//____ wrappingTextSize() _____________________________________________________________
	/**
	* @brief Size on canvas needed to print wrapping text.
	*
	* Gets the size needed on canvas for printing the specified text, using the current
	* font, font size and lineWidth.
	*
	* @param					Zero-terminated string of characters to calculate size of.
	* @param wrappedLinesIndent	Distance in subpixels to indent lines that have been wrapped. Defaults is zero.
	*
	* @return					Size needed in subpixels.
	*/

	SizeSPX Printer::wrappingTextSize( const char * pText, spx wrappedLinesIndent )
	{
		return _textSize(pText, nullptr, true, wrappedLinesIndent );
	}


	//____ printAt() ______________________________________________________________
	/**
	* @brief Print text at the given coordinate.
	* 
	* Prints the specified text, left-aligned, starting from the given coordinate.
	* 
	* This method ignores the cursor origo and doesn't update the cursor position.
	* If the text contains a newline character the horizontal position will return
	* to the specified position.
	* 
	* @return The coordinate for printing any character following the text.
	*/
	CoordSPX Printer::printAt( CoordSPX pos, const char * pText )
	{
		return _print(pos, pText, nullptr, pos.x );
	}

	//____ printInBox() ____________________________________________________________
	/**
	* @brief Print text aligned in specified rectangle.
	*
	* Prints text in the specified box with horizontal and vertical alignment.
	* 
	* @param box	The rectangular area to align the text within before printing.
	* 
	* @param alignment	Horizontal and vertical alignment of the text within the box.
	* 
	* @param pText		Text to be printed. Zero terminated UTF8 string of characters. The string
	*                   can be a multi-line string with tab and line-feed characters.
	*
	* The text printed will not be constrained to the box, if the text is bigger than the box it will be
	* drawn partially outside.
	* 
	* This method ignores the cursor origo and doesn't update the cursor position.
	* 
	* @return	Position for printing any following character.
	*/

	CoordSPX Printer::printInBox( const RectSPX& box, Placement alignment, const char * pText )
	{
		SizeSPX wholeTextSize = _textSize(pText, nullptr);
		
		int yMul = g_placementMulY[int(alignment)];

		CoordSPX pos = box.pos();
		pos.y += (box.h - wholeTextSize.h) * yMul / 2;
		pos.y += m_pFont->maxAscend();
		
		return _printAligned(pos, box.w, alignment, pText);
	}

	//____ _print() ______________________________________________________________

	CoordSPX Printer::_print( CoordSPX pos, const char * pBegin, const char * pEnd, spx origoX, bool bWrap, spx wrappedLinesIndent )
	{
		if( !m_pGfxDevice || !m_pFont )
			return pos;
				
		Glyph	glyphs[2];

		Glyph * pPrevGlyph = &glyphs[0];
		Glyph * pGlyph = &glyphs[1];
		
		pPrevGlyph->fontRef = nullptr;
		
		const char * p = pBegin;
		while( p != pEnd )
		{
			uint32_t chr = _parseChar(p);

			if( chr <= 32 )
			{
				if( chr == 32 )	// SPACE
					pos.x += m_pFont->whitespaceAdvance();
				else if( chr == 10 )	// LF
				{
					pos.x = origoX;
					pos.y += m_pFont->maxAscend() + m_pFont->maxDescend() + m_pFont->lineGap();
				}
				else if( chr == 9 )	// TAB
				{
					spx tablen = m_pFont->whitespaceAdvance()*m_tabSize;
					pos.x = pos.x - ((pos.x-origoX) % tablen) + tablen;
				}
				else if( chr == 0 )
				{
					return pos;
				}
					
				pPrevGlyph->fontRef = nullptr;
			}
			 else
			{
				m_pFont->getGlyphWithBitmap(chr, *pGlyph);
				
				if( pGlyph->fontRef && pPrevGlyph->fontRef )
					pos.x += m_pFont->kerning(*pPrevGlyph, *pGlyph);
				
				if( bWrap && (pos.x + pGlyph->advance > m_lineWidth ) )
				{
					pos.x = origoX + wrappedLinesIndent;
					pos.y += m_pFont->maxAscend() + m_pFont->maxDescend() + m_pFont->lineGap();
				}
								
				if (pGlyph->pSurface)
				{
					CoordSPX blitPos = pos;
					blitPos.x += pGlyph->bearingX;
					blitPos.y += pGlyph->bearingY;
				
					m_pGfxDevice->setBlitSource(pGlyph->pSurface);
					m_pGfxDevice->blit(blitPos,pGlyph->rect);
				}

				pos.x += pGlyph->advance;

			}
			std::swap(pGlyph,pPrevGlyph);
		}
		return pos;
	}

	//____ _printAligned() __________________________________________________

	CoordSPX Printer::_printAligned( CoordSPX pos, spx lineWidth, Placement xAlign, const char * pBegin )
	{
		int mul = g_placementMulX[int(xAlign)];

		while( * pBegin != 0 )
		{
			auto pEnd = _lineEnd(pBegin);
			
			SizeSPX lineSize = _textSize(pBegin,pEnd);

			CoordSPX printPos = pos;
			printPos.x += (lineWidth - lineSize.w)*mul/2;
			
			_print(printPos, pBegin, pEnd, printPos.x );
			pos.y += lineSize.h;
			pos.y += m_pFont->lineGap();
			
			pBegin = pEnd;

			if( * pBegin == 13 )
				pBegin++;

			if( * pBegin == 10 )
				pBegin++;

			if( * pBegin == 13 )
				pBegin++;
		}

		return pos;
	}


	//____ _parseChar() __________________________________________________________

	uint32_t Printer::_parseChar( const char *& pStr )
	{
		uint32_t g = * ((const unsigned char *&)pStr)++;

		if( g < 128 )
			return g;											// 1 byte character (7 bits data).

		if( (g & 0xE0) == 0xC0 )					// 2 bytes character (11 bits data).
		{
			g &= 0x1F;
			g <<= 6;
			g += ((*pStr++) & 0x3F);
		}
		else if( (g & 0xF0) == 0xE0 )			// 3 bytes character (16 bits data).
		{
			g &= 0x0F;
			g <<= 6;
			g += ((*pStr++) & 0x3F);
			g <<= 6;
			g += ((*pStr++) & 0x3F);
		}
		else if( (g & 0xF8) == 0xF0 )			// 4 bytes character (21 bits data).
		{
			g &= 0x07;
			g <<= 6;
			g += ((*pStr++) & 0x3F);
			g <<= 6;
			g += ((*pStr++) & 0x3F);
			g <<= 6;
			g += ((*pStr++) & 0x3F);
		}
		else
			return 0xFFFFFFFF;					// Broken character (or more than 4 bytes which we don't support)

		return g;
	}

	//____ _lineEnd() ____________________________________________________________

	const char * Printer::_lineEnd( const char * pPos )
	{
		while( * pPos != 0 && * pPos != 10 )
			pPos++;

		return pPos;
	}

	//____ _textSize() ___________________________________________________________

	SizeSPX Printer::_textSize( const char * pBegin, const char * pEnd, bool bWrap, spx wrappedLinesIndent )
	{
		if( !m_pGfxDevice || !m_pFont )
			return SizeSPX();
				
		Glyph	glyphs[2];

		Glyph * pPrevGlyph = &glyphs[0];
		Glyph * pGlyph = &glyphs[1];
		
		pPrevGlyph->fontRef = nullptr;
	
		CoordSPX pos;
		pos.y += m_pFont->maxAscend();
		
		spx maxX = 0;
		
		const char * p = pBegin;
		while( p != pEnd && * p != 0 )
		{
			uint32_t chr = _parseChar(p);

			if( chr <= 32 )
			{
				if( chr == 32 )	// SPACE
					pos.x += m_pFont->whitespaceAdvance();
				else if( chr == 10 )	// LF
				{
					pos.x = 0;
					pos.y += m_pFont->maxDescend() + m_pFont->lineGap() + m_pFont->maxAscend();
				}
				else if( chr == 9 )	// TAB
				{
					spx tablen = m_pFont->whitespaceAdvance()*m_tabSize;
					pos.x = pos.x - (pos.x % tablen) + tablen;
				}
					
				pPrevGlyph->fontRef = nullptr;
			}
			 else
			{
				m_pFont->getGlyphWithoutBitmap(chr, *pGlyph);

				spx newPosX = pos.x;
				
				newPosX += pGlyph->advance;
				if( pGlyph->fontRef && pPrevGlyph->fontRef )
					newPosX += m_pFont->kerning(*pPrevGlyph, *pGlyph);
				
				if( bWrap && newPosX > m_lineWidth )
				{
					if( pos.x > maxX )
						maxX = pos.x;
					pos.x = wrappedLinesIndent + pGlyph->advance;
					pos.y += m_pFont->maxDescend() + m_pFont->lineGap() + m_pFont->maxAscend();
				}
				else
					pos.x = newPosX;
				
				if( pos.x > maxX )
					maxX = pos.x;
			}
			std::swap(pGlyph,pPrevGlyph);
		}

		pos.y += m_pFont->maxDescend();
		
		return SizeSPX(maxX, pos.y);
	}


} // namespace wg
