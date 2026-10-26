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
#ifndef EDGEMAPTOOLS_DOT_H
#define EDGEMAPTOOLS_DOT_H
#pragma once

#include <wg_edgemap.h>
#include <wg_edgemapfactory.h>

namespace wg
{
namespace EdgemapTools
{

	Edgemap_p createDonut(SizeSPX size, spx thickness, HiColor fillColor, spx outlineThickness, HiColor outlineColor, EdgemapFactory * pFactory = nullptr );

	bool drawOutlinedDonut(Edgemap * pEdgemap, spx thickness, spx outlineThickness );

	void convertSamples( spx * pDest, const spx * pSource, SampleOrigo origo, spx height, int nEdges, int nSamples, int srcEdgePitch, int srcSamplePitch, int destEdgePitch, int destSamplePitch );
	void convertSamples( spx * pDest, const float * pSource, SampleOrigo origo, spx height, int nEdges, int nSamples, int srcEdgePitch, int srcSamplePitch, int destEdgePitch, int destSamplePitch );

	spx convertSample( spx sample, SampleOrigo origo, spx height );
	spx convertSample( float sample, SampleOrigo origo, spx height );

}

}

#endif //EDGEMAPTOOLS_DOT_H
