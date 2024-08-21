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

#include <wg_glow.h>
#include <wg_gfxdevice.h>
#include <cstring>

namespace wg
{

	//____ constructor ________________________________________________________
	
	Glow::Glow(Widget* pWidget) : Component(pWidget)
	{
	}

	//____ setActive() ____________________________________________________________

	void Glow::setActive(bool bActive)
	{
		if (m_bActive == bActive)
			return;

		if (m_refreshRate != 0)
		{
			if (bActive)
				_notify(ComponentNotif::StartReceiveUpdates, 0, nullptr);
			else
				_notify(ComponentNotif::StopReceiveUpdates, 0, nullptr);
		}

		m_bActive = bActive;
	}

	//____ isActive() ________________________________________________________

	bool Glow::isActive()
	{
		return m_bActive;
	}

	//____ clear() ____________________________________________________________

	void Glow::clear()
	{
		m_bClear = true;
		_requestRender();
	}

	//____ setResizeAction() __________________________________________________

	void Glow::setResizeAction(Placement move, bool bStretch, bool bClear)
	{
		m_resizePlacement = move;
		m_bStretchOnResize = bStretch;
		m_bClearOnResize = bClear;
	}

	//____ setRefreshRate() ___________________________________________________

	void Glow::setRefreshRate(int updatesPerSecond)
	{
		if (updatesPerSecond == m_refreshRate)
			return;

		if (updatesPerSecond < 0)
		{
			//TODO: Error handling!

			return;
		}

		if (m_bActive)
		{
			if (m_refreshRate == 0 && updatesPerSecond > 0)
				_notify(ComponentNotif::StartReceiveUpdates, 0, nullptr);

			if (m_refreshRate > 0 && updatesPerSecond == 0)
				_notify(ComponentNotif::StopReceiveUpdates, 0, nullptr);
		}

		m_refreshRate = updatesPerSecond;
	}

	//____ refreshRate() ______________________________________________________

	int Glow::refreshRate() const
	{
		return m_refreshRate;
	}

	//____ setBlurbrush() ______________________________________________________

	void Glow::setBlurbrush(Blurbrush * pBrush)
	{
		m_pBrush = pBrush;
	}


	//____ setPixelResolution() _______________________________________________

	void Glow::setPixelResolution(SizeI pixelSize)
	{
		if (pixelSize != m_resolution)
		{
			SizeI usedSize = _glowResolution();
			m_resolution = pixelSize;

			if (m_surface[0] && m_surface[0]->pixelSize() != pixelSize)
			{
				m_surface[0] = nullptr;
				m_surface[1] = nullptr;
				_requestRender();
			}
		}
	}

	//____ pixelResolution() __________________________________________________

	SizeI Glow::pixelResolution() const
	{
		return m_resolution;
	}

	//____ setSeedStates() ________________________________________________

	void Glow::setSeedStates(HiColor tint, BlendMode blendMode)
	{
		m_seedTint = tint;
		m_seedBlend = blendMode;
	}

	//____ setRenderStates() ______________________________________________

	void Glow::setRenderStates(HiColor tint, BlendMode blendMode)
	{
		m_glowTint = tint;
		m_glowBlend = blendMode;
	}


	//____ _initFromBlueprint() ________________________________________________

	void Glow::_initFromBlueprint(const Blueprint& bp)
	{
		m_refreshRate	= bp.refreshRate;
		m_pBrush		= bp.blurbrush;

		m_resolution	= bp.resolution;

		m_seedTint		= bp.seedTint;
		m_seedBlend		= bp.seedBlend;

		m_glowTint		= bp.glowTint;
		m_glowBlend		= bp.glowBlend;

		m_resizePlacement	= bp.resizePlacement;
		m_bStretchOnResize	= bp.stretchOnResize;
		m_bClearOnResize	= bp.clearOnResize;

		if (bp.active)
			setActive(true);
	}

	//____ _update() __________________________________________________________

	void Glow::_update(int microPassed)
	{
		if (!m_bActive || m_refreshRate == 0)
			return;
		
		int microsecBetweenUpdates = 1000000 / m_refreshRate;

		m_microSecAccumulator += microPassed;

		if (m_microSecAccumulator >= microsecBetweenUpdates)
			_requestRender();

	}


	//____ _render() __________________________________________________________

	void Glow::_render(GfxDevice* pDevice, const RectSPX& canvas, Surface_p pSeed, const RectSPX& seedArea)
	{
		if (!m_bActive)
			return;


		// Possibly regenerate the glow surfaces

		if (m_surface[1] == nullptr)
		{
			m_surface[1] = _createGlowCanvas(pDevice->surfaceFactory());
			pDevice->beginCanvasUpdate(m_surface[1]);
			pDevice->fill(HiColor::Black);
			pDevice->endCanvasUpdate();

			if (m_surface[0] == nullptr)
			{
				// Both are missing, we just recreate and clear them.

				m_surface[0] = _createGlowCanvas(pDevice->surfaceFactory());
				m_bClear = true;
			}
			else
			{
				pDevice->beginCanvasUpdate(m_surface[1]);
				pDevice->fill(Color::Black);

				pDevice->setBlitSource(m_surface[0]);
				SizeSPX destSize = (m_surface[1]->pixelSize() * 64) - SizeSPX(128, 128);
				SizeSPX sourceSize = (m_surface[0]->pixelSize() * 64) - SizeSPX(128, 128);

				if (m_bStretchOnResize)
				{
					pDevice->stretchBlit({ 64,64,destSize }, { 64,64,sourceSize });
				}
				else
				{
					RectSPX dest = Util::placementToRect(m_resizePlacement, destSize, sourceSize);

					pDevice->blit({ dest.x + 64,dest.y + 64 }, { 64,64,sourceSize });
				}

				pDevice->endCanvasUpdate();

				m_surface[0] = _createGlowCanvas(pDevice->surfaceFactory());
				pDevice->beginCanvasUpdate(m_surface[0]);
				pDevice->fill(HiColor::Black);
				pDevice->endCanvasUpdate();

				std::swap(m_surface[0], m_surface[1]);
			}
		}

		// Possibly clear glow canvas

		if (m_bClear)
		{
			pDevice->beginCanvasUpdate(m_surface[0]);
			pDevice->fill(HiColor::Black);
			pDevice->endCanvasUpdate();

			m_bClear = false;
		}

		// Update glow canvases

		int microsecBetweenUpdates = 1000000 / m_refreshRate;

		RectSPX srcRect = m_surface[0]->pixelSize() * 64;
		srcRect -= BorderSPX(64);

		RectSPX destRect = m_surface[1]->pixelSize() * 64;
		destRect -= BorderSPX(64);

		float scaleX = destRect.w / (float) canvas.w;
		float scaleY = destRect.h / (float)canvas.h;


		RectSPX scaledSeedRect = RectSPX( destRect.x + (seedArea.x - canvas.x) * scaleX,
										  destRect.y + (seedArea.y - canvas.y) * scaleY,
											seedArea.w * scaleX,
											seedArea.h * scaleY );

		while (m_microSecAccumulator >= microsecBetweenUpdates)
		{
			pDevice->beginCanvasUpdate(m_surface[1]);
			pDevice->setBlitSource(m_surface[0]);
			pDevice->setBlurbrush(m_pBrush);
			pDevice->blur(destRect.pos(), srcRect);
			pDevice->setTintColor(m_seedTint);
			pDevice->setBlendMode(m_seedBlend);
			pDevice->setBlitSource(pSeed);
			pDevice->stretchBlit(scaledSeedRect);
			pDevice->endCanvasUpdate();

			std::swap(m_surface[0], m_surface[1]);

			m_microSecAccumulator -= microsecBetweenUpdates;

			if (m_microSecAccumulator > microsecBetweenUpdates)
				m_microSecAccumulator = microsecBetweenUpdates;		// Make sure we don't loop more than twice.
		}

		// Blit to destination, first updated blur, then local canvas.

		BlendMode bm = pDevice->blendMode();
		HiColor c = pDevice->tintColor();

		pDevice->setBlendMode(m_glowBlend);
		pDevice->setTintColor(m_glowTint);
		pDevice->setBlitSource(m_surface[0]);
		pDevice->stretchBlit(canvas, destRect);

		pDevice->setTintColor(c);
		pDevice->setBlendMode(bm);
	}

	//____ _setSize() _________________________________________________________

	void Glow::_setSize(SizeSPX size, int scale)
	{
		if (m_resolution.isEmpty())
		{
			m_surface[1] = nullptr;

			if (m_bClearOnResize)
				m_surface[0] = nullptr;
		}
	}

	//____ _glowResolution() __________________________________________________

	SizeI Glow::_glowResolution()
	{
		if (!m_resolution.isEmpty())
			return m_resolution;

		return _size() / 64;
	}

	//____ _createGlowCanvas() ________________________________________________

	Surface_p Glow::_createGlowCanvas( SurfaceFactory * pFactory )
	{
		// Glow surfaces have a border that is not used to make sampling at corners
		// work better.

		SizeI pixelSize = _glowResolution() + SizeI(2, 2);

		return pFactory->createSurface(WGBP(Surface, _.size = pixelSize, _.format = PixelFormat::BGRX_8, _.canvas = true));
	}

};
