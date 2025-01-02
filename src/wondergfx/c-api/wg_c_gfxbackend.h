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

#ifndef WG_C_GFXBACKEND_DOT_H
#define WG_C_GFXBACKEND_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


//____ wg_canvasInfo __________________________________________________________

typedef struct wg_canvasInfo_struct		// NOT BINARY EQUIVALENT!
{
	wg_canvasRef	ref;
	wg_obj			surface;
	wg_sizeSPX		size;
	int				scale;
} wg_canvasInfo;

//____ wg_sessionInfo _________________________________________________________

typedef struct wg_sessionInfo_struct
{
	// Commands

	int		nSetCanvas;			// Number of setCanvas calls in session.
	int		nStateChanges;		// Number of times state will change through session.
	int		nLines;				// Number of line commands.
	int		nFill;
	int		nBlit;				// Includes Blit, ClipBlit och Tile.
	int		nBlur;
	int		nEdgemapDraws;
	int		nLineCoords;		// Total number of coordinates for line commands (=total lines in line commands *2)

	// Total content of coord buffers

	int		nLineClipRects;		// Total number of clip rectangles for line commands.
	int		nRects;				// Total number of rects for fill, blit, blur and edgemapDraw commands.

	// Total size of other buffers

	int		nColors;			// Total number of colors to be used through all commands.
	int		nTransforms;		// Total number of transforms for session.
	int		nObjects;			// Total number of object references for session.

} wg_sessionInfo;


void	wg_backendBeginRender( wg_obj backend );
void	wg_backendEndRender( wg_obj backend );

void	wg_backendBeginSession( wg_obj backend, wg_canvasRef canvasRef, wg_obj canvasSurface, int nUpdateRects, const wg_rectSPX * pUpdateRects, const wg_sessionInfo * pInfo );
void	wg_backendEndSession( wg_obj backend );

void	wg_backendSetCanvasWithSurface( wg_obj backend, wg_obj surface );
void	wg_backendSetCanvasWithRef( wg_obj backend, wg_canvasRef canvas );

void	wg_backendSetObjects(wg_obj backend, wg_obj* const * pBeg, wg_obj* const * pEnd);

void	wg_backendSetRects(wg_obj backend, const wg_rectSPX* pBeg, const wg_rectSPX* pEnd);
void	wg_backendSetColors(wg_obj backend, const wg_color * pBeg, const wg_color * pEnd);
void	wg_backendSetTransforms(wg_obj backend, const wg_transform * pBeg, const wg_transform * pEnd);

void	wg_backendProcessCommands(wg_obj backend, const uint16_t* pBeg, const uint16_t * pEnd);


wg_canvasInfo wg_backendCanvasInfo(wg_obj backend, wg_canvasRef ref);

wg_obj	wg_backendSurfaceFactory(wg_obj backend);
wg_obj	wg_backendEdgemapFactory(wg_obj backend);


int		wg_backendMaxEdges(wg_obj backend);

const wg_typeInfo * wg_backendSurfaceType(wg_obj backend);


#ifdef __cplusplus
}
#endif

#endif

