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
#ifndef WG_SKINDISPLAY_DOT_H
#define WG_SKINDISPLAY_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_skin.h>

namespace wg
{

	class SkinDisplay;
	typedef	StrongPtr<SkinDisplay>	SkinDisplay_p;
	typedef	WeakPtr<SkinDisplay>		SkinDisplay_wp;

	//____ SkinDisplay ____________________________________________________________
	/**
	 * @brief Simplest kind of widget.
	 *
	 * SkinDisplay is the simplest kind of widget that displays nothing but its skin
	 * and does nothing but posting the normal mouse messages that every widget does.
	 *
	 * It is typically used as backgrounds and for filling out empty spaces in
	 * organized layouts.
	 *
	 **/


	class SkinDisplay : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			Skin_p			displaySkin;
			State			displayState = State::Default;
			float			displayValue = 1.0f;		// Value used by skin, default is 1.0f.
			float			displayValue2 = -1.0f;	// Value2 used by skin, default is -1.0f (unused).
			int				displayScale = 64;		// Scale used by skin, default is 64 (1:1).
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
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

		static SkinDisplay_p	create() { return SkinDisplay_p(new SkinDisplay()); }
		static SkinDisplay_p	create( const Blueprint& blueprint ) { return SkinDisplay_p(new SkinDisplay( blueprint )); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ____________________________________________

		void			setDisplaySkin(Skin_p pSkin);
		Skin_p			displaySkin() const { return m_pDisplaySkin; }

		void			setDisplayState(State state);
		State			displayState() const { return m_displayState; }

		void			setDisplayValue(int value);
		int				displayValue() const { return m_displayValue; }

		void			setDisplayValue2(int value2);
		int				displayValue2() const { return m_displayValue2; }

		void			setDisplayScale(int scale);
		int				displayScale() const { return m_displayScale; }

	protected:
		SkinDisplay();
		virtual ~SkinDisplay();
		
		template<class BP> SkinDisplay(const BP& bp) : Widget(bp)
		{
			m_pDisplaySkin = bp.displaySkin;
			m_displayState = bp.displayState;
			m_displayValue = bp.displayValue;
			m_displayValue2 = bp.displayValue2;
			m_displayScale = bp.displayScale;
		}
		
		void _render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;	
		SizeSPX _defaultSize(int scale) const override;

	private:

		Skin_p	m_pDisplaySkin;
		State	m_displayState;
		float	m_displayValue = 1;		// Value used by skin, default is 1.
		float	m_displayValue2 = -1;	// Value2 used by skin, default is -1 (unused).
		int		m_displayScale = 64;	// Scale used by skin, default is 64 (1:1).

	};



} // namespace wg
#endif //WG_SkinDisplay_DOT_H
