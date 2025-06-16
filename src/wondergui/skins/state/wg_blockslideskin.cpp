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

#include <wg_blockslideskin.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

#include <cassert>

namespace wg
{

	using namespace Util;

	const TypeInfo BlockSlideSkin::TYPEINFO = { "BlockSlideSkin", &Skin::TYPEINFO };

	//____ create() _______________________________________________________________

	BlockSlideSkin_p BlockSlideSkin::create( const Blueprint& blueprint )
	{
		return BlockSlideSkin_p(new BlockSlideSkin(blueprint));
	}

	//____ constructor ________________________________________________________

	BlockSlideSkin::BlockSlideSkin( const Blueprint& bp) : StateSkin(bp)
	{	
		m_pSurface = bp.surface;
		m_slideDirection = bp.slideDirection;
		m_slideDuration = bp.slideDuration;
		m_slideState = bp.slideState;

		m_gradient	= bp.gradient;
		m_blendMode = bp.blendMode;

		m_transitionTimes[int(m_slideState)] = m_slideDuration;

		Axis slideAxis = bp.slideDirection == Direction::Up || bp.slideDirection == Direction::Down ? Axis::Y : Axis::X;


		Rect lane = bp.firstLane;
		
		if (lane.isEmpty())
		{

			int nStateBlocks = 1;
			for ( auto& entry : bp.states)
			{
				if (entry.state != State::Default && entry.data.blockless == false)
					nStateBlocks++;
			}


			if (slideAxis == Axis::X)
			{
				lane.w = m_pSurface->pointWidth();
				lane.h = (m_pSurface->pointHeight() - bp.blockSpacing * (nStateBlocks - 1)) / nStateBlocks;
			}
			else
			{
				lane.w = (m_pSurface->pointWidth() - bp.blockSpacing * (nStateBlocks - 1)) / nStateBlocks;
				lane.h = m_pSurface->pointHeight();
			}
		}
		
		Rect block = lane;
		if( slideAxis == Axis::X )
		{
			m_slideDistance = lane.w - bp.blockLength;
			block.w = bp.blockLength;
		}
		else
		{
			m_slideDistance = lane.h - bp.blockLength;
			block.h = bp.blockLength;
		}

		m_blockSize = block.size();

		if (m_slideDirection == Direction::Up)
			block.y += m_slideDistance;
		else if (m_slideDirection == Direction::Left)
			block.x += m_slideDistance;


		// Generate lists of states that affects shift, color and block.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		HiColor stateColors[State::NbStates];

		State	blockStates[State::NbStates];
		Coord	stateBlocks[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbColorStates = 1;
		int		nbBlockStates = 1;

		shiftingStates[0] = State::Default;
		colorStates[0] = State::Default;
		blockStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateColors[0] = bp.color;
		stateBlocks[0] = block;

		Coord blockOfs = block.pos();
		Coord pitch = slideAxis == Axis::X ? Coord(0, block.h + bp.blockSpacing) : Coord(block.w + bp.blockSpacing, 0);

		for (auto& stateInfo : bp.states)
		{
			int index = stateInfo.state;

			if (stateInfo.data.contentShift.x != 0 || stateInfo.data.contentShift.y != 0)
			{
				int index = stateInfo.state == State::Default ? 0 : nbShiftingStates++;
				shiftingStates[index] = stateInfo.state;
				stateShifts[index] = stateInfo.data.contentShift;
				m_bContentShifting = true;
			}

			if ( stateInfo.state != State::Default && !stateInfo.data.blockless )
			{
				int index = stateInfo.state == State::Default ? 0 : nbBlockStates++;
				stateBlocks[index] = blockOfs + pitch * index;
				blockStates[index] = stateInfo.state;
			}

			if(stateInfo.data.color != HiColor::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbColorStates++;
				colorStates[index] = stateInfo.state;
				stateColors[index] = stateInfo.data.color;
			}
		}

		// Calc size of index table for block and color, get their index masks & shifts.

		int	blockIndexEntries;
		int	colorIndexEntries;

		std::tie(blockIndexEntries,m_blockIndexMask,m_blockIndexShift) = calcStateToIndexParam(nbBlockStates, blockStates);
		std::tie(colorIndexEntries,m_colorIndexMask,m_colorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);


		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int blockBytes		= sizeof(Coord) * nbBlockStates;
		int colorBytes		= sizeof(HiColor) * nbColorStates;
		int indexBytes		= blockIndexEntries+colorIndexEntries;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(shiftBytes + blockBytes + colorBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		auto pBlocks = (Coord*) pDest;
		for( int i = 0 ; i < nbBlockStates ; i++ )
			pBlocks[i] = stateBlocks[i];

		m_pBlocks = pBlocks;
		pDest += blockBytes;

		auto pColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbColorStates ; i++ )
			pColors[i] = stateColors[i];

		m_pColors = pColors;

		pDest += colorBytes;

		m_pBlockIndexTab = pDest;
		m_pColorIndexTab = pDest + blockIndexEntries;

		generateStateToIndexTab(m_pBlockIndexTab, nbBlockStates, blockStates);
		generateStateToIndexTab(m_pColorIndexTab, nbColorStates, colorStates);

		_updateOpaqueFlags();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BlockSlideSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX BlockSlideSkin::_defaultSize(int scale) const
	{
		// Default size is when each point of the surface maps to a point of the skinarea,
		// independent of differences in scale.

		return align(ptsToSpx(m_blockSize, scale))  + align(ptsToSpx(m_spacing, scale));
	}

	//____ _render() _______________________________________________________________

	void BlockSlideSkin::_render(GfxDevice* pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		if (!m_pSurface)
			return;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		RenderSettingsWithGradient settings(pDevice, m_layer, m_blendMode, _getColor(state), canvas, m_gradient);

		pDevice->setBlitSource(m_pSurface);

		RectSPX source = _partInCanvas(scale, state, pStateFractions);

		pDevice->stretchBlit(canvas, source);
	}

	//____ _partInCanvas() _______________________________________________________

	RectSPX BlockSlideSkin::_partInCanvas(int scale, State state, float* pStateFractions) const
	{
		Coord blockOfs = _getBlock(state);

		Rect source = { blockOfs, m_blockSize };

		State	slideStateMask;
		switch (m_slideState)
		{
		case PrimState::Focused:
			slideStateMask = State::Focused;
			break;
		case PrimState::Hovered:
			slideStateMask = State::Hovered;
			break;
		case PrimState::Pressed:
			slideStateMask = State::Pressed;
			break;
		case PrimState::Targeted:
			slideStateMask = State::Targeted;
			break;
		case PrimState::Selected:
			slideStateMask = State::Selected;
			break;
		case PrimState::Checked:
			slideStateMask = State::Checked;
			break;
		case PrimState::Flagged:
			slideStateMask = State::Flagged;
			break;
		case PrimState::Disabled:
			slideStateMask = State::Disabled;
			break;
		default:
			assert(false);
		}

		bool bMaximized = ((slideStateMask.index() & state.index()) != 0);

		float offset = 0.f;
		if (pStateFractions != nullptr)
			offset = pStateFractions[int(m_slideState)];
		else if (bMaximized)
			offset = 1.f;

		switch (m_slideDirection)
		{
		case Direction::Up:
			source.y -= offset * m_slideDistance;
			break;
		case Direction::Down:
			source.y += offset * m_slideDistance;
			break;
		case Direction::Left:
			source.x -= offset * m_slideDistance;
			break;
		case Direction::Right:
			source.x += offset * m_slideDistance;
			break;
		}

		return ptsToSpx(source,scale);
	}


	//____ _markTest() _____________________________________________________________

	bool BlockSlideSkin::_markTest(const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		//TODO: Take blendMode and tint (incl gradient) into account.

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if( !canvas.contains(ofs) )
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		//TODO: Implement the rest!

		return true;
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX BlockSlideSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		//TODO: Implement!

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		return canvas;

/*
		if (oldState == newState)
			return Rect();

		int i1 = _stateToIndex(newState);
		int i2 = _stateToIndex(oldState);

		if (m_stateBlocks[i1] != m_stateBlocks[i2])
			return canvas;

		return StateSkin::dirtyRect(canvas, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
*/
	}

	//____ _transitioningStates() ______________________________________________

	Bitmask<uint8_t> BlockSlideSkin::_transitioningStates() const
	{
		Bitmask<uint8_t> states;
		states.setBit(int(m_slideState));

		return states;
	}

	//____ _transitionTimes() __________________________________________________

	const int* BlockSlideSkin::_transitionTimes() const
	{
		return m_transitionTimes;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX BlockSlideSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_bStateOpaque[state] )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _updateOpaqueFlags() ________________________________________________

	void BlockSlideSkin::_updateOpaqueFlags()
	{
		bool bOpaque = false;
		bool bTintDecides = false;

		if (!m_pSurface)
			bOpaque = false;
		else if (m_blendMode == BlendMode::Replace)
			bOpaque = true;
		else if (!m_gradient.isUndefined() && !m_gradient.isOpaque())
			bOpaque = false;
		else if (m_blendMode == BlendMode::Blend)
		{
			bOpaque = m_pSurface->isOpaque();
			bTintDecides = bOpaque;
		}
		else
			bOpaque = false;

		if (bTintDecides)
		{
			for (int i = 0; i < State::NbStates; i++)
				m_bStateOpaque[i] = _getColor((StateEnum)i).a == 4096;
		}
		else
		{
			for (int i = 0; i < State::NbStates; i++)
				m_bStateOpaque[i] = bOpaque;
		}
	}

}
