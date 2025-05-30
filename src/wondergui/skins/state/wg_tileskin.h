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
#ifndef WG_TILESKIN_DOT_H
#define WG_TILESKIN_DOT_H
#pragma once

#include <wg_stateskin.h>
#include <wg_surface.h>
#include <wg_gradient.h>

#include <vector>

namespace wg
{


	class TileSkin;
	typedef	StrongPtr<TileSkin>	TileSkin_p;
	typedef	WeakPtr<TileSkin>		TileSkin_wp;


	struct _TileSkinStateData
	{

		Coord		contentShift;
		Surface_p	surface;
		HiColor		color;
		bool		m_bOpaque;
	};




	class TileSkin : public StateSkin<_TileSkinStateData>
	{
		//TODO: Add sanity-checking to all Set-methods.
		//TODO: Optimize rendering based on invisibleSections and opaqueSections!

	public:

		//____ Blueprint ______________________________________________________

		struct StateData
		{
			HiColor			color = HiColor::Undefined;
			Coord			contentShift;
			Surface_p		surface;
		};

		struct StateBP
		{

			StateBP() {}
			StateBP( State state ) : state(state) {}
			StateBP( State state, StateData data ) : state(state), data(data) {}
			StateBP( State state, HiColor color ) : state(state) { data.color = color; }
			StateBP( State state, Coord contentShift ) : state(state)	{ data.contentShift = contentShift; }
			StateBP( State state, Surface_p surface ) : state(state)	{ data.surface = surface; }

			State			state = State::Default;
			StateData		data;
		};

		struct Blueprint
		{
			BlendMode		blendMode = BlendMode::Blend;
			HiColor			color = HiColor::Undefined;
			Finalizer_p		finalizer = nullptr;
			Gradient		gradient;
			int				layer = -1;
			int				markAlpha = 1;
			Border			overflow;
			Border			padding;
			Border			spacing;

			std::vector<StateBP>	states;
			Surface_p		surface;
		};

		//.____ Creation __________________________________________

		static TileSkin_p	create(Surface * pSurface, HiColor color = HiColor::Undefined, int layer = -1 );
		static TileSkin_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal _________________________________________________

		SizeSPX	_defaultSize(int scale) const override;

		bool	_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state, 
							float value = 1.f, float value2 = -1.f, int alphaOverride = -1 ) const override;

		void	_render(	GfxDevice * pDevice, const RectSPX& canvas, int scale, State state, 
							float value = 1.f, float value2 = -1.f, int animPos = 0,
							float* pStateFractions = nullptr) const override;

		RectSPX	_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
							float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
							float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX	_coverage(const RectSPX& geo, int scale, State state) const override;

	protected:

		TileSkin(const Blueprint& blueprint);
		~TileSkin() {};

		void		_updateOpaqueFlags();
		void		_updateUnsetStateSurfaces();
		void		_updateUnsetStateColors();

		BlendMode	m_blendMode;
		Gradient	m_gradient;
	};


} // namespace wg
#endif //WG_TILESKIN_DOT_H
