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
#include <wax_c_surfacewriter.h>
#include <wax_surfacewriter.h>

using namespace wax;

inline SurfaceWriter* getPtr(wax_obj obj) {
	return static_cast<SurfaceWriter*>(reinterpret_cast<Object*>(obj));
}


wax_obj wax_createSurfaceWriter(const wax_surfaceWriterBP* blueprint)
{
	SurfaceWriter::Blueprint bp;

	bp.saveInfo.scale = (blueprint->saveInfo & WAX_SAVE_SCALE);
	bp.saveInfo.identity = (blueprint->saveInfo & WAX_SAVE_IDENTITY);
	bp.saveInfo.sampleMethod = (blueprint->saveInfo & WAX_SAVE_SAMPLE_METHOD);
	bp.saveInfo.mipmap = (blueprint->saveInfo & WAX_SAVE_MIPMAP_FLAG);
	bp.saveInfo.tiling = (blueprint->saveInfo & WAX_SAVE_TILING_FLAG);
	bp.saveInfo.dynamic = (blueprint->saveInfo & WAX_SAVE_DYNAMIC_FLAG);
	bp.saveInfo.buffered = (blueprint->saveInfo & WAX_SAVE_BUFFERED_FLAG);
	bp.saveInfo.canvas = (blueprint->saveInfo & WAX_SAVE_CANVAS_FLAG);
	
	auto pWriter = SurfaceWriter::create(bp);
	pWriter->retain();
	return static_cast<Object*>(pWriter);
}

wax_obj	wax_writeSurfaceToBlob( wax_obj surfaceWriter, wax_obj surface, int extraDataSize, char * pExtraData )
{
	auto pSurface = static_cast<Surface*>(reinterpret_cast<Object*>(surface));
	auto pBlob = getPtr(surfaceWriter)->writeSurfaceToBlob(pSurface);
	pBlob->retain();
	return static_cast<Object*>(pBlob);
}
