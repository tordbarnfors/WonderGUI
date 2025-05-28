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

#include <wg_boxskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo BoxSkin::TYPEINFO = { "BoxSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________

	BoxSkin_p BoxSkin::create( const Blueprint& blueprint )
	{
		return BoxSkin_p(new BoxSkin(blueprint));
	}

	BoxSkin_p BoxSkin::create(Border outline, HiColor fillColor, HiColor outlineColor, Border padding )
	{
		Blueprint bp;
		bp.outlineThickness = outline;
		bp.color = fillColor;
		bp.outlineColor = outlineColor;
		bp.padding = padding;

		return BoxSkin_p(new BoxSkin(bp));
	}


	//____ constructor ____________________________________________________________

	BoxSkin::BoxSkin( const Blueprint& bp ) : StateSkin(bp)
	{
		m_outline		= bp.outlineThickness;
		m_blendMode		= bp.blendMode;

		m_fillColor[0] = bp.color;
		m_outlineColor[0] = bp.outlineColor;


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
				m_fillColor[index] = stateInfo.data.color;
			}

			if (stateInfo.data.outlineColor != HiColor::Undefined)
			{
				m_stateOutlineColorMask.setBit(index);
				m_outlineColor[index] = stateInfo.data.outlineColor;
			}
		}

		_updateContentShift();
		_updateUnsetColors();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BoxSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() _______________________________________________________________

	void BoxSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		//TODO: Optimize! Clip patches against canvas first.

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		RenderSettings settings(pDevice, m_layer, m_blendMode);

		int i = state;

		BorderSPX outline = align(ptsToSpx(m_outline, scale));

		if( outline.width() + outline.height() == 0 || m_outlineColor[i] == m_fillColor[i] )
		{
			pDevice->fill( canvas, m_fillColor[i] );
		}
		else
		{
			if (outline.width() >= canvas.w || outline.height() >= canvas.h)
			{
				pDevice->fill(canvas, m_outlineColor[i]);
			}
			else
			{
				RectSPX top( canvas.x, canvas.y, canvas.w, outline.top );
				RectSPX left( canvas.x, canvas.y+outline.top, outline.left, canvas.h - outline.height() );
				RectSPX right( canvas.x + canvas.w - outline.right, canvas.y+outline.top, outline.right, canvas.h - outline.height() );
				RectSPX bottom( canvas.x, canvas.y + canvas.h - outline.bottom, canvas.w, outline.bottom );
				RectSPX center( canvas - outline );

				pDevice->fill( top, m_outlineColor[i] );
				pDevice->fill( left, m_outlineColor[i] );
				pDevice->fill( right, m_outlineColor[i] );
				pDevice->fill( bottom, m_outlineColor[i] );

				if( center.w > 0 || center.h > 0 )
					pDevice->fill( center, m_fillColor[i] );
			}
		}

	}

	//____ _minSize() ______________________________________________________________

	SizeSPX BoxSkin::_minSize(int scale) const
	{
		SizeSPX content = StateSkin::_minSize(scale);
		SizeSPX outline = align(ptsToSpx(m_outline,scale));

		return SizeSPX::max(content,outline) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX BoxSkin::_defaultSize(int scale) const
	{
		SizeSPX content = StateSkin::_minSize(scale);
		SizeSPX outline = align(ptsToSpx(m_outline, scale));

		return SizeSPX::max(content, outline) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _sizeForContent() _______________________________________________________

	SizeSPX BoxSkin::_sizeForContent( const SizeSPX& contentSize, int scale ) const
	{
		SizeSPX content = StateSkin::_sizeForContent(contentSize,scale);
		SizeSPX outline = align(ptsToSpx(m_outline, scale));

		return SizeSPX::max(content, outline) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _markTest() _____________________________________________________________

	bool BoxSkin::_markTest( const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if( !canvas.contains(ofs) )
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		int opacity;

		RectSPX center = canvas - align(ptsToSpx(m_outline,scale));
		if( center.contains(ofs) )
			opacity = m_fillColor[state].a;
		else
			opacity = m_outlineColor[state].a;

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		return ( opacity >= alpha);
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX BoxSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		int i1 = newState;
		int i2 = oldState;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if (m_fillColor[i1] != m_fillColor[i2] || (!m_outline.isEmpty() && m_outlineColor[i1] != m_outlineColor[i2]))
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX BoxSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_blendMode == BlendMode::Blend )
		{
			if( m_fillColor[state].a == 4096 )
			{
				RectSPX coverage = geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
				if( m_outlineColor[state].a != 4096 )
					coverage -= align(ptsToSpx(m_outline,scale));
				
				return coverage;
			}
		}
		else if( m_blendMode == BlendMode::Replace )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));

		return RectSPX();
	}

	//____ _updateUnsetColors() _______________________________________________

	void BoxSkin::_updateUnsetColors()
	{
		for (int i = 0; i < State::NbStates; i++)
		{
			if (!m_stateColorMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateColorMask);
				m_fillColor[i] = m_fillColor[bestAlternative];
			}

			if (!m_stateOutlineColorMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateOutlineColorMask);
				m_outlineColor[i] = m_outlineColor[bestAlternative];
			}
		}
	}


} // namespace wg
