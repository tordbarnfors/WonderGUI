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
#ifndef WG_C_CANVASLAYERS_DOT_H
#define WG_C_CANVASLAYERS_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif


#define		WG_MAX_CANVAS_LAYERS	16							// Should be synced with CanvasLayers::c_maxLayers.
	typedef	void(*wg_canvasLayers_func)(wg_obj gfxDevice, void * pData, int data );


	//____ wg_canvasLayersEntryBP ________________________________________________

	typedef struct wg_canvasLayersEntryBP_struct
	{
		wg_pixelFormat			format;
		wg_canvasLayers_func	blendFunc;
		void*					blendPtr;
		int						blendInt;
		wg_canvasLayers_func	clearFunc;
		void*					clearPtr;
		int						clearInt;
		wg_canvasLayers_func	preBlendFunc;
		void*					preBlendPtr;
		int						preBlendInt;
		wg_canvasLayers_func	preBlendCanvasFunc;
		void*					preBlendCanvasPtr;
		int						preBlendCanvasInt;
	} wg_canvasLayersEntryBP;


	//____ wg_canvasLayersBP _____________________________________________________

	typedef struct wg_canvasLayersBP_struct
	{
		wg_canvasLayers_func	clearCanvasFunc;
		void*					clearCanvasPtr;
		int						clearCanvasInt;
		wg_canvasLayers_func	finalizeCanvasFunc;
		void*					finalizeCanvasPtr;
		int						finalizeCanvasInt;

		int						baseLayer;
		wg_canvasLayersEntryBP	layers[WG_MAX_CANVAS_LAYERS];				// Need to end list with entry where format = WG_PIXFMT_UNDEFINED unless full.

	} wg_canvasLayersBP;


	WG_EXPORT wg_obj			wg_createCanvasLayers(const wg_canvasLayersBP* pBlueprint);

	WG_EXPORT int				wg_canvasLayersSize(wg_obj canvasLayers);
	WG_EXPORT wg_pixelFormat	wg_layerFormat(wg_obj canvasLayers, int layer);
	WG_EXPORT int				wg_baseLayer(wg_obj canvasLayers);


#ifdef __cplusplus
}
#endif

#endif
