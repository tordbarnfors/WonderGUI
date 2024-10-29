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

#ifndef	WG_PLUGINCALLS_DOT_H
#define	WG_PLUGINCALLS_DOT_H
#pragma once

#include <wg_plugininterface.h>

namespace wg
{

	class PluginCalls
	{
	public:

		//.____ Internal __________________________________________
		
		static wg_bitmapcache_calls*	bitmapCache;
		static wg_bitmapfont_calls*		bitmapFont;
		static wg_canvaslayers_calls*	canvasLayers;
		static wg_font_calls*			font;
		static wg_object_calls*			object;
		static wg_gfxdevice_calls*		gfxDevice;
//		static wg_streambuffer_calls*	streamBuffer;
//		static wg_streamplayer_calls*	streamPlayer;
//		static wg_streampump_calls*		streamPump;
//		static wg_streamreader_calls*	streamReader;
		static wg_surface_calls*		surface;
		static wg_surfacefactory_calls* surfaceFactory;
		static wg_edgemap_calls*		edgemap;
		static wg_edgemapfactory_calls*edgemapFactory;
		static wg_hostbridge_calls*		hostBridge;
		static wg_plugincapsule_calls*	pluginCapsule;
		static wg_blurbrush_calls*		blurbrush;

		static bool _init(wg_plugin_interface* pCallsCollection);
	};


} // namespace wg
#endif //WG_PLUGINCALLS_DOT_H
