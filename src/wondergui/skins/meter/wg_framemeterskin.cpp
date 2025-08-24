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

#include <wg_framemeterskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

#include <algorithm>

namespace wg
{

	const TypeInfo FrameMeterSkin::TYPEINFO = { "FrameMeterSkin", &Skin::TYPEINFO };

	using namespace Util;

	//____ create() _______________________________________________________________

	FrameMeterSkin_p FrameMeterSkin::create( const Blueprint& bp )
	{
		return FrameMeterSkin_p(new FrameMeterSkin(bp));
	}

	//____ constructor ____________________________________________________________

	FrameMeterSkin::FrameMeterSkin( const Blueprint& bp) : Skin(bp)
	{
		m_blendMode			= bp.blendMode;
		m_color				= bp.color;
		m_gfxPadding		= bp.gfxPadding;
		m_pTintmap			= bp.tintmap;
		m_size				= bp.size;
		m_pSurface			= bp.surface;

		m_frames.reserve(bp.frames.size());
		
		int timestamp = 0;
		for (auto& frame : bp.frames)
		{
			m_frames.push_back({frame.pos, frame.duration, timestamp, frame.flip });
			timestamp += frame.duration;
		}

		m_frames.shrink_to_fit();

		m_duration = timestamp;
		m_bIgnoresValue = false;
		_updateOpacityFlag();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FrameMeterSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _defaultSize() ______________________________________________________________

	SizeSPX FrameMeterSkin::_defaultSize(int scale) const
	{
		return SizeSPX::max(align(ptsToSpx(m_size,scale)), align(ptsToSpx(m_padding,scale))) + align(ptsToSpx(m_spacing, scale)).size();
	}

	//____ _minSize() ______________________________________________________________

	SizeSPX FrameMeterSkin::_minSize(int scale) const
	{
		return SizeSPX::max(SizeSPX(align(ptsToSpx(m_gfxPadding,scale))), SizeSPX(align(ptsToSpx(m_padding,scale)))) + align(ptsToSpx(m_spacing, scale)).size();
	}


	//____ _render() ______________________________________________________________

	void FrameMeterSkin::_render(GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		//TODO: Support flip!

		auto pFrame = _valueToFrame(value);
		if (pFrame)
		{
			RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
			
			RenderSettingsWithTintmap settings(pDevice, m_layer, m_blendMode, m_color, canvas, m_pTintmap);

			pDevice->setBlitSource(m_pSurface);

			NinePatch patch;
			patch.block = Rect(pFrame->pos, m_size);
			patch.frame = m_gfxPadding;
			pDevice->blitNinePatch(canvas, align(ptsToSpx(m_gfxPadding,scale)), patch,scale);
		}
	}

	//____ _markTest() _________________________________________________________

	bool FrameMeterSkin::_markTest(const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		//TODO: Support flip!
		//TODO: Support tint!

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if (!canvas.contains(ofs))
			return false;
		
		canvas += align(ptsToSpx(m_overflow, scale));

		auto pFrame = _valueToFrame(value);
		if (pFrame)
		{
			NinePatch patch;
			patch.block = Rect(pFrame->pos, m_size);
			patch.frame = m_gfxPadding;
			
			int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

			return Util::markTestNinePatch(ofs, m_pSurface, patch, canvas, scale, alpha);
		}

		return false;
	}

	//____ _dirtyRect() ________________________________________________________

	RectSPX FrameMeterSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (newValue == oldValue)
			return RectSPX();

		auto pOldFrame = _valueToFrame(oldValue);
		auto pNewFrame = _valueToFrame(newValue);

		if (pOldFrame == pNewFrame)
			return RectSPX();
		else
			return canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
	}

	//____ _coverage() _______________________________________________________

	RectSPX FrameMeterSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		if (m_bOpaque)
			return geo - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		else
			return RectSPX();
	}

	//____ _updateOpacityFlag() _______________________________________________

	void FrameMeterSkin::_updateOpacityFlag()
	{
		if (m_blendMode == BlendMode::Replace)
			m_bOpaque = true;
		else if (m_blendMode == BlendMode::Blend)
		{
			if ((!m_pTintmap && !m_pTintmap->isOpaque()) || m_color.a != 4096)
				m_bOpaque = false;
			else
				m_bOpaque = m_pSurface->isOpaque();
		}
		else
			m_bOpaque = false;
	}

	//____ _valueToFrame() _________________________________________________

	const FrameMeterSkin::Frame* FrameMeterSkin::_valueToFrame(float fraction) const
	{
		int timestamp = int(fraction * m_duration);

		const Frame* pFirst = &m_frames.front();
		const Frame* pLast = &m_frames.back();
		const Frame* pFrame = pFirst + (pLast - pFirst);

		if (timestamp < pFirst->duration)
			return pFirst;

		if (timestamp >= pLast->timestamp)
			return pLast;

		while (true)
		{
			if (timestamp < pFrame->timestamp)
			{
				pLast = pFrame;
				pFrame -= (pFrame - pFirst + 1) / 2;
			}
			else if (timestamp < pFrame->timestamp + pFrame->duration)
				return pFrame;
			else
			{
				pFirst = pFrame;
				pFrame += (pLast - pFrame + 1) / 2;
			}
		}
	}

} // namespace wg
