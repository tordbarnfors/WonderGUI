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
#ifndef	WG_TEXTTOOL_DOT_H
#define WG_TEXTTOOL_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_textstyle.h>
#include <wg_charseq.h>
#include <wg_pointers.h>

namespace wg
{

	class Font;
	class Font;
	class Char;
	class TextNode;

	//____ TextTool _____________________________________________________________

	class TextTool		/** @private */
	{
		friend class CharBuffer;
		friend class String;
		friend class CharSeq;
		friend class CharSeq::CharBasket;

	public:

		inline static bool	cmpUTF8( const char * pChar1, uint16_t char2 );
		inline static int	sizeUTF8( uint16_t ch );

		inline static uint16_t readChar( const char *& pStr );
		static uint16_t	parseChar( const char * pStr );	// Should be null terminated

		static uint32_t	readString( const char *& pSrc, uint16_t * pDst, uint32_t maxChars = INT_MAX );
		static uint32_t	readString( const char *& pSrc, Char * pDst, uint32_t maxChars = INT_MAX );
		static uint32_t	readString( const uint16_t *& pSrc, Char * pDst, uint32_t maxChars = INT_MAX );
		static uint32_t	readString( const char *& pSrc, CodePage codepage, Char * pDst, uint32_t maxChars = INT_MAX );
		static uint32_t	readString( const char *& pSrc, CodePage codepage, uint16_t * pDst, uint32_t maxChars = INT_MAX );

		static int		countChars( const char * pStr, int strlen = INT_MAX );
		static int		countChars( const uint16_t * pStr, int strlen = INT_MAX );

		static uint32_t	countLines( const char * pStr );
		static uint32_t	countLines( const uint16_t * pStr );
		static uint32_t	countLines( const Char * pStr );

		static uint32_t	countWhitespaces( const char * pStr, uint32_t len = INT_MAX );
		static uint32_t	countWhitespaces( const uint16_t * pStr, uint32_t len = INT_MAX  );
		static uint32_t	countWhitespaces( const Char * pStr, uint32_t len = INT_MAX  );

		static uint32_t	countLineChars( const char * pStr, uint32_t len = INT_MAX );
		static uint32_t	countLineChars( const uint16_t * pStr, uint32_t len = INT_MAX );
		static uint32_t	countLineChars( const Char * pStr, uint32_t len = INT_MAX );

		static void		countCharsLines( const char * pStr, uint32_t& putChars, uint32_t& putLines, uint32_t strlen = INT_MAX );
		static void		countCharsLines( const uint16_t * pStr, uint32_t& putChars, uint32_t& putLines, uint32_t strlen = INT_MAX );
		static void		countCharsLines( const Char * pStr, uint32_t& putChars, uint32_t& putLines, uint32_t strlen = INT_MAX );

		static int		copyChars( const Char * pSrc, Char * pDst, int maxChars = INT_MAX );

		static char * nextLine( char * pStr );


		static uint32_t	writeUTF8( uint16_t charcode, char * pDest );

		//____

		static bool		uint16ToAscii( uint16_t value, uint16_t * pDest, uint32_t maxChars );
		static uint16_t	asciiTouint16_t( const uint16_t * pAscii );
		static bool		uint16ToAscii( uint16_t value, char * pDest, uint32_t maxChars );
		static uint16_t	asciiTouint16_t( const char * pAscii );

		static bool		uint8ToAscii( uint8_t value, uint16_t * pDest, uint32_t maxChars );
		static uint8_t	asciiTouint8_t( const uint16_t * pAscii );
		static bool		uint8ToAscii( uint8_t value, char * pDest, uint32_t maxChars );
		static uint8_t	asciiTouint8_t( const char * pAscii );

		static uint32_t	uint16ToUtf8( uint16_t value, char * pDest, uint32_t maxChars );


		static uint32_t	getTextUTF8( const Char * pSrc, char * pDest, uint32_t maxBytes );
		static uint32_t	getTextUTF8( const uint16_t * pSrc, char * pDest, uint32_t maxBytes );
		static uint32_t	getTextUTF8( const char * pSrc, CodePage codepage, char * pDest, int maxChars = INT_MAX );

		static uint32_t	getTextSizeUTF8( const Char * pSrc, uint32_t maxChars = INT_MAX );
		static uint32_t	getTextSizeUTF8( const uint16_t * pSrc, uint32_t maxChars = INT_MAX );
		static uint32_t	getTextSizeUTF8( const char * pSrc, CodePage codepage, int maxChars = INT_MAX );

		static uint32_t	strlen( const uint16_t * pSrc );
		static uint32_t	strlen( const Char * pSrc );
		static int		strcmp( const uint16_t * pStr1, const uint16_t * pStr2 );
		static int		strcmp( const Char * pStr1, const Char * pStr2 );
        static int      strcmp( const Char * pStr1, const char * pCString );
        static int      strcmp( const Char * pStr1, const std::string& str ) { return strcmp( pStr1, str.c_str() ); }
		static int		charcodecmp( const Char * pStr1, const Char * pStr2 );
        static int      charcodecmp( const Char * pStr1, const char * pCString );
        static int      charcodecmp( const Char * pStr1, const std::string& str ) { return charcodecmp( pStr1, str.c_str() ); }
		static int		charcodecmpIgnoreCase( const Char * pStr1, const Char * pStr2 );
        static int      charcodecmpIgnoreCase( const Char * pStr1, const char * pCString );
        static int      charcodecmpIgnoreCase( const Char * pStr1, const std::string& str ) { return charcodecmpIgnoreCase( pStr1, str.c_str() ); }

		static void		forwardCharacters( const char *& pChar, int nChars );

		static void			setCharCode( uint16_t glyph, Char * pChar, int nb );
		static void			setChars( const Char& ch, Char * pChar, int nb );

		static void			setStyle( TextStyle * pStyle, Char * pChar, int nb );

		inline static void	clearStyle( Char * pChar, uint32_t nb ) { setStyle( 0, pChar, nb ); }

		static char *           itoa( int value, char * str, int base );

		static BreakRules		isBreakAllowed( uint16_t chr, int breakLevel = 3 );

		static void				clearBreakRules();
		static bool				setBreakRule( unsigned char character, int level, BreakRules rule );
		static void				setDefaultBreakRules();

		//____


	private:

		inline static uint8_t	nibbleToAscii( uint8_t nibble );
		inline static uint8_t	asciiToNibble( uint8_t ascii );

		static void	refStyle( Char * p, int n );
		static void	derefStyle( Char * p, int n );
	};

	//____ readChar() _____________________________________________________________

	inline uint16_t TextTool::readChar( const char *& pStr )
	{

	#ifdef SAFE_UTF8	// SAFE UTF8 VERSION

		uint16_t g = * ((const unsigned char *&)pStr)++;

		if( g < 128 )
			return g;											// 1 byte character (7 bits data).

		// Skip incomplete character if we started in the middle of one.

		while( (g & 0x40) == 0 )
		{
			if( g == 0 )
				return 0;
			g = * ((const unsigned char *&)pStr)++;
		}

		// Decode character the safer way.

		if( (g & 0xE0) == 0xC0 )					// 2 bytes character (11 bits data).
		{
			g &= 0x1F;
			g <<= 6;
			if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
			g += ((*pStr++) & 0x3F);
		}
		else if( (g & 0xF0) == 0xE0 )			// 3 bytes character (16 bits data).
		{
			g &= 0x0F;
			g <<= 6;
			if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
			g += ((*pStr++) & 0x3F);
			g <<= 6;
			if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
			g += ((*pStr++) & 0x3F);
		}
		else if( (g & 0xF8) == 0xF0 )			// 4 bytes character (21 bits data).
		{
			g &= 0x07;
			g <<= 6;
			if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
			g += ((*pStr++) & 0x3F);
			g <<= 6;
			if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
			g += ((*pStr++) & 0x3F);
			g <<= 6;
			if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
			g += ((*pStr++) & 0x3F);
		}
		else
			return 0xFFFFFFFF;					// Broken character (or more than 4 bytes which we don't support)

	#else // FAST UTF8 VERSION

		uint32_t g = *((const unsigned char *&)pStr)++;

		if( g < 128 )
			return g;											// 1 byte character (7 bits data).

		g &= 0x3F;
		g <<= 6;
		g += ((*pStr++) & 0x3F);

		if( g >= (1<<11) )								// 3 bytes character (16 bits data).
		{
			g &= (1<<11)-1;
			g <<= 6;
			g += ((*pStr++) & 0x3F);
		}
	#endif
		return g;
	}

	//____ cmpUTF8() ______________________________________________________________

	inline bool TextTool::cmpUTF8( const char * pChar1, uint16_t char2 )
	{
		uint32_t g = (unsigned char) pChar1[0];

		if( g >= 128 )
		{
			g &= 0x3F;
			g <<= 6;
			g += ((pChar1[1]) & 0x3F);

			if( g >= (1<<11) )								// 3 bytes character (16 bits data).
			{
				g &= (1<<11)-1;
				g <<= 6;
				g += ((pChar1[2]) & 0x3F);
			}
		}

		return (g == char2);
	}

	//____ sizeUTF8() _____________________________________________________________

	inline int TextTool::sizeUTF8( uint16_t ch )
	{
		if( ch < 128 )
			return 1;

		if( ch < (1<<11) )
			return 2;

		return 3;
	}



	typedef TextTool TextParser;


} // namespace wg
#endif //WG_TEXTTOOL_DOT_H
