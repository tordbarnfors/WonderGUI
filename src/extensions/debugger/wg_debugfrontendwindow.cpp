/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Bärnfors's WonderGUI Graphics Toolkit
  and copyright (c) Bärnfors Technology AB, Sweden [info@barnfors.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact us [info@barnfors.com] for details.

=========================================================================*/

#include <wg_debugfrontendwindow.h>
#include <wg_debugcapsule.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

#include <wg_packpanel.h>
#include <wg_button.h>
#include <wg_renderlayercapsule.h>


#include <wg_blockskin.h>
#include <wg_colorskin.h>

namespace wg
{
	using namespace Util;

	const TypeInfo DebugFrontendWindow::TYPEINFO = { "DebugFrontendWindow", &Capsule::TYPEINFO };


	//____ constructor ____________________________________________________________

	DebugFrontendWindow::DebugFrontendWindow(const Blueprint& bp) : Capsule(bp)
	{
		auto pMainPanel = WGCREATE(PackPanel, _.axis = Axis::Y, _.skin = bp.theme->windowSkin() );

		auto pLabelRow = WGCREATE(PackPanel, _.axis = Axis::X );

		m_pLabel = WGCREATE(TextDisplay, _ = bp.theme->windowTitleBar(), _.pickHandle = true );

		auto pCloseButton = WGCREATE(Button, _ = bp.theme->pushButton(), _.label.text = "X" );

		Base::msgRouter()->addRoute(pCloseButton, MsgType::Select, [this](Msg* pMsg){
			this->releaseFromParent();
		});


		pLabelRow->slots.pushBack( m_pLabel, WGBP(PackPanelSlot, _.weight = 1.f));
		pLabelRow->slots.pushBack( pCloseButton, WGBP(PackPanelSlot, _.weight = 0.f));

		m_pContentHolder = WGCREATE(RenderLayerCapsule, _.skin = bp.theme->canvasSkin() );

		pMainPanel->slots.pushBack( pLabelRow, WGBP(PackPanelSlot, _.weight = 0.f));
		pMainPanel->slots.pushBack( m_pContentHolder, WGBP(PackPanelSlot, _.weight = 1.f));

		this->slot = pMainPanel;
	}

	//____ Destructor _____________________________________________________________

	DebugFrontendWindow::~DebugFrontendWindow()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugFrontendWindow::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setContent() __________________________________________________________

	void DebugFrontendWindow::setContent( Widget_p pContent )
	{
		m_pContentHolder->slot = pContent;
	}

	//____ setLabel() ____________________________________________________________

	void DebugFrontendWindow::setLabel( String label )
	{
		m_pLabel->display.setText(label);
	}



} // namespace wg

