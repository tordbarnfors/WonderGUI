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
#ifndef WG_C_GEO_DOT_H
#define WG_C_GEO_DOT_H
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


	typedef int		wg_spx;
	typedef float	wg_pts;


	typedef struct
	{
		int	x, y;
	} wg_coordI;

	typedef struct
	{
		float	x, y;
	} wg_coordF;

	typedef struct
	{
		wg_pts	x, y;
	} wg_coord;

	typedef struct
	{
		wg_spx	x, y;
	} wg_coordSPX;


	typedef struct
	{
		int	w, h;
	} wg_sizeI;

	typedef struct
	{
		float	w, h;
	} wg_sizeF;

	typedef struct
	{
		wg_pts	w, h;
	} wg_size;

	typedef struct
	{
		wg_spx	w, h;
	} wg_sizeSPX;


	typedef struct
	{
		int	top, right, bottom, left;
	} wg_borderI;

	typedef struct
	{
		float	top, right, bottom, left;
	} wg_borderF;

	typedef struct
	{
		wg_pts	top, right, bottom, left;
	} wg_border;

	typedef struct
	{
		wg_spx	top, right, bottom, left;
	} wg_borderSPX;


	typedef struct
	{
		int	x, y, w, h;
	} wg_rectI;

	typedef struct
	{
		float	x, y, w, h;
	} wg_rectF;

	typedef struct
	{
		wg_pts	x, y, w, h;
	} wg_rect;

	typedef struct
	{
		wg_spx	x, y, w, h;
	} wg_rectSPX;

#ifdef __cplusplus
}
#endif

#endif 
