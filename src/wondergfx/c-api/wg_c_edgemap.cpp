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

#include <wg_c_edgemap.h>
#include <wg_edgemap.h>

#include <type_traits>


using namespace wg;


inline Edgemap* getPtr(wg_obj obj) {
	return static_cast<Edgemap*>(reinterpret_cast<Object*>(obj));
}


wg_sizeI wg_edgemapPixelSize(wg_obj edgemap)
{
	SizeI sz = getPtr(edgemap)->pixelSize();
	return { sz.w,sz.h };
}


int wg_setRenderSegments(wg_obj edgemap, int nSegments)
{
	return getPtr(edgemap)->setRenderSegments(nSegments);
}

int wg_getRenderSegments(wg_obj edgemap)
{
	return getPtr(edgemap)->renderSegments();
}

int wg_setEdgemapColors(wg_obj edgemap, int begin, int end, const wg_color * pColors)
{
	return getPtr(edgemap)->setColors(begin, end, reinterpret_cast<const HiColor *>(pColors) );
}

int wg_setEdgemapColorsFromGradients(wg_obj edgemap, int begin, int end, const wg_gradient * pGradients )
{
	return getPtr(edgemap)->setColors(begin, end, reinterpret_cast<const Gradient*>(pGradients) );
}

int wg_setEdgemapColorsFromTintmaps(wg_obj edgemap, int begin, int end, wg_obj * pTintmaps )
{
	Tintmap_p	pointers[Edgemap::maxSegments];
	
	for( int i = 0 ; i < end - begin ; i++ )
		pointers[i] = static_cast<Tintmap*>(reinterpret_cast<Object*>(pTintmaps[i]));
	
	return getPtr(edgemap)->setColors(begin, end, pointers);
}

int wg_setEdgemapColorsFromStrips(wg_obj edgemap, int begin, int end, const wg_color * pColorstripX, const wg_color * pColorstripY )
{
	return getPtr(edgemap)->setColors(begin, end, (const HiColor*) pColorstripX, (const HiColor*) pColorstripY);
}

int wg_importEdgemapPaletteEntries(wg_obj edgemap, int begin, int end, const wg_color * pColors )
{
	return getPtr(edgemap)->importPaletteEntries(begin, end, (const HiColor*) pColors);
}

const wg_color * wg_edgemapFlatColors(wg_obj edgemap)
{
	return (const wg_color *) getPtr(edgemap)->flatColors();
}

const wg_color * wg_edgemapColorstripsX(wg_obj edgemap)
{
	return (const wg_color *) getPtr(edgemap)->colorstripsX();
}

const wg_color * wg_edgemapColorstripsY(wg_obj edgemap)
{
	return (const wg_color *) getPtr(edgemap)->colorstripsY();
}

int wg_edgemapSegments(wg_obj edgemap)
{
	return getPtr(edgemap)->segments();
}

int wg_edgemapSamples(wg_obj edgemap)
{
	return getPtr(edgemap)->samples();
}

int wg_importSpxSamples(wg_obj edgemap, wg_sampleOrigo origo, const wg_spx* pSource,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->importSamples((SampleOrigo)origo, (const spx*)pSource, 
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

int	wg_importFloatSamples(wg_obj edgemap, wg_sampleOrigo origo, const float* pSource,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->importSamples((SampleOrigo)origo, pSource,
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

int	wg_exportSpxSamples(wg_obj edgemap, wg_sampleOrigo origo, wg_spx* pDestination,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->exportSamples((SampleOrigo)origo, (spx*)pDestination,
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

int wg_exportFloatSamples(wg_obj edgemap, wg_sampleOrigo origo, float* pDestination,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->exportSamples((SampleOrigo)origo, pDestination,
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}
