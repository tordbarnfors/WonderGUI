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

#include <wg_canvaslayers.h>
#include <wg_gfxbase.h>

namespace wg
{
	const TypeInfo CanvasLayers::TYPEINFO = { "CanvasLayers", &Object::TYPEINFO };

	//____ create() ___________________________________________________________
	/**
	 * @brief Creates a new CanvasLayer object with the layers specified.
	 * 
	 * Creates a new CanvasLayer object with layers of the formats specified. 
	 * 
	 * The layers are specified through a list of PixelFormats. The first value
	 * specifies the pixel format of layer 1, which is the first layer to be
	 * blended onto the canvas surface (which is layer 0) and thus the bottom
	 * layer, bar the canvas surface itself.
	 * 
	 * 
	 * 
	 * @param extraLayers - The PixelFormat for each layer, starting with layer 1 which is
	 * 						the bottom layer (bar the canvas surface itself, which is layer 0)
	 *						and moving upwards.
	 * 
	 * @return The created CanvasLayer object or nullptr if something went wrong.
	 */


	CanvasLayers_p CanvasLayers::create(const Blueprint& bp)
	{
		// Sanity check
		
		if( bp.layers.size() > c_maxLayers )
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Number of layers exceeds maximum", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}
		
		for (auto& layer : bp.layers)
		{
			PixelFormat format = layer.format;

			if (format == PixelFormat::Index_8 || format == PixelFormat::Index_8_sRGB || format == PixelFormat::Index_8_linear)
			{
				GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Canvas layers can not be palette-based", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return nullptr;
			}
						
		}

		if (bp.baseLayer < 0 || bp.baseLayer > bp.layers.size())
		{
			GfxBase::throwError(ErrorLevel::Critical, ErrorCode::InvalidParam, "Default layer is out of bounds.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		//

		return CanvasLayers_p(new CanvasLayers(bp));
	}

	//____ constructor() ______________________________________________________

	CanvasLayers::CanvasLayers(const Blueprint& bp)
	{
		m_baseLayer = bp.baseLayer;
		m_clearCanvasFunc = bp.clearCanvasFunc;
		m_finalizeCanvasFunc = bp.finalizeCanvasFunc;

		m_layers = bp.layers;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CanvasLayers::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ layerFormat() _____________________________________________________
	/**
	 * @brief Get PixelFormat of specificed layer.
	 * 
	 * Gets the PixelFormat of the specified layer.
	 * 
	 * @param layer		The layer we want to ask the format of. Please note that layer numbering starts on 1, not 0.
	 * 
	 * @return The PixelFormat of the specified layer.
	 */

	PixelFormat CanvasLayers::layerFormat(int layer) const
	{
		if (layer < 1 || layer > m_layers.size() )
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Layer does not exist.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return PixelFormat::Undefined;
		}

		return m_layers[layer-1].format;
	}


	//___ setDefaultLayer() ___________________________________________________
	/**
	 * @brief  Set default layer.
	 * 
	 * Sets the default layer, which is the layer unto which all widgets and skins are rendered unless otherwise specified. 
	 * 
	 * This value is initialized to 0.
	 * 
	 * @param layer		The layer to be set as default. 0 = Base layer, 1 = First specified layer in this CanvasLayer specification.
	 */


	//____ setLayerInitializer() _______________________________________________
	/**
	 * @brief Set a lambda function for clearing or initializing a layers surface.
	 * 
	 * Set a lambda function for clearing or initializing a layers surface.
	 * 
	 * @param layer		The layer for which we specify the initializer. Please note that layer numbering starts on 1, not 0.
	 * @param func		A lambda function to be used to initializing the layer after it is created.
	 * 
	 * By default a layer surface is filled with transparency when created, but that will be replaced by this function if specified.
	 * Use this function to fill the surface with any content you like before widgets or skins render into it.
	 * 
	 * When the function is called, it receives one parameter which is the GfxDevice to use. This has been prepared
	 * for operation in the following way:
	 * 
	 * - RenderTarget is set to the surface for the layer.
	 * - ClipList is set to only update patches that might be used.
	 * - BlendMode is unspecified, you need to set it!
	 * - Tint is cleared.
	 *
	 * BlendMode, TintColor and ClipList are automatically saved and restored. Any other settings you change
	 * (like BlitSource) will have to be saved and restored within the lambda.
	 * 
	 */

	//____ setLayerFinalizer() _________________________________________________
	/**
	 * @brief Set a lambda function to finalize the layers surface.
	 * 
	 * Set a lambda function to finalize the layers surface before it is blended onto the base layer (the specified canvas surface). 
	 * 
	 * This finalization step is purely optional, it just provides the option to modify the surface content before blending it onto the base layer.
	 * 
	 * @param layer		The layer for which we specify the finalizer. Please note that layer numbering starts on 1, not 0.
	 * @param func		A lambda function to be used to finalize the layer after it is created.
	 * 
	 * When the function is called, it receives one parameter which is the GfxDevice to use. This has been prepared
	 * for operation in the following way:
	 * 
	 * - RenderTarget is set to the surface for the layer.
	 * - ClipList is set to only update patches that might be used.
	 * - BlendMode is set to Blend.
	 * - Tint is cleared.
	 *
	 * No GfxDevice settings are saved or restored. Any settings you change will have to be saved and restored within the lambda.
	 * 
	 */

	//____ setLayerBlender() ____________________________________________________
	/**
	 * @brief Set the lambda function for blending a layer onto base layer.
	 * 
	 * Sets the lambda function for blending the specified layer onto the base layer (the specified canvas surface). 
	 * 
	 * By default the layer is blitted onto the canvas surface using BlendMode::Blend. If a blend function
	 * is specified, that is used instead.
	 * 
	 * 
	 * @param layer		The layer for which we specify the blend function. Please note that layer numbering starts on 1, not 0.
	 * @param func		A lambda function to be used to blend the layer onto the canvas surface.
	 * 
	 * When the function is called, it receives one parameter which is the GfxDevice to use. This has been prepared
	 * for operation in the following way:
	 * 
	 * - RenderTarget is set to the canvas surface (base layer).
	 * - Blit source is set to the layer to be blended.
	 * - ClipList is set to that provided to beginCanvasUpdate().
	 * - BlendMode is set to Blend.
	 * - Tint is cleared.
	 *
	 * No GfxDevice settings are saved or restored. Any settings you change will have to be saved and restored within the lambda.
	 * 
	 */

	//____ setCanvasInitializer() ______________________________________________
	/**
	 * @brief Set a lambda function to initialize the canvas.
	 * 
	 * Sets a lambda function to initialize the canvas surface. This is called from GfxDevice::beginCanvasUpdate() and is
	 * an optional step.
	 * 
	 * @param func		A lambda function to be called when canvas surface is initialized.
	 * 
	 * When the function is called, it receives one parameter which is the GfxDevice to use. This has been prepared
	 * for operation in the following way:
	 * 
	 * - Render target is set to the canvas surface (base layer).
	 * - ClipList is set to that provided to beginCanvasUpdate().
	 * - BlendMode is set to Blend.
	 * - Tint is cleared.
	 *
	 * No GfxDevice settings are saved or restored. Any settings you change will have to be saved and restored within the lambda.
	 * 
	 */

	//____ setCanvasFinalizer() ________________________________________________
	/**
	 * @brief Set a lambda function to finalize the canvas.
	 * 
	 * Sets a lambda function to finalize the canvas surface. This is called from GfxDevice::endCanvasUpdate() and is
	 * an optional step.
	 * 
	 * @param func		A lambda function to be called when canvas surface is finalized.
	 * 
	 * When the function is called, it receives one parameter which is the GfxDevice to use. This has been prepared
	 * for operation in the following way:
	 * 
	 * - Render target is set to the canvas surface (base layer).
	 * - ClipList is set to that provided to beginCanvasUpdate().
	 * - BlendMode is set to Blend.
	 * - Tint is cleared.
	 *
	 * No GfxDevice settings are saved or restored. Any settings you change will have to be saved and restored within the lambda.
	 * 
	 */

	//____ setCanvasModifier() _________________________________________________
	/**
	 * @brief Set a lambda function to modify canvas before layer is blended onto it.
	 * 
	 * Sets a lambda function to modify the canvas surface before the specified layer is blended onto it. This is
	 * an optional step which provides you the oportunity to modify canvas content before next layer is blended onto it.
	 * 
	 * 
	 * @param layer		The layer before which the modifier should be called. Please note that layer numbering starts on 1, not 0.
	 * @param func		A lambda function to be used to modify content of the canvas surface.
	 * 
	 * When the function is called, it receives one parameter which is the GfxDevice to use. This has been prepared
	 * for operation in the following way:
	 * 
	 * - Render target is set to the canvas surface (base layer).
	 * - ClipList is set to that provided to GfxDevice::beginCanvasUpdate().
	 * - BlendMode is set to Blend.
	 * - Tint is cleared.
	 * 
	 * No GfxDevice settings are saved or restored. Any settings you change will have to be saved and restored within the lambda.
	 * 
	 */

}
