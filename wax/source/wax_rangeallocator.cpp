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

#include <wax_rangeallocator.h>

namespace wax
{

	//____ constructor ___________________________________________________________
	//
	// What doesn't make a whole number of the alignment at the end of the block is left unused.

	RangeAllocator::RangeAllocator( int size, int alignment )
	{
		m_alignment = alignment > 0 ? alignment : 1;
		m_size = size > 0 ? (size / m_alignment) * m_alignment : 0;

		if( m_size > 0 )
			m_free.push_back( { 0, m_size } );
	}

	//____ alloc() _______________________________________________________________

	int RangeAllocator::alloc( int size )
	{
		if( size <= 0 || size > m_size )
			return -1;

		size = _rounded(size);

		for( auto it = m_free.begin() ; it != m_free.end() ; ++it )
		{
			if( it->size < size )
				continue;

			int offset = it->offset;

			if( it->size == size )
				m_free.erase(it);
			else
			{
				it->offset += size;
				it->size -= size;
			}

			m_allocated += size;
			return offset;
		}

		return -1;
	}

	//____ free() ________________________________________________________________

	void RangeAllocator::free( int offset, int size )
	{
		if( size <= 0 || offset < 0 || offset % m_alignment != 0 )
			return;

		size = _rounded(size);

		if( offset + size > m_size )
			return;

		// The first free range after ours. The one before it in the list is then the free range
		// before ours, if there is one.

		auto it = m_free.begin();
		while( it != m_free.end() && it->offset < offset )
			++it;

		bool bJoinsPrev = (it != m_free.begin() && (it - 1)->offset + (it - 1)->size == offset);
		bool bJoinsNext = (it != m_free.end() && offset + size == it->offset);

		// Something that overlaps a free range was never ours to free.

		if( (it != m_free.begin() && (it - 1)->offset + (it - 1)->size > offset) || (it != m_free.end() && offset + size > it->offset) )
			return;

		if( bJoinsPrev && bJoinsNext )
		{
			(it - 1)->size += size + it->size;
			m_free.erase(it);
		}
		else if( bJoinsPrev )
			(it - 1)->size += size;
		else if( bJoinsNext )
		{
			it->offset = offset;
			it->size += size;
		}
		else
			m_free.insert( it, { offset, size } );

		m_allocated -= size;
	}

	//____ largestFree() _________________________________________________________

	int RangeAllocator::largestFree() const
	{
		int largest = 0;

		for( auto& range : m_free )
		{
			if( range.size > largest )
				largest = range.size;
		}

		return largest;
	}

} // namespace wax
