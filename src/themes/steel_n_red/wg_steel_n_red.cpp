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
#include <wg_steel_n_red.h>

#include <wg_blockskin.h>

#include <wg_togglegroup.h>

namespace wg
{

const TypeInfo SteelNRed::TYPEINFO = { "SteelNRed", &Theme::TYPEINFO };


SteelNRed_p SteelNRed::create(Surface* pSurface )
{
	return SteelNRed_p(new SteelNRed(pSurface));
}

const TypeInfo& SteelNRed::typeInfo(void) const
{
	return TYPEINFO;
}

const ToggleButton::Blueprint& SteelNRed::radioButton() const
{
	return m_radioButton;
}

SteelNRed::SteelNRed( Surface * pSurface ) :m_pSurface(pSurface)
{

	m_pRadioButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.axis = Axis::X,
		_.blockSpacing = 1,
		_.firstBlock = { 0,0,34,34 },
		_.frame = { 33,0,0,33 },
		_.padding = { 1,0,0,33 },
		_.states = { State::Default, State::Hovered, State::Selected, State::SelectedHovered },
		_.surface = pSurface,
		_.spacing = 1
	));

	m_radioButton.skin = m_pRadioButtonSkin;

}



}