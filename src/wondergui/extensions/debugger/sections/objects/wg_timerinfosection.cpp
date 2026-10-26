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
#include "wg_timerinfosection.h"


namespace wg
{

	const TypeInfo TimerInfoSection::TYPEINFO = { "TimerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TimerInfoSection::TimerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Timer * pInspected)
		: TypedInfoSection<Timer>( theme, pContext, Timer::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow( "Play mode: ",      [](Timer* t) { return toString(t->playMode()); } ),
			intRow ( "Duration (ms): ",  [](Timer* t) { return t->duration(); } ),
			intRow ( "Step size (ms): ", [](Timer* t) { return t->stepSize(); } ),
			boolRow( "On: ",             [](Timer* t) { return t->isOn(); } ),
			intRow ( "Value (ms): ",     [](Timer* t) { return t->value(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TimerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
