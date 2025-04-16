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

#include <wg_splitpanel.h>
#include <wg_patches.h>
#include <wg_msg.h>

//TODO: Set opaque if there is no skin (or skin has no borders) and both children and the handle are opaque.

namespace wg
{
	const TypeInfo SplitPanel::TYPEINFO = { "SplitPanel", &Container::TYPEINFO };
	const TypeInfo SplitPanel::Slot::TYPEINFO = { "SplitPanel::Slot", &DynamicSlot::TYPEINFO };

	using namespace Util;

	//____ constructor ____________________________________________________________

	SplitPanel::SplitPanel() : slots(this), m_handleSkin(this)
	{
		m_bHorizontal = false;
		m_handleThickness = 0;
		m_resizeRatio = 0.5f;

		m_bSiblingsOverlap = false;
	}

	//____ Destructor _____________________________________________________________

	SplitPanel::~SplitPanel()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SplitPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAxis() _______________________________________________________

	void SplitPanel::setAxis(Axis axis)
	{
		bool bHorizontal = (axis == Axis::X);

		if (bHorizontal != m_bHorizontal)
		{
			m_bHorizontal = bHorizontal;
			_updateDefaultSize();
			_updateGeo();
			_refreshOverflow();
		}
	}

	//____ setHandleSkin() ____________________________________________________

	void SplitPanel::setHandleSkin(Skin* pSkin)
	{
		if( m_handleSkin.get() != pSkin )
		{
			m_handleSkin.set(pSkin);
			_updateDefaultSize();
			_updateGeo();
			_refreshOverflow();
		}
	}


	//____ setHandleThickness() __________________________________________________

	void SplitPanel::setHandleThickness(pts thickness)
	{
		//TODO: Assert on negative value.

		if (thickness != m_handleThickness)
		{
			m_handleThickness = thickness;
			_updateDefaultSize();
			_updateGeo();
			_refreshOverflow();
		}
	}

	//____ pointerStyle() ______________________________________________________Ä_

	PointerStyle SplitPanel::pointerStyle() const
	{
		if( m_handleState.isHovered() )
			return m_bHorizontal ? PointerStyle::ResizeBeamWE : PointerStyle::ResizeBeamNS;

		return m_pointerStyle;
	}

	//____ setResizeRatio() ___________________________________________________

	void SplitPanel::setResizeRatio(float ratio)
	{
		if (ratio != m_resizeRatio)
		{
			m_resizeRatio = ratio;
		}
	}

	//____ setResizeFunction() ___________________________________________________

	void SplitPanel::setResizeFunction(std::function<pts(Widget * pFirst, Widget * pSecond, pts totalLength, pts handleMovement)> func)
	{
		m_resizeFunc = func;
		_updateGeo();
		_refreshOverflow();
	}

	//____ setSplit() _________________________________________________

	void SplitPanel::setSplit(float factor)
	{
		limit(factor, 0.f, 1.f);

		RectSPX geo = m_size;
		RectSPX contentGeo = m_skin.contentRect(geo, m_scale, m_state);

		RectSPX firstChildGeo;
		RectSPX secondChildGeo;
		RectSPX handleGeo;

		// Calculate new lengths using broker

		spx handleThickness = _handleThickness(m_scale);

		spx totalLength = (m_bHorizontal ? contentGeo.w : contentGeo.h) - handleThickness;
		spx firstChildLength;
		spx secondChildLength;

		firstChildLength = align(factor * totalLength);
		
		firstChildLength = _limitFirstSlotLength(firstChildLength, totalLength);
		secondChildLength = totalLength - firstChildLength;

		
		
		
		// Update geo rectangles

		if( m_bHorizontal )
		{
			firstChildGeo = RectSPX(contentGeo.x, contentGeo.y, firstChildLength, contentGeo.h);
			secondChildGeo = RectSPX(contentGeo.x + contentGeo.w - secondChildLength, contentGeo.y, secondChildLength, contentGeo.h);
			handleGeo = RectSPX(contentGeo.x + firstChildLength, contentGeo.y, handleThickness, contentGeo.h );
		}
		else
		{
			firstChildGeo = RectSPX(contentGeo.x, contentGeo.y, contentGeo.w, firstChildLength);
			secondChildGeo = RectSPX(contentGeo.x, contentGeo.y + contentGeo.h - secondChildLength, contentGeo.w, secondChildLength);
			handleGeo = RectSPX(contentGeo.x, contentGeo.y + firstChildLength, contentGeo.w, handleThickness);
		}

		// Request render and set sizes

		if (handleGeo != m_handleGeo || firstChildGeo != slots[0].m_geo || secondChildGeo != slots[1].m_geo)
		{
			BorderSPX oldOverflow = m_overflow;

			slots[0].m_geo = firstChildGeo;
			if( slots[0]._widget() )
				slots[0]._widget()->_resize(firstChildGeo, m_scale);

			slots[1].m_geo = secondChildGeo;
			if (slots[1]._widget())
				slots[1]._widget()->_resize(secondChildGeo, m_scale);

			m_handleGeo = handleGeo;

			_refreshOverflow();
			
			RectSPX renderArea = RectSPX(m_size) + BorderSPX::max(oldOverflow, m_overflow);
			_requestRender(renderArea);
		}
	}

	//____ split() _______________________________________________________________

	float SplitPanel::split() const
	{
		RectSPX geo = m_size;
		RectSPX contentGeo = m_skin.contentRect(geo, m_scale, m_state);

		spx handleThickness = _handleThickness(m_scale);
		spx totalLength = (m_bHorizontal ? contentGeo.w : contentGeo.h) - handleThickness;

		if( m_bHorizontal )
			return (m_handleGeo.x - contentGeo.x) / float(totalLength);
		else
			return (m_handleGeo.y - contentGeo.y) / float(totalLength);
	}

	//____ _defaultSize() _______________________________________________________

	SizeSPX  SplitPanel::_defaultSize(int scale) const
	{
		if (scale == m_scale)
			return m_defaultSize;
		else
			return _calcDefaultSize(scale);
	}

	//____ _handleThickness() ____________________________________________________

	spx SplitPanel::_handleThickness(int scale) const
	{
		spx thickness = align(ptsToSpx(m_handleThickness,scale));
		if (thickness == 0 )
			thickness = m_bHorizontal ? m_handleSkin.defaultSize(scale).w : m_handleSkin.defaultSize(scale).h;
		return thickness;
	}


	//____ _updateDefaultSize() ________________________________________________

	void SplitPanel::_updateDefaultSize()
	{
		SizeSPX sz = _calcDefaultSize(m_scale);
		//TODO: We can't trust that same defaultSize results in same matchingHeight. We need to find a more robust optimization.
//		if (sz != m_defaultSize)
		{
			m_defaultSize = sz;
			_requestResize();
		}

	}
	
	//____ _calcDefaultSize() _______________________________________________

	SizeSPX SplitPanel::_calcDefaultSize(int scale) const
	{
		SizeSPX firstSz;
		SizeSPX secondSz;

		SizeSPX sz;

		if (slots[0]._widget())
			firstSz = slots[0]._widget()->_defaultSize(scale);

		if (slots[1]._widget())
			secondSz = slots[1]._widget()->_defaultSize(scale);

		if (m_bHorizontal)
		{
			sz.w = firstSz.w + secondSz.w + _handleThickness(scale);
			sz.h = std::max(firstSz.h, secondSz.h);
			if (m_handleSkin.defaultSize(scale).h > sz.h)
				sz.h = m_handleSkin.defaultSize(scale).h;
		}
		else
		{
			sz.w = std::max(firstSz.w, secondSz.w);
			sz.h = firstSz.h + secondSz.h + _handleThickness(scale);
			if (m_handleSkin.defaultSize(scale).w > sz.w)
				sz.w = m_handleSkin.defaultSize(scale).w;
		}

		// Take skins padding and default size into account

		if (!m_skin.isEmpty())
		{
			sz += m_skin.contentBorderSize(scale);
			SizeSPX skinSz = m_skin.defaultSize(scale);
			if (skinSz.w > sz.w)
				sz.w = skinSz.w;
			if (skinSz.h > sz.h)
				sz.h = skinSz.h;
		}

		return sz;
	}

	//____ _updateGeo() __________________________________________________________

	bool SplitPanel::_updateGeo(spx handleMovement)
	{
		RectSPX geo = m_size;
		RectSPX contentGeo = m_skin.contentRect(geo, m_scale, m_state);

		RectSPX firstChildGeo;
		RectSPX secondChildGeo;
		RectSPX handleGeo;

		// Calculate new lengths using broker

		spx handleThickness = _handleThickness(m_scale);

		spx totalLength = (m_bHorizontal ? contentGeo.w : contentGeo.h) - handleThickness;
		spx	length;
		spx firstChildLength;
		spx secondChildLength;

		if (m_resizeFunc)
		{
			pts len = m_resizeFunc(slots[0]._widget(), slots[1]._widget(), spxToPts(totalLength,m_scale), spxToPts(handleMovement,m_scale));
			length = ptsToSpx(len, m_scale);
		}
		else
			length = _defaultResizeFunc(slots[0]._widget(), slots[1]._widget(), totalLength, handleMovement);

		firstChildLength = align(length);

		secondChildLength = totalLength - firstChildLength;

		// Update geo rectangles

		if( m_bHorizontal )
		{
			firstChildGeo = RectSPX(contentGeo.x, contentGeo.y, firstChildLength, contentGeo.h);
			secondChildGeo = RectSPX(contentGeo.x + contentGeo.w - secondChildLength, contentGeo.y, secondChildLength, contentGeo.h);
			handleGeo = RectSPX(contentGeo.x + firstChildLength, contentGeo.y, handleThickness, contentGeo.h );
		}
		else
		{
			firstChildGeo = RectSPX(contentGeo.x, contentGeo.y, contentGeo.w, firstChildLength);
			secondChildGeo = RectSPX(contentGeo.x, contentGeo.y + contentGeo.h - secondChildLength, contentGeo.w, secondChildLength);
			handleGeo = RectSPX(contentGeo.x, contentGeo.y + firstChildLength, contentGeo.w, handleThickness);
		}

		// Request render and set sizes


		if (handleGeo != m_handleGeo || firstChildGeo != slots[0].m_geo || secondChildGeo != slots[1].m_geo)
		{
			_requestRender(RectSPX{0,0,m_size} + m_overflow);

			slots[0].m_geo = firstChildGeo;
			if( slots[0]._widget() )
				slots[0]._widget()->_resize(firstChildGeo, m_scale);

			slots[1].m_geo = secondChildGeo;
			if (slots[1]._widget())
				slots[1]._widget()->_resize(secondChildGeo, m_scale);

			m_handleGeo = handleGeo;

			return true;
		}
		
		return false;
	}

	//____ _defaultResizeFunc() ___________________________________________________

	spx SplitPanel::_defaultResizeFunc(Widget * pFirst, Widget * pSecond, spx totalLength, spx handleMovement)
	{
		spx firstLength;

		if (handleMovement == 0)
		{
			if (m_bHorizontal)
			{
				spx extraLength = totalLength - slots[0].m_geo.w - slots[1].m_geo.w;
				firstLength = slots[0].m_geo.w + extraLength * m_resizeRatio;
			}
			else
			{
				spx extraLength = totalLength - slots[0].m_geo.h - slots[1].m_geo.h;
				firstLength = slots[0].m_geo.h + extraLength * m_resizeRatio;
			}
		}
		else
			firstLength = (m_bHorizontal ? slots[0].m_geo.w : slots[0].m_geo.h) + handleMovement;

		return _limitFirstSlotLength(firstLength, totalLength);
	}


	//____ _limitFirsSlotLength() ________________________________________________

	spx SplitPanel::_limitFirstSlotLength( spx firstLength, spx totalLength )
	{
		spx minLengthFirst = 0;
		spx minLengthSecond = 0;
		spx maxLengthFirst = spx_max;
		spx maxLengthSecond = spx_max;

		if( m_bHorizontal )
		{
			if (slots[0]._widget())
			{
				minLengthFirst = slots[0]._widget()->minSize().w;
				maxLengthFirst = slots[0]._widget()->maxSize().w;
			}

			if (slots[1]._widget())
			{
				minLengthSecond = slots[1]._widget()->minSize().w;
				maxLengthSecond = slots[1]._widget()->maxSize().w;
			}
		}
		else
		{
			if (slots[0]._widget())
			{
				minLengthFirst = slots[0]._widget()->minSize().h;
				maxLengthFirst = slots[0]._widget()->maxSize().h;
			}

			if (slots[1]._widget())
			{
				minLengthSecond = slots[1]._widget()->minSize().h;
				maxLengthSecond = slots[1]._widget()->maxSize().h;
			}
		}

		if (totalLength - firstLength < minLengthSecond)
			firstLength = totalLength - minLengthSecond;

		if (firstLength < minLengthFirst)
			firstLength = minLengthFirst;

		if (totalLength - firstLength > maxLengthSecond)
			firstLength = totalLength - maxLengthSecond;

		if (firstLength > maxLengthFirst)
			firstLength = maxLengthFirst;

		return firstLength;
	}

	//____ _receive() ___________________________________________________________

	void SplitPanel::_receive(Msg * pMsg)
	{
		//TODO: Implement!!!

		State handleState = m_handleState;

		switch (pMsg->type())
		{
			case MsgType::MouseEnter:
			case MsgType::MouseMove:
			{
				if (m_handleState.isPressed())
					return;

				CoordSPX pos = _toLocal(static_cast<InputMsg*>(pMsg)->pointerSpxPos());

				bool bHovered = m_handleGeo.contains(pos);
				handleState.setHovered(bHovered);
				break;
			}

			case MsgType::MouseLeave:
			{
				// Unhover handle unless it is pressed

				if (!handleState.isPressed())
					handleState.setHovered(false);
				break;
			}
			case MsgType::MousePress:
			{
				auto p = static_cast<MouseButtonMsg*>(pMsg);

				if (p->button() != MouseButton::Left)
					return;

				CoordSPX pos = _toLocal(p->pointerSpxPos());
				if (m_handleGeo.contains(pos))
				{
					m_handlePressOfs = m_bHorizontal ? pos.x - m_handleGeo.x : pos.y - m_handleGeo.y;
					handleState.setPressed(true);
					pMsg->swallow();
				}
				break;
			}

			case MsgType::MouseRelease:
			{
				auto p = static_cast<MouseButtonMsg*>(pMsg);

				if (p->button() != MouseButton::Left)
					return;

				if (handleState.isPressed() )
				{
					handleState.setPressed(false);
					pMsg->swallow();
				}
				break;
			}

			case MsgType::MouseDrag:
			{
				auto p = static_cast<MouseButtonMsg*>(pMsg);

				if (p->button() != MouseButton::Left)
					return;

				if (handleState.isPressed())
				{
					CoordSPX pos = _toLocal(p->pointerSpxPos());
					spx diff = (m_bHorizontal ? pos.x - m_handleGeo.x : pos.y - m_handleGeo.y) - m_handlePressOfs;

					_updateGeo(diff);
					_refreshOverflow();		// Yes, this could actually change influence...
					pMsg->swallow();
				}
				break;
			}

			default:
				break;
		}

		State oldHandleState = m_handleState;
		m_handleState = handleState;

		m_handleSkin.stateChanged(handleState, oldHandleState);
	}


	//____ _render() _____________________________________________________________

	void SplitPanel::_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		Container::_render(pDevice, canvas, window);

		m_handleSkin.render(pDevice, m_handleGeo + canvas.pos(), m_scale, m_handleState);
	}

	//____ _maskPatches() _____________________________________________________

	void SplitPanel::_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip)
	{
		RectSPX coverage = m_skin.coverage(geo, m_scale, m_state);
		
		if( !coverage.isEmpty() )
		{
			patches.sub( RectSPX::overlap(coverage,clip) );
			
			if( coverage.contains(_contentRect(geo)) )
				return;										// No need to loop through children, skins coverage contains them all.
		}
		
		if (slots[0]._widget())
			slots[0]._widget()->_maskPatches(patches, slots[0].m_geo + geo.pos(), clip );

		patches.sub(RectSPX::overlap(m_handleSkin.coverage(m_handleGeo + geo.pos(), m_scale, m_handleState ), clip));

		if (slots[1]._widget())
			slots[1]._widget()->_maskPatches(patches, slots[1].m_geo + geo.pos(), clip );
	}

	//____ _alphaTest() _______________________________________________________

	bool SplitPanel::_alphaTest(const CoordSPX& ofs)
	{
		bool bHit = Container::_alphaTest(ofs);

		if( !bHit )
			bHit = m_handleSkin.markTest(ofs, m_handleGeo, m_scale, m_handleState);

		return bHit;
	}

	//____ _resize() _________________________________________________________

	void SplitPanel::_resize(const SizeSPX& size, int scale)
	{
		Container::_resize(size,scale);
		_updateGeo();
		_refreshOverflow();
	}

	//____ _setState() ________________________________________________________

	void SplitPanel::_setState(State state)
	{
		// Disable handle when panel is disabled

		m_handleState.setDisabled(state.isDisabled());
		Container::_setState(state);
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	SplitPanel::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		return Slot::TYPEINFO;
	}

	//_____ _firstChild() _____________________________________________________

	Widget * SplitPanel::_firstChild() const
	{
		if (slots[0]._widget())
			return slots[0]._widget();
		else
			return slots[1]._widget();
	}

	//_____ _lastChild() ______________________________________________________

	Widget * SplitPanel::_lastChild() const
	{
		if (slots[1]._widget())
			return slots[1]._widget();
		else
			return slots[0]._widget();
	}

	//____ _calcOverflow() ___________________________________________________
					  
	BorderSPX SplitPanel::_calcOverflow()
	{
		// We need our own _refreshOverflow because of our handleSkin that needs to
		// be included.
			
		RectSPX overflowArea = RectSPX{ 0,0, m_size } + m_skin.overflow(m_scale);
			
		if( slots[0]._widget() && slots[0]._widget()->_hasOverflow() )
			overflowArea.growToContain(slots[0].m_geo + slots[0]._widget()->_overflow() );

		if( slots[1]._widget() && slots[1]._widget()->_hasOverflow() )
			overflowArea.growToContain(slots[1].m_geo + slots[1]._widget()->_overflow() );

		if( !m_handleSkin.isEmpty() )
			overflowArea.growToContain(m_handleGeo + m_handleSkin.overflow(m_scale));
			
		BorderSPX overflow = BorderSPX::diff( RectSPX{0,0,m_size}, overflowArea );
		
		return overflow;
	}
					  
	//_____ _firstSlotWithGeo() _______________________________________________

	void SplitPanel::_firstSlotWithGeo(SlotWithGeo& package) const
	{
		if (slots[0]._widget())
		{
			package.geo = slots[0].m_geo;
			package.pSlot = &slots[0];
		}
		else if (slots[1]._widget())
		{
			package.geo = slots[1].m_geo;
			package.pSlot = &slots[1];
		}
		else
		{
			package.pSlot = nullptr;
		}
	}

	//____ _nextSlotWithGeo() _________________________________________________

	void SplitPanel::_nextSlotWithGeo(SlotWithGeo& package) const
	{
		if (package.pSlot == &slots[0] && slots[1]._widget())
		{
			package.geo = slots[1].m_geo;
			package.pSlot = &slots[1];
		}
		else
			package.pSlot = nullptr;
	}

	//_____ _slotGeo() _______________________________________________________

	RectSPX SplitPanel::_slotGeo(const StaticSlot * pSlot) const
	{
		return static_cast<const Slot*>(pSlot)->m_geo;
	}

	//____ _childRequestRender() ______________________________________________

	void SplitPanel::_childRequestRender(StaticSlot * pSlot, const RectSPX& rect)
	{
		_requestRender(rect + static_cast<Slot*>(pSlot)->m_geo.pos());
	}

	//____ _childRequestResize() ______________________________________________

	void SplitPanel::_childRequestResize(StaticSlot * pSlot)
	{
		//TODO: Implement better solution, should be able to adapt width !!!

		auto p = static_cast<Slot*>(pSlot);
		p->_setSize(p->m_geo, m_scale);
	}

	//____ _prevChild() _______________________________________________________

	Widget * SplitPanel::_prevChild(const StaticSlot * pSlot) const
	{
		if (pSlot == &slots[1])
			return slots[0]._widget();
		else
			return nullptr;
	}

	//____ _nextChild() _______________________________________________________

	Widget * SplitPanel::_nextChild(const StaticSlot * pSlot) const
	{
		if (pSlot == &slots[0])
			return slots[1]._widget();
		else
			return nullptr;
	}

	//____ _releaseChild() ____________________________________________________

	void SplitPanel::_releaseChild(StaticSlot * pSlot)
	{
		_replaceChild(pSlot, nullptr);
	}

	//____ _replaceChild() _______________________________________________________

	void SplitPanel::_replaceChild(StaticSlot * _pSlot, Widget * pNewWidget)
	{
		auto pSlot = static_cast<Slot*>(_pSlot);

		RectSPX oldRenderArea = pSlot->m_geo;
		if( pSlot->_widget() && pSlot->_widget()->_hasOverflow() )
			oldRenderArea += pSlot->_widget()->_overflow();
		
		pSlot->_setWidget(pNewWidget);
		if( pNewWidget )
			pNewWidget->_resize(pSlot->m_geo, m_scale);
		bool bGeoChanged = _updateGeo();
		if (!bGeoChanged)
		{
			RectSPX newRenderArea = pSlot->m_geo;
			if( pSlot->_widget() && pSlot->_widget()->_hasOverflow() )
				newRenderArea += pSlot->_widget()->_overflow();

			if( newRenderArea.isEmpty() )
				newRenderArea = oldRenderArea;
			else if( !oldRenderArea.isEmpty() )
				newRenderArea.growToContain(oldRenderArea);
			
			_requestRender(newRenderArea);
		}

		_updateDefaultSize();

	}

	//____ _skinState() _______________________________________________________

	State SplitPanel::_skinState(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_handleSkin)
			return m_handleState;
		else
			return m_state;
	}

	//____ _skinSize() ___________________________________________________________

	SizeSPX SplitPanel::_skinSize(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_handleSkin)
			return m_handleGeo.size();
		else
			return m_size;
	}

	//____ _skinRequestRender() _______________________________________________

	void SplitPanel::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		if (pSlot == &m_handleSkin)
			_requestRender(rect + m_handleGeo.pos());
		else
			_requestRender(rect);
	}


}
