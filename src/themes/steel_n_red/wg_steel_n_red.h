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
#ifndef	WG_THEME_STEEL_N_RED_DOT_H
#define WG_THEME_STEEL_N_RED_DOT_H
#pragma once

#include <wg_theme.h>

#include <wg_skin.h>


namespace wg
{

	class SteelNRed;
	typedef	StrongPtr<SteelNRed>	SteelNRed_p;
	typedef	WeakPtr<SteelNRed>		SteelNRed_wp;


	class SteelNRed : public Theme
	{
	public:

		//.____ Creation __________________________________________

		static SteelNRed_p	create(Surface * pSurface);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		const ToggleButton::Blueprint& radioButton() const override;


	protected:
		SteelNRed( Surface * pSurface );
		~SteelNRed() {}

		Surface_p	m_pSurface;

		Skin_p		m_pRadioButtonSkin;

		ToggleButton::Blueprint	m_radioButton;

	};


} // namespace wg
#endif //WG_THEME_STEEL_N_RED_DOT_H
