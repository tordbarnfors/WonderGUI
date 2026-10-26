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
#include <wg2_colorset.h>


WgColorsetPtr WgColorset::Create()
{
	return WgColorsetPtr(new WgColorset());
}

WgColorsetPtr WgColorset::Create( WgColor allModes )
{
	WgColorset * p = new WgColorset();

	p->m_color[WG_MODE_NORMAL] = allModes;
	p->m_color[WG_MODE_MARKED] = allModes;
	p->m_color[WG_MODE_SELECTED] = allModes;
	p->m_color[WG_MODE_DISABLED] = allModes;
	p->m_color[WG_MODE_SPECIAL] = allModes;

	return WgColorsetPtr(p);
}

WgColorsetPtr  WgColorset::Create( WgColor normal, WgColor over, WgColor selected, WgColor disabled, WgColor special )
{
	WgColorset * p = new WgColorset();

	p->m_color[WG_MODE_NORMAL] = normal;
	p->m_color[WG_MODE_MARKED] = over;
	p->m_color[WG_MODE_SELECTED] = selected;
	p->m_color[WG_MODE_DISABLED] = disabled;
	p->m_color[WG_MODE_SPECIAL] = special;

	return WgColorsetPtr(p);
}

void WgColorset::SetColor( WgColor color, WgMode mode )
{
	if( mode == WG_MODE_ALL )
	{
		m_color[WG_MODE_NORMAL] = color;
		m_color[WG_MODE_MARKED] = color;
		m_color[WG_MODE_SELECTED] = color;
		m_color[WG_MODE_DISABLED] = color;
		m_color[WG_MODE_SPECIAL] = color;
	}
	else
		m_color[mode] = color;
}

