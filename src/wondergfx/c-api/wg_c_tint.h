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
#ifndef WG_C_TINT_DOT_H
#define WG_C_TINT_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif

//____ wg_colorStop ____________________________________________________________

typedef struct wg_colorStop_struct
{
	float		pos;				// 0.0 -> 1.0
	wg_color	color;
} wg_colorStop;

//____ wg_tintBP _______________________________________________________________

typedef struct wg_tintBP_struct			// NOT BINARY EQUIVALENT!
{
	wg_coordF				begin;			// Linear: start, relative to rect. Default {0,0}.
	wg_coordF				center;			// Radial: center, relative to rect. Default {0.5,0.5}.
	wg_colorSpace			colorSpace;		// WG_COLORSPACE_LINEAR or WG_COLORSPACE_SRGB.
	wg_coordF				end;			// Linear: end, relative to rect. Default {0,1}.
	wg_sizeF				radius;			// Radial: radius. Default {0.5,0.5}.
	wg_tintRadius			radiusMode;
	wg_tintShape			shape;
	wg_tintSpread			spread;
	int						nbStops;		// 1 -> 16.
	const wg_colorStop *	stops;
} wg_tintBP;


WG_EXPORT wg_tintBP		wg_defaultTintBP();								// Blueprint with default values.

WG_EXPORT wg_obj		wg_createTint( const wg_tintBP* pBlueprint );		// Returns retained object or null.
WG_EXPORT wg_obj		wg_createTintMix( int nbComponents, const wg_obj* pComponents, const float* pWeights );	// Returns retained object or null. 1 -> 4 simple tints.
WG_EXPORT wg_obj		wg_mixTints( wg_obj fromTint, wg_obj toTint, float progress );	// Returns retained object.

WG_EXPORT int			wg_isTintOpaque( wg_obj tint );
WG_EXPORT int			wg_isTintFlat( wg_obj tint );
WG_EXPORT int			wg_isTintMix( wg_obj tint );

// Blueprint of a simple tint. Its stops point into the tint and stay valid as long as it does.
// A mix has no blueprint of its own, get its components instead.

WG_EXPORT wg_tintBP		wg_getTintBlueprint( wg_obj tint );

WG_EXPORT int			wg_tintMixComponents( wg_obj tint );				// 0 for a simple tint.
WG_EXPORT wg_obj		wg_tintMixComponent( wg_obj tint, int index );		// Not retained.
WG_EXPORT float			wg_tintMixWeight( wg_obj tint, int index );

WG_EXPORT wg_color		wg_tintColorAt( wg_obj tint, wg_coordSPX pos, const wg_rectSPX* pRect );
WG_EXPORT int			wg_tintAlphaAt( wg_obj tint, wg_coordSPX pos, const wg_rectSPX* pRect );	// 0 -> 4096.

// Tints as bytes, e.g. for passing them between plugin and host. See TintTools::serializeTint()
// for the format. A mix survives this, which it doesn't through a blueprint.

#define WG_MAX_TINT_DATA_BYTES	(4 + 4 * (44 + 16 * 12))

WG_EXPORT int			wg_exportTintData( wg_obj tint, void* pDest, int maxBytes );	// Returns bytes written, 0 if it doesn't fit. tint may be null.
WG_EXPORT wg_obj		wg_createTintFromData( const void* pData, int bytes );			// Returns retained object or null.

#ifdef __cplusplus
}
#endif

#endif
