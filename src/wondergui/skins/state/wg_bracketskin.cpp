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


		m_color[0] = bp.color;

		for (auto& stateInfo : bp.states)
		{
			int index = stateInfo.state;

			if (stateInfo.data.contentShift.x != 0 || stateInfo.data.contentShift.y != 0)
			{
				m_contentShiftStateMask.setBit(index);
				m_contentShift[index] = stateInfo.data.contentShift;
				m_bContentShifting = true;
			}

			if (stateInfo.data.color != HiColor::Undefined)
			{
				m_stateColorMask.setBit(index);
				m_color[index] = stateInfo.data.color;
			}

		}

		_updateContentShift();
		_updateUnsetColors();
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

		int i = state;

		spx thickness = align(ptsToSpx(m_thickness, scale));
		SizeSPX size = align(ptsToSpx(m_size, scale));


		if (thickness*2 >= canvas.w || thickness*2 >= canvas.h)
		{
			pDevice->fill(canvas, m_color[i]); 
		}
		else
		{
			if (size.w * 2 >= canvas.w)
			{
				pDevice->fill({ canvas.x, canvas.y, canvas.w, thickness }, m_color[i]);
				pDevice->fill({ canvas.x, canvas.y + canvas.h - thickness, canvas.w, thickness }, m_color[i]);
			}
			else
			{
				pDevice->fill({ canvas.x, canvas.y, size.w, thickness }, m_color[i]);
				pDevice->fill({ canvas.x + canvas.w - size.w, canvas.y, size.w, thickness }, m_color[i]);

				pDevice->fill({ canvas.x, canvas.y + canvas.h - thickness, size.w, thickness }, m_color[i]);
				pDevice->fill({ canvas.x + canvas.w - size.w, canvas.y + canvas.h - thickness, size.w, thickness }, m_color[i]);
			}

			if (size.h * 2 >= canvas.h)
			{
				pDevice->fill({ canvas.x, canvas.y + thickness, thickness, canvas.h - thickness*2  }, m_color[i]);
				pDevice->fill({ canvas.x + canvas.w - thickness, canvas.y + thickness, thickness, canvas.h - thickness*2 }, m_color[i]);
			}
			else
			{
				pDevice->fill({ canvas.x, canvas.y + thickness, thickness, size.h - thickness }, m_color[i]);
				pDevice->fill({ canvas.x + canvas.w - thickness, canvas.y + thickness, thickness, size.h - thickness }, m_color[i]);

				pDevice->fill({ canvas.x, canvas.y + canvas.h - size.h, thickness, size.h - thickness }, m_color[i]);
				pDevice->fill({ canvas.x + canvas.w - thickness, canvas.y + canvas.h - size.h, thickness, size.h - thickness }, m_color[i]);
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

		if (m_color[i1] != m_color[i2] )
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

	//____ _updateUnsetColors() _______________________________________________

	void BracketSkin::_updateUnsetColors()
	{
		for (int i = 0; i < State::NbStates; i++)
		{
			if (!m_stateColorMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateColorMask);
				m_color[i] = m_color[bestAlternative];
			}
		}
	}


} // namespace wg
