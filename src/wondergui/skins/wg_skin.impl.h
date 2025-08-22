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
#ifndef WG_SKINIMPL_DOT_H
#define WG_SKINIMPL_DOT_H
#pragma once

#include <wg_gfxdevice.h>

namespace wg
{

	//____ RenderSettings _____________________________________________________
	/*
	* Simple class for quickly and easily set layer, blend mode and tint color for
	* rendering and then revert back automatically when deleted.
	*/

	class RenderSettings
	{
	public:
		RenderSettings(GfxDevice* pDevice, int layer, BlendMode blendMode = BlendMode::Undefined, HiColor tintColor = HiColor::White)
		{
			m_pDevice = pDevice;

			if (layer != -1 && pDevice->renderLayer() != layer)
			{
				m_prevLayer = pDevice->renderLayer();
				pDevice->setRenderLayer(layer);
			}

			if (blendMode != BlendMode::Undefined && blendMode != pDevice->blendMode())
			{
				m_prevBlendMode = pDevice->blendMode();
				pDevice->setBlendMode(blendMode);
			}

			if (tintColor != HiColor::White)
			{
				m_prevTintColor = pDevice->tintColor();
				pDevice->setTintColor(tintColor);
				m_bTintChanged = true;
			}

		}

		~RenderSettings()
		{
			if (m_prevLayer != -1)
				m_pDevice->setRenderLayer(m_prevLayer);
			if (m_prevBlendMode != BlendMode::Undefined)
				m_pDevice->setBlendMode(m_prevBlendMode);
			if (m_bTintChanged)
				m_pDevice->setTintColor(m_prevTintColor);
		}

		GfxDevice* m_pDevice;
		int			m_prevLayer = -1;
		BlendMode	m_prevBlendMode = BlendMode::Undefined;
		HiColor		m_prevTintColor;
		bool		m_bTintChanged = false;
	};

	//____ RenderSettingsWithTintmap _____________________________________________________
	/*
	* Simple class for quickly and easily set layer, blend mode and tint color/map for
	* rendering and then revert back automatically when deleted.
	*/

	class RenderSettingsWithTintmap
	{
	public:
		RenderSettingsWithTintmap(GfxDevice* pDevice, int layer, BlendMode blendMode, HiColor tintColor, const RectSPX& rect, Tintmap * pTintmap )
		{
			m_pDevice = pDevice;

			if (layer != -1 && pDevice->renderLayer() != layer)
			{
				m_prevLayer = pDevice->renderLayer();
				pDevice->setRenderLayer(layer);
			}

			if (blendMode != BlendMode::Undefined)
			{
				m_prevBlendMode = pDevice->blendMode();
				pDevice->setBlendMode(blendMode);
			}

			if (pTintmap)
			{
				pDevice->setTintmap(rect, pTintmap);
				m_bTintmap = true;
			}

			if (tintColor != HiColor::Undefined)
			{
				m_prevTintColor = pDevice->tintColor();
				if (tintColor != m_prevTintColor)
				{
					pDevice->setTintColor(tintColor);
					m_bTintChanged = true;
				}
			}

		}

		~RenderSettingsWithTintmap()
		{
			if (m_prevLayer != -1)
				m_pDevice->setRenderLayer(m_prevLayer);
			if (m_prevBlendMode != BlendMode::Undefined)
				m_pDevice->setBlendMode(m_prevBlendMode);
			if (m_bTintChanged)
				m_pDevice->setTintColor(m_prevTintColor);
			if (m_bTintmap)
				m_pDevice->clearTintmap();
		}

		GfxDevice* m_pDevice;
		int			m_prevLayer = -1;
		BlendMode	m_prevBlendMode = BlendMode::Undefined;
		HiColor		m_prevTintColor;
		bool		m_bTintChanged = false;
		bool		m_bTintmap = false;
	};



	//____ RenderSettingsWithGradient _____________________________________________________
	/*
	* Simple class for quickly and easily set layer, blend mode and tint color/gradient for
	* rendering and then revert back automatically when deleted.
	*/

	class RenderSettingsWithGradient
	{
	public:
		RenderSettingsWithGradient(GfxDevice* pDevice, int layer, BlendMode blendMode, HiColor tintColor, const RectSPX& rect, const Gradient& tintGradient )
		{
			m_pDevice = pDevice;

			if (layer != -1 && pDevice->renderLayer() != layer)
			{
				m_prevLayer = pDevice->renderLayer();
				pDevice->setRenderLayer(layer);
			}

			if (blendMode != BlendMode::Undefined)
			{
				m_prevBlendMode = pDevice->blendMode();
				pDevice->setBlendMode(blendMode);
			}

			if (!tintGradient.isUndefined())
			{
				pDevice->setTintGradient(rect, tintGradient);
				m_bGradient = true;
			}
			
			if (tintColor != HiColor::Undefined)
			{
				m_prevTintColor = pDevice->tintColor();
				if (tintColor != m_prevTintColor)
				{
					pDevice->setTintColor(tintColor);
					m_bTintChanged = true;
				}
			}

		}

		~RenderSettingsWithGradient()
		{
			if (m_prevLayer != -1)
				m_pDevice->setRenderLayer(m_prevLayer);
			if (m_prevBlendMode != BlendMode::Undefined)
				m_pDevice->setBlendMode(m_prevBlendMode);
			if (m_bTintChanged)
				m_pDevice->setTintColor(m_prevTintColor);
			if (m_bGradient)
				m_pDevice->clearTintGradient();
		}

		GfxDevice* m_pDevice;
		int			m_prevLayer = -1;
		BlendMode	m_prevBlendMode = BlendMode::Undefined;
		HiColor		m_prevTintColor;
		bool		m_bTintChanged = false;
		bool		m_bGradient = false;
	};


}
#endif //WG_SKINIMPL_DOT_H
