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

#include <wg_spinanimskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_base.h>
#include <wg_skin.impl.h>

#include <cmath>

namespace wg
{

	const TypeInfo SpinAnimSkin::TYPEINFO = { "SpinAnimSkin", &Skin::TYPEINFO };

	using namespace Util;

	//____ create() _______________________________________________________________

	SpinAnimSkin_p SpinAnimSkin::create( const Blueprint& blueprint )	
	{
		return SpinAnimSkin_p(new SpinAnimSkin(blueprint));
	}

	//____ constructor ____________________________________________________________

	SpinAnimSkin::SpinAnimSkin(	const Blueprint& bp) : AnimSkin(bp),
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
		m_blendMode(bp.blendMode),
		m_pTransition(bp.transition),
		m_pReturnTransition(bp.returnTransition)
	{
		//TODO: Also take frame opacity into account.
	
		if( m_pTransition == nullptr )
			m_pTransition = ValueTransition::create(1000000);
		
		m_cycleDuration = m_pTransition->duration();
		if( m_pReturnTransition )
			m_cycleDuration += m_pReturnTransition->duration();
				
	}

	//____ destructor _________________________________________________________

	SpinAnimSkin::~SpinAnimSkin()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SpinAnimSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _render() ______________________________________________________________

	void SpinAnimSkin::_render(GfxDevice* pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float* pStateFractions) const
	{
		float	zoom = m_zoom * scale / 64.f;

		// Scale zoom to fit content of default size into canvas size.

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if (!m_defaultSize.isEmpty())
		{
			SizeSPX prefSize = align(ptsToSpx(m_defaultSize, scale));
			if (canvas.size() != prefSize)
			{
				float xScale = float(canvas.w) / float(prefSize.w);
				float yScale = float(canvas.h) / float(prefSize.h);
				float scaleSrc = std::min(xScale, yScale);

				spx w = prefSize.w * scaleSrc;
				spx h = prefSize.h * scaleSrc;

				canvas.x += (w - canvas.w) / 2;
				canvas.y += (h - canvas.h) / 2;
				canvas.w = w;
				canvas.h = h;

				zoom *= scaleSrc;
			}
		}

		//

		canvas -= align(ptsToSpx(m_gfxPadding, scale));

		float	degrees;
		animPos = (animPos*1000) % m_cycleDuration;

		if( animPos < m_pTransition->duration() )
		{
			degrees = m_pTransition->snapshot(animPos, m_fromDegrees, m_toDegrees);
		}
		else
		{
			// We do have a return transition if m_cycleDuration is larger than m_pTransition->duration().

			animPos -= m_pTransition->duration();
			degrees = m_pReturnTransition->snapshot(animPos, m_toDegrees, m_fromDegrees);
		}


		if (degrees < 0.f)
			degrees = 360.f + (float)fmod(degrees, 360.f);
		else if (degrees >= 360.f)
			degrees = (float)fmod(degrees, 360.f);

		RenderSettingsWithTintmap settings(pDevice, m_layer, m_blendMode, m_color, canvas, m_pTintmap);

		pDevice->setBlitSource(m_pSurface);
		pDevice->rotScaleBlit(_canvas, degrees, zoom, m_pivot, m_placement);
	}

	//____ _defaultSize() ______________________________________________________________

	SizeSPX SpinAnimSkin::_defaultSize(int scale) const
	{
		return SizeSPX(align(ptsToSpx(m_spacing, scale))) + align(ptsToSpx(m_defaultSize, scale));
	}

	//____ _markTest() _________________________________________________________

	bool SpinAnimSkin::_markTest(const CoordSPX& ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride ) const
	{
		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));

		if (!canvas.contains(ofs))
			return false;

		canvas += align(ptsToSpx(m_overflow, scale));
		

		//TODO: Implement!

		return true;
	}

	//____ _coverage() ___________________________________________________________

	RectSPX SpinAnimSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		//TOOD: Implement!
		
		return RectSPX();
	}


	//____ _dirtyRect() ________________________________________________________

	RectSPX SpinAnimSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (newAnimPos == oldAnimPos)
			return RectSPX();

		return _canvas - align(ptsToSpx(m_spacing, scale))  + align(ptsToSpx(m_overflow, scale));
	}

	//____ _animationLength() __________________________________________________

	int SpinAnimSkin::_animationLength(State state) const
	{
		return m_cycleDuration;
	}


} // namespace wg
