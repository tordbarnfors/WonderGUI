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
#ifndef	WAX_UTF8_DOT_H
#define WAX_UTF8_DOT_H
#pragma once

#include <string>
#include <string_view>
#include <cstdint>

namespace wax
{

	//____ Utf8 _______________________________________________________________
	/**
		Helper functions for working with UTF-8 encoded text.

		Text in WonderGUI is stored as UTF-8 in std::string and all positions
		in a text are byte offsets. These functions decode and encode codepoints,
		step between codepoints and validate text.

		Functions taking a std::string_view are safe on any input. Invalid
		sequences (broken, overlong, surrogates or beyond U+10FFFF) decode
		as one U+FFFD per invalid byte.

		Functions named "...Unchecked" assume valid UTF-8 and are meant for text
		that already has been validated, like the text in a text component.
	*/

	namespace Utf8
	{
		const char32_t	ReplacementChar = 0xFFFD;
		const char32_t	MaxCodepoint = 0x10FFFF;

		//.____ Inspection ________________________________________________

		inline bool		isContinuation( char c ) { return (uint8_t(c) & 0xC0) == 0x80; }
		inline int		sequenceLength( char leadByte );		// Length of sequence from its lead byte, 1 for invalid lead bytes.
		inline int		encodedLength( char32_t codepoint );	// Bytes needed to encode codepoint, 3 for invalid ones (encoded as U+FFFD).

		bool			isValid( std::string_view text );
		int				countCodepoints( std::string_view text );

		//.____ Decoding __________________________________________________

		char32_t		decode( std::string_view text, int& ofs );		// Decodes codepoint at ofs and moves ofs to next one.
		inline char32_t	decodeUnchecked( const char *& p );				// Decodes codepoint at p and moves p to next one.
		inline char32_t	codepointAt( std::string_view text, int ofs );

		//.____ Encoding __________________________________________________

		int				encode( char32_t codepoint, char * pDest );		// Writes 1-4 bytes, returns number of bytes written.
		void			append( std::string& text, char32_t codepoint );
		std::string		fromCodepoint( char32_t codepoint );

		//.____ Navigation ________________________________________________

		int				next( std::string_view text, int ofs );				// Offset of the codepoint following the one at ofs.
		int				prev( std::string_view text, int ofs );				// Offset of the codepoint preceding ofs.
		int				alignToCodepoint( std::string_view text, int ofs );	// Moves ofs back to beginning of codepoint it is within.
		int				offsetOfCodepoint( std::string_view text, int index, int startOfs = 0 );	// Byte offset of codepoint number <index>.

		//.____ Conversion ________________________________________________

		std::string		sanitize( std::string_view text );				// Replaces invalid sequences with U+FFFD.
		bool			sanitize( std::string& text );					// In place, returns true if anything was replaced.

		std::string		fromUtf16( std::u16string_view text );
		std::string		fromUtf16( const uint16_t * pText, int len = -1 );	// len -1 = null terminated.
		std::u16string	toUtf16( std::string_view text );
		std::string		fromUtf32( std::u32string_view text );
		std::u32string	toUtf32( std::string_view text );

		std::string		fromLatin1( std::string_view text );			// ISO-8859-1 to UTF-8.
	}

	//____ Utf8::sequenceLength() _____________________________________________

	inline int Utf8::sequenceLength( char leadByte )
	{
		uint8_t c = uint8_t(leadByte);

		if( c < 0x80 )
			return 1;
		if( (c & 0xE0) == 0xC0 )
			return 2;
		if( (c & 0xF0) == 0xE0 )
			return 3;
		if( (c & 0xF8) == 0xF0 )
			return 4;
		return 1;
	}

	//____ Utf8::encodedLength() ______________________________________________

	inline int Utf8::encodedLength( char32_t codepoint )
	{
		if( codepoint < 0x80 )
			return 1;
		if( codepoint < 0x800 )
			return 2;
		if( codepoint < 0x10000 || codepoint > MaxCodepoint )
			return 3;
		return 4;
	}

	//____ Utf8::decodeUnchecked() ____________________________________________

	inline char32_t Utf8::decodeUnchecked( const char *& _p )
	{
		const uint8_t *& p = (const uint8_t *&) _p;

		char32_t c = *p++;
		if( c < 0x80 )
			return c;

		if( c < 0xE0 )
		{
			c = ((c & 0x1F) << 6) | (p[0] & 0x3F);
			p += 1;
		}
		else if( c < 0xF0 )
		{
			c = ((c & 0x0F) << 12) | ((p[0] & 0x3F) << 6) | (p[1] & 0x3F);
			p += 2;
		}
		else
		{
			c = ((c & 0x07) << 18) | ((p[0] & 0x3F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
			p += 3;
		}
		return c;
	}

	//____ Utf8::codepointAt() ________________________________________________

	inline char32_t Utf8::codepointAt( std::string_view text, int ofs )
	{
		return decode(text, ofs);
	}

} // namespace wax
#endif //WAX_UTF8_DOT_H
