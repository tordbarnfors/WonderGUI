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
#include <type_traits>

#include <wax_c_color.h>
#include <wax_color.h>


static_assert( sizeof(wax_color) == sizeof(wax::HiColor), "wax_color differs in size from wax::HiColor!" );
static_assert( std::is_trivially_copyable<wax::HiColor>::value, "HiColor is not trivially copyable!" );

