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

#ifndef WG_CANVASLAYERS_DOT_H
#define WG_CANVASLAYERS_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_object.h>
#include <wg_color.h>

#include <initializer_list>
#include <utility>
#include <functional>
#include <vector>

namespace wg
{
    class Surface;
    class GfxDevice;

	class CanvasLayers;
	typedef	StrongPtr<CanvasLayers>	CanvasLayers_p;
	typedef	WeakPtr<CanvasLayers>	CanvasLayers_wp;

	//____ Class CanvasLayers ________________________________________________________
	/**
	 * @class CanvasLayers
	 * @author Tord Jansson
	 * @date 22/01/21
	 * @file wg_canvaslayers.h
	 * @brief Specifies canvas layers and the operations on them.
	 * 
	 * CanvasLayers specifies layers 1 and above when performing multi-layer rendering.
	 * 
	 * Layer 0 (also called the base layer) is the surface set as canvas. The first specified layer
	 * in this CanvasLayer becomes layer 1. A maximum of 16 layers can be specified.
	 * 
	 * The surface of a layer is created on demand when the first widget or skin attempts to render to it
	 * (calls GfxDevice::setRenderLayer(layer-number)), thus there is no cost for specifying layers that
	 * are not used.
	 * 
	 * When the canvas is completed (when GfxDevice::endCanvasUpdate() is called), the generated
	 * surfaces are blended onto the base layer and destroyed, resulting in the canvas surface containing
	 * the composite image.
	 * 
	 * The format of the surface and the operation used to blend it onto the canvas surface can be specified.
	 * Several other methods can also be specified, dealing with initializing and finalizing the individual layer
	 * surfaces.
	 * 
	 * Using CanvasLayers allows for advanced rendering operations where shadows, glow, glass-effects etc
	 * can be inserted between backgrounds and foregrounds of the UI. It is also an easy way to bog down the user
	 * interface by allocating and blending large surfaces on the fly, using large amounts of memory, GPU and CPU.
	 * 
	 * CanvasLayers can also produce unexpected results when different widgets overlap. I.e. if an overlapping
	 * widget renders all its contents to a layer below the layer used by the widget behind it, 
	 * the content of the widget behind it could be rendered ontop anyway (if it really does or not depends on technical
	 * details of optimizations in the rendering pipeline we won't go into here, the point is you should be careful not to 
	 * mess with layers in a way that contradicts the layering of widgets).
	 * 
	 * It's a powerful tool, use it with caution.
	 */


	class	CanvasLayers : public Object	
	{
		friend class GfxDevice;
		friend class GfxDeviceGen1;
		friend class GfxDeviceGen2;

	public:

		//.____ Blueprint _________________________________________

		struct LayerBP
		{
/*
			LayerBP() {}
			LayerBP(PixelFormat format) : format(format) {}
			LayerBP(PixelFormat format, const std::function<void(GfxDevice* pDevice)>& blendFunc,
				const std::function<void(GfxDevice* pDevice)>& clearFunc = nullptr,
				const std::function<void(GfxDevice* pDevice)>& preBlendFunc = nullptr,
				const std::function<void(GfxDevice* pDevice)>& preBlendCanvasFunc = nullptr)
				: format(format), blendFunc(blendFunc), clearFunc(clearFunc), preBlendCanvasFunc(preBlendCanvasFunc), preBlendFunc(preBlendFunc) {}
*/

			PixelFormat format = PixelFormat::Undefined;

			std::function<void(GfxDevice* pDevice)> blendFunc = nullptr;
			std::function<void(GfxDevice* pDevice)> clearFunc = nullptr;
			std::function<void(GfxDevice* pDevice)> preBlendCanvasFunc = nullptr;
			std::function<void(GfxDevice* pDevice)> preBlendFunc = nullptr;
		};

		struct Blueprint
		{
			int						baseLayer = 0;

			std::function<void(GfxDevice* pDevice)>  clearCanvasFunc = nullptr;
			std::function<void(GfxDevice* pDevice)>  finalizeCanvasFunc = nullptr;

			std::vector<LayerBP>	layers;
		};

		//.____ Creation ______________________________________________________

		static CanvasLayers_p	create(const Blueprint& blueprint);

		//.____ Identification _________________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		inline int				size() const { return int(m_layers.size()); }

		PixelFormat				layerFormat(int layer) const;
		inline int				baseLayer() const { return m_baseLayer;  }

		//.____ Misc __________________________________________________________
		
		static constexpr int	maxLayers() { return c_maxLayers; }
		
		
	protected:
		CanvasLayers(const Blueprint& bp);
		~CanvasLayers() {}

		const static int	    c_maxLayers = 16;

		inline PixelFormat      _layerFormat(int i) const { return m_layers[i-1].format; }
        inline const std::function<void(GfxDevice * pDevice)>& _layerClearFunc(int i) const { return m_layers[i-1].clearFunc; }
        inline const std::function<void(GfxDevice * pDevice)>& _layerPreBlendFunc(int i) const { return m_layers[i-1].preBlendFunc; }
        inline const std::function<void(GfxDevice * pDevice)>& _layerBlendFunc(int i) const { return m_layers[i-1].blendFunc; }

        inline const std::function<void(GfxDevice * pDevice)>& _canvasClearFunc() const { return m_clearCanvasFunc; }
        inline const std::function<void(GfxDevice * pDevice)>& _canvasFinalizeFunc() const { return m_finalizeCanvasFunc; }

        inline const std::function<void(GfxDevice * pDevice)>& _canvasModifier(int i) const { return m_layers[i-1].preBlendCanvasFunc; }



        int						m_baseLayer = 0;                // 0 Means that the canvas surface is the base layer.
		std::vector<LayerBP>	m_layers;
        
        std::function<void(GfxDevice * pDevice)>  m_clearCanvasFunc = nullptr;
        std::function<void(GfxDevice * pDevice)>  m_finalizeCanvasFunc = nullptr;
        
    };



} // namespace wg
#endif //WG_CANVASLAYERS_DOT_H
