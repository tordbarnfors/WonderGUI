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
#include <wg_c_warpgenerator.h>
#include <wg_warpgenerator.h>

using namespace wg;

inline WarpGenerator* getPtr(wg_obj obj) {
	return static_cast<WarpGenerator*>(reinterpret_cast<Object*>(obj));
}

int wg_setWarpGeneratorSize( wg_obj warpGenerator, wg_sizeI size )
{
	return getPtr(warpGenerator)->setSize({ size.w, size.h });
}

wg_sizeI wg_warpGeneratorSize( wg_obj warpGenerator )
{
	SizeI sz = getPtr(warpGenerator)->size();
	return { sz.w, sz.h };
}

int wg_setWarpGeneratorSourceSize( wg_obj warpGenerator, wg_sizeSPX sourceSize )
{
	return getPtr(warpGenerator)->setSourceSize({ sourceSize.w, sourceSize.h });
}

wg_sizeSPX wg_warpGeneratorSourceSize( wg_obj warpGenerator )
{
	SizeSPX sz = getPtr(warpGenerator)->sourceSize();
	return { sz.w, sz.h };
}

int wg_setWarpGeneratorCellSize( wg_obj warpGenerator, int cellSize )
{
	return getPtr(warpGenerator)->setCellSize(cellSize);
}

int wg_warpGeneratorCellSize( wg_obj warpGenerator )
{
	return getPtr(warpGenerator)->cellSize();
}

int wg_setWarpGeneratorPrecision( wg_obj warpGenerator, float precision )
{
	return getPtr(warpGenerator)->setPrecision(precision);
}

float wg_warpGeneratorPrecision( wg_obj warpGenerator )
{
	return getPtr(warpGenerator)->precision();
}

wg_obj wg_generateWarpGenerator( wg_obj warpGenerator )
{
	Warpmap_p p = getPtr(warpGenerator)->generate();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}
