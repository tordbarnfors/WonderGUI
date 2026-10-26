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
#include <wax_c_canvaslayers.h>
#include <wax_canvaslayers.h>
#include <wax_gfxdevice.h>

using namespace wax;

inline CanvasLayers* getPtr(wax_obj obj) {
	return static_cast<CanvasLayers*>(reinterpret_cast<Object*>(obj));
}

static_assert(WAX_MAX_CANVAS_LAYERS == CanvasLayers::maxLayers(), "WAX_MAX_CANVAS_LAYERS needs to match CanvasLayers::MaxLayers()!" );

wax_obj wax_createCanvasLayers(const wax_canvasLayersBP* pBP)
{

	//TODO: Can we assign function directly without a lambda between? 
	// For this too work, the two casts below need to give the same result:
	// (void*) pDevice
	// reinterpret_cast<void*>(static_cast<Object*> pDevice);

	CanvasLayers::Blueprint bp;

	wax_canvasLayers_func func = pBP->finalizeCanvasFunc;
	void*	pointer = pBP->finalizeCanvasPtr;
	int		integer = pBP->finalizeCanvasInt;
	if (func != NULL)
		bp.finalizeCanvasFunc = [func,pointer,integer](GfxDevice* pDevice) { func(static_cast<Object*>(pDevice),pointer,integer); };

	func = pBP->clearCanvasFunc;
	pointer = pBP->clearCanvasPtr;
	integer = pBP->clearCanvasInt;
	if (func != NULL)
		bp.clearCanvasFunc = [func,pointer,integer](GfxDevice* pDevice) { func(static_cast<Object*>(pDevice),pointer,integer); };


	bp.baseLayer = pBP->baseLayer;

	for (int i = 0; i < WAX_MAX_CANVAS_LAYERS && pBP->layers[i].format != WAX_PIXFMT_UNDEFINED; i++)
	{
		CanvasLayers::LayerBP layer;

		layer.format = (PixelFormat) pBP->layers[i].format;
		layer.colorSpace = (ColorSpace) pBP->layers[i].colorSpace;

		func = pBP->layers[i].preBlendCanvasFunc;
		pointer = pBP->layers[i].preBlendCanvasPtr;
		integer = pBP->layers[i].preBlendCanvasInt;
		if( func != NULL )
			layer.preBlendCanvasFunc = [func,pointer,integer](GfxDevice* pDevice) { func(static_cast<Object*>(pDevice), pointer, integer); };

		func = pBP->layers[i].clearFunc;
		pointer = pBP->layers[i].clearPtr;
		integer = pBP->layers[i].clearInt;
		if (func != NULL)
			layer.clearFunc = [func, pointer, integer](GfxDevice* pDevice) { func(static_cast<Object*>(pDevice), pointer, integer); };

		func = pBP->layers[i].blendFunc;
		pointer = pBP->layers[i].blendPtr;
		integer = pBP->layers[i].blendInt;
		if (func != NULL)
			layer.blendFunc = [func, pointer, integer](GfxDevice* pDevice) { func(static_cast<Object*>(pDevice), pointer, integer); };

		func = pBP->layers[i].preBlendFunc;
		pointer = pBP->layers[i].preBlendPtr;
		integer = pBP->layers[i].preBlendInt;
		if (func != NULL)
			layer.blendFunc = [func, pointer, integer](GfxDevice* pDevice) { func(static_cast<Object*>(pDevice), pointer, integer); };

		bp.layers.push_back(layer);
	}

	auto p = CanvasLayers::create(bp);
	p->retain();
	return static_cast<Object*>(p);
}

int wax_canvasLayersSize(wax_obj canvasLayers)
{
	return getPtr(canvasLayers)->size();
}

wax_pixelFormat wax_layerFormat(wax_obj canvasLayers, int layer)
{
	return (wax_pixelFormat) getPtr(canvasLayers)->layerFormat(layer);
}

int wax_baseLayer(wax_obj canvasLayers)
{
	return getPtr(canvasLayers)->baseLayer();
}
