/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_flexpanel.h>
#include <wg_patches.h>
#include <wg_util.h>

#include <wg_dynamicslotvector.impl.h>
#include <wg_panel.impl.h>

#include <assert.h>

namespace wg
{
	using namespace Util;

	template class DynamicSlotVector<FlexPanelSlot>;
	template class PanelTemplate<FlexPanelSlot>;

	const TypeInfo FlexPanel::TYPEINFO = { "FlexPanel", &Panel::TYPEINFO };
	const TypeInfo FlexPanelSlot::TYPEINFO = { "FlexPanelSlot", &PanelSlot::TYPEINFO };

	float	FlexPos::s_origoTab[Placement_size][2] = {	-1.f, -1.f,			// Signals undefined.
														0.f, 0.f,
														0.5f, 0.f,
														1.f, 0.f,
														1.f, 0.5f,
														1.f, 1.f,
														0.5f, 1.f,
														0.f, 1.f,
														0.f, 0.5f,
														0.5f, 0.5f };



	//____ FlexPanelSlot::_setBlueprint() ________________________________________

	bool FlexPanelSlot::_setBlueprint( const Blueprint& bp )
	{
		if(bp.pin1.origo.x >= 0.f && bp.pin2.origo.x >= 0.f )
		{
			m_bPinned = true;
			m_topLeftPin = bp.pin1;
			m_bottomRightPin = bp.pin2;
		}
		else
		{
			m_placementGeo = Rect( bp.pos, bp.size );
			m_origo = bp.origo;
			m_hotspot = bp.hotspot.origo.x < 0 ? bp.origo : bp.hotspot;

		}
		
		m_bVisible = bp.visible;
		return true;
	}


	//____ FlexPanelSlot::setPinned() _______________________________________________

	void FlexPanelSlot::setPinned()
	{
		if (!m_bPinned)
		{
			m_bPinned = true;

			SizeSPX sz = static_cast<FlexPanel*>(_holder())->_size();

			m_topLeftPin = FlexPos(m_geo.x / (float)sz.w, m_geo.y / (float)sz.h);
			m_bottomRightPin = FlexPos(m_geo.right() / (float)sz.w, m_geo.bottom() / (float)sz.h);

			static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		}
	}

	void FlexPanelSlot::setPinned(const FlexPos& topLeft, const FlexPos& bottomRight)
	{
		m_bPinned = true;
		m_topLeftPin = topLeft;
		m_bottomRightPin = bottomRight;

		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
	}

	//____ FlexPanelSlot::setMovable() _______________________________________________

	void FlexPanelSlot::setMovable(const FlexPos& origo, const FlexPos& hotspot)
	{
		if (m_bPinned)
		{
			int scale = _holder()->_scale();
			Rect geo = spxToPts(m_geo, scale);


			m_bPinned = false;
			m_origo = origo;
			m_hotspot = hotspot.origo.x < 0 ? origo : hotspot;
			m_placementGeo = geo - origo.pos(static_cast<FlexPanel*>(_holder())->size()) + hotspot.pos(geo);

			static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		}
	}

	void FlexPanelSlot::setMovable(const Rect& geometry, const FlexPos& origo, const FlexPos& hotspot)
	{
		m_bPinned = false;
		m_origo = origo;
		m_hotspot = hotspot.origo.x < 0 ? origo : hotspot;
		m_placementGeo = geometry;

		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
	}

	//____ FlexPanelSlot::setOrigo() __________________________________________________

	bool FlexPanelSlot::setOrigo(const FlexPos& origo)
	{
		if (m_bPinned)
			return false;

		m_origo = origo;
		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		return true;
	}

	//____ FlexPanelSlot::setHotspot() _________________________________________________

	bool FlexPanelSlot::setHotspot(const FlexPos& hotspot)
	{
		if (m_bPinned)
			return false;

		m_hotspot = hotspot;
		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		return true;
	}

	//____ FlexPanelSlot::setGeo() ____________________________________________________

	bool FlexPanelSlot::setGeo(const Rect& geometry)
	{
		if (m_bPinned)
			return false;

		m_placementGeo = geometry;
		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		return true;
	}

	//____ FlexPanelSlot::setOffset() __________________________________________________

	bool FlexPanelSlot::setOffset(const Coord& ofs)
	{
		if (m_bPinned)
			return false;

		m_placementGeo.setPos(ofs);
		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		return true;
	}

	//____ FlexPanelSlot::setSize() _____________________________________________________

	bool FlexPanelSlot::setSize(const Size& size)
	{
		if (m_bPinned)
			return false;

		m_placementGeo.setSize(size);
		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		return true;
	}

	//____ FlexPanelSlot::move() _______________________________________________________

	bool FlexPanelSlot::move(const Coord& ofs)
	{
		if (m_bPinned)
			return false;

		m_placementGeo += ofs;
		static_cast<FlexPanel*>(_holder())->_refreshRealGeo(this);
		return true;
	}

	//____ constructor ____________________________________________________________

	FlexPanel::FlexPanel()
	{
		m_bSiblingsOverlap = true;
	}

	//____ Destructor _____________________________________________________________

	FlexPanel::~FlexPanel()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlexPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setEdgePolicy() ___________________________________________________

	void FlexPanel::setEdgePolicy( EdgePolicy policy )
	{
		if( policy != m_edgePolicy )
		{
			m_edgePolicy = policy;

			auto p = slots.begin();

			while( p < slots.end() )
			{
				_refreshRealGeo( p, false, false );
				p++;
			}
			
			_refreshOverflow();
		}
	}

	//____ setDefaultSize() __________________________________________________________

	void FlexPanel::setDefaultSize( Size size )
	{
		if( size != m_defaultSize )
		{
			m_defaultSize = size;
			_requestResize();
		}
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX FlexPanel::_defaultSize(int scale) const
	{
		if( !m_defaultSize.isEmpty() )
			return align(ptsToSpx(m_defaultSize, scale));

		SizeSPX minSize;

		auto p = slots.begin();
		while( p < slots.end() )
		{
			minSize = SizeSPX::max(minSize,_sizeNeededForGeo(p, scale));
			p++;
		}
		return minSize;
	}


	//____ _didMoveSlots() ___________________________________________________________

	void FlexPanel::_didMoveSlots(StaticSlot * _pFrom, StaticSlot * _pTo, int nb)
	{
		if (nb > 1)
		{
			_requestRender();	//TODO: Optimize! Correctly calculate what is dirty even if more than one is moved.
			return;
		}

		auto pFrom = static_cast<FlexPanelSlot*>(_pFrom);
		auto pTo = static_cast<FlexPanelSlot*>(_pTo);

		if (pTo->m_bVisible)		// This is correct, we have already switched places...
		{
			FlexPanelSlot * pBeg, * pEnd;
			
			if (pTo < pFrom)			// We were moved forward
			{
				pBeg = pTo+1;
				pEnd = pFrom+1;
			}
			else
			{
				pBeg = pFrom;
				pEnd = pTo;
			}
				
			for( auto p = pBeg ; p < pEnd ; p++ )
			{
				if (p->m_bVisible)
				{
					RectSPX cover = RectSPX::overlap(pTo->m_geo + pTo->_widget()->_overflow(), p->m_geo + p->_widget()->_overflow());
					
					if(!cover.isEmpty())
						_onRequestRender(cover, pTo);
				}
			}
		}
	}

	//____ _didAddSlots() _____________________________________________________________

	void FlexPanel::_didAddSlots( StaticSlot * _pSlot, int nb )
	{
		auto pSlot = static_cast<FlexPanelSlot*>(_pSlot);

		for( int i = 0 ; i < nb ; i++ )
		{
			if( pSlot[i].m_bVisible == true )
			{
				_refreshRealGeo(&pSlot[i]);
				
				RectSPX geo = pSlot[i]._widget()->_hasOverflow() ? pSlot[i].m_geo + pSlot[i]._widget()->_overflow() : pSlot[i].m_geo;
				_onRequestRender(geo, &pSlot[i]);
			}
		}
	}

	//____ _willEraseSlots() _____________________________________________________________

	void FlexPanel::_willEraseSlots( StaticSlot * _pSlot, int nb )
	{
		auto pSlot = static_cast<FlexPanelSlot*>(_pSlot);
		_hideSlots(pSlot,nb);
	}

	//____ _hideSlots() _____________________________________________________________

	void FlexPanel::_hideSlots( StaticSlot * _pSlot, int nb )
	{
		auto pSlot = static_cast<FlexPanelSlot*>(_pSlot);
		
		bool bHideOverflow = false;
		
		for( int i = 0 ; i < nb ; i++ )
		{
			if( pSlot[i].m_bVisible == true )
			{
				RectSPX area = pSlot[i].m_geo;
				if(pSlot[i]._widget()->_hasOverflow())
				{
					area += pSlot[i]._widget()->_overflow();
					bHideOverflow = true;
				}
				_onRequestRender(area, &pSlot[i]);
				
				pSlot[i].m_bVisible = false;					// Needs to be done AFTER _onRequestRender()!
			}
		}
		
		if( bHideOverflow )
		{
			if( nb == 1 )
				_removeChildOverflow(pSlot->m_geo, pSlot->_widget()->_overflow());
			else
				_refreshOverflow();
		}
	}

	//____ _unhideSlots() _____________________________________________________________

	void FlexPanel::_unhideSlots( StaticSlot * _pSlot, int nb )
	{
		auto pSlot = static_cast<FlexPanelSlot*>(_pSlot);

		bool bUnhideOverflow = false;
		
		for( int i = 0 ; i < nb ; i++ )
		{
			if( pSlot[i].m_bVisible == false )
			{
				pSlot[i].m_bVisible = true;
				_refreshRealGeo(&pSlot[i], false, false );
				
				RectSPX area = pSlot[i].m_geo;
				if(pSlot[i]._widget()->_hasOverflow())
				{
					area += pSlot[i]._widget()->_overflow();
					bUnhideOverflow = true;
				}
				
				_onRequestRender(area, &pSlot[i]);
			}
		}
		
		if( bUnhideOverflow )
		{
			if( nb == 1 )
				_addChildOverflow(pSlot->m_geo, pSlot->_widget()->_overflow());
			else
				_refreshOverflow();
		}
	}


	//____ _onRequestRender() ______________________________________________________

	void FlexPanel::_onRequestRender( const RectSPX& _rect, const FlexPanelSlot * pSlot )
	{
		if( !pSlot->m_bVisible )
			return;

		// Clip our geometry and put it in a dirtyrect-list

		PatchesSPX patches;
		
		RectSPX rect  = m_edgePolicy == EdgePolicy::Confine ? RectSPX::overlap(_rect, RectSPX(0, 0, m_size)) : _rect;
		patches.add( rect );

		// Remove portions of patches that are covered by upper siblings opaque areas

		for(auto pCover = slots.begin() ; pCover < pSlot ; pCover++ )
		{
			if( pCover->m_bVisible && (pCover->m_geo + pCover->_widget()->_overflow()).isOverlapping( rect ) )
				pCover->_widget()->_maskPatches( patches, pCover->m_geo, RectSPX(0,0,65536,65536 ) );
		}

		// Make request render calls

		for( const RectSPX * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
			_requestRender( * pRect );
	}

	//____ _resize() ____________________________________________________________

	void FlexPanel::_resize( const SizeSPX& size, int scale )
	{
		PanelTemplate::_resize(size,scale);

		auto p = slots.begin();
		while( p < slots.end() )
		{
			_refreshRealGeo(p, false, false);
			p++;
		}

		_refreshOverflow();
	}

	//____ _childRequestRender() _________________________________________________

	void FlexPanel::_childRequestRender( StaticSlot * _pSlot, const RectSPX& rect )
	{
		auto pSlot = static_cast<FlexPanelSlot*>(_pSlot);
		_onRequestRender( rect + pSlot->m_geo.pos(), pSlot );
	}

	//____ _childRequestResize() _________________________________________________

	void FlexPanel::_childRequestResize( StaticSlot * _pSlot )
	{
		auto pSlot = static_cast<FlexPanelSlot*>(_pSlot);
		_refreshRealGeo(pSlot, true);
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	FlexPanel::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		return FlexPanelSlot::TYPEINFO;
	}

	//____ _releaseChild() ____________________________________________________

	void FlexPanel::_releaseChild(StaticSlot * pSlot)
	{
		slots.erase(static_cast<FlexPanelSlot*>(pSlot));
	}

	//____ _replaceChild() _____________________________________________________

	void FlexPanel::_replaceChild(StaticSlot * _pSlot, Widget * pNewChild)
	{
		if( pNewChild == nullptr )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Slot in FlexPanel must contain pointer to widget and not nullptr.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		int index = slots._releaseUpdateIndex(pNewChild, int(static_cast<FlexPanelSlot*>(_pSlot) - slots.begin()));
		auto& slot = slots.at(index);

		slot._setWidget(pNewChild);

		if (slot.m_bVisible )
		{
			_refreshRealGeo(&slot, true);
			_onRequestRender(slot.m_geo + slot._widget()->_overflow(), &slot);	// In case geo was not refreshed, thus no request render from _refreshRealGeo().
		}
	}

	//____ _refreshRealGeo() ___________________________________________

	void FlexPanel::_refreshRealGeo( FlexPanelSlot * pSlot, bool bForceResize, bool bUpdateOverflow )
	{
		RectSPX	geo;

		if( pSlot->m_bPinned )
		{
			CoordSPX topLeft = pSlot->m_topLeftPin._pos( m_size, m_scale );
			CoordSPX bottomRight = pSlot->m_bottomRightPin._pos( m_size, m_scale );

			geo = RectSPX(topLeft,bottomRight);

			// Respect widgets limits, apply in such a way that rectangle centers in specified rectangle

			SizeSPX sz = geo.size();
			sz.limit( pSlot->_widget()->_minSize(m_scale), pSlot->_widget()->_maxSize(m_scale) );
			if( sz != geo.size() )
			{
				geo.x += (geo.w - sz.w) / 2;
				geo.y += (geo.h - sz.h) / 2;
				geo.w = sz.w;
				geo.h = sz.h;
			}
		}
		else
		{
			// Calculate size

			SizeSPX sz;
			if (pSlot->m_placementGeo.w > 0 && pSlot->m_placementGeo.h > 0)
				sz = align(ptsToSpx(pSlot->m_placementGeo.size(), m_scale));
			else if (pSlot->m_placementGeo.w == 0 && pSlot->m_placementGeo.h == 0)
				sz = pSlot->_widget()->_defaultSize(m_scale);
			else
			{
				if (pSlot->m_placementGeo.h == 0)
				{
					sz.w = align(ptsToSpx(pSlot->m_placementGeo.w, m_scale));
					sz.h = pSlot->_widget()->_matchingHeight(sz.w, m_scale);
				}
				else
				{
					sz.h = align(ptsToSpx(pSlot->m_placementGeo.h, m_scale));
					sz.w = pSlot->_widget()->_matchingWidth(sz.h, m_scale);
				}
			}

			sz.limit( pSlot->_widget()->_minSize(m_scale), pSlot->_widget()->_maxSize(m_scale) );		// Respect widgets limits.

			// Calculate position

			CoordSPX pos = pSlot->m_origo._pos( m_size, m_scale );		// Placement,
			pos -= pSlot->m_hotspot._pos(sz, m_scale);				// hotspot
			pos += ptsToSpx(pSlot->m_placementGeo.pos(),m_scale);				// and Offset.

			// Limit size/pos according to parent

			if( m_edgePolicy == EdgePolicy::Confine )
			{
				if (sz.w > m_size.w)
				{
					sz.w = m_size.w;
					if( pSlot->m_placementGeo.h == 0 )
						sz.h = pSlot->_widget()->_matchingHeight(sz.w, m_scale);
				}
				if (sz.h > m_size.h)
				{
					sz.h = m_size.h;
					if (pSlot->m_placementGeo.w == 0)
						sz.w = std::min(pSlot->_widget()->_matchingWidth(sz.h, m_scale),m_size.w);
				}

				limit(pos.x, 0, m_size.w - sz.w);
				limit(pos.y, 0, m_size.h - sz.h);
			}

			geo = RectSPX( pos, sz );
		}

		RectSPX newGeo = align(geo);
		RectSPX oldGeo = pSlot->m_geo;
		
		// Request render and update positions.

		if (newGeo != oldGeo)
		{
			pSlot->m_geo = newGeo;

			RectSPX oldRenderArea = oldGeo;
			RectSPX newRenderArea = newGeo;

			if( pSlot->_widget()->_hasOverflow() )
			{
				BorderSPX overflow = pSlot->_widget()->_overflow();
				oldRenderArea += overflow;
				newRenderArea += overflow;
			}
			

			if( m_edgePolicy == EdgePolicy::Clip )
			{
				RectSPX myGeo = m_size;
				oldRenderArea = RectSPX::overlap(oldRenderArea, myGeo);
				newRenderArea = RectSPX::overlap(newRenderArea, myGeo);
			}
			else if( bUpdateOverflow && pSlot->_widget()->_hasOverflow() )
				_moveChildOverflow(pSlot, oldGeo, newGeo);

			_onRequestRender(oldRenderArea, pSlot);
			_onRequestRender(newRenderArea, pSlot);
		}

		if (bForceResize || pSlot->_widget()->_size() != newGeo.size() || pSlot->_widget()->_scale() != m_scale )
			pSlot->_setSize(newGeo, m_scale);
	}

	//____ _sizeNeededForGeo() ________________________________________

	SizeSPX FlexPanel::_sizeNeededForGeo( FlexPanelSlot * pSlot, int scale ) const
	{
		SizeSPX sz;

		if( pSlot->m_bPinned )
		{
			sz = pSlot->_widget()->_defaultSize(scale);

			sz += SizeSPX( pSlot->m_topLeftPin.offset.x, pSlot->m_topLeftPin.offset.y );
			sz -= SizeSPX( pSlot->m_bottomRightPin.offset.x, pSlot->m_bottomRightPin.offset.y );

			auto dividerX = (pSlot->m_bottomRightPin.origo.x - pSlot->m_topLeftPin.origo.x);
			auto dividerY = (pSlot->m_bottomRightPin.origo.y - pSlot->m_topLeftPin.origo.y);

			if( dividerX > 0.f )
				sz.w /= dividerX;

			if( dividerY > 0.f )
				sz.h /= dividerY;
		}
		else
		{
			RectSPX geo = ptsToSpx(pSlot->m_placementGeo,scale);

			CoordSPX hotspot = pSlot->m_hotspot._pos(geo.size(), scale);
			CoordSPX offset = geo.pos() + ptsToSpx(pSlot->m_origo.offset,scale) - hotspot;

			spx leftOfOrigo = 0 - offset.x;
			spx rightOfOrigo = offset.x + geo.w;
			spx aboveOrigo = 0 - offset.y;
			spx belowOrigo = offset.y + geo.h;

			if( leftOfOrigo > 0 )
				sz.w = leftOfOrigo / pSlot->m_origo.origo.x;

			if( rightOfOrigo > 0 )
			{
				int w = rightOfOrigo / (1.f - pSlot->m_origo.origo.x);
				if( sz.w < w )
					sz.w = w;
			}

			if( aboveOrigo > 0 )
				sz.h = aboveOrigo / pSlot->m_origo.origo.y;

			if( belowOrigo > 0 )
			{
				int h = belowOrigo / (1.f - pSlot->m_origo.origo.y);
				if( sz.h < h )
					sz.h = h;
			}
		}

		return align(sz);
	}



} // namespace wg
