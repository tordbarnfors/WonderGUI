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
#ifndef WG_TINTSKIN_DOT_H
#define WG_TINTSKIN_DOT_H
#pragma once

#include <wg_stateskin.h>
#include <wg_tint.h>

#include <vector>

namespace wg
{

	class TintSkin;
	typedef	StrongPtr<TintSkin>	TintSkin_p;
	typedef	WeakPtr<TintSkin>		TintSkin_wp;

	class TintSkin : public StateSkin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct StateData
		{
			Tint_p		tint;
			Coord			contentShift;
		};

		struct StateBP
		{
			StateBP() {}
			StateBP( State state, StateData data ) : state(state), data(data) {}
			StateBP( State state, Tint * pTint ) : state(state) { data.tint = pTint; }
			StateBP( State state, Coord contentShift ) : state(state)	{ data.contentShift = contentShift; }
			
			State			state = State::Default;
			StateData		data;
		};

		struct Blueprint
		{
			BlendMode		blendMode = BlendMode::Blend;

			Tint_p		tint;
			Finalizer_p		finalizer = nullptr;

			int				layer = -1;
			int				markAlpha = 1;
			Border			overflow;
			Border			padding;
			Border			spacing;

			std::vector<StateBP> states;
		};


		//.____ Creation __________________________________________

		static TintSkin_p	create( const Blueprint& blueprint );
		static TintSkin_p	create( Tint * pTint, Border padding = Border() );

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
		TintSkin(const Blueprint& blueprint );
		~TintSkin();

		const Tint_p	_getTint(State state) const
		{
						int idxTabEntry = (state.index() & m_stateTintIndexMask) >> m_stateTintIndexShift;
						int entry = m_pStateTintIndexTab[idxTabEntry];
						return m_pStateTints[entry];
		}

		void *			m_pStateData;

		BlendMode		m_blendMode = BlendMode::Blend;

		uint8_t			m_stateTintIndexMask;
		uint8_t			m_stateTintIndexShift;
		uint8_t*		m_pStateTintIndexTab;		// Table with index values into m_pStateTints for each mode (72) or less.
		Tint_p*		m_pStateTints;				// Contains tints for states.
		int				m_nbStateTints;				// Needs to know amount to dereference.
	};


} // namespace wg
#endif //WG_TINTSKIN_DOT_H


