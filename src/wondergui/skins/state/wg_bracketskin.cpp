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

#include <wg_bracketskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo BracketSkin::TYPEINFO = { "BracketSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	BracketSkin_p BracketSkin::create( const Blueprint& blueprint )
	{
		return BracketSkin_p(new BracketSkin(blueprint));
	}

	//____ constructor ____________________________________________________________

	BracketSkin::BracketSkin( const Blueprint& bp ) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;
		m_thickness		= bp.thickness;
		m_size			= bp.size;

		if (m_size.w < m_thickness)
			m_size.w = m_thickness;

		if (m_size.h < m_thickness)
			m_size.h = m_thickness;

		// Generate lists of states that affects shift and color.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		HiColor stateColors[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbColorStates = 1;

		shiftingStates[0] = State::Default;
		colorStates[0] = State::Default;

		stateShifts[0] = { 0,0 };
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

			if (stateInfo.data.color != HiColor::Undefined)
			{
				int index = stateInfo.state == State::Default ? 0 : nbColorStates++;
				colorStates[index] = stateInfo.state;
				stateColors[index] = stateInfo.data.color;
			}
		}

		// Calc size of index table for color, get its index masks & shifts.

		int	colorIndexEntries;

		std::tie(colorIndexEntries, m_colorIndexMask, m_colorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);

		// Calculate memory needed for all state data

		int shiftBytes = _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int colorBytes = sizeof(HiColor) * nbColorStates;
		int indexBytes = colorIndexEntries;

		// Allocate and populate memory for state data

		m_pStateData = malloc(shiftBytes + colorBytes + indexBytes);

		auto pDest = (uint8_t*)m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for (int i = 0; i < nbShiftingStates; i++)
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		auto pColors = (HiColor*)pDest;
		for (int i = 0; i < nbColorStates; i++)
			pColors[i] = stateColors[i];

		m_pColors = pColors;

		pDest += colorBytes;

		m_pColorIndexTab = pDest;

		generateStateToIndexTab(m_pColorIndexTab, nbColorStates, colorStates);
	}

	//____ destructor() _______________________________________________________

	BracketSkin::~BracketSkin()
	{
		free(m_pStateData);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& BracketSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() _______________________________________________________________

	void BracketSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		//TODO: Optimize! Clip patches against canvas first.

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		RenderSettings settings(pDevice, m_layer, m_blendMode);


		spx thickness = align(ptsToSpx(m_thickness, scale));
		SizeSPX size = align(ptsToSpx(m_size, scale));

		const HiColor& color = _getColor(state);

		if (thickness*2 >= canvas.w || thickness*2 >= canvas.h)
		{
			pDevice->fill(canvas, color); 
		}
		else
		{
			if (size.w * 2 >= canvas.w)
			{
				pDevice->fill({ canvas.x, canvas.y, canvas.w, thickness }, color);
				pDevice->fill({ canvas.x, canvas.y + canvas.h - thickness, canvas.w, thickness }, color);
			}
			else
			{
				pDevice->fill({ canvas.x, canvas.y, size.w, thickness }, color);
				pDevice->fill({ canvas.x + canvas.w - size.w, canvas.y, size.w, thickness }, color);

				pDevice->fill({ canvas.x, canvas.y + canvas.h - thickness, size.w, thickness }, color);
				pDevice->fill({ canvas.x + canvas.w - size.w, canvas.y + canvas.h - thickness, size.w, thickness }, color);
			}

			if (size.h * 2 >= canvas.h)
			{
				pDevice->fill({ canvas.x, canvas.y + thickness, thickness, canvas.h - thickness*2  }, color);
				pDevice->fill({ canvas.x + canvas.w - thickness, canvas.y + thickness, thickness, canvas.h - thickness*2 }, color);
			}
			else
			{
				pDevice->fill({ canvas.x, canvas.y + thickness, thickness, size.h - thickness }, color);
				pDevice->fill({ canvas.x + canvas.w - thickness, canvas.y + thickness, thickness, size.h - thickness }, color);

				pDevice->fill({ canvas.x, canvas.y + canvas.h - size.h, thickness, size.h - thickness }, color);
				pDevice->fill({ canvas.x + canvas.w - thickness, canvas.y + canvas.h - size.h, thickness, size.h - thickness }, color);
			}
		}
	}

	//____ _minSize() ______________________________________________________________

	SizeSPX BracketSkin::_minSize(int scale) const
	{
		SizeSPX content = StateSkin::_minSize(scale);
		spx thickness	= align(ptsToSpx(m_thickness,scale));

		return SizeSPX::max(content,SizeSPX(thickness,thickness)) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX BracketSkin::_defaultSize(int scale) const
	{
		SizeSPX content = StateSkin::_minSize(scale);
		spx thickness = align(ptsToSpx(m_thickness, scale));

		return SizeSPX::max(content, SizeSPX(thickness,thickness)) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _sizeForContent() _______________________________________________________

	SizeSPX BracketSkin::_sizeForContent( const SizeSPX& contentSize, int scale ) const
	{
		SizeSPX content = StateSkin::_sizeForContent(contentSize,scale);
		spx thickness = align(ptsToSpx(m_thickness, scale));

		return SizeSPX::max(content, SizeSPX(thickness, thickness)) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _markTest() _____________________________________________________________

	bool BracketSkin::_markTest( const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		// We don't mark test brackets.

		return false;
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX BracketSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if (_getColor(newState) != _getColor(oldState) )
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX BracketSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_blendMode == BlendMode::Replace )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));

		return RectSPX();
	}

} // namespace wg
