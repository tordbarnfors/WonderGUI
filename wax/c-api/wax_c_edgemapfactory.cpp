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
#include <wax_c_edgemapfactory.h>
#include <wax_c_internal.h>
#include <wax_edgemapfactory.h>

using namespace wax;

inline EdgemapFactory* getPtr(wax_obj obj) {
	return static_cast<EdgemapFactory*>(reinterpret_cast<Object*>(obj));
}


static void convertBP(const wax_edgemapBP* pSrc, Edgemap::Blueprint& dest, Tint_p * pSpaceForTints)
{
	convertEdgemapBlueprint(&dest, pSrc, pSpaceForTints);
}


wax_obj wax_createEdgemap(wax_obj factory, const wax_edgemapBP* pCBP)
{
	Edgemap::Blueprint	cppBP;
	Tint_p tints[Edgemap::maxSegments];
	convertBP(pCBP,cppBP,tints);

	auto p = getPtr(factory)->createEdgemap(cppBP);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createEdgemapFromFloats(wax_obj factory, const wax_edgemapBP* pCBP, wax_sampleOrigo origo,
	const float* pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	cppBP;
	Tint_p tints[Edgemap::maxSegments];
	convertBP(pCBP,cppBP,tints);

	auto p = getPtr(factory)->createEdgemap(cppBP, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createEdgemapFromSpx(wax_obj factory, const wax_edgemapBP* pCBP, wax_sampleOrigo origo,
	const wax_spx* pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	cppBP;
	Tint_p tints[Edgemap::maxSegments];
	convertBP(pCBP,cppBP,tints);

	auto p = getPtr(factory)->createEdgemap(cppBP, (SampleOrigo)origo, (const spx*) pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}
