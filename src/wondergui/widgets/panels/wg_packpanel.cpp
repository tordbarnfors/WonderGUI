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

#include <initializer_list>

#include <wg_packpanel.h>
#include <wg_base.h>

#include <wg_dynamicslotvector.impl.h>
#include <wg_panel.impl.h>


namespace wg
{
	using namespace Util;

	template class DynamicSlotVector<PackPanelSlot>;
	template class PanelTemplate<PackPanelSlot>;

	const TypeInfo PackPanel::TYPEINFO = { "PackPanel", &Panel::TYPEINFO };
	const TypeInfo PackPanelSlot::TYPEINFO = { "PackPanelSlot", &PanelSlot::TYPEINFO };


	bool PackPanelSlot::_setBlueprint( const Blueprint& bp )
	{
		m_weight = bp.weight;
		m_baseline = bp.baseline;
		m_bVisible = bp.visible;
		return true;
	}

	void PackPanelSlot::setWeight(float weight)
	{
		if (weight != m_weight)
			static_cast<PackPanel*>(_holder())->_reweightSlots(this, 1, weight);
	}


	void PackPanelSlot::setBaseline(float baseline)
	{
		if (baseline != m_baseline)
			static_cast<PackPanel*>(_holder())->_setBaselines(this, 1, baseline);
	}


	//____ constructor ____________________________________________________________

	PackPanel::PackPanel()
	{
		m_bSiblingsOverlap = false;
	}

	//____ Destructor _____________________________________________________________

	PackPanel::~PackPanel()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAxis() ______________________________________________________

	void PackPanel::setAxis( Axis axis )
	{
		if( m_axis != axis )
		{
			m_axis = axis;
			_refreshGeometries();

		}
	}

	//____ setLayout() _______________________________________________________

	void PackPanel::setLayout( PackLayout * pLayout )
	{
		if( m_pLayout.rawPtr() != pLayout )
		{
			m_pLayout = pLayout;
			_refreshGeometries();

		}
	}

	//____ setSpacing() _______________________________________________________

	void PackPanel::setSpacing(pts before, pts between, pts after)
	{
		if (before == m_spacingBefore && between == m_spacingBetween && after == m_spacingAfter)
			return;

		m_spacingBefore = before;
		m_spacingBetween = between;
		m_spacingAfter = after;
		m_totalSpacing = _calcTotalSpacing(m_scale);
		_refreshGeometries();
	}

	void PackPanel::setSpacing(pts between)
	{
		if (0 == m_spacingBefore && between == m_spacingBetween && 0 == m_spacingAfter)
			return;

		m_spacingBefore = 0;
		m_spacingBetween = between;
		m_spacingAfter = 0;
		m_totalSpacing = _calcTotalSpacing(m_scale);
		_refreshGeometries();
	}

	//____ setSlotAlignment() ____________________________________________________

	void PackPanel::setSlotAlignment( Alignment alignment )
	{
		if( alignment != m_slotAlignment )
		{
			m_slotAlignment = alignment;
			_realignChildGeo(true);
		}
	}

	//____ setWeight() ___________________________________________________________

	bool PackPanel::setSlotWeight(int index, int amount, float weight)
	{
		if (index < 0 || amount <= 0 || index + amount > slots.size() || weight < 0.f)
			return false;

		_reweightSlots( &slots[index], amount, weight);
		return true;
	}

	bool PackPanel::setSlotWeight(iterator beg, iterator end, float weight)
	{
		if (weight < 0.f)
			return false;

		_reweightSlots(beg, int(end - beg), weight);
		return true;
	}

	bool PackPanel::setSlotWeight(int index, int amount, std::initializer_list<float> weights)
	{
		if (index < 0 || amount <= 0 || index + amount > slots.size() || amount >(int) weights.size())
			return false;

		_reweightSlots(&slots[index], amount, weights.begin());
		return true;
	}

	bool PackPanel::setSlotWeight(iterator beg, iterator end, std::initializer_list<float> weights)
	{
		if( weights.size() < (end - beg) )
			return false;

		_reweightSlots(beg, int(end - beg), weights.begin());
		return true;
	}

	//____ setSlotBaseline() ___________________________________________________________

	bool PackPanel::setSlotBaseline(int index, int amount, float baseline)
	{
		if (index < 0 || amount <= 0 || index + amount > slots.size() || baseline < 0.f  || baseline > 1.f)
			return false;

		_setBaselines( &slots[index], amount, baseline);
		return true;
	}

	bool PackPanel::setSlotBaseline(iterator beg, iterator end, float baseline)
	{
		if (baseline < 0.f  || baseline > 1.f)
			return false;

		_setBaselines(beg, int(end - beg), baseline);
		return true;
	}

	bool PackPanel::setSlotBaseline(int index, int amount, std::initializer_list<float> baselines)
	{
		if (index < 0 || amount <= 0 || index + amount > slots.size() || amount >(int) baselines.size())
			return false;

		_setBaselines(&slots[index], amount, baselines.begin());
		return true;
	}

	bool PackPanel::setSlotBaseline(iterator beg, iterator end, std::initializer_list<float> baselines)
	{
		if( baselines.size() < (end - beg) )
			return false;

		_setBaselines(beg, int(end - beg), baselines.begin());
		return true;
	}

	//____ _defaultSize() _______________________________________________________

	SizeSPX PackPanel::_defaultSize(int scale) const
	{
		if (scale != m_scale)
		{
			spx dummy1, dummy2;

			return _calcDefaultContentSize(scale, dummy1, dummy2) + m_skin.contentBorderSize(scale);
		}
		else
			return m_defaultContentSize + m_skin.contentBorderSize(scale);
	}

	//____ _matchingHeight() _______________________________________________________

	spx PackPanel::_matchingHeight( spx width, int scale ) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale);

		if( padding.w >= width )
			return padding.h;

		width -= padding.w;
		spx height = 0;

		if (slots.size() > 0)
		{
			auto pLayout = m_pLayout ? m_pLayout : Base::defaultPackLayout();

			if( m_axis == Axis::X )
			{
				width -= scale == m_scale ? m_totalSpacing : _calcTotalSpacing(scale);

				// Allocate and populate Layout array

				int arrayBytes = (sizeof(PackLayout::Item) + sizeof(spx)) * slots.size();
				PackLayout::Item * pItemArea = reinterpret_cast<PackLayout::Item*>(Base::memStackAlloc(arrayBytes));
				spx* pOutput = (spx*)&pItemArea[slots.size()];

				int nItems = _populateLayoutArray(pItemArea, scale);

				// Retrieve item lengths and find height of highest item.

				pLayout->getItemSizes(pOutput, width, scale, nItems, pItemArea);

				spx maxAscend = 0;
				spx maxDescend = 0;

				for( auto pS = slots.begin() ; pS != slots.end() ; pS++ )
				{
					if( pS->m_bVisible )
					{
						spx breadth = pS->_widget()->_matchingHeight( *pOutput, scale );
						spx ascend = align(breadth * pS->m_baseline);
						spx descend = breadth - ascend;

						if( ascend > maxAscend )
							maxAscend = ascend;

						if( descend > maxDescend )
							maxDescend = descend;

						pOutput++;
					}
				}

				height = maxAscend + maxDescend;

				// Release temporary memory area

				Base::memStackFree(arrayBytes);
			}
			else
			{
				if( pLayout && pLayout->doesCalcWantedLength() )
				{
					// Allocate and populate SizeBroker array

					int arrayBytes = (sizeof(PackLayout::Item) + sizeof(spx)) * slots.size();
					PackLayout::Item * pItemArea = reinterpret_cast<PackLayout::Item*>(Base::memStackAlloc(arrayBytes));
					spx* pOutput = (spx*)&pItemArea[slots.size()];

					int nItems = _populateLayoutArray(pItemArea, scale, width);

					// Retrieve default length

					height = pLayout->getWantedSizes(pOutput, scale, nItems, pItemArea);

					// Release temporary memory area

					Base::memStackFree(arrayBytes);
				}
				else
				{
					for( auto pS = slots.begin() ; pS != slots.end() ; pS++ )
					{
						if( pS->m_bVisible )
							height += pS->_widget()->_matchingHeight( width, scale );
					}
				}

				height += scale == m_scale ? m_totalSpacing : _calcTotalSpacing(scale);
			}
		}

		height += padding.h;

		return height;
	}

	//____ _matchingWidth() _______________________________________________________

	spx PackPanel::_matchingWidth( spx height, int scale ) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale);

		if( padding.h >= height )
			return padding.w;

		height -= padding.h;
		spx width = 0;

		if (slots.size() > 0)
		{
			auto pLayout = m_pLayout ? m_pLayout : Base::defaultPackLayout();

			if( m_axis == Axis::Y )
			{
				height -= scale == m_scale ? m_totalSpacing : _calcTotalSpacing(scale);

				{
					// Allocate and populate SizeBroker array

					int arrayBytes = (sizeof(PackLayout::Item)+sizeof(spx)) * slots.size();
					PackLayout::Item* pItemArea = reinterpret_cast<PackLayout::Item*>(Base::memStackAlloc(arrayBytes));
					spx* pOutput = (spx*)&pItemArea[slots.size()];

					int nItems = _populateLayoutArray(pItemArea, scale);

					// Retrieve item lengths and find height of highest item.


					pLayout->getItemSizes(pOutput, height, scale, nItems, pItemArea);

					spx maxAscend = 0;
					spx maxDescend = 0;

					for( auto pS = slots.begin() ; pS != slots.end() ; pS++ )
					{
						if( pS->m_bVisible )
						{
							spx breadth = pS->_widget()->_matchingWidth( *pOutput, scale );
							spx ascend = align(breadth * pS->m_baseline);
							spx descend = breadth - ascend;

							if( ascend > maxAscend )
								maxAscend = ascend;

							if( descend > maxDescend )
								maxDescend = descend;

							pOutput++;
						}
					}

					width = maxAscend + maxDescend;

					// Release temporary memory area

					Base::memStackFree(arrayBytes);
				}
			}
			else
			{
				if( pLayout && pLayout->doesCalcWantedLength() )
				{
					// Allocate and populate SizeBroker array

					int arrayBytes = (sizeof(PackLayout::Item) + sizeof(spx)) * slots.size();
					PackLayout::Item* pItemArea = reinterpret_cast<PackLayout::Item*>(Base::memStackAlloc(arrayBytes));
					spx* pOutput = (spx*)&pItemArea[slots.size()];

					int nItems = _populateLayoutArray(pItemArea, scale, height);

					// Retrieve preferred length

					width = pLayout->getWantedSizes(pOutput, scale, nItems, pItemArea);

					// Release temporary memory area

					Base::memStackFree(arrayBytes);
				}
				else
				{
					for( auto pS = slots.begin() ; pS != slots.end() ; pS++ )
					{
						if( pS->m_bVisible )
							width += pS->_widget()->_matchingWidth( height, scale );
					}
				}

				width += scale == m_scale ? m_totalSpacing : _calcTotalSpacing(scale);
			}
		}

		width += padding.w;

		return width;
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	PackPanel::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		return PackPanelSlot::TYPEINFO;
	}


	//____ _didAddSlots() _____________________________________________________

	void PackPanel::_didAddSlots(StaticSlot * _pSlot, int nb)
	{
		auto pSlot = static_cast<PackPanelSlot*>(_pSlot);

		for (int i = 0; i < nb; i++)
		{
			if (pSlot[i].m_bVisible == true)
				pSlot[i].m_defaultSize = pSlot[i]._widget()->_defaultSize(m_scale);
		}

		m_totalSpacing = _calcTotalSpacing(m_scale);
		_refreshGeometries();
	}

	//____ _didMoveSlots() _____________________________________________________

	void PackPanel::_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb)
	{
		//TODO: Optimize! Only update and re-render what is needed, but take
		// into account that SizeBroker might have weird rules and might affect
		// sizes in various ways when children change place...

		_refreshGeometries();
	}

	//____ _willEraseSlots() _________________________________________________

	void PackPanel::_willEraseSlots(StaticSlot * pSlot, int nb)
	{
		_hideChildren((PackPanelSlot*) pSlot, nb);
	}

	//____ _hideSlots() _______________________________________________________

	void PackPanel::_hideSlots(StaticSlot * pSlot, int nb)
	{
		_hideChildren((PackPanelSlot*) pSlot, nb);
	}

	//____ _unhideSlots() _____________________________________________________

	void PackPanel::_unhideSlots(StaticSlot * pSlot, int nb)
	{
		_unhideChildren((PackPanelSlot*) pSlot, nb);
	}

	//____ _reweightSlots() ______________________________________________________

	void PackPanel::_reweightSlots(PackPanelSlot * pSlot, int nb, float weight)
	{
		bool bModified = false;

		for (int i = 0; i < nb; i++)
		{
			if (pSlot[i].m_weight != weight)
			{
				bModified = true;
				pSlot[i].m_weight = weight;
			}
		}

		if (bModified)
		{
			auto pLayout = m_pLayout ? m_pLayout : Base::defaultPackLayout();
			if (pLayout->doesCalcWantedLength())
				_refreshGeometries();
			else
				_refreshChildGeo(true);
		}
	}

	void PackPanel::_reweightSlots(PackPanelSlot * pSlot, int nb, const float * pWeights)
	{
		bool bModified = false;

		for (int i = 0; i < nb; i++)
		{
			if (pSlot[i].m_weight != *pWeights)
			{
				bModified = true;
				pSlot[i].m_weight = * pWeights;
			}
			pWeights++;
		}

		if (bModified)
		{
			auto pLayout = m_pLayout ? m_pLayout : Base::defaultPackLayout();
			if (pLayout->doesCalcWantedLength())
				_refreshGeometries();
			else
				_refreshChildGeo(true);
		}
	}


	//____ _setBaselines() ______________________________________________________

	void PackPanel::_setBaselines(PackPanelSlot * pSlot, int nb, float baseline)
	{
		bool bModified = false;

		for (int i = 0; i < nb; i++)
		{
			if (pSlot[i].m_baseline != baseline)
			{
				bModified = true;
				pSlot[i].m_baseline = baseline;
			}
		}

		if (bModified)
		{
			_refreshGeometries();
			_realignChildGeo(true);
		}
	}

	void PackPanel::_setBaselines(PackPanelSlot * pSlot, int nb, const float * pBaselines)
	{
		bool bModified = false;

		for (int i = 0; i < nb; i++)
		{
			if (pSlot[i].m_baseline != *pBaselines)
			{
				bModified = true;
				pSlot[i].m_baseline = * pBaselines;
			}
			pBaselines++;
		}

		if (bModified)
		{
			_refreshGeometries();
			_realignChildGeo(true);
		}
	}

	//____ _childRequestResize() ______________________________________________

	void PackPanel::_childRequestResize(StaticSlot * _pSlot)
	{
		// Update cached preferred size of child

		auto pSlot = static_cast<PackPanelSlot*>(_pSlot);

		if( pSlot->m_bVisible )
		{
			pSlot->m_defaultSize = pSlot->_widget()->_defaultSize(m_scale);
			pSlot->m_bResizeRequired = true;
			_refreshGeometries();
		}

	}

	//____ _releaseChild() ____________________________________________________

	void PackPanel::_releaseChild(StaticSlot * pSlot)
	{
		slots.erase(static_cast<PackPanelSlot*>(pSlot));
	}

	//____ _replaceChild() _____________________________________________________

	void PackPanel::_replaceChild(StaticSlot * _pSlot, Widget * pNewChild)
	{
		if( pNewChild == nullptr )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Slot in PackPanel must contain pointer to widget and not nullptr.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		int index = slots._releaseUpdateIndex(pNewChild, int(static_cast<PackPanelSlot*>(_pSlot) - slots.begin()));
		auto& slot = slots.at(index);

		Widget_p pOldChild = slot._widget();

		slot._setWidget(pNewChild);

		if (slot.m_bVisible)
		{
			slot.m_defaultSize = slot._widget()->_defaultSize(m_scale);
			slot._widget()->_resize(slot.m_geo.size(), m_scale);

			RectSPX renderArea = slot.m_geo + pNewChild->_overflow();
			if (pOldChild)
				renderArea.growToContain(slot.m_geo + pOldChild->_overflow() );
			_requestRender(renderArea);

			m_totalSpacing = _calcTotalSpacing(m_scale);
			_refreshGeometries();
		}

	}


	//____ _unhideChildren() _______________________________________________________

	void PackPanel::_unhideChildren(PackPanelSlot * pSlot, int nb)
	{
		for (int i = 0; i < nb; i++)
		{
			if (pSlot[i].m_bVisible == false)
			{
				pSlot[i].m_bVisible = true;
				pSlot[i].m_defaultSize = pSlot[i]._widget()->_defaultSize(m_scale);
			}
		}

		m_totalSpacing = _calcTotalSpacing(m_scale);
		_refreshGeometries();
	}

	//____ _hideChildren() _______________________________________________________

	void PackPanel::_hideChildren(PackPanelSlot * pSlot, int nb)
	{
		for (int i = 0; i < nb; i++)
			pSlot[i].m_bVisible = false;

		m_totalSpacing = _calcTotalSpacing(m_scale);
		_refreshGeometries();
	}


	//____ _onWidgetsReordered() ____________________________________________________
/*
	void PackPanel::_onWidgetsReordered()
	{
		_refreshChildGeo();
	}
*/
	//____ _refreshGeometries() _____________________________________________________

	void PackPanel::_refreshGeometries()
	{
		// Recalculate preferred sizes for widget and content.

		SizeSPX newDefaultContentSize = _calcDefaultContentSize(m_scale, m_maxAscend, m_maxDescend);
		SizeSPX newDefaultSize = newDefaultContentSize + m_skin.contentBorderSize(m_scale);

		// request resize or just refresh child geo, depending on what is needed.

		if (newDefaultContentSize != m_defaultContentSize || newDefaultSize != m_size)
		{
			m_defaultContentSize = newDefaultContentSize;
			_requestResize();
		}
		else
			_refreshChildGeo(true);
	}


	//____ _resize() ____________________________________________________________

	void PackPanel::_resize( const SizeSPX& size, int scale )
	{
		if( scale != m_scale )
		{
			for (auto pSlot = slots.begin(); pSlot != slots.end(); pSlot++)
			{
				pSlot->m_bResizeRequired = true;
				pSlot->m_defaultSize = pSlot->_widget()->_defaultSize(scale);
			}

			m_totalSpacing = _calcTotalSpacing(scale);

			//TODO: This is slow since we repeat the above loop, but we need to support PackLayout that modifies default size.

			m_defaultContentSize = _calcDefaultContentSize(scale, m_maxAscend, m_maxDescend);
		}

		PanelTemplate::_resize(size, scale);
		_refreshChildGeo(true);

		if (slots.isEmpty())
			_refreshOverflow();
	}

	//____ _calcDefaultContentSize() ______________________________________________________

	SizeSPX PackPanel::_calcDefaultContentSize( int scale, spx& maxAscend, spx& maxDescend ) const
	{
		spx length = 0;

		maxAscend = 0;
		maxDescend = 0;

		if (slots.size() > 0)
		{
			auto pLayout = m_pLayout ? m_pLayout : Base::defaultPackLayout();
			if( pLayout->doesCalcWantedLength() )
			{
				// Allocate and populate SizeBroker array

				int arrayBytes = (sizeof(PackLayout::Item)+sizeof(spx)) * slots.size();
				PackLayout::Item* pItemArea = reinterpret_cast<PackLayout::Item*>(Base::memStackAlloc(arrayBytes));
				spx* pOutput = (spx*) & pItemArea[slots.size()];

				int nItems = _populateLayoutArray(pItemArea, scale);

				// Retrieve preferred length and breadth

				length = pLayout->getWantedSizes(pOutput, scale, nItems, pItemArea);

				PackLayout::Item* pI = pItemArea;
				for (auto pS = slots.begin(); pS != slots.end(); pS++)
				{
					if( pS->m_bVisible )
					{
						spx breadth = m_axis == Axis::X ? pS->_widget()->_matchingHeight(*pOutput, scale) : pS->_widget()->_matchingWidth(*pOutput, scale);

						spx ascend = align(breadth * pS->m_baseline);
						spx descend = breadth - ascend;

						if( ascend > maxAscend )
							maxAscend = ascend;

						if( descend > maxDescend )
							maxDescend = descend;

						pI++;
						pOutput++;
					}
				}

				// Release temporary memory area

				Base::memStackFree(arrayBytes);

			}
			else
			{
				if( scale == m_scale )
				{
					if( m_axis == Axis::X )
					{
						for (auto p = slots.begin(); p != slots.end(); p++)
						{
							if( p->m_bVisible )
							{
								length += p->m_defaultSize.w;

								spx breadth = p->m_defaultSize.h;
								spx ascend = align(breadth * p->m_baseline);
								spx descend = breadth - ascend;

								if( ascend > maxAscend )
									maxAscend = ascend;

								if( descend > maxDescend )
									maxDescend = descend;
							}
						}
					}
					else
					{
						for (auto p = slots.begin(); p != slots.end(); p++)
						{
							if( p->m_bVisible )
							{
								length += p->m_defaultSize.h;
								spx breadth = p->m_defaultSize.w;
								spx ascend = align(breadth * p->m_baseline);
								spx descend = breadth - ascend;

								if( ascend > maxAscend )
									maxAscend = ascend;

								if( descend > maxDescend )
									maxDescend = descend;
							}
						}
					}
				}
				else
				{
					if( m_axis == Axis::X )
					{
						for (auto p = slots.begin(); p != slots.end(); p++)
						{
							if( p->m_bVisible )
							{
								SizeSPX defaultSize = p->_widget()->_defaultSize(scale);
								length += defaultSize.w;

								spx breadth = defaultSize.h;
								spx ascend = align(breadth * p->m_baseline);
								spx descend = breadth - ascend;

								if( ascend > maxAscend )
									maxAscend = ascend;

								if( descend > maxDescend )
									maxDescend = descend;
							}
						}
					}
					else
					{
						for (auto p = slots.begin(); p != slots.end(); p++)
						{
							if( p->m_bVisible )
							{
								SizeSPX defaultSize = p->_widget()->_defaultSize(scale);
								length += defaultSize.h;

								spx breadth = defaultSize.w;
								spx ascend = align(breadth * p->m_baseline);
								spx descend = breadth - ascend;

								if( ascend > maxAscend )
									maxAscend = ascend;

								if( descend > maxDescend )
									maxDescend = descend;
							}
						}
					}
				}
			}

			length += scale == m_scale ? m_totalSpacing : _calcTotalSpacing(scale);
		}

		//

		return m_axis == Axis::X ? SizeSPX(length,maxAscend + maxDescend) : SizeSPX(maxAscend + maxDescend,length);
	}

	//____ _realignChildGeo() _________________________________________________________

	void PackPanel::_realignChildGeo( bool bRequestRender )
	{
		if( slots.isEmpty() )
			return;

		if( m_slotAlignment == Alignment::Justify )
			return;

		SizeSPX sz = m_size - m_skin.contentBorderSize(m_scale);
		CoordSPX contentOfs = m_skin.contentOfs(m_scale, State::Default);
		spx givenBreadth = m_axis == Axis::X ? sz.h : sz.w;

		CoordSPX pos = contentOfs;

		spx baselineOffset = _calcBaselineOffset(givenBreadth);
		if (m_axis == Axis::X)
			baselineOffset += pos.y;
		else
			baselineOffset += pos.x;


		for (auto p = slots.begin(); p != slots.end(); p++)
		{
			if( p->m_bVisible )
			{
				RectSPX renderArea = p->m_geo + p->_widget()->_overflow();

				spx slotOfs = baselineOffset;
				if( m_axis == Axis::X )
				{
					slotOfs -= align(p->m_baseline * p->m_geo.h);

					if( slotOfs != p->m_geo.y )
					{
						RectSPX geo = p->m_geo;
						if( slotOfs < geo.y )
						{
							renderArea.h += geo.y - slotOfs;
							renderArea.y -= geo.y - slotOfs;
						}
						else
						{
							renderArea.h += slotOfs - geo.y;
						}

						p->m_geo.y = slotOfs;
					}
				}
				else
				{
					slotOfs -= align(p->m_baseline * p->m_geo.w);

					if( slotOfs != p->m_geo.x )
					{
						RectSPX geo = p->m_geo;
						if( slotOfs < geo.x )
						{
							renderArea.w += geo.x - slotOfs;
							renderArea.x -= geo.x - slotOfs;
						}
						else
						{
							renderArea.w += slotOfs - geo.x;
						}

						p->m_geo.x = slotOfs;
					}
				}

				if (bRequestRender)
					_requestRender(renderArea);
			}
		}
	}

	//____ _calcBaselineOffset() _______________________________________________________

	spx PackPanel::_calcBaselineOffset(spx givenBreadth)
	{
		switch( m_slotAlignment )
		{
			default:
			case Alignment::Begin:
				return m_maxAscend;
			case Alignment::Center:
				return align((givenBreadth - (m_maxAscend+m_maxDescend))/2 + m_maxAscend);
			case Alignment::End:
				return givenBreadth - m_maxDescend;
			case Alignment::Justify:
				return 0;
		}

		return 0;
	}

	//____ _calcTotalSpacing() _______________________________________________________

	spx PackPanel::_calcTotalSpacing( int scale ) const
	{

		int nVisible = 0;
		for( auto& slot : slots )
		{
			if( slot.m_bVisible)
				nVisible++;
		}

		if( nVisible == 0 )
			return 0;				// No spacing if we have no visible slots.

		return 	align(ptsToSpx(m_spacingBefore, scale)) +
				align(ptsToSpx(m_spacingAfter, scale)) +
				align(ptsToSpx(m_spacingBetween, scale)) * (nVisible - 1);
	}


	//____ _refreshChildGeo() _________________________________________________________

	void PackPanel::_refreshChildGeo( bool bRequestRender )
	{
		if( slots.isEmpty() )
			return;

		SizeSPX sz = m_size - m_skin.contentBorderSize(m_scale);
		CoordSPX contentOfs = m_skin.contentOfs(m_scale, State::Default);			//TODO: Support offset changing in different states.

		spx wantedLength = m_axis == Axis::X ? m_defaultContentSize.w : m_defaultContentSize.h;
		spx givenLength = m_axis == Axis::X ? sz.w : sz.h;
		spx givenBreadth = m_axis == Axis::X ? sz.h : sz.w;

		spx spacingBefore = align(ptsToSpx(m_spacingBefore, m_scale));
//		spx spacingAfter = align(ptsToSpx(m_spacingAfter, m_scale));
		spx spacingBetween = align(ptsToSpx(m_spacingBetween, m_scale));


		givenLength -= m_totalSpacing;

		// Optimized special case, just copy preferred to length.
		//TODO: We probably need to use matchingWidth()/matchingHeight() here anyway... prefered length might change with given breadth

		auto pLayout = m_pLayout ? m_pLayout : Base::defaultPackLayout();

		CoordSPX pos = contentOfs;

		spx baselineOffset = _calcBaselineOffset(givenBreadth);
		if (m_axis == Axis::X)
		{
			pos.x += spacingBefore;
			pos.y += baselineOffset;
		}
		else
		{
			pos.x += baselineOffset;
			pos.y += spacingBefore;
		}

		if( wantedLength == givenLength && !pLayout->doesCalcWantedLength() )
		{
			RectSPX geo;

			for (auto p = slots.begin(); p != slots.end(); p++)
			{
				if( p->m_bVisible )
				{
					geo.x = pos.x;
					geo.y = pos.y;
					if( m_axis == Axis::X )
					{
						geo.w = p->m_defaultSize.w;

						if( m_slotAlignment == Alignment::Justify )
							geo.h = sz.h;
						else
						{
							geo.h = p->m_defaultSize.h;
							geo.y -= align(p->m_baseline * geo.h);
						}

						pos.x += p->m_defaultSize.w + spacingBetween;
					}
					else
					{
						if( m_slotAlignment == Alignment::Justify )
							geo.w = sz.w;
						else
						{
							geo.w = p->m_defaultSize.w;
							geo.x -= align(p->m_baseline * geo.w);
						}

						geo.h = p->m_defaultSize.h;
						pos.y += p->m_defaultSize.h + spacingBetween;
					}

					if( geo != p->m_geo )
					{
						if (bRequestRender)
							_requestRender(p->m_geo + p->_widget()->_overflow());

						spx oldW = p->m_geo.w;
						spx oldH = p->m_geo.h;
						p->m_geo = geo;
						if( geo.w != oldW || geo.h != oldH )
						{
							p->_widget()->_resize( geo.size(), m_scale );
							p->m_bResizeRequired = false;
						}

						if (bRequestRender)
							_requestRender(p->m_geo + p->_widget()->_overflow());

					}
				}
				else
				{
					if( bRequestRender && p->m_geo.w != 0 && p->m_geo.h != 0 )
						_requestRender(p->m_geo + p->_widget()->_overflow());

					p->m_geo.x = pos.x + contentOfs.x;
					p->m_geo.y = pos.y + contentOfs.y;
					if( m_axis == Axis::X )
					{
						geo.w = 0;
						geo.h = sz.h;
					}
					else
					{
						geo.w = sz.w;
						geo.h = 0;
					}
				}

				if( p->m_bResizeRequired )
				{
					p->_widget()->_resize(geo.size(), m_scale);
					p->m_bResizeRequired = false;
				}
			}
		}
		else
		{
			// Allocate and populate SizeBroker array

			int arrayBytes = (sizeof(PackLayout::Item)+sizeof(spx)) * slots.size();
			PackLayout::Item* pItemArea = reinterpret_cast<PackLayout::Item*>(Base::memStackAlloc(arrayBytes));
			spx* pOutput = (spx*) &pItemArea[slots.size()];

			int nItems = _populateLayoutArray(pItemArea, m_scale, givenBreadth);

			// Retrieve length and set geo for all children, call _requestRender() and _setSize() where needed.

			pLayout->getItemSizes(pOutput, givenLength, m_scale, nItems, pItemArea);

			RectSPX geo;

			for (auto p = slots.begin(); p != slots.end(); p++)
			{
				if( p->m_bVisible )
				{
					geo.x = pos.x;
					geo.y = pos.y;
					if( m_axis == Axis::X )
					{
						geo.w = *pOutput;

						if( m_slotAlignment == Alignment::Justify )
							geo.h = sz.h;
						else
						{
							geo.h = p->m_defaultSize.h;
							geo.y -= align(p->m_baseline * geo.h);
						}

						pos.x += *pOutput + spacingBetween;
					}
					else
					{
						if( m_slotAlignment == Alignment::Justify )
							geo.w = sz.w;
						else
						{
							geo.w = p->m_defaultSize.w;
							geo.x -= align(p->m_baseline * geo.w);
						}

						geo.h = *pOutput;
						pos.y += *pOutput + spacingBetween;
					}

					if( geo != p->m_geo )
					{
						if (bRequestRender)
							_requestRender(p->m_geo + p->_widget()->_overflow());

						spx oldW = p->m_geo.w;
						spx oldH = p->m_geo.h;
						p->m_geo = geo;
						if( geo.w != oldW || geo.h != oldH )
						{
							p->_widget()->_resize( geo.size(), m_scale );
							p->m_bResizeRequired = false;
						}

						if (bRequestRender)
							_requestRender(p->m_geo + p->_widget()->_overflow());
					}
					pOutput++;
				}
				else
				{
					if( bRequestRender && p->m_geo.w != 0 && p->m_geo.h != 0 )
						_requestRender(p->m_geo + p->_widget()->_overflow());

					p->m_geo.x = pos.x + contentOfs.x;
					p->m_geo.y = pos.y + contentOfs.y;
					if( m_axis == Axis::X )
					{
						geo.w = 0;
						geo.h = sz.h;
					}
					else
					{
						geo.w = sz.w;
						geo.h = 0;
					}
				}

				if (p->m_bResizeRequired)
				{
					p->_widget()->_resize(geo.size(), m_scale);
					p->m_bResizeRequired = false;
				}
			}

			// Release SizeBroker array

			Base::memStackFree(arrayBytes);
		}

		//TODO: This should be baked into the loop above instead to make it faster.

		_refreshOverflow();
	}

	//____ _populateLayoutArray() ___________________________________________

	int PackPanel::_populateLayoutArray(PackLayout::Item* pArray, int scale ) const
	{
		PackLayout::Item* pI = pArray;

		if( m_axis == Axis::X )
		{
			for (auto pS = slots.begin(); pS != slots.end(); pS++)
			{
				if( pS->m_bVisible )
				{
					pI->def = scale == m_scale ? pS->m_defaultSize.w : pS->_widget()->_defaultSize(scale).w;
					pI->min = pS->_widget()->_minSize(scale).w;
					pI->max = pS->_widget()->_maxSize(scale).w;
					pI->weight = pS->m_weight*65536;
					pI++;
				}
			}
		}
		else
		{
			for (auto pS = slots.begin(); pS != slots.end(); pS++)
			{
				if( pS->m_bVisible )
				{
					pI->def = scale == m_scale ? pS->m_defaultSize.h : pS->_widget()->_defaultSize(scale).h;
					pI->min = pS->_widget()->_minSize(scale).h;
					pI->max = pS->_widget()->_maxSize(scale).h;
					pI->weight = pS->m_weight*65536;
					pI++;
				}
			}
		}

		return int(pI - pArray);
	}

	int PackPanel::_populateLayoutArray(PackLayout::Item* pArray, int scale, spx forcedBreadth ) const
	{
		PackLayout::Item* pI = pArray;

		if( m_axis == Axis::X )
		{
			for (auto pS = slots.begin(); pS != slots.end(); pS++)
			{
				if( pS->m_bVisible )
				{
					pI->def = pS->_widget()->_matchingWidth(forcedBreadth,scale);
					pI->min = pS->_widget()->_minSize(scale).w;
					pI->max = pS->_widget()->_maxSize(scale).w;
					pI->weight = pS->m_weight*65536;
					pI++;
				}
			}
		}
		else
		{
			for (auto pS = slots.begin(); pS != slots.end(); pS++)
			{
				if( pS->m_bVisible )
				{
					pI->def = pS->_widget()->_matchingHeight(forcedBreadth,scale);
					pI->min = pS->_widget()->_minSize(scale).h;
					pI->max = pS->_widget()->_maxSize(scale).h;
					pI->weight = pS->m_weight*65536;
					pI++;
				}
			}
		}

		return int(pI - pArray);
	}


} // namespace wg
