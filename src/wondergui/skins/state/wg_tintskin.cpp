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
#include <wg_tintskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

#include <cstring>

namespace wg
{

	using namespace Util;

	const TypeInfo TintSkin::TYPEINFO = { "TintSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	TintSkin_p TintSkin::create( const Blueprint& blueprint )
	{
		return TintSkin_p(new TintSkin(blueprint));
	}

	TintSkin_p TintSkin::create(Tint * pTint, Border padding )
	{
		Blueprint bp;
		bp.tint = pTint;
		bp.padding = padding;

		return TintSkin_p(new TintSkin(bp));
	}

	//____ constructor ____________________________________________________________

	TintSkin::TintSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;

		// Generate lists of states that affects shift and color.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	tintStates[State::NbStates];
		Tint_p stateTints[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbTintStates = 1;

		shiftingStates[0] = State::Default;
		tintStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateTints[0] = bp.tint;

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

			if(stateInfo.data.tint )
			{
				int index = stateInfo.state == State::Default ? 0 : nbTintStates++;
				tintStates[index] = stateInfo.state;
				stateTints[index] = stateInfo.data.tint;
			}
		}

		// Calc size of index table for color, get its index masks & shifts.

		int	tintIndexEntries;

		std::tie(tintIndexEntries,m_stateTintIndexMask,m_stateTintIndexShift) = calcStateToIndexParam(nbTintStates, tintStates);

		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int tintBytes	= sizeof(Tint_p) * nbTintStates;
		int indexBytes		= tintIndexEntries;

		// Allocate and populate memory for state data

		m_pStateData = malloc(shiftBytes + tintBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		memset(pDest,0,tintBytes);				// Need to clear what will become tint pointers.

		auto pTints = (Tint_p*) pDest;
		for( int i = 0 ; i < nbTintStates ; i++ )
			pTints[i] = stateTints[i];

		m_pStateTints = pTints;
		m_nbStateTints = nbTintStates;

		pDest += tintBytes;

		m_pStateTintIndexTab = pDest;

		generateStateToIndexTab(m_pStateTintIndexTab, nbTintStates, tintStates);
	}

	//____ destructor ____________________________________________________________

	TintSkin::~TintSkin()
	{
		// Need to dereference Tints before we release the memory

		for( int i = 0 ; i < m_nbStateTints ; i++ )
			m_pStateTints[i] = nullptr;

		free( m_pStateData );
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& TintSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX TintSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( (_getTint(state)->isOpaque() && m_blendMode == BlendMode::Blend) || m_blendMode == BlendMode::Replace )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _render() _______________________________________________________________

	void TintSkin::_render( GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RectSPX rect = canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		auto pTint = _getTint(state);

		RenderSettingsWithTint settings(pDevice, m_layer, m_blendMode, HiColor::Undefined, rect, pTint);

		pDevice->fill( rect, HiColor::White );
	}

	//____ _markTest() _____________________________________________________________

	bool TintSkin::_markTest( const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if( !canvas.contains(ofs) )
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		return (_getTint(state)->alpha(ofs, canvas) >= alpha);		
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX TintSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState,
		float newValue, float oldValue, float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		if (_getTint(newState) != _getTint(oldState))
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}




} // namespace wg
