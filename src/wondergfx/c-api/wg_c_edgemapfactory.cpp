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
#include <wg_c_edgemapfactory.h>
#include <wg_c_internal.h>
#include <wg_edgemapfactory.h>

using namespace wg;

inline EdgemapFactory* getPtr(wg_obj obj) {
	return static_cast<EdgemapFactory*>(reinterpret_cast<Object*>(obj));
}


static void convertBP(const wg_edgemapBP* pSrc, Edgemap::Blueprint& dest)
{
	dest.colors			= (const HiColor*) pSrc->colors;
	dest.colorstripsX	= (const HiColor*) pSrc->colorstripsX;
	dest.colorstripsY	= (const HiColor*) pSrc->colorstripsY;
	dest.paletteType	= (EdgemapPalette) pSrc->paletteType;
	dest.tintmaps		= (const Tintmap_p *) pSrc->tintmaps;		 // IS THIS INDIRECT CONVERSION SAFE?
	dest.segments 		= pSrc->segments;
	dest.size.w			= pSrc->size.w;
	dest.size.h			= pSrc->size.h;
}


wg_obj wg_createEdgemap(wg_obj factory, const wg_edgemapBP* pCBP)
{
	Edgemap::Blueprint	cppBP;
	convertBP(pCBP,cppBP);

	auto p = getPtr(factory)->createEdgemap(cppBP);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createEdgemapFromFloats(wg_obj factory, const wg_edgemapBP* pCBP, wg_sampleOrigo origo,
	const float* pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	cppBP;
	convertBP(pCBP,cppBP);

	auto p = getPtr(factory)->createEdgemap(cppBP, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createEdgemapFromSpx(wg_obj factory, const wg_edgemapBP* pCBP, wg_sampleOrigo origo,
	const wg_spx* pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	cppBP;
	convertBP(pCBP,cppBP);

	auto p = getPtr(factory)->createEdgemap(cppBP, (SampleOrigo)origo, (const spx*) pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}
