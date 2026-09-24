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

			if (tintColor != HiColor::Undefined && tintColor != HiColor::White)
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

	//____ RenderSettingsWithTint _____________________________________________________
	/*
	* Sets layer, blend mode, tint color and Tint of device for the scope, if needed, and restores
	* them when destroyed.
	*/

	class RenderSettingsWithTint
	{
	public:
		RenderSettingsWithTint(GfxDevice* pDevice, int layer, BlendMode blendMode, HiColor tintColor, const RectSPX& rect, Tint * pTint )
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

			if (pTint)
			{
				m_pPrevTint = pDevice->tint();
				m_prevTintRect = pDevice->tintRect();
				pDevice->setTint(rect, pTint);
				m_bTint = true;
			}

			if (tintColor != HiColor::Undefined && tintColor != HiColor::White )
			{
				m_prevTintColor = pDevice->tintColor();
				if (tintColor != m_prevTintColor)
				{
					pDevice->setTintColor(tintColor);
					m_bTintChanged = true;
				}
			}
		}

		~RenderSettingsWithTint()
		{
			if (m_prevLayer != -1)
				m_pDevice->setRenderLayer(m_prevLayer);
			if (m_prevBlendMode != BlendMode::Undefined)
				m_pDevice->setBlendMode(m_prevBlendMode);
			if (m_bTintChanged)
				m_pDevice->setTintColor(m_prevTintColor);
			if (m_bTint)
			{
				if (m_pPrevTint)
					m_pDevice->setTint(m_prevTintRect, m_pPrevTint);
				else
					m_pDevice->clearTint();
			}
		}

		GfxDevice* m_pDevice;
		int			m_prevLayer = -1;
		BlendMode	m_prevBlendMode = BlendMode::Undefined;
		HiColor		m_prevTintColor;
		bool		m_bTintChanged = false;
		bool		m_bTint = false;
		Tint_p		m_pPrevTint;
		RectSPX		m_prevTintRect;
	};

}
#endif //WG_SKINIMPL_DOT_H