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

#include <wg_c_blurbrush.h>
#include <wg_blurbrush.h>

#include <type_traits>


using namespace wg;


inline Blurbrush* getPtr(wg_obj obj) {
	return static_cast<Blurbrush*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createBlurbrush( wg_blurbrushBP _bp )
{
	Blurbrush::Blueprint bp;
	bp.size = _bp.size;
	
	for (int i = 0; i < 9; i++)
	{
		bp.blue[i] = _bp.blue[i];
		bp.green[i] = _bp.green[i];
		bp.red[i] = _bp.red[i];
	}
	
	auto pBlurbrush = Blurbrush::create(bp);
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



