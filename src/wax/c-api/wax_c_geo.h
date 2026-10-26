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
#ifndef WAX_C_GEO_DOT_H
#define WAX_C_GEO_DOT_H
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


	typedef int		wax_spx;
	typedef float	wax_pts;


	typedef struct
	{
		int	x, y;
	} wax_coordI;

	typedef struct
	{
		float	x, y;
	} wax_coordF;

	typedef struct
	{
		wax_pts	x, y;
	} wax_coord;

	typedef struct
	{
		wax_spx	x, y;
	} wax_coordSPX;


	typedef struct
	{
		int	w, h;
	} wax_sizeI;

	typedef struct
	{
		float	w, h;
	} wax_sizeF;

	typedef struct
	{
		wax_pts	w, h;
	} wax_size;

	typedef struct
	{
		wax_spx	w, h;
	} wax_sizeSPX;


	typedef struct
	{
		int	top, right, bottom, left;
	} wax_borderI;

	typedef struct
	{
		float	top, right, bottom, left;
	} wax_borderF;

	typedef struct
	{
		wax_pts	top, right, bottom, left;
	} wax_border;

	typedef struct
	{
		wax_spx	top, right, bottom, left;
	} wax_borderSPX;


	typedef struct
	{
		int	x, y, w, h;
	} wax_rectI;

	typedef struct
	{
		float	x, y, w, h;
	} wax_rectF;

	typedef struct
	{
		wax_pts	x, y, w, h;
	} wax_rect;

	typedef struct
	{
		wax_spx	x, y, w, h;
	} wax_rectSPX;

#ifdef __cplusplus
}
#endif

#endif 
