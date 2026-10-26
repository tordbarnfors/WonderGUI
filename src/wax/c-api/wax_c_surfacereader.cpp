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
#include <wax_c_surfacereader.h>
#include <wax_surfacereader.h>

using namespace wax;

inline SurfaceReader* getPtr(wax_obj obj) {
	return static_cast<SurfaceReader*>(reinterpret_cast<Object*>(obj));
}


wax_obj wax_createSurfaceReader(const wax_surfaceReaderBP* blueprint)
{
	SurfaceReader::Blueprint bp;
	bp.factory = blueprint->factory == nullptr ? nullptr : static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(blueprint->factory));
	
	auto pReader = SurfaceReader::create(bp);
	pReader->retain();
	return static_cast<Object*>(pReader);
}

wax_obj wax_readSurfaceFromBlob( wax_obj surfaceReader, wax_obj blob )
{
	auto pSurface = getPtr(surfaceReader)->readSurfaceFromBlob(static_cast<Blob*>(reinterpret_cast<Object*>(blob)));
	pSurface->retain();
	return static_cast<Object*>(pSurface);
}

wax_obj wax_readSurfaceFromMemory( wax_obj surfaceReader, const char * pData )
{
	auto pSurface = getPtr(surfaceReader)->readSurfaceFromMemory(pData);
	pSurface->retain();
	return static_cast<Object*>(pSurface);
}
