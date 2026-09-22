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
#ifndef	WG_DEBUGBACKEND_DOT_H
#define WG_DEBUGBACKEND_DOT_H
#pragma once

#include <map>
#include <functional>

#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_widget.h>
#include <wg_infosection.h>
#include <wg_inspectorview.h>
#include <wg_idebugcontext.h>

#include <wg_objectinspector.h>
#include <wg_widgetinspector.h>
#include <wg_skininspector.h>
#include <wg_widgettreeview.h>
#include <wg_msglogviewer.h>

namespace wg
{
	class DebugBackend;
	typedef	StrongPtr<DebugBackend>		DebugBackend_p;
	typedef	WeakPtr<DebugBackend>		DebugBackend_wp;


	class DebugBackend : public Object , protected IDebugContext
	{
	public:
		//.____ Creation __________________________________________

		static DebugBackend_p	create() { return DebugBackend_p(new DebugBackend()); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc __________________________________________


		void			setTheme(const DebugTheme& theme);
		const DebugTheme& theme() override;

		Widget_p		createObjectInfoSection( const TypeInfo * pType, Object * pObject ) override;
		Widget_p		createSlotInfoSection( const TypeInfo * pType, StaticSlot * pSlot ) override;
		Widget_p		createComponentInfoSection( const TypeInfo* pType, Component* pComponent ) override;

		ObjectInspector_p	createObjectInspector(Object* pObject);
		WidgetInspector_p	createWidgetInspector(Widget* pWidget);
		SkinInspector_p		createSkinInspector(Skin* pSkin);
		WidgetTreeView_p	createWidgetTreeView(Widget * pWidget);
		MsgLogViewer_p		createMsgLogViewer();

		void			setObjectSelectedCallback(std::function<void(Object*,Object*)> pCallback);



	protected:
		DebugBackend();
		~DebugBackend() {}

		void			objectSelected(Object* pSelected, Object* pCaller) override;

		DebugTheme	m_theme;


		std::map<const TypeInfo*,Widget_p(*)(const DebugTheme&, IDebugContext *, Object *)>	m_objectInfoFactories;
		std::map<const TypeInfo*,Widget_p(*)(const DebugTheme&, IDebugContext *, StaticSlot *)>	m_slotInfoFactories;
	 	std::map<const TypeInfo*, Widget_p(*)(const DebugTheme&, IDebugContext *, Component*)>	m_componentInfoFactories;

		std::vector<const TypeInfo*>	m_ignoreClasses;

		std::function<void(Object*, Object*)> m_objectSelectedCallback;
	};


} // namespace wg
#endif //WG_DEBUGBACKEND_DOT_H

