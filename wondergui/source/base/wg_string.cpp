#include <memory.h>

#include <wg_string.h>
#include <wg_charseq.h>
#include <wg_texttool.h>

namespace wg
{


	//____ constructor _____________________________________________________________

	String::String()
	{
	}

	String::String( const char * pStr )
	{
		m_buffer.pushBack(pStr);
	}

	String::String( const String& str )
	{
		m_buffer = str.m_buffer;
	}

	String::String( const CharBuffer* pBuffer )
	{
		if( pBuffer )
		{
			m_buffer = * pBuffer;
			m_buffer.trim();
		}
	}

	String::String( const CharSeq& seq )
	{
		m_buffer.pushBack( seq );
	}


	String::~String()
	{
	}

	const Char* String::chars() const
	{
		return m_buffer.chars();
	}

	int String::length() const
	{
		return (int) m_buffer.nbChars();
	}

	bool String::isEmpty() const
	{
		return m_buffer.nbChars()==0?true:false;
	}

	//____ set() ___________________________________________________________________

	void String::set( const String& str )
	{
		m_buffer = str.m_buffer;
	}

	void String::set( const CharBuffer* pBuffer )
	{
		if( pBuffer )
		{
			m_buffer = * pBuffer;
			m_buffer.trim();
		}
		else
			m_buffer.reset();
	}

	void String::set( const CharSeq& seq )
	{
		m_buffer.reset(seq.length());
		m_buffer.pushBack(seq);
	}

	//____ append() ________________________________________________________________

	void String::append( const String& str )
	{
		m_buffer.pushBack(str.chars(), str.length());
	}

	void String::append( const CharBuffer* pBuffer )
	{
		if( pBuffer )
			m_buffer.pushBack( pBuffer->chars(), pBuffer->nbChars() );
	}

	void String::append( const CharSeq& seq )
	{
		m_buffer.pushBack(seq);
	}

	//____ prepend() _______________________________________________________________

	void String::prepend( const String& str )
	{
		m_buffer.pushFront(str.chars(), str.length());
	}

	void String::prepend( const CharBuffer* pBuffer )
	{
		if( pBuffer )
			m_buffer.pushFront( pBuffer->chars(), pBuffer->nbChars() );
	}

	void String::prepend( const CharSeq& seq )
	{
		m_buffer.pushFront(seq);
	}

	//____ clear() _________________________________________________________________

	void String::clear()
	{
		m_buffer.reset();
	}

} // namespace wg
