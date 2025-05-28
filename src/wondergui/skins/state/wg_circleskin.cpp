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

		m_stateInfo[0].color = bp.color;
		m_stateInfo[0].outlineColor = bp.outlineColor;
		m_stateInfo[0].outlineThickness = bp.outlineThickness;
		m_stateInfo[0].size = bp.size;
		m_stateInfo[0].thickness = bp.thickness;

		for (auto& stateInfo : bp.states)
		{
			int index = stateInfo.state;

			if (stateInfo.data.color != HiColor::Undefined)
			{
				m_stateColorMask.setBit(index);
				m_stateInfo[index].color = stateInfo.data.color;
			}

			if (stateInfo.data.size >= 0)
			{
				m_stateSizeMask.setBit(index);
				m_stateInfo[index].size = stateInfo.data.size;
			}

			if (stateInfo.data.thickness >= 0)
			{
				m_stateThicknessMask.setBit(index);
				m_stateInfo[index].thickness = stateInfo.data.thickness;
			}

			if (stateInfo.data.outlineThickness >= 0)
			{
				m_stateOutlineThicknessMask.setBit(index);
				m_stateInfo[index].outlineThickness = stateInfo.data.outlineThickness;
			}

			if (stateInfo.data.outlineColor != HiColor::Undefined)
			{
				m_stateOutlineColorMask.setBit(index);
				m_stateInfo[index].outlineColor = stateInfo.data.outlineColor;
			}

			if (stateInfo.data.contentShift.x != 0 || stateInfo.data.contentShift.y != 0)
			{
				m_contentShiftStateMask.setBit(index);
				m_contentShift[index] = stateInfo.data.contentShift;
				m_bContentShifting = true;
			}
		}

		_updateContentShift();
		_updateUnsetStates();
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

		int i = state;

		if (canvas.w != canvas.h || m_stateInfo[i].size != 1.f )
		{
			spx sideLength = std::min(canvas.w, canvas.h) * m_stateInfo[i].size;
			canvas.x += (canvas.w - sideLength) / 2;
			canvas.y += (canvas.h - sideLength) / 2;
			canvas.w = sideLength;
			canvas.h = sideLength;
		}


		spx thickness = m_stateInfo[i].thickness * scale;
		spx outlineThickness = m_stateInfo[i].outlineThickness * scale;


		pDevice->drawElipse(canvas, thickness, m_stateInfo[i].color, outlineThickness, m_stateInfo[i].outlineColor);
	}

	//____ _markTest() _____________________________________________________________

	bool CircleSkin::_markTest(const CoordSPX& _ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		int i = state;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));
		
		if( !canvas.contains(_ofs) )
			return false;
		
		canvas + align(ptsToSpx(m_overflow, scale));
		
		CoordSPX ofs = _ofs;

		spx sideLength = std::min(canvas.w, canvas.h) * m_stateInfo[i].size;

		if (canvas.w != canvas.h || m_stateInfo[i].size != 1.f)
		{
			canvas.x += (canvas.w - sideLength) / 2;
			canvas.y += (canvas.h - sideLength) / 2;
			canvas.w = sideLength;
			canvas.h = sideLength;
		}

		CoordSPX center = canvas.center();

		float distanceSquared = (float) (pow((ofs.x - center.x), 2) + pow((ofs.y - center.y), 2));

		float radius = sideLength / 2;

		float thickness = m_stateInfo[i].thickness*scale;
		float outlineThickness = m_stateInfo[i].outlineThickness*scale;


		if (distanceSquared > radius * radius)
			return false;

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		
		float fillRadius = radius - outlineThickness;

		if( distanceSquared > fillRadius * fillRadius )
			return (m_stateInfo[i].outlineColor.a >= alpha);

		float innerOutlineRadius = fillRadius - thickness;

		if( innerOutlineRadius < 0 || distanceSquared > innerOutlineRadius * innerOutlineRadius )
			return (m_stateInfo[i].color.a >= alpha);

		float holeRadius = innerOutlineRadius - outlineThickness;

		if( holeRadius < 0 || distanceSquared > holeRadius*holeRadius )
			return (m_stateInfo[i].outlineColor.a >= alpha);

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

		
		if ( m_stateInfo[i1].color != m_stateInfo[i2].color || m_stateInfo[i1].size != m_stateInfo[i2].size ||
			m_stateInfo[i1].thickness != m_stateInfo[i2].thickness || m_stateInfo[i1].outlineThickness != m_stateInfo[i2].outlineThickness ||
			m_stateInfo[i1].outlineColor != m_stateInfo[i2].outlineColor)
			return canvas;

		return StateSkin::_dirtyRect(	canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
										newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}


	//____ _updateUnsetStates() _______________________________________________

	void CircleSkin::_updateUnsetStates()
	{
		for (int i = 0; i < State::NbStates; i++)
		{
			if (!m_stateColorMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateColorMask);
				m_stateInfo[i].color = m_stateInfo[bestAlternative].color;
			}

			if (!m_stateSizeMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateSizeMask);
				m_stateInfo[i].size = m_stateInfo[bestAlternative].size;
			}

			if (!m_stateThicknessMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateThicknessMask);
				m_stateInfo[i].thickness = m_stateInfo[bestAlternative].thickness;
			}

			if (!m_stateOutlineThicknessMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateOutlineThicknessMask);
				m_stateInfo[i].outlineThickness = m_stateInfo[bestAlternative].outlineThickness;
			}

			if (!m_stateOutlineColorMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateOutlineColorMask);
				m_stateInfo[i].outlineColor = m_stateInfo[bestAlternative].outlineColor;
			}
		}
	}


} // namespace wg
