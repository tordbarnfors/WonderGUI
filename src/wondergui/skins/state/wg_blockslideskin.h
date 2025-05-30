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
#ifndef WG_SCROLLSKIN_DOT_H
#define WG_SCROLLSKIN_DOT_H
#pragma once

#include <wg_stateskin.h>
#include <wg_surface.h>
#include <wg_gradient.h>

#include <vector>

namespace wg
{


	class BlockSlideSkin;
	typedef	StrongPtr<BlockSlideSkin>	BlockSlideSkin_p;
	typedef	WeakPtr<BlockSlideSkin>		BlockSlideSkin_wp;


	class BlockSlideSkin : public StateSkin
	{
		//TODO: Add sanity-checking to all Set-methods.
		//TODO: Optimize rendering based on invisibleSections and opaqueSections!

	public:

		//____ Blueprint ______________________________________________________

		struct StateData
		{
			bool			blockless = false;
			HiColor			color = HiColor::Undefined;
			Coord			contentShift;
		};

		struct StateBP
		{
			StateBP() {}
			StateBP( State state ) : state(state) {}
			StateBP( State state, StateData data ) : state(state), data(data) {}
			StateBP( State state, HiColor color, bool blockless = false ) : state(state) { data.color = color; data.blockless = blockless; }
			StateBP( State state, Coord contentShift, bool blockless = false ) : state(state)	{ data.contentShift = contentShift; data.blockless = blockless; }
			
			State			state = State::Default;
			StateData		data;
		};

		struct Blueprint
		{

			BlendMode		blendMode = BlendMode::Blend;

			pts				blockLength;					// Required!
			pts				blockSpacing = 0;
			HiColor			color = HiColor::Undefined;

			Finalizer_p		finalizer = nullptr;
			Rect			firstLane;
			Gradient		gradient;
			int				layer = -1;
			int				markAlpha = 1;
			Border			overflow;
			Border			padding;

			Direction		slideDirection = Direction::Right;
			int				slideDuration = 250;
			PrimState		slideState = PrimState::Selected;

			Border			spacing;
			std::vector<StateBP> states;
			Surface_p		surface;
		};

		//.____ Creation __________________________________________

		static BlockSlideSkin_p	create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Internal _________________________________________________

		SizeSPX		_defaultSize(int scale) const override;

		bool		_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state,
								float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void		_render(	GfxDevice* pDevice, const RectSPX& canvas, int scale, State state,
								float value = 1.f, float value2 = -1.f, int animPos = 0,
								float* pStateFractions = nullptr) const override;

		RectSPX		_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
								float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
								float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX		_coverage(const RectSPX& geo, int scale, State state) const override;
		
		Bitmask<uint8_t>	_transitioningStates() const override;
		const int*			_transitionTimes() const override;

	protected:

		BlockSlideSkin();
		BlockSlideSkin( const Blueprint& blueprint);
		~BlockSlideSkin() {};

		void		_updateOpaqueFlags();
		void		_updateUnsetStateBlocks();
		void		_updateUnsetStateColors();

		RectSPX		_partInCanvas(int scale, State state, float* pStateFractions) const;

		Surface_p	m_pSurface;
		Direction	m_slideDirection = Direction::Right;
		Size		m_blockSize;
		pts			m_slideDistance = 0;
		int			m_slideDuration = 250;						// Millisec
		PrimState	m_slideState = PrimState::Selected;

		Gradient	m_gradient;

		BlendMode	m_blendMode;

		Bitmask<uint32_t>	m_stateBlockMask = 1;
		Bitmask<uint32_t>	m_stateColorMask = 1;

		Coord		m_stateBlocks[State::NbStates];
		HiColor		m_stateColors[State::NbStates];
		bool		m_bStateOpaque[State::NbStates];

		int			m_transitionTimes[PrimState_Nb] = { 0,0,0,0,0,0,0,0 };
	};


} // namespace wg
#endif //WG_BLOCKSKIN_DOT_H
