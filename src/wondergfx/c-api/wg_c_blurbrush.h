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

#ifndef WG_C_BLURBRUSH_DOT_H
#define WG_C_BLURBRUSH_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


//____ wg_edgemapBP __________________________________________________________

typedef struct wg_blurbrushBP_struct			// NOT BINARY EQUIVALENT!
{
	float	blue[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
	float	green[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
	float	red[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };

	wg_spx		size = 64 * 4;;
} wg_blurbrushBP;


wg_obj	wg_createBlurbrush( wg_blurbrushBP bp );

wg_spx		wg_blurbrushSize( wg_obj blurbrush );

const float* wg_blurbrushBlue( wg_obj blurbrush );
const float* wg_blurbrushGreen( wg_obj blurbrush );
const float* wg_blurbrushRed( wg_obj blurbrush );


#ifdef __cplusplus
}
#endif

#endif
