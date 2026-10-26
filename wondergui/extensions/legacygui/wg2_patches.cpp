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
#include <wg2_patches.h>

//____ Constructor _____________________________________________________________

WgPatches::WgPatches()
{
	m_pFirst	= 0;
	m_size		= 0;
	m_capacity	= 0;
	m_bOwnsArray = true;
}

WgPatches::WgPatches( int startCapacity )
{
	m_pFirst	= new WgRect[startCapacity];
	m_size		= 0;
	m_capacity	= startCapacity;
	m_bOwnsArray = true;
}

WgPatches::WgPatches( WgRect * pArray, int capacity )
{
	m_pFirst 	= pArray;
	m_size		= 0;
	m_capacity	= capacity;
	m_bOwnsArray = false;
}

//____ Destructor ______________________________________________________________

WgPatches::~WgPatches()
{
	if( m_bOwnsArray )
		delete [] m_pFirst;
}

//____ setCapacity() ___________________________________________________________

bool WgPatches::setCapacity( int capacity )
{
	if( capacity < m_size )
		return false;
	
	WgRect * pNew = 0;
	if( capacity > 0 )
	{
		pNew = new WgRect[capacity];
        if(m_pFirst != nullptr)
		    memcpy( pNew, m_pFirst, sizeof(WgRect)*m_size );
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

void WgPatches::_add( const WgRect& rect, int startOffset )
{
	WgRect newR = rect;
	
	for( int i = startOffset ; i < m_size ; i++ )
	{
		WgRect * pR = m_pFirst + i;

		// Bail out early if no intersection at all.

		if( newR.x >= pR->x + pR->w || newR.x + newR.w <= pR->x ||
			newR.y >= pR->y + pR->h || newR.y + newR.h <= pR->y )
			continue;															// No intersection.
		
		// Check for total coverage

		if( newR.x >= pR->x  &&  newR.x + newR.w <= pR->x + pR->w  &&
		  newR.y >= pR->y  &&  newR.y + newR.h <= pR->y + pR->h  )
			return;                                      						// newR totally covered by pR

		if( newR.x <= pR->x  &&  newR.x + newR.w >= pR->x + pR->w  &&
		  newR.y <= pR->y  &&  newR.y + newR.h >= pR->y + pR->h  )
		{
			remove( i-- );														// pR totally covered by newR
			continue;
		}

		// In four special cases we rather clip pR than newR...

		if( newR.x <= pR->x && newR.x + newR.w >= pR->x + pR->w )
 		{
 			if( newR.y <= pR->y && newR.y + newR.h > pR->y )
 			{
 				int diff = newR.y + newR.h - pR->y;
 				pR->y += diff;
 				pR->h -= diff;
				continue;
 			}

 			if( newR.y < pR->y + pR->h && newR.y + newR.h >= pR->y + pR->h )
 			{
 				int diff = pR->y + pR->h - newR.y;
 				pR->h -= diff;
				continue;
 			}

 		}
 		else if( newR.y <= pR->y && newR.y + newR.h >= pR->y + pR->h )
 		{
 			if( newR.x <= pR->x && newR.x + newR.w > pR->x )
 			{
 				int diff = newR.x + newR.w - pR->x;
 				pR->x += diff;
 				pR->w -= diff;
				continue;
 			}

 			if( newR.x < pR->x + pR->w && newR.x + newR.w >= pR->x + pR->w )
			{
				int diff = pR->x + pR->w - newR.x;
 				pR->w -= diff;
				continue;
 			}
 		}
		
		// Clip newR against pR.

		WgRect	xR;
		bool	bExtraRect = false;

		// Cut off upper part

		if( newR.y < pR->y )
		{
			bExtraRect = true;
			xR.x = newR.x;
			xR.w = newR.w;
			xR.y = newR.y;
			xR.h = pR->y - newR.y;

			newR.h -= xR.h;
			newR.y += xR.h;
		}

		// Take care of middle part(s)

		if( newR.x < pR->x )
		{
			if( !bExtraRect )
			{
				bExtraRect = true;
				xR.x = newR.x;
				xR.y = newR.y;
				xR.w = pR->x - newR.x;
				if( newR.y + newR.h < pR->y + pR->h )
					xR.h = newR.h;
				else
					xR.h = pR->y + pR->h - xR.y;
			}
			else
			{
				newR.w = pR->x - newR.x;
				_add( xR, i+1 );
				continue;
			}
		}

		if( newR.x + newR.w > pR->x + pR->w )
		{
			if( !bExtraRect )
			{
				bExtraRect = true;
				xR.x = pR->x + pR->w;
				xR.y = newR.y;
				xR.w = newR.x + newR.w - (pR->x + pR->w);
				if( newR.y + newR.h < pR->y + pR->h )
					xR.h = newR.h;
				else
					xR.h = pR->y + pR->h - xR.y;
			}
			else
			{
				newR.w = newR.x + newR.w - (pR->x + pR->w);
				newR.x = pR->x + pR->w;
				_add( xR, i+1 );
				continue;
			}
		}

		// Take care of lower part

		if( newR.y + newR.h > pR->y + pR->h )
		{
			newR.h = newR.y + newR.h - (pR->y + pR->h);
			newR.y = pR->y + pR->h;
			_add( xR, i+1 );
			continue;
		}

		// If we got here, we certainly have xR and just trash in newR...

		newR = xR;
	}
	
	// If we haven't returned yet we have a patch left to add.
	
	push( newR );
}

//____ add() ___________________________________________________________________

void WgPatches::add( const WgPatches * pSource, int ofs, int len )
{
	if( ofs > pSource->m_size )
		return;
		
	if( len > pSource->m_size - ofs )
		len = pSource->m_size - ofs;

	for( int i = 0 ; i < len ; i++ )
		_add( pSource->m_pFirst[ofs++], 0 );
}

//____ sub() ___________________________________________________________________

void WgPatches::sub( const WgPatches * pSource, int ofs, int len )
{
	if( ofs > pSource->m_size )
		return;
		
	if( len > pSource->m_size - ofs )
		len = pSource->m_size - ofs;

	for( int i = 0 ; i < len ; i++ )
		sub( pSource->m_pFirst[ofs++] );
}

void WgPatches::sub( const WgRect& subR )
{
	if( subR.w == 0 || subR.h == 0 )
		return;

	for( int i = 0 ; i < m_size ; i++ )
	{
		WgRect rect = m_pFirst[i];
				
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
				push( WgRect( rect.x, rect.y, rect.w, subR.y - rect.y) );				// Not optimal, will cause unnecessary processing later.
			}

			if( rect.x < subR.x )													// Left part
			{
				WgRect newR;

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
				WgRect newR;

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
				push( WgRect( rect.x,
							  subR.y + subR.h, 
							  rect.w, 
							  rect.y + rect.h - ( subR.y + subR.h )) );				// Not optimal, will cause unnecessary processing later.
			}
		}

		// Delete the old rectangle

		remove( i-- );
	}
}

//____ push() __________________________________________________________________

int WgPatches::push( const WgPatches * pSource, int ofs, int len )
{
	if( ofs > pSource->m_size )
		return 0;
		
	if( len > pSource->m_size - ofs )
		len = pSource->m_size - ofs;
	
	if( m_capacity - m_size < len )
		_expandMem(len);

	memcpy( m_pFirst + m_size, pSource->m_pFirst + ofs, sizeof(WgRect)*len );
	m_size += len;
	return len;
}

//____ remove() ________________________________________________________________

void  WgPatches::remove( int ofs )
{
	if( ofs >= m_size )
		return;
		
	m_pFirst[ofs] = m_pFirst[--m_size];
}

int WgPatches::remove( int ofs, int len )
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

void WgPatches::clip( const WgRect& clip )
{
	for( WgRect * pRect = m_pFirst ; pRect < m_pFirst + m_size ; pRect++ )
	{
		if( clip.isOverlapping(*pRect) )
			*pRect = WgRect::overlap(*pRect, clip);
		else
			*pRect-- = m_pFirst[--m_size];				// Remove the rectangle
	}
}

//____ bounds() _________________________________________________________________

WgRect WgPatches::bounds() const
{
	if( m_size == 0 )
		return WgRect();

	int x1 = m_pFirst->x;
	int x2 = m_pFirst->x + m_pFirst->w;
	int y1 = m_pFirst->y;
	int y2 = m_pFirst->y + m_pFirst->h;
		
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
	
	return WgRect(x1,y1,x2-x1,y2-y1);
}

//____ _expand() _______________________________________________________________

void WgPatches::_expandMem( int spaceNeeded )
{
	int capacity = m_capacity==0?c_defaultCapacity:m_capacity;
	
	
	while( capacity - m_size < spaceNeeded )
		capacity += capacity;

	setCapacity(capacity);
}
