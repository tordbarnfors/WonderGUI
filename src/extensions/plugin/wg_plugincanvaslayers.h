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
#ifndef WG_PLUGINCANVASLAYERS_DOT_H
#define WG_PLUGINCANVASLAYERS_DOT_H
#pragma once

#include <wg_canvaslayers.h>


typedef void* wg_obj;

namespace wg
{
	class PluginSurface;
	class PluginGfxDevice;
	typedef	StrongPtr<PluginGfxDevice>	PluginGfxDevice_p;
	typedef	WeakPtr<PluginGfxDevice>		PluginGfxDevice_wp;


	class PluginCanvasLayers;
	typedef	StrongPtr<PluginCanvasLayers>	PluginCanvasLayers_p;
	typedef	WeakPtr<PluginCanvasLayers>	PluginCanvasLayers_wp;

	//____ Class PluginCanvasLayers ________________________________________________________

	class	PluginCanvasLayers : public CanvasLayers
	{
		friend class PluginGfxDevice;

	public:

		//.____ Blueprint _________________________________________

		// Uses same blueprint as base class.

		//.____ Creation ______________________________________________________

		static PluginCanvasLayers_p	create( PluginGfxDevice * pDevice, const Blueprint& blueprint);

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		inline wg_obj		cObject() { return m_cCanvasLayer; }

	protected:
		PluginCanvasLayers( PluginGfxDevice * pDevice, const Blueprint& bp);
		~PluginCanvasLayers();

		static void _callbackRouter( wg_obj cDevice, void* pMe, int funcNb );

		wg_obj			m_cCanvasLayer;
		PluginGfxDevice_p	m_pDevice;
	};



} // namespace wg
#endif //WG_PLUGINCANVASLAYERS_DOT_H
