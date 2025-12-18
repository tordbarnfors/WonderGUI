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
#ifndef	WG_GEARENUMEXTRAS_DOT_H
#define WG_GEARENUMEXTRAS_DOT_H
#pragma once

#include <wg_geartypes.h>

namespace wg
{

//. startAutoSection
/*=========================================================================

				>>> START OF AUTO GENERATED SECTION <<<

				Any modifications here will be overwritten!

=========================================================================*/

	const static ErrorLevel      ErrorLevel_min      = ErrorLevel::Warning;
	const static ErrorCode       ErrorCode_min       = ErrorCode::OutOfRange;

	const static ErrorLevel      ErrorLevel_max      = ErrorLevel::Critical;
	const static ErrorCode       ErrorCode_max       = ErrorCode::Other;

	const static int             ErrorLevel_size     = (int)ErrorLevel::Critical + 1;
	const static int             ErrorCode_size      = (int)ErrorCode::Other + 1;

	const char * toString(ErrorLevel);
	const char * toString(ErrorCode);

//=========================================================================
//. endAutoSection


} // namespace wg

#endif //WG_GEARENUMEXTRAS_DOT_H
