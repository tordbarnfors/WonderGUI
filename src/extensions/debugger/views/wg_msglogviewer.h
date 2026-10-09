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
#ifndef	WG_MSGLOGVIEWER_DOT_H
#define WG_MSGLOGVIEWER_DOT_H
#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <wg_inspectorview.h>
#include <wg_packpanel.h>
#include <wg_msglogger.h>
#include <wg_msgrouter.h>
#include <wg_scrollcapsule.h>
#include <wg_selectbox.h>

namespace wg
{
	class MsgLogViewer;
	typedef	StrongPtr<MsgLogViewer>	MsgLogViewer_p;
	typedef	WeakPtr<MsgLogViewer>	MsgLogViewer_wp;


	//____ MsgLogViewer _________________________________________________________
	//
	// Logs messages from one or more sources, each with a log of its own, and
	// shows one of them at a time. A source is typically a window: it listens to
	// the message router of that window's GUI context and keeps the messages that
	// pass its filter. A dropdown picks the source shown, as does showSource().
	//
	// While recording, every open source is logged, not just the one shown, so
	// switching between them doesn't lose anything. Each log keeps its latest
	// lines only.

	class MsgLogViewer : public InspectorView
	{
	public:

		//.____ Creation __________________________________________

		static MsgLogViewer_p	create( const DebugTheme& theme, IDebugContext * pContext ) { return MsgLogViewer_p(new MsgLogViewer(theme,pContext) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________

		void		clear();							// Clears the log shown. A closed source is removed.
		void		setRecording(bool bRecording);

		//.____ Sources _______________________________________________
		//
		// pKey identifies the source in later calls. pFilter decides which
		// messages to log, all are logged if it is empty.
		//
		// closeSource() stops logging the source but keeps its log, marked as
		// closed, until it is cleared. A closed source can no longer be reached
		// through its key, so the key may be reused for a new source.

		void		addSource( const Object * pKey, const std::string& name, GUIContext * pContext, std::function<bool(const Msg * pMsg)> pFilter = nullptr );
		void		closeSource( const Object * pKey );
		void		showSource( const Object * pKey );


	protected:
		MsgLogViewer(const DebugTheme& theme, IDebugContext* pContext);
		~MsgLogViewer();

		static const int	c_maxLines = 500;		// Per source.

		struct Source
		{
			const Object *			pKey = nullptr;			// Null once closed.
			int						id = 0;					// Id of its dropdown entry.
			std::string				name;
			MsgRouter_wp			pRouter;
			MsgLogger_p				pLogger;
			RouteId					routeId = 0;			// Non-zero while recording.
			std::deque<std::string>	lines;
			int64_t					nbLinesLogged = 0;		// Since last cleared, including lines dropped.
		};

		void		_update(int microPassed, int64_t microsecTimestamp) override;

		Source *	_findSource( const Object * pKey );
		void		_show( Source * pSource );
		void		_startRecording( Source * pSource );
		void		_stopRecording( Source * pSource );
		void		_applyLogSettings( MsgLogger * pLogger );
		void		_setEntryText( Source * pSource );
		void		_appendLine( const std::string& line, int64_t lineNb );
		void		_trimLogList();

		std::vector<std::unique_ptr<Source>>	m_sources;

		Source *			m_pShownSource = nullptr;
		int64_t				m_nbLinesShown = 0;		// Value of m_pShownSource->nbLinesLogged when last synced.

		bool				m_bRecording = false;
		int					m_nextSourceId = 1;

		PackLayout_p		m_pPackLayout;

		PackPanel_p			m_pMainPanel;

		ToggleButton_p		m_pRecordButton;
		Button_p			m_pClearButton;
		SelectBox_p			m_pSourceSelector;

		ToggleButton_p		m_pLogMoveToggle;
		ToggleButton_p		m_pLogDragToggle;
		ToggleButton_p		m_pLogButtonToggle;
		ToggleButton_p		m_pLogKeysToggle;
		ToggleButton_p		m_pLogPointerStyleToggle;

		PackPanel_p			m_pLogList;
		ScrollCapsule_p		m_pLogWindow;

		Skin_p				m_entrySkin[2];

	};

} // namespace wg
#endif //WG_MSGLOGVIEWER_DOT_H
