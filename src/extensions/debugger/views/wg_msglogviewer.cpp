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
#include "wg_msglogviewer.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>
#include <wg_filler.h>

#include <wg_colorskin.h>
#include <wg_blockskin.h>
#include <wg_rootpanel.h>

#include <wg_msgrouter.h>
#include <wg_msg.h>
#include <wg_msglogger.h>

#include <wg_debugcontextswitch.h>


namespace wg
{

	const TypeInfo MsgLogViewer::TYPEINFO = { "MsgLogViewer", &InspectorView::TYPEINFO };


	//____ constructor _____________________________________________________________

	MsgLogViewer::MsgLogViewer(const DebugTheme& bp, IDebugContext* pContext) : InspectorView(bp,pContext)
	{
		m_title = "Message Log";

		m_pMainPanel = PackPanel::create(WGBP(PackPanel,
			_.axis = Axis::Y));

		m_pPackLayout = PackLayout::create({});

		//

		auto pButtonRow = PackPanel::create(WGBP(PackPanel,
			_.axis = Axis::X,
			_.layout = m_pPackLayout,
			_.skin = dbgkit::Skins::Plate));


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


		m_pRecordButton = WGCREATE(dbgkit::ToggleButton, _.icon.skin = pRecordIcon);
		m_pClearButton = WGCREATE(dbgkit::Button, _.icon.skin = pClearIcon	);
		m_pSourceSelector = WGCREATE(dbgkit::SelectBox);

		auto pPadding = WGCREATE(Filler, _.defaultSize = { 20,0 });

		m_pLogMoveToggle = WGCREATE(dbgkit::ToggleButton, _.checked = true, _.icon.skin = pPointerIcon);
		m_pLogDragToggle = WGCREATE(dbgkit::ToggleButton, _.checked = true, _.icon.skin = pDragIcon);
		m_pLogButtonToggle = WGCREATE(dbgkit::ToggleButton, _.checked = true, _.icon.skin = pButtonIcon);
		m_pLogKeysToggle = WGCREATE(dbgkit::ToggleButton, _.checked = true, _.icon.skin = pKeyIcon);
		m_pLogPointerStyleToggle = WGCREATE(dbgkit::ToggleButton, _.checked = true, _.icon.skin = pPointerStyleIcon);

		Base::msgRouter()->addRoute(m_pRecordButton, MsgType::Toggle, [this](Msg* _pMsg) {

			auto pMsg = static_cast<ToggleMsg*>(_pMsg);
			setRecording(pMsg->isChecked());
			});

		Base::msgRouter()->addRoute(m_pClearButton, MsgType::Select, [this](Msg* _pMsg) {

			clear();
			});

		Base::msgRouter()->addRoute(m_pSourceSelector, MsgType::Select, [this](Msg* _pMsg) {

			int id = m_pSourceSelector->selectedEntryId();
			for( auto& pSource : m_sources )
			{
				if( pSource->id == id )
				{
					_show(pSource.get());
					break;
				}
			}
			});

		// The log toggles apply to every source's logger.

		auto applyToLoggers = [this](Msg* _pMsg) {
			for( auto& pSource : m_sources )
				if( pSource->pLogger )
					_applyLogSettings(pSource->pLogger);
		};

		for( auto& pToggle : { m_pLogMoveToggle, m_pLogDragToggle, m_pLogButtonToggle, m_pLogKeysToggle, m_pLogPointerStyleToggle } )
			Base::msgRouter()->addRoute(pToggle, MsgType::Toggle, applyToLoggers);


		pButtonRow->slots.pushBack( {m_pRecordButton, m_pClearButton, m_pSourceSelector, pPadding, m_pLogMoveToggle, m_pLogDragToggle, m_pLogButtonToggle, m_pLogKeysToggle, m_pLogPointerStyleToggle });

		m_pMainPanel->slots.pushBack(pButtonRow, WGBP(PackPanelSlot, _.weight = 0.f ) );

		//

		m_pLogWindow = dbgkit::ScrollCapsuleXY::create();
		
		m_pLogList = PackPanel::create( WGBP(PackPanel, 
			_.axis = Axis::Y,
			_.layout = m_pPackLayout,
			_.skin = dbgkit::Skins::Canvas ) );

		m_pLogWindow->slot = m_pLogList;

		m_pMainPanel->slots.pushBack(m_pLogWindow, WGBP(PackPanelSlot, _.weight = 1.f));

		m_entrySkin[0] = ColorSkin::create(HiColor::Transparent, 2);
		m_entrySkin[1] = ColorSkin::create(HiColor(Color::Blue).withAlpha(1024), 2);

		slot = m_pMainPanel;

		setSkin(nullptr);

		// New lines are moved from the logs to the list in _update().

		_startReceiveUpdates();
	}

	//____ Destructor ______________________________________________________________

	MsgLogViewer::~MsgLogViewer()
	{
		// A logger routed to stays alive and calls back to us, so end every
		// broadcast before we're gone.

		for( auto& pSource : m_sources )
			_stopRecording(pSource.get());

		_stopReceiveUpdates();
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

		Source * pSource = m_pShownSource;
		if( !pSource )
			return;

		if( pSource->pKey )
		{
			pSource->lines.clear();
			pSource->nbLinesLogged = 0;
			m_nbLinesShown = 0;
			return;
		}

		// A closed source is done with once cleared.

		auto& entries = m_pSourceSelector->entries;
		for( int i = 0 ; i < entries.size() ; i++ )
		{
			if( entries[i].id() == pSource->id )
			{
				entries.erase(i);
				break;
			}
		}

		m_sources.erase( std::find_if(m_sources.begin(), m_sources.end(), [pSource](auto& p) { return p.get() == pSource; }) );

		m_pShownSource = nullptr;
		if( !m_sources.empty() )
		{
			_show(m_sources.back().get());
			m_pSourceSelector->selectEntryById(m_pShownSource->id);
		}
	}

	//____ setRecording() _________________________________________________________

	void MsgLogViewer::setRecording(bool bRecording)
	{
		m_bRecording = bRecording;

		for( auto& pSource : m_sources )
		{
			if( bRecording )
				_startRecording(pSource.get());
			else
				_stopRecording(pSource.get());
		}

		if( m_pRecordButton->isChecked() != bRecording )
			m_pRecordButton->setChecked(bRecording);
	}

	//____ addSource() ___________________________________________________________

	void MsgLogViewer::addSource( const Object * pKey, const std::string& name, GUIContext * pContext, std::function<bool(const Msg * pMsg)> pFilter )
	{
		if( !pKey || _findSource(pKey) )
			return;

		auto pSource = new Source();
		m_sources.emplace_back(pSource);

		pSource->pKey = pKey;
		pSource->id = m_nextSourceId++;
		pSource->name = name;

		{
			// The router belongs to the source's context, not necessarily ours.

			DebugContextSwitch contextSwitch(pContext);
			pSource->pRouter = Base::msgRouter();
		}

		// Called while the source's context dispatches its messages, so only
		// store the line here. _update() puts it on screen.

		pSource->pLogger = MsgLogger::create([pSource](const char* pString) {

			pSource->lines.push_back(pString);
			pSource->nbLinesLogged++;

			if( pSource->lines.size() > c_maxLines )
				pSource->lines.pop_front();
		});

		if( pFilter )
			pSource->pLogger->setFilter(pFilter);

		_applyLogSettings(pSource->pLogger);

		m_pSourceSelector->entries.pushBack(WGBP(SelectBoxEntry, _.id = pSource->id, _.text = name.c_str()));

		if( m_bRecording )
			_startRecording(pSource);

		if( !m_pShownSource )
			showSource(pKey);
	}

	//____ closeSource() _________________________________________________________

	void MsgLogViewer::closeSource( const Object * pKey )
	{
		Source * pSource = _findSource(pKey);
		if( !pSource )
			return;

		_stopRecording(pSource);

		pSource->pKey = nullptr;
		pSource->pRouter = nullptr;
		pSource->pLogger = nullptr;

		_setEntryText(pSource);
	}

	//____ showSource() __________________________________________________________

	void MsgLogViewer::showSource( const Object * pKey )
	{
		Source * pSource = _findSource(pKey);
		if( pSource && pSource != m_pShownSource )
		{
			_show(pSource);
			m_pSourceSelector->selectEntryById(pSource->id);
		}
	}

	//____ _update() _____________________________________________________________

	void MsgLogViewer::_update(int microPassed, int64_t microsecTimestamp)
	{
		Source * pSource = m_pShownSource;
		if( !pSource || pSource->nbLinesLogged == m_nbLinesShown )
			return;

		// Lines logged since last time, of which only the latest are still kept.

		int64_t nbNew = std::min( pSource->nbLinesLogged - m_nbLinesShown, (int64_t) pSource->lines.size() );
		int64_t lineNb = pSource->nbLinesLogged - nbNew;

		for( auto it = pSource->lines.end() - nbNew ; it != pSource->lines.end() ; it++ )
			_appendLine(*it, lineNb++);

		_trimLogList();
		m_nbLinesShown = pSource->nbLinesLogged;
	}

	//____ _findSource() _________________________________________________________

	MsgLogViewer::Source * MsgLogViewer::_findSource( const Object * pKey )
	{
		if( !pKey )
			return nullptr;

		for( auto& pSource : m_sources )
			if( pSource->pKey == pKey )
				return pSource.get();

		return nullptr;
	}

	//____ _show() _______________________________________________________________

	void MsgLogViewer::_show( Source * pSource )
	{
		m_pShownSource = pSource;
		m_pLogList->slots.clear();

		int64_t lineNb = pSource->nbLinesLogged - pSource->lines.size();
		for( auto& line : pSource->lines )
			_appendLine(line, lineNb++);

		m_nbLinesShown = pSource->nbLinesLogged;
	}

	//____ _startRecording() _____________________________________________________

	void MsgLogViewer::_startRecording( Source * pSource )
	{
		if( pSource->routeId != 0 || !pSource->pRouter || !pSource->pLogger )
			return;

		pSource->routeId = pSource->pRouter->broadcastTo(pSource->pLogger);
	}

	//____ _stopRecording() ______________________________________________________

	void MsgLogViewer::_stopRecording( Source * pSource )
	{
		if( pSource->routeId == 0 )
			return;

		if( pSource->pRouter )
			pSource->pRouter->endBroadcast(pSource->routeId);

		pSource->routeId = 0;
	}

	//____ _applyLogSettings() ___________________________________________________

	void MsgLogViewer::_applyLogSettings( MsgLogger * pLogger )
	{
		pLogger->logPointerMsgs(m_pLogMoveToggle->isChecked());
		pLogger->logMsg(MsgType::MouseDrag, m_pLogDragToggle->isChecked());
		pLogger->logMouseButtonMsgs(m_pLogButtonToggle->isChecked());
		pLogger->logKeyboardMsgs(m_pLogKeysToggle->isChecked());
		pLogger->logMsg(MsgType::PointerChange, m_pLogPointerStyleToggle->isChecked());
	}

	//____ _setEntryText() _______________________________________________________

	void MsgLogViewer::_setEntryText( Source * pSource )
	{
		std::string text = pSource->pKey ? pSource->name : pSource->name + " (closed)";

		auto& entries = m_pSourceSelector->entries;
		for( int i = 0 ; i < entries.size() ; i++ )
		{
			if( entries[i].id() == pSource->id )
			{
				bool bSelected = (m_pSourceSelector->selectedEntryId() == pSource->id);

				entries.erase(i);
				entries.insert(i, WGBP(SelectBoxEntry, _.id = pSource->id, _.text = text.c_str()));

				if( bSelected )
					m_pSourceSelector->selectEntryById(pSource->id);
				break;
			}
		}
	}

	//____ _appendLine() _________________________________________________________

	void MsgLogViewer::_appendLine( const std::string& line, int64_t lineNb )
	{
		auto pDisplay = TextDisplay::create(WGBP(TextDisplay,
			_.display.text = line.c_str(),
			_.skin = m_entrySkin[lineNb & 1]));

		m_pLogList->slots.pushBack(pDisplay);
	}

	//____ _trimLogList() ________________________________________________________

	void MsgLogViewer::_trimLogList()
	{
		if (m_pLogList->slots.size() > c_maxLines)
			m_pLogList->slots.erase(0, m_pLogList->slots.size() - c_maxLines);
	}

} // namespace wg


