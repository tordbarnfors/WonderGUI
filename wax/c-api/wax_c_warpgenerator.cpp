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
#include <wax_c_warpgenerator.h>
#include <wax_warpgenerator.h>

using namespace wax;

inline WarpGenerator* getPtr(wax_obj obj) {
	return static_cast<WarpGenerator*>(reinterpret_cast<Object*>(obj));
}

int wax_setWarpGeneratorSize( wax_obj warpGenerator, wax_sizeI size )
{
	return getPtr(warpGenerator)->setSize({ size.w, size.h });
}

wax_sizeI wax_warpGeneratorSize( wax_obj warpGenerator )
{
	SizeI sz = getPtr(warpGenerator)->size();
	return { sz.w, sz.h };
}

int wax_setWarpGeneratorSourceSize( wax_obj warpGenerator, wax_sizeSPX sourceSize )
{
	return getPtr(warpGenerator)->setSourceSize({ sourceSize.w, sourceSize.h });
}

wax_sizeSPX wax_warpGeneratorSourceSize( wax_obj warpGenerator )
{
	SizeSPX sz = getPtr(warpGenerator)->sourceSize();
	return { sz.w, sz.h };
}

int wax_setWarpGeneratorCellSize( wax_obj warpGenerator, int cellSize )
{
	return getPtr(warpGenerator)->setCellSize(cellSize);
}

int wax_warpGeneratorCellSize( wax_obj warpGenerator )
{
	return getPtr(warpGenerator)->cellSize();
}

int wax_setWarpGeneratorPrecision( wax_obj warpGenerator, float precision )
{
	return getPtr(warpGenerator)->setPrecision(precision);
}

float wax_warpGeneratorPrecision( wax_obj warpGenerator )
{
	return getPtr(warpGenerator)->precision();
}

wax_obj wax_refreshWarpGenerator( wax_obj warpGenerator )
{
	Warpmap_p p = getPtr(warpGenerator)->refresh();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}
