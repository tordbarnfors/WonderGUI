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

#include <wg_multiblockskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>

#include <assert.h>

namespace wg
{
	using namespace Util;

	const TypeInfo MultiBlockSkin::TYPEINFO = { "MultiBlockSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	MultiBlockSkin_p MultiBlockSkin::create(Size blockSize, Border frame)
	{
		return MultiBlockSkin_p(new MultiBlockSkin(blockSize,frame));
	}


	//____ constructor ____________________________________________________________

	MultiBlockSkin::MultiBlockSkin(Size blockSize, Border frame)
	{
		m_blockSizePoints	= blockSize;
		m_frame				= frame;

		for( int i = 0 ; i < State::NbStates ; i++ )
		m_bStateOpaque[i] = false;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MultiBlockSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ addLayer() _____________________________________________________________

	int MultiBlockSkin::addLayer(Surface * pSurf, Coord ofs)
	{
		//

		m_layers.emplace_back();
		auto& layer = m_layers.back();

		layer.blendMode = BlendMode::Blend;
		layer.pSurface = pSurf;
		layer.stateBlockMask = 1;               // Only normal state is set.


		for (int i = 0; i < State::NbStates; i++)
		{
			layer.blockOfs[i] = ofs;
			layer.tintColor[i] = HiColor::White;

			_updateStateOpacity(i);
		}


		return (int) m_layers.size();
	}

	int MultiBlockSkin::addLayer(Surface * pSurf, Coord blockStartOfs, Size blockPitch, std::initializer_list<State> stateBlocks)
	{
		//

		m_layers.emplace_back();
		auto& layer = m_layers.back();

		layer.blendMode = BlendMode::Blend;
		layer.pSurface = pSurf;
		layer.stateBlockMask = 0;

		//

		for (int i = 0; i < State::NbStates; i++)
			layer.tintColor[i] = HiColor::White;

		//

		int ofs = 0;
		for (State state : stateBlocks)
		{
			int index = state;

			layer.stateBlockMask.setBit(index);
			layer.blockOfs[index] = blockStartOfs + Coord( blockPitch.w*ofs, blockPitch.h*ofs );
			ofs++;
		}

		//

		assert(layer.stateBlockMask.bit(0) == true);				// A block for state normal is required.

		// Fill in fallback states and update opacity flag

		for (int i = 0; i < State::NbStates; i++)
		{
			if (!layer.stateBlockMask.bit(i))
			{
				int fallbackIndex = bestStateIndexMatch(i, layer.stateBlockMask);
				layer.blockOfs[i] = layer.blockOfs[fallbackIndex];
			}

			_updateStateOpacity(i);
		}

		//

		return (int) m_layers.size();
	}

	int MultiBlockSkin::addLayer(Surface * pSurf, std::initializer_list<State> stateBlocks, Axis axis, pts spacing, Coord blockStartOfs )
	{
		Size blockPitch;

		if (axis == Axis::X)
			blockPitch.w = m_blockSizePoints.w + spacing;
		else
			blockPitch.h = m_blockSizePoints.h + spacing;

		return addLayer(pSurf, blockStartOfs, blockPitch, stateBlocks);
	}



	//____ setLayerColor() _________________________________________________________

	bool MultiBlockSkin::setLayerColor(int layerIdx, HiColor tintColor)
	{
		auto& layer = m_layers.at(layerIdx-1);

		for (int i = 0; i < State::NbStates; i++)
		{
			int16_t		oldAlpha = layer.tintColor[i].a;

			layer.tintColor[i] = tintColor;

			if (oldAlpha != layer.tintColor[i].a)
				_updateStateOpacity(i);
		}
		return true;
	}


	bool MultiBlockSkin::setLayerColor(int layerIdx, std::initializer_list< std::pair<State,HiColor> > stateColors)
	{
		auto& layer = m_layers.at(layerIdx-1);

		// Set default color for normal state.

		layer.tintColor[0] = HiColor::White;

		//

		for (auto& stateColor : stateColors)
		{
			int index = stateColor.first;

			int16_t		oldAlpha = layer.tintColor[index].a;

			layer.tintColor[index] = stateColor.second;

			if (oldAlpha != stateColor.second.a)
				_updateStateOpacity(index);

			layer.stateColorMask.setBit(index);
		}

		// Fill in fallback states and update opacity flag

		for (int i = 0; i < State::NbStates; i++)
		{
			if (!layer.stateColorMask.bit(i))
			{
				int16_t		oldAlpha = layer.tintColor[i].a;

				int fallbackIndex = bestStateIndexMatch(i, layer.stateBlockMask);
				layer.tintColor[i] = layer.tintColor[fallbackIndex];

				if (oldAlpha != layer.tintColor[i].a )
					_updateStateOpacity(i);
			}
		}

		//

		return true;
	}

	//____ SetLayerBlendMode() ____________________________________________________

	bool MultiBlockSkin::setLayerBlendMode(int layerIdx, BlendMode blendMode)
	{
		auto& layer = m_layers.at(layerIdx - 1);

		if (layer.blendMode != blendMode)
		{
			layer.blendMode = blendMode;

			for (int i = 0; i < State::NbStates; i++)
				_updateStateOpacity(i);
		}

		return true;
	}

	//____ _render() _______________________________________________________________

	void MultiBlockSkin::_render( GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RectSPX blockSize = align(ptsToSpx(m_blockSizePoints, scale));

		if (m_layers.empty() || blockSize.w <= 0 || blockSize.h <= 0 )
			return;

		int stateIndex = state;

		int orgLayer;
		if (m_layer != -1)
		{
			orgLayer = pDevice->renderLayer();
			pDevice->setRenderLayer(m_layer);
		}

		BlendMode	orgBlendMode = pDevice->blendMode();
		HiColor		orgTintColor = pDevice->tintColor();

		BlendMode	blendMode = orgBlendMode;
		HiColor		tintColor = orgTintColor;
		HiColor		mixedTint = orgTintColor;

		for (auto& layer : m_layers)
		{
			if (layer.blendMode != blendMode)
			{
				blendMode = layer.blendMode;
				pDevice->setBlendMode(blendMode);
			}

			if (layer.tintColor[stateIndex] != tintColor )
			{
				tintColor = layer.tintColor[stateIndex];
				mixedTint = orgTintColor * tintColor;
				pDevice->setTintColor(mixedTint);
			}

			pDevice->setBlitSource(layer.pSurface);

			NinePatch patch;
			patch.block = Rect(layer.blockOfs[stateIndex], m_blockSizePoints);
			patch.frame = m_frame;
			pDevice->blitNinePatch(canvas, align(ptsToSpx(m_frame,scale)), patch, scale);
		}

		if (mixedTint != orgTintColor)
			pDevice->setTintColor(orgTintColor);

		if (blendMode != orgBlendMode)
			pDevice->setBlendMode(orgBlendMode);

		if (m_layer != -1)
			pDevice->setRenderLayer(orgLayer);
	}


	//____ _minSize() ______________________________________________________________

	SizeSPX MultiBlockSkin::_minSize(int scale) const
	{
		SizeSPX content = StateSkin::_minSize(scale);
		SizeSPX frame = align(ptsToSpx(m_frame,scale));
		return SizeSPX::max(content, frame);
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX MultiBlockSkin::_defaultSize(int scale) const
	{
        // Default size is when each point of the surface maps to a pixel of the skinarea.
        
        return align(ptsToSpx(m_blockSizePoints,scale));
	}

	//____ _sizeForContent() _______________________________________________________

	SizeSPX MultiBlockSkin::_sizeForContent( const SizeSPX& contentSize, int scale ) const
	{
		SizeSPX sz = StateSkin::_sizeForContent(contentSize,scale);
		SizeSPX min = align(ptsToSpx(m_frame, scale));

		return SizeSPX::max(sz, min);
	}

	//____ _markTest() _____________________________________________________________

	bool MultiBlockSkin::_markTest( const CoordSPX& _ofs, const RectSPX& canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX blockSize = align(ptsToSpx(m_blockSizePoints, scale));

		if (!canvas.contains(_ofs) || m_layers.empty() || blockSize.w <= 0 || blockSize.h <= 0)
			return false;

		int stateIndex = state;

		for (auto& layer : m_layers)
		{
			Coord srcOfs = layer.blockOfs[stateIndex];

			NinePatch patch;
			patch.block = { srcOfs, m_blockSizePoints };
			patch.frame = m_frame;

			int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

			bool bMarked = markTestNinePatch(_ofs, layer.pSurface, patch, canvas, scale, alpha);
			if (bMarked)
				return true;
		}

		return false;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX MultiBlockSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_bStateOpaque[state] )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}


	//____ _dirtyRect() ______________________________________________________

	RectSPX MultiBlockSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		for (auto& layer : m_layers)
		{
			if (layer.blockOfs[i1] != layer.blockOfs[i2] || layer.tintColor[i1] != layer.tintColor[i2])
				return canvas;
		}

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _updateStateOpacity() __________________________________________________

	void MultiBlockSkin::_updateStateOpacity(int stateIdx)
	{
		bool bOpaque = false;

		for (auto& layer : m_layers)
		{
			if (layer.blendMode == BlendMode::Replace || (layer.blendMode == BlendMode::Blend && layer.pSurface->isOpaque() && layer.tintColor[stateIdx].a == 4096) )
			{
				bOpaque = true;
				break;
			}
		}

		m_bStateOpaque[stateIdx] = bOpaque;
	}

}
