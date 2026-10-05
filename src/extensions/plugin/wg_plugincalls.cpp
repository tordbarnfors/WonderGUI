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
#include <wg_plugincalls.h>
#include <wg_tinttools.h>

namespace wg
{

	wg_bitmapcache_calls*		PluginCalls::bitmapCache	= nullptr;
	wg_bitmapfont_calls*		PluginCalls::bitmapFont		= nullptr;
	wg_canvaslayers_calls*		PluginCalls::canvasLayers 	= nullptr;
	wg_font_calls*				PluginCalls::font			= nullptr;
	wg_object_calls*			PluginCalls::object			= nullptr;
	wg_gfxdevice_calls*			PluginCalls::gfxDevice		= nullptr;
//	wg_streambuffer_calls*		PluginCalls::streamBuffer	= nullptr;
//	wg_streamplayer_calls*		PluginCalls::streamPlayer	= nullptr;
//	wg_streampump_calls*		PluginCalls::streamPump		= nullptr;
//	wg_streamreader_calls*		PluginCalls::streamReader	= nullptr;
	wg_surface_calls*			PluginCalls::surface		= nullptr;
	wg_surfacefactory_calls*	PluginCalls::surfaceFactory = nullptr;
	wg_edgemap_calls*			PluginCalls::edgemap		= nullptr;
	wg_edgemapfactory_calls*	PluginCalls::edgemapFactory = nullptr;
	wg_hostbridge_calls*		PluginCalls::hostBridge		= nullptr;
	wg_plugincapsule_calls*		PluginCalls::pluginCapsule	= nullptr;
	wg_blurbrush_calls*			PluginCalls::blurbrush		= nullptr;
	wg_tint_calls*				PluginCalls::tint			= nullptr;


	//___ _init() ______________________________________________________________

	bool PluginCalls::_init(wg_plugin_interface* pCallsCollection)
	{
			
		if (pCallsCollection->structSize < sizeof(wg_plugin_interface))
			goto	error_too_old_abi;

		if (pCallsCollection->pBitmapCache->structSize < sizeof(wg_bitmapcache_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pBitmapFont->structSize < sizeof(wg_bitmapfont_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pCanvasLayers->structSize < sizeof(wg_canvaslayers_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pFont->structSize < sizeof(wg_font_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pObject->structSize < sizeof(wg_object_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pGfxDevice->structSize < sizeof(wg_gfxdevice_calls))
			goto	error_too_old_abi;

/*
		if (pCallsCollection->pStreamBuffer->structSize < sizeof(wg_streambuffer_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pStreamPlayer->structSize < sizeof(wg_streamplayer_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pStreamPump->structSize < sizeof(wg_streampump_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pStreamReader->structSize < sizeof(wg_streamreader_calls))
			goto	error_too_old_abi;
*/

		if (pCallsCollection->pSurface->structSize < sizeof(wg_surface_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pSurfaceFactory->structSize < sizeof(wg_surfacefactory_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pEdgemap->structSize < sizeof(wg_edgemap_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pEdgemapFactory->structSize < sizeof(wg_edgemapfactory_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pHostBridge->structSize < sizeof(wg_hostbridge_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pPluginCapsule->structSize < sizeof(wg_plugincapsule_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pBlurbrush->structSize < sizeof(wg_blurbrush_calls))
			goto	error_too_old_abi;

		if (pCallsCollection->pTint->structSize < sizeof(wg_tint_calls))
			goto	error_too_old_abi;


		bitmapCache		= pCallsCollection->pBitmapCache;
		bitmapFont		= pCallsCollection->pBitmapFont;
		canvasLayers	= pCallsCollection->pCanvasLayers;
		font			= pCallsCollection ->pFont;
		object			= pCallsCollection->pObject;
		gfxDevice		= pCallsCollection->pGfxDevice;
//		streamBuffer	= pCallsCollection->pStreamBuffer;
//		streamPlayer	= pCallsCollection->pStreamPlayer;
//		streamPump		= pCallsCollection->pStreamPump;
		surface			= pCallsCollection->pSurface;
		surfaceFactory	= pCallsCollection->pSurfaceFactory;
		edgemap			= pCallsCollection->pEdgemap;
		edgemapFactory = pCallsCollection->pEdgemapFactory;
		hostBridge		= pCallsCollection->pHostBridge;
		pluginCapsule	= pCallsCollection->pPluginCapsule;
		blurbrush		= pCallsCollection->pBlurbrush;
		tint			= pCallsCollection->pTint;

		return true;

	error_too_old_abi:
		return false;
	}


	//____ _hostTint() ________________________________________________________

	wg_obj PluginCalls::_hostTint(Tint* pTint)
	{
		if( !pTint )
			return nullptr;

		uint8_t	buffer[TintTools::c_maxSerializedTintBytes];
		int bytes = TintTools::serializeTint(pTint, buffer);

		return tint->createTintFromData(buffer, bytes);
	}

	//____ _localTint() _______________________________________________________

	Tint_p PluginCalls::_localTint(wg_obj hostTint)
	{
		if( !hostTint )
			return nullptr;

		uint8_t	buffer[TintTools::c_maxSerializedTintBytes];
		int bytes = tint->exportTintData(hostTint, buffer, TintTools::c_maxSerializedTintBytes);
		if( bytes <= 0 )
			return nullptr;

		int bytesRead = 0;
		return TintTools::deserializeTint(buffer, bytesRead);
	}

}
