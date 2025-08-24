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
#ifndef WG_SPINMETERSKIN_DOT_H
#define WG_SPINMETERSKIN_DOT_H
#pragma once

#include <wg_skin.h>
#include <wg_color.h>
#include <wg_tintmap.h>
#include <wg_surface.h>

namespace wg
{

	class SpinMeterSkin;

	typedef	StrongPtr<SpinMeterSkin>	SpinMeterSkin_p;
	typedef	WeakPtr<SpinMeterSkin>		SpinMeterSkin_wp;


	class SpinMeterSkin : public Skin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			float		angleBegin = 0.f;
			float		angleEnd = 360.f;

			BlendMode	blendMode = BlendMode::Blend;
			HiColor		color = HiColor::Undefined;
			Size		defaultSize;						// Mandatory

			Finalizer_p	finalizer = nullptr;
			Border		gfxPadding;
			int			layer = -1;
			int			markAlpha = 1;
			Border		overflow;
			Border		padding;

			CoordF		pivot = { 0.5f, 0.5f };
			CoordF		placement = { 0.5f, 0.5f };
			Border		spacing;

			Surface_p	surface;							// Mandatory
			Tintmap_p	tintmap;
			float		zoom = 1.f;
		};

		//.____ Creation __________________________________________

		static SpinMeterSkin_p create(	const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ____________________________________________________

		SizeSPX	_defaultSize(int scale) const override;

		bool	_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state, 
							float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void 	_render(	GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, 
							float value = 1.f, float value2 = -1.f, int animPos = 0,
							float* pStateFractions = nullptr) const override;

		RectSPX	_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
							float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
							float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX	_coverage(const RectSPX& geo, int scale, State state) const override;

	protected:
		SpinMeterSkin(	const Blueprint& blueprint );
		~SpinMeterSkin() {};

		Surface_p	m_pSurface;
		Size		m_defaultSize;
		CoordF		m_pivot;
		CoordF		m_placement;
		float		m_fromDegrees;
		float		m_toDegrees;
		float		m_zoom;
		Border		m_gfxPadding;

		BlendMode		m_blendMode = BlendMode::Blend;
		HiColor			m_color = HiColor::White;
		Tintmap_p		m_pTintmap;
		bool			m_bGradient = false;
	};


} // namespace wg
#endif //WG_SPINMETERSKIN_DOT_H
