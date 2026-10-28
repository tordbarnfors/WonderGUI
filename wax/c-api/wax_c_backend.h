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
#ifndef WAX_C_BACKEND_DOT_H
#define WAX_C_BACKEND_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


//____ wax_canvasInfo __________________________________________________________

typedef struct wax_canvasInfo_struct		// NOT BINARY EQUIVALENT!
{
	wax_canvasRef	ref;
	wax_obj			surface;
	wax_sizeSPX		size;
	int				scale;
} wax_canvasInfo;

//____ wax_sessionInfo _________________________________________________________

typedef struct wax_sessionInfo_struct		// Binary equivalent of Backend::SessionInfo.
{
	// Commands

	int		nSetCanvas;			// Number of setCanvas calls in session.
	int		nStateChanges;		// Number of times state will change through session.
	int		nLines;				// Number of line commands.
	int		nFill;
	int		nBlit;				// Includes Blit, ClipBlit och Tile.
	int		nBlur;
	int		nEdgemapDraws;		// Includes DrawEdgemap and TileEdgemap.
	int		nWarpDraws;			// WarpBlit and WarpDrawEdgemap.
	int		nLineCoords;		// Total number of coordinates for line commands (=total lines in line commands *2)

	// Total content of coord buffers

	int		nLineClipRects;		// Total number of clip rectangles for line commands.
	int		nRects;				// Total number of rects for fill, blit, blur and edgemapDraw commands.

	// Total size of other buffers

	int		nColors;			// Total number of colors to be used through all commands.
	int		nTransforms;		// Total number of transforms for session.
	int		nObjects;			// Total number of object references for session.

	// Shapes

	int		nShapeDraws;		// Number of DrawShape commands.
	int		nShapeSegments;		// Total number of segments (fill, outline, slices...) of all shapes.
	int		nShapeTints;		// Total number of Tints encoded in DrawShape commands.

} wax_sessionInfo;


WAX_EXPORT void	wax_backendBeginRender( wax_obj backend );
WAX_EXPORT void	wax_backendEndRender( wax_obj backend );

WAX_EXPORT void	wax_backendBeginSession( wax_obj backend, wax_canvasRef canvasRef, wax_obj canvasSurface, int nUpdateRects, const wax_rectSPX * pUpdateRects, const wax_sessionInfo * pInfo );
WAX_EXPORT void	wax_backendEndSession( wax_obj backend );

WAX_EXPORT void	wax_backendSetCanvasWithSurface( wax_obj backend, wax_obj surface );
WAX_EXPORT void	wax_backendSetCanvasWithRef( wax_obj backend, wax_canvasRef canvas );

WAX_EXPORT void	wax_backendSetObjects(wax_obj backend, wax_obj* const * pBeg, wax_obj* const * pEnd);

WAX_EXPORT void	wax_backendSetRects(wax_obj backend, const wax_rectSPX* pBeg, const wax_rectSPX* pEnd);
WAX_EXPORT void	wax_backendSetColors(wax_obj backend, const wax_color * pBeg, const wax_color * pEnd);
WAX_EXPORT void	wax_backendSetTransforms(wax_obj backend, const wax_transform * pBeg, const wax_transform * pEnd);

WAX_EXPORT void	wax_backendProcessCommands(wax_obj backend, const uint16_t* pBeg, const uint16_t * pEnd, int version);


WAX_EXPORT wax_canvasInfo wax_backendCanvasInfo(wax_obj backend, wax_canvasRef ref);

WAX_EXPORT wax_obj	wax_backendSurfaceFactory(wax_obj backend);
WAX_EXPORT wax_obj	wax_backendEdgemapFactory(wax_obj backend);
WAX_EXPORT wax_obj	wax_backendWarpmapFactory(wax_obj backend);


WAX_EXPORT int		wax_backendMaxEdges(wax_obj backend);

WAX_EXPORT const wax_typeInfo * wax_backendSurfaceType(wax_obj backend);


#ifdef __cplusplus
}
#endif

#endif

