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
#ifndef WG_CIRCLESKIN_DOT_H
#define WG_CIRCLESKIN_DOT_H
#pragma once


#include <wg_stateskin.h>
#include <wg_color.h>

#include <vector>

namespace wg
{

	class CircleSkin;
	typedef	StrongPtr<CircleSkin>	CircleSkin_p;
	typedef	WeakPtr<CircleSkin>		CircleSkin_wp;

	class CircleSkin : public StateSkin
	{
	public:

		//____ Blueprint ______________________________________________________

		struct StateData
		{
			HiColor			color = HiColor::Undefined;
			Coord			contentShift;

			HiColor			outlineColor = HiColor::Undefined;
			pts				outlineThickness = -1;

			float			size = -1;
			pts				thickness = -1;
		};

		struct StateBP
		{
			StateBP() {}
			StateBP( State state, StateData data ) : state(state), data(data) {}
			StateBP( State state, HiColor color ) : state(state) { data.color = color; }
			StateBP( State state, Coord contentShift ) : state(state)	{ data.contentShift = contentShift; }

			StateBP( State state, HiColor color, HiColor outlineColor ) : state(state)
			{
				data.color = color;
				data.outlineColor = outlineColor;
			}

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

			HiColor			outlineColor = HiColor::Black;
			pts				outlineThickness = 0;
			Border			overflow;
			Border			padding;
			float			size = 1;
			Border			spacing;

			std::vector<StateBP>	states;

			pts				thickness = 1;
		};

		//.____ Creation __________________________________________

		static CircleSkin_p	create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ____________________________________________________

		bool		_markTest(const CoordSPX& ofs, const RectSPX& canvas, int scale, State state,
								float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void		_render(GfxDevice* pDevice, const RectSPX& canvas, int scale, State state,
							float value = 1.f, float value2 = -1.f, int animPos = 0,
							float* pStateFractions = nullptr) const override;

		RectSPX		_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
								float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
								float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX		_coverage(const RectSPX& geo, int scale, State state) const override;

	protected:
		CircleSkin( const Blueprint& blueprint );
		~CircleSkin();

		const HiColor& _getColor(State state) const
		{
			int idxTabEntry = (state.index() & m_colorIndexMask) >> m_colorIndexShift;
			int entry = m_pColorIndexTab[idxTabEntry];
			return m_pColors[entry];
		}

		const float _getSize(State state) const
		{
			int idxTabEntry = (state.index() & m_sizeIndexMask) >> m_sizeIndexShift;
			int entry = m_pSizeIndexTab[idxTabEntry];
			return m_pSizes[entry];
		}

		const pts _getThickness(State state) const
		{
			int idxTabEntry = (state.index() & m_thicknessIndexMask) >> m_thicknessIndexShift;
			int entry = m_pThicknessIndexTab[idxTabEntry];
			return m_pThickness[entry];
		}

		const HiColor& _getOutlineColor(State state) const
		{
			int idxTabEntry = (state.index() & m_outlineColorIndexMask) >> m_outlineColorIndexShift;
			int entry = m_pOutlineColorIndexTab[idxTabEntry];
			return m_pOutlineColors[entry];
		}

		const pts _getOutlineThickness(State state) const
		{
			int idxTabEntry = (state.index() & m_outlineThicknessIndexMask) >> m_outlineThicknessIndexShift;
			int entry = m_pOutlineThicknessIndexTab[idxTabEntry];
			return m_pOutlineThickness[entry];
		}

		BlendMode	m_blendMode = BlendMode::Blend;

		void*		m_pStateData;				// Pointer at memory block with state data.

		uint8_t		m_colorIndexMask;
		uint8_t		m_colorIndexShift;
		uint8_t*	m_pColorIndexTab;
		HiColor*	m_pColors;

		uint8_t		m_sizeIndexMask;
		uint8_t		m_sizeIndexShift;
		uint8_t*	m_pSizeIndexTab;
		pts*		m_pSizes;

		uint8_t		m_thicknessIndexMask;
		uint8_t		m_thicknessIndexShift;
		uint8_t*	m_pThicknessIndexTab;
		pts*		m_pThickness;

		uint8_t		m_outlineThicknessIndexMask;
		uint8_t		m_outlineThicknessIndexShift;
		uint8_t*	m_pOutlineThicknessIndexTab;
		pts*		m_pOutlineThickness;

		uint8_t		m_outlineColorIndexMask;
		uint8_t		m_outlineColorIndexShift;
		uint8_t*	m_pOutlineColorIndexTab;
		HiColor*	m_pOutlineColors;
	};


} // namespace wg
#endif //WG_CIRCLESKIN_DOT_H


