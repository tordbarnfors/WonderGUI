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

#include <functional>

#include <wg_capsule.h>
#include <wg_textdisplay.h>
#include <wg_debugtheme.h>

namespace wg
{

	class DebugWindow;
	typedef	StrongPtr<DebugWindow>	DebugWindow_p;
	typedef	WeakPtr<DebugWindow>	DebugWindow_wp;

	//____ DebugWindow __________________________________________________________
	//
	// The window chrome of the debugger: a title bar with a close button and a
	// holder for the content. Used by both frontends - DebugFrontend puts them
	// in its workspace, DebugOverlay in its window slots - so that a debugger
	// window looks and behaves the same wherever it is shown.
	//
	// The window doesn't position itself, that is up to whoever holds it.
	// Pressing the close button releases the window from its parent unless an
	// onClose callback is set, in which case that decides what closing means.

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
			String			label;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			std::function<void(DebugWindow*)>	onClose;
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

		//.____ Internal ________________________________________________

		// Lets a holder that implements dragging and resizing tell what was hit:
		// the title bar (drag the window), the frame around it (resize the
		// window), or something in the title row or the content (leave it alone,
		// it is a widget of its own).

		Widget_p	_titleBar() const { return m_pLabel; }
		bool		_isFrame( Widget * pWidget ) const;

	protected:
		DebugWindow(const Blueprint& blueprint);
		virtual ~DebugWindow();

		TextDisplay_p	m_pLabel;
		Widget_p		m_pLabelRow;
		Capsule_p		m_pContentHolder;
		Object_p		m_pInspected;

		std::function<void(DebugWindow*)>	m_onClose;
	};


} // namespace wg
#endif //WG_DEBUGWINDOW_DOT_H
