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

#ifndef	WG_NUMBERDISPLAY_DOT_H
#define	WG_NUMBERDISPLAY_DOT_H
#pragma once

#	include <wg_widget.h>
#	include <wg_number.h>

namespace wg
{


	class NumberDisplay;
	typedef	StrongPtr<NumberDisplay>		NumberDisplay_p;
	typedef	WeakPtr<NumberDisplay>		NumberDisplay_wp;

	/**
	* @brief Widget that displays a formatted value.
	*/
	class NumberDisplay : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p			baggage;
			bool				disabled = false;
			Number::Blueprint	display;
			bool				dropTarget = false;
			Finalizer_p			finalizer = nullptr;
			int					id = 0;
			MarkPolicy			markPolicy = MarkPolicy::AlphaTest;
			bool				pickable = false;
			uint8_t				pickCategory = 0;
			bool				pickHandle = false;
			PointerStyle		pointer = PointerStyle::Undefined;
			bool				selectable = false;
			Skin_p				skin;
			bool				stickyFocus = false;
			bool				tabLock = false;
			String				tooltip;
		};

		//.____ Creation __________________________________________

		static NumberDisplay_p	create() { return NumberDisplay_p(new NumberDisplay()); }
		static NumberDisplay_p	create( const Blueprint& blueprint ) { return NumberDisplay_p(new NumberDisplay(blueprint)); }

		//.____ Components _______________________________________

		Number		display;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal _________________________________________________

		SizeSPX			_defaultSize(int scale) const override;



	protected:
		NumberDisplay()  : display(this) {}

		template<class BP> NumberDisplay( const BP& bp ) : display(this), Widget(bp)
		{
			display._initFromBlueprint(bp.display);
		}
		
		virtual ~NumberDisplay() {}

		void	_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void	_setState( State state ) override;

	private:

		class DisplayAccess : public Number { friend class NumberDisplay; };
		const DisplayAccess& _display() const { return reinterpret_cast<const DisplayAccess&>(display); }
		DisplayAccess& _display() { return reinterpret_cast<DisplayAccess&>(display); }

	};

} // namespace wg
#endif // WG_NUMBERDISPLAY_DOT_H
