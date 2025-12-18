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
#ifndef WG2_PATCHES_DOT_H
#define WG2_PATCHES_DOT_H

#include <limits.h>

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

class WgPatches
{
public:
	
	WgPatches();
	WgPatches( int startCapacity );
	WgPatches( WgRect * pArray, int capacity );
	~WgPatches();

	bool			setCapacity( int capacity );
	bool			setArray( WgRect * pArray, int capacity );

	inline void		add( const WgRect& rect ) { if( rect.w > 0 && rect.h > 0 ) _add( rect, 0 ); }						// Adds the area
	void			add( const WgPatches * pSource, int ofs = 0, int len = INT_MAX );
	
	void			sub( const WgRect& rect );											// Subtracts the area
	void			sub( const WgPatches * pSource, int ofs = 0, int len = INT_MAX );

	inline void		push( const WgRect& rect );											// Adds the rect (no optimizations, overlap may occur).
	int				push( const WgPatches * pSource, int ofs = 0, int len = INT_MAX );
	inline WgRect	pop();																// Pops last rect from the patches.

	void			remove( int ofs );													// Deletes specific rect from the patches.
	int				remove( int ofs, int len );											// Deletes range of rects from the patches.

	inline void		clear() { m_size = 0; }
	void			clip( const WgRect& clip );
	WgRect			bounds() const;

	const WgRect *	begin() const { return m_pFirst; }
	const WgRect *	end() const { return m_pFirst + m_size; }
	int				size() const { return m_size; }
	int				capacity() const { return m_capacity; }
	bool			isEmpty() const { return (m_size == 0); }

private:
	const static int	c_defaultCapacity = 64;

	void		_add( const WgRect& rect, int startOffset );
	void		_expandMem( int spaceNeeded );

	WgRect * 	m_pFirst;
	int			m_size;
	int			m_capacity;
	bool		m_bOwnsArray;
};



void WgPatches::push( const WgRect& rect )
{ 
	if(m_size==m_capacity)
		_expandMem(1);
	m_pFirst[m_size++]=rect;
}

WgRect WgPatches::pop()
{ 
	if( m_size>0 )
		return m_pFirst[--m_size];
	else
		return WgRect(); 
}


#endif //WG2_PATCHES_DOT_H
