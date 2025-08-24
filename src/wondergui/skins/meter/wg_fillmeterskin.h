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
#ifndef WG_FILLMETERSKIN_DOT_H
#define WG_FILLMETERSKIN_DOT_H
#pragma once

#include <wg_skin.h>
#include <wg_color.h>
#include <wg_tintmap.h>

namespace wg
{

	class FillMeterSkin;

	typedef	StrongPtr<FillMeterSkin>	FillMeterSkin_p;
	typedef	WeakPtr<FillMeterSkin>		FillMeterSkin_wp;


	class FillMeterSkin : public Skin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			HiColor		backColor = HiColor::Transparent;
			BlendMode	blendMode = BlendMode::Blend;
			HiColor		color = Color::Blue;
			Size		defaultSize;						// Mandatory
			Direction	direction = Direction::Right;
			Finalizer_p	finalizer = nullptr;
			Border		gfxPadding;

			int			layer = -1;
			int			markAlpha = 1;

			HiColor		maxColor = HiColor::Undefined;		// Overrides barColor when both min/max are set
			HiColor		minColor = HiColor::Undefined;

			Border		overflow;
			Border		padding;
			Border		spacing;
			bool		startFromCenter = false;
			pts			startLength = 0;

			Tintmap_p	tintmap;
		};

		//.____ Creation __________________________________________

		static FillMeterSkin_p create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
        
		//.____ Internal ____________________________________________________

		SizeSPX	_defaultSize(int scale) const override;

		bool	_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int sale, State state, 
							float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void 	_render(	GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, float value = 1.f, 
							float value2 = -1.f, int animPos = 0, float* pStateFractions = nullptr) const override;

		RectSPX	_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
							float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
							float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX	_coverage(const RectSPX& geo, int scale, State state) const override;

	protected:
		FillMeterSkin(	const Blueprint& blueprint );
		~FillMeterSkin() {};

		RectSPX		_barFillArea(const RectSPX& canvas, int scale, float value, float value2) const;
		void		_updateOpacity();
		RectSPX		_valueChangeRect(const RectSPX& canvas, int scale, State state, float oldFraction, float newFraction) const;

		BlendMode	m_blendMode = BlendMode::Blend;
		Direction	m_direction;
        pts		    m_minBarLength;
		HiColor		m_barColorEmpty;
		HiColor		m_barColorFull;
		HiColor		m_backColor;
		Size		m_defaultSize;
        bool        m_bCenteredBarOrigin = false;
		Border		m_gfxPadding;
		
		Tintmap_p	m_pTintmap;
		bool		m_bGradient = false;
		bool		m_bOpaque;
	};


} // namespace wg
#endif //WG_FILLMETERSKIN_DOT_H
