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
#ifndef	WG_NUMBERLAYOUT_DOT_H
#define WG_NUMBERLAYOUT_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_string.h>

namespace wg
{

	class NumberLayout;
	typedef	StrongPtr<NumberLayout>		NumberLayout_p;
	typedef	WeakPtr<NumberLayout>		NumberLayout_wp;

	//____ NumberLayout __________________________________________________________

	class NumberLayout : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc __________________________________________________

		virtual void 	render(GfxDevice* pDevice, const RectSPX& canvas, double value, int scale, State state) = 0;
		virtual SizeSPX	defaultSize(double value, int scale, State state) const = 0;
		virtual bool	stateChangeNeedsRender(State newState, State oldState) const = 0;

	protected:
		~NumberLayout() {}
		
		void	_renderString(GfxDevice * pDevice, const Char* pString, TextStyle* pStyle, CoordSPX pos, int scale, State state );

	};


} // namespace wg
#endif //WG_NUMBERLAYOUT_DOT_H
