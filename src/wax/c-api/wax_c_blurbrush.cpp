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
#include <wg_c_blurbrush.h>
#include <wg_blurbrush.h>

#include <type_traits>


using namespace wg;


inline Blurbrush* getPtr(wg_obj obj) {
	return static_cast<Blurbrush*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createBlurbrush( wg_blurbrushBP _bp )
{	
	auto pBlurbrush = Blurbrush::create( WGBP(Blurbrush, _.size = _bp.size, _.blue = _bp.blue, _.green = _bp.green, _.red = _bp.red) );
	pBlurbrush->retain();
	return static_cast<Object*>(pBlurbrush);
}

wg_spx wg_blurbrushSize( wg_obj blurbrush )
{
	return getPtr(blurbrush)->size();
}

const float* wg_blurbrushBlue( wg_obj blurbrush )
{
	return getPtr(blurbrush)->blue();
}

const float* wg_blurbrushGreen( wg_obj blurbrush )
{
	return getPtr(blurbrush)->green();
}

const float* wg_blurbrushRed( wg_obj blurbrush )
{
	return getPtr(blurbrush)->red();
}



