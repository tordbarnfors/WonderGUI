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
#ifndef WG_BUTTON_DOT_H
#define WG_BUTTON_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_dynamictext.h>
#include <wg_icon.h>

namespace wg
{

	class Button;
	typedef	StrongPtr<Button>		Button_p;
	typedef	WeakPtr<Button>			Button_wp;

	//____ Button ____________________________________________________________
	/**
	 * @brief A normal push button widget.
	 *
	 * A simple push button widget with label and icon.
	 **/

	class Button : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			Icon::Blueprint	icon;
			int				id = 0;
			DynamicText::Blueprint label;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			bool			selectOnPress = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static Button_p	create() { return Button_p(new Button()); }
		static Button_p	create( const Blueprint& blueprint ) { return Button_p(new Button(blueprint)); }

		//.____ Components _______________________________________

		DynamicText	label;
		Icon		icon;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior ______________________________________________________

		void	setSelectOnPress(bool bSelectOnPress);
		bool	selectOnPress() const { return m_bSelectOnPress;  }


		//.____ Internal ______________________________________________________

		spx				_matchingHeight(spx width, int scale) const override;
		SizeSPX			_defaultSize(int scale) const override;


	protected:
		Button();
		
		template<class BP> Button( const BP& bp ) : icon(this), label(this), Widget(bp)
		{
			icon._initFromBlueprint(bp.icon);
			label._initFromBlueprint(bp.label);
			m_bSelectOnPress = bp.selectOnPress;
		}
		
		virtual ~Button();

		//	virtual int		_matchingWidth( int height ) const override;



		void			_receive( Msg * pMsg ) override;
		void			_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool			_alphaTest( const CoordSPX& ofs ) override;
		void			_resize( const SizeSPX& size, int scale ) override;
		void			_setState( State state ) override;


		CoordSPX		_componentPos( const Component * pComponent ) const override;
		SizeSPX			_componentSize( const Component * pComponent ) const override;
		RectSPX			_componentGeo( const Component * pComponent ) const override;

		class IconAccess : public Icon { friend class Button; };
		const IconAccess& _icon() const { return reinterpret_cast<const IconAccess&>(icon); }
		IconAccess& _icon() { return reinterpret_cast<IconAccess&>(icon); }

		bool		m_bReturnPressed	= false;
		bool		m_bSelectOnPress	= false;
		bool		m_bHovered			= false;				// Set when widget actually is hovered.
	};


} // namespace wg
#endif //WG_BUTTON_DOT_H
