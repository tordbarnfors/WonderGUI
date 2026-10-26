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
#include <wax_c_patches.h>
#include <wax_patches.h>

#include <type_traits>

using namespace wax;

wax_patches wax_createPatches()
{
	return new PatchesSPX();
}

void wax_deletePatches(wax_patches obj)
{
	delete (PatchesSPX *) obj;
}

int wax_setPatchesCapacity( wax_patches obj, int capacity )
{
	return reinterpret_cast<PatchesSPX*>(obj)->capacity();
}

void wax_clearPatches(wax_patches obj)
{
	reinterpret_cast<PatchesSPX*>(obj)->clear();
}

void wax_addPatch( wax_patches obj, wax_rectSPX rect )
{
	reinterpret_cast<PatchesSPX*>(obj)->add( *(RectSPX*)&rect );
}

void wax_addPatches( wax_patches dest, wax_patches from )
{
	reinterpret_cast<PatchesSPX*>(dest)->add( reinterpret_cast<PatchesSPX*>(from) );
}

void wax_addPatchesSubset( wax_patches dest, wax_patches from, int ofs, int len )
{
	reinterpret_cast<PatchesSPX*>(dest)->add( reinterpret_cast<PatchesSPX*>(from), ofs, len );
}

void wax_subPatch( wax_patches obj, wax_rectSPX rect )
{
	reinterpret_cast<PatchesSPX*>(obj)->sub( *(RectSPX*)&rect );
}

void wax_subPatches( wax_patches dest, wax_patches from )
{
	reinterpret_cast<PatchesSPX*>(dest)->sub( reinterpret_cast<PatchesSPX*>(from) );
}

void wax_subPatchesSubset( wax_patches dest, wax_patches from, int ofs, int len )
{
	reinterpret_cast<PatchesSPX*>(dest)->sub( reinterpret_cast<PatchesSPX*>(from), ofs, len );
}

void wax_pushPatch( wax_patches obj, wax_rectSPX rect )
{
	reinterpret_cast<PatchesSPX*>(obj)->push( *(RectSPX*)&rect );
}

void wax_pushPatches( wax_patches dest, wax_patches from )
{
	reinterpret_cast<PatchesSPX*>(dest)->push( reinterpret_cast<PatchesSPX*>(from) );
}

void wax_pushPatchesSubset( wax_patches dest, wax_patches from, int ofs, int len )
{
	reinterpret_cast<PatchesSPX*>(dest)->push( reinterpret_cast<PatchesSPX*>(from), ofs, len );
}

void wax_popPatch( wax_patches obj )
{
	reinterpret_cast<PatchesSPX*>(obj)->pop();
}

void wax_trimPushPatches( wax_patches dest, wax_patches from, wax_rectSPX trim )
{
	reinterpret_cast<PatchesSPX*>(dest)->trimPush(*reinterpret_cast<PatchesSPX*>(from), *(RectSPX*)&trim );
}

int wax_removePatches( wax_patches obj, int ofs, int len )
{
	return reinterpret_cast<PatchesSPX*>(obj)->remove( ofs, len );
}

void wax_clipPatches( wax_patches obj, wax_rectSPX clip )
{
	reinterpret_cast<PatchesSPX*>(obj)->clip( *(RectSPX*)&clip );
}

wax_rectSPX wax_patchesBounds( wax_patches obj )
{
	RectSPX r = reinterpret_cast<PatchesSPX*>(obj)->bounds();

	return { r.x, r.y, r.w, r.h };
}

const wax_rectSPX * wax_patchesBegin( wax_patches obj )
{
	return reinterpret_cast<const wax_rectSPX*>( reinterpret_cast<PatchesSPX*>(obj)->begin() );
}

const wax_rectSPX * wax_patchesEnd( wax_patches obj )
{
	return reinterpret_cast<const wax_rectSPX*>( reinterpret_cast<PatchesSPX*>(obj)->end() );
}

int wax_patchesSize( wax_patches obj )
{
	return reinterpret_cast<PatchesSPX*>(obj)->size();
}

int wax_patchesCapacity( wax_patches obj )
{
	return reinterpret_cast<PatchesSPX*>(obj)->capacity();
}

int wax_patchesIsEmpty( wax_patches obj )
{
	return reinterpret_cast<PatchesSPX*>(obj)->isEmpty();
}
