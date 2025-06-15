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

#include <wg_blockskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo BlockSkin::TYPEINFO = { "BlockSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	BlockSkin_p BlockSkin::create(const Blueprint& blueprint)
	{
		return BlockSkin_p(new BlockSkin(blueprint));
	}


	BlockSkin_p BlockSkin::create(Surface * pSurface, Border frame )
	{
		BorderI pixelFrame = roundToPixels(ptsToSpx(frame,pSurface->scale()));

		if (pSurface == nullptr || pixelFrame.width() >= pSurface->pixelSize().w || pixelFrame.height() >= pSurface->pixelSize().h  )
			return nullptr;

		return BlockSkin_p( new BlockSkin(pSurface, pSurface->pointSize(), frame) );
	}

	BlockSkin_p BlockSkin::create(Surface * pSurface, Rect block, Border frame)
	{
		if (pSurface == nullptr)
			return nullptr;

		SizeI surfSize = pSurface->pixelSize();
		BorderI pixelFrame = roundToPixels(ptsToSpx(frame, pSurface->scale()));
		RectI pixelBlock = roundToPixels(ptsToSpx(block, pSurface->scale()));

		if( pixelFrame.width() >= surfSize.w || pixelFrame.height() >= surfSize.h ||
			pixelBlock.x < 0 || pixelBlock.y < 0 || pixelBlock.right() > surfSize.w || pixelBlock.bottom() > surfSize.h )
			return nullptr;

		return BlockSkin_p(new BlockSkin(pSurface, block, frame));
	}

	BlockSkin_p	BlockSkin::create(Surface * pSurface, Rect _firstBlock, std::initializer_list<State> stateBlocks, Border _frame, Axis axis, pts _spacing)
	{
		if (pSurface == nullptr || stateBlocks.size() < 1 )
			return nullptr;

		// Get pixel values

		int scale = pSurface->scale();

		SizeI surfSize = pSurface->pixelSize();
		RectI firstBlock = roundToPixels(ptsToSpx(_firstBlock, scale));
		BorderI frame = roundToPixels(ptsToSpx(_frame, scale));
		int   spacing = roundToPixels(ptsToSpx(_spacing, scale));

		// Check so all blocks fit

		int nBlocks = (int) stateBlocks.size();
		RectI blockArea = firstBlock;
		if (axis == Axis::X)
			blockArea.w += (nBlocks-1) * (firstBlock.w + spacing);
		else
			blockArea.h += (nBlocks - 1) * (firstBlock.h + spacing);

		if (frame.width() >= firstBlock.w || frame.height() >= firstBlock.h ||
			blockArea.x < 0 || blockArea.y < 0 || blockArea.right() > surfSize.w || blockArea.bottom() > surfSize.h)
			return nullptr;

		// Create the skin

		Blueprint bp;
		bp.surface 	= pSurface;
		bp.frame	= _frame;
		bp.axis		= axis,
		bp.blockSpacing	= _spacing;
		bp.firstBlock = _firstBlock;

		for( auto state : stateBlocks )
			bp.states.push_back(state);

		BlockSkin * p = new BlockSkin(bp);
		return BlockSkin_p(p);
	}

	BlockSkin_p BlockSkin::create(Surface * pSurface, std::initializer_list<State> stateBlocks, Border _frame, Axis axis, pts _spacing)
	{
		if (pSurface == nullptr || stateBlocks.size() < 1)
			return nullptr;

		int scale = pSurface->scale();

		SizeI	surfSize = pSurface->pixelSize();
		int		nBlocks = (int) stateBlocks.size();
		BorderI frame = roundToPixels(ptsToSpx(_frame, scale));
		int   spacing = roundToPixels(ptsToSpx(_spacing, scale));

		// Check so blocks fit evenly

		int		length = axis == Axis::X ? surfSize.w : surfSize.h;

		if ((length - (nBlocks - 1) * spacing) % nBlocks != 0)
			return nullptr;

		// Create the skin

		int blockLen = (length - (nBlocks - 1) * spacing) / nBlocks;

		SizeI blockSize = axis == Axis::X ? SizeI(blockLen, surfSize.h) : SizeI(surfSize.w, blockLen);

		Blueprint bp;
		bp.surface 	= pSurface;
		bp.frame	= _frame;
		bp.axis		= axis,
		bp.blockSpacing	= _spacing;
		bp.firstBlock = spxToPts(blockSize*64, scale);

		for( auto state : stateBlocks )
			bp.states.push_back(state);

		BlockSkin * p = new BlockSkin(bp);
		return BlockSkin_p(p);
	}

	BlockSkin_p BlockSkin::createStatic( Surface * pSurface, Rect block, Border frame )
	{
		return create(pSurface, block, frame);
	}


	BlockSkin_p BlockSkin::createClickable( Surface * pSurface, Size blockGeo, Coord blockStartOfs, Size blockPitch, Border blockFrame )
	{
		Axis axis;
		int spacing;
		if (blockPitch.w > 0)
		{
			axis = Axis::X;
			spacing = blockPitch.w - blockGeo.w;
		}
		else
		{
			axis = Axis::Y;
			spacing = blockPitch.h - blockGeo.h;
		}

		return BlockSkin::create(pSurface, Rect(blockStartOfs,blockGeo), { State::Default, State::Hovered, State::Pressed, State::Disabled }, blockFrame, axis, spacing);
	}


	BlockSkin_p BlockSkin::createStaticFromSurface( Surface * pSurface, Border frame )
	{
		return create(pSurface, frame);
	}

	//____ constructor ____________________________________________________________


	BlockSkin::BlockSkin(Surface * pSurface, Rect block, Border frame)
	{

		m_pSurface			= pSurface;
		m_ninePatch.block.setSize( block.size() );
		m_ninePatch.frame	= frame;
		bool bOpaque		= pSurface->isOpaque();

		// Calc size of index table for block and color, get their index masks & shifts.

		int	blockIndexEntries = 1;
		int	colorIndexEntries = 1;

		m_blockIndexMask = 0;
		m_blockIndexShift = 0;

		m_colorIndexMask = 0;
		m_colorIndexShift = 0;

		// Calculate memory needed for all state data

		int blockBytes		= sizeof(Coord);
		int colorBytes		= sizeof(HiColor);
		int indexBytes		= 1;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(blockBytes + colorBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pBlocks = (Coord*) pDest;
		pBlocks[0] = block.pos();

		m_pBlocks = pBlocks;
		pDest += blockBytes;

		auto pColors = (HiColor*) pDest;
		pColors[0] = HiColor::White;

		m_pColors = pColors;

		pDest += colorBytes;

		m_pBlockIndexTab = pDest;
		m_pColorIndexTab = pDest;

		m_pBlockIndexTab[0] = 0;

		for( int i = 0 ; i < State::NbStates ; i++ )
			m_bStateOpaque[i] = bOpaque;
	}

	BlockSkin::BlockSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_pSurface = bp.surface;
		m_ninePatch.frame = bp.frame;
		m_blendMode = bp.blendMode;
		m_gradient = bp.gradient;

		// Make sure we have a correct block

		Rect block = bp.firstBlock;

		if (block.isEmpty())
		{
			int nStateBlocks = 1;
			for ( auto& entry : bp.states )
			{
				if( entry.state != State::Default && entry.data.blockless == false)
					nStateBlocks++;
			}


			if (bp.axis == Axis::X)
			{
				block.w = (m_pSurface->pointWidth() - bp.blockSpacing * (nStateBlocks - 1)) / nStateBlocks;
				block.h = m_pSurface->pointHeight();
			}
			else
			{
				block.w = m_pSurface->pointWidth();
				block.h = (m_pSurface->pointHeight() - bp.blockSpacing * (nStateBlocks - 1)) / nStateBlocks;
			}
		}

		m_ninePatch.block.setSize(block.size());

		// RigidPartX

		{
			pts ofs = bp.rigidPartX.begin;
			pts length = bp.rigidPartX.length;

			pts	midSecLen = m_ninePatch.block.w - m_ninePatch.frame.width();
			ofs -= m_ninePatch.frame.left;

			if (ofs < 0)
			{
				length += ofs;
				ofs = 0;
			}

			if (ofs + length > midSecLen)
				length = midSecLen - ofs;

			m_ninePatch.rigidPartXOfs = ofs;
			m_ninePatch.rigidPartXLength = length;
			m_ninePatch.rigidPartXSections = bp.rigidPartX.sections;
		}

		// RigidPartY

		{
			pts ofs = bp.rigidPartY.begin;
			pts length = bp.rigidPartY.length;
			
			pts	midSecLen = m_ninePatch.block.h - m_ninePatch.frame.height();
			ofs -= m_ninePatch.frame.top;

			if (ofs < 0)
			{
				length += ofs;
				ofs = 0;
			}

			if (ofs + length > midSecLen)
				length = midSecLen - ofs;

			m_ninePatch.rigidPartYOfs = ofs;
			m_ninePatch.rigidPartYLength = length;
			m_ninePatch.rigidPartYSections = bp.rigidPartY.sections;
		}

		// Generate lists of states that affects shift, color and surface.

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
		Coord pitch = bp.axis == Axis::X ? Coord(block.w + bp.blockSpacing, 0) : Coord(0, block.h + bp.blockSpacing);

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

	//____ destructor _________________________________________________________

	BlockSkin::~BlockSkin()
	{
		free(m_pBlocks);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BlockSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ render() _______________________________________________________________

	void BlockSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		if( !m_pSurface )
			return;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		RenderSettingsWithGradient settings(pDevice, m_layer, m_blendMode, _getColor(state), canvas, m_gradient);

		pDevice->setBlitSource(m_pSurface);

		NinePatch	patch = m_ninePatch;
		patch.block.setPos(_getBlock(state));

		pDevice->blitNinePatch(canvas, align(ptsToSpx(patch.frame,scale)), patch, scale);
	}

	//____ _minSize() ______________________________________________________________

	SizeSPX BlockSkin::_minSize(int scale) const
	{
		SizeSPX content = align(ptsToSpx(m_padding,scale));
		SizeSPX frame = align(ptsToSpx(m_ninePatch.frame,scale));
		return SizeSPX::max( content, frame ) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX BlockSkin::_defaultSize(int scale) const
	{
        //This takes the scale of the surface into account
        // Default size is when each point of the surface maps to a point of the skinarea.
        
		return align(ptsToSpx(m_ninePatch.block.size(),scale)) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _sizeForContent() _______________________________________________________

	SizeSPX BlockSkin::_sizeForContent( const SizeSPX& contentSize, int scale ) const
	{
		SizeSPX sz = StateSkin::_sizeForContent(contentSize,scale);
		SizeSPX min = align(ptsToSpx(m_ninePatch.frame,scale));

		return SizeSPX::max(sz, min) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _markTest() _____________________________________________________________

	bool BlockSkin::_markTest( const CoordSPX& _ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		//TODO: Take blendMode and tint (incl gradient) into account.

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if( !canvas.contains(_ofs) )
			return false;
		
		canvas += align(ptsToSpx(m_overflow, scale));
		
		NinePatch	patch = m_ninePatch;
		patch.block.setPos(_getBlock(state));

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		return markTestNinePatch(_ofs, m_pSurface, patch, canvas, scale, alpha);
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX BlockSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		if (_getBlock(newState) != _getBlock(oldState))
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX BlockSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_bStateOpaque[state] )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _updateOpaqueFlags() ________________________________________________

	void BlockSkin::_updateOpaqueFlags()
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

} // namespace wg
