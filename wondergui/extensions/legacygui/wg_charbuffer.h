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
#ifndef WG_CHARBUFFER_DOT_H
#define WG_CHARBUFFER_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_char.h>

namespace wg
{

	class CharSeq;

	//____ CharBuffer ____________________________________________________________

	/**
		CharBuffer is a buffer class providing convenient and efficient ways for
		building and manipulating Char-based texts to be displayed. It's essentially
		a buffer for Chars allocated to a specified capacity in which a string can
		grow and shrink dynamically. The content is always NULL-terminated.

		If a CharBuffer exhausts its capacity it will automatically be reallocated
		to the smallest size possible for holding the new content, so there is no risk
		for overflow. However, you should for performance reasons not rely too much
		on this behaviour but allocate a big enough buffer from start and then grow
		it by a suitable amount using setCapacity() if it starts to run low.
		A CharBuffer is never automatically shrunk. If you want to shrink a buffer
		you will have to do it manually by calling trim() or setCapacity().

		CharBuffers are reference counted copy-on-change objects which can
		share the buffer itself with other CharBuffer and  String objects.
		When copying a CharBuffer to a String you should keep in mind that String
		always trims its buffer if it has some unused capacity. You should therefore
		trim the CharBuffer first if you copy its content to multiple strings without
		any change inbetween.

		Example:

		@code
		CharBuffer	buffer(256);
		String		str[100];

		buffer.pushBack( "Hello World!" );
		buffer.trim();
		for( int i = 0 ; i < 100 ; i++ )
			str[i] = buffer;
		@endcode

		The above is fast and efficient, the buffer and all strings points to the same
		memory buffer. No memory allocation or copying of data is done.
		However, if you would remove the trim() call then you would end up with 100 identical
		memory buffers, all individually allocated and with data copied from the CharBuffer.

		On the other hand, you shouldn't truncate your buffer unnecessarily since it means
		allocation and dealocation of memory (unless it was already trimmed).

		Example:

		@code
		CharBuffer	buffer(256);
		String		str[3];

		buffer.pushBack( "1... " );
		buffer.trim();
		str[0] = buffer;
		buffer.pushBack( "2... " );
		buffer.trim();
		str[1] = buffer;
		buffer.pushBack( "3!" );
		buffer.trim();
		str[2] = buffer;
		@endcode

		We here get three different strings containing the text "1... ", "1... 2... " and "1... 2... 3!"
		respectively. Trimming the buffer as done in the example is NOT a good idea. It doesn't save us
		anything since each string needs an individual buffer allocated anyway. Even worse, we actually
		add overhead by trimming down the buffer shortly before we need to grow it again to hold more
		content, thus we introduce one unnecessary memory allocation and copying of content for each step.


	*/


	class CharBuffer
	{
	public:

		//.____ Creation ___________________________________________________

		CharBuffer( int size = 0 );
		inline CharBuffer(const CharBuffer& r);
		inline CharBuffer(CharBuffer&& r) noexcept;

		~CharBuffer() { _derefBuffer(); }

		//.____ Content ___________________________________________________________

		void	trim();
		void	trimWhiteSpace();
		void	reset( int size = 0 );

		inline void	setCapacity( int capacity );
		inline void	setUnusedCapacity( int front, int back );
		Char*	beginWrite();
		void	endWrite();

		int		pushFront( int nChars );
		int		pushFront( const Char& character );
		int		pushFront( const Char * pChars );
		int		pushFront( const Char * pChars, int nChars );
		int		pushFront( const CharSeq& seq );

		int		popFront( int nChars );

		int		pushBack( int nChars );
		int		pushBack( const Char& character );
		int		pushBack( const Char * pChars );
		int		pushBack( const Char * pChars, int nChars );
		int		pushBack( const CharSeq& seq );

		int		popBack( int nChars );

		int		insert( int ofs, int nChars );
		int		insert( int ofs, const Char& character );
		int		insert( int ofs, const Char * pChars );
		int		insert( int ofs, const Char * pChars, int nChars );
		int		insert( int ofs, const CharSeq& seq );

		int		remove( int ofs, int nChars = 1 );

		int		replace( int ofs, const Char& character );
		int		replace( int ofs, int nDelete, int nInsert );
		int		replace( int ofs, int nDelete, const Char * pChars );
		int		replace( int ofs, int nDelete, const Char * pChars, int nInsert );
		int		replace( int ofs, int nDelete, const CharSeq& seq );

		inline const Char * chars() const;

		inline int			nbChars() const;
		inline int			length() const;

		inline int			capacity() const;

		inline int			unusedFrontCapacity() const;
		inline int			unusedBackCapacity() const;
		inline int			unusedCapacity() const;

		inline void			fill( const Char& ch );
		void				fill( const Char& ch, int ofs, int len );


		inline void			setCharCodes( uint16_t charCode );
		void				setCharCodes( uint16_t charCode, int ofs, int len );

		inline void			setStyle( TextStyle * pProp );
		void				setStyle( TextStyle * pProp, int ofs, int len);


		inline void			clear();

		inline void 		clearStyle();
		void				clearStyle( int ofs, int len );

		//.____ Misc ___________________________________________________________

		int					findFirst( const CharSeq& seq, int ofs = 0 ) const;
		int					findFirst( uint16_t character, int ofs = 0 ) const;


		int					compareTo( const CharBuffer * pBuffer ) const;
        int                 compareTo( const char * pCString ) const;
        int                 compareTo( const std::string& str ) const;

        int					compareCharCodesTo( const CharBuffer * pBuffer ) const;
        int                 compareCharCodesTo( const char * pCString ) const;
        int                 compareCharCodesTo( const std::string& str ) const;

		int					compareCharCodesIgnoreCaseTo( const CharBuffer * pBuffer ) const;
        int                 compareCharCodesIgnoreCaseTo( const char * pCString ) const;
        int                 compareCharCodesIgnoreCaseTo( const std::string& str ) const;

        
		// Methods mostly for debugging and profiling purposes.

		inline int			refCount() const;
		inline static int	nbBuffers();

		//.____ Operators ______________________________________________________

		CharBuffer& operator=(CharBuffer const & r);
		CharBuffer& operator=(String const & r);
		CharBuffer& operator=(CharSeq const & r);

		inline bool operator==(const CharBuffer& other) const { return _compareBuffers(this->m_pHead, other.m_pHead); }
        inline bool operator==(const char * pCString ) const { return (compareTo(pCString) == 0); }
        inline bool operator==(const std::string& str ) const { return (compareTo(str) == 0); }

        inline bool operator!=(const CharBuffer& other) const { return !_compareBuffers(this->m_pHead, other.m_pHead); }
        inline bool operator!=(const char * pCString ) const { return (compareTo(pCString) != 0); }
        inline bool operator!=(const std::string& str ) const { return (compareTo(str) != 0); }

        
		// These operator[] are slow, please use chars() or beginWrite() instead.

		Char&				operator[](int i) { if (m_pHead->m_refCnt > 1) _reshapeBuffer(0, 0, m_pHead->m_len, 0); return *(Char*)_ptr(i); }
		const Char&		operator[](int i) const { return *(const Char*)_ptr(i); }

		inline operator bool() const { return m_pHead->m_len != 0 ? true : false; }

	private:

		struct BufferHead
		{
			int			m_lockCnt;
			int			m_refCnt;
			int			m_size;					// SizeI in number of Char of buffer.
			int			m_beg;					// Beginning of chars.
			int			m_len;					// Number of chars.
		};


		void			_clearCharsNoDeref( int ofs, int n );  ///< Clears specified characters in buffer without dereferencing style.
		inline void *	_ptr( int ofs ) const { return ((char*) &m_pHead[1]) + sizeof(Char)*(m_pHead->m_beg+ofs); }

		void			_pushFront( int nChars );
		void			_pushBack( int nChars );
		int				_replace( int ofs, int delChar, int addChar, const Char * pChars = 0);



		inline void		_derefBuffer()
		{
			m_pHead->m_refCnt--;
			if( m_pHead->m_refCnt == 0 )
			{
				_derefStyle(0, m_pHead->m_len);
				_destroyBuffer(m_pHead);
			}
		}

		BufferHead *	_createBuffer( int size );
		inline void 	_destroyBuffer( BufferHead * pBuffer ) { delete [] (char*) pBuffer; g_nBuffers--; }

		void			_copyChars( BufferHead * pDst, int ofsDst, const BufferHead * pSrc, int ofsSrc, int nChars );
		void			_copyChars( BufferHead * pDst, int ofsDst, const Char * pChars, int nChars );
		void			_reshapeBuffer( int begMargin, int copyOfs, int copyLen, int endMargin );

		void			_setChars( int ofs, int nChars, uint32_t value );

		inline void		_derefStyle( int ofs, int n ) { TextTool::derefStyle( (Char*) _ptr(ofs), n ); }
		inline void		_refStyle( int ofs, int n ) { TextTool::refStyle( (Char*) _ptr(ofs), n ); }

		static bool		_compareBuffers( const BufferHead * p1, const BufferHead * p2 );


		const static uint32_t	c_emptyChar = 0x00000020;	// Value to fill out empty Chars with.
		static int				g_nBuffers;					// Number of real buffers, <= number of CharBuffer.
		static	BufferHead *	g_pEmptyBuffer;				// We keep one common empty buffer as an optimization



		BufferHead *	m_pHead;
	};



	//____ constructor ____________________________________________________________
	//
	/// @brief Creates a copy of the specified buffer.

	CharBuffer::CharBuffer(const CharBuffer& r)
	{
		m_pHead = r.m_pHead;
		m_pHead->m_refCnt++;

		// See if we need to copy the buffer because of write-lock

		if( m_pHead->m_lockCnt != 0 )
			_reshapeBuffer(0,0,m_pHead->m_len,0);
	};

	CharBuffer::CharBuffer(CharBuffer&& r) noexcept
	{
		m_pHead = r.m_pHead;
		r.m_pHead = g_pEmptyBuffer;
		g_pEmptyBuffer->m_refCnt++;		// We always have exactly one more instance pointing to g_pEmptyBuffer now.
	}



	//____ setCapacity() __________________________________________________________
	//
	/// @brief Reallocates the buffer to the specified size.
	/// If content doesn't fit in the new size, the content is cut at the end. Any unused capacity
	/// will be at the end of the buffer.
	///
	///	@param capacity	The new size of the buffer.
	///

	void CharBuffer::setCapacity( int capacity )
	{
		if( capacity < 0 )
			capacity = 0;

		if( m_pHead->m_len > capacity )
			_reshapeBuffer( 0, 0, capacity, 0 );
		else
			_reshapeBuffer( 0, 0, m_pHead->m_len, capacity-m_pHead->m_len );
	}


	//____ setUnusedCapacity() __________________________________________________________
	//
	/// @brief Reallocates the buffer to hold its content plus the specified extra capacity at the front and back.
	/// setUnusedCapacity(0,0) is identical to trim().
	///
	///	@param front	The amount of space to leave in the front of the buffer to allow for future growth without
	///					having to move the content or reallocate the buffer.
	///
	///	@param back		The amount of space to leave in the back of the buffer to allow for future growth without
	///					having to move the content or reallocate the buffer.
	///

	void CharBuffer::setUnusedCapacity( int front, int back )
	{
		_reshapeBuffer( front, 0, m_pHead->m_len, back );
	}

	//____ _Chars() _______________________________________________________________

	/// @brief Returns a read-only pointer to the null-terminated content of the buffer.
	///
	/// The pointer is only valid until a non-const buffer method is called.
	///
	/// @return Pointer to the null-terminated content of the buffer. A valid pointer is always returned, never null.

	const Char * CharBuffer::chars() const
	{
		return (const Char*) _ptr(0);
	}

	//____ nbChars() ______________________________________________________________

	/// @brief Returns the number of characters in the buffer.
	///
	/// This method is identical to length()-
	///
	/// @return Number of characters in the buffer (or length of the buffer content if you so will).

	int CharBuffer::nbChars() const
	{
		return m_pHead->m_len;
	}

	//____ length() _______________________________________________________________

	/// @brief Returns the number of characters in the buffer.
	///
	/// This method is identical to getNbChars()-
	///
	/// @return Number of characters in the buffer (or length of the buffer content if you so will).

	int CharBuffer::length() const
	{
		return m_pHead->m_len;
	}

	//____ capacity() _________________________________________________________________

	/// @brief	Returns the size of the buffer itself, not size of content.
	///
	/// @return Number of characters the buffer can hold before it needs to be reallocated.

	int CharBuffer::capacity() const
	{
		return m_pHead->m_size;
	}

	//____ unusedFrontCapacity() ________________________________________________________

	/// @brief 	Returns available space for new characters in the front of the buffer.
	///
	///	The size of the front capacity determines how many characters you can push to the
	/// front before content needs to be reorganized or buffer reallocated.
	///
	/// @return Number of characters that can be pushed to the front without causing extra overhead.

	inline int CharBuffer::unusedFrontCapacity() const
	{
		return m_pHead->m_beg;
	}

	//____ unusedBackCapacity() _________________________________________________________

	/// @brief 	Returns available space for new characters in the back of the buffer.
	///
	///	The size of the back capacity determines how many characters you can push to the
	/// back before content needs to be reorganized or buffer reallocated.
	///
	/// @return Number of characters that can be pushed to the back without causing extra overhead.

	int CharBuffer::unusedBackCapacity() const
	{
		return m_pHead->m_size - (m_pHead->m_beg + m_pHead->m_len);
	}

	//____ unusedCapacity() _____________________________________________________________

	/// @brief 	Returns total available space for new characters in the buffer.
	///
	///	This is identical to unusedFrontCapacity() + unusedBackCapacity() and determines how many characters can
	/// be pushed or inserted before a bigger buffer needs to be allocated.
	///
	/// @return Number of characters that can be added to the buffer without reallocation.

	int CharBuffer::unusedCapacity() const
	{
		return m_pHead->m_size - m_pHead->m_len;
	}

	//____ fill() _________________________________________________________________

	/// @brief	Fills the buffer with specified character.
	///
	/// @param	ch		Character to fill buffer with.
	///
	/// Fully replaces the content of the buffer by filling it with the specified
	/// character. Only the used section of the buffer is filled, unused capacity
	/// is not affected.

	void CharBuffer::fill( const Char& ch )
	{
		fill( ch, 0, INT_MAX );
	}

	//____ 	setCharCodes() ___________________________________________________________

	/// @brief	Sets the charcode for all characters currently in the buffer.
	///
	/// @param	charCode	The charcode to be used. This may not be 0.
	///
	/// Fully replaces the charcode content of the buffer with the specified charcode.
	/// Only the used section of the buffer is affected, unused capacity is not
	/// filled with the charcode.

	void CharBuffer::setCharCodes( uint16_t charCode )
	{
		setCharCodes( charCode, 0, INT_MAX );
	}

	//____ setStyle() __________________________________________________________

	/// @brief	Sets the style for all characters currently in the buffer.
	///
	/// @param	pStyle	Pointer to the new style for all the characters.

	void CharBuffer::setStyle( TextStyle * pStyle )
	{
		setStyle( pStyle, 0, INT_MAX );
	}

	//____ clear() ________________________________________________________________

	/// @brief	Clears the content of the buffer.
	///
	/// Clears the content of the buffer without resizing it.

	void CharBuffer::clear()
	{
		remove( 0, INT_MAX );
	}

	//____ clearStyle() ________________________________________________________
	//
	/// @brief	Clears the style for all characters currently in the buffer.

	void CharBuffer::clearStyle()
	{
		clearStyle( 0, INT_MAX );
	}


	//____ refCount() _____________________________________________________________
	//
	/// @brief Returns number of objects referencing the same memory buffer.
	///
	/// This method only provided for profiling and debugging purposes.
	///
	/// The same buffer of characters can be used by multiple CharBuffer, String and Text
	/// objects currently having the same content. This method provides you with the total number of
	/// objects sharing the same buffer as this CharBuffer, which is included in the count.
	///
	/// @return Number of objects referencing the same buffer. Always one or higher.

	int CharBuffer::refCount() const
	{
		return m_pHead->m_refCnt;
	}

	//____ nbBuffers() ____________________________________________________________
	//
	/// @brief Returns total number of memory buffers currently in use.
	///
	/// This method only provided for profiling and debugging purposes.
	///
	/// @return Number of memory buffers currently in use, which is @<=
	/// total number of CharBuffer, String and Text objects combined.

	int CharBuffer::nbBuffers()
	{
		return g_nBuffers;
	}



} // namespace wg
#endif //WG_CHARBUFFER_DOT_H
