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
#ifndef	WAX_RANGEALLOCATOR_DOT_H
#define	WAX_RANGEALLOCATOR_DOT_H
#pragma once

#include <vector>

namespace wax
{

	//____ RangeAllocator ________________________________________________________
	/**
	 * @brief Hands out ranges of a block, by offset, and takes them back.
	 *
	 * For blocks that shouldn't hold the bookkeeping of what is taken of them, like a buffer
	 * shared with the GPU that is slow for the CPU to read. All that is kept is here, in
	 * ordinary memory, and the block itself is never touched: what is at an offset is up to
	 * whoever has the block. MemHeap, which keeps its bookkeeping in the memory it hands out,
	 * is for the opposite case.
	 *
	 * Ranges are whole numbers of the alignment given, both where they start and how long they
	 * are. The first range that is big enough is taken, counting from the start of the block,
	 * and a range given back is joined with free ones next to it.
	 */

	class RangeAllocator
	{
	public:
		RangeAllocator( int size, int alignment );

		int		alloc( int size );						// Returns the offset of the range, -1 if there is no room.
		void	free( int offset, int size );			// A range from alloc(), with the size it was asked for.

		int		size() const { return m_size; }
		int		allocated() const { return m_allocated; }		// With sizes rounded up to the alignment.
		bool	isEmpty() const { return m_allocated == 0; }
		int		largestFree() const;

	private:

		int		_rounded( int size ) const { return ((size + m_alignment - 1) / m_alignment) * m_alignment; }

		struct Range
		{
			int		offset;
			int		size;
		};

		std::vector<Range>	m_free;				// In order of offset, no two of them next to each other.

		int		m_size;
		int		m_alignment;
		int		m_allocated = 0;
	};

} // namespace wax
#endif //WAX_RANGEALLOCATOR_DOT_H
