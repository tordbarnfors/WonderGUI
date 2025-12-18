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
#include <cstring>

namespace wg
{

	//____ constructor _____________________________________________________________

	template <class T>
	PatchesT<T>::PatchesT()
	{
		m_pFirst	= 0;
		m_size		= 0;
		m_capacity	= 0;
		m_bOwnsArray = true;
	}

	template <class T>
	PatchesT<T>::PatchesT( int startCapacity )
	{
		m_pFirst	= new RectT<T>[startCapacity];
		m_size		= 0;
		m_capacity	= startCapacity;
		m_bOwnsArray = true;
	}

	template <class T>
	PatchesT<T>::PatchesT( RectT<T> * pArray, int capacity )
	{
		m_pFirst 	= pArray;
		m_size		= 0;
		m_capacity	= capacity;
		m_bOwnsArray = false;
	}

	template <class T>
	PatchesT<T>::PatchesT(const PatchesT<T>& source)
	{
		m_pFirst = new RectT<T>[source.m_capacity];
		m_size = source.m_size;
		m_capacity = source.m_capacity;
		m_bOwnsArray = true;

		for (int i = 0; i < source.m_size; i++)
				m_pFirst[i] = source.m_pFirst[i];
	}


	template <class T>
	PatchesT<T>::PatchesT(const PatchesT<T>& source, const RectT<T>& trim)
	{
		m_pFirst = new RectT<T>[source.m_capacity];
		m_size = 0;
		m_capacity = source.m_capacity;
		m_bOwnsArray = true;

		for (int i = 0; i < source.m_size; i++)
		{
			const RectT<T>& rect = source.m_pFirst[i];
			if (rect.isOverlapping(trim))
				m_pFirst[m_size++] = RectT<T>::overlap(rect, trim);
		}
	}



	//____ Destructor ______________________________________________________________

	template <class T>
	PatchesT<T>::~PatchesT()
	{
		if( m_bOwnsArray )
			delete [] m_pFirst;
	}

	//____ setCapacity() ___________________________________________________________

	template <class T>
	bool PatchesT<T>::setCapacity( int capacity )
	{
		if( capacity < m_size )
			return false;

		RectT<T> * pNew = nullptr;
		if( capacity > 0 )
		{
			pNew = new RectT<T>[capacity];

			if( m_size > 0 )
				memcpy( pNew, m_pFirst, sizeof(RectT<T>)*m_size );
		}

		if( m_bOwnsArray )
		{
			delete [] m_pFirst;
		}

		m_pFirst = pNew;
		m_capacity = capacity;
		return true;
	}

	//____ _add() __________________________________________________________________

	template <class T>
	void PatchesT<T>::_add( const RectT<T>& rect, int startOffset )
	{
		for( int i = startOffset ; i < m_size ; i++ )
		{
			RectT<T> * pR = m_pFirst + i;

			// Bail out early if no intersection at all.

			if( rect.x >= pR->x + pR->w || rect.x + rect.w <= pR->x ||
			   rect.y >= pR->y + pR->h || rect.y + rect.h <= pR->y )
				continue;															// No intersection.

			// Check for total coverage

			if( rect.x >= pR->x  &&  rect.x + rect.w <= pR->x + pR->w  &&
			   rect.y >= pR->y  &&  rect.y + rect.h <= pR->y + pR->h  )
				return;  															// newR totally covered by pR

			if( rect.x <= pR->x  &&  rect.x + rect.w >= pR->x + pR->w  &&
			   rect.y <= pR->y  &&  rect.y + rect.h >= pR->y + pR->h  )
			{
				remove( i-- );														// pR totally covered by newR
				continue;
			}

			
			// Clip newR against pR.

			RectT<T> newR = rect;

			RectT<T> mask = *pR;

			// Cut off upper part

			if (newR.y < mask.y)
			{
				RectT<T> xR(newR.x, newR.y, newR.w, mask.y - newR.y);
				_add(xR, i + 1);

				newR.h -= xR.h;
				newR.y += xR.h;
			}

			// Cut off lower part

			if (newR.y + newR.h > mask.y + mask.h )
			{
				RectT<T>	xR;
				xR.x = newR.x;
				xR.y = mask.y + mask.h;
				xR.w = newR.w;
				xR.h = (newR.y + newR.h) - (mask.y + mask.h);
				_add(xR, i + 1);

				newR.h -= xR.h;
			}

			if (newR.h > 0)
			{
				// Cut off left part

				if (newR.x < mask.x)
				{
					RectT<T>	xR;
					xR.x = newR.x;
					xR.y = newR.y;
					xR.w = mask.x - newR.x;
					xR.h = newR.h;

					_add(xR, i + 1);
				}

				// Cut off right part

				if (newR.x + newR.w > mask.x + mask.w)
				{
					RectT<T>	xR;
					xR.x = mask.x + mask.w;
					xR.y = newR.y;
					xR.w = (newR.x + newR.w) - (mask.x + mask.w);
					xR.h = newR.h;

					_add(xR, i + 1);
				}
			}

			// We have split our rectangle into visible pieces and masked them one by one.
			// There is nothing left of original rectangle.

			return;
		}

		// If we haven't returned yet we have a patch left to add.

		push( rect );
	}

	//____ add() ___________________________________________________________________

	template <class T>
	void PatchesT<T>::add( const PatchesT<T> * pSource, int ofs, int len )
	{
		if( ofs > pSource->m_size )
			return;

		if( len > pSource->m_size - ofs )
			len = pSource->m_size - ofs;

		for( int i = 0 ; i < len ; i++ )
			_add( pSource->m_pFirst[ofs++], 0 );
	}

	//____ sub() ___________________________________________________________________

	template <class T>
	void PatchesT<T>::sub( const PatchesT<T> * pSource, int ofs, int len )
	{
		if( ofs > pSource->m_size )
			return;

		if( len > pSource->m_size - ofs )
			len = pSource->m_size - ofs;

		for( int i = 0 ; i < len ; i++ )
			sub( pSource->m_pFirst[ofs++] );
	}

	template <class T>
	void PatchesT<T>::sub( const RectT<T>& subR )
	{
		if( subR.w == 0 || subR.h == 0 )
			return;

		for( int i = 0 ; i < m_size ; i++ )
		{
			RectT<T> rect = m_pFirst[i];

			if( !(rect.x + rect.w > subR.x && rect.y + rect.h > subR.y &&
				rect.x < subR.x + subR.w && rect.y < subR.y + subR.h) )
				continue;												// No overlap at all.

	   		// Only partially covered by our subtract rectangle?

	   		if( subR.x > rect.x || subR.y > rect.y || subR.x + subR.w < rect.x + rect.w
	   				|| subR.y + subR.h < rect.y + rect.h )
			{
				// Handles partial coverage. Up to 4 new rectangles are created for
				// the areas that aren't covered.

				if( rect.y < subR.y )													// Top part
				{
					push( RectT<T>( rect.x, rect.y, rect.w, subR.y - rect.y) );				// Not optimal, will cause unnecessary processing later.
				}

				if( rect.x < subR.x )													// Left part
				{
					RectT<T> newR;

					newR.x = rect.x;
					newR.w = subR.x - rect.x;

					if( rect.y > subR.y )
						newR.y = rect.y;
					else
						newR.y = subR.y;

					if( rect.y + rect.h < subR.y + subR.h )
						newR.h = rect.y + rect.h - newR.y;
					else
						newR.h = subR.y + subR.h - newR.y;

						push( newR );													// Not optimal, will cause unnecessary processing later.
				}

				if( rect.x + rect.w > subR.x + subR.w )					// Right part
				{
					RectT<T> newR;

					newR.x = subR.x + subR.w;
					newR.w = rect.x + rect.w - ( subR.x + subR.w );

					if( rect.y > subR.y )
						newR.y = rect.y;
					else
						newR.y = subR.y;

					if( rect.y + rect.h < subR.y + subR.h )
						newR.h = rect.y + rect.h - newR.y;
					else
						newR.h = subR.y + subR.h - newR.y;

						push( newR );													// Not optimal, will cause unnecessary processing later.
				}

				if( rect.y + rect.h > subR.y + subR.h )					// Bottom part
				{
					push( RectT<T>( rect.x,
								  subR.y + subR.h,
								  rect.w,
								  rect.y + rect.h - ( subR.y + subR.h )) );				// Not optimal, will cause unnecessary processing later.
				}
			}

			// Remove the old rectangle

			remove( i-- );
		}
	}

	//____ push() __________________________________________________________________

	template <class T>
	int PatchesT<T>::push( const PatchesT<T> * pSource, int ofs, int len )
	{
		if( ofs > pSource->m_size )
			return 0;

		if( len > pSource->m_size - ofs )
			len = pSource->m_size - ofs;

		if( m_capacity - m_size < len )
			_expandMem(len);

		memcpy( m_pFirst + m_size, pSource->m_pFirst + ofs, sizeof(RectT<T>)*len );
		m_size += len;
		return len;
	}

	//____ trimPush() _________________________________________________________

	template <class T>
	void PatchesT<T>::trimPush(const PatchesT<T>& source, const RectT<T>& trim)
	{
		for (int i = 0; i < source.m_size; i++)
		{
			const RectT<T>& rect = source.m_pFirst[i];
			if (rect.isOverlapping(trim))
			{
				if (m_size == m_capacity)
					_expandMem(1);
				m_pFirst[m_size++] = RectT<T>::overlap(rect, trim);
			}
		}
	}



	//____ remove() ________________________________________________________________

	template <class T>
	void  PatchesT<T>::remove( int ofs )
	{
		if( ofs >= m_size )
			return;

		m_pFirst[ofs] = m_pFirst[--m_size];
	}

	template <class T>
	int PatchesT<T>::remove( int ofs, int len )
	{
		if( ofs > m_size )
			return 0;

		if( len > m_size - ofs )
			len = m_size - ofs;


		int src = m_size - len;
		if( src < ofs + len )
			src = ofs + len;

		while( src < m_size )
			m_pFirst[ofs++] = m_pFirst[src++];

		m_size -= len;
		return len;
	}


	//____ clip() __________________________________________________________________

	template <class T>
	void PatchesT<T>::clip( const RectT<T>& clip )
	{
		for( RectT<T> * pRect = m_pFirst ; pRect < m_pFirst + m_size ; pRect++ )
		{

			if( clip.isOverlapping(*pRect) )
				*pRect = RectT<T>::overlap(*pRect, clip);
			else
				*pRect-- = m_pFirst[--m_size];				// Remove the rectangle

		}
	}

	//____ bounds() _________________________________________________________________

	template <class T>
	RectT<T> PatchesT<T>::bounds() const
	{
		if( m_size == 0 )
			return RectT<T>();

		T x1 = m_pFirst->x;
		T x2 = m_pFirst->x + m_pFirst->w;
		T y1 = m_pFirst->y;
		T y2 = m_pFirst->y + m_pFirst->h;

		for( int i = 1 ; i < m_size ; i++ )
		{
			if( m_pFirst[i].x < x1 )
				x1 = m_pFirst[i].x;
			if( m_pFirst[i].y < y1 )
				y1 = m_pFirst[i].y;
			if( m_pFirst[i].x + m_pFirst[i].w > x2 )
				x2 = m_pFirst[i].x + m_pFirst[i].w;
			if( m_pFirst[i].y + m_pFirst[i].h > y2 )
				y2 = m_pFirst[i].y + m_pFirst[i].h;
		}

		return RectT<T>(x1,y1,x2-x1,y2-y1);
	}

	//____ _expand() _______________________________________________________________

	template <class T>
	void PatchesT<T>::_expandMem( int spaceNeeded )
	{
		int capacity = m_capacity==0?c_defaultCapacity:m_capacity;


		while( capacity - m_size < spaceNeeded )
			capacity += capacity;

		setCapacity(capacity);
	}

} // namespace wg
