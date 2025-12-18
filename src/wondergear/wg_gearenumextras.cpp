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
#include <wg_gearenumextras.h>

namespace wg
{

//. startAutoSection
/*=========================================================================

				>>> START OF AUTO GENERATED SECTION <<<

				Any modifications here will be overwritten!

=========================================================================*/

	const char * toString(ErrorLevel i)
	{
		static const char * names[] = { 
			"Warning",
			"SilentError",
			"Error",
			"Critical" };

		return names[(int)i];
	}

	const char * toString(ErrorCode i)
	{
		static const char * names[] = { 
			"OutOfRange",
			"InvalidIterator",
			"InvalidParam",
			"FailedPrerequisite",
			"OpenGL",
			"Internal",
			"SystemIntegrity",
			"IllegalCall",
			"ResourceExhausted",
			"RenderFailure",
			"Performance",
			"Other" };

		return names[(int)i];
	}


//=========================================================================
//. endAutoSection



}	//namespace wg
