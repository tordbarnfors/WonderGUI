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

#include <wg_fillmeterskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	const TypeInfo FillMeterSkin::TYPEINFO = { "FillMeterSkin", &Skin::TYPEINFO };

	using namespace Util;

	//____ create() ___________________________________________________________

	FillMeterSkin_p FillMeterSkin::create( const Blueprint& blueprint )
	{
		return FillMeterSkin_p(new FillMeterSkin(blueprint));
	}

	//____ constructor ________________________________________________________

	FillMeterSkin::FillMeterSkin( const Blueprint& bp) : Skin(bp)
	{
		if (bp.minColor != HiColor::Undefined && bp.maxColor != HiColor::Undefined)
		{
			m_barColorEmpty = bp.minColor;
			m_barColorFull	= bp.maxColor;
		}
		else
		{
			m_barColorEmpty = bp.color;
			m_barColorFull	= bp.color;
		}

		m_gfxPadding		= bp.gfxPadding;
		m_backColor			= bp.backColor;
		m_blendMode			= bp.blendMode;
		m_direction			= bp.direction;
		m_pTintmap			= bp.tintmap;

		m_minBarLength		= bp.startLength;
		m_bCenteredBarOrigin = bp.startFromCenter;

		if (bp.defaultSize.isEmpty())
		{
			Size pref = (bp.direction == Direction::Up || bp.direction == Direction::Down) ? Size(10, 50) : Size(50, 10);
			m_defaultSize = Size::max(pref, m_padding.size());
		}
		else
			m_defaultSize = bp.defaultSize;
		
		m_bIgnoresValue = false;
		_updateOpacity();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FillMeterSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _defaultSize() ____________________________________________________

	SizeSPX FillMeterSkin::_defaultSize(int scale) const
	{
		return ptsToSpx(m_defaultSize, scale) + align(ptsToSpx(m_spacing, scale));
	}


	//____ render() ______________________________________________________________

	void FillMeterSkin::_render(GfxDevice* pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) - align(ptsToSpx(m_gfxPadding, scale)) + align(ptsToSpx(m_overflow, scale));
		
		RenderSettingsWithTintmap settings(pDevice, m_layer, m_blendMode, HiColor::White, canvas, m_pTintmap );

		HiColor barColor = HiColor::mix(m_barColorEmpty, m_barColorFull, int(4096 * value));

		RectSPX barCanvas = align(_barFillArea(canvas, scale, value, value2));
		pDevice->fill(barCanvas, barColor);


		if (m_backColor.a != 0)
		{
			RectSPX backCanvas	= canvas;
			RectSPX backCanvas1 = canvas;
			RectSPX backCanvas2 = canvas;

			switch (m_direction)
			{
			case Direction::Up:
			case Direction::Down:
			{
				backCanvas1.h = barCanvas.y - backCanvas.y;

				backCanvas2.h = backCanvas.bottom() - barCanvas.bottom();
				backCanvas2.y = barCanvas.bottom();
				break;
			}

			case Direction::Left:
			case Direction::Right:
			{
				backCanvas1.w = barCanvas.x - backCanvas.x;

				backCanvas2.w = backCanvas.right() - barCanvas.right();
				backCanvas2.x = barCanvas.right();
				break;
			}
			}

			pDevice->fill(backCanvas1, m_backColor);
			pDevice->fill(backCanvas2, m_backColor);
		}
	}

	//____ _markTest() _________________________________________________________

	bool FillMeterSkin::_markTest(const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if (!canvas.contains(ofs))
			return false;
		
		canvas += align(ptsToSpx(m_overflow, scale));

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		if( _barFillArea(canvas, scale, value, value2).contains(ofs) )
			return (HiColor::mix(m_barColorEmpty, m_barColorFull, int(4096*value)).a >= alpha);

		return (m_backColor.a >= alpha);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX FillMeterSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if( m_bOpaque )
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}


	//____ _dirtyRect() ________________________________________________________

	RectSPX FillMeterSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{

		if (newValue == oldValue)
			return RectSPX();

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		
		if (m_barColorFull != m_barColorEmpty)
			return canvas;

		canvas -= align(ptsToSpx(m_padding, scale));

		RectSPX result1;
		RectSPX result2;

		if (oldValue != newValue)
			result1 = _valueChangeRect(canvas, scale, newState, oldValue, newValue);

		if (oldValue2 != newValue2)
			result2 = _valueChangeRect(canvas, scale, newState, oldValue2, newValue2);

		if (result2.isEmpty())
			return result1;

		if (result1.isEmpty())
			return result2;

		return RectSPX::bounds(result1, result2);
	}


	RectSPX FillMeterSkin::_valueChangeRect(const RectSPX& _canvas, int scale, State state, float oldFraction, float newFraction ) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if (m_bCenteredBarOrigin)
		{
			// If we start from center we ignore value2.

			bool bHorizontal = (m_direction == Direction::Left || m_direction == Direction::Right);
			bool bInverted = (m_direction == Direction::Left || m_direction == Direction::Up);

			//

			spx totalLen = bHorizontal ? canvas.w : canvas.h;
			spx minFillLength = ptsToSpx(m_minBarLength, scale);

			spx sideLen = (totalLen - minFillLength) / 2;
			spx centerLen = totalLen - sideLen * 2;

			// Invert value if direction is inverted

			if (bInverted)
			{
				oldFraction = 1.f - oldFraction;
				newFraction = 1.f - newFraction;
			}

			// Calculate offsets.

			spx ofs1, ofs2;

			if (oldFraction < 0.5f)
				ofs1 = oldFraction * 2.f * sideLen;
			else
				ofs1 = sideLen + centerLen + (oldFraction - 0.5f) * 2.f * sideLen;

			if (newFraction < 0.5f)
				ofs2 = newFraction * 2.f * sideLen;
			else
				ofs2 = sideLen + centerLen + (newFraction - 0.5f) * 2.f * sideLen;

			if (ofs1 > ofs2)
				std::swap(ofs1, ofs2);

			// Convert ofs & len to a rectangle.

			if (bHorizontal)
				return align(RectSPX(canvas.x + ofs1, canvas.y, ofs2 - ofs1, canvas.h));
			else
				return align(RectSPX(canvas.x, canvas.y + ofs1, canvas.w, ofs2 - ofs1));
		}
		else
		{
			spx minFillLength = ptsToSpx(m_minBarLength, scale);

			switch (m_direction)
			{
			case Direction::Up:
			{
				spx len = canvas.h - minFillLength;
				spx ofs1 = len - len * oldFraction;
				spx ofs2 = len - len * newFraction;
				if (ofs1 > ofs2)
					std::swap(ofs1, ofs2);

				return align(RectSPX(canvas.x, canvas.y + ofs1, canvas.w, ofs2 - ofs1));
			}

			case Direction::Down:
			{
				spx len = canvas.h - minFillLength;
				spx ofs1 = len * oldFraction;
				spx ofs2 = len * newFraction;
				if (ofs1 > ofs2)
					std::swap(ofs1, ofs2);

				return align(RectSPX(canvas.x, canvas.y + minFillLength + ofs1, canvas.w, ofs2 - ofs1));
			}

			case Direction::Left:
			{
				spx len = canvas.w - minFillLength;
				spx ofs1 = len - len * oldFraction;
				spx ofs2 = len - len * newFraction;
				if (ofs1 > ofs2)
					std::swap(ofs1, ofs2);

				return align(RectSPX(canvas.x + ofs1, canvas.y, ofs2 - ofs1, canvas.h));
			}

			case Direction::Right:
			{
				spx len = canvas.w - minFillLength;
				spx ofs1 = len * oldFraction;
				spx ofs2 = len * newFraction;
				if (ofs1 > ofs2)
					std::swap(ofs1, ofs2);

				return align(RectSPX(canvas.x + minFillLength + ofs1, canvas.y, ofs2 - ofs1, canvas.h));

			}

			default:
				return RectSPX();			// Just to avoid compiler warnings.
			}
		}
	}



	//____ _barFillArea() _____________________________________________________

	RectSPX FillMeterSkin::_barFillArea(const RectSPX& _canvas, int scale, float value, float value2) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		spx minFillLength = ptsToSpx(m_minBarLength, scale);

		if (m_bCenteredBarOrigin)
		{
			// If we start from center we ignore value2.

			bool bHorizontal = (m_direction == Direction::Left || m_direction == Direction::Right);
			bool bInverted = (m_direction == Direction::Left || m_direction == Direction::Up);

			//

			spx totalLen = bHorizontal ? canvas.w : canvas.h;

			spx sideLen = (totalLen - minFillLength) / 2;
			spx centerLen = totalLen - sideLen * 2;

			// Invert value if direction is inverted

			if (bInverted)
				value = 1.f - value;

			// Calculate offset and length of bar.

			spx ofs, len;

			if (value < 0.5f)
			{
				ofs = value * 2.f * sideLen;
				len = sideLen + centerLen - ofs;
			}
			else
			{
				ofs = sideLen;
				len = centerLen + (value - 0.5f) * 2.f * sideLen;
			}

			// Convert ofs & len to a rectangle.

			if (bHorizontal)
				return { canvas.x + ofs, canvas.y, len, canvas.h };
			else
				return { canvas.x, canvas.y + ofs, canvas.w, len };

		}
		else
		{
			float beg = value2 >= 0.f ? value : 0.f;
			float end = value2 >= 0.f ? value2 : value;

			switch (m_direction)
			{
			case Direction::Up:
			{
				spx ofs = (canvas.h - minFillLength) - (canvas.h - minFillLength) * end;
				spx len = (canvas.h - minFillLength) * (end - beg);

				return { canvas.x, canvas.y + ofs, canvas.w, len + minFillLength };
			}

			case Direction::Down:
			{
				spx ofs = (canvas.h - minFillLength) * beg;
				spx len = (canvas.h - minFillLength) * (end - beg);

				return { canvas.x, canvas.y + ofs, canvas.w, len + minFillLength };
			}

			case Direction::Left:
			{
				spx ofs = (canvas.w - minFillLength) - (canvas.w - minFillLength) * end;
				spx len = (canvas.w - minFillLength) * (end - beg);

				return { canvas.x + ofs, canvas.y, len + minFillLength, canvas.h };
			}

			case Direction::Right:
			{
				spx ofs = (canvas.w - minFillLength) * beg;
				spx len = (canvas.w - minFillLength) * (end - beg);

				return { canvas.x + ofs, canvas.y, len + minFillLength, canvas.h };
			}

			default:
				return RectSPX();			// Just to avoid compiler warnings.
			}
		}
	}

	//____ _updateOpacity() ______________________________________________________________

	void FillMeterSkin::_updateOpacity()
	{
		if (m_blendMode == BlendMode::Replace)
			m_bOpaque = true;
		else if (m_blendMode != BlendMode::Blend)
			m_bOpaque = false;
		else
			m_bOpaque = int(m_barColorEmpty.a) + int(m_barColorFull.a) + int(m_backColor.a) == 4096 * 3 ? true : false;
	}


} // namespace wg
