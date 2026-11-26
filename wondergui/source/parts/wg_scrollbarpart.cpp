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
#include <wg_scrollbarpart.h>


namespace wg
{
	using namespace Util;

	const TypeInfo	ScrollbarPart::TYPEINFO = { "ScrollbarPart", &Part::TYPEINFO };

	//____ constructor ____________________________________________________________

	ScrollbarPart::ScrollbarPart(Widget* pWidget, ScrollbarPart::Holder * pHolderInterface, Axis axis) : 
		Part(pWidget),m_pHolderInterface(pHolderInterface), m_axis(axis)
	{
	}

	//____ destructor _________________________________________________________

	ScrollbarPart::~ScrollbarPart()
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollbarPart::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ setSkins() _________________________________________________________
	/**
	 * Set all skins of this scrollbar in one go.
	 *
	 * @param pBackgroundSkin	Background skin for the scrollbar handle area.
	 * @param pHandleSkin		Skin for the scrollbar handle.
	 * @param pBwdButtonSkin	Skin for the backward (up or left) button.
	 * @param pFwdButtonSkin	Skin for the forward (down or right) button.
	 *
	 **/
	void ScrollbarPart::setSkins(Skin* pBackground, Skin* pBar, Skin* pPrevButton, Skin* pNextButton)
	{
		int scale = _scale();

		bool	bRequestRender = false;
		bool	bRequestResize = false;

		Skin* skin[4] = { pBackground, pBar, pPrevButton, pNextButton };

		for (int i = 0; i < 4; i++)
		{
			if (skin[i] != m_skins[i])
			{
				SizeSPX oldPref = m_skins[i] ? m_skins[i]->_defaultSize(scale) : SizeSPX();
				SizeSPX newPref = skin[i] ? skin[i]->_defaultSize(scale) : SizeSPX();

				m_skins[i] = skin[i];
				bRequestRender = true;

				if (oldPref != newPref)
					bRequestResize = true;
			}
		}

		if (bRequestRender)
			_requestRender();

		if (bRequestResize)
			_requestResize();
	}

	//____ setBackSkin() ____________________________________________________

	void ScrollbarPart::setBackSkin(Skin* pSkin)
	{
		_replaceSkin(pSkin, m_skins[Element::Back]);
	}

	//____ setBarSkin() _______________________________________________________

	void ScrollbarPart::setBarSkin(Skin* pSkin)
	{
		_replaceSkin(pSkin, m_skins[Element::Bar]);
	}

	//____ setForwardButtonSkin() _________________________________________________

	void ScrollbarPart::setForwardButtonSkin(Skin* pSkin)
	{
		_replaceSkin(pSkin, m_skins[Element::Next]);
	}

	//____ setBackwardButtonSkin() ________________________________________________

	void ScrollbarPart::setBackwardButtonSkin(Skin* pSkin)
	{
		_replaceSkin(pSkin, m_skins[Element::Prev]);
	}

	//____ setJumpToPress() ___________________________________________________

	void ScrollbarPart::setJumpToPress(bool bJump)
	{
		m_bJumpToPress = bJump;
	}

	//____ setVisible() __________________________________________________________

	void ScrollbarPart::setVisible(bool bVisible)
	{
		if( bVisible != m_bVisible )
		{
			m_bVisible = bVisible;
			_requestRender();
			_requestResize();			
		}
	}


	//____ _initFromBlueprint() _______________________________________________

	void ScrollbarPart::_initFromBlueprint(const Blueprint& bp)
	{
		if( bp.back )
			m_skins[Element::Back] = bp.back;

		if (bp.backwardButton)
			m_skins[Element::Prev] = bp.backwardButton;

		if (bp.bar)
			m_skins[Element::Bar] = bp.bar;

		if (bp.forwardButton)
			m_skins[Element::Next] = bp.forwardButton;

		m_bJumpToPress	= bp.jumpToPress;
		m_bVisible		= bp.visible;
	}

	//____ _defaultSize() ___________________________________________________

	SizeSPX ScrollbarPart::_defaultSize(int scale) const
	{
		if( !m_bVisible )
			return {0,0};
		
		SizeSPX prefSize = m_skins[Element::Bar] ? m_skins[Element::Bar]->_defaultSize(scale) : SizeSPX();

		if (m_skins[Element::Back])
			prefSize += m_skins[Element::Back]->_contentBorderSize(scale);

		if (m_axis == Axis::X)
		{
			if (m_skins[Element::Next])
			{
				SizeSPX pref = m_skins[Element::Next]->_defaultSize(scale);
				prefSize.h = std::max(prefSize.h, pref.h);
				prefSize.w += pref.w;
			}

			if (m_skins[Element::Prev])
			{
				SizeSPX pref = m_skins[Element::Prev]->_defaultSize(scale);
				prefSize.h = std::max(prefSize.h, pref.h);
				prefSize.w += pref.w;
			}

			prefSize.w += 20 * scale;
		}
		else
		{
			if (m_skins[Element::Next])
			{
				SizeSPX pref = m_skins[Element::Next]->_defaultSize(scale);
				prefSize.w = std::max(prefSize.w, pref.w);
				prefSize.h += pref.h;
			}

			if (m_skins[Element::Prev])
			{
				SizeSPX pref = m_skins[Element::Prev]->_defaultSize(scale);
				prefSize.w = std::max(prefSize.w, pref.w);
				prefSize.h += pref.h;
			}

			prefSize.h += 20 * scale;
		}

		return prefSize;
	}

	//____ _render() __________________________________________________________

	void ScrollbarPart::_render(GfxDevice* pDevice, const RectSPX& canvas, int scale)
	{
		for (int i = 0; i < Element_size; i++)
		{
			if (m_skins[i])
				m_skins[i]->_render(pDevice, _elementGeo((Element)i, canvas, scale), scale, m_states[i] );
		}
	}

	//____ _pointerMoved() ________________________________________________

	void ScrollbarPart::_pointerMoved(CoordSPX pointerPos)
	{
		RectSPX geo = _size();
		Element markedElement = _getMarkedElement(geo, pointerPos);

		if (m_pressedElement != Element::None && markedElement != m_pressedElement)
		{
			if (m_pressedElement == Element::Bar)
				markedElement = Element::Bar;
			else
				markedElement = Element::None;										// We can not mark anything else than what we have pressed.

		}

		if (markedElement != m_markedElement)
		{
			int scale = _scale();

			if (m_markedElement != Element::None)
				_setElementState(m_markedElement, m_states[m_markedElement] - State::Hovered,
					m_states[m_markedElement], geo, scale);

			if (markedElement != Element::None)
			{
				State newState = m_states[markedElement] + (markedElement == m_pressedElement ? State::Pressed : State::Hovered);
				_setElementState(markedElement, newState, m_states[markedElement], geo, scale);
			}

			m_markedElement = markedElement;
		}

	}

	//____ _receive() _________________________________________________________

	bool ScrollbarPart::_receive(Msg* _pMsg)
	{
		bool bProcessed = false;

		switch (_pMsg->type())
		{
			case MsgType::MouseEnter:		// Continue to MouseMove
			case MsgType::MouseLeave:		// Continue to MouseMove
			case MsgType::MouseMove:
			{
				auto pMsg = static_cast<InputMsg*>(_pMsg);

				_pointerMoved( _toLocal(pMsg->pointerSpxPos()) );
				break;
			}

			case MsgType::MousePress:
			{
				if (m_markedElement == Element::None)
					break;

				m_pressedElement = m_markedElement;
				RectSPX geo = _size();
				int scale = _scale();

				switch (m_pressedElement)
				{			
					case Element::Bar:
					{
						auto pMsg = static_cast<MousePressMsg*>(_pMsg);
						CoordSPX pointerPos = _toLocal(pMsg->pointerSpxPos());

						auto barTopLeft = _elementGeo(Element::Bar, geo, scale).pos();

						m_barPressOfs = m_axis == Axis::X ? pointerPos.x - barTopLeft.x : pointerPos.y - barTopLeft.y;
						break;
					}
					case Element::Back:
					{
						auto pMsg = static_cast<MousePressMsg*>(_pMsg);
						CoordSPX pointerPos = _toLocal(pMsg->pointerSpxPos());
						auto barCenter = _elementGeo(Element::Bar, geo, scale).center();

						m_barPressOfs = m_axis == Axis::X ? pointerPos.x - barCenter.x : pointerPos.y - barCenter.y;
						if (m_barPressOfs < 0 )
							m_pHolderInterface->_scrollbarPage(this, -1);
						else
							m_pHolderInterface->_scrollbarPage(this, 1);
						break;
					}
					case Element::Prev:
						m_pHolderInterface->_scrollbarStep(this, -1);
						break;
					case Element::Next:
						m_pHolderInterface->_scrollbarStep(this, 1);
						break;
					default:				// Just to avoid compiler warnings
						break;
				}

				_setElementState(m_pressedElement, m_states[m_pressedElement] + State::Pressed,
					m_states[m_pressedElement], geo, scale);
				_pMsg->swallow();
				bProcessed = true;
				break;
			}
			case MsgType::MouseRepeat:
			{
				if (m_pressedElement == Element::None || m_pressedElement != m_markedElement)
					break;

				switch (m_pressedElement)
				{
				case Element::Bar:
					break;
				case Element::Back:
				{
					RectSPX geo = _size();
					int scale = _scale();

					auto pMsg = static_cast<MousePressMsg*>(_pMsg);
					CoordSPX pointerPos = _toLocal(pMsg->pointerSpxPos());

					auto barCenter = _elementGeo(Element::Bar, geo, scale).center();

					spx barRepeatOfs = m_axis == Axis::X ? pointerPos.x - barCenter.x : pointerPos.y - barCenter.y;

					if (barRepeatOfs < 0 && m_barPressOfs < 0 )
						m_pHolderInterface->_scrollbarPage(this, -1);
					else if(barRepeatOfs > 0 && m_barPressOfs > 0 )
						m_pHolderInterface->_scrollbarPage(this, 1);
					break;
				}
				case Element::Prev:
					m_pHolderInterface->_scrollbarStep(this, -1);
					break;
				case Element::Next:
					m_pHolderInterface->_scrollbarStep(this, 1);
					break;
				default:				// Just to avoid compiler warnings
					break;
				}

				_pMsg->swallow();
				bProcessed = true;
				break;
			}
			case MsgType::MouseRelease:
			{
				if (m_pressedElement == Element::None)
					break;

				RectSPX geo = _size();
				int scale = _scale();

				_setElementState(m_pressedElement, m_states[m_pressedElement] - State::Pressed,
					m_states[m_pressedElement], geo, scale);

				m_pressedElement = Element::None;
				
				auto pMsg = static_cast<MouseReleaseMsg*>(_pMsg);
				_pointerMoved( _toLocal(pMsg->pointerSpxPos()));
				

				_pMsg->swallow();
				bProcessed = true;
				break;
			}
			case MsgType::MouseDrag:
			{
				if (m_pressedElement == Element::Bar)
				{
					auto pMsg = static_cast<MouseDragMsg*>(_pMsg);

					RectSPX geo = {0,0,_size()};
					int scale = _scale();

					RectSPX backGeo = _elementGeo(Element::Back, geo, scale);

					RectSPX barGeo = _dragbarArea(backGeo, scale);

					CoordSPX pointerOfs = _toLocal(pMsg->pointerSpxPos()) - backGeo.pos();


					spx barMaxOfs;
					spx barOfs;

					if (m_axis == Axis::X)
					{
						barMaxOfs = backGeo.w - barGeo.w;
						barOfs = pointerOfs.x - m_barPressOfs;
					}
					else
					{
						barMaxOfs = backGeo.h - barGeo.h;
						barOfs = pointerOfs.y - m_barPressOfs;
					}

					limit(barOfs, 0, barMaxOfs);

					spx viewOfs;
					spx viewLen;
					spx contentLen;
					std::tie(viewOfs, viewLen, contentLen) = m_pHolderInterface->_scrollbarOfsLenContent(this);

					spx newViewOfs = (barOfs / float(barMaxOfs)) * (contentLen - viewLen);

					m_pHolderInterface->_scrollbarMove(this, newViewOfs);
					bProcessed = true;
				}
				break;
			}
			case MsgType::WheelRoll:
			{
				auto pMsg = static_cast<WheelRollMsg*>(_pMsg);
				int dir = pMsg->distance();
				int wheel = pMsg->wheel();

				if( wheel == 1 && dir != 0 && m_markedElement != Element::None)
				{
					m_pHolderInterface->_scrollbarWheel(this, dir);
					_pMsg->swallow();
					bProcessed = true;
				}
				break;
			}
				
			default:
				break;
		}
		return bProcessed;
	}

	//____ _setState() ________________________________________________________

	void ScrollbarPart::_setState(State state)
	{
		// We just copy Disabled/Focused/Checked/Flagged to all our skins.
		// We ignore Targeted since that won't work without hover anyway.

		bool bDisabled = state.isDisabled();
		bool bFocused = state.isFocused();
		bool bChecked = state.isChecked();
		bool bFlagged = state.isFlagged();

		if (bDisabled != m_states[Element::Back].isDisabled() || bFocused != m_states[Element::Back].isFocused() ||
			bChecked != m_states[Element::Back].isChecked() || bFlagged != m_states[Element::Back].isFlagged() )
		{
			for (int i = 0; i < Element_size; i++)
			{
				m_states[i].setDisabled(bDisabled);
				m_states[i].setFocused(bFocused);
				m_states[i].setChecked(bChecked);
				m_states[i].setFlagged(bFlagged);
			}

			_requestRender();
		}
	}

	//____ _setAxis() _________________________________________________________

	void ScrollbarPart::_setAxis(Axis axis)
	{
		// No request render here. We assume it is done anyway.

		m_axis = axis;
	}

	//____ _alphaTest() _______________________________________________________

	bool ScrollbarPart::_alphaTest(CoordSPX ofs, SizeSPX canvas)
	{
		//TODO: Support rounded outer corners on buttons?
		//TODO: Check against the skins?

		return true;
	}

	//____ _update() __________________________________________________________

	void ScrollbarPart::_update(spx newViewPos, spx oldViewPos, spx newViewLen, spx oldViewLen, spx newContentLen, spx oldContentLen)
	{
		RectSPX canvas = _size();
		int scale = _scale();

		RectSPX sliderBack = _elementGeo(Element::Back, canvas, scale);
		if (m_skins[Element::Back])
			sliderBack = m_skins[Element::Back]->_contentRect(sliderBack, scale, m_states[Element::Back]);

		RectSPX oldBar = _dragbarArea(sliderBack, scale, oldViewPos, oldViewLen, oldContentLen );
		RectSPX newBar = _dragbarArea(sliderBack, scale, newViewPos, newViewLen, newContentLen);

		RectSPX dirt = RectSPX::bounds(oldBar, newBar);

		if( !dirt.isEmpty() )
			_requestRender(dirt);
	}


	//____ _getMarkedElement() ___________________________________________________
	//
	// We accept bar being marked even if we are beside it. This is per design.

	ScrollbarPart::Element ScrollbarPart::_getMarkedElement(const RectSPX& canvas, CoordSPX pos) const
	{
		if (!canvas.contains(pos))
			return Element::None;

		int scale = _scale();
		int nextButtonLen = _buttonLen(Element::Next, canvas, scale);
		int prevButtonLen = _buttonLen(Element::Prev, canvas, scale);


		if (m_axis == Axis::X)
		{
			if (pos.x < canvas.x + prevButtonLen)
				return Element::Prev;

			if (pos.x >= canvas.x + canvas.w - nextButtonLen)
				return Element::Next;

			RectSPX backContent = { canvas.x + prevButtonLen, canvas.y, canvas.w - prevButtonLen - nextButtonLen, canvas.h };
			if (m_skins[Element::Back])
				backContent = m_skins[Element::Back]->_contentRect(backContent, scale, m_states[Element::Back]);

			RectSPX bar = _dragbarArea(backContent, scale);
			if (pos.x >= bar.x && pos.x < bar.x + bar.w)
				return Element::Bar;
		}

		if (m_axis == Axis::Y)
		{
			if (pos.y < canvas.y + prevButtonLen)
				return Element::Prev;

			if (pos.y >= canvas.y + canvas.h - nextButtonLen)
				return Element::Next;

			RectSPX backContent = { canvas.x, canvas.y + prevButtonLen, canvas.w, canvas.h - prevButtonLen - nextButtonLen };
			if (m_skins[Element::Back])
				backContent = m_skins[Element::Back]->_contentRect(backContent, scale, m_states[Element::Back]);

			RectSPX bar = _dragbarArea(backContent, scale);
			if (pos.y >= bar.y && pos.y < bar.y + bar.h)
				return Element::Bar;
		}

		return Element::Back;
	}	

	//____ _elementGeo() _______________________________________________

	RectSPX ScrollbarPart::_elementGeo(Element element, const RectSPX& canvas, int scale) const
	{
		int nextButtonLen = _buttonLen(Element::Next, canvas, scale);
		int prevButtonLen = _buttonLen(Element::Prev, canvas, scale);

		switch (element)
		{
			case Element::Back:
				if (m_axis == Axis::X)
					return { canvas.x + prevButtonLen, canvas.y, canvas.w - prevButtonLen - nextButtonLen, canvas.h };
				else
					return { canvas.x, canvas.y + prevButtonLen, canvas.w, canvas.h - prevButtonLen - nextButtonLen };
			case Element::Bar:
			{
				RectSPX availableArea;
				if (m_axis == Axis::X)
					availableArea = { canvas.x + prevButtonLen, canvas.y, canvas.w - prevButtonLen - nextButtonLen, canvas.h };
				else
					availableArea = { canvas.x, canvas.y + prevButtonLen, canvas.w, canvas.h - prevButtonLen - nextButtonLen };

				if (m_skins[Element::Back])
					availableArea = m_skins[Element::Back]->_contentRect(availableArea, scale, m_states[Element::Back]);

				return  _dragbarArea(availableArea, scale);
			}
			case Element::Prev:
				if (m_axis == Axis::X)
					return { canvas.x, canvas.y, prevButtonLen, canvas.h };
				else
					return { canvas.x, canvas.y, canvas.w, prevButtonLen };
			case Element::Next:
				if (m_axis == Axis::X)
					return { canvas.x + canvas.w - nextButtonLen, canvas.y, nextButtonLen, canvas.h };
				else
					return { canvas.x, canvas.y + canvas.h - nextButtonLen, canvas.w, nextButtonLen };
			default:
				return RectSPX();
		}
	}

	//____ _setElementState() _________________________________________________

	void ScrollbarPart::_setElementState(Element element, State newState, State oldState, const RectSPX& fullCanvas, int scale )
	{
		if (element == Element::None)
			return;

		if (m_skins[element])
		{
			RectSPX dirt = m_skins[element]->_dirtyRect(_elementGeo(element, fullCanvas, scale), scale, newState, oldState );

			if (!dirt.isEmpty())
				_requestRender(dirt);

			m_states[element] = newState;
		}
	}

	//____ _replaceSkin() _____________________________________________________

	void ScrollbarPart::_replaceSkin(Skin* pNew, Skin_p& pOld)
	{
		if (pNew != pOld)
		{
			int scale = _scale();
			SizeSPX oldPref = pOld ? pOld->_defaultSize(scale) : SizeSPX();
			SizeSPX newPref = pNew ? pNew->_defaultSize(scale) : SizeSPX();

			pOld = pNew;
			_requestRender();

			if (oldPref != newPref)
				_requestResize();
		}
	}

	//____ _buttonLen() _____________________________________________________

	spx ScrollbarPart::_buttonLen(Element button, const RectSPX& canvas, int scale) const
	{
		if (m_skins[button])
		{
			SizeSPX buttonSize = m_skins[button]->_defaultSize(scale);

			if (m_axis == Axis::X)
			{
				if (buttonSize.h != canvas.h)
					buttonSize.w *= canvas.h / (float) buttonSize.h;

				return buttonSize.w;
			}
			else
			{
				if (buttonSize.w != canvas.w)
					buttonSize.h *= canvas.w / (float)buttonSize.w;

				return buttonSize.h;
			}
		}
		else 
			return 0;
	}

	//____ _dragbarArea() _____________________________________________________

	RectSPX ScrollbarPart::_dragbarArea(const RectSPX& availableArea, int scale) const
	{
			spx viewOfs;
			spx viewLen;
			spx contentLen;
			std::tie(viewOfs, viewLen, contentLen) = m_pHolderInterface->_scrollbarOfsLenContent(this);
			
			return _dragbarArea(availableArea, scale, viewOfs, viewLen, contentLen);
	}

	RectSPX ScrollbarPart::_dragbarArea(const RectSPX& availableArea, int scale, spx viewOfs, spx viewLen, spx contentLen) const
	{
		float fracLen = contentLen == 0 ? 1.f : float(viewLen) / float(contentLen);
		float fracOfs = contentLen == viewLen ? 0 : float(viewOfs) / float(contentLen - viewLen);

		spx maxLen = m_axis == Axis::X ? availableArea.w : availableArea.h;
		spx minLen = m_axis == Axis::X ? m_skins[Element::Bar]->_defaultSize(scale).w : m_skins[Element::Bar]->_defaultSize(scale).h;

		spx length = std::max(minLen,align(maxLen * fracLen));
		spx ofs = align((maxLen - length) * fracOfs);

		if (m_axis == Axis::X)
			return { availableArea.x + ofs, availableArea.y, length, availableArea.h };
		else
			return { availableArea.x, availableArea.y + ofs, availableArea.w, length };
	}

}
