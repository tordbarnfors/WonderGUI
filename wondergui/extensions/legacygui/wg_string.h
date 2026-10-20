#ifndef	WG_STRING_DOT_H
#define WG_STRING_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_charbuffer.h>
#include <wg_charseq.h>

namespace wg
{


	class Char;

	//____ String _____________________________________________________________
	/**
	 * @brief Decorated string.
	 *
	 * String is a simple string-class for decorated strings. Each character
	 * in the string can have its own style.
	 *
	 **/


	class String
	{
	public:
		friend class CharBuffer;

		//.____ Creation ______________________________________________________

		String();
		String( const char * str );
		String( const String& str );
		String( const CharBuffer* pBuffer );
		String( const CharSeq& seq );
		~String();

		//.____ Content _______________________________________________________

		const Char*	chars() const;
		int				length() const;
		bool			isEmpty() const;

		//.____ Control _______________________________________________________

		void			clear();

		void			set( const char * str ) { set(CharSeq(str)); }
		void			set( const String& str );
		void			set( const CharBuffer* pBuffer );
		void			set( const CharSeq& seq );

		void			append( const char * str ) { append(CharSeq(str)); }
		void			append( const String& str );
		void			append( const CharBuffer* pBuffer );
		void			append( const CharSeq& seq );

		void			prepend( const char * str ) { prepend(CharSeq(str)); }
		void			prepend( const String& str );
		void			prepend( const CharBuffer* pBuffer );
		void			prepend( const CharSeq& seq );

		//.____ Operators _____________________________________________________

		inline String& operator = ( const char * str )
		{
			set( str );
			return *this;
		}


		inline String& operator = ( const String& kSource )
		{
			set( kSource );
			return *this;
		}
		inline String& operator = ( const CharBuffer* pBuffer )
		{
			set( pBuffer );
			return *this;
		}
		inline String& operator = ( const CharSeq& seq )
		{
			set( seq );
			return *this;
		}

		inline String& operator += ( const String& kSource )
		{
			append( kSource );
			return *this;
		}
		inline String& operator += ( const CharBuffer* pBuffer )
		{
			append( pBuffer );
			return *this;
		}
		inline String& operator += ( const CharSeq& seq )
		{
			append( seq );
			return *this;
		}

		inline String operator + ( const String& kSource ) const
		{
			String str(*this);
			str.append( kSource );
			return str;
		}
		inline String operator + ( const CharBuffer* pBuffer ) const
		{
			String str(*this);
			str.append( pBuffer );
			return str;
		}
		inline String operator + ( const CharSeq& seq ) const
		{
			String str(*this);
			str.append( seq );
			return str;
		}


		bool operator == ( const String& kOther ) const	{ return (m_buffer == kOther.m_buffer); }
        bool operator == ( const char * pCString ) const    { return (m_buffer == pCString); }
        bool operator == ( const std::string& str ) const    { return (m_buffer == str); }

        bool operator != ( const String& kOther ) const    { return (m_buffer != kOther.m_buffer); }
        bool operator != ( const char * pCString ) const    { return (m_buffer != pCString); }
        bool operator != ( const std::string& str ) const    { return (m_buffer != str); }


	private:

		CharBuffer	m_buffer;
	};


} // namespace wg
#endif //WG_STRING_DOT_H
