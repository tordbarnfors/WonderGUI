
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


#include <wg_c_internal.h>
#include <wg_gfxbase.h>

using namespace wg;

void convertSurfaceBlueprint(Surface::Blueprint* pDest, const wg_surfaceBP* pSource)
{
	pDest->buffered = pSource->buffered;
	pDest->canvas = pSource->canvas;
	pDest->palette = (Color8*)pSource->palette;
	pDest->paletteCapacity = pSource->paletteCapacity;
	pDest->paletteSize = pSource->paletteSize;
	pDest->palette = (Color8*)pSource->palette;
	pDest->dynamic = pSource->dynamic;
	pDest->format = (PixelFormat)pSource->format;
	pDest->identity = pSource->identity;
	pDest->mipmap = pSource->mipmap;
	pDest->sampleMethod = (SampleMethod)pSource->sampleMethod;
	pDest->scale = pSource->scale;
	pDest->size.w = pSource->size.w;
	pDest->size.h = pSource->size.h;
	pDest->tiling = pSource->tiling;
}

void convertEdgemapBlueprint(Edgemap::Blueprint* pDest, const wg_edgemapBP* pSource, Tintmap_p * pSpaceForTintmaps )
{
	pDest->colors = (HiColor*) pSource->colors;
	pDest->colorstripsX = (HiColor*) pSource->colorstripsX;
	pDest->colorstripsY = (HiColor*) pSource->colorstripsY;
	pDest->paletteType	= (EdgemapPalette) pSource->paletteType;

	pDest->segments		= pSource->segments;
	pDest->size			= { pSource->size.w, pSource->size.h };

	if( pSource->tintmaps != nullptr )
	{
		for( int i = 0 ; i < pSource->segments ; i++ )
			pSpaceForTintmaps[i] = static_cast<Tintmap*>(reinterpret_cast<Object*>(pSource->tintmaps[i]));

		pDest->tintmaps = pSpaceForTintmaps;
	}
	else
		pDest->tintmaps		= nullptr;
}
