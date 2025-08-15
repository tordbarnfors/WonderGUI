
/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_DEBUGFRONTEND_DOT_H
#define WG_DEBUGFRONTEND_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_debugbackend.h>
#include <wg_debugfrontendwindow.h>

#include <windows/wg_objectinspector.h>
#include <windows/wg_widgetinspector.h>
#include <windows/wg_skininspector.h>
#include <windows/wg_widgettreeview.h>



namespace wg
{
	class DebugCapsule;
	typedef	StrongPtr<DebugCapsule>	DebugCapsule_p;
	typedef	WeakPtr<DebugCapsule>	DebugCapsule_wp;


	class DebugFrontend;
	typedef	StrongPtr<DebugFrontend>	DebugFrontend_p;
	typedef	WeakPtr<DebugFrontend>	DebugFrontend_wp;

	//____ DebugFrontend __________________________________________________________

	class DebugFrontend : public Capsule
	{

	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			DebugBackend_p	backend;									// Mandatory!!!
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			Theme_p			theme;										// Mandatory!!!
			String			tooltip;
			bool			usePickHandles = false;
			Surface_p		icons;										// Mandatory!!!
			Surface_p		transparencyGrid;							// Mandatory!!! Chessboard pattern or similar
		};

		//.____ Creation __________________________________________

		static DebugFrontend_p	create( const Blueprint& blueprint ) { return DebugFrontend_p(new DebugFrontend(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ________________________________________________

		void		activate();		// Call when debugfrontend's window is recreated.
		void		deactivate();	// Call when debugfrontend's window is deleted.

		void		selectObject(Object* pSelected, Object * pSelectedFrom);

		void		setSelectMode(bool selectMode);
		bool		selectMode() const { return m_bSelectMode; }

		//.____ Internal ________________________________________________

		void		_addDebugCapsule( DebugCapsule * pCapsule );
		void		_removeDebugCapsule( DebugCapsule * pCapsule );


	protected:
		DebugFrontend(const Blueprint& blueprint);
		virtual ~DebugFrontend();

		Widget_p	_createToolbox();
		void 		_createDebuggerBP();

		void		_createResources();
		void		_setupGUI();

		void		_refreshTreeSelector();


		void		_addWorkspaceWindow( Object * pObject, bool bReuse );


		DebugBackend_p		m_pBackend;
		std::vector<DebugCapsule*>	m_capsules;

		std::vector<DebugFrontendWindow_p>	m_widgetWindows;
		std::vector<DebugFrontendWindow_p>	m_skinWindows;
		std::vector<DebugFrontendWindow_p>	m_objectWindows;

		PackPanel_p			m_pWorkspace;
		SelectBox_p			m_pTreeSelector;
		Capsule_p			m_pTreeViewCapsule;


		// Modes

		bool			m_bSelectMode = false;


		// Resources

		Theme_p			m_pTheme;
		Surface_p		m_pIcons;
		Surface_p		m_pTransparencyGrid;

		Skin_p			m_pRefreshIcon;
		Skin_p			m_pSelectIcon;
		Skin_p			m_pExpandIcon;
		Skin_p			m_pCondenseIcon;

		IDebugger::Blueprint	m_debugPanelBP;
	};


} // namespace wg
#endif //WG_DEBUGFRONTEND_DOT_H
