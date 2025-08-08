
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
#include <wg_debugger.h>


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
			Debugger_p		backend;									// Mandatory!!!
			Object_p		baggage;
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

		void		selectWidget(Widget * pWidget);
		void		selectSkin(Skin* pSkin);
		void		selectObject(Object* pSelected, Object * pSelectedFrom);

		void		addDebugCapsule( DebugCapsule * pCapsule );
		void		removeDebugCapsule( DebugCapsule * pCapsule );

	protected:
		DebugFrontend(const Blueprint& blueprint);
		virtual ~DebugFrontend();

		void _createDebuggerBP();

		Debugger_p		m_pBackend;

		



		std::vector<DebugCapsule*>	m_capsules;

		// Resources

		Theme_p			m_pTheme;
		Surface_p		m_pIcons;
		Surface_p		m_pTransparencyGrid;

		IDebugger::Blueprint	m_debugPanelBP;
	};


} // namespace wg
#endif //WG_DEBUGFRONTEND_DOT_H
