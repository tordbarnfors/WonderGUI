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
#include <wg_plugincanvaslayers.h>
#include <wg_plugingfxdevice.h>
#include <wg_base.h>

#include <wg_plugincalls.h>

#include <cstring>

namespace wg
{
	const TypeInfo PluginCanvasLayers::TYPEINFO = { "PluginCanvasLayers", &CanvasLayers::TYPEINFO };


	//____ create() ___________________________________________________________

	PluginCanvasLayers_p PluginCanvasLayers::create(PluginGfxDevice* pDevice, const Blueprint& bp)
	{
		// Sanity check

		if (bp.layers.size() > c_maxLayers)
		{
			Base::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Number of layers exceeds maximum", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		for (auto& layer : bp.layers)
		{
			PixelFormat format = layer.format;

			if (format == PixelFormat::Index_8 || format == PixelFormat::Index_8_sRGB || format == PixelFormat::Index_8_linear)
			{
				Base::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Canvas layers can not be palette-based", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return nullptr;
			}
		}

		if (bp.baseLayer < 0 || bp.baseLayer > bp.layers.size())
		{
			Base::throwError(ErrorLevel::Critical, ErrorCode::InvalidParam, "Default layer is out of bounds.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		//

		return PluginCanvasLayers_p(new PluginCanvasLayers(pDevice,bp));
	}

	//____ constructor() ______________________________________________________

	PluginCanvasLayers::PluginCanvasLayers( PluginGfxDevice * pDevice, const Blueprint& bp) : CanvasLayers(bp)
	{
		m_pDevice = pDevice;

		wg_canvasLayersBP cBP;

		memset(&cBP, 0, sizeof(wg_canvasLayersBP));

		cBP.baseLayer = bp.baseLayer;

		if (bp.clearCanvasFunc)
		{
			cBP.clearCanvasFunc = _callbackRouter;
			cBP.clearCanvasPtr = this;
			cBP.clearCanvasInt = 1;
		}

		if (bp.finalizeCanvasFunc)
		{
			cBP.finalizeCanvasFunc = _callbackRouter;
			cBP.finalizeCanvasPtr = this;
			cBP.finalizeCanvasInt = 2;
		}

		int nLayers = bp.layers.size();

		for (int i = 0; i < nLayers; i++)
		{
			cBP.layers[i].format = (wg_pixelFormat) bp.layers[i].format;

			if (bp.layers[i].preBlendCanvasFunc)
			{
				cBP.layers[i].preBlendCanvasFunc = _callbackRouter;
				cBP.layers[i].preBlendCanvasPtr = this;
				cBP.layers[i].preBlendCanvasInt = 3 + (i * 4);
			}

			if (bp.layers[i].clearFunc)
			{
				cBP.layers[i].clearFunc = _callbackRouter;
				cBP.layers[i].clearPtr = this;
				cBP.layers[i].clearInt = 3 + (i * 4)+1;
			}

			if (bp.layers[i].blendFunc)
			{
				cBP.layers[i].blendFunc = _callbackRouter;
				cBP.layers[i].blendPtr = this;
				cBP.layers[i].blendInt = 3 + (i * 4) + 2;
			}

			if (bp.layers[i].preBlendFunc)
			{
				cBP.layers[i].preBlendFunc = _callbackRouter;
				cBP.layers[i].preBlendPtr = this;
				cBP.layers[i].preBlendInt = 3 + (i * 4) + 3;
			}

		}

		m_cCanvasLayer = PluginCalls::canvasLayers->createCanvasLayers(&cBP);
	}

	//____ PluginCanvasLayers::~PluginCanvasLayers() ______________________________

	PluginCanvasLayers::~PluginCanvasLayers()
	{
		PluginCalls::object->release(m_cCanvasLayer);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginCanvasLayers::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _callbackRouter() __________________________________________________

	void PluginCanvasLayers::_callbackRouter( wg_obj cDevice, void* _pMe, int funcNb )
	{
		PluginCanvasLayers* pMe = (PluginCanvasLayers*)_pMe;

		GfxDevice* pDevice = pMe->m_pDevice;

		if (funcNb == 1)
			pMe->m_clearCanvasFunc(pDevice);
		else if (funcNb == 2)
			pMe->m_finalizeCanvasFunc(pDevice);
		else
		{
			auto& layer = pMe->m_layers[(funcNb - 3) / 4];

			int func = (funcNb - 3) % 4;

			switch (func)
			{
				case 0:
					layer.preBlendCanvasFunc(pDevice);
					break;
				case 1:
					layer.clearFunc(pDevice);
					break;
				case 2:
					layer.blendFunc(pDevice);
					break;
				case 3:
					layer.preBlendFunc(pDevice);
					break;

				default:
					break;
			}
		}
	}
}
