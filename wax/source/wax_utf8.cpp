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

#include <wax_utf8.h>

namespace wax
{
	namespace Utf8
	{

		//____ _decodeLength() ________________________________________________
		//
		// Returns length in bytes of a valid sequence starting at ofs, or 0 if
		// sequence is invalid. Decoded codepoint is written to <codepoint>.

		static int _decodeLength( std::string_view text, int ofs, char32_t& codepoint )
		{
			const uint8_t * p = reinterpret_cast<const uint8_t*>(text.data()) + ofs;
			int left = int(text.size()) - ofs;

			uint8_t c = p[0];

			if( c < 0x80 )
			{
				codepoint = c;
				return 1;
			}

			int		len;
			char32_t	cp;
			char32_t	min;

			if( (c & 0xE0) == 0xC0 )
			{
				len = 2;
				cp = c & 0x1F;
				min = 0x80;
			}
			else if( (c & 0xF0) == 0xE0 )
			{
				len = 3;
				cp = c & 0x0F;
				min = 0x800;
			}
			else if( (c & 0xF8) == 0xF0 )
			{
				len = 4;
				cp = c & 0x07;
				min = 0x10000;
			}
			else
				return 0;

			if( len > left )
				return 0;

			for( int i = 1 ; i < len ; i++ )
			{
				if( (p[i] & 0xC0) != 0x80 )
					return 0;
				cp = (cp << 6) | (p[i] & 0x3F);
			}

			if( cp < min || cp > MaxCodepoint || (cp >= 0xD800 && cp <= 0xDFFF) )
				return 0;

			codepoint = cp;
			return len;
		}

		//____ isValid() ______________________________________________________

		bool isValid( std::string_view text )
		{
			int size = int(text.size());
			int ofs = 0;
			char32_t cp;

			while( ofs < size )
			{
				if( uint8_t(text[ofs]) < 0x80 )
				{
					ofs++;
					continue;
				}

				int len = _decodeLength(text, ofs, cp);
				if( len == 0 )
					return false;
				ofs += len;
			}
			return true;
		}

		//____ countCodepoints() ______________________________________________

		int countCodepoints( std::string_view text )
		{
			int size = int(text.size());
			int ofs = 0;
			int count = 0;

			while( ofs < size )
			{
				ofs = next(text, ofs);
				count++;
			}
			return count;
		}

		//____ decode() _______________________________________________________

		char32_t decode( std::string_view text, int& ofs )
		{
			if( ofs < 0 || ofs >= int(text.size()) )
				return 0;

			char32_t cp;
			int len = _decodeLength(text, ofs, cp);
			if( len == 0 )
			{
				ofs++;
				return ReplacementChar;
			}

			ofs += len;
			return cp;
		}

		//____ encode() _______________________________________________________

		int encode( char32_t cp, char * pDest )
		{
			if( cp > MaxCodepoint || (cp >= 0xD800 && cp <= 0xDFFF) )
				cp = ReplacementChar;

			if( cp < 0x80 )
			{
				pDest[0] = char(cp);
				return 1;
			}
			if( cp < 0x800 )
			{
				pDest[0] = char(0xC0 | (cp >> 6));
				pDest[1] = char(0x80 | (cp & 0x3F));
				return 2;
			}
			if( cp < 0x10000 )
			{
				pDest[0] = char(0xE0 | (cp >> 12));
				pDest[1] = char(0x80 | ((cp >> 6) & 0x3F));
				pDest[2] = char(0x80 | (cp & 0x3F));
				return 3;
			}

			pDest[0] = char(0xF0 | (cp >> 18));
			pDest[1] = char(0x80 | ((cp >> 12) & 0x3F));
			pDest[2] = char(0x80 | ((cp >> 6) & 0x3F));
			pDest[3] = char(0x80 | (cp & 0x3F));
			return 4;
		}

		//____ append() _______________________________________________________

		void append( std::string& text, char32_t codepoint )
		{
			char buffer[4];
			int len = encode(codepoint, buffer);
			text.append(buffer, len);
		}

		//____ fromCodepoint() ________________________________________________

		std::string fromCodepoint( char32_t codepoint )
		{
			char buffer[4];
			int len = encode(codepoint, buffer);
			return std::string(buffer, len);
		}

		//____ next() _________________________________________________________

		int next( std::string_view text, int ofs )
		{
			int size = int(text.size());
			if( ofs >= size )
				return size;
			if( ofs < 0 )
				return 0;

			char32_t cp;
			int len = _decodeLength(text, ofs, cp);
			return ofs + (len == 0 ? 1 : len);
		}

		//____ prev() _________________________________________________________

		int prev( std::string_view text, int ofs )
		{
			if( ofs <= 0 )
				return 0;
			if( ofs > int(text.size()) )
				return int(text.size());

			// Find a lead byte at most 3 bytes back and check that its sequence ends exactly at ofs.

			int candidate = ofs - 1;
			while( candidate > 0 && ofs - candidate < 4 && isContinuation(text[candidate]) )
				candidate--;

			char32_t cp;
			int len = _decodeLength(text, candidate, cp);
			if( len != 0 && candidate + len == ofs )
				return candidate;

			return ofs - 1;
		}

		//____ alignToCodepoint() _____________________________________________

		int alignToCodepoint( std::string_view text, int ofs )
		{
			int size = int(text.size());
			if( ofs <= 0 )
				return 0;
			if( ofs >= size )
				return size;

			if( !isContinuation(text[ofs]) )
				return ofs;

			int candidate = ofs - 1;
			while( candidate > 0 && ofs - candidate < 3 && isContinuation(text[candidate]) )
				candidate--;

			char32_t cp;
			int len = _decodeLength(text, candidate, cp);
			if( len != 0 && candidate + len > ofs )
				return candidate;

			return ofs;			// Stray continuation byte, a codepoint (U+FFFD) of its own.
		}

		//____ offsetOfCodepoint() ____________________________________________

		int offsetOfCodepoint( std::string_view text, int index, int ofs )
		{
			int size = int(text.size());
			while( index > 0 && ofs < size )
			{
				ofs = next(text, ofs);
				index--;
			}
			return ofs;
		}

		//____ sanitize() _____________________________________________________

		std::string sanitize( std::string_view text )
		{
			std::string out;
			out.reserve(text.size());

			int size = int(text.size());
			int ofs = 0;
			char32_t cp;

			while( ofs < size )
			{
				int len = _decodeLength(text, ofs, cp);
				if( len == 0 )
				{
					append(out, ReplacementChar);
					ofs++;
				}
				else
				{
					out.append(text.data() + ofs, len);
					ofs += len;
				}
			}
			return out;
		}

		bool sanitize( std::string& text )
		{
			if( isValid(text) )
				return false;

			text = sanitize( std::string_view(text) );
			return true;
		}

		//____ fromUtf16() ____________________________________________________

		std::string fromUtf16( std::u16string_view text )
		{
			std::string out;
			out.reserve(text.size());

			size_t size = text.size();
			for( size_t i = 0 ; i < size ; i++ )
			{
				char32_t c = text[i];

				if( c >= 0xD800 && c <= 0xDBFF && i+1 < size && text[i+1] >= 0xDC00 && text[i+1] <= 0xDFFF )
				{
					c = 0x10000 + ((c - 0xD800) << 10) + (text[i+1] - 0xDC00);
					i++;
				}
				else if( c >= 0xD800 && c <= 0xDFFF )
					c = ReplacementChar;

				append(out, c);
			}
			return out;
		}

		std::string fromUtf16( const uint16_t * pText, int len )
		{
			if( pText == nullptr )
				return std::string();

			if( len < 0 )
			{
				len = 0;
				while( pText[len] != 0 )
					len++;
			}

			return fromUtf16( std::u16string_view( reinterpret_cast<const char16_t*>(pText), len ) );
		}

		//____ toUtf16() ______________________________________________________

		std::u16string toUtf16( std::string_view text )
		{
			std::u16string out;
			out.reserve(text.size());

			int size = int(text.size());
			int ofs = 0;
			while( ofs < size )
			{
				char32_t c = decode(text, ofs);
				if( c >= 0x10000 )
				{
					c -= 0x10000;
					out.push_back( char16_t(0xD800 + (c >> 10)) );
					out.push_back( char16_t(0xDC00 + (c & 0x3FF)) );
				}
				else
					out.push_back( char16_t(c) );
			}
			return out;
		}

		//____ fromUtf32() ____________________________________________________

		std::string fromUtf32( std::u32string_view text )
		{
			std::string out;
			out.reserve(text.size());

			for( char32_t c : text )
				append(out, c);

			return out;
		}

		//____ toUtf32() ______________________________________________________

		std::u32string toUtf32( std::string_view text )
		{
			std::u32string out;
			out.reserve(text.size());

			int size = int(text.size());
			int ofs = 0;
			while( ofs < size )
				out.push_back( decode(text, ofs) );

			return out;
		}

		//____ fromLatin1() ___________________________________________________

		std::string fromLatin1( std::string_view text )
		{
			std::string out;
			out.reserve(text.size());

			for( char c : text )
				append(out, char32_t(uint8_t(c)) );

			return out;
		}

	}
} // namespace wax
