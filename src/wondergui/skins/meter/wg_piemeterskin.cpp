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
#include <wg_piemeterskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

#include <algorithm>

namespace wg
{

	const TypeInfo PieMeterSkin::TYPEINFO = { "PieMeterSkin", &Skin::TYPEINFO };

	using namespace Util;

	//____ create() _______________________________________________________________

	PieMeterSkin_p PieMeterSkin::create( const Blueprint& bp )
	{
		return PieMeterSkin_p(new PieMeterSkin(bp));
	}

	//____ constructor ____________________________________________________________

	PieMeterSkin::PieMeterSkin( const Blueprint& bp ) : Skin(bp)
	{

		m_rangeStart = bp.rotation / 360;
		if( m_rangeStart < 0.f )
			m_rangeStart += 1.f;
		
		
		m_maxRange = bp.length / 360.f;
		m_minRange = bp.min * (bp.length / 360.f);
		m_hubSize = bp.hubSize;
		
		m_hubColor = bp.hubColor;
		m_backColor = bp.backColor;
		m_emptyColor = bp.emptyColor;

		m_gfxPadding = bp.gfxPadding;
		m_bStaticSections = !bp.movingSlices;
		m_bRectangular = bp.square;

		m_bStartFromCenter	= bp.startFromCenter;
		
		// Copy slices

		m_nSlices = 0;
		float	pieSize = 0.f;

		for (auto& slice : bp.slices)
		{
			if (slice.size <= 0)
				break;

			m_slices[m_nSlices++] = slice;
			pieSize += slice.size;
		}

		// Normalize slice sizes

		if (pieSize > 0 && pieSize != 1.f)
		{
			float factor = 1.f / pieSize;

			for (int i = 0; i < m_nSlices; i++)
				m_slices[i].size *= factor;
		}
		
		_updateOpacity();
		
		m_blendMode = bp.blendMode;
		m_defaultSize = bp.defaultSize.isEmpty() ? Size(64, 64) : bp.defaultSize;
		m_bIgnoresValue = false;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PieMeterSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() ______________________________________________________________

	void PieMeterSkin::_render(GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RenderSettings settings(pDevice, m_layer, m_blendMode);

		bool	bFramed = false;

		RectSPX outerCanvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		RectSPX canvas = outerCanvas;

		// Shrink canvas with padding

		if (!m_gfxPadding.isEmpty())
		{
			canvas -= align(ptsToSpx(m_gfxPadding,scale));

			if (canvas.w <= 0 || canvas.h <= 0)
			{
				pDevice->fill(outerCanvas, m_backColor);
				return;
			}
			bFramed = true;
		}

		// 

		// Force canvas to be square.

		if (canvas.w != canvas.h)
		{
			if (canvas.w > canvas.h)
			{
				canvas.x += (canvas.w - canvas.h) / 2;
				canvas.w = canvas.h;
			}
			else if (canvas.h > canvas.w)
			{
				canvas.y += (canvas.h - canvas.w) / 2;
				canvas.h = canvas.w;
			}
			bFramed = true;
		}

		// Fill borders with background color.

		if (bFramed && m_backColor.a > 0.f)
		{
			RectSPX	outer = outerCanvas;
			RectSPX	inner = canvas;
			BorderSPX frame = { inner.y - outer.y, outer.right() - inner.right(), outer.bottom() - inner.bottom(), inner.x - outer.x };

			RectSPX top(outer.x, outer.y, outer.w, frame.top);
			RectSPX left(outer.x, outer.y + frame.top, frame.left, outer.h - frame.height());
			RectSPX right(outer.x + outer.w - frame.right, outer.y + frame.top, frame.right, outer.h - frame.height());
			RectSPX bottom(outer.x, outer.y + outer.h - frame.bottom, outer.w, frame.bottom);

			pDevice->fill(top, m_backColor);
			pDevice->fill(left, m_backColor);
			pDevice->fill(right, m_backColor);
			pDevice->fill(bottom, m_backColor);
		}

		//

		float sliceSizes[c_maxSlices];
		HiColor sliceColors[c_maxSlices];
		int nSlices = 0;

		HiColor hubColor = m_hubColor.a == 4096 ? m_hubColor : HiColor::blend(m_backColor, m_hubColor, BlendMode::Blend);

		float startPos, totalLength;
		
		if( m_bStartFromCenter )
		{
			if( !m_emptyColor.isFullyTransparent() )
			{
				sliceColors[nSlices] = m_emptyColor;
				sliceSizes[nSlices++] = (m_maxRange-m_minRange)/2;
			}

			totalLength = m_minRange + std::abs(value - 0.5f) * (m_maxRange - m_minRange);

			if( value < 0.5f || !m_emptyColor.isFullyTransparent() )
				startPos = m_rangeStart - m_maxRange/2;
			else
				startPos = m_rangeStart - (m_minRange/2);
		}
		else
		{
			startPos = m_rangeStart;
			totalLength = m_minRange + value * (m_maxRange - m_minRange);
		}
		
		while( startPos < 0 )
			startPos += 1.f;
		
//		if (totalLength == 0.f)
//			return;


		if (m_bStaticSections)
		{
			float length = 0;

			for (int i = 0; i < m_nSlices; i++)
			{
				sliceColors[nSlices] = HiColor::mix(m_slices[i].minColor, m_slices[i].maxColor, int(value*4096));

				float size = m_slices[i].size*m_maxRange;
				if (length + size >= totalLength)
				{
					sliceSizes[nSlices] = totalLength - length;
					nSlices++;
					break;
				}
				sliceSizes[nSlices] = size;
				length += size;
				nSlices++;
			}
		}
		else
		{
			float length = 0;

			float gapSize = m_maxRange - totalLength;

			for (int i = 0; i < m_nSlices; i++)
			{
				float size = m_slices[i].size*m_maxRange;
				if (length + size > gapSize )
				{
					sliceSizes[nSlices] = length < gapSize ? length + size - gapSize : size;
					sliceColors[nSlices] = HiColor::mix(m_slices[i].minColor, m_slices[i].maxColor, int(value * 4096));
					nSlices++;
				}
				length += size;
			}
		}

		// Possibly fill out with empty section

		if( m_bStartFromCenter)
		{
			if (value > 0.f && value < 1.f)
			{
				sliceColors[nSlices] = m_emptyColor;
				sliceSizes[nSlices++] = (0.5f - std::abs(value-0.5f) ) * (m_maxRange - m_minRange);
			}

			if( m_bStartFromCenter && value < 0.5f )
			{
				std::reverse(sliceColors, sliceColors+nSlices);
				std::reverse(sliceSizes, sliceSizes+nSlices);
			}

		}
		else
		{
			if (value < 1.f && !m_emptyColor.isFullyTransparent() )
			{
				sliceColors[nSlices] = m_emptyColor;
				sliceSizes[nSlices++] = (1.f - value) * (m_maxRange - m_minRange);
			}
		}
		
		
		//
		
		pDevice->drawPieChart(canvas, startPos, nSlices, sliceSizes, sliceColors, m_hubSize, hubColor, m_backColor, m_bRectangular);
	}

	//____  _defaultSize() ___________________________________________________

	SizeSPX	PieMeterSkin::_defaultSize(int scale) const
	{
		return align(ptsToSpx(m_defaultSize, scale)) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _markTest() _________________________________________________________

	bool PieMeterSkin::_markTest(const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));
		
		if (!canvas.contains(ofs))
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		if (m_bOpaque)
			return true;

		//TODO: Implement!

		return true;
	}

	//____ _dirtyRect() ________________________________________________________

	RectSPX PieMeterSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		//TODO: Implement more optimized version.

		if (newValue != oldValue)
			return canvas;

		return RectSPX();
	}

	//____ _coverage() ___________________________________________________________

	RectSPX PieMeterSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_bOpaque )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

	//____ _updateOpacity() ___________________________________________________

	void PieMeterSkin::_updateOpacity()
	{
		if (m_blendMode == BlendMode::Replace)
			m_bOpaque = true;
		else if (m_blendMode != BlendMode::Blend || m_backColor.a < 4096 || (m_hubSize > 0.f && m_hubColor.a < 4096) )
			m_bOpaque = false;
		else
		{
			int totalAlpha = 0;

			for (int i = 0; i < m_nSlices; i++)
			{
				totalAlpha += m_slices[i].minColor.a;
				totalAlpha += m_slices[i].maxColor.a;
			}

			m_bOpaque = (totalAlpha == m_nSlices * 2 * 4096);
		}
	}

} // namespace wg
