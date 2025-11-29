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
#ifndef WG_TINTMAPSKIN_DOT_H
#define WG_TINTMAPSKIN_DOT_H
#pragma once

#include <wg_stateskin.h>
#include <wg_tintmap.h>

#include <vector>

namespace wg
{

	class TintmapSkin;
	typedef	StrongPtr<TintmapSkin>	TintmapSkin_p;
	typedef	WeakPtr<TintmapSkin>		TintmapSkin_wp;

	class TintmapSkin : public StateSkin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct StateData
		{
			Tintmap_p		tintmap;
			Coord			contentShift;
		};

		struct StateBP
		{
			StateBP() {}
			StateBP( State state, StateData data ) : state(state), data(data) {}
			StateBP( State state, Tintmap * pTintmap ) : state(state) { data.tintmap = pTintmap; }
			StateBP( State state, Coord contentShift ) : state(state)	{ data.contentShift = contentShift; }
			
			State			state = State::Default;
			StateData		data;
		};

		struct Blueprint
		{
			BlendMode		blendMode = BlendMode::Blend;

			Tintmap_p		tintmap;
			Finalizer_p		finalizer = nullptr;

			int				layer = -1;
			int				markAlpha = 1;
			Border			overflow;
			Border			padding;
			Border			spacing;

			std::vector<StateBP> states;
		};


		//.____ Creation __________________________________________

		static TintmapSkin_p	create( const Blueprint& blueprint );
		static TintmapSkin_p	create( Tintmap * pTintmap, Border padding = Border() );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ____________________________________________________

		bool		_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state,
								float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void		_render(	GfxDevice* pDevice, const RectSPX& canvas, int scale, State state,
							float value = 1.f, float value2 = -1.f, int animPos = 0,
							float* pStateFractions = nullptr) const override;

		RectSPX	_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
							float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
							float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX	_coverage(const RectSPX& geo, int scale, State state) const override;

	protected:
		TintmapSkin(const Blueprint& blueprint );
		~TintmapSkin();

		const Tintmap_p	_getTintmap(State state) const
		{
						int idxTabEntry = (state.index() & m_stateTintmapIndexMask) >> m_stateTintmapIndexShift;
						int entry = m_pStateTintmapIndexTab[idxTabEntry];
						return m_pStateTintmaps[entry];
		}

		void *			m_pStateData;

		BlendMode		m_blendMode = BlendMode::Blend;

		uint8_t			m_stateTintmapIndexMask;
		uint8_t			m_stateTintmapIndexShift;
		uint8_t*		m_pStateTintmapIndexTab;		// Table with index values into m_pStateTintmaps for each mode (72) or less.
		Tintmap_p*		m_pStateTintmaps;				// Contains tintmaps for states.
		int				m_nbStateTintmaps;				// Needs to know amount to dereference.
	};


} // namespace wg
#endif //WG_TINTMAPSKIN_DOT_H


