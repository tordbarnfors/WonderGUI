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

#include <wg_colorskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo ColorSkin::TYPEINFO = { "ColorSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	ColorSkin_p ColorSkin::create( const Blueprint& blueprint )
	{
		return ColorSkin_p(new ColorSkin(blueprint));
	}

	ColorSkin_p ColorSkin::create(HiColor color, Border padding )
	{
		Blueprint bp;
		bp.color = color;
		bp.padding = padding;

		return ColorSkin_p(new ColorSkin(bp));
	}

	//____ constructor ____________________________________________________________

	ColorSkin::ColorSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;

		// Generate lists of states that affects shift and color.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		HiColor stateColors[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbColorStates = 1;

		shiftingStates[0] = State::Default;
		colorStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateColors[0] = bp.color;

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

			if(stateInfo.data.color != HiColor::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbColorStates++;
				colorStates[index] = stateInfo.state;
				stateColors[index] = stateInfo.data.color;
			}
		}

		// Calc size of index table for color, get its index masks & shifts.

		int	colorIndexEntries;

		std::tie(colorIndexEntries,m_stateColorIndexMask,m_stateColorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);

		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int colorBytes		= sizeof(HiColor) * nbColorStates;
		int indexBytes		= colorIndexEntries;

		// Allocate and populate memory for state data

		m_pStateData = malloc(shiftBytes + colorBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		auto pColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbColorStates ; i++ )
			pColors[i] = stateColors[i];

		m_pStateColors = pColors;

		pDest += colorBytes;

		m_pStateColorIndexTab = pDest;

		generateStateToIndexTab(m_pStateColorIndexTab, nbColorStates, colorStates);
	}

	//____ destructor ____________________________________________________________

	ColorSkin::~ColorSkin()
	{
		free( m_pStateData );
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& ColorSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX ColorSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( (_getColor(state).a == 4096 && m_blendMode == BlendMode::Blend) || m_blendMode == BlendMode::Replace )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _render() _______________________________________________________________

	void ColorSkin::_render( GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RenderSettings settings(pDevice, m_layer, m_blendMode);

		int i = state;
		pDevice->fill( canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale)), _getColor(state) );
	}

	//____ _markTest() _____________________________________________________________

	bool ColorSkin::_markTest( const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if( !canvas.contains(ofs) )
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		return ( _getColor(state).a >= alpha);
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX ColorSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState,
		float newValue, float oldValue, float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		if (_getColor(newState) != _getColor(oldState))
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}




} // namespace wg
