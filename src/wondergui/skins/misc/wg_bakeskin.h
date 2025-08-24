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
#ifndef WG_BAKESKIN_DOT_H
#define WG_BAKESKIN_DOT_H
#pragma once

#include <wg_surface.h>
#include <wg_skin.h>
#include <wg_color.h>
#include <wg_tintmap.h>

#include <initializer_list>
#include <utility>
#include <vector>

namespace wg
{
	class BakeSkin;
	typedef	StrongPtr<BakeSkin>	BakeSkin_p;
	typedef	WeakPtr<BakeSkin>		BakeSkin_wp;

	class BakeSkin : public Skin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			BlendMode	blendMode = BlendMode::Undefined;
			HiColor		color = HiColor::Undefined;
			int			layer = -1;
			Border		padding;

			bool		skinInSkin = true;
			std::vector<Skin_p>	skins;

			Surface_p	surface;
			Tintmap_p	tintmap;
		};

		//.____ Creation __________________________________________

		static BakeSkin_p	create( const Blueprint& blueprint );
		static BakeSkin_p	create( Surface * pBakeSurface, std::initializer_list<Skin_p> skins );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal _________________________________________________

		SizeSPX			_minSize(int scale) const override;
		SizeSPX			_defaultSize(int scale) const override;

//		BorderSPX		_padding(int scale, State state) const override;
//		SizeSPX			_paddingSize(int scale) const override;
		CoordSPX		_contentOfs(int scale, State state) const override;
		RectSPX			_contentRect(const RectSPX& canvas, int scale, State state) const override;

		bool			_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state, 
									float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void			_render(	GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, 
									float value = 1.f, float value2 = -1.f, int animPos = 0, float* pStateFractions = nullptr) const override;

		RectSPX			_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
									float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
									float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX			_coverage(const RectSPX& geo, int scale, State state) const override;
		
		int				_animationLength(State state) const override;

		Bitmask<uint8_t>_transitioningStates() const override;
		const int*		_transitionTimes() const override;


	protected:
		BakeSkin(const Blueprint& blueprint);
		BakeSkin(Surface* pBakeSurface, std::initializer_list<Skin_p> skins);
		~BakeSkin();

		void			_incUseCount() override;
		void			_decUseCount() override;

		void			_updateCachedGeo(int scale) const;
		void			_onModified();
		BorderSPX		_stateContentBorder(int scale, State state) const;

		std::vector<Skin_p>	m_skins;

		Surface_p			m_pBakeSurface;
		BlendMode			m_blendMode = BlendMode::Undefined;
		HiColor				m_tintColor = HiColor::White;
		Tintmap_p			m_pTintmap;
		bool				m_bContentPaddingSet = false;
		bool				m_bSkinInSkin = false;

		mutable int			m_cachedScale = 0;
		mutable SizeSPX		m_cachedMinSize;						// Calculated minSize for scale represented by m_cachedScale;
		mutable SizeSPX		m_cachedDefaultSize;					// Calculated defaultSize for scale represented by m_cachedScale;
		mutable BorderSPX	m_cachedContentPadding[State::NbStates];

		Bitmask<uint8_t>	m_transitioningStates;
		int					m_transitionTimes[PrimState_Nb];

		int					m_animationLengths[State::NbStates];
	};

} // namespace wg
#endif //WG_BAKESKIN_DOT_H


