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
#include <memory.h>
#include <string.h>

#include <wg_charseq.h>
//#include <wg_texttool.h>
#include <wg_charbuffer.h>
#include <wg_string.h>

namespace wg
{


	//____ constructors ____________________________________________________________


	CharSeq::CharSeq( const char * pChar )
	{
		m_type 		= Type::Utf8;
		m_pChar		= (void *) pChar;
		m_nbChars	= TextTool::countChars(pChar);
	}

	CharSeq::CharSeq( const char * pChar, int len )
	{
		m_type 		= Type::Utf8;
		m_pChar		= (void *) pChar;
		m_nbChars	= TextTool::countChars(pChar, len);
	}

	CharSeq::CharSeq( const uint16_t * pChar )
	{
		m_type 		= Type::Utf16;
		m_pChar		= (void *) pChar;
		m_nbChars	= TextTool::countChars(pChar);

	}
	CharSeq::CharSeq( const uint16_t * pChar, int len )
	{
		m_type 		= Type::Utf16;
		m_pChar		= (void *) pChar;
		m_nbChars	= len;
	}

	CharSeq::CharSeq( const std::string& str )
	{
		m_type 		= Type::Utf8;
		m_pChar     = (void *) str.c_str();
		m_nbChars   = TextTool::countChars(str.c_str());
	}

	CharSeq::CharSeq( const std::string& str, int ofs, int len )
	{
		const char * p = str.c_str();
		int strlen = (int) str.length();

		if( ofs + len > strlen )
		{
			if( ofs > strlen )
				ofs = strlen;
			len = strlen - ofs;
		}

		m_type 		= Type::EscapedUtf8;
		m_pChar     = p + ofs;
		m_nbChars   = TextTool::countChars(p+ofs, len);
	}

	CharSeq::CharSeq( const std::wstring& str )
	{
		m_type      = Type::EscapedUtf16;
		m_pChar     = (void *) str.c_str();
		m_nbChars   = TextTool::countChars( (uint16_t*) str.c_str());
	}

	CharSeq::CharSeq( const std::wstring& str, int ofs, int len )
	{
		const wchar_t * p = str.c_str();
		int strlen = (int) str.length();

		if( ofs + len > strlen )
		{
			if( ofs > strlen )
				ofs = strlen;
			len = strlen - ofs;
		}

		m_type      = Type::Utf16;
		m_pChar     = p + ofs;
		m_nbChars   = TextTool::countChars( ((uint16_t*)p)+ofs, len);
	}


	CharSeq::CharSeq( const Char * pChar )
	{
		m_type		= Type::StyledChars;
		m_pChar		= (void *) pChar;
		m_nbChars	= TextTool::strlen(pChar);
	}

	CharSeq::CharSeq( const Char * pChar, int len )
	{
		m_type      = Type::StyledChars;
		m_pChar     = (void *) pChar;
		m_nbChars   = len;
	}



	CharSeq::CharSeq( const CharBuffer * pBuffer )
	{
		m_type		= Type::StyledChars;
		m_pChar		= (void *) pBuffer->chars();
		m_nbChars	= pBuffer->nbChars();
	}

	CharSeq::CharSeq( const CharBuffer * pBuffer, int ofs, int len )
	{
		int realNbChars = (int) pBuffer->nbChars();
		if( ofs + len > realNbChars )
		{
			if( ofs > realNbChars )
				ofs = realNbChars;
			len = realNbChars - ofs;
		}

		m_type		= Type::StyledChars;
		m_pChar		= (void *)  (pBuffer->chars() + ofs);
		m_nbChars	= len;
	}

	CharSeq::CharSeq( const String& str )
	{
		m_type		= Type::StyledChars;
		m_pChar		= (void *) str.chars();
		m_nbChars	= str.length();
	}

	CharSeq::CharSeq( const String& str, int ofs, int len )
	{
		int realNbChars = (int) str.length();
		if( ofs + len > realNbChars )
		{
			if( ofs > realNbChars )
				ofs = realNbChars;
			len = realNbChars - ofs;
		}

		m_type		= Type::StyledChars;
		m_pChar		= (void *) (str.chars() + ofs);
		m_nbChars	= len;
	}

	CharSeq::CharSeq( const CharSeq& seq, int ofs, int len )
	{
		if( ofs > seq.m_nbChars )
			ofs = seq.m_nbChars;

		if( len > seq.m_nbChars - ofs )
			len = seq.m_nbChars - ofs;

		m_type = seq.m_type;
		m_nbChars = len;

		switch( seq.m_type )
		{
			case Type::StyledChars:
				m_pChar = ((const Char *)seq.m_pChar) + ofs;
				break;
			case Type::Utf8:
			{
				const char * p = (const char *) seq.m_pChar;
				TextTool::forwardCharacters( p, ofs );
				m_pChar = p;
				break;
			}
			case Type::Utf16:
				m_pChar = ((const uint16_t *)seq.m_pChar) + ofs;
				break;
			case Type::Codepage8:
			{
				m_pChar = (const char*) seq.m_pChar + ofs;
				break;
			}
			default:						// EMPTY
				m_pChar = seq.m_pChar;
				break;
		}
	}



	//____ lengthUtf8() ___________________________________________________________

	int CharSeq::lengthUtf8() const
	{
		switch( m_type )
		{
		case Type::StyledChars:
			return (int) TextTool::getTextSizeUTF8( (const Char*) m_pChar, m_nbChars );
		case Type::Utf8:
		{
			const char * p = (const char *) m_pChar;
			TextTool::forwardCharacters( p, m_nbChars );
			return int(p - (const char *) m_pChar);
		}
		case Type::Utf16:
			return (int) TextTool::getTextSizeUTF8( (const uint16_t*) m_pChar, m_nbChars );
		case Type::Codepage8:
			return (int) TextTool::getTextSizeUTF8( (const char *) m_pChar, ((CharSeq8*)this)->m_codepage, m_nbChars );
		default:						// EMPTY
			return 0;
		}
	}


	//____ getNbLines() ____________________________________________________________

	int CharSeq::getNbLines() const
	{
		int nbLines = 1;		// We always have at least one line...

		switch( m_type )
		{
			case Type::StyledChars:
				for( int i = 0 ; i < m_nbChars ; i++ )
					if( ((const Char*)m_pChar)[i].isEndOfLine() )
						nbLines++;
				break;
			case Type::Utf8:
			case Type::Codepage8:
				for( int i = 0 ; i < m_nbChars ; i++ )
					if( ((const char*)m_pChar)[i] == '\n' )
						nbLines++;
				break;

			default:					// EMPTY
				break;
		}

		return nbLines;
	}


	//___ copyTo() _________________________________________________________________

	void CharSeq::copyTo( Char * pDest ) const
	{
		switch( m_type )
		{
			case Type::StyledChars:
				TextTool::copyChars( (Char*) m_pChar, pDest, m_nbChars );
				break;

			case Type::Utf8:
			{
				const char * pSrc = (const char*) m_pChar;

				TextTool::derefStyle( pDest, m_nbChars );
				TextTool::readString( pSrc, pDest, m_nbChars );
				break;
			}
			case Type::Utf16:
			{
				const uint16_t * pSrc = (const uint16_t*) m_pChar;

				TextTool::derefStyle( pDest, m_nbChars );
				TextTool::readString( pSrc, pDest, m_nbChars );
				break;
			}
			case Type::Codepage8:
			{
				const char * pSrc = (const char*) m_pChar;
				TextTool::derefStyle( pDest, m_nbChars );
				TextTool::readString( pSrc, ((CharSeq8*)this)->m_codepage, pDest, m_nbChars );
				break;
			}

			default:
				break;
		}
	}

	//____ getChars() ______________________________________________________________

	const CharSeq::CharBasket CharSeq::getChars() const
	{
		CharBasket	basket;

		switch( m_type )
		{
			case Type::StyledChars:
				basket.ptr = (const Char *) m_pChar;
				basket.length = m_nbChars;
				basket.bIsOwner = false;
				return basket;

			case Type::Utf8:
			{
				int bytes = sizeof(Char)*m_nbChars;

				Char * p = (Char *) new char[bytes];
				memset( p, 0, bytes);
				const char * pSrc = (char*) m_pChar;
				TextTool::readString( pSrc, p, m_nbChars );

				basket.ptr = p;
				basket.length = m_nbChars;
				basket.bIsOwner = true;
				return basket;
			}
			case Type::Utf16:
			{
				int bytes = sizeof(Char)*m_nbChars;

				Char * p = (Char *) new char[bytes];
				memset( p, 0, bytes);
				const uint16_t * pSrc = (uint16_t*) m_pChar;
				TextTool::readString( pSrc, p, m_nbChars );

				basket.ptr = p;
				basket.length = m_nbChars;
				basket.bIsOwner = true;
				return basket;
			}
			case Type::Codepage8:
			{
				int bytes = sizeof(Char)*m_nbChars;

				Char * p = (Char *) new char[bytes];
				memset( p, 0, bytes);
				const char * pSrc = (char*) m_pChar;
				TextTool::readString( pSrc, ((CharSeq8*)this)->m_codepage, p, m_nbChars );

				basket.ptr = p;
				basket.length = m_nbChars;
				basket.bIsOwner = true;
				return basket;
			}

			default:					// EMPTY

				basket.ptr = 0;
				basket.length = 0;
				basket.bIsOwner = false;
				return basket;
		}
	}





	//____ getUnicode() ____________________________________________________________

	const CharSeq::UnicodeBasket CharSeq::getUnicode() const
	{
		UnicodeBasket	basket;

		switch( m_type )
		{
			case Type::StyledChars:
			{
				uint16_t * p = new uint16_t[m_nbChars];
				for( int i = 0 ; i < m_nbChars ; i++ )
					p[i] = ((Char*)m_pChar)[i].code();
				basket.ptr = p;
				basket.length = m_nbChars;
				basket.bIsOwner = true;
				return basket;
			}
			case Type::Utf8:
			{
				uint16_t * p = new uint16_t[m_nbChars];
				const char * pSrc = (const char *) m_pChar;
				TextTool::readString( pSrc, p, m_nbChars );
				basket.ptr = p;
				basket.length = m_nbChars;
				basket.bIsOwner = true;
				return basket;
			}
			case Type::Utf16:
				basket.ptr = (const uint16_t *) m_pChar;
				basket.length = m_nbChars;
				basket.bIsOwner = false;
				return basket;

			case Type::Codepage8:
			{
				uint16_t * p = new uint16_t[m_nbChars];
				const char * pSrc = (const char *) m_pChar;
				TextTool::readString( pSrc, ((CharSeq8*)this)->m_codepage, p, m_nbChars );
				basket.ptr = p;
				basket.length = m_nbChars;
				basket.bIsOwner = true;
				return basket;
			}

			default:					// EMPTY
				basket.ptr = 0;
				basket.length = 0;
				basket.bIsOwner = false;
				return basket;
		}
	}

	//____ getStdWstring() ____________________________________________________________

	std::wstring CharSeq::getStdWstring() const
	{
		std::wstring str;

		switch( m_type )
		{
			case Type::StyledChars:
			{
				uint16_t * p = new uint16_t[m_nbChars];
				for( int i = 0 ; i < m_nbChars ; i++ )
					p[i] = ((Char*)m_pChar)[i].code();
				str.assign( (const wchar_t *)p, m_nbChars );
				delete [] p;
				return str;
			}
			case Type::Utf8:
			{
				uint16_t * p = new uint16_t[m_nbChars];
				const char * pSrc = (const char *) m_pChar;
				TextTool::readString( pSrc, p, m_nbChars );
				str.assign( (const wchar_t *)p, m_nbChars );
				delete [] p;
				return str;
			}
			case Type::Utf16:
				str.assign( (const wchar_t *) m_pChar, m_nbChars );
				return str;

			case Type::Codepage8:
			{
				uint16_t * p = new uint16_t[m_nbChars];
				const char * pSrc = (const char *) m_pChar;
				TextTool::readString( pSrc, ((CharSeq8*)this)->m_codepage, p, m_nbChars );
				str.assign( (const wchar_t *)p, m_nbChars );
				delete [] p;
				return str;
			}

			default:					// EMPTY
				return str;
		}
	}

	//____ getUtf8() _______________________________________________________________

	const CharSeq::UTF8Basket CharSeq::getUtf8() const
	{
		UTF8Basket	basket;

		switch( m_type )
		{
			case Type::StyledChars:
			{
				int size = TextTool::getTextSizeUTF8( (Char*) m_pChar, m_nbChars );
				char * p = new char[size+1];
				TextTool::getTextUTF8( (Char*) m_pChar, p, size+1 );
				basket.ptr = p;
				basket.length = size;
				basket.bIsOwner = true;
				return basket;
			}
			case Type::Utf8:
			{
				basket.ptr = (const char *) m_pChar;
				const char * p = (const char *) m_pChar;
				TextTool::forwardCharacters( p, m_nbChars );
				basket.length = int( p - (const char *) m_pChar );
				basket.bIsOwner = false;
				return basket;
			}
			case Type::Utf16:
			{
				int size = TextTool::getTextSizeUTF8( (uint16_t*) m_pChar, m_nbChars );
				char * p = new char[size+1];
				TextTool::getTextUTF8( (uint16_t*) m_pChar, p, size+1 );
				basket.ptr = p;
				basket.length = size;
				basket.bIsOwner = true;
				return basket;
			}

			case Type::Codepage8:
			{
				int size = TextTool::getTextSizeUTF8( (char*) m_pChar, ((CharSeq8*)this)->m_codepage, m_nbChars );
				char * p = new char[size+1];
				TextTool::getTextUTF8( (char*) m_pChar, ((CharSeq8*)this)->m_codepage, p, size+1 );
				basket.ptr = p;
				basket.length = size;
				basket.bIsOwner = true;
				return basket;
			}

			default:					// EMPTY

				basket.ptr = 0;
				basket.length = 0;
				basket.bIsOwner = false;
				return basket;
		}
	}

	//____ getStdString() _________________________________________________________

	std::string CharSeq::getStdString() const
	{
	//	char	temp[512];

		switch( m_type )
		{
			case Type::StyledChars:
			{
				int size = TextTool::getTextSizeUTF8( (Char*) m_pChar, m_nbChars );
				char * p = new char[size+1];
				TextTool::getTextUTF8( (Char*) m_pChar, p, size+1 );
				std::string str( p, size );
				delete [] p;
				return str;
			}
			case Type::Utf8:
			{
				const char * p = (const char *) m_pChar;
				TextTool::forwardCharacters( p, m_nbChars );
				int size = int( p - (const char *) m_pChar );
				return std::string( p, size );
			}
			case Type::Utf16:
			{
				int size = TextTool::getTextSizeUTF8( (uint16_t*) m_pChar, m_nbChars );
				char * p = new char[size+1];
				TextTool::getTextUTF8( (uint16_t*) m_pChar, p, size+1 );
				std::string str( p, size );
				delete [] p;
				return str;
			}
			case Type::Codepage8:
			{
				int size = TextTool::getTextSizeUTF8( (char*) m_pChar, ((CharSeq8*)this)->m_codepage, m_nbChars );
				char * p = new char[size+1];
				TextTool::getTextUTF8( (char*) m_pChar, ((CharSeq8*)this)->m_codepage, p, size+1 );
				std::string str( p, size );
				delete [] p;
				return str;
			}

			default:					// EMPTY
				return "";
		}
	}



	//____ CharBasket::Destructor __________________________________________________

	CharSeq::CharBasket::~CharBasket()
	{
		if( bIsOwner )
		{
			TextTool::derefStyle( (Char*) ptr, length );
			delete [] (char*) ptr;
		}
	}

	//____ CharSeqLiteral::Constructors _________________________________________

	CharSeqLiteral::CharSeqLiteral( const char * pChar)
	{
		m_type		= Type::Utf8;
		m_pChar		= (void *) pChar;
		m_nbChars	= TextTool::countChars(pChar);
	}

	CharSeqLiteral::CharSeqLiteral( const char * pChar, int len )
	{
		m_type		= Type::Utf8;
		m_pChar		= (void *) pChar;
		m_nbChars	= len;
	}

	CharSeqLiteral::CharSeqLiteral( const uint16_t * pChar )
	{
		m_type		= Type::Utf16;
		m_pChar		= (void *) pChar;
		m_nbChars	= TextTool::strlen(pChar);
	}

	CharSeqLiteral::CharSeqLiteral( const uint16_t * pChar, int len )
	{
		m_type		= Type::Utf16;
		m_pChar		= (void *) pChar;
		m_nbChars	= len;
	}

	CharSeqLiteral::CharSeqLiteral( const std::string& str )
	{
		m_type      = Type::Utf8;
		m_pChar     = (void *) str.c_str();
		m_nbChars   = (int) str.length();
	}

	CharSeqLiteral::CharSeqLiteral( const std::string& str, int ofs, int len )
	{
		const char * p = str.c_str();
		int strlen = (int) str.length();

		if( ofs + len > strlen )
		{
			if( ofs > strlen )
				ofs = strlen;
			len = strlen - ofs;
		}

		m_type      = Type::Utf8;
		m_pChar     = p + ofs;
		m_nbChars   = TextTool::countChars(p+ofs, len);
	}


	CharSeqLiteral::CharSeqLiteral( const std::wstring& str )
	{
		m_type      = Type::Utf16;
		m_pChar     = (void *) str.c_str();
		m_nbChars   = (int) str.length();
	}

	CharSeqLiteral::CharSeqLiteral( const std::wstring& str, int ofs, int len )
	{
		const wchar_t * p = str.c_str();
		int strlen = (int) str.length();

		if( ofs + len > strlen )
		{
			if( ofs > strlen )
				ofs = strlen;
			len = strlen - ofs;
		}

		m_type      = Type::Utf16;
		m_pChar     = p + ofs;
		m_nbChars   = len;
	}


	//____ CharSeq8::Constructors _________________________________________


	CharSeq8::CharSeq8( const char * pChar, CodePage codePage )
	{
		m_type 		= Type::Codepage8;
		m_pChar		= (void *) pChar;
		m_nbChars	= (int) strlen(pChar);
		m_codepage	= codePage;
	}


	CharSeq8::CharSeq8( const char * pChar, int len, CodePage codePage )
	{
		m_type 		= Type::Codepage8;
		m_pChar		= (void *) pChar;
		m_nbChars	= len;
		m_codepage	= codePage;
	}

	CharSeq8::CharSeq8( const std::string& str, CodePage codePage )
	{
		m_type 		= Type::Codepage8;
		m_pChar		= str.c_str();
		m_nbChars	= (int) str.length();
		m_codepage	= codePage;
	}

	CharSeq8::CharSeq8( const std::string& str, int ofs, int len, CodePage codePage )
	{
		const char * p = str.c_str();
		int strlen = (int) str.length();

		if( ofs + len > strlen )
		{
			if( ofs > strlen )
				ofs = strlen;
			len = strlen - ofs;
		}

		m_type 		= Type::Codepage8;
		m_pChar     = p + ofs;
		m_nbChars   = len;
		m_codepage	= codePage;
	}



} // namespace wg
