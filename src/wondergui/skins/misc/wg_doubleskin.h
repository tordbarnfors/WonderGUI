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
#ifndef WG_DOUBLESKIN_DOT_H
#define WG_DOUBLESKIN_DOT_H
#pragma once

#include <wg_skin.h>
#include <wg_color.h>

#include <initializer_list>
#include <utility>

namespace wg
{

	class DoubleSkin;
	typedef	StrongPtr<DoubleSkin>	DoubleSkin_p;
	typedef	WeakPtr<DoubleSkin>		DoubleSkin_wp;

	class DoubleSkin : public Skin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Finalizer_p	finalizer = nullptr;
			int			layer = -1;
			int			markAlpha = 1;
			Border		overflow;
			Border		padding;

			bool		skinInSkin = true;
			Skin_p		skins[2];
			Border		spacing;
		};

		//.____ Creation __________________________________________

		static DoubleSkin_p create( const Blueprint& blueprint );
		static DoubleSkin_p create( Skin * pFrontSkin, Skin * pBackSkin, bool bSkinInSkin = true );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		Skin_p			frontSkin() const { return m_pFrontSkin; }
		Skin_p			backSkin() const { return m_pBackSkin; }

		//.____ Internal ____________________________________________________

		SizeSPX			_minSize(int scale) const override;
		SizeSPX			_defaultSize(int scale) const override;
		SizeSPX			_sizeForContent(const SizeSPX& contentSize, int scale) const override;

		BorderSPX		_padding(int scale, State state) const override;
		SizeSPX			_paddingSize(int scale) const override;

		CoordSPX		_contentOfs(int scale, State state) const override;
		RectSPX			_contentRect(const RectSPX& canvas, int scale, State state) const override;
		BorderSPX		_contentBorder(int scale, State state) const override;
		SizeSPX			_contentBorderSize(int scale) const override;

		bool			_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state, 
									float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void			_render(	GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, 
									float value = 1.f, float value2 = -1.f, int animPos = 0, float* pStateFractions = nullptr) const override;

		BorderSPX		_geoOverflow(int scale) const override;
		RectSPX			_coverage(const RectSPX& geo, int scale, State state) const override;

		RectSPX			_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
									float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
									float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		int				_animationLength(State state) const override;

		Bitmask<uint8_t>_transitioningStates() const override;
		const int*		_transitionTimes() const override;

	protected:
		DoubleSkin( const Blueprint& blueprint );
		DoubleSkin(Skin * pFrontSkin, Skin * pBackSkin, bool bSkinInSkin = true);
		~DoubleSkin();

		void		_incUseCount() override;
		void		_decUseCount() override;
		void		_onModified();

		Skin_p		m_pFrontSkin;
		Skin_p		m_pBackSkin;
		bool		m_bSkinInSkin = true;
		bool		m_bContentPaddingSet = false;

		int			m_transitionTimes[BaseState_Nb];
	};


} // namespace wg
#endif //WG_BOXSKIN_DOT_H


