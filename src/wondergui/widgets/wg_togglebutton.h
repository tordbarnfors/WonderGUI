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

#ifndef	WG_TOGGLEBUTTON_DOT_H
#define	WG_TOGGLEBUTTON_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_text.h>
#include <wg_icon.h>
#include <wg_togglegroup.h>

namespace wg
{

	class	Surface;

	class ToggleButton;
	typedef	StrongPtr<ToggleButton>		ToggleButton_p;
	typedef	WeakPtr<ToggleButton>	ToggleButton_wp;


	/**
	 * @brief Combined ToggleButton, Checkbox and RadioButton widget.
	 *
	 * ToggleButton is a button widget that toggles its checked-state each time it is pressed.
	 * Press once to check, press again to uncheck. ToggleButton is also used for Checkboxes
	 * and RadioButtons since these are technically just ToggleButtons with different skinning
	 * and possibly (depending on exact behavior desired) a different ClickArea.
	 *
	 * The ToggleButton is built up by three different visual elements: The button itself,
	 * the label and the icon. A typical StateButton will have the button itself skinned and
	 * the label set to some suitable text while a CheckBox and RadioButton will leave the button
	 * unskinned and use the icon and label for its visual representation.
	 *
	 **/

	class	ToggleButton : public Widget
	{
	friend class ToggleGroup;
	public:
		enum class ClickArea
		{
			Default,		///< Full geometry of icon (no alpha test) + text + area between + alpha test on background.
			Alpha,			///< Alpha test on background and icon.
			Geo,			///< Full geometry of Widget is clickable.
			Icon,			///< Only the icon (alpha test) is clickable.
			Text			///< Only the text (no alpha test) is clickable.
		};

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			checked = false;
			ClickArea		clickArea = ClickArea::Default;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			bool			flipOnRelease = false;
			Icon::Blueprint	icon;
			int				id = 0;
			Text::Blueprint	label;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static ToggleButton_p	create() { return ToggleButton_p(new ToggleButton()); }
		static ToggleButton_p	create(const Blueprint& blueprint) { return ToggleButton_p(new ToggleButton(blueprint)); }

		//.____ Components ____________________________________

		Text		label;
		Icon		icon;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior _______________________________________________________

	 	void				setClickArea( ClickArea clickArea );
		inline ClickArea	clickArea() const;

		void				setFlipOnRelease( bool bFlipOnRelease );
		inline bool			flipOnRelease();

		//.____ Misc _________________________________________________________________

		inline ToggleGroup_p toggleGroup() const;

		//.____ Geometry _________________________________________________

		SizeSPX			_defaultSize(int scale) const override;


	protected:
		ToggleButton();
		template<class BP> ToggleButton(const BP& bp) : label(this), icon(this), Widget(bp)
		{
			label._initFromBlueprint(bp.label);
			icon._initFromBlueprint(bp.icon);

			m_clickArea		= bp.clickArea;
			m_bFlipOnRelease = bp.flipOnRelease;

			if (bp.checked)
				setChecked(true);
		}
		
		virtual ~ToggleButton();

		void		_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void		_receive( Msg * pMsg ) override;
		void		_resize( const SizeSPX& size, int scale ) override;
		bool		_alphaTest( const CoordSPX& ofs ) override;
		void		_setState(State state) override { _setState(state, false); }
		void		_setState( State state, bool bPostMsg );

		void		_setToggleGroup( ToggleGroup * pGroup );
		ToggleGroup * _toggleGroup() const { return m_pToggleGroup.rawPtr(); }

		CoordSPX	_componentPos( const Component * pComponent ) const override;
		SizeSPX		_componentSize( const Component * pComponent ) const override;
		RectSPX		_componentGeo( const Component * pComponent ) const override;



	private:
		class IconAccess : public Icon { friend class ToggleButton; };
		const IconAccess& _icon() const { return reinterpret_cast<const IconAccess&>(icon); }
		IconAccess& _icon() { return reinterpret_cast<IconAccess&>(icon); }


//		void	_refreshTextArea();
		bool	_markTestTextArea( const CoordSPX& pos );

		bool			m_bPressed 			= false;						// Set when mouse is pressed and over.
		bool			m_bReturnPressed	= false;
		bool			m_bFlipOnRelease;				// Set if we want to flip StateButton on press (default), not click.

		ToggleGroup_p	m_pToggleGroup;

		ClickArea		m_clickArea;
	};


	inline ToggleButton::ClickArea ToggleButton::clickArea() const
	{
		return m_clickArea;
	}

	inline void ToggleButton::setClickArea( ClickArea clickArea )
	{
		m_clickArea = clickArea;
	}

	inline bool ToggleButton::flipOnRelease()
	{
		return m_bFlipOnRelease;
	}

	inline ToggleGroup_p ToggleButton::toggleGroup() const
	{
		return m_pToggleGroup;
	}




} // namespace wg
#endif //	WG_TOGGLEBUTTON_DOT_H
