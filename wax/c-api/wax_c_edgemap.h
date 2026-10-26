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
#ifndef WAX_C_EDGEMAP_DOT_H
#define WAX_C_EDGEMAP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif



	//____ wax_edgemapBP __________________________________________________________

	typedef struct wax_edgemapBP_struct			// NOT BINARY EQUIVALENT!
	{
		const wax_color*		colors;					// Flat colors, one color for each segment.
		int					segments;
		wax_sizeI			size;
		const wax_obj * 		tints;					// One Tint per segment if any. Null entries use colors.
		int					tiling;					// Edgemap can be tiled. Has one sample less per edge if set.

	} wax_edgemapBP;


	//.____ Geometry ______________________________________________________

	WAX_EXPORT wax_sizeI		wax_edgemapPixelSize(wax_obj edgemap);
	WAX_EXPORT int			wax_edgemapIsTiling(wax_obj edgemap);

	//.____ Appearance ____________________________________________________

	WAX_EXPORT int		wax_setRenderSegments(wax_obj edgemap, int nSegments);
	WAX_EXPORT int		wax_getRenderSegments(wax_obj edgemap);

	WAX_EXPORT int		wax_setEdgemapColors(wax_obj edgemap, int begin, int end, const wax_color * pColors);
	WAX_EXPORT int		wax_setEdgemapTints(wax_obj edgemap, int begin, int end, const wax_obj * pTints );

	WAX_EXPORT const wax_color *  wax_edgemapFlatColors(wax_obj edgemap);
	WAX_EXPORT wax_obj	wax_edgemapTint(wax_obj edgemap, int segment);		// Null if segment has flat color.

	//.____ Content _______________________________________________________

	WAX_EXPORT int	wax_edgemapSegments(wax_obj edgemap);

	WAX_EXPORT int	wax_edgemapSamples(wax_obj edgemap);

	WAX_EXPORT int 	wax_importSpxSamples(wax_obj edgemap, wax_sampleOrigo origo, const wax_spx* pSource,
										int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, 
										int edgePitch, int samplePitch);

	WAX_EXPORT int 	wax_importFloatSamples(	wax_obj edgemap, wax_sampleOrigo origo, const float* pSource,
											int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, 
											int edgePitch, int samplePitch);

	WAX_EXPORT int 	wax_exportSpxSamples(wax_obj edgemap, wax_sampleOrigo origo, wax_spx* pDestination,
										int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, 
										int edgePitch, int samplePitch);

	WAX_EXPORT int 	wax_exportFloatSamples(	wax_obj edgemap, wax_sampleOrigo origo, float* pDestination,
											int edgeBegin, int edgeEnd,	int sampleBegin, int sampleEnd, 
											int edgePitch, int samplePitch);

	WAX_EXPORT void	wax_exportBounds( wax_obj edgemap, wax_spx * pMinMaxOutput, int nSections, int sectionWidth,
									 int topEdge, int bottomEdge, int mapOffset, int minMaxPitch );


#ifdef __cplusplus
}
#endif

#endif
