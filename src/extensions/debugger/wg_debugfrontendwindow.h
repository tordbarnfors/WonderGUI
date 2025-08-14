
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

#ifndef WG_DEBUGFRONTENDWINDOW_DOT_H
#define WG_DEBUGFRONTENDWINDOW_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_debugbackend.h>

#include <windows/wg_objectinspector.h>
#include <windows/wg_skininspector.h>
#include <windows/wg_widgettreeview.h>



namespace wg
{

	class DebugFrontendWindow;
	typedef	StrongPtr<DebugFrontendWindow>	DebugFrontendWindow_p;
	typedef	WeakPtr<DebugFrontendWindow>	DebugFrontendWindow_wp;

	//____ DebugFrontendWindow __________________________________________________________

	class DebugFrontendWindow : public Capsule
	{

	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
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
		};

		//.____ Creation __________________________________________

		static DebugFrontendWindow_p	create( const Blueprint& blueprint ) { return DebugFrontendWindow_p(new DebugFrontendWindow(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _________________________________________________

		void		setContent( Widget_p pContent );
		Widget_p	content() const { return m_pContentHolder->slot.widget(); }

		void		setLabel( String label );

	protected:
		DebugFrontendWindow(const Blueprint& blueprint);
		virtual ~DebugFrontendWindow();

		TextDisplay_p	m_pLabel;
		Capsule_p		m_pContentHolder;

	};


} // namespace wg
#endif //WG_DEBUGFRONTENDWINDOW_DOT_H
