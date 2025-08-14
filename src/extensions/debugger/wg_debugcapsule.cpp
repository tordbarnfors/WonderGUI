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

#include <wg_debugcapsule.h>
#include <wg_debugfrontend.h>
#include <wg_msg.h>

namespace wg
{
	using namespace Util;

	const TypeInfo DebugCapsule::TYPEINFO = { "DebugCapsule", &Capsule::TYPEINFO };

	//____ create() ______________________________________________________________

	DebugCapsule_p DebugCapsule::create(const Blueprint& blueprint)
	{
		return DebugCapsule_p(new DebugCapsule(blueprint));
	}

	//____ destructor _____________________________________________________________

	DebugCapsule::~DebugCapsule()
	{
		m_pFrontend->_removeDebugCapsule(this);
		_stopReceiveUpdates();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ pointerStyle() ________________________________________________________

	PointerStyle DebugCapsule::pointerStyle() const
	{
		if( m_bInSelectMode )
			return PointerStyle::Crosshair;
		else
			return Capsule::pointerStyle();
	}

	//____ _receive() ____________________________________________________________

	void DebugCapsule::_receive(Msg* _pMsg)
	{
		switch( _pMsg->type() )
		{
			case MsgType::MouseClick:
			{
				if( m_bInSelectMode )
				{
					auto pMsg = static_cast<MouseClickMsg*>(_pMsg);
					CoordSPX mousePos = _toLocal(pMsg->pointerSpxPos());

					auto pContainer = wg_dynamic_cast<Container_p>(slot._widget());
					if (pContainer)
					{
						Widget * pWidget = pContainer->_findWidget(mousePos, SearchMode::MarkPolicy);
						if (pWidget && m_pFrontend )
							m_pFrontend->selectObject(pWidget,this);
					}
					break;									// Press on us is ignored
				}
			}

			default:
				break;
		}

		Capsule::_receive(_pMsg);
	}

	//____ _update() _____________________________________________________________

	void DebugCapsule::_update(int microPassed, int64_t microsecTimestamp)
	{
		RectSPX	currentArea = _selectionArea();

		if( currentArea != m_renderedSelectionArea )
		{
			_requestRender(currentArea);
			_requestRender(m_renderedSelectionArea);
		}
	}

	//____ _render() _____________________________________________________________

	void DebugCapsule::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Capsule::_render(pDevice,_canvas,_window);

		m_renderedSelectionArea = _selectionArea();

		if( m_pSelectionSkin && !m_renderedSelectionArea.isEmpty() )
			m_pSelectionSkin->_render(pDevice, m_renderedSelectionArea, m_scale, State::Default);
	}


	//____ _findWidget() ____________________________________________________________

	Widget * DebugCapsule::_findWidget(const CoordSPX& ofs, SearchMode mode)
	{
		if (m_bInSelectMode)
			return this;
		else
			return Container::_findWidget(ofs, mode);
	}

	//____ _widgetSelected() _____________________________________________________

	void DebugCapsule::_widgetSelected(Widget * pWidget)
	{
		if( pWidget->isDescendantOf(this))
			m_pSelectedWidget = pWidget;
		else
			m_pSelectedWidget = nullptr;
	}

	//____ _setSelectMode() ______________________________________________________

	void DebugCapsule::_setSelectMode(bool bSelectMode)
	{
		m_bInSelectMode = bSelectMode;
	}

	//____ _selectionArea() __________________________________________________________

	RectSPX DebugCapsule::_selectionArea()
	{
		if( m_pSelectedWidget && m_pSelectedWidget->isDescendantOf(this) )
			return (m_pSelectedWidget->_globalGeo() - _globalGeo().pos()) + m_pSelectionSkin->_contentBorder(m_scale, State::Default);

		return RectSPX();
	}


} // namespace wg

