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
#include <wg_c_gradyent.h>
#include <wg_gradyent.h>

#include <type_traits>


using namespace wg;


wg_obj wg_createGradyent( wg_color top, wg_color bottom, wg_color left, wg_color right )
{
	auto pGradyent = Gradyent::create( HiColor( top.r, top.g, top.b, top.a),
									  HiColor( bottom.r, bottom.g, bottom.b, bottom.a),
									  HiColor( left.r, left.g, left.b, left.a),
									  HiColor( right.r, right.g, right.b, right.a) );
	pGradyent->retain();
	return static_cast<Object*>(pGradyent);
}
