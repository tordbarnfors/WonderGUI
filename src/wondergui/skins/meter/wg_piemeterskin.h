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
#ifndef WG_PIEMETERSKIN_DOT_H
#define WG_PIEMETERSKIN_DOT_H
#pragma once

#include <wg_skin.h>
#include <wg_color.h>

namespace wg
{

	class PieMeterSkin;

	typedef	StrongPtr<PieMeterSkin>	PieMeterSkin_p;
	typedef	WeakPtr<PieMeterSkin>	PieMeterSkin_wp;


	class PieMeterSkin : public Skin
	{
	public:

		const static int c_maxSlices = 12;

		//____ Blueprint ______________________________________________________

		struct Slice
		{
			float	size = -1.f;
			HiColor	minColor;
			HiColor	maxColor;
		};

		struct Blueprint
		{
			HiColor		backColor = HiColor::Transparent;

			BlendMode	blendMode = BlendMode::Blend;
			Size		defaultSize;
			HiColor		emptyColor = Color::DarkBlue;

			Finalizer_p	finalizer = nullptr;
			Border		gfxPadding;

			HiColor		hubColor = HiColor::Transparent;
			float		hubSize = 0.75f;

			int			layer = -1;
			int			markAlpha = 1;

			float		length = 360;
			float		min = 0.05f;

			bool		movingSlices = false;

			Border		overflow;
			Border		padding;

			bool		square = false;

			float		rotation = 0.f;

			Slice		slices[c_maxSlices];
			Border		spacing;

			bool		startFromCenter = false;

		};

		//.____ Creation __________________________________________

		static PieMeterSkin_p create( const Blueprint& bp );

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ____________________________________________________

		//TODO: We probably need all kind of size related methods, taking set default size and content + padding into account.

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
		PieMeterSkin( const Blueprint& bp );
		~PieMeterSkin() {};

		void		_updateOpacity();

		Size		m_defaultSize;

		float		m_rangeStart = 0.55f;
		float		m_minRange = 0.01f;
		float		m_maxRange = 0.9f;
		float		m_hubSize = 0.75f;
		bool		m_bRectangular = false;
		bool		m_bStaticSections = true;
		bool		m_bStartFromCenter = false;

		Border		m_gfxPadding;

		BlendMode	m_blendMode = BlendMode::Blend;
		HiColor		m_hubColor = HiColor::Transparent;
		HiColor		m_backColor = HiColor::Transparent;
		HiColor		m_emptyColor = Color::DarkBlue;

		Slice		m_slices[c_maxSlices];
		int			m_nSlices;
		
		bool		m_bOpaque;
	};


} // namespace wg
#endif //WG_PIEMETERSKIN_DOT_H
