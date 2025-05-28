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

#include <wg_gradientskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	const TypeInfo GradientSkin::TYPEINFO = { "GradientSkin", &Skin::TYPEINFO };

	using namespace Util;

	//____ create() _______________________________________________________________

	GradientSkin_p GradientSkin::create(const Gradient& gradient)
	{
		if (!gradient.isValid())
			return nullptr;

		Blueprint blueprint;
		blueprint.gradient = gradient;
		return GradientSkin_p(new GradientSkin(blueprint));

	}

	GradientSkin_p GradientSkin::create( const Blueprint& blueprint )
	{
		if (!blueprint.gradient.isValid())
			return nullptr;

		return GradientSkin_p(new GradientSkin(blueprint));
	}

	//____ constructor ____________________________________________________________

	GradientSkin::GradientSkin( const Blueprint& bp ) : StateSkin(bp)
	{
		m_gradient[0]		= bp.gradient;
		m_blendMode			= bp.blendMode;

		Bitmask<uint32_t>	gradientStateMask;

		bool bOpaque = bp.gradient.isOpaque();

		for (auto& stateInfo : bp.states)
		{
			int index = stateInfo.state;

			if (stateInfo.data.contentShift.x != 0 || stateInfo.data.contentShift.y != 0)
			{
				m_contentShiftStateMask.setBit(index);
				m_contentShift[index] = stateInfo.data.contentShift;
				m_bContentShifting = true;
			}

			const Gradient& gradient = stateInfo.data.gradient;

			if (gradient.isValid() )
			{
				gradientStateMask.setBit(index);
				m_gradient[index] = gradient;
				if (!gradient.isOpaque())
					bOpaque = false;
			}
		}
		
		_updateContentShift();
		_updateUnsetGradients(gradientStateMask);
		
		if (m_blendMode == BlendMode::Replace)
			m_bOpaque = bOpaque;
		else if (m_blendMode == BlendMode::Blend)
			m_bOpaque = bOpaque;
		else
			m_bOpaque = false;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GradientSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() ______________________________________________________________

	void GradientSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow, scale));

		RenderSettingsWithGradient settings(pDevice, m_layer, m_blendMode, HiColor::White, canvas, m_gradient[state]);
		
		pDevice->fill(canvas, HiColor::White);
	}

	//____ _coverage() ________________________________________________________

	RectSPX GradientSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_bOpaque )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _markTest() _________________________________________________________

	bool GradientSkin::_markTest( const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing,scale));

		if (!canvas.contains(ofs) )
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;
		
		int xFrac = (ofs.x - canvas.x) * 4096 / canvas.w;
		int yFrac = (ofs.y - canvas.y) * 4096 / canvas.h;

		const Gradient& gradient = m_gradient[state];

		int xVal1 = gradient.topLeft.a + (gradient.topRight.a - gradient.topLeft.a) * xFrac / 4096;
		int xVal2 = gradient.bottomLeft.a + (gradient.bottomRight.a - gradient.bottomLeft.a) * xFrac / 4096;

		int val = xVal1 + (xVal2 - xVal1) * yFrac / 4096;

		return (val >= alpha);
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX GradientSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState,
		float newValue, float oldValue, float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if (m_gradient[i1] != m_gradient[i2])
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _updateUnsetGradients() _______________________________________________

	void GradientSkin::_updateUnsetGradients(Bitmask<uint32_t> stateMask)
	{
		for (int i = 0; i < State::NbStates; i++)
		{
			if (!stateMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, stateMask);
				m_gradient[i] = m_gradient[bestAlternative];
			}
		}
	}


} // namespace wg
