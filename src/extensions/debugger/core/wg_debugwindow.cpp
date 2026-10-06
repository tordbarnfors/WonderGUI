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
#include <wg_debugwindow.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

#include <wg_packpanel.h>
#include <wg_renderlayercapsule.h>


namespace wg
{
	using namespace Util;

	const TypeInfo DebugWindow::TYPEINFO = { "DebugWindow", &Capsule::TYPEINFO };


	//____ constructor ____________________________________________________________

	DebugWindow::DebugWindow(const Blueprint& bp) : Capsule(bp)
	{
		m_onClose = bp.onClose;

		auto pMainPanel = WGCREATE(PackPanel, _.axis = Axis::Y, _.skin = dbgkit::Skins::Window );

		auto pLabelRow = WGCREATE(PackPanel, _.axis = Axis::X );

		m_pLabel = WGCREATE(dbgkit::WindowTitleBar, _.pickHandle = true, _.display.text = bp.label );

		auto pCloseButton = WGCREATE(dbgkit::Button, _.label.text = " X " );

		Base::msgRouter()->addRoute(pCloseButton, MsgType::Select, [this](Msg* pMsg){

			if( m_onClose )
				m_onClose(this);
			else
				this->releaseFromParent();
		});


		pLabelRow->slots.pushBack( m_pLabel, WGBP(PackPanelSlot, _.weight = 1.f));
		pLabelRow->slots.pushBack( pCloseButton, WGBP(PackPanelSlot, _.weight = 0.f));

		m_pLabelRow = pLabelRow;

		m_pContentHolder = WGCREATE(RenderLayerCapsule, _.skin = dbgkit::Skins::Canvas );

		pMainPanel->slots.pushBack( pLabelRow, WGBP(PackPanelSlot, _.weight = 0.f));
		pMainPanel->slots.pushBack( m_pContentHolder, WGBP(PackPanelSlot, _.weight = 1.f));

		this->slot = pMainPanel;

		if( bp.inspected )
			m_pInspected = bp.inspected;
	}

	//____ Destructor _____________________________________________________________

	DebugWindow::~DebugWindow()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugWindow::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setContent() __________________________________________________________

	void DebugWindow::setContent( Widget_p pContent )
	{
		m_pContentHolder->slot = pContent;
	}

	//____ setInspected() _____________________________________________________

	void DebugWindow::setInspected(Object_p pInspected)
	{
		m_pInspected = pInspected;
	}

	//____ setLabel() ____________________________________________________________

	void DebugWindow::setLabel( String label )
	{
		m_pLabel->display.setText(label);
	}

	//____ _isFrame() ____________________________________________________________

	bool DebugWindow::_isFrame( Widget * pWidget ) const
	{
		while( pWidget != nullptr && pWidget != this )
		{
			if( pWidget == m_pContentHolder.rawPtr() || pWidget == m_pLabelRow.rawPtr() )
				return false;							// Our content or our title row, not the frame around them.

			pWidget = pWidget->parent();
		}

		return pWidget == this;
	}


} // namespace wg
