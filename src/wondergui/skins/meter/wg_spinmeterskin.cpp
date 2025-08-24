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

#include <wg_spinmeterskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_base.h>
#include <wg_skin.impl.h>

#include <cmath>

namespace wg
{

	const TypeInfo SpinMeterSkin::TYPEINFO = { "SpinMeterSkin", &Skin::TYPEINFO };

	using namespace Util;

	//____ create() _______________________________________________________________

	SpinMeterSkin_p SpinMeterSkin::create(	const Blueprint& blueprint )
	{
		return SpinMeterSkin_p(new SpinMeterSkin(blueprint));
	}

	//____ constructor ____________________________________________________________

	SpinMeterSkin::SpinMeterSkin( const Blueprint& bp ) : Skin(bp),
		m_pSurface(bp.surface),
		m_defaultSize(bp.defaultSize),
		m_pivot(bp.pivot),
		m_placement(bp.placement),
		m_fromDegrees(bp.angleBegin),
		m_toDegrees(bp.angleEnd),
		m_zoom(bp.zoom),
		m_gfxPadding(bp.gfxPadding),
		m_color(bp.color),
		m_pTintmap(bp.tintmap),
		m_blendMode(bp.blendMode)
	{
		//TODO: Also take frame opacity into account.

		m_bIgnoresValue = false;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SpinMeterSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() ______________________________________________________________

	void SpinMeterSkin::_render(GfxDevice* pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		float	zoom = m_zoom * scale / 64.f;

		// Scale zoom to fit content of default size into canvas size.


		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
		if (!m_defaultSize.isEmpty() )
		{
			SizeSPX defaultSize = align(ptsToSpx(m_defaultSize, scale));
			if (canvas.size() != defaultSize)
			{
				float xScale = float(canvas.w) / float(defaultSize.w);
				float yScale = float(canvas.h) / float(defaultSize.h);
				float scaleSrc = std::min(xScale, yScale);

				spx w = defaultSize.w * scaleSrc;
				spx h = defaultSize.h * scaleSrc;

				canvas.x += (w - canvas.w) / 2;
				canvas.y += (h - canvas.h) / 2;
				canvas.w = w;
				canvas.h = h;

				zoom *= scaleSrc;
			}
		}

		//

		canvas -= align(ptsToSpx(m_gfxPadding, scale));

		float	degrees = m_fromDegrees + (m_toDegrees - m_fromDegrees) * value;

		if (degrees < 0.f)
			degrees = 360.f + (float)fmod(degrees, 360.f);
		else if (degrees >= 360.f)
			degrees = (float)fmod(degrees, 360.f);

		RenderSettingsWithTintmap settings(pDevice, m_layer, m_blendMode, m_color, canvas, m_pTintmap);

		pDevice->setBlitSource(m_pSurface);
		pDevice->rotScaleBlit(canvas, degrees, zoom, m_pivot, m_placement);
	}

	//____ _defaultSize() ______________________________________________________________

	SizeSPX SpinMeterSkin::_defaultSize(int scale) const
	{
		return align(ptsToSpx(m_defaultSize, scale)) +  align(ptsToSpx(m_spacing, scale)).size();
	}

	//____ _markTest() _________________________________________________________

	bool SpinMeterSkin::_markTest(const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if (!canvas.contains(ofs))
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		
		//TODO: Implement!

		return true;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX SpinMeterSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		//TOOD: Implement!
		
		return RectSPX();
	}


	//____ _dirtyRect() ________________________________________________________

	RectSPX SpinMeterSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (newValue != oldValue)
			return canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		return RectSPX();
	}

} // namespace wg
