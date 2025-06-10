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

#include <wg_circleskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

#include <cmath>
#include <algorithm>

namespace wg
{
	using namespace Util;

	const TypeInfo CircleSkin::TYPEINFO = { "CircleSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	CircleSkin_p CircleSkin::create( const Blueprint& blueprint )
	{
		return CircleSkin_p(new CircleSkin( blueprint ));
	}


	//____ constructor ____________________________________________________________

	CircleSkin::CircleSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;

		// Generate lists of states that affects shift, color and surface.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		HiColor stateColors[State::NbStates];

		State	sizeStates[State::NbStates];
		float	stateSizes[State::NbStates];

		State	thicknessStates[State::NbStates];
		pts		stateThickness[State::NbStates];

		State	outlineColorStates[State::NbStates];
		HiColor stateOutlineColors[State::NbStates];

		State	outlineThicknessStates[State::NbStates];
		pts		stateOutlineThickness[State::NbStates];

		int		nbShiftingStates = 1;
		int		nbColorStates = 1;
		int		nbSizeStates = 1;
		int		nbThicknessStates = 1;
		int		nbOutlineColorStates = 1;
		int		nbOutlineThicknessStates = 1;

		shiftingStates[0] = State::Default;
		colorStates[0] = State::Default;
		sizeStates[0] = State::Default;
		thicknessStates[0] = State::Default;
		outlineColorStates[0] = State::Default;
		outlineThicknessStates[0] = State::Default;

		stateShifts[0] = { 0,0 };
		stateColors[0] = bp.color;
		stateSizes[0] = bp.size;
		stateThickness[0] = bp.thickness;
		stateOutlineColors[0] = bp.outlineColor;
		stateOutlineThickness[0] = bp.outlineThickness;

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

			if (stateInfo.data.size != -1)
			{
				int index = stateInfo.state == State::Default ? 0 : nbSizeStates++;
				sizeStates[index] = stateInfo.state;
				stateSizes[index] = stateInfo.data.size;
			}

			if (stateInfo.data.thickness != -1)
			{
				int index = stateInfo.state == State::Default ? 0 : nbThicknessStates++;
				thicknessStates[index] = stateInfo.state;
				stateThickness[index] = stateInfo.data.thickness;
			}

			if (stateInfo.data.outlineColor != HiColor::Undefined)
			{
				int index = stateInfo.state == State::Default ? 0 : nbOutlineColorStates++;
				outlineColorStates[index] = stateInfo.state;
				stateOutlineColors[index] = stateInfo.data.outlineColor;
			}

			if (stateInfo.data.outlineThickness != -1)
			{
				int index = stateInfo.state == State::Default ? 0 : nbOutlineThicknessStates++;
				outlineThicknessStates[index] = stateInfo.state;
				stateOutlineThickness[index] = stateInfo.data.outlineThickness;
			}

		}

		// Calc size of index table for surface and color, get their index masks & shifts.

		int	colorIndexEntries;
		int	sizeIndexEntries;
		int	thicknessIndexEntries;
		int	outlineColorIndexEntries;
		int	outlineThicknessIndexEntries;

		std::tie(colorIndexEntries, m_colorIndexMask, m_colorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);
		std::tie(sizeIndexEntries, m_sizeIndexMask, m_sizeIndexShift) = calcStateToIndexParam(nbSizeStates, sizeStates);
		std::tie(thicknessIndexEntries, m_thicknessIndexMask, m_thicknessIndexShift) = calcStateToIndexParam(nbThicknessStates, thicknessStates);

		std::tie(outlineColorIndexEntries, m_outlineColorIndexMask, m_outlineColorIndexShift) = calcStateToIndexParam(nbOutlineColorStates, outlineColorStates);
		std::tie(outlineThicknessIndexEntries, m_outlineThicknessIndexMask, m_outlineThicknessIndexShift) = calcStateToIndexParam(nbOutlineThicknessStates, outlineThicknessStates);

		// Calculate memory needed for all state data

		int shiftBytes = _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int colorBytes = sizeof(HiColor) * nbColorStates;
		int sizeBytes = sizeof(float) * nbSizeStates;
		int thicknessBytes = sizeof(pts) * nbThicknessStates;
		int outlineColorBytes = sizeof(HiColor) * nbOutlineColorStates;
		int outlineThicknessBytes = sizeof(pts) * nbOutlineThicknessStates;
		int indexBytes = colorIndexEntries + sizeIndexEntries + thicknessIndexEntries + outlineColorIndexEntries + outlineThicknessIndexEntries;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(shiftBytes + colorBytes + sizeBytes + thicknessBytes + outlineColorBytes + outlineThicknessBytes + indexBytes);

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

		auto pOutlineColors = (HiColor*)pDest;
		for (int i = 0; i < nbOutlineColorStates; i++)
			pOutlineColors[i] = stateOutlineColors[i];

		m_pOutlineColors = pOutlineColors;
		pDest += outlineColorBytes;

		auto pSizes = (float*)pDest;
		for (int i = 0; i < nbSizeStates; i++)
			pSizes[i] = stateSizes[i];

		m_pSizes = pSizes;
		pDest += sizeBytes;

		auto pThickness = (pts*)pDest;
		for (int i = 0; i < nbThicknessStates; i++)
			pThickness[i] = stateThickness[i];

		m_pThickness = pThickness;
		pDest += thicknessBytes;

		auto pOutlineThickness = (pts*)pDest;
		for (int i = 0; i < nbOutlineThicknessStates; i++)
			pOutlineThickness[i] = stateOutlineThickness[i];

		m_pOutlineThickness = pOutlineThickness;
		pDest += outlineThicknessBytes;

		m_pColorIndexTab = pDest;
		pDest += colorIndexEntries;

		m_pOutlineColorIndexTab = pDest;
		pDest += outlineColorIndexEntries;

		m_pSizeIndexTab = pDest;
		pDest += sizeIndexEntries;

		m_pThicknessIndexTab = pDest;
		pDest += thicknessIndexEntries;

		m_pOutlineThicknessIndexTab = pDest;
		pDest += outlineThicknessIndexEntries;

		generateStateToIndexTab(m_pColorIndexTab, nbColorStates, colorStates);
		generateStateToIndexTab(m_pOutlineColorIndexTab, nbOutlineColorStates, outlineColorStates);
		generateStateToIndexTab(m_pSizeIndexTab, nbSizeStates, sizeStates);
		generateStateToIndexTab(m_pThicknessIndexTab, nbThicknessStates, thicknessStates);
		generateStateToIndexTab(m_pOutlineThicknessIndexTab, nbOutlineThicknessStates, outlineThicknessStates);
	}

	//____ destructor _________________________________________________________

	CircleSkin::~CircleSkin()
	{
		free(m_pStateData);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CircleSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() _______________________________________________________________

	void CircleSkin::_render(GfxDevice* pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		RenderSettings settings(pDevice, m_layer, m_blendMode);

		float size = _getSize(state);

		if (canvas.w != canvas.h || size != 1.f )
		{
			spx sideLength = std::min(canvas.w, canvas.h) * size;
			canvas.x += (canvas.w - sideLength) / 2;
			canvas.y += (canvas.h - sideLength) / 2;
			canvas.w = sideLength;
			canvas.h = sideLength;
		}


		spx thickness = _getThickness(state) * scale;
		spx outlineThickness = _getOutlineThickness(state) * scale;


		pDevice->drawElipse(canvas, thickness, _getColor(state), outlineThickness, _getOutlineColor(state));
	}

	//____ _markTest() _____________________________________________________________

	bool CircleSkin::_markTest(const CoordSPX& _ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));
		
		if( !canvas.contains(_ofs) )
			return false;
		
		canvas + align(ptsToSpx(m_overflow, scale));
		
		CoordSPX ofs = _ofs;

		float size = _getSize(state);

		spx sideLength = std::min(canvas.w, canvas.h) * size;

		if (canvas.w != canvas.h || size != 1.f)
		{
			canvas.x += (canvas.w - sideLength) / 2;
			canvas.y += (canvas.h - sideLength) / 2;
			canvas.w = sideLength;
			canvas.h = sideLength;
		}

		CoordSPX center = canvas.center();

		float distanceSquared = (float) (pow((ofs.x - center.x), 2) + pow((ofs.y - center.y), 2));

		float radius = sideLength / 2;

		float thickness = _getThickness(state)*scale;
		float outlineThickness = _getOutlineThickness(state)*scale;


		if (distanceSquared > radius * radius)
			return false;

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		
		float fillRadius = radius - outlineThickness;

		if( distanceSquared > fillRadius * fillRadius )
			return (_getOutlineColor(state).a >= alpha);

		float innerOutlineRadius = fillRadius - thickness;

		if( innerOutlineRadius < 0 || distanceSquared > innerOutlineRadius * innerOutlineRadius )
			return (_getColor(state).a >= alpha);

		float holeRadius = innerOutlineRadius - outlineThickness;

		if( holeRadius < 0 || distanceSquared > holeRadius*holeRadius )
			return (_getOutlineColor(state).a >= alpha);

		return false;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX CircleSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		//TODO: Implement!
		
		return RectSPX();
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX CircleSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		
		if ( _getColor(newState) != _getColor(oldState) || _getSize(newState) != _getSize(oldState) ||
			_getThickness(newState) != _getThickness(oldState) || _getOutlineThickness(newState) != _getOutlineThickness(oldState) ||
			_getOutlineColor(newState) != _getOutlineColor(oldState))
			return canvas;

		return StateSkin::_dirtyRect(	canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
										newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}



} // namespace wg
