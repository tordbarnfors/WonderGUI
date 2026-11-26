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
#include <wg_blockskin.h>
#include <wax_device.h>
#include <wax_geo.h>
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


	BlockSkin_p BlockSkin::create(Bitmap * pBitmap, Border frame )
	{
		BorderI pixelFrame = roundToPixels(ptsToSpx(frame,pBitmap->scale()));

		if (pBitmap == nullptr || pixelFrame.width() >= pBitmap->pixelSize().w || pixelFrame.height() >= pBitmap->pixelSize().h  )
			return nullptr;

		return BlockSkin_p( new BlockSkin(pBitmap, pBitmap->pointSize(), frame) );
	}

	BlockSkin_p BlockSkin::create(Bitmap * pBitmap, Rect block, Border frame)
	{
		if (pBitmap == nullptr)
			return nullptr;

		SizeI bitmapSize = pBitmap->pixelSize();
		BorderI pixelFrame = roundToPixels(ptsToSpx(frame, pBitmap->scale()));
		RectI pixelBlock = roundToPixels(ptsToSpx(block, pBitmap->scale()));

		if( pixelFrame.width() >= bitmapSize.w || pixelFrame.height() >= bitmapSize.h ||
			pixelBlock.x < 0 || pixelBlock.y < 0 || pixelBlock.right() > bitmapSize.w || pixelBlock.bottom() > bitmapSize.h )
			return nullptr;

		return BlockSkin_p(new BlockSkin(pBitmap, block, frame));
	}

	BlockSkin_p	BlockSkin::create(Bitmap * pBitmap, Rect _firstBlock, std::initializer_list<State> stateBlocks, Border _frame, Axis axis, pts _spacing)
	{
		if (pBitmap == nullptr || stateBlocks.size() < 1 )
			return nullptr;

		// Get pixel values

		int scale = pBitmap->scale();

		SizeI bitmapSize = pBitmap->pixelSize();
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
			blockArea.x < 0 || blockArea.y < 0 || blockArea.right() > bitmapSize.w || blockArea.bottom() > bitmapSize.h)
			return nullptr;

		// Create the skin

		Blueprint bp;
		bp.bitmap 	= pBitmap;
		bp.frame	= _frame;
		bp.axis		= axis,
		bp.blockSpacing	= _spacing;
		bp.firstBlock = _firstBlock;

		for( auto state : stateBlocks )
			bp.states.push_back(state);

		BlockSkin * p = new BlockSkin(bp);
		return BlockSkin_p(p);
	}

	BlockSkin_p BlockSkin::create(Bitmap * pBitmap, std::initializer_list<State> stateBlocks, Border _frame, Axis axis, pts _spacing)
	{
		if (pBitmap == nullptr || stateBlocks.size() < 1)
			return nullptr;

		int scale = pBitmap->scale();

		SizeI	bitmapSize = pBitmap->pixelSize();
		int		nBlocks = (int) stateBlocks.size();
		BorderI frame = roundToPixels(ptsToSpx(_frame, scale));
		int   spacing = roundToPixels(ptsToSpx(_spacing, scale));

		// Check so blocks fit evenly

		int		length = axis == Axis::X ? bitmapSize.w : bitmapSize.h;

		if ((length - (nBlocks - 1) * spacing) % nBlocks != 0)
			return nullptr;

		// Create the skin

		int blockLen = (length - (nBlocks - 1) * spacing) / nBlocks;

		SizeI blockSize = axis == Axis::X ? SizeI(blockLen, bitmapSize.h) : SizeI(bitmapSize.w, blockLen);

		Blueprint bp;
		bp.bitmap 	= pBitmap;
		bp.frame	= _frame;
		bp.axis		= axis,
		bp.blockSpacing	= _spacing;
		bp.firstBlock = spxToPts(blockSize*64, scale);

		for( auto state : stateBlocks )
			bp.states.push_back(state);

		BlockSkin * p = new BlockSkin(bp);
		return BlockSkin_p(p);
	}

	BlockSkin_p BlockSkin::createStatic( Bitmap * pBitmap, Rect block, Border frame )
	{
		return create(pBitmap, block, frame);
	}


	BlockSkin_p BlockSkin::createClickable( Bitmap * pBitmap, Size blockGeo, Coord blockStartOfs, Size blockPitch, Border blockFrame )
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

		return BlockSkin::create(pBitmap, Rect(blockStartOfs,blockGeo), { State::Default, State::Hovered, State::Pressed, State::Disabled }, blockFrame, axis, spacing);
	}


	BlockSkin_p BlockSkin::createStaticFromBitmap( Bitmap * pBitmap, Border frame )
	{
		return create(pBitmap, frame);
	}

	//____ constructor ____________________________________________________________


	BlockSkin::BlockSkin(Bitmap * pBitmap, Rect block, Border frame)
	{

		m_pBitmap			= pBitmap;
		m_ninePatch.block.setSize( block.size() );
		m_ninePatch.frame	= frame;
		bool bOpaque		= pBitmap->isOpaque();

		// Calc size of index table for block and color, get their index masks & shifts.

		int	blockIndexEntries = 1;
		int	colorIndexEntries = 1;

		m_blockIndexMask = 0;
		m_blockIndexShift = 0;

		m_colorIndexMask = 0;
		m_colorIndexShift = 0;

		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(1, &State::Default);
		int blockBytes		= sizeof(Coord);
		int colorBytes		= sizeof(Color);
		int indexBytes		= 1;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(shiftBytes + blockBytes + colorBytes + indexBytes);
		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, 1, &State::Default);
		pCoords[0] = {0,0};
		pDest += shiftBytes;

		auto pBlocks = (Coord*) pDest;
		pBlocks[0] = block.pos();

		m_pBlocks = pBlocks;
		pDest += blockBytes;

		auto pColors = (Color*) pDest;
		pColors[0] = Color::White;

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
		m_pBitmap = bp.bitmap;
		m_ninePatch.frame = bp.frame;
		m_blendMode = bp.blendMode;
		m_pTint = bp.tint;

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
				block.w = (m_pBitmap->pointWidth() - bp.blockSpacing * (nStateBlocks - 1)) / nStateBlocks;
				block.h = m_pBitmap->pointHeight();
			}
			else
			{
				block.w = m_pBitmap->pointWidth();
				block.h = (m_pBitmap->pointHeight() - bp.blockSpacing * (nStateBlocks - 1)) / nStateBlocks;
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

		// Generate lists of states that affects shift, color and block.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		Color stateColors[State::NbStates];

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

			if(stateInfo.data.color != Color::Undefined )
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
		int colorBytes		= sizeof(Color) * nbColorStates;
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

		auto pColors = (Color*) pDest;
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
		free(m_pStateData);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BlockSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ render() _______________________________________________________________

	void BlockSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		if( !m_pBitmap )
			return;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		RenderSettingsWithTint settings(pDevice, m_layer, m_blendMode, _getColor(state), canvas, m_pTint);

		pDevice->setBlitSource(m_pBitmap);

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
        //This takes the scale of the bitmap into account
        // Default size is when each point of the bitmap maps to a point of the skinarea.
      
		SizeSPX content = align(ptsToSpx(m_padding, scale));
		SizeSPX blockSize = align(ptsToSpx(m_ninePatch.block.size(), scale));

		SizeSPX def = { std::max(content.w, blockSize.w), std::max(content.h, blockSize.h) };

		return def + align(ptsToSpx(m_spacing, scale));
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

		return markTestNinePatch(_ofs, m_pBitmap, patch, canvas, scale, alpha);
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

		if (!m_pBitmap)
			bOpaque = false;
		else if (m_blendMode == BlendMode::Replace)
			bOpaque = true;
		else if (m_pTint && !m_pTint->isOpaque())
			bOpaque = false;
		else if (m_blendMode == BlendMode::Blend)
		{
			bOpaque = m_pBitmap->isOpaque();
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
