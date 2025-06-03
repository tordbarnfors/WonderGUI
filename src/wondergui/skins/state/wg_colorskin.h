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
#ifndef WG_COLORSKIN_DOT_H
#define WG_COLORSKIN_DOT_H
#pragma once

#include <wg_stateskin.h>
#include <wg_color.h>

#include <vector>

namespace wg
{

	class ColorSkin;
	typedef	StrongPtr<ColorSkin>	ColorSkin_p;
	typedef	WeakPtr<ColorSkin>		ColorSkin_wp;

	class ColorSkin : public StateSkin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct StateData
		{
			HiColor			color = HiColor::Undefined;
			Coord			contentShift;
		};

		struct StateBP
		{
			StateBP() {}
			StateBP( State state, StateData data ) : state(state), data(data) {}
			StateBP( State state, HiColor color ) : state(state) { data.color = color; }
			StateBP( State state, Coord contentShift ) : state(state)	{ data.contentShift = contentShift; }
			
			State			state = State::Default;
			StateData		data;
		};

		struct Blueprint
		{
			BlendMode		blendMode = BlendMode::Blend;

			HiColor			color = HiColor::White;
			Finalizer_p		finalizer = nullptr;

			int				layer = -1;
			int				markAlpha = 1;
			Border			overflow;
			Border			padding;
			Border			spacing;

			std::vector<StateBP> states;
		};


		//.____ Creation __________________________________________

		static ColorSkin_p	create( const Blueprint& blueprint );
		static ColorSkin_p create(HiColor color,  Border contentBorder = Border() );

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
		ColorSkin(const Blueprint& blueprint );
		~ColorSkin();

		const HiColor&	_getColor(State state) const
		{
						int idxTabEntry = (state.index() & m_stateColorIndexMask) >> m_stateColorIndexShift;
						int entry = m_pStateColorIndexTab[idxTabEntry];
						return m_pStateColors[entry];
		}

		void *			m_pStateData;

		BlendMode		m_blendMode = BlendMode::Blend;

		uint8_t			m_stateColorIndexMask;
		uint8_t			m_stateColorIndexShift;
		uint8_t*		m_pStateColorIndexTab;		// Table with index values into m_pStateColors for each mode (72) or less.
		HiColor*		m_pStateColors;				// Contains colors for states.
	};


} // namespace wg
#endif //WG_COLORSKIN_DOT_H


