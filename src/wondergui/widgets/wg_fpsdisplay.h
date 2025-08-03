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

#ifndef	WG_FPSDISPLAY_DOT_H
#define WG_FPSDISPLAY_DOT_H
#pragma once

#include <wg_widget.h>
#include	<wg_dynamictext.h>

namespace wg
{


	class Font;

	class FpsDisplay;
	typedef	StrongPtr<FpsDisplay>		FpsDisplay_p;
	typedef	WeakPtr<FpsDisplay>	FpsDisplay_wp;

	class FpsDisplay : public Widget
	{
	public:

		//.____ Creation __________________________________________

		static FpsDisplay_p	create() { return FpsDisplay_p(new FpsDisplay()); }


		//.____ Components _______________________________________

		DynamicText		labels;
		StaticText		values;


		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal _________________________________________________

		SizeSPX		_defaultSize(int scale) const override;


	protected:
		FpsDisplay();
		virtual ~FpsDisplay();

		void		_setState( State state ) override;
		void		_update(int microPassed, int64_t microsecTimestamp) override;
		void		_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;

	private:

	const static int c_tickBuffer = 64;


		int *			m_pTickBuffer;
		int				m_tickBufferOfs;
	};




} // namespace wg
#endif //FpsDisplay_DOT_H
