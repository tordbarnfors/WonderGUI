/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_c_gradient.h>
#include <wg_gradient.h>

#include <type_traits>

static_assert( sizeof(wg_gradient) == sizeof(wg::Gradient), "wg_gradient differs in size from wg:Gradient!" );
static_assert( std::is_trivially_copyable<wg::Gradient>::value, "wg::Gradient is not trivially copyable" );
