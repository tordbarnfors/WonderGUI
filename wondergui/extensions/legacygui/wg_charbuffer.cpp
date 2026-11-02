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

#include <wg_charbuffer.h>
#include <wg_string.h>

namespace wg
{

	int CharBuffer::g_nBuffers = 0;

	CharBuffer::BufferHead * CharBuffer::g_pEmptyBuffer = 0;


	//____ constructor _____________________________________________________________
	//
	/// @brief Creates a new buffer of the specified initial capacity.

	CharBuffer::CharBuffer( int size )
	{
		if( size < 0 )
			size = 0;

		m_pHead = _createBuffer( size );
		m_pHead->m_refCnt++;
	}

	//____ operator=() _____________________________________________________________

	CharBuffer& CharBuffer::operator=( CharBuffer const & r)
	{
		if( m_pHead != r.m_pHead )
		{
			_derefBuffer();

			m_pHead = r.m_pHead;
			m_pHead->m_refCnt++;

			// See if we need to copy the buffer because of write-lock

			if( m_pHead->m_lockCnt != 0 )
				_reshapeBuffer(0,0,m_pHead->m_len,0);
		}
		return *this;
	}

	CharBuffer& CharBuffer::operator=( String const & r)
	{
		if( m_pHead != r.m_buffer.m_pHead )
		{
			_derefBuffer();

			m_pHead = r.m_buffer.m_pHead;
			m_pHead->m_refCnt++;

			// See if we need to copy the buffer because of write-lock

			if( m_pHead->m_lockCnt != 0 )
				_reshapeBuffer(0,0,m_pHead->m_len,0);
		}
		return *this;
	}

	CharBuffer& CharBuffer::operator=( CharSeq const & seq)
	{
		reset(seq.length());
		pushBack(seq);

		return *this;
	}




	//____ _clearCharsNoDeref() _____________________________________________________

	void CharBuffer::_clearCharsNoDeref( int ofs, int n )
	{
		uint32_t * p = (uint32_t*) _ptr(ofs);

		for( int i = 0 ; i < (int) n ; i++ )
			* p++ = c_emptyChar;
	}


	//____ trim() __________________________________________________________________
	//
	/// @brief Trims down the buffer to the size of the content, thus removing any extra capacity.

	void CharBuffer::trim()
	{
		// See if we don't have to do anything...

		if( m_pHead->m_size == m_pHead->m_len )
			return;

		// We need to trim the buffer

		_reshapeBuffer(0,0,m_pHead->m_len,0);
	}

	//____ trimWhiteSpace() _______________________________________________________
	//
	/// @brief Removes white spaces from the beginning and end of the string

	void CharBuffer::trimWhiteSpace()
	{
	}

	//____ reset() _________________________________________________________________
	//
	/// @brief Deletes all content and reallocates the buffer to the specified capacity.

	void CharBuffer::reset( int size )
	{
		if( size < 0 )
			size = 0;

		if( m_pHead->m_size == size && m_pHead->m_refCnt == 1 )
		{
			_derefStyle( 0, m_pHead->m_len );
			m_pHead->m_beg = 0;
			m_pHead->m_len = 0;

			*((uint32_t*) _ptr(0))= 0;	// NULL-terminate content.
		}
		else
		{
			_derefBuffer();
			m_pHead = _createBuffer( size );
			m_pHead->m_refCnt++;
		}
	}


	//____ _reshapeBuffer() _____________________________________________________________

	/**
		Reshapes the buffer by specifying begin/end margins and what subset of content
		to keep.
	*/

	void CharBuffer::_reshapeBuffer( int begMargin, int copyOfs, int copyLen, int endMargin )
	{

		// First create a buffer of right size and
		// copy the content.

		BufferHead * p = _createBuffer( begMargin + copyLen + endMargin );
		_copyChars( p, begMargin, m_pHead, m_pHead->m_beg + copyOfs, copyLen );
		((uint32_t*)&p[1])[begMargin + copyLen] = 0;	// Terminate the buffer content.


		p->m_beg = begMargin;
		p->m_len = copyLen;

		// Do our own quick deref of the old buffer.

		m_pHead->m_refCnt--;
		if( m_pHead->m_refCnt == 0 )
		{
			// We only need to dereference the props we haven't copied to the new buffer

			if( copyOfs > 0 )
				_derefStyle( 0, copyOfs );

			if( copyOfs + copyLen < m_pHead->m_len )
				_derefStyle( copyOfs + copyLen, m_pHead->m_len - copyOfs - copyLen );
			//

			_destroyBuffer(m_pHead);
		}
		else
		{
			// Since the previous buffer wasn't destroyed, we need to reference the new
			// one since we have one extra copy of all the Char.

			_refStyle( copyOfs, copyLen );		// Easier to scan through the old buffer, content is identical anyway...
		}

		//

		m_pHead = p;
		p->m_refCnt++;
	}



	//____ beginWrite() ____________________________________________________________
	//
	/// @brief Gives access to direct manipulation of the buffer content.
	///
	/// Any attempt to directly read or manipulate the buffer content needs to be enclosed by calls to beginWrite()
	/// and endWrite().
	///
	/// You may NOT call any non-const methods of this buffer until you have called endWrite() and not write
	/// outside the null-terminated content of the buffer even if you know that there is extra capacity.
	/// If you need to grow the content before manipulation you can use the appropriate pushFront(), pushBack()
	/// or insert() methods before calling beginWrite().
	///
	/// @return Pointer to the content of the buffer, null-terminated.

	Char*	CharBuffer::beginWrite()
	{
		if( m_pHead->m_refCnt > 1 )
			_reshapeBuffer(0,0,m_pHead->m_len,0);

		m_pHead->m_lockCnt++;
		return (Char*) _ptr(0);
	}


	//____ endWrite() ______________________________________________________________
	//
	/// @brief Ends your direct access to the buffer content.

	void CharBuffer::endWrite()
	{
		m_pHead->m_lockCnt--;
	}

	//____ popFront() ______________________________________________________________
	//
	/// @brief Shrinks the content by removing characters from the front.
	///
	///	@param 	nChars	Number of characters to remove from the front. May be larger than content
	///					in which case all content is removed.
	///
	/// Removing characters from the front increases the front capacity by the same amount.
	///
	///	@return Number of characters that was removed from the front of the content.

	int CharBuffer::popFront( int nChars )
	{
		if( nChars <= 0 )
			return 0;

		if( nChars > m_pHead->m_len )
			nChars = m_pHead->m_len;

		if( m_pHead->m_refCnt > 1 )
		{
			_reshapeBuffer(0, nChars, m_pHead->m_len - nChars, 0);
		}
		else
		{
			_derefStyle( 0, nChars );

			m_pHead->m_beg += nChars;
			m_pHead->m_len -= nChars;
		}

		return nChars;
	}

	//____ popBack() _______________________________________________________________
	//
	/// @brief Shrinks the content by removing characters from the back.
	///
	///	@param 	nChars	Number of characters to remove from the back. May be larger than content
	///					in which case all content is removed.
	///
	/// Removing characters from the back increases the back capacity by the same amount.
	///
	///	@return Number of characters that was removed from the back of the content.

	int CharBuffer::popBack( int nChars )
	{
		if( nChars <= 0 )
			return 0;

		if( nChars > m_pHead->m_len )
			nChars = m_pHead->m_len;

		if( m_pHead->m_refCnt > 1 )
		{
			_reshapeBuffer(0, 0, m_pHead->m_len - nChars, 0);
		}
		else
		{
			_derefStyle( m_pHead->m_len - nChars, nChars );
			m_pHead->m_len -= nChars;
			* ((uint32_t *) _ptr(m_pHead->m_len)) = 0;		// null terminate.
		}

		return nChars;
	}

	//____ _copyChars() _____________________________________________________________

	/**
		Copies Chars in buffers from source to destination. Areas may overlap.
		Style references are not affected and will need to be de/referenced
		separately where needed.

		Please note that both offsets in this method start right after header
		and not where m_beg points to!

		No error checking, parameters are assumed to have been verified before call.
	*/

	void CharBuffer::_copyChars( BufferHead * pDst, int ofsDst, const BufferHead * pSrc, int ofsSrc, int nChars )
	{
		memmove( ((char*)&pDst[1])+ofsDst*sizeof(Char), ((char*)&pSrc[1])+ofsSrc*sizeof(Char), nChars*sizeof(Char) );
	}

	/**
		Copies Char-array content to buffer. Areas may not overlap.
		Style references are not affected and will need to be de/referenced
		separately where needed.

		Please note that the offset in this method starts right after header
		and not where m_beg points to!

		No error checking, parameters are assumed to have been verified before call.

	*/

	void CharBuffer::_copyChars( BufferHead * pDst, int ofsDst, const Char * pChars, int nChars )
	{
		memcpy( ((char*)&pDst[1])+ofsDst*sizeof(Char), pChars, nChars*sizeof(Char) );
	}

	//____ _setChars() _____________________________________________________________

	void CharBuffer::_setChars( int ofs, int nChars, uint32_t value )
	{
		uint32_t * pChar = ((uint32_t *) &m_pHead[1]) + m_pHead->m_beg + ofs;

		for( int i = 0 ; i < (int) nChars ; i++ )
			pChar[i] = value;
	}

	//____ pushFront() _____________________________________________________________

	/// @brief Grows the content by pushing a character to the front.
	///
	///	@param 	character	Character to push to the front.
	///
	///	@return Number of characters the content was grown with. Always 1.

	int CharBuffer::pushFront( const Char& character )
	{
		_pushFront(1);
		*((uint32_t*)_ptr(0)) = character.m_all;

		if( character.m_style )
			TextStyleManager::_getPointer(character.m_style)->_incRefCount();

		return 1;
	}


	/// @brief Grows the content by pushing whitespace to the front.
	///
	///	@param 	nChars	Number of whitespace characters to push to the front.
	///
	///	@return Number of characters the content was grown with. Always equals nChars.

	int CharBuffer::pushFront( int nChars )
	{
		if( nChars <= 0 )
			return 0;

		_pushFront(nChars);
		_setChars( 0, nChars, c_emptyChar );
		return nChars;
	}


	/// @brief Grows the content by pushing a null-terminated string of characters to the front.
	///
	///	@param 	pChars	Pointer to string of null-terminated characters to push to the front.
	///
	///	@return Number of characters the content was grown with.

	int CharBuffer::pushFront( const Char * pChars )
	{
		int len = TextTool::strlen( pChars );

		_pushFront(len);

		memcpy( _ptr(0), pChars, sizeof(Char)*len );
		_refStyle(0,len);
		return len;
	}


	/// @brief Grows the content by pushing a specified number of characters to the front.
	///
	///	@param 	pChars	Pointer to string of characters to push to the front.
	/// @param	nChars	The number of characters to push.
	///
	/// The string may not end prematurely. If any of the characters pushed contains a null-character the behaviour is undefined.
	///
	///	@return Number of characters the content was grown with. Always equals nChars.

	int CharBuffer::pushFront( const Char * pChars, int nChars )
	{
		if( nChars <= 0 )
			return 0;

		_pushFront(nChars);

		memcpy( _ptr(0), pChars, sizeof(Char)*nChars );
		_refStyle(0,nChars);
		return nChars;
	}


	/// @brief Grows the content by pushing a character sequence to the front.
	///
	///	@param 	seq		Character sequence to be pushed to the front.
	///
	///	@return Number of characters the content was grown with. Always equals length of the sequence.

	int CharBuffer::pushFront( const CharSeq& seq )
	{
		int nChars = seq.length();
		_pushFront(nChars);
		memset(((Char*)_ptr(0)), 0, sizeof(Char) * nChars);
		seq.copyTo((Char*)_ptr(0));
		return nChars;
	}

	//____ pushBack() ______________________________________________________________
	//
	/// @brief Grows the content by pushing a character to the back.
	///
	///	@param 	character	Character to push to the back.
	///
	///	@return Number of characters the content was grown with. Always 1.

	int CharBuffer::pushBack( const Char& character )
	{
		_pushBack(1);
		*((uint32_t*)_ptr( m_pHead->m_len - 1)) = character.m_all;

		if( character.m_style )
			TextStyleManager::_getPointer(character.m_style)->_incRefCount();

		return 1;
	}


	/// @brief Grows the content by pushing whitespace to the back.
	///
	///	@param 	nChars	Number of whitespace characters to push to the back.
	///
	///	@return Number of characters the content was grown with. Always equals nChars.

	int CharBuffer::pushBack( int nChars )
	{
		if( nChars <= 0 )
			return 0;

		_pushBack(nChars);
		_setChars( m_pHead->m_len - nChars, nChars, c_emptyChar );
		return nChars;
	}

	/// @brief Grows the content by pushing a null-terminated string of characters to the back.
	///
	///	@param 	pChars	Pointer to string of null-terminated characters to push to the back.
	///
	///	@return Number of characters the content was grown with.

	int CharBuffer::pushBack( const Char * pChars )
	{
		int len = TextTool::strlen( pChars );

		_pushBack(len);

		memcpy( ((Char*) _ptr(0)) + m_pHead->m_len-len, pChars, sizeof(Char)*len );
		_refStyle(m_pHead->m_len-len,len);
		return len;
	}


	/// @brief Grows the content by pushing a specified number of characters to the back.
	///
	///	@param 	pChars	Pointer to string of characters to push to the back.
	///
	/// @param	nChars	The number of characters to push.
	///
	/// The string may not end prematurely. If any of the characters pushed contains a null-character the behaviour is undefined.
	///
	///	@return Number of characters the content was grown with. Always equals nChars.

	int CharBuffer::pushBack( const Char * pChars, int nChars )
	{
		if( nChars <= 0 )
			return 0;

		_pushBack(nChars);

		memcpy( ((Char*) _ptr(0)) + m_pHead->m_len-nChars, pChars, sizeof(Char)*nChars );
		_refStyle(m_pHead->m_len-nChars,nChars);
		return nChars;
	}


	/// @brief Grows the content by pushing a character sequence to the back.
	///
	///	@param 	seq		Character sequence to be pushed to the back.
	///
	///	@return Number of characters the content was grown with. Always equals length of the sequence.

	int CharBuffer::pushBack( const CharSeq& seq )
	{
		int nChars = seq.length();
		_pushBack(nChars);
		memset( ((Char*) _ptr(0)) + m_pHead->m_len-nChars, 0, sizeof(Char)*nChars );
		seq.copyTo( ((Char*) _ptr(0)) + m_pHead->m_len-nChars );
		return nChars;
	}



	//____ _pushFront() _____________________________________________________

	void CharBuffer::_pushFront( int nChars )
	{
		// Check if we can just modify this buffer to fit things in

		if( m_pHead->m_refCnt == 1 && m_pHead->m_size >= m_pHead->m_len + nChars )
		{
			// Check if we have enough space at front

			if( m_pHead->m_beg >= nChars )
			{
				m_pHead->m_beg -= nChars;
			}
			else
			{
				// Not enough space at front, need to move chars...

				_copyChars( m_pHead, nChars, m_pHead, m_pHead->m_beg, m_pHead->m_len+1 );
				m_pHead->m_beg = 0;
			}
		}
		else
		{
			_reshapeBuffer(nChars, 0, m_pHead->m_len, 0 );
			m_pHead->m_beg = 0;
		}


		m_pHead->m_len += nChars;
	}

	//____ _pushBack() ______________________________________________________

	void CharBuffer::_pushBack( int nChars )
	{
		// Check if we can just modify this buffer to fit things in

		if( m_pHead->m_refCnt == 1 && m_pHead->m_size >= m_pHead->m_len + nChars )
		{
			// Check if we have enough space at end

			if( m_pHead->m_size - (m_pHead->m_beg + m_pHead->m_len) >= nChars )
			{
			}
			else
			{
				// Not enough space at end, need to move chars.
				// Move it right to the beginning of the buffer even if not
				// needed since it seems more likely we will have use of
				// any extra space at the end of the buffer now...

				_copyChars( m_pHead, 0, m_pHead, m_pHead->m_beg, m_pHead->m_len+1 );
				m_pHead->m_beg = 0;
			}
		}
		else
		{
			_reshapeBuffer( 0, 0, m_pHead->m_len, nChars );
			m_pHead->m_beg = 0;
		}

		m_pHead->m_len += nChars;
		* ((uint32_t*) _ptr(m_pHead->m_len)) = 0;					// Null terminate the string.
	}

	//____ insert() ________________________________________________________________

	/// @brief Inserts a character.
	///
	/// @param ofs		Offset to insert character at, counting from beginning of buffer content.
	///
	/// @param pChars	Pointer at null-terminated string of characters to insert.
	///
	/// If ofs is larger than number of characters in buffer the character is pushed on the back.
	///
	/// @return Number of characters that have been insert.

	int CharBuffer::insert( int ofs, const Char& character )
	{
		if( ofs < 0 )
			ofs = 0;

		_replace( ofs, 0, 1, &character );
		return 1;
	}


	/// @brief Inserts whitespace characters.
	///
	/// @param ofs		Offset to insert characters at, counting from beginning of buffer content.
	///
	/// @param nChars	Number of whitespace characters to insert.
	///
	/// If ofs is larger than number of characters in buffer the whitespaces are pushed on the back.
	///
	/// @return Number of characters that have been insert. Always equals nChars.

	int CharBuffer::insert( int ofs, int nChars )
	{
		if( ofs < 0 )
			ofs = 0;

		if( nChars <= 0 )
			return 0;

		_replace( ofs, 0, nChars );
		return nChars;
	}


	/// @brief Inserts null-terminated string of characters.
	///
	/// @param ofs		Offset to insert characters at, counting from beginning of buffer content.
	///
	/// @param pChars	Pointer at null-terminated string of characters to insert.
	///
	/// If ofs is larger than number of characters in buffer the content of pChars is pushed on the back.
	///
	/// @return Number of characters that have been insert. Always equals 1.

	int CharBuffer::insert( int ofs, const Char * pChars )
	{
		if( ofs < 0 )
			ofs = 0;

		int nInsert = TextTool::strlen( pChars );
		_replace( ofs, 0, nInsert, pChars );
		return nInsert;
	}


	/// @brief Inserts a specified number of characters.
	///
	/// @param ofs		Offset to insert characters at, counting from beginning of buffer content.
	///
	/// @param pChars	Pointer at string of characters to insert.
	///
	/// @param nChars	Number of characters to insert.
	///
	/// The string may not end prematurely. If any of the characters pushed contains a null-character the behaviour is undefined.
	/// If ofs is larger than number of characters in buffer the content of pChars is pushed on the back.
	///
	/// @return Number of characters that have been insert. Always equals nChars.

	int CharBuffer::insert( int ofs, const Char * pChars, int nChars )
	{
		if( ofs < 0 )
			ofs = 0;

		if( nChars <= 0 )
			return 0;

		_replace( ofs, 0, nChars, pChars );
		return nChars;
	}

	/// @brief Inserts characters from a character sequence.
	///
	/// @param ofs		Offset to insert characters at, counting from beginning of buffer content.
	///
	/// @param seq		Character sequence to be inserted.
	///
	/// If ofs is larger than number of characters in buffer the content of pChars is pushed on the back.
	///
	/// @return Number of characters that have been insert. Always equals length of sequence.

	int CharBuffer::insert( int ofs, const CharSeq& seq )
	{
		if( ofs < 0 )
			ofs = 0;

		if( ofs > m_pHead->m_len )
			ofs = m_pHead->m_len;

		_replace( ofs, 0, seq.length() );
		Char * pChars = beginWrite();
		seq.copyTo( pChars+ofs );
		endWrite();
		return seq.length();
	}

	//____ remove() ________________________________________________________________
	//
	/// @brief Removes characters from the buffer.
	///
	///
	/// @param ofs		Offset to remove characters from, counting from beginning of buffer content.
	///
	/// @param nChars	Number of characters to remove, counting from ofs. This can be set larger than
	///					number of characters available for removal in which case all content starting
	///					from ofs is removed.
	///
	/// If ofs is larger than number of characters no characters are removed.
	///
	/// @return Number of characters that were removed.

	int CharBuffer::remove( int ofs, int nChars )
	{
		if( ofs < 0 )
			ofs = 0;

		if( nChars <= 0 )
			return 0;

		return 0 - _replace( ofs, nChars, 0 );
	}

	//____ replace() _______________________________________________________________
	//
	/// @brief Replace a character.
	///
	/// @param ofs			Offset of character to be replaced, counting from beginning of buffer content.
	///
	///	@param character	Character to replace buffer content at ofs with.
	///
	/// If ofs is larger than number of characters then character is pushed on the back of the buffer instead.
	///
	/// @return Change in number of characters in buffer. Always equals 0.

	int CharBuffer::replace( int ofs, const Char& character )
	{
		if( ofs < 0 )
			return 0;

		if( ofs >= m_pHead->m_len )
			pushBack( character );
		else
			*((Char*)_ptr(ofs)) = character;
		return 0;
	}


	/// @brief Replace specified number of characters with (another number of) whitespaces.
	///
	/// @param ofs		Offset of first character to be replaced, counting from beginning of buffer content.
	///
	///	@param nDelete	Number of characters to be replaced.
	///
	/// @param nInsert	Number of whitespaces to replace the characters with.
	///
	/// If ofs is larger than number of characters nothing will be deleted and the to-be-inserted characters
	/// will just be pushed on the back of the buffer. If nDelete is larger than what is left of the buffer
	/// everything from ofs and forward is replaced.
	///
	/// @return Change in number of characters in buffer.

	int CharBuffer::replace( int ofs, int nDelete, int nInsert )
	{
		if( ofs < 0 )
			ofs = 0;

		if( nDelete < 0 )
			nDelete = 0;

		if( nInsert < 0 )
			nInsert = 0;

		return _replace( ofs, nDelete, nInsert );
	}


	/// @brief Replace specified number of characters with a specified string.
	///
	/// @param ofs		Offset of first character to be replaced, counting from beginning of buffer content.
	///
	///	@param nDelete	Number of characters to be replaced.
	///
	/// @param pChars	Pointer at null-terminated string to replace the characters with. This string can be
	///					longer or shorter than the characters it replaces.
	///
	/// If ofs is larger than number of characters nothing will be deleted and the to-be-inserted characters
	/// will just be pushed on the back of the buffer. If nDelete is larger than what is left of the buffer
	/// everything from ofs and forward is replaced.
	///
	/// @return Change in number of characters in buffer.

	int CharBuffer::replace( int ofs, int nDelete, const Char * pChars )
	{
		if( ofs < 0 )
			ofs = 0;
		if( nDelete < 0 )
			nDelete = 0;

		int nInsert = TextTool::strlen( pChars );
		return _replace( ofs, nDelete, nInsert, pChars );
	}


	/// @brief Replace specified number of characters with a string of specified length.
	///
	/// @param ofs		Offset of first character to be replaced, counting from beginning of buffer content.
	///
	///	@param nDelete	Number of characters to be replaced.
	///
	/// @param pChars	Pointer at a string of characters to replace the characters with.
	///
	///	@param nInsert 	Length of the string to replace the characters with. This can be
	///					longer or shorter than the characters it replaces.
	///
	/// If ofs is larger than number of characters nothing will be deleted and the to-be-inserted characters
	/// will just be pushed on the back of the buffer. If nDelete is larger than what is left of the buffer
	/// everything from ofs and forward is replaced.
	///
	/// @return Change in number of characters in buffer.

	int CharBuffer::replace( int ofs, int nDelete, const Char * pChars, int nInsert)
	{
		if( ofs < 0 )
			ofs = 0;
		if( nDelete < 0 )
			nDelete = 0;
		if( nInsert < 0 )
			nInsert = 0;

		return _replace( ofs, nDelete, nInsert, pChars );
	}


	/// @brief Replace specified number of characters with content of a char sequence.
	///
	/// @param ofs		Offset of first character to be replaced, counting from beginning of buffer content.
	///
	///	@param nDelete	Number of characters to be replaced.
	///
	/// @param seq		Character sequence to replace with. This can be longer or shorter than the characters it replaces.
	///
	///	@param nInsert 	Length of the string to replace the characters with. This can be
	///					longer or shorter than the characters to replace.
	///
	/// If ofs is larger than number of characters nothing will be deleted and the to-be-inserted characters
	/// will just be pushed on the back of the buffer. If nDelete is larger than what is left of the buffer
	/// everything from ofs and forward is replaced.
	///
	/// @return Change in number of characters in buffer.

	int CharBuffer::replace( int ofs, int nDelete, const CharSeq& seq )
	{
		if( ofs < 0 )
			ofs = 0;
		if( nDelete < 0 )
			nDelete = 0;

		if( ofs > m_pHead->m_len )
			ofs = m_pHead->m_len;

		int ret = _replace( ofs, nDelete, seq.length() );
		Char * pChars = beginWrite();
		seq.copyTo( pChars+ofs );
		endWrite();
		return ret;
	}




	//____ _replace() ______________________________________________________

	int CharBuffer::_replace( int ofs, int delChar, int addSpace, const Char * pChars )
	{
		// Forcing sane values

		if( ofs > m_pHead->m_len )
			ofs = m_pHead->m_len;

		if( delChar > m_pHead->m_len - ofs )
			delChar = m_pHead->m_len - ofs;


		int sizeChange = addSpace - delChar;

		// Dereference the styles of the characters to be deleted

		if( delChar != 0 )
			_derefStyle( ofs, delChar );

		// Check if we can just modify this buffer to fit things in

		if( m_pHead->m_refCnt == 1 && m_pHead->m_size >= m_pHead->m_len + sizeChange )
		{
			if( sizeChange != 0 )
			{
				// The changes will fit in this buffer. We will first attempt to just
				// move the beginning or end of the buffer to adjust space depending
				// on what would copy the least characters. Only if that fails we
				// move both buffers after which any extra space is at the end of
				// the buffer.

				int begBuffer = m_pHead->m_beg;
				int endBuffer = m_pHead->m_size - (m_pHead->m_beg + m_pHead->m_len);

				if( begBuffer >= sizeChange && ((ofs < m_pHead->m_len/2) || sizeChange > endBuffer) )
				{
					// Just move the beginning to adjust space.

					_copyChars( m_pHead, m_pHead->m_beg-sizeChange, m_pHead, m_pHead->m_beg, ofs );
					m_pHead->m_beg -= sizeChange;
				}
				else if( endBuffer >= sizeChange )
				{
					// Just move the end to adjust space.

					_copyChars( m_pHead, m_pHead->m_beg+ofs+addSpace, m_pHead, m_pHead->m_beg+ofs+delChar, m_pHead->m_len-ofs-delChar+1 );
				}
				else
				{
					// Move both beginning and end to make space

					_copyChars( m_pHead, 0, m_pHead, m_pHead->m_beg, ofs );
					_copyChars( m_pHead, ofs+addSpace, m_pHead, m_pHead->m_beg+ofs+delChar, sizeChange - begBuffer +1 );
					m_pHead->m_beg = 0;
				}

				m_pHead->m_len += sizeChange;
			}
		}
		else
		{
			// Create a new buffer, copy the content to be keept and leave room
			// at ofs for any characters to be added.

			BufferHead * pBuffer = _createBuffer( m_pHead->m_len + sizeChange );
			_copyChars( pBuffer, 0, m_pHead, m_pHead->m_beg, ofs );
			_copyChars( pBuffer, 0+ofs+addSpace, m_pHead, m_pHead->m_beg+ofs+delChar, m_pHead->m_len-ofs-delChar+1);
			pBuffer->m_len = m_pHead->m_len + sizeChange;

			// Do our own deref buffer

			m_pHead->m_refCnt--;
			if( m_pHead->m_refCnt == 0 )
			{
				_destroyBuffer(m_pHead);
				m_pHead = pBuffer;
			}
			else
			{
				m_pHead = pBuffer;
				_setChars( ofs, addSpace, c_emptyChar );
				_refStyle( 0, m_pHead->m_len );
			}

			m_pHead->m_refCnt++;
		}

		// Add the chars and reference the styles that were added

		if( addSpace != 0 )
		{
			if( pChars != 0 )
			{
				_copyChars( m_pHead, m_pHead->m_beg + ofs, pChars, addSpace );
				_refStyle( ofs, addSpace );
			}
			else
				_setChars( ofs, addSpace, c_emptyChar );
		}
		return addSpace - delChar;
	}


	//____ _createBuffer() __________________________________________________________

	CharBuffer::BufferHead * CharBuffer::_createBuffer( int size )
	{
		if( size <= 0 && g_pEmptyBuffer )
			return g_pEmptyBuffer;

		int nBytes = sizeof(BufferHead) + sizeof(Char)*(size+1);

		BufferHead * pBuffer = (BufferHead *) new char[nBytes];

		pBuffer->m_lockCnt	= 0;
		pBuffer->m_beg      = 0;
		pBuffer->m_len      = 0;
		pBuffer->m_refCnt   = 0;
		pBuffer->m_size     = size;

		* ((uint32_t *) &pBuffer[1]) = 0;		// null terminate.
		g_nBuffers++;

		if( size == 0 )
		{
			g_pEmptyBuffer = pBuffer;
			pBuffer->m_refCnt++;
		}

		return pBuffer;
	}

	//____ _compareBuffers() _______________________________________________________

	bool CharBuffer::_compareBuffers( const BufferHead * p1, const BufferHead * p2 )
	{
		// Some quick compares for common cases

		if( p1 == p2 )
			return true;

		if( p1->m_len != p2->m_len )
			return false;

		// More exhaustive test

		if( 0 == TextTool::strcmp( ((const Char*)&p1[1]) + sizeof(Char)*p1->m_beg, ((const Char*)&p2[1]) + sizeof(Char)*p2->m_beg ))
			return true;

		return false;
	}

	//____ compareTo() ____________________________________________________________

	int CharBuffer::compareTo( const CharBuffer * pBuffer ) const
	{
		if( m_pHead == pBuffer->m_pHead )
			return 0;

		return TextTool::strcmp( (Char*) _ptr(0), (Char*) pBuffer->_ptr(0) );
	}

    int CharBuffer::compareTo( const char * pCString ) const
    {
        return TextTool::strcmp( (Char*) _ptr(0), pCString);
    }

    int CharBuffer::compareTo( const std::string& str ) const
    {
        return TextTool::strcmp( (Char*) _ptr(0), str);
    }

	//____ compareCharCodesTo() ______________________________________________________

	int CharBuffer::compareCharCodesTo( const CharBuffer * pBuffer ) const
	{
		if( m_pHead == pBuffer->m_pHead )
			return 0;

		return TextTool::charcodecmp( (Char*) _ptr(0), (Char*) pBuffer->_ptr(0) );
	}

    int CharBuffer::compareCharCodesTo( const char * pCString ) const
    {
        return TextTool::charcodecmp( (Char*) _ptr(0), pCString );
    }

    int CharBuffer::compareCharCodesTo( const std::string& str ) const
    {
        return TextTool::charcodecmp( (Char*) _ptr(0), str );
    }

	//____ compareCharCodesIgnoreCaseTo() ____________________________________________

	int CharBuffer::compareCharCodesIgnoreCaseTo( const CharBuffer * pBuffer ) const
	{
		if( m_pHead == pBuffer->m_pHead )
			return 0;

		return TextTool::charcodecmpIgnoreCase( (Char*) _ptr(0), (Char*) pBuffer->_ptr(0) );
	}

    int CharBuffer::compareCharCodesIgnoreCaseTo( const char * pCString ) const
    {
        return TextTool::charcodecmpIgnoreCase( (Char*) _ptr(0), pCString );
    }

    int CharBuffer::compareCharCodesIgnoreCaseTo( const std::string& str ) const
    {
        return TextTool::charcodecmpIgnoreCase( (Char*) _ptr(0), str );
    }

	//____ fill() __________________________________________________________________
	//
	/// @brief	Fills a section of the buffer with specified character.
	///
	/// @param ch		Character to fill section with.
	/// @param ofs		Offset of first character to be replaced, counting from beginning of buffer content.
	/// @param len		Number of characters to be replaced.
	///
	/// Fully replaces the content of the specified buffer section by filling it with the specified
	/// character. If the section spans outside the buffer content it will be adjusted properly.

	void CharBuffer::fill( const Char& ch, int ofs, int len )
	{
		if( ofs < 0 || len <= 0 || ofs >= m_pHead->m_len )
			return;

		if( ofs + len > m_pHead->m_len )
			len = m_pHead->m_len - ofs;

		if( m_pHead->m_refCnt > 1 )
			_reshapeBuffer(0,0,m_pHead->m_len,0);

		TextTool::setChars( ch, (Char*)_ptr(ofs), len );
	}

	//____ setCharCodes() _____________________________________________________________
	//
	/// @brief	Sets the charcode for a range of characters.
	///
	/// @param charCode	The charcode to be used. This may not be 0.
	/// @param ofs		Offset of first character to have its charcode changed.
	/// @param len		Number of characters to have their charcode changed.
	///
	/// This method sets the charcode for all characters in the specified range. If the
	/// range spans outside the buffer content it will be adjusted properly.

	void CharBuffer::setCharCodes( uint16_t charCode, int ofs, int len )
	{
		if( ofs < 0 || len <= 0 || ofs >= m_pHead->m_len )
			return;

		if( ofs + len > m_pHead->m_len )
			len = m_pHead->m_len - ofs;

		if( m_pHead->m_refCnt > 1 )
			_reshapeBuffer(0,0,m_pHead->m_len,0);

		TextTool::setCharCode( charCode, (Char*)_ptr(ofs), len );
	}



	//___ setStyle() __________________________________________________________
	//
	/// @brief	Sets the style for a range of characters.
	///
	/// @param	pStyle	Pointer to the new style for the characters.
	/// @param	ofs		Offset of first character to have its style set, counting from beginning of buffer content.
	/// @param	len		Number of characters to have their style set.
	///
	/// If the range spans outside the buffer content it will be adjusted properly.

	void CharBuffer::setStyle( TextStyle * pStyle, int ofs, int len  )
	{
		if( ofs < 0 || len <= 0 || ofs >= m_pHead->m_len )
			return;

		if( ofs + len > m_pHead->m_len )
			len = m_pHead->m_len - ofs;

		if( m_pHead->m_refCnt > 1 )
			_reshapeBuffer(0,0,m_pHead->m_len,0);

		TextTool::setStyle( pStyle, (Char*)_ptr(ofs), len );
	}

	//___ clearStyle() ________________________________________________________
	//
	/// @brief	Clears the style for a range of characters.
	///
	/// @param ofs		Offset of first character to have its style cleared.
	///
	/// @param len		Number of characters to have their styles cleared.
	///

	void CharBuffer::clearStyle( int ofs, int len  )
	{
		if( ofs < 0 || len <= 0 || ofs >= m_pHead->m_len )
			return;

		if( ofs + len > m_pHead->m_len )
			len = m_pHead->m_len - ofs;

		if( m_pHead->m_refCnt > 1 )
			_reshapeBuffer(0,0,m_pHead->m_len,0);

		TextTool::clearStyle( (Char*)_ptr(ofs), len );
	}

	//____ findFirst() ____________________________________________________________
	//
	/// @brief	Finds the first occurence of the specified character sequence in buffer starting from offset.
	///
	/// @param seq		Sequence of characters to search for.
	/// @param ofs		Offset in buffer to start searching from.
	///
	/// Character styles are ignored by the comparison. Only the characters
	/// themselves are compared.
	///
	/// @return Offset in buffer for first match found or -1 if none found.

	int CharBuffer::findFirst( const CharSeq& _seq, int ofs ) const
	{
		if( ofs < 0 )
			ofs = 0;

		CharSeq::CharBasket seq		= _seq.getChars();
		Char *				pBuff	= (Char*)_ptr(0);

		while( ofs + seq.length <= m_pHead->m_len )
		{
			int i = 0;
			while( i < seq.length )
			{
				if( pBuff[ofs+i].code() != seq.ptr[i].code() )
					break;
				i++;
			}

			if( i == seq.length )
				return ofs;

			ofs++;
		}

		return -1;
	}


	/// @brief	Finds the first occurence of the specified character in buffer starting from offset.
	///
	/// @param character	Characters to search for.
	/// @param ofs			Offset in buffer to start searching from.
	///
	/// Any characters style is ignored by the comparison. Only the characters
	/// themselves are compared.
	///
	/// @return Offset in buffer for first match found or -1 if none found.

	int CharBuffer::findFirst( uint16_t character, int ofs ) const
	{
		if( ofs < 0 )
			ofs = 0;

		Char *				pBuff	= (Char*)_ptr(0);

		while( ofs < m_pHead->m_len )
		{
			if( pBuff[ofs].code() == character )
				return ofs;

			ofs++;
		}

		return -1;
	}

} // namespace wg
