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

#ifndef	WG_DYNAMICBUFFER_DOT_H
#define	WG_DYNAMICBUFFER_DOT_H
#pragma once

namespace wg
{

class DynamicBuffer
{
public:

	class iterator
	{
	public:
		iterator() = default;
		iterator( uint8_t * _p ) : p(_p) {};

		template<class T>
		void push( const T& source ) {
			* (T*) p = source;
			p += sizeof(T);
		};

	//		operator +=
	//		operator -=


		template<typename T>
		T& as(void) {
			return * (T*) p;
		}

	private:
		uint8_t * p = nullptr;
	};


	DynamicBuffer() = default;
	DynamicBuffer( size_t capacity ) 
	{
		m_pBuffer = capacity > 0 ? new uint8_t[capacity] : nullptr;
	 	m_capacity = capacity;
	 	m_size = 0;
	 };


	iterator reserveSpace( int bytes )
	{
		if( m_size + bytes > m_capacity )
			_reallocMin(m_size + bytes);

		uint8_t * p = m_pBuffer + m_size;
		m_size += bytes;
		return p;
	}

	iterator at( size_t ofs )
	{
		return iterator( m_pBuffer + ofs );
	}

	template<typename T>
	void * push( const T& source )
	{
		if( m_size + sizeof(T) > m_capacity )
			_reallocMin(m_size + sizeof(T) );

		* (T*) &m_pBuffer[m_size] = source;
		m_size += sizeof(T);
	}


//	void * pushRange( ANYTHING );


	iterator 	begin() { return iterator( m_pBuffer ); }
	iterator 	end() { return iterator( m_pBuffer + m_size); }

	size_t size()
	{
		return m_size;
	}

	void clear()
	{
		delete [] m_pBuffer;
		m_pBuffer = nullptr;
		m_size = 0;
		m_capacity = 0;
	}

	size_t capacity()
	{
		return m_capacity;
	}

	void setCapacity( size_t bytes)
	{
		if( m_capacity == bytes )
			return;

		if( bytes == 0 )
			clear();
		else
			_reallocExactly(bytes);
	}


//	operator << 


private:

	void _reallocMin( size_t bytes )
	{
		size_t newCapacity = m_capacity < 4096 ? 4096 : m_capacity + m_capacity;

		while( newCapacity < bytes )
			newCapacity += newCapacity;

		_reallocExactly( newCapacity );
	}

	void _reallocExactly( size_t bytes )
	{
		if( m_pBuffer == nullptr )
		{
			m_pBuffer = new uint8_t[bytes];
			m_capacity = bytes;
			return;
		}

		uint8_t * pNewBuffer = new uint8_t[bytes];
		size_t newSize = std::min( m_size, bytes );

		memcpy( pNewBuffer, m_pBuffer, newSize );

		delete [] m_pBuffer;
		m_pBuffer = pNewBuffer;
		m_size = newSize;
		m_capacity = bytes;
	}


	uint8_t * 	m_pBuffer = nullptr;
	size_t		m_size = 0;
	size_t		m_capacity = 0;

};
	
/*
	int x = * (int*) &buffer[ofs]


	it.as<Header>() = 0;

	auto& x = it.as<Header>();

	auto& header = buffer.at(1024).as<Header>();

	auto& header = (Header&) * &buffer[1024];
*/

} // namespace wg
#endif //WG_BASE_DOT_H
