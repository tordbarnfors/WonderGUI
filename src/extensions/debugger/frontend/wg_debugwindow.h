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
#ifndef WG_DEBUGWINDOW_DOT_H
#define WG_DEBUGWINDOW_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_debugbackend.h>

#include <wg_objectinspector.h>
#include <wg_skininspector.h>
#include <wg_widgettreeview.h>



namespace wg
{

	class DebugWindow;
	typedef	StrongPtr<DebugWindow>	DebugWindow_p;
	typedef	WeakPtr<DebugWindow>	DebugWindow_wp;

	//____ DebugWindow __________________________________________________________

	class DebugWindow : public Capsule
	{

	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			Widget_p		inspected;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static DebugWindow_p	create( const Blueprint& blueprint ) { return DebugWindow_p(new DebugWindow(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _________________________________________________

		void		setContent( Widget_p pContent );
		Widget_p	content() const { return m_pContentHolder->slot.widget(); }

		void		setInspected(Object_p pInspected);
		Object_p	inspected() const { return m_pInspected; }

		void		setLabel( String label );

		void	setFocused(bool focused)
		{
			if (m_pLabel)
				m_pLabel->setFlagged(focused);
		}

	protected:
		DebugWindow(const Blueprint& blueprint);
		virtual ~DebugWindow();

		TextDisplay_p	m_pLabel;
		Capsule_p		m_pContentHolder;
		Object_p		m_pInspected;

	};


} // namespace wg
#endif //WG_DEBUGWINDOW_DOT_H
