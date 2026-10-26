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
#include <wg_c_patches.h>
#include <wg_patches.h>

#include <type_traits>

using namespace wg;

wg_patches wg_createPatches()
{
	return new PatchesSPX();
}

void wg_deletePatches(wg_patches obj)
{
	delete (PatchesSPX *) obj;
}

int wg_setPatchesCapacity( wg_patches obj, int capacity )
{
	return reinterpret_cast<PatchesSPX*>(obj)->capacity();
}

void wg_clearPatches(wg_patches obj)
{
	reinterpret_cast<PatchesSPX*>(obj)->clear();
}

void wg_addPatch( wg_patches obj, wg_rectSPX rect )
{
	reinterpret_cast<PatchesSPX*>(obj)->add( *(RectSPX*)&rect );
}

void wg_addPatches( wg_patches dest, wg_patches from )
{
	reinterpret_cast<PatchesSPX*>(dest)->add( reinterpret_cast<PatchesSPX*>(from) );
}

void wg_addPatchesSubset( wg_patches dest, wg_patches from, int ofs, int len )
{
	reinterpret_cast<PatchesSPX*>(dest)->add( reinterpret_cast<PatchesSPX*>(from), ofs, len );
}

void wg_subPatch( wg_patches obj, wg_rectSPX rect )
{
	reinterpret_cast<PatchesSPX*>(obj)->sub( *(RectSPX*)&rect );
}

void wg_subPatches( wg_patches dest, wg_patches from )
{
	reinterpret_cast<PatchesSPX*>(dest)->sub( reinterpret_cast<PatchesSPX*>(from) );
}

void wg_subPatchesSubset( wg_patches dest, wg_patches from, int ofs, int len )
{
	reinterpret_cast<PatchesSPX*>(dest)->sub( reinterpret_cast<PatchesSPX*>(from), ofs, len );
}

void wg_pushPatch( wg_patches obj, wg_rectSPX rect )
{
	reinterpret_cast<PatchesSPX*>(obj)->push( *(RectSPX*)&rect );
}

void wg_pushPatches( wg_patches dest, wg_patches from )
{
	reinterpret_cast<PatchesSPX*>(dest)->push( reinterpret_cast<PatchesSPX*>(from) );
}

void wg_pushPatchesSubset( wg_patches dest, wg_patches from, int ofs, int len )
{
	reinterpret_cast<PatchesSPX*>(dest)->push( reinterpret_cast<PatchesSPX*>(from), ofs, len );
}

void wg_popPatch( wg_patches obj )
{
	reinterpret_cast<PatchesSPX*>(obj)->pop();
}

void wg_trimPushPatches( wg_patches dest, wg_patches from, wg_rectSPX trim )
{
	reinterpret_cast<PatchesSPX*>(dest)->trimPush(*reinterpret_cast<PatchesSPX*>(from), *(RectSPX*)&trim );
}

int wg_removePatches( wg_patches obj, int ofs, int len )
{
	return reinterpret_cast<PatchesSPX*>(obj)->remove( ofs, len );
}

void wg_clipPatches( wg_patches obj, wg_rectSPX clip )
{
	reinterpret_cast<PatchesSPX*>(obj)->clip( *(RectSPX*)&clip );
}

wg_rectSPX wg_patchesBounds( wg_patches obj )
{
	RectSPX r = reinterpret_cast<PatchesSPX*>(obj)->bounds();

	return { r.x, r.y, r.w, r.h };
}

const wg_rectSPX * wg_patchesBegin( wg_patches obj )
{
	return reinterpret_cast<const wg_rectSPX*>( reinterpret_cast<PatchesSPX*>(obj)->begin() );
}

const wg_rectSPX * wg_patchesEnd( wg_patches obj )
{
	return reinterpret_cast<const wg_rectSPX*>( reinterpret_cast<PatchesSPX*>(obj)->end() );
}

int wg_patchesSize( wg_patches obj )
{
	return reinterpret_cast<PatchesSPX*>(obj)->size();
}

int wg_patchesCapacity( wg_patches obj )
{
	return reinterpret_cast<PatchesSPX*>(obj)->capacity();
}

int wg_patchesIsEmpty( wg_patches obj )
{
	return reinterpret_cast<PatchesSPX*>(obj)->isEmpty();
}
