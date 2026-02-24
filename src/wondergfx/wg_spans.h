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
#ifndef WG_SPANS_DOT_H
#define WG_SPANS_DOT_H
#pragma once

#include <algorithm>

namespace wg
{

	template<typename Prim, int Capacity, Prim minGap>
	class Spans
	{
	public:
		struct Span
		{
			Prim	begin;
			Prim	end;
		};

		int		size = 0;
		Span	array[Capacity+1];		// We need margin since we add before removing gap.

		//____ clear() ______________________________________________________________

		void clear()
		{
			size = 0;
		}

		//____ add() ________________________________________________________________

		void add( Prim begin, Prim end )
		{
			Span * pX = array;
			Span * pEnd = array + size;

			// Copy existing spans until we reach our insertion point

			while( (pX < pEnd) && (begin - minGap) > pX->end )
				pX++;

			Span * pOut = pX;

			// Merge our new span with any overlapping existing ones

			while( pX < pEnd && (end + minGap) >= pX->begin )
			{
				begin = std::min(begin, pX->begin );
				end = std::max(end, pX->end );
				pX++;
			}

			// Insert our new span

			if( pOut == pX )
			{
				// Make space and insert our span.

				pX = pEnd;

				while( pX > pOut )
				{
					pX[0] = pX[-1];
					pX--;
				}

				pOut->begin = begin;
				pOut->end = end;

				size = int(pEnd+1 - array);
			}
			else
			{
				// Insert our span and copy remaining existing ones.

				pOut->begin = begin;
				pOut->end = end;
				pOut++;

				while( pX < pEnd )
					* pOut++ = * pX++;

				size = int(pOut - array);
			}

			if( size > Capacity )
				removeSmallestGap();
		}

		//____ removeSmallestGap() __________________________________________________

		void removeSmallestGap()
		{
			assert( size > 1 );

			Span * pLast = &array[size-1];
			Span * pSmallestGap = array;
			Prim smallestGap = array[1].begin - array[0].end;

			for( Span * p = array + 1 ; p < pLast ; p++ )
			{
				Prim gap = p[1].begin - p[0].end;
				if( gap < smallestGap )
				{
					smallestGap = gap;
					pSmallestGap = p;
				}
			}

			pSmallestGap[0].end = pSmallestGap[1].end;

			for( Span * p = pSmallestGap+1 ; p < pLast ; p++ )
				p[0] = p[1];
		}
	};

} // namespace wg
#endif //WG_SPANS_DOT_H
