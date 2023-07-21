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

#ifndef WG_C_EDGEMAPFACTORY_DOT_H
#define WG_C_EDGEMAPFACTORY_DOT_H
#pragma once

#include <wg_c_edgemap.h>

#ifdef __cplusplus
extern "C" {
#endif


	//.____ Misc _______________________________________________________

	wg_obj wg_createEdgemap( wg_obj factory, const wg_edgemapBP* blueprint);

	wg_obj wg_createEdgemapFromFloats( wg_obj factory, const wg_edgemapBP* blueprint, wg_waveOrigo origo, 
										const float* pSamples, int edges, int edgePitch, int samplePitch);

	wg_obj wg_createEdgemapFromSpx( wg_obj factory, const wg_edgemapBP* blueprint, wg_waveOrigo origo, 
									 const wg_spx* pSamples, int edges, int edgePitch, int samplePitch);

#ifdef __cplusplus
}
#endif

#endif
