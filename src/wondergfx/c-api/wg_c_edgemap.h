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
#ifndef WG_C_EDGEMAP_DOT_H
#define WG_C_EDGEMAP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>
#include <wg_c_gradient.h>

#ifdef __cplusplus
extern "C" {
#endif



	//____ wg_edgemapBP __________________________________________________________

	typedef struct wg_edgemapBP_struct			// NOT BINARY EQUIVALENT!
	{
		const wg_color*		colors;					// Flat colors, one color for each segment.
		const wg_color*		colorstripsX;			// One color for each pixel along width for each segment.
		const wg_color*		colorstripsY;			// One color for each pixel along height for each segment.

		wg_edgemapPalette	paletteType;

		const wg_obj * 		tintmaps;				// Needs to have one tintmap per segement if any.
		int					segments;
		wg_sizeI			size;

	} wg_edgemapBP;


	//.____ Geometry ______________________________________________________

	WG_EXPORT wg_sizeI		wg_edgemapPixelSize(wg_obj edgemap);

	//.____ Appearance ____________________________________________________

	WG_EXPORT int		wg_setRenderSegments(wg_obj edgemap, int nSegments);
	WG_EXPORT int		wg_getRenderSegments(wg_obj edgemap);

	WG_EXPORT wg_edgemapPalette	wg_edgemapPaletteType(wg_obj edgemap);

	WG_EXPORT int		wg_setEdgemapColors(wg_obj edgemap, int begin, int end, const wg_color * pColors);
	WG_EXPORT int		wg_setEdgemapColorsFromGradients(wg_obj edgemap, int begin, int end, const wg_gradient * pGradients );
	WG_EXPORT int		wg_setEdgemapColorsFromTintmaps(wg_obj edgemap, int begin, int end, wg_obj * pTintmaps );
	WG_EXPORT int		wg_setEdgemapColorsFromStrips(wg_obj edgemap, int begin, int end, const wg_color * pColorstripX, const wg_color * pColorstripY );

	WG_EXPORT int		wg_importEdgemapPaletteEntries(wg_obj edgemap, int begin, int end, const wg_color * pColors );

	WG_EXPORT const wg_color *  wg_edgemapFlatColors(wg_obj edgemap);
	WG_EXPORT const wg_color *  wg_edgemapColorstripsX(wg_obj edgemap);
	WG_EXPORT const wg_color *  wg_edgemapColorstripsY(wg_obj edgemap);


	//.____ Content _______________________________________________________

	WG_EXPORT int	wg_edgemapSegments(wg_obj edgemap);

	WG_EXPORT int	wg_edgemapSamples(wg_obj edgemap);

	WG_EXPORT int 	wg_importSpxSamples(wg_obj edgemap, wg_sampleOrigo origo, const wg_spx* pSource,
										int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, 
										int edgePitch, int samplePitch);

	WG_EXPORT int 	wg_importFloatSamples(	wg_obj edgemap, wg_sampleOrigo origo, const float* pSource,
											int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, 
											int edgePitch, int samplePitch);

	WG_EXPORT int 	wg_exportSpxSamples(wg_obj edgemap, wg_sampleOrigo origo, wg_spx* pDestination,
										int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, 
										int edgePitch, int samplePitch);

	WG_EXPORT int 	wg_exportFloatSamples(	wg_obj edgemap, wg_sampleOrigo origo, float* pDestination,
											int edgeBegin, int edgeEnd,	int sampleBegin, int sampleEnd, 
											int edgePitch, int samplePitch);

	WG_EXPORT void	wg_exportBounds( wg_obj edgemap, wg_spx * pMinMaxOutput, int nSections, int sectionWidth,
									 int topEdge, int bottomEdge, int mapOffset, int minMaxPitch );


#ifdef __cplusplus
}
#endif

#endif
