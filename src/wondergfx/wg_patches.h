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
#ifndef WG_PATCHES_DOT_H
#define WG_PATCHES_DOT_H
#pragma once

#include <limits.h>

#include <wg_geo.h>

namespace wg
{
	template<class T>
	class PatchesT
	{
	public:

		//.____ Creation ______________________________________________________

		PatchesT();
		PatchesT( int startCapacity );
		PatchesT( RectT<T> * pArray, int capacity );
		PatchesT(const PatchesT<T>& source);
		PatchesT(const PatchesT<T>& source, const RectT<T>& trim);
		~PatchesT();

		//.____ Control ________________________________________________________

		bool			setCapacity( int capacity );
//		bool			setArray( Rect * pArray, int capacity );
		inline void		clear() { m_size = 0; }

		inline void		add( const RectT<T>& rect ) { if( rect.w > 0 && rect.h > 0 ) _add( rect, 0 ); }						// Adds the area
		void			add( const PatchesT<T> * pSource, int ofs = 0, int len = INT_MAX );

		void			sub( const RectT<T>& rect );											// Subtracts the area
		void			sub( const PatchesT<T> * pSource, int ofs = 0, int len = INT_MAX );

		inline void		push( const RectT<T>& rect );											// Adds the rect (no optimizations, overlap may occur).
		int				push( const PatchesT<T> * pSource, int ofs = 0, int len = INT_MAX );
		inline RectT<T>	pop();																// Pops last rect from the PatchesT.

		void			trimPush(const PatchesT<T>& source, const RectT<T>& trim);					// Adds the rects (no optimizations, overlap may occur).

		void			remove( int ofs );													// Deletes specific rect from the PatchesT.
		int				remove( int ofs, int len );											// Deletes range of rects from the PatchesT.

		//.____ Misc __________________________________________________________

		void			clip( const RectT<T>& clip );
		RectT<T>		bounds() const;

		//.____ Content ________________________________________________________

		const RectT<T>* begin() const { return m_pFirst; }
		const RectT<T>* end() const { return m_pFirst + m_size; }
		int				size() const { return m_size; }
		int				capacity() const { return m_capacity; }
		bool			isEmpty() const { return (m_size == 0); }

	private:
		const static int	c_defaultCapacity = 64;

		void		_add( const RectT<T>& rect, int startOffset );
		void		_expandMem( int spaceNeeded );

		RectT<T> * 	m_pFirst;
		int			m_size;
		int			m_capacity;
		bool		m_bOwnsArray;
	};


	template <class T>
	void PatchesT<T>::push( const RectT<T>& rect )
	{
		if(m_size==m_capacity)
			_expandMem(1);
		m_pFirst[m_size++]=rect;
	}

	template <class T>
	RectT<T> PatchesT<T>::pop()
	{
		if( m_size > 0 )
			return m_pFirst[--m_size];
		else
			return RectT<T>();
	}

	typedef PatchesT<pts>	PatchesPTS;
	typedef PatchesT<spx>	PatchesSPX;
	typedef	PatchesT<int>	PatchesI;
	typedef PatchesT<float>	PatchesF;


} // namespace wg
#endif //WG_PATCHES_DOT_H
