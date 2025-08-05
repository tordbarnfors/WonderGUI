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

#ifndef	WG_MSGLOGVIEWER_DOT_H
#define WG_MSGLOGVIEWER_DOT_H
#pragma once

#include <wg_debugwindow.h>
#include <wg_packpanel.h>
#include <wg_texteditor.h>
#include <wg_msglogger.h>
#include <wg_scrollpanel.h>

namespace wg
{
	class MsgLogViewer;
	typedef	StrongPtr<MsgLogViewer>	MsgLogViewer_p;
	typedef	WeakPtr<MsgLogViewer>	MsgLogViewer_wp;



	class MsgLogViewer : public DebugWindow
	{
	public:

		//.____ Creation __________________________________________

		static MsgLogViewer_p	create( const Blueprint& blueprint, IDebugger * pHolder ) { return MsgLogViewer_p(new MsgLogViewer(blueprint,pHolder) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________

		void		clear();
		void		setRecording(bool bRecording);


	protected:
		MsgLogViewer(const Blueprint& blueprint, IDebugger* pHolder);
		~MsgLogViewer();

		RouteId				m_routeId;

		MsgLogger_p			m_pLogger;

		PackLayout_p		m_pPackLayout;

		PackPanel_p			m_pMainPanel;

		ToggleButton_p		m_pRecordButton;
		Button_p			m_pClearButton;

		ToggleButton_p		m_pLogMoveToggle;
		ToggleButton_p		m_pLogDragToggle;
		ToggleButton_p		m_pLogButtonToggle;
		ToggleButton_p		m_pLogKeysToggle;
		ToggleButton_p		m_pLogPointerStyleToggle;

		PackPanel_p			m_pLogList;
		ScrollPanel_p		m_pLogWindow;

		Skin_p				m_entrySkin[2];

	};

} // namespace wg
#endif //WG_MSGLOGVIEWER_DOT_H


