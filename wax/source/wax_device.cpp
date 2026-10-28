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

#include <wax_device.h>
#include <wax_geo.h>
#include <wax_util.h>
#include <wax_core.h>

using namespace std;
using namespace wax::Util;

namespace wax
{

	const TypeInfo Device::TYPEINFO = { "Device", &Object::TYPEINFO };

	const LineCap LineCap::None		= LineCap( LineCapShape::None );
	const LineCap LineCap::Flat			= LineCap( LineCapShape::Flat );
	const LineCap LineCap::Square		= LineCap( LineCapShape::Square );
	const LineCap LineCap::Round		= LineCap( LineCapShape::Round );
	const LineCap LineCap::Vertical		= LineCap( LineCapShape::Vertical );
	const LineCap LineCap::Horizontal	= LineCap( LineCapShape::Horizontal );


	//____ constructor _____________________________________________________________

	Device::Device()
	{
	}

	//____ Destructor _________________________________________________________

	Device::~Device()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Device::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wax
