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
#ifndef WAX_C_CANVASLAYERS_DOT_H
#define WAX_C_CANVASLAYERS_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif


#define		WAX_MAX_CANVAS_LAYERS	16							// Should be synced with CanvasLayers::c_maxLayers.
	typedef	void(*wax_canvasLayers_func)(wax_obj gfxDevice, void * pData, int data );


	//____ wax_canvasLayersEntryBP ________________________________________________

	typedef struct wax_canvasLayersEntryBP_struct
	{
		wax_pixelFormat			format;
		wax_colorSpace			colorSpace;		// WAX_COLORSPACE_UNDEFINED gives the color space of the canvas.
		wax_canvasLayers_func	blendFunc;
		void*					blendPtr;
		int						blendInt;
		wax_canvasLayers_func	clearFunc;
		void*					clearPtr;
		int						clearInt;
		wax_canvasLayers_func	preBlendFunc;
		void*					preBlendPtr;
		int						preBlendInt;
		wax_canvasLayers_func	preBlendCanvasFunc;
		void*					preBlendCanvasPtr;
		int						preBlendCanvasInt;
	} wax_canvasLayersEntryBP;


	//____ wax_canvasLayersBP _____________________________________________________

	typedef struct wax_canvasLayersBP_struct
	{
		wax_canvasLayers_func	clearCanvasFunc;
		void*					clearCanvasPtr;
		int						clearCanvasInt;
		wax_canvasLayers_func	finalizeCanvasFunc;
		void*					finalizeCanvasPtr;
		int						finalizeCanvasInt;

		int						baseLayer;
		wax_canvasLayersEntryBP	layers[WAX_MAX_CANVAS_LAYERS];				// Need to end list with entry where format = WAX_PIXFMT_UNDEFINED unless full.

	} wax_canvasLayersBP;


	WAX_EXPORT wax_obj			wax_createCanvasLayers(const wax_canvasLayersBP* pBlueprint);

	WAX_EXPORT int				wax_canvasLayersSize(wax_obj canvasLayers);
	WAX_EXPORT wax_pixelFormat	wax_layerFormat(wax_obj canvasLayers, int layer);
	WAX_EXPORT int				wax_baseLayer(wax_obj canvasLayers);


#ifdef __cplusplus
}
#endif

#endif
