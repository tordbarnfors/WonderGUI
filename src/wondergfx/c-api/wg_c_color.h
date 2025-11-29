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
#ifndef WG_C_COLOR_DOT_H
#define WG_C_COLOR_DOT_H
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_color ______________________________________________________________

	typedef struct wg_color_struct
	{
		union
		{
			struct
			{
				int16_t b;
				int16_t g;
				int16_t r;
				int16_t a;
			};
			int64_t argb;
		};
	} wg_color;


	//____ wg_color8 _____________________________________________________________

	typedef struct wg_color8_struct
	{
		union
		{
			struct
			{
				uint8_t b;
				uint8_t g;
				uint8_t r;
				uint8_t a;
			};
			uint32_t argb;
		};
	} wg_color8;

#ifdef __cplusplus
}
#endif

#endif
