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
#ifndef WG_METER_DOT_H
#define WG_METER_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_skin.h>

namespace wg
{

	class Meter;
	typedef	StrongPtr<Meter>	Meter_p;
	typedef	WeakPtr<Meter>		Meter_wp;

	//____ Meter ____________________________________________________________
	/**
	 * @brief A simple Meter widget.
	 *
	 *
	 **/


	class Meter : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			bool			pickHandle = false;
			uint8_t			pickCategory = 0;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static Meter_p	create() { return Meter_p(new Meter()); }
		static Meter_p	create( const Blueprint& blueprint ) { return Meter_p(new Meter(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		void			setValue(float value);
		inline float	value() const { return m_value;	}


	protected:
		Meter() {};
		template<class BP> Meter(const BP& bp) : Widget(bp)
		{
		}

		void		_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		float		_skinValue(const SkinSlot* pSlot) const override;

	private:

		float		m_value = 0;
	};



} // namespace wg
#endif //WG_METER_DOT_H
