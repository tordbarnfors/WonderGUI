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

#include <assert.h>
#include <memory.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <wg_codepages.h>
#include <wg_util.h>

#include <string>

#include <wg_texttool.h>
#include <wg_charseq.h>
#include <wg_base.h>
#include <wg_char.h>

namespace wg
{

	const static char itoa_table [35+36+1]= { "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" };

	static char	breakRulesTab[256];


	//____ parseChar() ____________________________________________________________

	uint16_t TextTool::parseChar( const char * pChar )
	{
		if( pChar[0] == 0 )
			return 0;

		if( pChar[0] == '0' && pChar[1] == 'x' )
		{
			uint16_t value = 0;
			pChar += 2;

			for( char in = * pChar++ ; in != 0 ; in = * pChar++ )
			{
				if( in >= '0' && in <= '9' )
					value = (value << 4) + (in - '0' );
				else if( in >= 'a' && in <= 'f' )
					value = (value << 4 )+ (in - 'a') + 10;
				else if( in >= 'A' && in <= 'F' )
					value = (value << 4 )+ (in - 'A') + 10;
				else
					break;
			}

			return value;
		}
		else
		{
			return readChar( pChar );
		}

	}



	//____ clearBreakRules() ______________________________________________________

	void TextTool::clearBreakRules()
	{
		for( int i = 0 ; i < 256 ; i++ )
			breakRulesTab[i] = ((char)BreakRules::NoBreak) | 0xF;
	}


	//____ addBreakRule() _________________________________________________________

	bool TextTool::setBreakRule( unsigned char character, int level, BreakRules rule )
	{
		if( level < 0 || level > 15 )
			return false;

		breakRulesTab[character] = ((char)rule) | level;
		return true;
	}

	//____ setDefaultBreakRules() _________________________________________________

	void TextTool::setDefaultBreakRules()
	{
		clearBreakRules();

		// Set level 1 breaks

		breakRulesTab[(int)ExtChar::BreakPermitted] = ((char)BreakRules::BreakOn) | 0x1;
		breakRulesTab[(int)ExtChar::HyphenBreakPermitted] = ((char)BreakRules::BreakOn) | 0x1;

		// Set level 2 breaks

		breakRulesTab[0x20] = ((char)BreakRules::BreakOn) | 0x2;			// Whitespace
		breakRulesTab[0x09] = ((char)BreakRules::BreakOn) | 0x2;			// Tab

		// Set level 3 breaks

		breakRulesTab['\\']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['/']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['-']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['+']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['*']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['~']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['=']	= ((char)BreakRules::BreakAfter) | 0x3;

		breakRulesTab[')']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['}']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab[']']	= ((char)BreakRules::BreakAfter) | 0x3;
		breakRulesTab['>']	= ((char)BreakRules::BreakAfter) | 0x3;

		breakRulesTab['(']	= ((char)BreakRules::BreakBefore) | 0x3;
		breakRulesTab['{']	= ((char)BreakRules::BreakBefore) | 0x3;
		breakRulesTab['[']	= ((char)BreakRules::BreakBefore) | 0x3;
		breakRulesTab['<']	= ((char)BreakRules::BreakBefore) | 0x3;

		// Set level 4 breaks

		breakRulesTab[0xbf]	= ((char)BreakRules::BreakBefore) | 0x5;	// inverted question mark (beginning of question).
		breakRulesTab['?']	= ((char)BreakRules::BreakAfter) | 0x5;
		breakRulesTab[':']	= ((char)BreakRules::BreakAfter) | 0x4;
		breakRulesTab[';']	= ((char)BreakRules::BreakAfter) | 0x4;
		breakRulesTab['.']	= ((char)BreakRules::BreakAfter) | 0x5;
		breakRulesTab[',']	= ((char)BreakRules::BreakAfter) | 0x5;
		breakRulesTab['!']	= ((char)BreakRules::BreakAfter) | 0x5;
		breakRulesTab['_']	= ((char)BreakRules::BreakAfter) | 0x4;
		breakRulesTab['|']	= ((char)BreakRules::BreakAfter) | 0x4;
	}


	//____ isBreakAllowed() _______________________________________________________

	BreakRules TextTool::isBreakAllowed( uint16_t chr, int breakLevel )
	{
		if( chr > 255 )
			return BreakRules::NoBreak;

		if( breakLevel >= (breakRulesTab[chr] & 0xF) )
			return (BreakRules) (breakRulesTab[chr] & 0xF0);

		return BreakRules::NoBreak;
	}


	//____ itoa() _________________________________________________________________

	char * TextTool::itoa( int value, char * str, int base )
	{

		char * p1 = str+33;
		char * p2 = str;

		if( value < 0 )
			* p2++ = '-';

		while( value != 0 )
		{
			* --p1 = itoa_table[(value%base)+35];
			value /= base;
		}

		while( p1 < str+33 )
			* p2++ = * p1++;

		* p2++ = 0;

		return str;
	}


	//____ readString() ___________________________________________________________

	uint32_t TextTool::readString( const char *& pSrc, uint16_t * pDst, uint32_t maxChars )
	{
		if( !pSrc )
		{
			if( maxChars > 0 )
				pDst[0] = 0;
			return 0;
		}
		uint32_t n = 0;

		while(n < maxChars && * pSrc != 0 )
			pDst[n++] = readChar(pSrc);

		if( n != maxChars )
			pDst[n] = 0;
		return n;
	}


	uint32_t TextTool::readString( const char *& pSrc, Char * pDst, uint32_t maxChars )
	{
		if( !pSrc )
		{
			if( maxChars > 0 )
				pDst[0].setCode(0);
			return 0;
		}
		uint32_t n = 0;

		while(n < maxChars && * pSrc != 0 )
		{
			pDst[n++].setCode(readChar(pSrc));
		}
		if( n != maxChars )
			pDst[n].setCode(0);
		return n;
	}

	uint32_t TextTool::readString( const uint16_t *& pSrc, Char * pDst, uint32_t maxChars )
	{
		if( !pSrc )
		{
			if( maxChars > 0 )
				pDst[0].setCode(0);
			return 0;
		}
		uint32_t n = 0;

		while(n < maxChars && * pSrc != 0 )
			pDst[n++].setCode( * pSrc++);

		if( n != maxChars )
			pDst[n].setCode(0);
		return n;
	}


	uint32_t TextTool::readString( const char *& pSrc, CodePage codepage, Char * pDst, uint32_t maxChars )
	{
		const uint16_t * pCP = CodePages::getCodePage( codepage );
		if( !pCP || !pSrc )
		{
			if( maxChars > 0 )
				pDst[0].setCode(0);
			return 0;
		}

		uint32_t n = 0;
		for( unsigned char * p = (unsigned char *) pDst ; n < maxChars && p[n] != 0 ; n++ )
			pDst[n].setCode( pCP[p[n]] );

		if( n != maxChars )
			pDst[n].setCode(0);
		return n;
	}

	uint32_t TextTool::readString( const char *& pSrc, CodePage codepage, uint16_t * pDst, uint32_t maxChars )
	{
		const uint16_t * pCP = CodePages::getCodePage( codepage );
		if( !pCP || !pSrc )
		{
			if( maxChars > 0 )
				pDst[0] = 0;
			return 0;
		}

		uint32_t n = 0;
		for( unsigned char * p = (unsigned char *) pDst ; n < maxChars && p[n] != 0 ; n++ )
			pDst[n] = pCP[p[n]];

		if( n != maxChars )
			pDst[n] = 0;
		return n;
	}


	//____ CountWhitespaces() ______________________________________________________

	uint32_t TextTool::countWhitespaces( const char * pStr, uint32_t len )
	{
		if( !pStr )
			return 0;

		uint32_t n = 0;
		for( unsigned int i = 0 ; i < len && pStr[i] != 0 ; i++ )
		{
			if( pStr[i] == ' ' || pStr[i] == (char) ExtChar::NoBreakSpace )
				n++;
		}
		return n;
	}

	uint32_t TextTool::countWhitespaces( const uint16_t * pStr, uint32_t len  )
	{
		if( !pStr )
			return 0;

		uint32_t n = 0;
		for( unsigned int i = 0 ; i < len && pStr[i] != 0 ; i++ )
		{
			if( pStr[i] == ' ' || pStr[i] == (uint16_t) ExtChar::NoBreakSpace )
				n++;
		}
		return n;
	}

	uint32_t TextTool::countWhitespaces( const Char * pStr, uint32_t len  )
	{
		if( !pStr )
			return 0;

		uint32_t n = 0;
		for( unsigned int i = 0 ; i < len && !pStr[i].isEndOfText() ; i++ )
		{
			if( pStr[i].isWhitespace() )
				n++;
		}
		return n;
	}






	//____ copyChars() ____________________________________________________________

	int TextTool::copyChars( const Char * pSrc, Char * pDst, int maxChars )
	{
		// Special refProps() equivalent which also counts characters and stops at NULL.

		TextStyle_h	hStyle = 0;
		int			nStyle = 0;

		int n = 0;					// Also number of characters after the for-loop...
		for( ; n < maxChars ; n++ )
		{

			TextStyle_h h = pSrc[n].styleHandle();
			if( h == hStyle )
				nStyle++;
			else
			{
				if( hStyle )
					TextStyleManager::_getPointer(hStyle)->_incRefCount( nStyle );

				hStyle = h;
				nStyle = 1;
			}
			if( pSrc[n].isEndOfText() )
			{
				n++;
				break;
			}
		}

		if( hStyle )
			TextStyleManager::_getPointer(hStyle)->_incRefCount( nStyle );

		// Dereference the same amount of characters from destination.

		derefStyle( pDst, n );

		// Copy chars in a fast and straight way...

		memcpy( pDst, pSrc, n*sizeof(Char) );

		return n;
	}



	//____ derefStyle() ____________________________________________________________

	void TextTool::derefStyle( Char * p, int n )
	{
		TextStyle_h	hStyle = 0;
		int			nStyle = 0;

		for( int i = 0 ; i < n ; i++ )
		{

			TextStyle_h h = p[i].styleHandle();
			if( h == hStyle )
				nStyle++;
			else
			{
				if( hStyle )
					TextStyleManager::_getPointer(hStyle)->_decRefCount( nStyle );

				hStyle = h;
				nStyle = 1;
			}
		}

		if( hStyle )
			TextStyleManager::_getPointer(hStyle)->_decRefCount( nStyle );
	}


	//____ refStyle() ______________________________________________________________

	void TextTool::refStyle( Char * p, int n )
	{
		TextStyle_h	hStyle = 0;
		int			nStyle = 0;

		for( int i = 0 ; i < n ; i++ )
		{

			uint16_t h = p[i].styleHandle();
			if( h == hStyle )
				nStyle++;
			else
			{
				if( hStyle )
					TextStyleManager::_getPointer(hStyle)->_incRefCount( nStyle );

				hStyle = h;
				nStyle = 1;
			}
		}

		if( hStyle )
			TextStyleManager::_getPointer(hStyle)->_incRefCount( nStyle );
	}


	//____ countChars() ___________________________________________________________

	int	TextTool::countChars( const char * pStr, int strlen )
	{
		if( !pStr )
			return 0;

		int n = 0;
		for( const char * p = pStr ; (p - pStr) < strlen ; n++ )
			if( readChar(p) == 0 )
				break;

		return n;
	}

	int	TextTool::countChars( const uint16_t * pStr, int strlen )
	{
		if( !pStr )
			return 0;

		int n = 0;
		while( n < strlen && pStr[n] != 0 )
			n++;

		return n;
	}


	//____ countLines() ___________________________________________________________

	uint32_t	TextTool::countLines( const char * pStr )
	{
		if( !pStr )
			return 0;

		uint32_t n = 1;
		while( * pStr != 0 )
		{
			if( * pStr++ == '\n' )
				n++;
		}

		return n;
	}

	uint32_t	TextTool::countLines( const uint16_t * pStr )
	{
		if( !pStr )
			return 0;

		uint32_t n = 1;
		while( * pStr != 0 )
		{
			if( * pStr++ == '\n' )
				n++;
		}

		return n;
	}

	uint32_t	TextTool::countLines( const Char * pStr )
	{
		if( !pStr )
			return 0;

		uint32_t n = 1;
		while( !pStr->isEndOfText() )
		{
			if( pStr->isEndOfLine() )
				n++;
			pStr++;
		}

		return n;
	}



	//____ countLineChars() _______________________________________________________
	/**
		Count characters up until (but not including) the end of the string
		or the first line termination character (CR or LF) found.
	*/
	uint32_t	TextTool::countLineChars( const char * pStr, uint32_t len )
	{
		if( !pStr || len == 0 )
			return 0;

		uint32_t n = 0;
		const char * pEnd = pStr + len;

		uint16_t ch = readChar(pStr);
		while(pStr < pEnd && ch != 0 && ch != 10 && ch != 13 )
		{
			n++;
			ch = readChar(pStr);
		}
		return n;
	}

	//____ countLineChars() _______________________________________________________
	/**
		Count characters up until (but not including) the end of the string
		or the first line termination character (CR or LF) found.
	*/
	uint32_t	TextTool::countLineChars( const uint16_t * pStr, uint32_t len )
	{
		if( !pStr || len == 0 )
			return 0;

		uint32_t n = 0;

		uint16_t ch = * pStr++;
		while(n < len && ch != 0 && ch != 10 && ch != 13 )
		{
			n++;
			ch = * pStr++;
		}
		return n;
	}

	//____ countLineChars() _______________________________________________________
	/**
		Count characters up until (but not including) the end of the string
		or the first line termination character (CR or LF) found.
	*/
	uint32_t	TextTool::countLineChars( const Char * pStr, uint32_t len )
	{
		if( !pStr || len == 0 )
			return 0;

		uint32_t n = 0;

		while(n < len && !pStr->isEndOfLine() )
			n++;

		return n;
	}


	//____ countCharsLines() ______________________________________________________

	void	TextTool::countCharsLines( const char * pStr, uint32_t& nChars, uint32_t& nLines, uint32_t strlen )
	{
		if( !pStr )
			return;

		const char * pEnd = pStr + strlen;

		nChars = 0;
		nLines = 1;

		while( pStr != pEnd )
		{
			uint16_t c = readChar(pStr);
			if( c == 0 )
				break;

			if( c == (uint16_t) '\n' )
				nLines++;

			nChars++;
		}
	}

	void	TextTool::countCharsLines( const uint16_t * pStr, uint32_t& nChars, uint32_t& nLines, uint32_t strlen )
	{
		if( !pStr )
			return;

		const uint16_t * pEnd = pStr + strlen;

		nChars = 0;
		nLines = 1;

		while( pStr != pEnd )
		{
			uint16_t c = * pStr++;
			if( c == 0 )
				break;

			if( c == (uint16_t) '\n' )
				nLines++;

			nChars++;
		}
	}


	void	TextTool::countCharsLines( const Char * pStr, uint32_t& nChars, uint32_t& nLines, uint32_t strlen )
	{
		if( !pStr )
			return;

		const Char * pEnd = pStr + strlen;

		nChars = 0;
		nLines = 1;

		while( pStr + nChars != pEnd )
		{
			if( pStr[nChars].isEndOfText() )
				break;

			if( pStr[nChars].isEndOfLine() )
				nLines++;

			nChars++;
		}
	}


	//____ nextLine() _____________________________________________________________

	char * TextTool::nextLine( char * p )
	{
		while( * p != 10 )
		{
			if( * p == 0 )
				return p;
			p++;
		}
		p++;

		if( * p == 13 )
			p++;

		return p;
	}

	//____ writeUTF8() ____________________________________________________________

	uint32_t TextTool::writeUTF8( uint16_t glyph, char * pDest )
	{
		if( glyph < 128 )
		{
			pDest[0] = (char) glyph;
			return 1;
		}
		else if( glyph < 0x800 )
		{
			pDest[0] =  0xC0 | ((char) ((glyph & 0x07C0) >> 6));
			pDest[1] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			return 2;
		}
		else
		{
			pDest[0] =  0xE0 | ((char) ((glyph & 0xF000) >> 12));
			pDest[1] =  0x80 | ((char) ((glyph & 0x0FC0) >> 6));
			pDest[2] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			return 3;
		}
	}


	//____ strlen() ____________________________________________________________
	uint32_t TextTool::strlen( const uint16_t * pSrc )
	{
		uint32_t nChars = 0;
		while( * pSrc != 0 )
		{
			++nChars;
			++pSrc;
		}
		return nChars;
	}


	//____ strlen() ____________________________________________________________
	uint32_t TextTool::strlen( const Char * pSrc )
	{
		uint32_t nChars = 0;
		while( !pSrc->isEndOfText() )
		{
			++nChars;
			++pSrc;
		}
		return nChars;
	}

	//____ strcmp() ____________________________________________________________
	int TextTool::strcmp( const uint16_t* pStr1, const uint16_t* pStr2 )
	{
		while( *pStr1 != L'\0' && *pStr1 == *pStr2 )
		{
			pStr1++;
			pStr2++;
		}
		return *pStr1 - *pStr2;
	}

	//____ strcmp() ____________________________________________________________

    int TextTool::strcmp( const Char * pStr1, const Char * pStr2 )
	{
		while( !pStr1->isEndOfText() && pStr1->equals(*pStr2) )
		{
			pStr1++;
			pStr2++;
		}

		if( pStr1->isEndOfText() && pStr2->isEndOfText() )
			return 0;

        int diff = pStr1->code() - pStr2->code();
        if( diff == 0 )
            diff = pStr1->styleHandle() - pStr2->styleHandle();
        
        return diff;
	}

    int TextTool::strcmp( const Char * pStr1, const char * pUTF8String )
    {
        const char * readPos = pUTF8String;
        uint16_t chr = readChar( readPos );

        while( !pStr1->isEndOfText() && pStr1->styleHandle() == 0 && pStr1->code() == chr )
        {
            pStr1++;
            chr = readChar( readPos );
        }

        if( pStr1->isEndOfText() && chr == 0 )
            return 0;

        int diff = pStr1->code() - chr;
        if( diff == 0 )
            diff = pStr1->styleHandle();
        
        return diff;
    }


	//____ charcodecmp() ____________________________________________________________

    int TextTool::charcodecmp( const Char * pStr1, const Char * pStr2 )
	{
		while( !pStr1->isEndOfText() && pStr1->code() == pStr2->code() )
		{
			pStr1++;
			pStr2++;
		}

		if( pStr1->isEndOfText() && pStr2->isEndOfText() )
			return 0;

		return pStr1->code() - pStr2->code();
	}

    int TextTool::charcodecmp( const Char * pStr1, const char * pUTF8String )
    {
        const char * readPos = pUTF8String;
        uint16_t chr = readChar( readPos );

        while( !pStr1->isEndOfText() && pStr1->code() == chr )
        {
            pStr1++;
            chr = readChar( readPos );
        }

        if( pStr1->isEndOfText() && chr == 0 )
            return 0;

        return pStr1->code() - chr;
    }


	//____ charcodecmpIgnoreCase() _______________________________________________________
	int TextTool::charcodecmpIgnoreCase( const Char * pStr1, const Char * pStr2 )
	{
		while( !pStr1->isEndOfText() && towlower(pStr1->code()) == towlower(pStr2->code()) )
		{
			pStr1++;
			pStr2++;
		}

		if( pStr1->isEndOfText() && pStr2->isEndOfText() )
			return 0;

		return towlower(pStr1->code()) - towlower(pStr2->code());
	}

    //____ charcodecmpIgnoreCase() _______________________________________________________
    int TextTool::charcodecmpIgnoreCase( const Char * pStr1, const char * pUTF8String )
    {
        const char * readPos = pUTF8String;
        uint16_t chr = readChar( readPos );
        
        while( !pStr1->isEndOfText() && towlower(pStr1->code()) == towlower(chr) )
        {
            pStr1++;
            chr = readChar( readPos );
        }

        if( pStr1->isEndOfText() && chr == 0)
            return 0;

        return towlower(pStr1->code()) - towlower(chr);
    }



	//____ nibbleToAscii() ____________________________________________________________
	inline uint8_t TextTool::nibbleToAscii( uint8_t nibble )
	{
		if( nibble <= 9 )
			return '0' + nibble;
		else
			return 'A' + ( nibble - 0x0A );
	}

	//____ asciiToNibble() ____________________________________________________________
	inline uint8_t TextTool::asciiToNibble( uint8_t ascii )
	{
		if( ascii >= '0' && ascii <= '9' )
			return 0x00 + ascii - '0';
		else if( ascii >= 'A' && ascii <= 'F' )
			return 0x0A + (ascii - 'A');
		else if( ascii >= 'a' && ascii <= 'f' )
			return 0x0A + (ascii - 'a');

		return 0;
	}

	//____ uint16ToAscii() ____________________________________________________________
	bool TextTool::uint16ToAscii( uint16_t value, uint16_t * pDest, uint32_t maxChars )
	{
		if( 0 == pDest || maxChars < 4 )
			return false;

		uint8ToAscii( value >> 8, (uint16_t*)&pDest[0], 2 );
		uint8ToAscii( value & 0xFF, (uint16_t*)&pDest[2], 2 );

		return true;
	}

	//____ asciiTouint16_t() ____________________________________________________________
	uint16_t TextTool::asciiTouint16_t( const uint16_t * pAscii )
	{
		uint16_t high = asciiTouint8_t( (uint16_t*)&pAscii[0] );
		uint16_t low = asciiTouint8_t( (uint16_t*)&pAscii[2] );

		uint16_t value = high << 8 | low;

		return value;
	}

	//____ uint16ToAscii() ____________________________________________________________
	bool TextTool::uint16ToAscii( uint16_t value, char * pDest, uint32_t maxChars )
	{
		if( 0 == pDest || maxChars < 4 )
			return false;

		uint8ToAscii( value >> 8, (char*)&pDest[0], 2 );
		uint8ToAscii( value & 0xFF, (char*)&pDest[2], 2 );

		return true;
	}

	//____ asciiTouint16_t() ____________________________________________________________
	uint16_t TextTool::asciiTouint16_t( const char * pAscii )
	{
		uint16_t high = asciiTouint8_t( (char*)&pAscii[0] );
		uint16_t low = asciiTouint8_t( (char*)&pAscii[2] );

		uint16_t value = high << 8 | low;

		return value;
	}

	//____ uint8ToAscii( uint16_t* ) ____________________________________________________________
	bool TextTool::uint8ToAscii( uint8_t value, uint16_t * pDest, uint32_t maxChars )
	{
		if( 0 == pDest || maxChars < 2 )
			return false;

		uint8_t high = nibbleToAscii( value >> 4 );
		uint8_t low = nibbleToAscii( value & 0x0F );

		pDest[ 0 ] = high;
		pDest[ 1 ] = low;

		return true;
	}

	//____ asciiTouint8_t() ____________________________________________________________
	uint8_t TextTool::asciiTouint8_t( const uint16_t * pAscii )
	{
		uint8_t high = asciiToNibble( (uint8_t)pAscii[ 0 ] );
		uint8_t low = asciiToNibble( (uint8_t)pAscii[ 1 ] );

		uint8_t value = high << 4 | low;

		return value;
	}

	//____ uint8ToAscii( char* ) ____________________________________________________________
	bool TextTool::uint8ToAscii( uint8_t value, char * pDest, uint32_t maxChars )
	{
		if( 0 == pDest || maxChars < 2 )
			return false;

		uint8_t high = nibbleToAscii( value >> 4 );
		uint8_t low = nibbleToAscii( value & 0x0F );

		pDest[ 0 ] = high;
		pDest[ 1 ] = low;

		return true;
	}

	//____ asciiTouint8_t() ____________________________________________________________
	uint8_t TextTool::asciiTouint8_t( const char * pAscii )
	{
		uint8_t high = asciiToNibble( pAscii[ 0 ] );
		uint8_t low = asciiToNibble( pAscii[ 1 ] );

		uint8_t value = high << 4 | low;

		return value;
	}

	//____ uint16ToUtf8() ____________________________________________________________
	uint32_t TextTool::uint16ToUtf8( uint16_t value, char * pDest, uint32_t maxChars )
	{
		if( maxChars < 1 )
			return 0;

		unsigned int nChars = 0;

		uint16_t charCode = value;
		if( charCode < 128 )
		{
			pDest[nChars++] = (char) charCode;
			if( nChars == maxChars )
				return nChars;
		}
		else if( charCode < 0x800 )
		{
			pDest[nChars++] =  0xC0 | ((char) ((charCode & 0x07C0) >> 6));
			if( nChars == maxChars )
				return nChars;

			pDest[nChars++] =  0x80 | ((char) ((charCode & 0x003F) >> 0));
			if( nChars == maxChars )
				return nChars;
		}
		else
		{
			pDest[nChars++] =  0xE0 | ((char) ((charCode & 0xF000) >> 12));
			if( nChars == maxChars )
				return nChars;

			pDest[nChars++] =  0x80 | ((char) ((charCode & 0x0FC0) >> 6));
			if( nChars == maxChars )
				return nChars;

			pDest[nChars++] =  0x80 | ((char) ((charCode & 0x003F) >> 0));
			if( nChars == maxChars )
				return nChars;
		}

		return nChars;
	}


	//____ getTextUTF8() __________________________________________________________

	/**
		Writes a UTF8 representation of the Char-string (or first part thereof) into
		the specified area. Char effects such as underline, font changes etc are
		ignored.


		@param	maxBytes		Maximum number of bytes for the line, including
								termination character.



		@returns		Length of UTF8-string written to the area (excluding
						termination character). No incomplete multibyte characters
						are written, so number of bytes written can be less
						than maxChars-1 even when whole string didn't fit.

	*/

	uint32_t TextTool::getTextUTF8( const Char * pSrc, char * pDest, uint32_t maxBytes )
	{
		if( maxBytes < 1 || pSrc == 0 || pDest == 0 )
			return 0;

		uint32_t nChars = 0;
		uint16_t charCode;

		while( (charCode = pSrc->code()) != 0 )
		{
			if( charCode < 128 )
			{
				if( nChars +2 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.

				pDest[nChars++] = (char) charCode;
			}
			else if( charCode < 0x800 )
			{
				if( nChars +3 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.

				pDest[nChars++] =  0xC0 | ((char) ((charCode & 0x07C0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((charCode & 0x003F) >> 0));
			}
			else
			{
				if( nChars +4 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.

				pDest[nChars++] =  0xE0 | ((char) ((charCode & 0xF000) >> 12));
				pDest[nChars++] =  0x80 | ((char) ((charCode & 0x0FC0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((charCode & 0x003F) >> 0));
			}

			pSrc++;
		}

		pDest[nChars] = 0;
		return nChars;
	}

	uint32_t TextTool::getTextUTF8( const uint16_t * pSrc, char * pDest, uint32_t maxBytes )
	{
		if( maxBytes < 1 || pSrc == 0 || pDest == 0 )
			return 0;

		uint32_t nChars = 0;
		uint16_t charCode;

		while( (charCode = * pSrc) != 0 )
		{
			if( charCode < 128 )
			{
				if( nChars +2 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.

				pDest[nChars++] = (char) charCode;
			}
			else if( charCode < 0x800 )
			{
				if( nChars +3 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.

				pDest[nChars++] =  0xC0 | ((char) ((charCode & 0x07C0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((charCode & 0x003F) >> 0));
			}
			else
			{
				if( nChars +4 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.

				pDest[nChars++] =  0xE0 | ((char) ((charCode & 0xF000) >> 12));
				pDest[nChars++] =  0x80 | ((char) ((charCode & 0x0FC0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((charCode & 0x003F) >> 0));
			}

			pSrc++;
		}

		pDest[nChars] = 0;
		return nChars;
	}

	//____ getTextUTF8() __________________________________________________________

	uint32_t TextTool::getTextUTF8( const char * pSrc, CodePage codepage, char * pDest, int maxChars )
	{
		const uint16_t * pCP = CodePages::getCodePage( codepage );
		if( !pCP )
		{
			pDest[0] = 0;
			return 0;
		}

		unsigned char * p = (unsigned char *) pSrc;

		int nBytes = 0;
		for( int i = 0 ; i < maxChars && p[i] != 0 ; i++ )
			nBytes += writeUTF8( pCP[p[i]], pDest + nBytes );

		pDest[nBytes] = 0;					// Yes, we do terminate destination!
		return nBytes;
	}



	//____ getTextSizeUTF8() ______________________________________________________

	uint32_t TextTool::getTextSizeUTF8( const Char* pSrc, uint32_t len )
	{
		uint32_t size = 0;
		uint16_t charCode = 0;
		for( uint32_t i = 0 ; i < len && (0 != ( charCode = pSrc->code()) ) ; i++ )
		{
			size++;
			if( charCode > 127 )
			{
				size++;
				if( charCode > 0x7FF )
					size++;
			}
			pSrc++;
		}

		return size;
	}

	//____ getTextSizeUTF8() ____________________________________________________________

	uint32_t TextTool::getTextSizeUTF8( const uint16_t* pSrc, uint32_t len )
	{
		uint32_t size = 0;
		uint16_t charCode = 0;
		for( uint32_t i = 0 ; i < len && (0 != ( charCode = * pSrc) ) ; i++ )
		{
			size++;
			if( charCode > 127 )
			{
				size++;
				if( charCode > 0x7FF )
					size++;
			}
			pSrc++;
		}

		return size;
	}

	//____ getTextSizeUTF8() ____________________________________________________________

	uint32_t TextTool::getTextSizeUTF8( const char * pSrc, CodePage codepage, int maxChars )
	{
		const uint16_t * pCP = CodePages::getCodePage( codepage );
		if( !pCP )
			return 0;

		unsigned char * p = (unsigned char *) pSrc;

		int nBytes = 0;
		for( int i = 0 ; i < maxChars && p[i] != 0 ; i++ )
			nBytes += sizeUTF8( p[i] );

		return nBytes;
	}

	//____ forwardCharacters() ____________________________________________________

	void TextTool::forwardCharacters( const char *& pChar, int nChars )
	{
		for( int i = 0 ; i < nChars && * pChar != 0 ; i++ )
		{
			char c = * pChar++;

			if( (c & 0xC0) == 0xC0 )			// Beginning of multibyte character
			{
				pChar++;						// skip one byte

				if( (c & 0xE0) == 0xE0 )		// Beginning of 3+ byte character?
				{
					pChar++;					// skip one byte

					if( (c & 0xF0) == 0xF0 )	// Beginning of 4 byte character? (not really supported, but we don't want a crash)
						pChar++;
				}
			}
		}

	}



	//____ setCharCode() ______________________________________________________________

	void TextTool::setCharCode( uint16_t charCode, Char * pChar, int nb )
	{
		for( int i = 0 ; i < nb ; i++ )
			pChar[i].m_code = charCode;
	}

	//____ setChars() ______________________________________________________________

	void TextTool::setChars( const Char& ch, Char * pChar, int nb )
	{
		derefStyle( pChar, nb );

		for( int i = 0 ; i < nb ; i++ )
			pChar[i].m_all = ch.m_all;

		if( ch.m_style != 0 )
			TextStyleManager::_getPointer(ch.m_style)->_incRefCount( nb );
	}



	//____ setStyle() ________________________________________________________

	void TextTool::setStyle( TextStyle * pStyle, Char * pChar, int nb )
	{
		int				refCnt = 0;
		int				refCntTotal = 0;
		TextStyle_h		old_style = 0xFFFF;
		TextStyle_h		new_style = pStyle->handle();

		for( int i = 0 ; i < nb ; i++ )
		{
			if( pChar[i].m_style != old_style )
			{
				if( refCnt != 0 )
				{
					if( old_style != 0 )
						TextStyleManager::_getPointer(old_style)->_decRefCount( refCnt );

					refCntTotal += refCnt;
					refCnt = 0;
				}
				old_style = pChar[i].m_style;
			}

			pChar[i].m_style = new_style;
			refCnt++;
		}

		if( refCnt != 0 && old_style != 0 )
			TextStyleManager::_getPointer(old_style)->_decRefCount( refCnt );

		if( new_style != 0 )
			TextStyleManager::_getPointer(new_style)->_incRefCount( refCntTotal + refCnt );
	}


} // namespace wg
