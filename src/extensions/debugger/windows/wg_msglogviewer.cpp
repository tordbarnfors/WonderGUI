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
#include "wg_msglogviewer.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>
#include <wg_togglebutton.h>
#include <wg_button.h>
#include <wg_filler.h>

#include <wg_boxskin.h>
#include <wg_colorskin.h>
#include <wg_blockskin.h>

#include <wg_msgrouter.h>
#include <wg_msg.h>
#include <wg_msglogger.h>

namespace wg
{

	const TypeInfo MsgLogViewer::TYPEINFO = { "MsgLogViewer", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	MsgLogViewer::MsgLogViewer(const Blueprint& bp, DebugPanel::Holder* pHolder) : DebugPanel(bp,pHolder)
	{
		m_pMainPanel = PackPanel::create(WGBP(PackPanel,
			_.axis = Axis::Y));

		m_pPackLayout = PackLayout::create({});

		//

		auto pButtonRow = PackPanel::create(WGBP(PackPanel,
			_.axis = Axis::X,
			_.layout = m_pPackLayout,
			_.skin = bp.theme->plateSkin()));


		auto pRecordIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 0,16,16,16 },
			_.axis = Axis::X,
			_.states = { State::Default, State::Checked }));

		auto pTicksIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 32,16,16,16 },
			_.axis = Axis::X));

		auto pPointerIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 48,16,16,16 },
			_.axis = Axis::X));

		auto pDragIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 0,32,16,16 },
			_.axis = Axis::X));

		auto pButtonIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 16,32,16,16 },
			_.axis = Axis::X));

		auto pKeyIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 32,32,16,16 },
			_.axis = Axis::X));

		auto pPointerStyleIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 0,48,20,16 },
			_.axis = Axis::X));

		auto pClearIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = bp.icons,
			_.firstBlock = { 32,48,16,16 },
			_.axis = Axis::X));


		m_pRecordButton = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.icon.skin = pRecordIcon));
		m_pClearButton = Button::create(WGOVR(bp.theme->pushButton(), _.icon.skin = pClearIcon));

		auto pPadding = Filler::create(WGBP(Filler, _.defaultSize = { 20,0 }));

		m_pLogMoveToggle = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.checked = true, _.icon.skin = pPointerIcon));
		m_pLogDragToggle = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.checked = true, _.icon.skin = pDragIcon));
		m_pLogButtonToggle = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.checked = true, _.icon.skin = pButtonIcon));
		m_pLogKeysToggle = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.checked = true, _.icon.skin = pKeyIcon));
		m_pLogPointerStyleToggle = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.checked = true, _.icon.skin = pPointerStyleIcon));

		Base::msgRouter()->addRoute(m_pRecordButton, MsgType::Toggle, [this](Msg* _pMsg) {

			auto pMsg = static_cast<ToggleMsg*>(_pMsg);
			if (pMsg->isChecked())
				this->m_routeId = Base::msgRouter()->broadcastTo(m_pLogger);
			else
				Base::msgRouter()->endBroadcast(m_routeId);
			});

		Base::msgRouter()->addRoute(m_pClearButton, MsgType::Select, [this](Msg* _pMsg) {

			m_pLogList->slots.clear();
			});

		Base::msgRouter()->addRoute(m_pLogMoveToggle, MsgType::Toggle, [this](Msg* _pMsg) {
			m_pLogger->logPointerMsgs(static_cast<ToggleMsg*>(_pMsg)->isChecked());
			});

		Base::msgRouter()->addRoute(m_pLogDragToggle, MsgType::Toggle, [this](Msg* _pMsg) {
			m_pLogger->logMsg(MsgType::MouseDrag, static_cast<ToggleMsg*>(_pMsg)->isChecked());
			});

		Base::msgRouter()->addRoute(m_pLogButtonToggle, MsgType::Toggle, [this](Msg* _pMsg) {
			m_pLogger->logMouseButtonMsgs(static_cast<ToggleMsg*>(_pMsg)->isChecked());
			});

		Base::msgRouter()->addRoute(m_pLogKeysToggle, MsgType::Toggle, [this](Msg* _pMsg) {
			m_pLogger->logKeyboardMsgs(static_cast<ToggleMsg*>(_pMsg)->isChecked());
			});

		Base::msgRouter()->addRoute(m_pLogPointerStyleToggle, MsgType::Toggle, [this](Msg* _pMsg) {
			m_pLogger->logMsg(MsgType::PointerChange, static_cast<ToggleMsg*>(_pMsg)->isChecked());
			});


		pButtonRow->slots.pushBack( {m_pRecordButton, m_pClearButton, pPadding, m_pLogMoveToggle, m_pLogDragToggle, m_pLogButtonToggle, m_pLogKeysToggle, m_pLogPointerStyleToggle });

		m_pMainPanel->slots.pushBack(pButtonRow, WGBP(PackPanelSlot, _.weight = 0.f ) );

		//

		m_pLogWindow = ScrollPanel::create(bp.theme->scrollPanelXY());
		
		m_pLogList = PackPanel::create( WGBP(PackPanel, 
			_.axis = Axis::Y,
			_.layout = m_pPackLayout,
			_.skin = bp.theme->canvasSkin() ) );

		m_pLogWindow->slot = m_pLogList;

		m_pMainPanel->slots.pushBack(m_pLogWindow, WGBP(PackPanelSlot, _.weight = 1.f));

		m_entrySkin[0] = ColorSkin::create(HiColor::Transparent, 2);
		m_entrySkin[1] = ColorSkin::create(HiColor(Color::Blue).withAlpha(1024), 2);

		//

		m_pLogger = MsgLogger::create([this](const char* pString) {

			auto pDisplay = TextDisplay::create(WGBP(TextDisplay,
				_.display.text = pString,
				_.skin = m_entrySkin[0]));

			std::swap(m_entrySkin[0], m_entrySkin[1]);

			m_pLogList->slots.pushBack(pDisplay);

			if (m_pLogList->slots.size() > 200)
				m_pLogList->slots.erase(0, m_pLogList->slots.size() - 200);
		});


		slot = m_pMainPanel;

		setSkin(nullptr);
	}

	//____ Destructor ______________________________________________________________

	MsgLogViewer::~MsgLogViewer()
	{
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& MsgLogViewer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ clear() ___________________________________________________________

	void MsgLogViewer::clear()
	{
		m_pLogList->slots.clear();
	}
	
	//____ setRecording() _________________________________________________________
	
	void MsgLogViewer::setRecording(bool bRecording)
	{
		if (bRecording)
			this->m_routeId = Base::msgRouter()->broadcastTo(m_pLogger);
		else
			Base::msgRouter()->endBroadcast(m_routeId);

		m_pRecordButton->setChecked(bRecording);
	}

} // namespace wg


