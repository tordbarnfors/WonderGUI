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

#include <wg_c_surfacewriter.h>
#include <wg_surfacewriter.h>

using namespace wg;

inline SurfaceWriter* getPtr(wg_obj obj) {
	return static_cast<SurfaceWriter*>(reinterpret_cast<Object*>(obj));
}


wg_obj wg_createSurfaceWriter(const wg_surfaceWriterBP* blueprint)
{
	SurfaceWriter::Blueprint bp;

	bp.saveInfo.scale = (blueprint->saveInfo & WG_SAVE_SCALE);
	bp.saveInfo.identity = (blueprint->saveInfo & WG_SAVE_IDENTITY);
	bp.saveInfo.sampleMethod = (blueprint->saveInfo & WG_SAVE_SAMPLE_METHOD);
	bp.saveInfo.mipmap = (blueprint->saveInfo & WG_SAVE_MIPMAP_FLAG);
	bp.saveInfo.tiling = (blueprint->saveInfo & WG_SAVE_TILING_FLAG);
	bp.saveInfo.dynamic = (blueprint->saveInfo & WG_SAVE_DYNAMIC_FLAG);
	bp.saveInfo.buffered = (blueprint->saveInfo & WG_SAVE_BUFFERED_FLAG);
	bp.saveInfo.canvas = (blueprint->saveInfo & WG_SAVE_CANVAS_FLAG);
	
	auto pWriter = SurfaceWriter::create(bp);
	pWriter->retain();
	return static_cast<Object*>(pWriter);
}

wg_obj	wg_writeSurfaceToBlob( wg_obj surfaceWriter, wg_obj surface, int extraDataSize, char * pExtraData )
{
	auto pSurface = static_cast<Surface*>(reinterpret_cast<Object*>(surface));
	auto pBlob = getPtr(surfaceWriter)->writeSurfaceToBlob(pSurface);
	pBlob->retain();
	return static_cast<Object*>(pBlob);
}
