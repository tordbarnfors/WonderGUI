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
#ifndef WG_POPUPOPENER_DOT_H
#define WG_POPUPOPENER_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_text.h>
#include <wg_icon.h>

namespace wg
{
	class PopupOpener;
	typedef	StrongPtr<PopupOpener>	PopupOpener_p;
	typedef	WeakPtr<PopupOpener>	PopupOpener_wp;



	class PopupOpener : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Placement		attachPoint		= Placement::SouthWest;
			Object_p		baggage;
			bool			closeOnSelect	= true;
			bool			disabled		= false;
			bool			dropTarget		= false;
			Finalizer_p		finalizer		= nullptr;
			Icon::Blueprint	icon;
			int				id				= 0;
			Text::Blueprint label;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			MouseButton		mouseButton		= MouseButton::Left;
			bool			openOnHover		= false;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PointerStyle	pointer			= PointerStyle::Undefined;
			Widget_p		popup;
			Border			popupOverflow;
			bool			selectable		= false;
			Skin_p			skin;
			bool			stickyFocus		= false;
			bool			tabLock			= false;
			String			tooltip;

		};

		//.____ Creation __________________________________________

		static PopupOpener_p	create() { return PopupOpener_p(new PopupOpener()); }
		static PopupOpener_p	create( const Blueprint& blueprint ) { return PopupOpener_p(new PopupOpener(blueprint)); }

		//.____ Components _______________________________________

		Text			label;
		Icon			icon;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior ____________________________________________

		void		setPopup(Widget * pPopup);
		Widget_p	popup() const { return m_pPopup;  }

		void		setOpenOnHover(bool bOpen);
		bool		openOnHover() const { return m_bOpenOnHover; }

		void		setMouseButton(MouseButton button);
		MouseButton	mouseButton() const { return m_mouseButton; }

		void		setAttachPoint(Placement attachPoint);
		Placement	attachPoint() const { return m_attachPoint; }

		//.____ Internal ______________________________________________________

		virtual spx		_matchingHeight(spx width, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;

        bool            isOpen() const { return m_bOpen; }
	protected:
		PopupOpener();

		template<class BP> PopupOpener( const BP& bp ) : label(this), icon(this), Widget(bp)
		{
			icon._initFromBlueprint(bp.icon);
			label._initFromBlueprint(bp.label);

			m_bSelectable	= false;
			m_pPopup		= bp.popup;
			m_bOpenOnHover	= bp.openOnHover;
			m_bCloseOnSelect= bp.closeOnSelect;
			m_attachPoint	= bp.attachPoint;
			m_mouseButton	= bp.mouseButton;
			m_popupOverflow	= bp.popupOverflow;
		}

		virtual ~PopupOpener();

		void			_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		void			_resize(const SizeSPX& size, int scale) override;
		void			_receive(Msg * pMsg) override;
		void			_setState(State state) override;


		void			_open();
		void			_close();

		CoordSPX		_componentPos(const Component * pComponent) const override;
		SizeSPX			_componentSize(const Component * pComponent) const override;
		RectSPX			_componentGeo(const Component * pComponent) const override;

		class IconAccess : public Icon { friend class PopupOpener; };
		const IconAccess& _icon() const { return reinterpret_cast<const IconAccess&>(icon); }
		IconAccess& _icon() { return reinterpret_cast<IconAccess&>(icon); }



		Widget_p		m_pPopup;

		Placement		m_attachPoint = Placement::SouthWest;
		Border			m_popupOverflow;
		bool			m_bOpenOnHover = false;
		bool			m_bOpen = false;
		bool			m_bCloseOnSelect = true;
		MouseButton		m_mouseButton = MouseButton::Left;
	};


}


#endif //WG_POPUPOPENER_DOT_H
