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
#include <algorithm>
#include <cmath>
#include <cstring>

#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

using namespace std;
using namespace wg::Util;

namespace wg
{

	const TypeInfo GfxDevice::TYPEINFO = { "GfxDevice", &Object::TYPEINFO };


	//____ constructor _____________________________________________________________

	GfxDevice::GfxDevice()
	{
	}

	//____ Destructor _________________________________________________________

	GfxDevice::~GfxDevice()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GfxDevice::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg
