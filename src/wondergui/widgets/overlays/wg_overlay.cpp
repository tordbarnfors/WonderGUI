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

#include <wg_overlay.h>
#include <wg_patches.h>

namespace wg
{

	const TypeInfo Overlay::TYPEINFO = { "Overlay", &Container::TYPEINFO };
	const TypeInfo Overlay::Slot::TYPEINFO = { "Overlay::Slot", &StaticSlot::TYPEINFO };


	//____ constructor ____________________________________________________________

	Overlay::Overlay() : mainSlot(this)
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Overlay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _matchingHeight() _______________________________________________________

	spx Overlay::_matchingHeight( spx width, int scale ) const
	{
		if( mainSlot._widget() )
			return mainSlot._widget()->_matchingHeight( width, scale );
		else
			return Widget::_matchingHeight(width, scale);
	}

	//____ _matchingWidth() _______________________________________________________

	spx Overlay::_matchingWidth( spx height, int scale ) const
	{
		if( mainSlot._widget() )
			return mainSlot._widget()->_matchingWidth( height, scale );
		else
			return Widget::_matchingWidth(height, scale);
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX Overlay::_defaultSize(int scale) const
	{
		if( mainSlot._widget() )
			return mainSlot._widget()->_defaultSize(scale);
		else
			return SizeSPX(64,64);
	}


	//____ _onRequestRender() _____________________________________________________

	void Overlay::_onRequestRender( const RectSPX& rect, const Slot * pSlot )
	{
		// Clip our geometry and put it in a dirtyrect-list

		PatchesSPX patches;
		patches.add( RectSPX::overlap( rect, RectSPX(0,0,m_size)) );

		// Remove portions of dirty rect that are covered by opaque upper siblings,
		// possibly filling list with many small dirty rects instead.

		const Slot * pCover = _beginOverlaySlots();
		const Slot * pEnd = pSlot ? pSlot : _endOverlaySlots();

		int incNext = _sizeOfOverlaySlot();

		while( pCover <  pEnd )
		{
			if( pCover->m_bVisible && (pCover->m_geo + pCover->_widget()->_overflow()).isOverlapping( rect ) )
				pCover->_widget()->_maskPatches( patches, pCover->m_geo, RectSPX(0,0, m_size ) );

			pCover = _incOverlaySlot(pCover,incNext);
		}

		// Make request render calls

		for( const RectSPX * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
			_requestRender( * pRect );
	}


	//____ _firstChild() ___________________________________________________________

	Widget* Overlay::_firstChild() const
	{
		const Slot * p = _beginOverlaySlots();
		if (p != _endOverlaySlots())
			return p->_widget();

		return mainSlot._widget();
	}

	//____ _lastChild() ____________________________________________________________

	Widget* Overlay::_lastChild() const
	{
		if (mainSlot._widget())
			return mainSlot._widget();
		else
		{
			const Slot * pSlot = _endOverlaySlots();
			if (pSlot > _beginOverlaySlots())
			{
				pSlot = _decOverlaySlot(pSlot, _sizeOfOverlaySlot());
				return pSlot->_widget();
			}

			return nullptr;
		}
	}

	//____ _firstSlotWithGeo() _____________________________________________________

	void Overlay::_firstSlotWithGeo( SlotWithGeo& package ) const
	{
		const Slot * p = _beginOverlaySlots();

		while( p < _endOverlaySlots() && !p->m_bVisible )
			p = _incOverlaySlot(p,_sizeOfOverlaySlot());

		if( p < _endOverlaySlots() )
		{
			package.geo = p->m_geo;
			package.pSlot = p;
		}
		else if (mainSlot._widget())
		{
			package.geo = m_size;
			package.pSlot = &mainSlot;
		}
		else
			package.pSlot = nullptr;
	}

	//____ _nextSlotWithGeo() _______________________________________________________

	void Overlay::_nextSlotWithGeo( SlotWithGeo& package ) const
	{
		const Slot * p = (Slot*) package.pSlot;

		if (p == (StaticSlot*) &mainSlot)
		{
			package.pSlot = nullptr;
			return;
		}

		int slotSize = _sizeOfOverlaySlot();
		p = _incOverlaySlot(p,slotSize);

		while( p < _endOverlaySlots() && !p->m_bVisible )
			p = _incOverlaySlot(p,slotSize);


		if( p < _endOverlaySlots() )
		{
			package.geo = ((Slot*)p)->m_geo;
			package.pSlot = p;
		}
		else if (mainSlot._widget())
		{
			package.geo = m_size;
			package.pSlot = &mainSlot;
		}
		else
			package.pSlot = nullptr;
	}

	//____ _releaseChild() ____________________________________________________

	void Overlay::_releaseChild(StaticSlot * pSlot)
	{
		if (pSlot == &mainSlot)
		{
			mainSlot._setWidget(nullptr);
			_requestRender();
			_requestResize();
		}
	}

	//____ _replaceChild() _______________________________________________________

	void Overlay::_replaceChild( StaticSlot * pSlot, Widget * pNewWidget )
	{
		if (pSlot == &mainSlot)
		{
			mainSlot._setWidget(pNewWidget);
			if( pNewWidget )
				pNewWidget->_resize(m_size, m_scale);			//TODO: Should be content size here (and in all other _setWidget() methods?)
			_requestRender();
			_requestResize();
		}
	}

	//____ _resize() _______________________________________________________

	void Overlay::_resize(const SizeSPX& size, int scale)
	{
		Container::_resize(size, scale);

		if ( mainSlot._widget())
			mainSlot._widget()->_resize(size, scale);

		//TODO: Update size and scale of other children
	}


	//____ _slotGeo() _________________________________________________________

	RectSPX Overlay::_slotGeo( const StaticSlot * pSlot ) const
	{
		if( pSlot == &mainSlot )
			return m_skin.contentRect( m_size, m_scale, m_state );

		return ((Slot*)pSlot)->m_geo;
	}

	//____ _childRequestRender() _______________________________________________

	void Overlay::_childRequestRender( StaticSlot * _pSlot, const RectSPX& rect )
	{
		if( _pSlot == &mainSlot )
			_onRequestRender( rect + m_skin.contentOfs(m_scale, m_state), 0 );		//TODO: Take padding into account
		else
		{
			Slot * pSlot = reinterpret_cast<Slot*>(_pSlot);
			_onRequestRender( rect + pSlot->m_geo.pos(), pSlot );
		}
	}
/*
	//____ _childRequestResize() _______________________________________________

	void Overlay::_childRequestResize( StaticSlot * pSlot )
	{
		_requestResize();			//TODO: Smarter handling, not request resize unless we need to.
	}
*/
	//____ _prevChild() ________________________________________________________

	Widget * Overlay::_prevChild( const StaticSlot * pSlot ) const
	{
		if( pSlot == &mainSlot )
			pSlot = _endOverlaySlots();

		if (pSlot == _beginOverlaySlots())
			return nullptr;

		Slot * p = _decOverlaySlot((Slot*)pSlot,_sizeOfOverlaySlot());
		return p->_widget();
	}

	//____ _nextChild() ________________________________________________________

	Widget * Overlay::_nextChild( const StaticSlot * pSlot ) const
	{
		if (pSlot == &mainSlot)
			return nullptr;

		Slot * p = _incOverlaySlot((Slot*)pSlot, _sizeOfOverlaySlot());
		if (p < _endOverlaySlots())
			return p->_widget();

		return mainSlot._widget();
	}


} // namespace wg
