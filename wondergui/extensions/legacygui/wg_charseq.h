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
#ifndef WG_CHARSEQ_DOT_H
#define WG_CHARSEQ_DOT_H
#pragma once

#include <climits>
#include <string>

#include <wg_types.h>

namespace wg
{

	class Char;
	class String;
	class CharBuffer;

	/*

		TODO:

		- copyTo() använder sig av TextTool::readString() som inte rensar attributes...

		DÄREFTER REKURSIVT ARBETA IGENOM:

		- String
		- Char (ta bort softbreak-flaggan).
		- Text (basera på CharBuffer).
		- Text (ändra metodparametrar, utnyttja CharSeq och String).
		- Text (inkludera line-variabler).
		- Text (stöd för varierande radhöjd).

	*/




	//____ CharSeq _______________________________________________________________

	class CharSeq
	{
	public:

		//.____ Creation _______________________________________________________

		CharSeq( const char * pChar );
		CharSeq( const char * pChar, int len );
		CharSeq( const uint16_t * pChar );
		CharSeq( const uint16_t * pChar, int len );
		CharSeq( const Char * pChar );
		CharSeq( const Char * pChar, int len );
		CharSeq( const CharBuffer * pBuffer );
		CharSeq( const CharBuffer * pBuffer, int ofs, int len );
		CharSeq( const std::string& str );
		CharSeq( const std::string& str, int ofs, int len );
		CharSeq( const std::wstring& str );
		CharSeq( const std::wstring& str, int ofs, int len );
		CharSeq( const String& str );
		CharSeq( const String& str, int ofs, int len );
		CharSeq( const CharSeq& seq, int ofs = 0, int len = INT_MAX );

		class CharBasket
		{
		public:
			friend class CharSeq;

			//.____ Creation ___________________________________________________

			CharBasket() {};
			CharBasket(const CharBasket& r) { ptr = r.ptr ; length = r.length; bIsOwner = r.bIsOwner; r.bIsOwner = false; }
			~CharBasket();

			//.____ Properties _________________________________________________

			const Char *	ptr;
			int				length;

		private:
			CharBasket& operator=(const CharBasket& r) { ptr = r.ptr ; length = r.length; bIsOwner = r.bIsOwner; r.bIsOwner = false; return *this; }

			mutable bool	bIsOwner;
		};

		class UTF8Basket
		{
		public:
			friend class CharSeq;

			//.____ Creation ___________________________________________________

			UTF8Basket() {};
			UTF8Basket(const UTF8Basket& r) { ptr = r.ptr ; length = r.length; bIsOwner = r.bIsOwner; r.bIsOwner = false; }
			~UTF8Basket() { if( bIsOwner ) delete [] ptr; }

			//.____ Properties _________________________________________________

			const char *	ptr;
			int				length;		// Length of char string, NOT number of UTF8 characters!

		private:
			UTF8Basket& operator=(const UTF8Basket& r) { ptr = r.ptr ; length = r.length; bIsOwner = r.bIsOwner; r.bIsOwner = false; return *this; }

			mutable bool	bIsOwner;
		};

		class UnicodeBasket
		{
		public:
			friend class CharSeq;

			//.____ Creation ___________________________________________________

			UnicodeBasket() {};
			UnicodeBasket(const UnicodeBasket& r) { ptr = r.ptr ; length = r.length; bIsOwner = r.bIsOwner; r.bIsOwner = false; }
			~UnicodeBasket() { if( bIsOwner ) delete [] ptr; }

			//.____ Properties _________________________________________________

			const uint16_t *	ptr;
			int				length;

		private:
			UnicodeBasket& operator=(const UnicodeBasket& r) { ptr = r.ptr ; length = r.length; bIsOwner = r.bIsOwner; r.bIsOwner = false; return *this; }

			mutable bool	bIsOwner;
		};

		//.____ Content ________________________________________________________

		inline int				length() const { return m_nbChars; }
		int						lengthUtf8() const;
		const CharBasket		getChars() const;
		const UnicodeBasket		getUnicode() const;
		const UTF8Basket		getUtf8() const;
		std::string				getStdString() const;
		std::wstring			getStdWstring() const;
		int						getNbLines() const;

		//.____ Misc ___________________________________________________________

		void					copyTo( Char * pDest ) const;
	//	void					copyTo( char * pDest ) const;			//TODO: implement.
	//	void					copyTo( uint16_t * pDest ) const;			//TODO: implement.

//		void					copyFormattedTo( char * pDest ) const;
//		void					copyFormattedTo( uint16_t * pDest ) const;

	protected:
		CharSeq() {};

		enum class Type
		{
			Empty,
			StyledChars,
			Codepage8,
			Utf8,
			Utf16,
			EscapedUtf8,
			EscapedUtf16

		};

		Type			m_type;
		const void * 	m_pChar;
		int				m_nbChars;		// Length of sequence in number of characters.
	};


	class CharSeqLiteral : public CharSeq
	{
	public:
		//.____ Creation _______________________________________________________

		CharSeqLiteral( const char * pChar );
		CharSeqLiteral( const char * pChar, int len );
		CharSeqLiteral( const uint16_t * pChar );
		CharSeqLiteral( const uint16_t * pChar, int len );
		CharSeqLiteral( const std::string& str );
		CharSeqLiteral( const std::string& str, int ofs, int len );
		CharSeqLiteral( const std::wstring& str );
		CharSeqLiteral( const std::wstring& str, int ofs, int len );
	};




	class CharSeq8 : public CharSeq
	{
		friend class CharSeq;
	public:
		//.____ Creation _______________________________________________________

		CharSeq8( const char * pChar, CodePage codePage = DefaultCodePage );
		CharSeq8( const char * pChar, int len, CodePage codePage = DefaultCodePage );
		CharSeq8( const std::string& str, CodePage codePage = DefaultCodePage );
		CharSeq8( const std::string& str, int ofs, int len, CodePage codePage = DefaultCodePage );

	protected:
		CodePage	m_codepage;
	};


} // namespace wg
#endif //WG_CHARSEQ_DOT_H
