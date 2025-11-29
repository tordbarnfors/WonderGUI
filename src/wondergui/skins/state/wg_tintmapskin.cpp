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
#include <wg_tintmapskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo TintmapSkin::TYPEINFO = { "TintmapSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	TintmapSkin_p TintmapSkin::create( const Blueprint& blueprint )
	{
		return TintmapSkin_p(new TintmapSkin(blueprint));
	}

	TintmapSkin_p TintmapSkin::create(Tintmap * pTintmap, Border padding )
	{
		Blueprint bp;
		bp.tintmap = pTintmap;
		bp.padding = padding;

		return TintmapSkin_p(new TintmapSkin(bp));
	}

	//____ constructor ____________________________________________________________

	TintmapSkin::TintmapSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;

		// Generate lists of states that affects shift and color.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	tintmapStates[State::NbStates];
		Tintmap_p stateTintmaps[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbTintmapStates = 1;

		shiftingStates[0] = State::Default;
		tintmapStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateTintmaps[0] = bp.tintmap;

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

			if(stateInfo.data.tintmap )
			{
				int index = stateInfo.state == State::Default ? 0 : nbTintmapStates++;
				tintmapStates[index] = stateInfo.state;
				stateTintmaps[index] = stateInfo.data.tintmap;
			}
		}

		// Calc size of index table for color, get its index masks & shifts.

		int	tintmapIndexEntries;

		std::tie(tintmapIndexEntries,m_stateTintmapIndexMask,m_stateTintmapIndexShift) = calcStateToIndexParam(nbTintmapStates, tintmapStates);

		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int tintmapBytes	= sizeof(Tintmap_p) * nbTintmapStates;
		int indexBytes		= tintmapIndexEntries;

		// Allocate and populate memory for state data

		m_pStateData = malloc(shiftBytes + tintmapBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		memset(pDest,0,tintmapBytes);				// Need to clear what will become tintmap pointers.

		auto pTintmaps = (Tintmap_p*) pDest;
		for( int i = 0 ; i < nbTintmapStates ; i++ )
			pTintmaps[i] = stateTintmaps[i];

		m_pStateTintmaps = pTintmaps;

		pDest += tintmapBytes;

		m_pStateTintmapIndexTab = pDest;

		generateStateToIndexTab(m_pStateTintmapIndexTab, nbTintmapStates, tintmapStates);
	}

	//____ destructor ____________________________________________________________

	TintmapSkin::~TintmapSkin()
	{
		// Need to dereference Tintmaps before we release the memory

		for( int i = 0 ; i < m_nbStateTintmaps ; i++ )
			m_pStateTintmaps[i] = nullptr;

		free( m_pStateData );
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& TintmapSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX TintmapSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( (_getTintmap(state)->isOpaque() && m_blendMode == BlendMode::Blend) || m_blendMode == BlendMode::Replace )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _render() _______________________________________________________________

	void TintmapSkin::_render( GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RectSPX rect = canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		auto pTintmap = _getTintmap(state);

		RenderSettingsWithTintmap settings(pDevice, m_layer, m_blendMode, HiColor::Undefined, rect, pTintmap);

		pDevice->fill( rect, HiColor::White );
	}

	//____ _markTest() _____________________________________________________________

	bool TintmapSkin::_markTest( const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if( !canvas.contains(ofs) )
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		return (_getTintmap(state)->alpha(ofs, canvas) >= alpha);		//TODO: Add method to Tintmap for markTesting.
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX TintmapSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState,
		float newValue, float oldValue, float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		if (_getTintmap(newState) != _getTintmap(oldState))
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}




} // namespace wg
