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
#ifndef WAX_C_BLURBRUSH_DOT_H
#define WAX_C_BLURBRUSH_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


//____ wax_blurbrushBP __________________________________________________________

typedef struct wax_blurbrushBP_struct			// NOT BINARY EQUIVALENT!
{
	const float *	blue;
	const float *	green;
	const float *	red;
	
	wax_spx		size = 64 * 4;;
} wax_blurbrushBP;


WAX_EXPORT wax_obj		wax_createBlurbrush( wax_blurbrushBP bp );

WAX_EXPORT wax_spx		wax_blurbrushSize( wax_obj blurbrush );

WAX_EXPORT const float*	wax_blurbrushBlue( wax_obj blurbrush );
WAX_EXPORT const float*	wax_blurbrushGreen( wax_obj blurbrush );
WAX_EXPORT const float*	wax_blurbrushRed( wax_obj blurbrush );


#ifdef __cplusplus
}
#endif

#endif
