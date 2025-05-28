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
#ifndef WG_MULTIBLOCKSKIN_DOT_H
#define WG_MULTIBLOCKSKIN_DOT_H
#pragma once

#include <wg_stateskin.h>
#include <wg_surface.h>

#include <initializer_list>
#include <utility>
#include <vector>

namespace wg
{

	class MultiBlockSkin;
	typedef	StrongPtr<MultiBlockSkin>	MultiBlockSkin_p;
	typedef	WeakPtr<MultiBlockSkin>	MultiBlockSkin_wp;



	class MultiBlockSkin : public StateSkin
	{
		//TODO: Add sanity-checking to all Set-methods.
		//TODO: Optimize rendering based on invisibleSections and opaqueSections!

	public:

		//.____ Creation __________________________________________

		static MultiBlockSkin_p create(Size blockSize, Border frame = Border(0) );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Appearance _________________________________________________

		int		addLayer(Surface * pSurf, Coord ofs);
		int		addLayer(Surface * pSurf, Coord blockStartOfs, Size blockPitch, std::initializer_list<State> stateBlocks);		// DEPRECATED!!!
		int		addLayer(Surface * pSurf, std::initializer_list<State> stateBlocks, Axis axis = Axis::Y, pts spacing = 0, Coord blockStartOfs = { 0,0 } );

		bool	setLayerColor(int layerIdx, HiColor tintColor);
		bool	setLayerColor(int layerIdx, std::initializer_list< std::pair<State,HiColor> > stateColors);
		bool	setLayerBlendMode(int layerIdx, BlendMode blendMode);

		//.____ Internal _________________________________________________

		SizeSPX	_minSize(int scale) const override;
		SizeSPX	_defaultSize(int scale) const override;
		SizeSPX	_sizeForContent(const SizeSPX& contentSize, int scale) const override;

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

		struct LayerData
		{
			Surface *			pSurface;
			BlendMode			blendMode;
			Bitmask<uint32_t>   stateBlockMask = 0;
			Bitmask<uint32_t>   stateColorMask = 1;         // Normal state always exist for colors and is by default white.

			Coord				blockOfs[State::NbStates];		// Block for each state
			HiColor				tintColor[State::NbStates];
	//		Placement			placementOrigo;
	//		CoordI			placementOfs;
	//		SizeI			dimensions;                   // Stored in pixels
	//		int				m_tiledSections;
		};


		MultiBlockSkin(Size blockSize, Border frame);
		~MultiBlockSkin() {};

		void			_updateStateOpacity( int stateIdx );

		Size			m_blockSizePoints;
		Border			m_frame;

		std::vector<LayerData>	m_layers;

		bool			m_bStateOpaque[State::NbStates];

	};

}

#endif //WG_MULTIBLOCKSKIN_DOT_H
