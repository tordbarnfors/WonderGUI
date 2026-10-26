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
#ifndef WAX_C_TINT_DOT_H
#define WAX_C_TINT_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif

//____ wax_colorStop ____________________________________________________________

typedef struct wax_colorStop_struct
{
	float		pos;				// 0.0 -> 1.0
	wax_color	color;
} wax_colorStop;

//____ wax_tintBP _______________________________________________________________

typedef struct wax_tintBP_struct			// NOT BINARY EQUIVALENT!
{
	wax_coordF				begin;			// Linear: start, relative to rect. Default {0,0}.
	wax_coordF				center;			// Radial: center, relative to rect. Default {0.5,0.5}.
	wax_coordF				end;			// Linear: end, relative to rect. Default {0,1}.
	wax_sizeF				radius;			// Radial: radius. Default {0.5,0.5}.
	wax_tintRadius			radiusMode;
	wax_tintShape			shape;
	wax_tintSpread			spread;
	int						nbStops;		// 1 -> 16.
	const wax_colorStop *	stops;
} wax_tintBP;


WAX_EXPORT wax_tintBP		wax_defaultTintBP();								// Blueprint with default values.

WAX_EXPORT wax_obj		wax_createTint( const wax_tintBP* pBlueprint );		// Returns retained object or null.
WAX_EXPORT wax_obj		wax_createTintMix( int nbComponents, const wax_obj* pComponents, const float* pWeights );	// Returns retained object or null. 1 -> 4 simple tints.
WAX_EXPORT wax_obj		wax_mixTints( wax_obj fromTint, wax_obj toTint, float progress );	// Returns retained object.

WAX_EXPORT int			wax_isTintOpaque( wax_obj tint );
WAX_EXPORT int			wax_isTintFlat( wax_obj tint );
WAX_EXPORT int			wax_isTintMix( wax_obj tint );

// Blueprint of a simple tint. Its stops point into the tint and stay valid as long as it does.
// A mix has no blueprint of its own, get its components instead.

WAX_EXPORT wax_tintBP		wax_getTintBlueprint( wax_obj tint );

WAX_EXPORT int			wax_tintMixComponents( wax_obj tint );				// 0 for a simple tint.
WAX_EXPORT wax_obj		wax_tintMixComponent( wax_obj tint, int index );		// Not retained.
WAX_EXPORT float			wax_tintMixWeight( wax_obj tint, int index );

WAX_EXPORT wax_color		wax_tintColorAt( wax_obj tint, wax_coordSPX pos, const wax_rectSPX* pRect );
WAX_EXPORT int			wax_tintAlphaAt( wax_obj tint, wax_coordSPX pos, const wax_rectSPX* pRect );	// 0 -> 4096.

// Tints as bytes, e.g. for passing them between plugin and host. See TintTools::serializeTint()
// for the format. A mix survives this, which it doesn't through a blueprint.

#define WAX_MAX_TINT_DATA_BYTES	(4 + 4 * (44 + 16 * 12))

WAX_EXPORT int			wax_exportTintData( wax_obj tint, void* pDest, int maxBytes );	// Returns bytes written, 0 if it doesn't fit. tint may be null.
WAX_EXPORT wax_obj		wax_createTintFromData( const void* pData, int bytes );			// Returns retained object or null.

#ifdef __cplusplus
}
#endif

#endif
