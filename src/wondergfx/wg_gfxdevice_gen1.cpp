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

#include <algorithm>
#include <cmath>
#include <cstring>

#include <wg_gfxdevice_gen1.h>
#include <wg_geo.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

using namespace std;
using namespace wg::Util;

namespace wg
{

	const TypeInfo GfxDeviceGen1::TYPEINFO = { "GfxDeviceGen1", &Object::TYPEINFO };

	// Transforms for flipping movement over SOURCE when blitting

	const int GfxDeviceGen1::s_blitFlipTransforms[GfxFlip_size][2][2] = { { 1,0,0,1 },			// Normal
																{ -1,0,0,1 },			// FlipX
																{ 1,0,0,-1 },			// FlipY
																{ 0,-1,1,0 },			// Rot90
																{ 0,1,1,0 },			// Rot90FlipX
																{ 0,-1,-1,0 },			// Rot90FlipY
																{ -1,0,0,-1 },			// Rot180
																{ 1,0,0,-1 },			// Rot180FlipX
																{ -1,0,0,1 },			// Rot180FlipY
																{ 0,1,-1,0 },			// Rot270
																{ 0,-1,-1,0 },			// Rot270FlipX
																{ 0,1,1,0 } };			// Rot270FlipY

	const int GfxDeviceGen1::s_blitFlipOffsets[GfxFlip_size][2] = {	{ 0,0 },				// Normal
															{ 1,0 },				// FlipX
															{ 0,1 },				// FlipY
															{ 0,1 },				// Rot90
															{ 0,0 },				// Rot90FlipX
															{ 1,1 },				// Rot90FlipY
															{ 1,1 },				// Rot180
															{ 0,1 },				// Rot180FlipX
															{ 1,0 },				// Rot180FlipY
															{ 1,0 },				// Rot270
															{ 1,1 },				// Rot270FlipX
															{ 0,0 } };				// Rot270FlipY

	// Transforms for flipping movement over CANVAS when drawing
/*
	static const int drawFlipTransforms[GfxFlip_size][2][2] = { { 1,0,0,1 },		// Normal
															{ -1,0,0,1 },			// FlipX
															{ 1,0,0,-1 },			// FlipY
															{ 0,1,-1,0 },			// Rot90
															{ 0,1,1,0 },			// Rot90FlipX
															{ 0,-1,-1,0 },			// Rot90FlipY
															{ -1,0,0,-1 },			// Rot180
															{ 1,0,0,-1 },			// Rot180FlipX
															{ -1,0,0,1 },			// Rot180FlipY
															{ 0,-1,1,0 },			// Rot270
															{ 0,-1,-1,0 },			// Rot270FlipX
															{ 0,1,1,0 } };			// Rot270FlipY
*/






	//____ constructor _____________________________________________________________

	GfxDeviceGen1::GfxDeviceGen1()
	{
	}

	//____ Destructor _________________________________________________________

	GfxDeviceGen1::~GfxDeviceGen1()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GfxDeviceGen1::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setClipList() __________________________________________________________

	bool GfxDeviceGen1::setClipList(int nRectangles, const RectSPX* pRectangles)
	{
		RectSPX bounds;
		if( nRectangles == 0 )
			bounds = { m_clipBounds.x, m_clipBounds.y, 0, 0 };
		else
		{
			bounds = *pRectangles;
			for (int i = 1; i < nRectangles; i++)
				bounds.growToContain(pRectangles[i]);

			
			if (bounds.x < 0 || bounds.y < 0 || bounds.w > m_canvas.size.w || bounds.h > m_canvas.size.h)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "ClipList contains rectangels outside canvas. ClipList not set.",
									this, &TYPEINFO, __func__, __FILE__, __LINE__ );
				return false;
			}
		}

		m_pClipRects = pRectangles;
		m_nClipRects = nRectangles;
		m_clipBounds = bounds;
		_clipListWasChanged();
		return true;
	}

	//____ resetClipList() __________________________________________________________

	void GfxDeviceGen1::resetClipList()
	{
		m_clipBounds = m_canvasUpdateBounds;
		m_pClipRects = m_pCanvasUpdateRects;
		m_nClipRects = m_nCanvasUpdateRects;
		_clipListWasChanged();
	}

	//____ pushClipList() _____________________________________________________

	bool GfxDeviceGen1::pushClipList(int nRectangles, const RectSPX* pRectangles)
	{
		m_clipListStack.push_back({ m_nClipRects,m_pClipRects,m_clipBounds });

		RectSPX bounds;
		if( nRectangles == 0 )
			bounds = { m_clipBounds.x, m_clipBounds.y, 0, 0 };
		else
		{
			bounds = *pRectangles;
			for (int i = 1; i < nRectangles; i++)
				bounds.growToContain(pRectangles[i]);

			//TODO: This is incomplete, we should only allow rectangles that are a subset of those used in beginCanvasUpdate.
			
			if (bounds.x < 0 || bounds.y < 0 || bounds.w > m_canvas.size.w || bounds.h > m_canvas.size.h)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "ClipList contains rectangels outside canvas. ClipList not set.", 
									this, &TYPEINFO, __func__, __FILE__, __LINE__ );
				return false;
			}
		}

		m_pClipRects = pRectangles;
		m_nClipRects = nRectangles;
		m_clipBounds = bounds;
		_clipListWasChanged();
		return true;
	}

	//____ popClipList() ______________________________________________________

	bool GfxDeviceGen1::popClipList()
	{
		if (m_clipListStack.empty())
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "No ClipLists in stack, nothing to pop.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		auto& clipList = m_clipListStack.back();

		m_clipBounds = clipList.clipBounds;
		m_pClipRects = clipList.pClipRects;
		m_nClipRects = clipList.nClipRects;

		m_clipListStack.pop_back();

		_clipListWasChanged();
		return true;
	}

	//____ setTintmap() ___________________________________________________________

	void GfxDeviceGen1::setTintmap(const RectSPX& rect, Tintmap* pTintmap)
	{
		// Do nothing. Not supported in Gen1
	}

	//____ clearTintmap() _________________________________________________________

	void GfxDeviceGen1::clearTintmap()
	{
		// Do nothing. Not supported in Gen1
	}

	//____ tintmap() ___________________________________________________________

	Tintmap_p	GfxDeviceGen1::tintmap() const
	{
		//Tintmaps not supportd in Gen1

		return nullptr;
	}

	//____ tintmapRect() _____________________________________________________

	RectSPX		GfxDeviceGen1::tintmapRect() const
	{
		//Tintmaps not supportd in Gen1

		return RectSPX();
	}

	//____ setTintColor() __________________________________________________________

	void GfxDeviceGen1::setTintColor( HiColor color )
	{
		m_tintColor = color;
	}

	//____ setTintGradient() __________________________________________________

	void GfxDeviceGen1::setTintGradient(const RectSPX& rect, const Gradient& gradient)
	{
		m_tintGradientRect = rect;
		m_tintGradient = gradient;
		m_bTintGradient = true;
	}

	//____ clearTintGradient() ________________________________________________

	void GfxDeviceGen1::clearTintGradient()
	{
		m_bTintGradient = false;
	}

	//____ setBlendMode() __________________________________________________________

	bool GfxDeviceGen1::setBlendMode( BlendMode blendMode )
	{
		if (blendMode == BlendMode::Undefined)
			m_blendMode = BlendMode::Blend;
		else
			m_blendMode = blendMode;

		return true;
	}

	//____ setBlitSource() ____________________________________________________

	bool GfxDeviceGen1::setBlitSource(Surface * pSource)
	{
		m_pBlitSource = pSource;
		return true;
	}

	//____ setMorphFactor() ____________________________________________________

	void GfxDeviceGen1::setMorphFactor(float factor)
	{
		limit(factor, 0.f, 1.f);
		m_morphFactor = factor;
	}

	//____ setBlurbrush() _____________________________________________________

	void GfxDeviceGen1::setBlurbrush(Blurbrush* pBrush)
	{
		if (pBrush != m_pBlurbrush)
			m_pBlurbrush = pBrush;
	}

	//____ blurbrush() _________________________________________________________

	Blurbrush_p	GfxDeviceGen1::blurbrush() const
	{
		return m_pBlurbrush;
	}

	//____ setFixedBlendColor() __________________________________________________

	void GfxDeviceGen1::setFixedBlendColor( HiColor color )
	{
		m_fixedBlendColor = color;
	}

	//____ setRenderLayer() ___________________________________________________

	void GfxDeviceGen1::setRenderLayer(int layer)
	{
		if (layer == m_renderLayer)
			return;

		if (!m_pCanvasLayers || layer < 0 || layer > m_pCanvasLayers->size())
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Specified layer out of bounds.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		m_renderLayer = layer;
		_renderLayerWasChanged();

		// Init layer if we are rendering.

	}

	//____ _clearRenderLayer() _____________________________________________________

	void GfxDeviceGen1::_clearRenderLayer()
	{
		if (m_renderLayer > 0 )
		{
			//TODO: Save tint gradient.

			const RectSPX* pSavedClipRects = m_pClipRects;
			int				nSavedClipRects = m_nClipRects;
			RectSPX			savedClipBounds = m_clipBounds;

			m_pClipRects = m_pCanvasUpdateRects;
			m_nClipRects = m_nCanvasUpdateRects;
			m_clipBounds = m_canvasUpdateBounds;

			_clipListWasChanged();

			BlendMode	savedBlendMode = m_blendMode;
			HiColor		savedTintColor = m_tintColor;

            setTintColor(Color::White);
            clearTintGradient();

            if( m_pCanvasLayers->_layerClearFunc(m_renderLayer) != nullptr )
            {
                //NOTE! BlendMode can be anything when calling initializer.

                m_pCanvasLayers->_layerClearFunc(m_renderLayer)(this);
            }
            else
            {
                setBlendMode(BlendMode::Replace);
                fill(Color::Transparent);
            }


            setBlendMode(savedBlendMode);
			setTintColor(savedTintColor);

			m_pClipRects = pSavedClipRects;
			m_nClipRects = nSavedClipRects;
			m_clipBounds = savedClipBounds;

			_clipListWasChanged();
		}
	}


	//____ beginRender() ___________________________________________________________

	bool GfxDeviceGen1::beginRender()
	{
		if( m_bRendering )
			return false;

		m_bRendering = true;
		return true;	// Assumed to be ok if device doesn't have its own method.
	}

	//____ endRender() ________________________________________________________

	bool GfxDeviceGen1::endRender()
	{
		if( !m_bRendering )
			return false;

		m_bRendering = false;
		return true;	// Assumed to be ok if device doesn't have its own method.
	}

	//____ isRendering() _______________________________________________________
	/**
	 * @brief Check if GfxDeviceGen1 is in rendering state.
	 *
	 * Check if GfxDeviceGen1 is in rendering state, e.g. beginRender() has been called
	 * but not yet followed by endRender().
	 *
	 * Please note that GfxDeviceGen1 might remain busy for an undefined time drawing
	 * graphics after endRender() has been called if rendering is done
	 * asynchronously. To determine if all graphics has been rendered to the
	 * framebuffer you should call isIdle() instead.
	 *
	 * @return True if between beginRender()/endRender() calls.
	 */

	bool GfxDeviceGen1::isRendering()
	{
		return m_bRendering;
	};

	//____ isIdle() ____________________________________________________________
	/**
	 * @brief Check if all rendering has completed.
	 *
	 * Check if GfxDeviceGen1 has fully completed rendering all issued graphics commands
	 * and the canvas content is up-to-date.
	 *
	 * This method will start returning false once beginRender() has been called
	 * and return true again some time after endRender() has been called.
	 *
	 * @return True if all rendering has completed.
	 */

	bool GfxDeviceGen1::isIdle()
	{
		return !m_bRendering;
	}

	//____ flush() ____________________________________________________________
	/**
	 * @brief Flush internal rendering queue.
	 *
	 * Most GfxDeviceGen1s, except for SoftGfxDeviceGen1, queues rendering calls
	 * in order to optimize for speed by sending them in bulk to the underlying
	 * rendering library.
	 *
	 * A call to flush() forces the GfxDeviceGen1 to immediately send all queued rendering
	 * calls.
	 *
	 * Calling flush only makes sense between beginRender() and endRender() since endRender()
	 * flushes for you.
	 */

	void GfxDeviceGen1::flush()
	{
	}


	//____ _beginCanvasUpdate() ________________________________________________

	bool GfxDeviceGen1::_beginCanvasUpdate(CanvasRef ref, Surface * pSurface, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pCanvasLayers, int startLayer )
	{

		SizeSPX sz;
		PixelFormat format;

		if( ref != CanvasRef::None )
		{
			sz = canvas(ref).size;
			format = canvas(ref).format;
		}
		else if( pSurface )
		{
			sz = pSurface->pixelSize()*64;
			format = pSurface->pixelFormat();
		}
		else
		{
			//TODO: Error handling!
			return false;
		}

		if( sz.isEmpty() )
		{
			//TODO: Error handling!
			return false;
		}

		RectSPX bounds;

		if (nUpdateRects > 0)
		{
			bounds = *pUpdateRects;
			for (int i = 1; i < nUpdateRects; i++)
				bounds.growToContain(pUpdateRects[i]);

			if (bounds.x < 0 || bounds.y < 0 || bounds.w > sz.w || bounds.h > sz.h)
			{
				//TODO: Error handling!

				return false;
			}
		}
		else
		{
			bounds = sz;
			nUpdateRects = 1;
			pUpdateRects = &m_canvasUpdateBounds;
		}

		// Push old values onto stack, if we have a canvas.

		m_canvasStack.emplace_back();
		auto& back = m_canvasStack.back();

		back.pCanvasLayers = m_pCanvasLayers;
		back.canvas = m_canvas;
		back.updateRects.pClipRects = m_pCanvasUpdateRects;
		back.updateRects.nClipRects = m_nCanvasUpdateRects;
		back.updateRects.clipBounds = m_canvasUpdateBounds;
		back.clipRects.pClipRects = m_pClipRects;
		back.clipRects.nClipRects = m_nClipRects;
		back.clipRects.clipBounds = m_clipBounds;
		back.renderLayer = m_renderLayer;
		back.tintColor = m_tintColor;
		back.tintGradient = m_tintGradient;
		back.tintGradientRect = m_tintGradientRect;
		back.bTintGradient = m_bTintGradient;
		back.blendMode = m_blendMode;
		back.morphFactor = m_morphFactor;
		back.fixedBlendColor = m_fixedBlendColor;

		memcpy(back.layerSurfaces, m_layerSurfaces, sizeof(Surface_p) * CanvasLayers::c_maxLayers);
		memset(m_layerSurfaces, 0, sizeof(Surface_p) * CanvasLayers::c_maxLayers);

		// Set values

		m_canvas.ref = ref;
		m_canvas.pSurface = pSurface;
		m_canvas.size = sz;
		m_canvas.format = format;

		m_pCanvasLayers = pCanvasLayers;

		m_pCanvasUpdateRects = pUpdateRects;
		m_nCanvasUpdateRects = nUpdateRects;
		m_canvasUpdateBounds = bounds;

		m_pClipRects = pUpdateRects;
		m_nClipRects = nUpdateRects;
		m_clipBounds = bounds;

		int layer = 0;
		if (pCanvasLayers)
			layer = (startLayer > 0 && startLayer <= pCanvasLayers->size()) ? startLayer : pCanvasLayers->baseLayer();

        m_layerSurfaces[0] = pSurface;

		m_renderLayer = layer;
		m_tintColor = HiColor::White;
		m_tintGradient.clear();
		m_tintGradientRect = sz;
		m_bTintGradient = false;
		m_blendMode = BlendMode::Blend;
		m_morphFactor = 0.5f;
		m_fixedBlendColor = HiColor::Black;
		_canvasWasChanged();

        // Call Canvas Initializer

		if( !m_bIsProxyDevice )
		{
			if( pCanvasLayers && pCanvasLayers->_canvasClearFunc() != nullptr )
			{
				setRenderLayer(0);
				pCanvasLayers->_canvasClearFunc()(this);
				setRenderLayer(layer);
			}
		}

		return true;
	}

	//____ endCanvasUpdate() __________________________________________________

	void GfxDeviceGen1::endCanvasUpdate()
	{
/*		// Sanity checks

        // It is OK for canvas stack to be empty, we need another way to sync
        // begin/endCanvasUpdate() calls.

		if (m_canvasStack.empty())
		{
			Base::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "No Canvas being updated, nothing to end.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}
*/
		// Blend together the layers

		if (!m_bIsProxyDevice && m_pCanvasLayers)
		{
			bool bFirst = true;

			int nbLayers = m_pCanvasLayers->size();
			for (int layerIdx = 1; layerIdx <= nbLayers; layerIdx++)
			{
                // Call pre-blend canvas modifier no matter if layer has a surface or not

                if( m_pCanvasLayers->_canvasModifier(layerIdx) != nullptr )
                {
                    if (bFirst)
                    {
                        setClipList(m_nCanvasUpdateRects, m_pCanvasUpdateRects);
                        setBlendMode(BlendMode::Blend);
                        setTintColor(Color::White);
                        clearTintGradient();
                        setRenderLayer(0);
                        bFirst = false;
                    }

                    m_pCanvasLayers->_canvasModifier(layerIdx)(this);
                }

                // Finalize and blend layer if it has a surface.

                if (m_layerSurfaces[layerIdx])
				{
					if (bFirst)
					{
						setClipList(m_nCanvasUpdateRects, m_pCanvasUpdateRects);
                        setBlendMode(BlendMode::Blend);
						setTintColor(Color::White);
						clearTintGradient();
						setRenderLayer(0);
						bFirst = false;
					}

                    // Call layer finalizer before blending

                    if( m_pCanvasLayers->_layerPreBlendFunc(layerIdx) != nullptr )
                    {
                        setRenderLayer(layerIdx);
                        m_pCanvasLayers->_layerPreBlendFunc(layerIdx)(this);
                        setRenderLayer(0);
                    }

                    // Blend layer onto base layer

                    setBlitSource(m_layerSurfaces[layerIdx]);

                    if( m_pCanvasLayers->_layerBlendFunc(layerIdx) != nullptr )
                        m_pCanvasLayers->_layerBlendFunc(layerIdx)(this);
                    else
                        blit({ 0,0 });
				}
			}

            // Call Canvas Finalizer

            if( m_pCanvasLayers->_canvasFinalizeFunc() != nullptr )
                m_pCanvasLayers->_canvasFinalizeFunc()(this);
        }

		// Dereference surfaces

		for (int i = 0; i < CanvasLayers::c_maxLayers; i++)
			m_layerSurfaces[i] = nullptr;

		// Unwind the stack, retrieve saved values

		if( !m_canvasStack.empty() )
		{
			auto& back = m_canvasStack.back();

			m_pCanvasLayers = back.pCanvasLayers;
			m_canvas = back.canvas;
			m_pCanvasUpdateRects = back.updateRects.pClipRects;
			m_nCanvasUpdateRects = back.updateRects.nClipRects;
			m_canvasUpdateBounds = back.updateRects.clipBounds;
			m_pClipRects = back.clipRects.pClipRects;
			m_nClipRects = back.clipRects.nClipRects;
			m_clipBounds = back.clipRects.clipBounds;
			m_renderLayer = back.renderLayer;
			m_tintColor = back.tintColor;
			m_tintGradient = back.tintGradient;
			m_tintGradientRect = back.tintGradientRect;
			m_bTintGradient = back.bTintGradient;
			m_blendMode = back.blendMode;
			m_morphFactor = back.morphFactor;
			m_fixedBlendColor = back.fixedBlendColor;


			// Move surface pointers from stack without referencing/dereferencing.

			memcpy(m_layerSurfaces, back.layerSurfaces, sizeof(Surface_p) * CanvasLayers::c_maxLayers);
			memset(back.layerSurfaces, 0, sizeof(Surface_p) * CanvasLayers::c_maxLayers);

			m_canvasStack.pop_back();
		}
		else
		{
			m_canvas.ref = CanvasRef::None;
			m_canvas.pSurface = nullptr;
			m_canvas.size.clear();

			m_pCanvasLayers = nullptr;
			m_pCanvasUpdateRects = nullptr;
			m_nCanvasUpdateRects = 0;
			m_canvasUpdateBounds.clear();
			m_pClipRects = nullptr;
			m_nClipRects = 0;
			m_clipBounds.clear();
			m_renderLayer = 0;
		}

		_canvasWasChanged();
	}


	//____ fill() ______________________________________________________
	/**
	 * @brief Fill the canvas with specified color.
	 *
	 * Fills the entire canvas (or whatever areas are allowed by the
	 * clip list) with the specified color.
	 *
	 * @return Nothing.
	 */

	void GfxDeviceGen1::fill(HiColor col)
	{
		fill(m_canvas.size, col);
	}

	//____ drawLine() __________________________________________________

	void GfxDeviceGen1::drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness)
	{
		CoordSPX end;

		switch (dir)
		{
		case Direction::Up:
			end.x = begin.x;
			end.y = begin.y - length;
			break;
		case Direction::Down:
			end.x = begin.x;
			end.y = begin.y + length;
			break;
		case Direction::Left:
			end.x = begin.x - length;
			end.y = begin.y;
			break;
		case Direction::Right:
			end.x = begin.x + length;
			end.y = begin.y;
			break;
		}

		drawLine(begin, end, color, thickness);
	}


	//____ blit() __________________________________________________________________

	void GfxDeviceGen1::blit(CoordSPX dest)
	{
		assert(m_pBlitSource != nullptr);

		_transformBlitSimple({ dest, m_pBlitSource->pixelSize()*64 }, { 0,0 }, s_blitFlipTransforms[0], OpType::Blit);
	}

	void GfxDeviceGen1::blit(CoordSPX dest, const RectSPX& src)
	{
		assert(m_pBlitSource != nullptr);

		_transformBlitSimple({ dest, src.size() }, src.pos(), s_blitFlipTransforms[0], OpType::Blit);
	}

	//____ flipBlit() _________________________________________________________

	void GfxDeviceGen1::flipBlit(CoordSPX dest, GfxFlip flip)
	{
		assert(m_pBlitSource != nullptr);

		SizeSPX srcSize  = m_pBlitSource->pixelSize()*64;

		spx ofsX = srcSize.w * s_blitFlipOffsets[(int)flip][0];
		spx ofsY = srcSize.h * s_blitFlipOffsets[(int)flip][1];

		SizeSPX dstSize = srcSize;
		if (s_blitFlipTransforms[(int)flip][0][0] == 0)
			swap(dstSize.w, dstSize.h);

		_transformBlitSimple({ dest, dstSize }, { ofsX, ofsY }, s_blitFlipTransforms[(int)flip], OpType::Blit);
	}

	void GfxDeviceGen1::flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip)
	{
		assert(m_pBlitSource != nullptr);

		spx ofsX = src.x + src.w * s_blitFlipOffsets[(int)flip][0];
		spx ofsY = src.y + src.h * s_blitFlipOffsets[(int)flip][1];

		SizeSPX dstSize = src.size();
		if (s_blitFlipTransforms[(int)flip][0][0] == 0)
			swap(dstSize.w, dstSize.h);


        _transformBlitSimple({ dest, dstSize }, {ofsX, ofsY}, s_blitFlipTransforms[(int)flip], OpType::Blit);
	}



	//____ stretchBlit() ___________________________________________________________

	void GfxDeviceGen1::stretchBlit(const RectSPX& dest )
	{
		assert(m_pBlitSource != nullptr);

		stretchBlit(dest, RectSPX(0, 0, m_pBlitSource->pixelSize()*64) );
	}

	void GfxDeviceGen1::stretchBlit(const RectSPX& dest, const RectSPX& src )
	{
		assert(m_pBlitSource != nullptr);

		if (m_pBlitSource == nullptr || dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
			return;

		if (dest.w == src.w && dest.h == src.h)
		{
			// This is a 1:1 blit, let's use the fast alternative.

			_transformBlitSimple( dest, src.pos(), s_blitFlipTransforms[0], OpType::Blit);
		}
		else
		{
			binalInt	mtx[2][2];

			if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
			{
				if (dest.w == 64)
					mtx[0][0] = 0;
				else
					mtx[0][0] = binalInt(src.w-64) * (BINAL_MUL/64) / ((dest.w/64) - 1);

				mtx[0][1] = 0;
				mtx[1][0] = 0;

				if( dest.h == 64)
					mtx[1][1] = 0;
				else
					mtx[1][1] = binalInt(src.h-64) * (BINAL_MUL/64) / ((dest.h/64) - 1);
			}
			else
			{
				// We want last src sample to be taken as close to the end of the source
				// rectangle as possible in order to get a more balanced representation.

				mtx[0][0] = (binalInt(src.w) * (BINAL_MUL/64)) / (dest.w/64);
				mtx[0][1] = 0;
				mtx[1][0] = 0;
				mtx[1][1] = (binalInt(src.h) * (BINAL_MUL/64)) / (dest.h/64);
			}

			_transformBlitComplex(dest, { binalInt(src.x) * (BINAL_MUL/64), binalInt(src.y) * (BINAL_MUL/64) }, mtx, OpType::Blit);
		}
	}

	//____ stretchFlipBlit() _____________________________________________________

	void GfxDeviceGen1::stretchFlipBlit(const RectSPX& dest, GfxFlip flip)
	{
		assert(m_pBlitSource != nullptr);

		stretchFlipBlit(dest, RectSPX(0, 0, m_pBlitSource->pixelSize()*64), flip);
	}

	void GfxDeviceGen1::stretchFlipBlit(const RectSPX& dest, const RectSPX& src, GfxFlip flip)
	{
		assert(m_pBlitSource != nullptr);

		binalInt scaleX, scaleY;
		binalInt ofsX, ofsY;

		binalInt srcW = binalInt(src.w)  * (BINAL_MUL/64);
		binalInt srcH = binalInt(src.h)  * (BINAL_MUL/64);

		if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
		{
			srcW -= BINAL_MUL;
			srcH -= BINAL_MUL;

			if( dest.w == 64 )
				scaleX = 0;
			else
				scaleX = srcW / ((dest.w/64)-1);

			if( dest.h == 64 )
				scaleY = 0;
			else
				scaleY = srcH / ((dest.h/64)-1);

			ofsX = (binalInt(src.x)  * (BINAL_MUL/64)) + srcW * s_blitFlipOffsets[(int)flip][0];
			ofsY = (binalInt(src.y)  * (BINAL_MUL/64)) + srcH * s_blitFlipOffsets[(int)flip][1];
		}
		else
		{

			//TODO: Not sure about this -1. Was -0.99 when dealing with spx and floats which looked suspicious.

			scaleX = (srcW / (dest.w/64)) -1;
			scaleY = (srcH / (dest.h/64)) -1;

			ofsX = (binalInt(src.x) * (BINAL_MUL/64)) + (srcW - scaleX) * s_blitFlipOffsets[(int)flip][0];
			ofsY = (binalInt(src.y) * (BINAL_MUL/64)) + (srcH - scaleY) * s_blitFlipOffsets[(int)flip][1];
		}

		binalInt	mtx[2][2];

		mtx[0][0] = scaleX * s_blitFlipTransforms[(int)flip][0][0];
		mtx[0][1] = scaleY * s_blitFlipTransforms[(int)flip][0][1];
		mtx[1][0] = scaleX * s_blitFlipTransforms[(int)flip][1][0];
		mtx[1][1] = scaleY * s_blitFlipTransforms[(int)flip][1][1];

		_transformBlitComplex(dest, { ofsX, ofsY }, mtx, OpType::Blit);
	}

	//____ precisionBlit() ____________________________________________________

	void GfxDeviceGen1::precisionBlit(const RectSPX& dest, const RectF& _src)
	{
		assert(m_pBlitSource != nullptr);

		binalInt	mtx[2][2];

		BinalRect src(toBinalInt(_src.x),toBinalInt(_src.y),toBinalInt(_src.w),toBinalInt(_src.h));

		if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
		{
			mtx[0][0] = src.w / (dest.w/64);
			mtx[0][1] = 0;
			mtx[1][0] = 0;
			mtx[1][1] = src.h / (dest.h/64);
		}
		else
		{
			mtx[0][0] = src.w / (dest.w/64);
			mtx[0][1] = 0;
			mtx[1][0] = 0;
			mtx[1][1] = src.h / (dest.h/64);
		}

		_transformBlitComplex(dest, src.pos(), mtx, OpType::Blit);
	}

	//____ transformBlit() ________________________________________________

	void GfxDeviceGen1::transformBlit(const RectSPX& dest, CoordF src, const Transform& transform)
	{
		assert(m_pBlitSource != nullptr);

		binalInt	mtx[2][2];

		mtx[0][0] = binalInt(transform.xx*BINAL_MUL);
		mtx[0][1] = binalInt(transform.xy*BINAL_MUL);
		mtx[1][0] = binalInt(transform.yx*BINAL_MUL);
		mtx[1][1] = binalInt(transform.yy*BINAL_MUL);

		OpType type = m_pBlitSource->isTiling() ? OpType::Tile : OpType::Blit;

		_transformBlitComplex(dest, { toBinalInt(src.x), toBinalInt(src.y) }, mtx, type);
	}

	//____ rotScaleBlit() _____________________________________________________

	void GfxDeviceGen1::rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
	{
		assert(m_pBlitSource != nullptr);

		if (m_pBlitSource->m_size.w * scale < 1.f || m_pBlitSource->m_size.h * scale < 1.f )			// Values very close to zero gives overflow in calculations.
			return;

		BinalCoord	src;
		binalInt	mtx[2][2];

		float	sz = (float)sin(-rotationDegrees*3.14159265/180);
		float	cz = (float)cos(-rotationDegrees*3.14159265 / 180);

		scale = 1.f / scale;

		mtx[0][0] = cz * scale * BINAL_MUL;
		mtx[0][1] = sz * scale * BINAL_MUL;

		mtx[1][0] = -sz * scale * BINAL_MUL;
		mtx[1][1] = cz * scale * BINAL_MUL;

		src = { binalInt(srcCenter.x * m_pBlitSource->m_size.w * BINAL_MUL), binalInt(srcCenter.y * m_pBlitSource->m_size.h * BINAL_MUL) };

//		src.x -= dest.w / 2.f * mtx[0][0] + dest.h / 2.f * mtx[1][0];
//		src.y -= dest.w / 2.f * mtx[0][1] + dest.h / 2.f * mtx[1][1];

		src.x -= (dest.w * destCenter.x * mtx[0][0] + dest.h * destCenter.y * mtx[1][0]) / 64;
		src.y -= (dest.w * destCenter.x * mtx[0][1] + dest.h * destCenter.y * mtx[1][1]) / 64;

		OpType type = m_pBlitSource->isTiling() ? OpType::Tile : OpType::Blit;

		_transformBlitComplex(dest, { src.x,src.y }, mtx, type);
	}

	//____ tile() _____________________________________________________________

	void GfxDeviceGen1::tile(const RectSPX& dest, CoordSPX shift)
	{
		assert(m_pBlitSource != nullptr);

		if (!m_pBlitSource->isTiling())
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		_transformBlitSimple( dest, shift, s_blitFlipTransforms[0], OpType::Tile);
	}

	//____ flipTile() _________________________________________________________

	void GfxDeviceGen1::flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift)
	{
		assert(m_pBlitSource != nullptr);

		if (!m_pBlitSource->isTiling())
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		SizeSPX srcSize = m_pBlitSource->pixelSize()*64;

		spx ofsX = srcSize.w * s_blitFlipOffsets[(int)flip][0];
		spx ofsY = srcSize.h * s_blitFlipOffsets[(int)flip][1];

		ofsX += shift.x * s_blitFlipTransforms[(int)flip][0][0] + shift.y * s_blitFlipTransforms[(int)flip][1][0];
		ofsY += shift.x * s_blitFlipTransforms[(int)flip][0][1] + shift.y * s_blitFlipTransforms[(int)flip][1][1];

		SizeSPX dstSize = dest.size();
		if (s_blitFlipTransforms[(int)flip][0][0] == 0)
			swap(dstSize.w, dstSize.h);

		_transformBlitSimple({ dest.pos(), dstSize }, { ofsX, ofsY }, s_blitFlipTransforms[(int)flip], OpType::Tile);
	}

	//____ scaleTile() _________________________________________________________

	void GfxDeviceGen1::scaleTile(const RectSPX& dest, float scale, CoordSPX shift)
	{
		assert(m_pBlitSource != nullptr);

		if (!m_pBlitSource->isTiling())
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		binalInt	mtx[2][2];

		mtx[0][0] = BINAL_MUL / scale;
		mtx[0][1] = 0;
		mtx[1][0] = 0;
		mtx[1][1] = BINAL_MUL / scale;

		BinalCoord sh = { toBinalInt(shift.x / scale), toBinalInt(shift.y / scale) };

		_transformBlitComplex(dest, sh, mtx, OpType::Tile);
	}

	//____ scaleFlipTile() _________________________________________________________

	void GfxDeviceGen1::scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift)
	{
		assert(m_pBlitSource != nullptr);

		if (!m_pBlitSource->isTiling())
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		binalInt		mtx[2][2];

		mtx[0][0] = s_blitFlipTransforms[(int)flip][0][0] * (BINAL_MUL / scale);
		mtx[0][1] = s_blitFlipTransforms[(int)flip][0][1] * (BINAL_MUL / scale);
		mtx[1][0] = s_blitFlipTransforms[(int)flip][1][0] * (BINAL_MUL / scale);
		mtx[1][1] = s_blitFlipTransforms[(int)flip][1][1] * (BINAL_MUL / scale);

		BinalSize srcSize = BinalSize(m_pBlitSource->pixelSize()) * BINAL_MUL;
		binalInt ofsX = (srcSize.w-BINAL_MUL) * s_blitFlipOffsets[(int)flip][0];
		binalInt ofsY = (srcSize.h-BINAL_MUL) * s_blitFlipOffsets[(int)flip][1];

		ofsX += (shift.x * mtx[0][0] + shift.y * mtx[1][0]) / 64;
		ofsY += (shift.x * mtx[0][1] + shift.y * mtx[1][1]) / 64;

		_transformBlitComplex(dest, {ofsX,ofsY}, mtx, OpType::Tile);
	}

	//____ blur() __________________________________________________________________

	void GfxDeviceGen1::blur(CoordSPX dest)
	{
		assert(m_pBlitSource != nullptr);

		_transformBlitSimple({ dest, m_pBlitSource->pixelSize() * 64 }, { 0,0 }, s_blitFlipTransforms[0], OpType::Blur);
	}

	void GfxDeviceGen1::blur(CoordSPX dest, const RectSPX& src)
	{
		assert(m_pBlitSource != nullptr);

		_transformBlitSimple({ dest, src.size() }, src.pos(), s_blitFlipTransforms[0], OpType::Blur);
	}

	//____ stretchBlur() ___________________________________________________________

	void GfxDeviceGen1::stretchBlur(const RectSPX& dest )
	{
		assert(m_pBlitSource != nullptr);

		stretchBlur(dest, RectSPX(0, 0, m_pBlitSource->pixelSize()*64) );
	}

	void GfxDeviceGen1::stretchBlur(const RectSPX& dest, const RectSPX& src )
	{
		assert(m_pBlitSource != nullptr);

		if (m_pBlitSource == nullptr || dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
			return;

		if (dest.w == src.w && dest.h == src.h)
		{
			// This is a 1:1 blit, let's use the fast alternative.

			_transformBlitSimple( dest, src.pos(), s_blitFlipTransforms[0], OpType::Blur);
		}
		else
		{
			binalInt	mtx[2][2];

			if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
			{
				if (dest.w == 64)
					mtx[0][0] = 0;
				else
					mtx[0][0] = binalInt(src.w-64) * (BINAL_MUL/64) / ((dest.w/64) - 1);

				mtx[0][1] = 0;
				mtx[1][0] = 0;

				if( dest.h == 64)
					mtx[1][1] = 0;
				else
					mtx[1][1] = binalInt(src.h-64) * (BINAL_MUL/64) / ((dest.h/64) - 1);
			}
			else
			{
				// We want last src sample to be taken as close to the end of the source
				// rectangle as possible in order to get a more balanced representation.

				mtx[0][0] = (binalInt(src.w) * (BINAL_MUL/64)) / (dest.w/64);
				mtx[0][1] = 0;
				mtx[1][0] = 0;
				mtx[1][1] = (binalInt(src.h) * (BINAL_MUL/64)) / (dest.h/64);
			}

			_transformBlitComplex(dest, { binalInt(src.x) * (BINAL_MUL/64), binalInt(src.y) * (BINAL_MUL/64) }, mtx, OpType::Blur);
		}
	}

	//____ transformBlur() ________________________________________________

	void GfxDeviceGen1::transformBlur(const RectSPX& dest, CoordF src, const Transform& transform)
	{
		assert(m_pBlitSource != nullptr);

		binalInt	mtx[2][2];

		mtx[0][0] = binalInt(transform.xx * BINAL_MUL);
		mtx[0][1] = binalInt(transform.xy * BINAL_MUL);
		mtx[1][0] = binalInt(transform.yx * BINAL_MUL);
		mtx[1][1] = binalInt(transform.yy * BINAL_MUL);

		_transformBlitComplex(dest, { toBinalInt(src.x), toBinalInt(src.y) }, mtx, OpType::Blur);
	}

	//____ rotScaleBlur() _____________________________________________________

	void GfxDeviceGen1::rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
	{
		assert(m_pBlitSource != nullptr);

		if (m_pBlitSource->m_size.w * scale < 1.f || m_pBlitSource->m_size.h * scale < 1.f )			// Values very close to zero gives overflow in calculations.
			return;

		BinalCoord	src;
		binalInt	mtx[2][2];

		float	sz = (float)sin(-rotationDegrees*3.14159265/180);
		float	cz = (float)cos(-rotationDegrees*3.14159265 / 180);

		scale = 1.f / scale;

		mtx[0][0] = cz * scale * BINAL_MUL;
		mtx[0][1] = sz * scale * BINAL_MUL;

		mtx[1][0] = -sz * scale * BINAL_MUL;
		mtx[1][1] = cz * scale * BINAL_MUL;

		src = { binalInt(srcCenter.x * m_pBlitSource->m_size.w * BINAL_MUL), binalInt(srcCenter.y * m_pBlitSource->m_size.h * BINAL_MUL) };

	//		src.x -= dest.w / 2.f * mtx[0][0] + dest.h / 2.f * mtx[1][0];
	//		src.y -= dest.w / 2.f * mtx[0][1] + dest.h / 2.f * mtx[1][1];

		src.x -= (dest.w * destCenter.x * mtx[0][0] + dest.h * destCenter.y * mtx[1][0]) / 64;
		src.y -= (dest.w * destCenter.x * mtx[0][1] + dest.h * destCenter.y * mtx[1][1]) / 64;

		_transformBlitComplex(dest, { src.x,src.y }, mtx, OpType::Blur);
	}


	//____ blitNinePatch() ________________________________________________

	void GfxDeviceGen1::blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale)
	{
		if( !m_pBlitSource )
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "No blit source set", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		int srcScale = m_pBlitSource->scale();

		const RectSPX srcRect = align(ptsToSpx(patch.block, srcScale));
		const BorderSPX srcFrame = align(ptsToSpx(patch.frame, srcScale));


		if (srcRect.w == dstRect.w && srcRect.h == dstRect.h && srcFrame == dstFrame)
		{
			blit(dstRect.pos(), srcRect);
			return;
		}

		if ((srcFrame.isEmpty() || dstFrame.isEmpty()) && patch.rigidPartXSections == YSections::None && patch.rigidPartYSections == XSections::None)
		{
			stretchBlit(dstRect, srcRect);
			return;
		}

		//TODO: Optimize! Call transformBlit directly instead of going through stretchBlit(), reuse transforms where possible.
		//TODO: Optimize! Use blit instead of stretchBlit on opportunity,fill if center is only 1 pixel and just blit corners if not stretched.

		SizeSPX srcMidSize(srcRect.w - srcFrame.left - srcFrame.right, srcRect.h - srcFrame.top - srcFrame.bottom);
		SizeSPX dstMidSize(dstRect.w - dstFrame.left - dstFrame.right, dstRect.h - dstFrame.top - dstFrame.bottom);

		spx rigidPartXOfs = align(ptsToSpx(patch.rigidPartXOfs, srcScale));
		spx rigidPartXLength = align(ptsToSpx(patch.rigidPartXLength, srcScale));

		spx rigidPartYOfs = align(ptsToSpx(patch.rigidPartYOfs, srcScale));
		spx rigidPartYLength = align(ptsToSpx(patch.rigidPartYLength, srcScale));

		spx rigidPartXLengthDst = align(ptsToSpx(patch.rigidPartXLength, scale));
		spx rigidPartYLengthDst = align(ptsToSpx(patch.rigidPartYLength, scale));


		if (srcFrame.top + dstFrame.top > 0)
		{
			RectSPX	srcNW(srcRect.x, srcRect.y, srcFrame.left, srcFrame.top);
			RectSPX	srcN(srcRect.x + srcFrame.left, srcRect.y, srcMidSize.w, srcFrame.top);
			RectSPX	srcNE(srcRect.x + srcRect.w - srcFrame.right, srcRect.y, srcFrame.right, srcFrame.top);

			RectSPX	dstNW(dstRect.x, dstRect.y, dstFrame.left, dstFrame.top);
			RectSPX	dstN(dstRect.x + dstFrame.left, dstRect.y, dstMidSize.w, dstFrame.top);
			RectSPX	dstNE(dstRect.x + dstRect.w - dstFrame.right, dstRect.y, dstFrame.right, dstFrame.top);

			if (srcNW.w + dstNW.w > 0)
				stretchBlit(dstNW, srcNW);

			if (srcN.w + dstN.w > 0)
			{
				if ((patch.rigidPartXSections & YSections::Top) != YSections::None)
					_stretchBlitWithRigidPartX(srcN, dstN, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
				else
					stretchBlit(dstN, srcN);
			}

			if (srcNE.w + dstNE.w > 0)
				stretchBlit(dstNE, srcNE);
		}


		if (srcMidSize.h > 0 && dstMidSize.h > 0)
		{
			RectSPX	srcW(srcRect.x, srcRect.y + srcFrame.top, srcFrame.left, srcMidSize.h);
			RectSPX	srcC(srcRect.x + srcFrame.left, srcRect.y + srcFrame.top, srcMidSize.w, srcMidSize.h);
			RectSPX	srcE(srcRect.x + srcRect.w - srcFrame.right, srcRect.y + srcFrame.top, srcFrame.right, srcMidSize.h);

			RectSPX	dstW(dstRect.x, dstRect.y + dstFrame.top, dstFrame.left, dstMidSize.h);
			RectSPX	dstC(dstRect.x + dstFrame.left, dstRect.y + dstFrame.top, dstMidSize.w, dstMidSize.h);
			RectSPX	dstE(dstRect.x + dstRect.w - dstFrame.right, dstRect.y + dstFrame.top, dstFrame.right, dstMidSize.h);

			if (srcW.w + dstW.w > 0)
			{
				if((patch.rigidPartYSections & XSections::Left) != XSections::None)
					_stretchBlitWithRigidPartY(srcW, dstW, rigidPartYOfs, rigidPartYLength, rigidPartYLengthDst);
				else
					stretchBlit(dstW, srcW);
			}

			if (srcC.w + dstC.w > 0)
			{
				if ((patch.rigidPartYSections & XSections::Center) != XSections::None)
				{
					spx topSrcLen = rigidPartYOfs;
					spx bottomSrcLen = srcC.h - rigidPartYOfs - rigidPartYLength;

					spx totalSrcLen = topSrcLen + bottomSrcLen;

					spx midDstLen = std::min(rigidPartYLengthDst, dstC.h);

					spx topDstLen = (dstC.h - midDstLen) * topSrcLen / totalSrcLen;
					spx bottomDstLen = dstC.h - midDstLen - topDstLen;


					if (topDstLen > 0)
					{
						RectSPX dst(dstC.x, dstC.y, dstC.w, topDstLen);
						RectSPX src(srcC.x, srcC.y, srcC.w, topSrcLen);

						if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
							_stretchBlitWithRigidPartX(src, dst, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
						else
							stretchBlit(dst, src);
					}

					if (midDstLen > 0)
					{
						RectSPX dst(dstC.x, dstC.y + topDstLen, dstC.w, midDstLen);
						RectSPX src(srcC.x, srcC.y + topSrcLen, srcC.w, rigidPartYLength);

						if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
							_stretchBlitWithRigidPartX(src, dst, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
						else
							stretchBlit(dst, src);
					}

					if (bottomDstLen > 0)
					{
						RectSPX dst(dstC.x, dstC.y + topDstLen + midDstLen, dstC.w, bottomDstLen);
						RectSPX src(srcC.x, srcC.y + topSrcLen + rigidPartYLength, srcC.w, bottomSrcLen);

						if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
							_stretchBlitWithRigidPartX(src, dst, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
						else
							stretchBlit(dst, src);
					}
				}
				else
				{
					if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
						_stretchBlitWithRigidPartX(srcC, dstC, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
					else
						stretchBlit(dstC, srcC);
				}
			}


			if (srcE.w + dstE.w > 0)
			{
				if ((patch.rigidPartYSections & XSections::Right) != XSections::None)
					_stretchBlitWithRigidPartY(srcE, dstE, rigidPartYOfs, rigidPartYLength, rigidPartYLengthDst);
				else
					stretchBlit(dstE, srcE);
			}

		}

		if (srcFrame.bottom + dstFrame.bottom > 0)
		{
			RectSPX	srcSW(srcRect.x, srcRect.y + srcRect.h - srcFrame.bottom, srcFrame.left, srcFrame.bottom);
			RectSPX	srcS(srcRect.x + srcFrame.left, srcRect.y + srcRect.h - srcFrame.bottom, srcMidSize.w, srcFrame.bottom);
			RectSPX	srcSE(srcRect.x + srcRect.w - srcFrame.right, srcRect.y + srcRect.h - srcFrame.bottom, srcFrame.right, srcFrame.bottom);

			RectSPX	dstSW(dstRect.x, dstRect.y + dstRect.h - dstFrame.bottom, dstFrame.left, dstFrame.bottom);
			RectSPX	dstS(dstRect.x + dstFrame.left, dstRect.y + dstRect.h - dstFrame.bottom, dstMidSize.w, dstFrame.bottom);
			RectSPX	dstSE(dstRect.x + dstRect.w - dstFrame.right, dstRect.y + dstRect.h - dstFrame.bottom, dstFrame.right, dstFrame.bottom);

			if (srcSW.w + dstSW.w > 0)
				stretchBlit(dstSW, srcSW);

			if (srcS.w + dstS.w > 0)
			{
				if ((patch.rigidPartXSections & YSections::Bottom) != YSections::None)
					_stretchBlitWithRigidPartX(srcS, dstS, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
				else
					stretchBlit(dstS, srcS);
			}

			if (srcSE.w + dstSE.w > 0)
				stretchBlit(dstSE, srcSE);
		}
	}

	//____ drawWave() ______________________________________________________

	void GfxDeviceGen1::drawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill )
	{
		_transformDrawWave(dest, pTopBorder, pBottomBorder, fill, s_blitFlipTransforms[(int)GfxFlip::None] );
	}

	//____ flipDrawWave() ______________________________________________________

	void GfxDeviceGen1::flipDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill, GfxFlip flip )
	{
		_transformDrawWave(dest, pTopBorder, pBottomBorder, fill, s_blitFlipTransforms[(int)flip] );
	}

	//____ _transformDrawWave() ______________________________________________________

	void GfxDeviceGen1::_transformDrawWave(const RectSPX& _destIn, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill, const int simpleTransform[2][2] )
	{
		//TODO: If borders have different colors and cross, colors are not swapped.

		// Make a bounding box around all clip rectangles

		RectI _dest = _destIn/64;
		RectI bounds = RectI::overlap( m_clipBounds/64, _dest );

		if (bounds.w == 0 || bounds.h == 0)
			return;

		// Generate new destination rect and get waveLength, traceLength and clipping info.

		RectI	dest;
		int		clipBeg, clipEnd;
		int		length;
		int		traceLength;

		if (simpleTransform[0][0] != 0)								// Wave is horizontal
		{
			traceLength = _dest.w;
			dest = { bounds.x, _dest.y, bounds.w, _dest.h };
			clipBeg = bounds.x - _dest.x;
			clipEnd = (_dest.x + _dest.w) - (bounds.x + bounds.w);

			if (simpleTransform[0][0] < 0)							// Wave goes from right to left
				swap(clipBeg, clipEnd);

			length = bounds.w;
		}
		else														// Wave is vertical
		{
			traceLength = _dest.h;
			dest = { _dest.x, bounds.y, _dest.w, bounds.h };
			clipBeg = bounds.y - _dest.y;
			clipEnd = (_dest.y + _dest.h) - (bounds.y + bounds.h);

			if (simpleTransform[1][0] < 0)							// Wave goes from bottom to top
				swap(clipBeg, clipEnd);

			length = bounds.h;
		}

		// Do clipping of our trace

		int traceOfs = 0;
		int startColumn = clipBeg;										// Column of traced lines we start to generate edges from.

		int margin = (int)((max(pTopBorder->thickness, pBottomBorder->thickness)+1) / (2*64));

		if (clipBeg > margin)
		{
			traceOfs = clipBeg - margin;
			traceLength -= traceOfs;
			startColumn -= traceOfs;
		}

		if (clipEnd > margin)
			traceLength -= clipEnd - margin;

		// Generate line traces

		int	lineBufferSize = (traceLength + 1) * 2 * sizeof(int) * 2;	// length+1 * values per point * sizeof(int) * 2 separate traces.
		char * pBuffer = GfxBase::memStackAlloc(lineBufferSize);
		int * pTopBorderTrace = (int*)pBuffer;
		int * pBottomBorderTrace = (int*)(pBuffer + lineBufferSize / 2);

		_traceLine(pTopBorderTrace, traceLength + 1, pTopBorder, traceOfs);
		_traceLine(pBottomBorderTrace, traceLength + 1, pBottomBorder, traceOfs);




		// Generate edges

		int edgeBufferSize;

		// Generate edges

		edgeBufferSize = (length + 1) * 4 * sizeof(int);
		int *	pEdgeBuffer = (int*)GfxBase::memStackAlloc(edgeBufferSize);
		int *	pEdges = pEdgeBuffer;

		for (int i = startColumn; i <= length + startColumn; i++)
		{
			if (pTopBorderTrace[i * 2] > pBottomBorderTrace[i * 2])
				swap(pTopBorderTrace, pBottomBorderTrace);

			pEdges[0] = pTopBorderTrace[i * 2];
			pEdges[1] = pTopBorderTrace[i * 2 + 1];

			pEdges[2] = pBottomBorderTrace[i * 2];
			pEdges[3] = pBottomBorderTrace[i * 2 + 1];

			if (pEdges[2] < pEdges[1])
			{
				pEdges[2] = pEdges[1];
				if (pEdges[3] < pEdges[2])
					pEdges[3] = pEdges[2];
			}

			pEdges += 4;
		}

		// Render the segments

		HiColor	col[5];

		col[0] = HiColor::Transparent;
		col[1] = pTopBorder->color;
		col[2] = fill;
		col[3] = pBottomBorder->color;
		col[4] = HiColor::Transparent;

		_transformDrawSegments(dest*64, 5, col, length + 1, pEdgeBuffer, 4, TintMode::Flat, simpleTransform );

		// Free temporary work memory

		GfxBase::memStackFree(edgeBufferSize);
		GfxBase::memStackFree(lineBufferSize);
	}

	//____ drawElipse() ______________________________________________________

	void GfxDeviceGen1::drawElipse(const RectSPX& canvas, spx thickness, HiColor fillColor, spx outlineThickness, HiColor outlineColor )
	{
		auto pCurveTab = GfxBase::curveTab();
		int curveTabSize = GfxBase::curveTabSize();


		// Center and corners in 26.6 format.

		int x1 = (int) canvas.x;
		int y1 = (int) canvas.y;
		int x2 = (int)(canvas.x + canvas.w);
		int y2 = (int)(canvas.y + canvas.h);

		CoordI center = { (x1 + x2) / 2, (y1 + y2) / 2 };

		// Outer rect of elipse rounded to full pixels.

		RectI outerRect;
		outerRect.x = x1 >> 6;
		outerRect.y = y1 >> 6;
		outerRect.w = ((x2 + 63) >> 6) - outerRect.x;
		outerRect.h = ((y2 + 63) >> 6) - outerRect.y;

		// Adjusted clip

		RectI clip = RectI::overlap(m_clipBounds / 64, outerRect);
		if (clip.w == 0 || clip.h == 0)
			return;

		int clipLeft = clip.x - outerRect.x;

		// Calculate maximum width and height from center for the 4 edges of the elipse.

		int radiusY[4];
		radiusY[0] = (y2 - y1) / 2;
		radiusY[1] = (int)(radiusY[0] - outlineThickness);
		radiusY[2] = (int)(radiusY[1] - thickness);
		radiusY[3] = (int)(radiusY[2] - outlineThickness);

		int radiusX[4];
		radiusX[0] = (x2 - x1) / 2;
		radiusX[1] = (int)(radiusX[0] - outlineThickness);
		radiusX[2] = (int)(radiusX[1] - thickness);
		radiusX[3] = (int)(radiusX[2] - outlineThickness);

		// Reserve buffer for our line traces

		int samplePoints = clip.w + 1;

		int bufferSize = samplePoints * sizeof(int) * 4 * 2;		// length+1 * sizeof(int) * 4 separate traces * 2 halves.
		int * pBuffer = (int*)GfxBase::memStackAlloc(bufferSize);

		// Do line traces.

		int yMid = (center.y & 0xFFFFFFC0) - outerRect.y * 64;
		int yAdjust = center.y & 0x3F;						// Compensate for center not being on pixel boundary.
		int centerOfs = center.x - (outerRect.x << 6);
		int samplePitch = 4;

		for (int edge = 0; edge < 4; edge++)
		{
			int * pOutUpper = pBuffer + edge;
			int * pOutLower = pBuffer + 3 - edge + samplePoints * 4;

			if (radiusX[edge] <= 0 || radiusY[edge] <= 0)
			{
				for (int sample = 0; sample < samplePoints; sample++)
				{
					pOutUpper[sample*samplePitch] = yMid;
					pOutLower[sample*samplePitch] = yMid;
				}
			}
			else
			{
				//TODO: We need to handle subpixels better here.
				// We should increase curvePos with the real xStart offset (could be as much as 63/64 of a pixel)
				// and handle xMid similarly.
				
				int xStart = (centerOfs - radiusX[edge] + 63) >> 6;		// First pixel-edge inside curve.
				int xMid = (centerOfs) >> 6;							// Pixel edge on or right before center.
				int xEnd = (centerOfs + radiusX[edge]) >> 6;			// Last pixel-edge inside curve.


				int curveInc = int(int64_t((65536 * 64)-1) * (curveTabSize - 1) / radiusX[edge]); // Keep as many decimals as possible, this is important!
				int curvePos = int((((radiusX[edge] - centerOfs) & 0x3F) * ((int64_t)curveInc)) >> 6);

				if (clipLeft > 0)
				{
					xStart -= clipLeft;
					xMid -= clipLeft;
					xEnd -= clipLeft;

					if (xStart < 0)
						curvePos += (-xStart) * curveInc;
				}

				// Clip xStart, xMid and xEnd

				if (xEnd >= samplePoints)
				{
					xEnd = samplePoints - 1;

					xStart = min(xStart, xEnd);
					xMid = min(xMid, xEnd);
				}

				//

				int sample = 0;
				while (sample < xStart)
				{
					pOutUpper[sample*samplePitch] = yMid;
					pOutLower[sample*samplePitch] = yMid;
					sample++;
				}

				while (sample <= xMid)
				{
					int i = curvePos >> 16;
					uint32_t f = curvePos & 0xFFFF;

					uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
					int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16);  // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

					pOutUpper[sample*samplePitch] = yMid + yAdjust - height;
					pOutLower[sample*samplePitch] = yMid + yAdjust + height;
					sample++;
					curvePos += curveInc;
				}

				curvePos = (curveTabSize - 1) * 65536 * 2 - curvePos;

				while (sample <= xEnd)
				{
					int i = curvePos >> 16;
					uint32_t f = curvePos & 0xFFFF;

					uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
					int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16); // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

					pOutUpper[sample*samplePitch] = yMid + yAdjust - height;
					pOutLower[sample*samplePitch] = yMid + yAdjust + height;
					sample++;
					curvePos -= curveInc;
				}

				while (sample < samplePoints)
				{
					pOutUpper[sample*samplePitch] = yMid;
					pOutLower[sample*samplePitch] = yMid;
					sample++;
				}

				// Take care of left and right edges that needs more calculations to get the angle right.

				int pixFracLeft = (xStart << 6) - (centerOfs - radiusX[edge]);
				int pixFracRight = (centerOfs + radiusX[edge]) & 0x3F;

				if (pixFracLeft > 0 && xStart > 0 && xStart <= samplePoints )
				{
					pOutUpper[(xStart - 1)*samplePitch] = pOutUpper[xStart*samplePitch] + (yMid + yAdjust - pOutUpper[xStart*samplePitch]) * 64 / pixFracLeft;
					pOutLower[(xStart - 1)*samplePitch] = pOutLower[xStart*samplePitch] + (yMid + yAdjust - pOutLower[xStart*samplePitch]) * 64 / pixFracLeft;

				}
				if (pixFracRight > 0 && xEnd < samplePoints - 1 && xEnd >= -1)
				{
					pOutUpper[(xEnd + 1)*samplePitch] = pOutUpper[xEnd*samplePitch] + (yMid + yAdjust - pOutUpper[xEnd*samplePitch]) * 64 / pixFracRight;
					pOutLower[(xEnd + 1)*samplePitch] = pOutLower[xEnd*samplePitch] + (yMid + yAdjust - pOutLower[xEnd*samplePitch]) * 64 / pixFracRight;
				}

			}
		}

		// Split clip rectangles into upper and lower half, so we clip at the middle.

		int split = min(clip.y + clip.h, outerRect.y + (yMid >> 6)) * 64;

		int clipBufferSize = sizeof(RectSPX)*m_nClipRects * 2;
		RectSPX * pTopClips = (RectSPX*)GfxBase::memStackAlloc(clipBufferSize);
		RectSPX * pBottomClips = pTopClips + m_nClipRects;
		int nTopClips = 0;
		int nBottomClips = 0;

		for (int i = 0; i < m_nClipRects; i++)
		{
			const RectSPX& clip = m_pClipRects[i];

			if (clip.y < split)
			{
				pTopClips[nTopClips] = clip;
				if (clip.bottom() > split)
					pTopClips[nTopClips].h = split - clip.y;
				nTopClips++;
			}
			if (clip.bottom() > split)
			{
				pBottomClips[nBottomClips] = clip;
				if (clip.y < split)
				{
					pBottomClips[nBottomClips].h -= split - clip.y;
					pBottomClips[nBottomClips].y = split;
				}
				nBottomClips++;
			}
		}

		// Render columns

		HiColor	col[5];
		col[0] = HiColor::Transparent;
		col[1] = outlineColor;
		col[2] = fillColor;
		col[3] = outlineColor;
		col[4] = HiColor::Transparent;

		const RectI * pOldClipRects = m_pClipRects;
		int nOldClipRects = m_nClipRects;

		setClipList(nTopClips, pTopClips);
		drawSegments( RectSPX(clip.x,outerRect.y,clip.w,outerRect.h)*64, 5, col, samplePoints, pBuffer, 4);
		setClipList(nBottomClips, pBottomClips);
		drawSegments( RectSPX(clip.x,outerRect.y,clip.w,outerRect.h)*64, 5, col, samplePoints, pBuffer + samplePoints * 4, 4);
		setClipList(nOldClipRects, pOldClipRects);

		// Free temporary work memory

		GfxBase::memStackFree(clipBufferSize);
		GfxBase::memStackFree(bufferSize);

	}

	//____ drawPieChart() _____________________________________________________

	void GfxDeviceGen1::drawPieChart(const RectSPX& _canvas, float start, int nSlices, const float * _pSliceSizes, const HiColor * pSliceColors, float hubSize, HiColor hubColor, HiColor backColor, bool bRectangular)
	{
		static const int c_maxSlices = c_maxSegments - 2;

		if (nSlices < 0 || nSlices > c_maxSlices)
		{
			//TODO: Error handling;
			return;
		}

		//TODO: Early out if no slices, no hud and no background
		//TODO: Replace with fill if no hub, no slices.

		RectI canvas = _canvas/64;
		canvas.w = (canvas.w + 1) & 0xFFFFFFFC;
		canvas.h = (canvas.h + 1) & 0xFFFFFFFC;

		if (canvas.w <= 0 || canvas.h <= 0 || (hubSize == 1.f && !bRectangular))
			return;


		// Setup our slices

		struct Slice
		{
			float ofs;
			float size;
			HiColor color;
		};

		Slice slices[c_maxSlices+2];			// Maximum two extra slices in the end. Beginning offset + end transparency.

		float totalSize = 0.f;

		// Trim our slices, so we have a length of most 1.0.

		Slice trimmedSlices[c_maxSlices+1];

		float ofs = start;
		for (int i = 0; i < nSlices; i++)
		{
			if (ofs >= 1.f)
				ofs = fmod(ofs, 1.f);

			float sliceSize = _pSliceSizes[i];

			if (totalSize + sliceSize >= 1.f)
			{
				trimmedSlices[i] = { ofs, 1.f - totalSize, pSliceColors[i] };
				totalSize = 1.f;
				nSlices = i + 1;
				break;
			}


			trimmedSlices[i] = { ofs,sliceSize, pSliceColors[i] };
			totalSize += sliceSize;
			ofs += sliceSize;
		}

		// Adding an empty slice at end if totalSize < 1.0

		if (totalSize < 0.9999f)
		{
			if (ofs >= 1.f)
				ofs = fmod(ofs, 1.f);

			trimmedSlices[nSlices++] = { ofs,1.f-totalSize, backColor };
		}

		// Find first slice (one with smallest offset)

		int		firstSlice = 0, lastSlice;
		float	firstSliceOfs = 1.f;

		for (int i = 0; i < nSlices; i++)
		{
			if (trimmedSlices[i].ofs < firstSliceOfs)
			{
				firstSlice = i;
				firstSliceOfs = trimmedSlices[i].ofs;
			}

		}

		// Find last slice

		if (firstSlice == 0)
			lastSlice = nSlices - 1;
		else
			lastSlice = firstSlice - 1;

		// Take care of possible rounding errors on inparameters

		if (firstSliceOfs < 0.0001f)
			trimmedSlices[firstSlice].ofs = firstSliceOfs = 0.f;

		// Rearrange our slices so we start from offset 0. Possibly adding one more slice in the beginning.

		int sliceIdx = 0;

		if (nSlices == 1)
		{
			slices[sliceIdx++] = { 0.f, 1.f, trimmedSlices[0].color };
		}
		else
		{
			// Fill in rollover from last slice (or transparent gap) due to rotation

			if (firstSliceOfs > 0.f )
				slices[sliceIdx++] = { 0.f, firstSliceOfs, trimmedSlices[lastSlice].color };

			// Our buffer is circular, take care of slices from first slice to end of buffer.

			for (int i = firstSlice; i < nSlices; i++)
				slices[sliceIdx++] = trimmedSlices[i];

			// Take care of slices from beginning of buffer to last slice.

			if (lastSlice < firstSlice)
			{
				for (int i = 0; i <= lastSlice; i++)
					slices[sliceIdx++] = trimmedSlices[i];
			}

			// Correct for any rollover or inprecision for last slice

			slices[sliceIdx - 1].size = 1.f - slices[sliceIdx - 1].ofs;
		}

		nSlices = sliceIdx;			// Repurpose this variable

		// Slices now in order, lets render the quadrants

		int quadW = canvas.w / 2, quadH = canvas.h / 2;

		RectI quadCanvas[4] = { {canvas.x + quadW, canvas.y, quadW, quadH },
								{canvas.x + quadW, canvas.y + quadH, quadW, quadH},
								{canvas.x, canvas.y + quadH, quadW, quadH},
								{canvas.x, canvas.y, quadW, quadH} };

		GfxFlip quadFlip[4] = { GfxFlip::None, GfxFlip::Rot90, GfxFlip::Rot180, GfxFlip::Rot270 };

		HiColor colors[c_maxSegments];

		int maxSegments = nSlices + 2;
		int edgePitch = maxSegments - 1;
		int bufferSize = (quadW + 1) * edgePitch * sizeof(int);

		int * pBuffer = (int*) GfxBase::memStackAlloc(bufferSize);

		
		auto pCurveTab = GfxBase::curveTab();
		int curveTabSize = GfxBase::curveTabSize();

		// Setting the outer edge (same for all quads)

		if (bRectangular)
		{
			int * pEdge = pBuffer;
			for (int i = 0; i <= quadW; i++)
			{
				*pEdge = 0;
				pEdge += edgePitch;
			}
		}
		else
		{
			int * pEdge = pBuffer;

			int curveTabInc = ((curveTabSize << 16)-1) / (quadW);

			int curveTabOfs = 0;
			for (int i = 0; i <= quadW; i++)
			{
				*pEdge = (quadH<<6) - ((pCurveTab[(curveTabSize-1)-(curveTabOfs >> 16)] * quadH) >> 10);
				pEdge += edgePitch;
				curveTabOfs += curveTabInc;
			}
		}

		// Generating an inner edge if we have one.
		// Storing it separately, used for all 4 quads. Copied into right place later.

		int * pHubBuffer = nullptr;
		int hubBufferSize = 0;
		if (hubSize > 0.f)
		{
			hubBufferSize = (quadW + 1) * sizeof(int);
			pHubBuffer = (int*)GfxBase::memStackAlloc(hubBufferSize);

			int * p = pHubBuffer;

			int ringW = int(hubSize * quadW);
			int ringH = int(hubSize * quadH);

			int inc = ((curveTabSize << 16)-1) / (ringW);
			int ofs = 0;

			for (int i = 0; i <= ringW; i++)
			{
				*p++ = (quadH << 6) - ((pCurveTab[(curveTabSize - 1) - (ofs >> 16)] * ringH) >> 10);
				ofs += inc;
			}

			int maxVal = quadH << 6;
			for (int i = ringW + 1; i <= quadW; i++)
				*p++ = maxVal;
		}

		//

		for (int quad = 0; quad < 4; quad++)
		{
			int nSegments = 0;
			Slice * pSlice = slices;
			Slice * pSliceEnd = slices + nSlices;

			float quadStartOfs = 0.25f * quad;

			// Add background as first segment

			colors[nSegments] = backColor;
			nSegments++;

			// Find first slice to include

			while (pSlice != pSliceEnd && pSlice->ofs + pSlice->size < quadStartOfs )
				pSlice++;

			colors[nSegments] = pSlice->color;
			nSegments++;
			pSlice++;

			// Generate edges for all following slices included

			while (pSlice != pSliceEnd && pSlice->ofs < quadStartOfs + 0.25f)
			{
				int * pEdge = pBuffer + nSegments-1;

				// Set startvalue and decrease per strip.

				int value = quadH << 6;
				int dec;

				float rot = (pSlice->ofs - quadStartOfs);

				if (rot == 0.f)
				{
					value = 0;
					dec = 0;
				}
				else
				{
					rot *= 3.14159265358979f * 2;
					float s = sin(rot);
					float c = cos(rot);

					float decF = (c/s) * (quadH << 12) / float(quadW);

					if (decF > 400000*4096)
						decF = 400000*4096;

					dec = int(decF);
				}

				// Fill in the edge

				int strip;
				int precisionValue = value << 6;
				for (strip = 0; strip <= quadW && value >= pEdge[-1] ; strip++)
				{
					* pEdge = value;
					pEdge += edgePitch;
					precisionValue -= dec;
					value = precisionValue >> 6;
				}

				while (strip <= quadW)
				{
					*pEdge = pEdge[-1];
					pEdge += edgePitch;
					strip++;
				}

				colors[nSegments] = pSlice->color;
				nSegments++;
				pSlice++;
			}

			// Add edge for hub if present

			if (pHubBuffer)
			{
				int * pEdge = pBuffer + nSegments - 1;

				for (int i = 0; i <= quadW; i++)
				{
					int value = pHubBuffer[i];
					*pEdge = value;

					int * pPrev = pEdge - 1;
					while (*pPrev > value)
					{
						* pPrev = value;
						pPrev--;
					}

					pEdge += edgePitch;
				}

				colors[nSegments] = hubColor;
				nSegments++;
			}

			// Draw

			int * pEdges = pBuffer;
			HiColor * pColors = colors;
			if (bRectangular)
			{
				pEdges++;
				pColors++;
				nSegments--;
			}

			int16_t		alphaCheck = 0;
			for (int i = 0; i < nSegments; i++)
				alphaCheck |= pColors[i].a;

			if (alphaCheck == 0)
				continue;

			if (nSegments == 1)
				fill(quadCanvas[quad], pColors[0]);
			else
				_transformDrawSegments(quadCanvas[quad]*64, nSegments, pColors, quadW+1, pEdges, edgePitch, TintMode::Flat, s_blitFlipTransforms[(int)quadFlip[quad]]);
		}

		if( hubBufferSize != 0 )
			GfxBase::memStackFree(hubBufferSize);

		GfxBase::memStackFree(bufferSize);
	}

	//____ drawSegments() ______________________________________________________

	void GfxDeviceGen1::drawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode )
	{
		_transformDrawSegments( dest, nSegments, pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, tintMode, s_blitFlipTransforms[(int)GfxFlip::None] );
	}

	//____ flipDrawSegments() ______________________________________________________

	void GfxDeviceGen1::flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode)
	{
		_transformDrawSegments(dest, nSegments, pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, tintMode, s_blitFlipTransforms[(int)flip] );
	}

	//____ drawEdgemap() __________________________________________________________

	void GfxDeviceGen1::drawEdgemap(CoordSPX dest, Edgemap * pEdgemap )
	{
		flipDrawEdgemap(dest,pEdgemap,GfxFlip::None);
	}


	//____ _clipListWasChanged() _________________________________________________

	void GfxDeviceGen1::_clipListWasChanged()
	{
		// Do nothing.
	}

	//____ _traceLine() __________________________________________________________

	void GfxDeviceGen1::_traceLine(int * pDest, int nPoints, const WaveLine * pWave, int offset)
	{
		auto pCurveTab = GfxBase::curveTab();
		int curveTabSize = GfxBase::curveTabSize();
		
		static const int c_supersamples = 4;

		static int brush[128 * c_supersamples];
		static spx prevThickness = -1;

		spx thickness = pWave->thickness;
		int brushSteps = (int)(thickness * c_supersamples / (2*64));

		// Generate brush

		if (thickness != prevThickness)
		{
			brush[0] = thickness / 2;
			for (int i = 1; i <= brushSteps; i++)
				brush[i] = (thickness * pCurveTab[curveTabSize - (i*curveTabSize) / brushSteps]) >> (16+1);
			prevThickness = thickness;
		}

		int nTracePoints = max(0, min(nPoints, pWave->length - offset));
		int nFillPoints = nPoints - nTracePoints;

		// Trace...
		int * pSrc = nullptr;
		if(pWave->pWave != nullptr)
			pSrc = pWave->pWave + offset;

		for (int i = 0; i < nTracePoints; i++)
		{
			// Start with top and bottom for current point

			int top = pSrc[i] - brush[0];
			int bottom = pSrc[i] + brush[0];

			// Check brush's coverage from previous points

			int end = min(i + 1, brushSteps + 1);

			for (int j = 1; j < end; j++)
			{
				int from = pSrc[i - j / c_supersamples];
				int to = pSrc[i - j / c_supersamples - 1];

				int sample = (to - from) * (j%c_supersamples) / c_supersamples + from;

				int topCover = sample - brush[j];
				int bottomCover = sample + brush[j];

				if (topCover < top)
					top = topCover;
				else if (bottomCover > bottom)
					bottom = bottomCover;
			}

			// Check brush's coverage from following points

			end = min(nTracePoints - i, brushSteps + 1);

			for (int j = 1; j < end; j++)
			{
				int from = pSrc[i + j / c_supersamples];
				int to = pSrc[i + j / c_supersamples + 1];

				int sample = (to - from) * (j%c_supersamples) / c_supersamples + from;

				int topCover = sample - brush[j];
				int bottomCover = sample + brush[j];

				if (topCover < top)
					top = topCover;
				else if (bottomCover > bottom)
					bottom = bottomCover;
			}

			// Save traced values

			*pDest++ = top;
			*pDest++ = bottom;
		}

		// Fill...

		if (nFillPoints)
		{
			int top = pWave->hold - brush[0];
			int bottom = pWave->hold + brush[0];
			for (int i = 0; i < nFillPoints; i++)
			{
				*pDest++ = top;
				*pDest++ = bottom;
			}
		}
	}

	//____ _stretchBlitWithRigidPartX() _______________________________________

	void GfxDeviceGen1::_stretchBlitWithRigidPartX(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst)
	{
		int leftSrcLen = rigidPartOfs;
		int rightSrcLen = src.w - rigidPartOfs - rigidPartLength;

		int totalSrcLen = leftSrcLen + rightSrcLen;

		int midDstLen = std::min(rigidPartLengthDst, dst.w);

		int leftDstLen = (dst.w - midDstLen) * leftSrcLen / totalSrcLen;
		int rightDstLen = dst.w - midDstLen - leftDstLen;

		if (leftDstLen > 0)
			stretchBlit(RectSPX(dst.x, dst.y, leftDstLen, dst.h), RectSPX(src.x, src.y, leftSrcLen, src.h));

		if (midDstLen > 0)
			stretchBlit(RectSPX(dst.x + leftDstLen, dst.y, midDstLen, dst.h), RectSPX(src.x + leftSrcLen, src.y, rigidPartLength, src.h));

		if (rightDstLen > 0)
			stretchBlit(RectSPX(dst.x + leftDstLen + midDstLen, dst.y, rightDstLen, dst.h), RectSPX(src.x + leftSrcLen + rigidPartLength, src.y, rightSrcLen, src.h));
	}

	//____ _stretchBlitWithRigidPartY() _______________________________________

	void GfxDeviceGen1::_stretchBlitWithRigidPartY(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst)
	{
		int topSrcLen = rigidPartOfs;
		int bottomSrcLen = src.h - rigidPartOfs - rigidPartLength;

		int totalSrcLen = topSrcLen + bottomSrcLen;

		int midDstLen = std::min(rigidPartLengthDst, dst.h);

		int topDstLen = (dst.h - midDstLen) * topSrcLen / totalSrcLen;
		int bottomDstLen = dst.h - midDstLen - topDstLen;

		if (topDstLen > 0)
			stretchBlit(RectSPX(dst.x, dst.y, dst.w, topDstLen), RectSPX(src.x, src.y, src.w, topSrcLen));

		if (midDstLen > 0)
			stretchBlit(RectSPX(dst.x, dst.y + topDstLen, dst.w, midDstLen), RectSPX(src.x, src.y + topSrcLen, src.w, rigidPartLength));

		if (bottomDstLen > 0)
			stretchBlit(RectSPX(dst.x, dst.y + topDstLen + midDstLen, dst.w, bottomDstLen), RectSPX(src.x, src.y + topSrcLen + rigidPartLength, src.w, bottomSrcLen));
	}

} // namespace wg
