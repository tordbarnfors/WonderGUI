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
#include <wg_boxskin.h>
#include <wax_gfxdevice.h>
#include <wax_geo.h>
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

		for (int i = 0; i < 4; i++)
			m_cornerRadius[i] = std::max(bp.cornerRadius[i], pts(0));

		m_bRounded = !m_cornerRadius.isEmpty();

		// Generate lists of states that affects shift, color and surface.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	fillColorStates[State::NbStates];
		HiColor stateFillColors[State::NbStates];

		State	outlineColorStates[State::NbStates];
		HiColor stateOutlineColors[State::NbStates];

		int		nbShiftingStates = 1;
		int		nbFillColorStates = 1;
		int		nbOutlineColorStates = 1;

		shiftingStates[0] = State::Default;
		fillColorStates[0] = State::Default;
		outlineColorStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateFillColors[0] = bp.color;
		stateOutlineColors[0] = bp.outlineColor;

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
				int index = stateInfo.state == State::Default ? 0 : nbFillColorStates++;
				fillColorStates[index] = stateInfo.state;
				stateFillColors[index] = stateInfo.data.color;
			}

			if(stateInfo.data.outlineColor != HiColor::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbOutlineColorStates++;
				outlineColorStates[index] = stateInfo.state;
				stateOutlineColors[index] = stateInfo.data.outlineColor;
			}
		}

		// Calc size of index table for surface and color, get their index masks & shifts.

		int	fillColorIndexEntries;
		int	outlineColorIndexEntries;

		std::tie(fillColorIndexEntries,m_fillColorIndexMask,m_fillColorIndexShift) = calcStateToIndexParam(nbFillColorStates, fillColorStates);
		std::tie(outlineColorIndexEntries,m_outlineColorIndexMask,m_outlineColorIndexShift) = calcStateToIndexParam(nbOutlineColorStates, outlineColorStates);

		// Calculate memory needed for all state data

		int shiftBytes 			= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int fillColorBytes		= sizeof(HiColor) * nbFillColorStates;
		int outlineColorBytes	= sizeof(HiColor) * nbOutlineColorStates;
		int indexBytes			= fillColorIndexEntries+outlineColorIndexEntries;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(shiftBytes + fillColorBytes + outlineColorBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		auto pFillColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbFillColorStates ; i++ )
			pFillColors[i] = stateFillColors[i];

		m_pFillColors = pFillColors;
		pDest += fillColorBytes;

		auto pOutlineColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbOutlineColorStates ; i++ )
			pOutlineColors[i] = stateOutlineColors[i];

		m_pOutlineColors = pOutlineColors;
		pDest += outlineColorBytes;

		m_pFillColorIndexTab = pDest;
		m_pOutlineColorIndexTab = pDest + fillColorIndexEntries;

		generateStateToIndexTab(m_pFillColorIndexTab, nbFillColorStates, fillColorStates);
		generateStateToIndexTab(m_pOutlineColorIndexTab, nbOutlineColorStates, outlineColorStates);
	}

	//____ destructor ____________________________________________________________

	BoxSkin::~BoxSkin()
	{
		free(m_pStateData);
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

		BorderSPX outline = align(ptsToSpx(m_outline, scale));

		const HiColor& fillColor = _getFillColor(state);
		const HiColor& outlineColor = _getOutlineColor(state);

		if( m_bRounded )
		{
			RoundedBoxSpec spec;
			spec.rect = canvas;
			spec.radius = _cornerRadii(canvas, scale);
			spec.fillColor = fillColor;

			if( fillColor != outlineColor )
			{
				spec.outline = outline;
				spec.outlineColor = outlineColor;
			}

			pDevice->drawRoundedBox(spec);
		}
		else if( outline.width() + outline.height() == 0 || fillColor == outlineColor )
		{
			pDevice->fill( canvas, fillColor );
		}
		else
		{
			if (outline.width() >= canvas.w || outline.height() >= canvas.h)
			{
				pDevice->fill(canvas, outlineColor);
			}
			else
			{
				RectSPX top( canvas.x, canvas.y, canvas.w, outline.top );
				RectSPX left( canvas.x, canvas.y+outline.top, outline.left, canvas.h - outline.height() );
				RectSPX right( canvas.x + canvas.w - outline.right, canvas.y+outline.top, outline.right, canvas.h - outline.height() );
				RectSPX bottom( canvas.x, canvas.y + canvas.h - outline.bottom, canvas.w, outline.bottom );
				RectSPX center( canvas - outline );

				pDevice->fill( top, outlineColor );
				pDevice->fill( left, outlineColor );
				pDevice->fill( right, outlineColor );
				pDevice->fill( bottom, outlineColor );

				if( center.w > 0 || center.h > 0 )
					pDevice->fill( center, fillColor );
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

		BorderSPX outline = align(ptsToSpx(m_outline,scale));
		RectSPX center = canvas - outline;

		if( m_bRounded )
		{
			// Test against the rounded box, with inner corners as drawn by GfxDevice::drawRoundedBox().

			CornersSPX radius = _cornerRadii(canvas, scale);

			if( !Util::isInsideRoundedBox(ofs, canvas, radius) )
				return false;

			if( Util::isInsideRoundedBox(ofs, center, Util::innerCornerRadii(radius, outline)) )
				opacity = _getFillColor(state).a;
			else
				opacity = _getOutlineColor(state).a;
		}
		else if( center.contains(ofs) )
			opacity = _getFillColor(state).a;
		else
			opacity = _getOutlineColor(state).a;

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

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if (_getFillColor(newState) != _getFillColor(oldState) || (!m_outline.isEmpty() && _getOutlineColor(newState) != _getOutlineColor(oldState)))
			return canvas;

		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2,
			newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX BoxSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_blendMode == BlendMode::Blend )
		{
			if( _getFillColor(state).a == 4096 )
			{
				RectSPX canvas = geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
				RectSPX coverage = canvas;

				if( _getOutlineColor(state).a != 4096 )
					coverage -= align(ptsToSpx(m_outline,scale));
				
				return m_bRounded ? _roundedCoverage(canvas, coverage, scale) : coverage;
			}
		}
		else if( m_blendMode == BlendMode::Replace )
		{
			RectSPX canvas = geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
			return m_bRounded ? _roundedCoverage(canvas, canvas, scale) : canvas;
		}

		return RectSPX();
	}

	//____ _cornerRadii() _______________________________________________________

	CornersSPX BoxSkin::_cornerRadii(const RectSPX& canvas, int scale) const
	{
		CornersSPX radius = ptsToSpx(m_cornerRadius, scale);
		Util::clampCornerRadii(canvas.size(), radius);
		return radius;
	}

	//____ _roundedCoverage() ___________________________________________________
	// The part of coverage (a rectangle inside canvas) that is fully covered despite the
	// rounded corners: the band between the corners, horizontal or vertical, whichever is
	// larger, shrunk to whole pixels.

	RectSPX BoxSkin::_roundedCoverage(const RectSPX& canvas, const RectSPX& coverage, int scale) const
	{
		CornersSPX radius = _cornerRadii(canvas, scale);

		spx top = canvas.y + std::max(radius[0], radius[1]);
		spx bottom = canvas.y + canvas.h - std::max(radius[2], radius[3]);
		spx left = canvas.x + std::max(radius[0], radius[3]);
		spx right = canvas.x + canvas.w - std::max(radius[1], radius[2]);

		RectSPX horizontal = RectSPX::overlap(coverage, RectSPX(canvas.x, top, canvas.w, std::max(bottom - top, 0)));
		RectSPX vertical = RectSPX::overlap(coverage, RectSPX(left, canvas.y, std::max(right - left, 0), canvas.h));

		RectSPX band = int64_t(horizontal.w) * horizontal.h >= int64_t(vertical.w) * vertical.h ? horizontal : vertical;

		if (band.w <= 0 || band.h <= 0)
			return RectSPX();

		RectSPX out;
		out.x = (band.x + 63) & ~63;
		out.y = (band.y + 63) & ~63;
		out.w = ((band.x + band.w) & ~63) - out.x;
		out.h = ((band.y + band.h) & ~63) - out.y;

		if (out.w <= 0 || out.h <= 0)
			return RectSPX();

		return out;
	}

} // namespace wg
