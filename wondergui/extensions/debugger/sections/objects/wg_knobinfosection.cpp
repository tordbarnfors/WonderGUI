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
#include "wg_knobinfosection.h"


namespace wg
{

	const TypeInfo KnobInfoSection::TYPEINFO = { "KnobInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	KnobInfoSection::KnobInfoSection(const DebugTheme& theme, IDebugContext* pContext, Knob * pInspected)
		: TypedInfoSection<Knob>( theme, pContext, Knob::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			decimalRow( "Value: ",            [](Knob* k) { return k->value(); } ),
			intRow    ( "Steps: ",            [](Knob* k) { return k->steps(); } ),
			textRow   ( "Drag axis: ",        [](Knob* k) { return toString(k->dragAxis()); } ),
			ptsRow    ( "Drag range (pts): ", [](Knob* k) { return k->dragRange(); } ),
			decimalRow( "Wheel step size: ",  [](Knob* k) { return k->wheelStepSize(); } ),
			boolRow   ( "Lock mouse: ",       [](Knob* k) { return k->isLockingMouse(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& KnobInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
