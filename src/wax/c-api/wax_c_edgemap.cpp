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
#include <wax_c_edgemap.h>
#include <wax_edgemap.h>

#include <type_traits>


using namespace wax;


inline Edgemap* getPtr(wax_obj obj) {
	return static_cast<Edgemap*>(reinterpret_cast<Object*>(obj));
}


wax_sizeI wax_edgemapPixelSize(wax_obj edgemap)
{
	SizeI sz = getPtr(edgemap)->pixelSize();
	return { sz.w,sz.h };
}

int wax_edgemapIsTiling(wax_obj edgemap)
{
	return getPtr(edgemap)->isTiling();
}


int wax_setRenderSegments(wax_obj edgemap, int nSegments)
{
	return getPtr(edgemap)->setRenderSegments(nSegments);
}

int wax_getRenderSegments(wax_obj edgemap)
{
	return getPtr(edgemap)->renderSegments();
}

int wax_setEdgemapColors(wax_obj edgemap, int begin, int end, const wax_color * pColors)
{
	return getPtr(edgemap)->setColors(begin, end, reinterpret_cast<const HiColor *>(pColors) );
}

int wax_setEdgemapTints(wax_obj edgemap, int begin, int end, const wax_obj * pTints )
{
	if( end - begin > Edgemap::maxSegments || end < begin )
		return 0;

	Tint_p	pointers[Edgemap::maxSegments];

	for( int i = 0 ; i < end - begin ; i++ )
		pointers[i] = static_cast<Tint*>(reinterpret_cast<Object*>(pTints[i]));

	return getPtr(edgemap)->setColors(begin, end, pointers);
}

const wax_color * wax_edgemapFlatColors(wax_obj edgemap)
{
	return (const wax_color *) getPtr(edgemap)->flatColors();
}

wax_obj wax_edgemapTint(wax_obj edgemap, int segment)
{
	auto pEdgemap = getPtr(edgemap);
	if( segment < 0 || segment >= pEdgemap->segments() )
		return nullptr;

	return static_cast<Object*>(pEdgemap->tints()[segment].rawPtr());
}

int wax_edgemapSegments(wax_obj edgemap)
{
	return getPtr(edgemap)->segments();
}

int wax_edgemapSamples(wax_obj edgemap)
{
	return getPtr(edgemap)->samples();
}

int wax_importSpxSamples(wax_obj edgemap, wax_sampleOrigo origo, const wax_spx* pSource,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->importSamples((SampleOrigo)origo, (const spx*)pSource, 
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

int	wax_importFloatSamples(wax_obj edgemap, wax_sampleOrigo origo, const float* pSource,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->importSamples((SampleOrigo)origo, pSource,
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

int	wax_exportSpxSamples(wax_obj edgemap, wax_sampleOrigo origo, wax_spx* pDestination,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->exportSamples((SampleOrigo)origo, (spx*)pDestination,
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

int wax_exportFloatSamples(wax_obj edgemap, wax_sampleOrigo origo, float* pDestination,
	int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd,
	int edgePitch, int samplePitch)
{
	return getPtr(edgemap)->exportSamples((SampleOrigo)origo, pDestination,
		edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
}

void wax_exportBounds( wax_obj edgemap, wax_spx * pMinMaxOutput, int nSections, int sectionWidth,
	int topEdge, int bottomEdge, int mapOffset, int minMaxPitch )
{
	return getPtr(edgemap)->exportBounds(pMinMaxOutput, nSections, sectionWidth, topEdge, bottomEdge, mapOffset, minMaxPitch );
}
