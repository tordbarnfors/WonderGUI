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
#ifndef	WG_CHAR_DOT_H
#define	WG_CHAR_DOT_H
#pragma once

#include <wg_types.h>

#include <wg_textstylemanager.h>

#include <wg_texttool.h>

namespace wg
{


	//_____ Char ________________________________________________________________

	/** The character class in WonderGUI is different from other toolkits in that
		it not only holds a 16-bit unicode character but also specifies what
		display properties the character has such as font, style, color etc by
		containing a handle to a properties specification.

		This makes the characters slightly heavier (32-bits instead of 16 or 8 bits)
		and adds some overhead to creation/deletion since CharProp objects needs to
		be referenced and dereferenced. On the flipside it gives all displayed text
		"rich text"-functionality in an easy and consistent way that hardly affects
		rendering performance.

		Because of the overhead involved in manipulation and storing Chars, it is
		not recommended that Char or any of the Char containers (CharBuffer,
		String and Text) is used for general purpose text not meant for display.

		Also, when manipulating chunks of text stored as Chars it is highly
		recommended that you use the CharBuffer container class instead of
		manipulating arrays of Chars. All the container classes
		have methods for manipulating properties on multiple characters which are
		both faster and more convenient than manipulating the Chars one by one.

	*/

	class Char
	{
	friend class TextStyle;
	friend class TextTool;
	friend class CharBuffer;

	public:

		//.____ Creation ___________________________________________________

		/// Initializes an empty character containing null (End of Text) and no properties.

		Char() : m_all(0) {};

		/// Initializes a character to contain the charcode and style of the specified character.

		Char( const Char& r ) { m_all = r.m_all; if( m_style ) TextStyleManager::_getPointer(m_style)->_incRefCount(); }

		/// Initializes a character to contain the specified charcode and no style.

		Char( uint16_t charCode ) { m_style = 0; m_code = charCode; }

		/// Initializes a character to contain the charcode and style as specified.

		Char(uint16_t charCode, TextStyle* pStyle) { m_code = charCode; if (pStyle) { m_style = pStyle->handle(); pStyle->_incRefCount(); } else m_style = 0; }
		~Char() { if( m_style ) TextStyleManager::_getPointer(m_style)->_decRefCount(); }

		//.____ Content ___________________________________________________________

		inline bool				equals(const Char& ch) const { return m_all == ch.m_all; }

								/// Sets the character code part of the character without affecting the styling.

		inline void				setCode( uint16_t _code ) { m_code = _code; }

								/// Gets the character code part of the character object.
								///
								/// @return The character code part of the character.

		inline uint16_t			code() const { return m_code; }

								/// Sets the style for the character.

		inline void				setStyle( TextStyle * pStyle ) { if(m_style) TextStyleManager::_getPointer(m_style)->_decRefCount(); if (pStyle) { m_style = pStyle->handle(); pStyle->_incRefCount(); } else m_style = 0; }

								/// Gets a handle to the TextStyle of the character.
								///
								/// Getting a handle is faster than getting a pointer. Use this method if you just want to check
								/// what style (if any) is used and don't need access to the TextStyle object.
								///
								/// @return A handle to the TextStyle of the character 0 if none.

		inline TextStyle_h		styleHandle() const { return m_style; }

								/// Gets a pointer to the TextStyle of the character.
								///
								/// @return Pointer to the TextStyle of the character if one is specified, or null.

		inline TextStyle_p		stylePtr() const { return m_style == 0 ? 0 : TextStyleManager::_getPointer(m_style); }


								/// Checks if the character terminates the line.
								/// @return True if the character code is End-of-line or End-of-Text (null).

		inline bool 			isEndOfLine() const { if( m_code == '\n' || m_code == 0 ) return true; return false; }

								/// Checks if the character terminates the text.
								/// @return True if the character code is End-of-Text (null).

		inline bool				isEndOfText() const { if( m_code == 0 ) return true; return false; }

								/// Checks if the character is a whitespace.
								/// @return True if the character code is a space or ExtChar::NoBreakSpace.

		inline bool 			isWhitespace() const { if( m_code == ' ' || m_code == (uint16_t)ExtChar::NoBreakSpace ) return true; return false; }

		//.____ Operators ______________________________________________________

		inline Char & operator=(const Char &ref)
		{
			if (m_style == ref.m_style)		// Don't dec/inc ref if properties are same! Could be a self-assignment...
			{							// also speeds things up...
				m_all = ref.m_all;
			}
			else
			{
				if (m_style)
					TextStyleManager::_getPointer(m_style)->_decRefCount();
				m_all = ref.m_all;
				if (m_style)
					TextStyleManager::_getPointer(m_style)->_incRefCount();
			}
			return *this;
		}

	protected:


	private:

		union
		{
			struct
			{
				uint16_t	m_code;
				TextStyle_h	m_style;				// 0 = Default properties of Text.
			};
			uint32_t	m_all;					// For quickly copying all...
		};
	};




} // namespace wg
#endif //WG_CHAR_DOT_H
