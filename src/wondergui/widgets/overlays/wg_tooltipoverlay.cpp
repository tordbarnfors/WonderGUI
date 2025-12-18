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
#include <wg_tooltipoverlay.h>
#include <wg_msg.h>
#include <wg_base.h>
#include <wg_inputhandler.h>
#include <wg_boxskin.h>
#include <wg_textdisplay.h>

namespace wg
{

	const TypeInfo TooltipOverlay::TYPEINFO = { "TooltipOverlay", &Overlay::TYPEINFO };
	const TypeInfo TooltipOverlay::Slot::TYPEINFO = { "TooltipOverlay::Slot", &Overlay::Slot::TYPEINFO };

	using namespace Util;

	//____ constructor ____________________________________________________________

	TooltipOverlay::TooltipOverlay() : m_tooltipSlot(this)
	{
	}

	//____ Destructor _____________________________________________________________

	TooltipOverlay::~TooltipOverlay()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TooltipOverlay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setTooltipGenerator() ______________________________________________

	void TooltipOverlay::setTooltipGenerator(const std::function<Widget_p(Position& position, const Widget*, const Border&)> func)
	{
		m_tooltipGenerator = func;
	}

	//____ setTooltipPosition() ______________________________________________

	void TooltipOverlay::setTooltipPosition(Placement placement, Border spacing, bool bTooltipAroundPointer)
	{
		m_defaultPosition.placement = placement;
		m_defaultPosition.spacing = spacing;
		m_defaultPosition.bAroundPointer = bTooltipAroundPointer;
	}

	//____ setDisplayTooltips() _______________________________________________

	void TooltipOverlay::setDisplayTooltips(bool bDisplay)
	{
		m_bDisplayTooltips = bDisplay;
		if (!m_bDisplayTooltips)
			_closeTooltip();
	}

	//____ _updateTooltipGeo() ________________________________________________

	void TooltipOverlay::_updateTooltipGeo()
	{
		//TODO: Implement!
	}

	//____ _openTooltip() _____________________________________________________

	void TooltipOverlay::_openTooltip()
	{
		if (!m_tooltipSlot.isEmpty() || !m_bDisplayTooltips )
			return;

		Widget* pHovered = _findWidget(m_hoverPos, SearchMode::ActionTarget);
		if (pHovered == nullptr || pHovered->tooltip().isEmpty() )
			return;

		RectSPX hoveredGeo = _toLocal(pHovered->_globalGeo());
		BorderSPX widgetMargin{ hoveredGeo.y, m_size.w - hoveredGeo.right(), m_size.h - hoveredGeo.bottom(), hoveredGeo.x };

		m_activePosition = m_defaultPosition;
		Widget_p pWidget = m_tooltipGenerator(m_activePosition, pHovered, spxToPts(widgetMargin,m_scale));

		if (pWidget)
		{
			m_tooltipSlot._setWidget(pWidget);

			SizeSPX tooltipSize = pWidget->_defaultSize(m_scale);

			Position& position = m_activePosition;

			RectSPX center = position.bAroundPointer ? RectSPX(m_hoverPos) : hoveredGeo;
			center += ptsToSpx(position.spacing,m_scale);

			CoordSPX tooltipPos;

			// Take care of vertical position

			if( position.placement == Placement::NorthWest || position.placement == Placement::North || position.placement == Placement::NorthEast )
			{
				if (center.y < tooltipSize.h)
					tooltipPos.y = center.y + center.h;
				else
					tooltipPos.y = center.y - tooltipSize.h;
			}
			else if (position.placement == Placement::SouthWest || position.placement == Placement::South || position.placement == Placement::SouthEast)
			{
				if (m_size.h - center.bottom() < tooltipSize.h)
					tooltipPos.y = center.y - tooltipSize.h;
				else
					tooltipPos.y = center.y + center.h;
			}
			else
			{
				tooltipPos.y = (center.y + center.h / 2) - tooltipSize.h / 2;
			}

			// Take care of horizontal position

			if (position.placement == Placement::NorthWest || position.placement == Placement::West || position.placement == Placement::SouthWest)
			{
				if (position.placement == Placement::West && center.x < tooltipSize.w)
					tooltipPos.x = center.x + center.w;
				else
					tooltipPos.x = center.x - tooltipSize.w;
			}
			else if (position.placement == Placement::NorthEast || position.placement == Placement::East || position.placement == Placement::SouthEast)
			{
				if (position.placement == Placement::East && m_size.w - center.right() < tooltipSize.w)
					tooltipPos.x = center.x - tooltipSize.w;
				else
					tooltipPos.x = center.x + center.w;
			}
			else
			{
				tooltipPos.x = (center.x + center.w / 2) - tooltipSize.w / 2;
			}

			// Make sure to stay inside screen (if possible).

			limit(tooltipPos.x, 0, m_size.w - tooltipSize.w);
			limit(tooltipPos.y, 0, m_size.h - tooltipSize.h);

			//

			m_tooltipSlot._setGeo({ align(tooltipPos), tooltipSize }, m_scale);
			_requestRender( pWidget->_renderBounds() + m_tooltipSlot.m_geo.pos() );
		}
	}

	//____ _closeTooltip() ____________________________________________________

	void TooltipOverlay::_closeTooltip()
	{
		if (!m_tooltipSlot.isEmpty())
		{
			if (!m_tooltipSlot.m_geo.isEmpty())
			{
				_requestRender(m_tooltipSlot._widget()->_renderBounds() + m_tooltipSlot.m_geo.pos());
				m_tooltipSlot.m_geo.clear();
			}

			m_tooltipSlot._setWidget(nullptr);
			m_hoverCountdown = m_hoverMillisec;			
		}
	}


	//____ _receive() _________________________________________________________

	void TooltipOverlay::_receive(Msg* _pMsg)
	{
		Overlay::_receive(_pMsg);

		switch (_pMsg->type())
		{
		case MsgType::MouseEnter:
			_startReceiveUpdates();
			// No break, fall through to next case on purpose.
		case MsgType::MouseMove:
		{
			CoordSPX mousePos = _toLocal(static_cast<InputMsg*>(_pMsg)->pointerSpxPos());
			Widget* pHovered = _findWidget(mousePos, SearchMode::ActionTarget);

			if (pHovered->tooltip().isEmpty())
				pHovered = nullptr;

			if (!m_tooltipSlot.isEmpty())
			{
				if (pHovered != m_pHoverWidget)
				{
					_closeTooltip();

					m_pHoverWidget = pHovered;
					m_hoverPos = mousePos;
					m_hoverCountdown = m_hoverMillisec;
				}
				else if (abs(mousePos.x - m_hoverPos.x) > m_openTooltipMaxMovement || abs(mousePos.y - m_hoverPos.y) > m_openTooltipMaxMovement)
				{
					_closeTooltip();
					m_bBlockReopen = true;
				}
			}
			else
			{
				if (pHovered != m_pHoverWidget)
				{
					m_bBlockReopen = false;
					m_pHoverWidget = pHovered;
				}

				m_hoverPos = mousePos;
				m_hoverCountdown = m_hoverMillisec;
			}
			break;
		}
		case MsgType::MouseLeave:
			_stopReceiveUpdates();
			m_hoverPos = { -1,-1 };
			break;

		default:
			break;
		}
	}

	//____ _update() __________________________________________________________

	void TooltipOverlay::_update(int microPassed, int64_t microsecTimestamp)
	{
		if (Base::inputHandler()->isAnyButtonPressed())
			return;

		if (m_hoverCountdown > 0 && m_pHoverWidget && !m_bBlockReopen)
		{
			m_hoverCountdown -= microPassed / 1000;
			if (m_hoverCountdown <= 0)
			{
				m_hoverCountdown = 0;
				_openTooltip();
			}
		}


	}

	//____ _slotTypeInfo() ____________________________________________________

	const TypeInfo& TooltipOverlay::_slotTypeInfo(const StaticSlot* pSlot) const
	{
		if (pSlot == &mainSlot)
			return DynamicSlot::TYPEINFO;

		return Slot::TYPEINFO;
	}

	//____ _findWidget() ______________________________________________________

	Widget* TooltipOverlay::_findWidget(const CoordSPX& ofs, SearchMode mode)
	{
		if (!RectSPX(m_size).contains(ofs))
			return nullptr;

		if (mode == SearchMode::ActionTarget)
		{
			if (!mainSlot.isEmpty())
			{
				//TODO: Take padding into account

				Widget* pChild = mainSlot._widget();			
				if (pChild->isContainer())
					return static_cast<Container*>(pChild)->_findWidget(ofs, mode);
				else if (pChild->_markTest(ofs))
					return pChild;
			}
			return nullptr;
		}
		else
			return Overlay::_findWidget(ofs, mode);
	}

	//____ _childRequestResize() ______________________________________________

	void TooltipOverlay::_childRequestResize(StaticSlot* pSlot)
	{
		if (pSlot == &mainSlot)
			_requestResize();
		else
			_updateTooltipGeo();
	}

	//____ _beginOverlaySlots() _________________________________________________

	const Overlay::Slot* TooltipOverlay::_beginOverlaySlots() const
	{
		return &m_tooltipSlot;
	}

	//____ _endOverlaySlots() ______________________________________________________

	const Overlay::Slot* TooltipOverlay::_endOverlaySlots() const
	{
		if (m_tooltipSlot.isEmpty())
			return &m_tooltipSlot;

		return &m_tooltipSlot + 1;
	}

	//____ _sizeOfOverlaySlot() ____________________________________________________

	int	TooltipOverlay::_sizeOfOverlaySlot() const
	{
		return sizeof(Slot);
	}

	//____ _defaultTooltipGenerator() ____________________________________________

	Widget_p TooltipOverlay::_defaultTooltipGenerator(Position& position, const Widget* pHovered, const Border& margin)
	{
		BoxSkin_p pSkin = BoxSkin::create(1, Color::LightYellow, Color::Black, 2);
		auto pWidget = TextDisplay::create();
		pWidget->setSkin(pSkin);

		pWidget->display.setText(pHovered->tooltip());
		return pWidget;
	}
}
