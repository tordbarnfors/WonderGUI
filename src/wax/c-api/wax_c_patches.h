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
#ifndef WAX_C_PATCHES_DOT_H
#define WAX_C_PATCHES_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* wax_patches;

WAX_EXPORT wax_patches	wax_createPatches();
WAX_EXPORT void			wax_deletePatches(wax_patches obj);

WAX_EXPORT int			wax_setPatchesCapacity( wax_patches obj, int capacity );

WAX_EXPORT void			wax_clearPatches(wax_patches obj);

WAX_EXPORT void			wax_addPatch( wax_patches obj, wax_rectSPX rect );
WAX_EXPORT void			wax_addPatches( wax_patches dest, wax_patches from );
WAX_EXPORT void			wax_addPatchesSubset( wax_patches dest, wax_patches from, int ofs, int len );

WAX_EXPORT void			wax_subPatch(wax_patches obj, wax_rectSPX rect);
WAX_EXPORT void			wax_subPatches(wax_patches dest, wax_patches from);
WAX_EXPORT void			wax_subPatchesSubset( wax_patches dest, wax_patches from, int ofs, int len );

WAX_EXPORT void			wax_pushPatch(wax_patches obj, wax_rectSPX rect);
WAX_EXPORT void			wax_pushPatches(wax_patches dest, wax_patches from);
WAX_EXPORT void			wax_pushPatchesSubset( wax_patches dest, wax_patches from, int ofs, int len );

WAX_EXPORT void			wax_popPatch( wax_patches obj );
WAX_EXPORT void			wax_trimPushPatches( wax_patches dest, wax_patches from, wax_rectSPX trim );

WAX_EXPORT int			wax_removePatches( wax_patches dest, int ofs, int len );

//.____ Misc __________________________________________________________

WAX_EXPORT void			wax_clipPatches(	wax_patches obj, wax_rectSPX clip );
WAX_EXPORT wax_rectSPX	wax_patchesBounds( wax_patches obj );


//.____ Content ________________________________________________________

WAX_EXPORT const wax_rectSPX * wax_patchesBegin( wax_patches obj );
WAX_EXPORT const wax_rectSPX * wax_patchesEnd( wax_patches obj );

WAX_EXPORT int			wax_patchesSize( wax_patches obj );
WAX_EXPORT int			wax_patchesCapacity( wax_patches obj );
WAX_EXPORT int			wax_patchesIsEmpty( wax_patches obj );


#ifdef __cplusplus
}
#endif

#endif
