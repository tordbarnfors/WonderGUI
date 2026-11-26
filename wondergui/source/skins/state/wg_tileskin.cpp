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
#include <wg_tileskin.h>
#include <wax_device.h>
#include <wax_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo TileSkin::TYPEINFO = { "TileSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________


	TileSkin_p TileSkin::create(Bitmap * pBitmap, Color color, int layer )
	{
		if (pBitmap == nullptr || !pBitmap->isTiling() )
			return nullptr;

		Blueprint bp;
		bp.bitmap = pBitmap;
		bp.color = color;
		bp.layer = layer;

		return TileSkin_p( new TileSkin(bp) );
	}

	TileSkin_p TileSkin::create(const Blueprint& blueprint)
	{
		if (blueprint.bitmap == nullptr || !blueprint.bitmap->isTiling())
			return nullptr;


		return TileSkin_p(new TileSkin(blueprint));
	}


	//____ constructor ____________________________________________________________

	TileSkin::TileSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;
		m_pTint		= bp.tint;

		// Generate lists of states that affects shift, color and bitmap.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		Color stateColors[State::NbStates];

		State	bitmapStates[State::NbStates];
		Bitmap * stateBitmaps[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbColorStates = 1;
		int		nbBitmapStates = 1;

		shiftingStates[0] = State::Default;
		colorStates[0] = State::Default;
		bitmapStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateColors[0] = bp.color;
		stateBitmaps[0] = bp.bitmap;

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

			if (stateInfo.data.bitmap)
			{
				int index = stateInfo.state == State::Default ? 0 : nbBitmapStates++;
				bitmapStates[index] = stateInfo.state;
				stateBitmaps[index] = stateInfo.data.bitmap;
			}

			if(stateInfo.data.color != Color::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbColorStates++;
				colorStates[index] = stateInfo.state;
				stateColors[index] = stateInfo.data.color;
			}
		}

		// Calc size of index table for bitmap and color, get their index masks & shifts.

		int	bitmapIndexEntries;
		int	colorIndexEntries;

		std::tie(bitmapIndexEntries,m_stateBitmapIndexMask,m_stateBitmapIndexShift) = calcStateToIndexParam(nbBitmapStates, bitmapStates);
		std::tie(colorIndexEntries,m_stateColorIndexMask,m_stateColorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);


		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int bitmapBytes	= sizeof(Bitmap*) * nbBitmapStates;
		int colorBytes		= sizeof(Color) * nbColorStates;
		int indexBytes		= bitmapIndexEntries+colorIndexEntries;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(shiftBytes + bitmapBytes + colorBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		auto pBitmaps = (Bitmap**) pDest;
		for( int i = 0 ; i < nbBitmapStates ; i++ )
		{
			pBitmaps[i] = stateBitmaps[i];
			pBitmaps[i]->retain();
		}

		m_pStateBitmaps = pBitmaps;
		m_nbStateBitmaps = nbBitmapStates;

		pDest += bitmapBytes;

		auto pColors = (Color*) pDest;
		for( int i = 0 ; i < nbColorStates ; i++ )
			pColors[i] = stateColors[i];

		m_pStateColors = pColors;

		pDest += colorBytes;

		m_pStateBitmapIndexTab = pDest;
		m_pStateColorIndexTab = pDest + bitmapIndexEntries;

		generateStateToIndexTab(m_pStateBitmapIndexTab, nbBitmapStates, bitmapStates);
		generateStateToIndexTab(m_pStateColorIndexTab, nbColorStates, colorStates);
	}

	//____ destructor ____________________________________________________________

	TileSkin::~TileSkin()
	{
		for( int i = 0 ; i < m_nbStateBitmaps ; i++ )
			m_pStateBitmaps[i]->release();

		free(m_pStateData);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& TileSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ _render() _______________________________________________________________

	void TileSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float * pStateFractions) const
	{
		int idx = state;

		Bitmap * pBitmap = _getBitmap(state);

		if( !pBitmap )
			return;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		RenderSettingsWithTint settings(pDevice, m_layer, m_blendMode, _getColor(state), canvas, m_pTint);

		pDevice->setBlitSource(pBitmap);
		pDevice->scaleTile(canvas,scale/64.f);
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX TileSkin::_defaultSize(int scale) const
	{
		SizeSPX content = align(ptsToSpx(m_padding,scale));
		SizeSPX bitmap;

		Bitmap * pBitmap = m_pStateBitmaps[0];
		if (pBitmap)
			bitmap = align(ptsToSpx(pBitmap->pointSize(),scale));

		return SizeSPX::max(content, bitmap) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _markTest() _____________________________________________________________

	bool TileSkin::_markTest( const CoordSPX& _ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		//TODO: Take gradient and tintColor into account.

		Bitmap * pBitmap = _getBitmap(state);

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));
		
		if( !canvas.contains(_ofs) )
			return false;
		
		canvas += align(ptsToSpx(m_overflow, scale));

		return markTestTileRect(_ofs, pBitmap, canvas, scale, alpha);
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX TileSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if(_getBitmap(oldState) != _getBitmap(newState))
			return canvas;
		
		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2, 
									newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX TileSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		Bitmap * pBitmap = _getBitmap(state);
		const Color& color = _getColor(state);

		if (color.a == 4096 && pBitmap->isOpaque())
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

} // namespace wg
