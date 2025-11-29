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
#ifndef WG2_COLORSET_DOT_H
#define WG2_COLORSET_DOT_H

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#include <wg_object.h>
#include <wg_pointers.h>

class WgColorset;

typedef	wg::StrongPtr<WgColorset>	WgColorsetPtr;

class WgColorset : public wg::Object
{
public:
	static WgColorsetPtr	Create();
	static WgColorsetPtr	Create( WgColor allModes );
	static WgColorsetPtr	Create( WgColor normal, WgColor over, WgColor selected, WgColor disabled, WgColor special );

	inline WgColor	Color(WgMode mode) const { return m_color[mode]; }
	void			SetColor(WgColor color, WgMode mode = WG_MODE_ALL);

private:
	WgColorset() {};
	virtual ~WgColorset() {};

	WgColor		m_color[WG2_NB_MODES];
};



#endif //WG2_COLORSET_DOT_H
