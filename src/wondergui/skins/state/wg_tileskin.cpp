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

#include <wg_tileskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>
#include <wg_skin.impl.h>

namespace wg
{

	using namespace Util;

	const TypeInfo TileSkin::TYPEINFO = { "TileSkin", &StateSkin::TYPEINFO };

	//____ create() _______________________________________________________________


	TileSkin_p TileSkin::create(Surface * pSurface, HiColor color, int layer )
	{
		if (pSurface == nullptr || !pSurface->isTiling() )
			return nullptr;

		Blueprint bp;
		bp.surface = pSurface;
		bp.color = color;
		bp.layer = layer;

		return TileSkin_p( new TileSkin(bp) );
	}

	TileSkin_p TileSkin::create(const Blueprint& blueprint)
	{
		if (blueprint.surface == nullptr || !blueprint.surface->isTiling())
			return nullptr;


		return TileSkin_p(new TileSkin(blueprint));
	}


	//____ constructor ____________________________________________________________

	TileSkin::TileSkin(const Blueprint& bp) : StateSkin(bp)
	{
		m_blendMode		= bp.blendMode;
		m_gradient		= bp.gradient;

		// Generate lists of states that affects shift, color and surface.

		State	shiftingStates[State::NbStates];
		Coord	stateShifts[State::NbStates];

		State	colorStates[State::NbStates];
		HiColor stateColors[State::NbStates];

		State	surfaceStates[State::NbStates];
		Surface * stateSurfaces[State::NbStates];

		int 	nbShiftingStates = 1;
		int		nbColorStates = 1;
		int		nbSurfaceStates = 1;

		shiftingStates[0] = State::Default;
		colorStates[0] = State::Default;
		surfaceStates[0] = State::Default;

		stateShifts[0] = {0,0};
		stateColors[0] = bp.color;
		stateSurfaces[0] = bp.surface;

		for (auto& stateInfo : bp.states)
		{
			int index = stateInfo.state;

			if (stateInfo.data.contentShift.x != 0 || stateInfo.data.contentShift.y != 0)
			{
				int index = stateInfo.state == State::Default ? 0 : nbShiftingStates++;
				shiftingStates[index] = stateInfo.state;
				stateShifts[index] = stateInfo.data.contentShift;
				m_bContentShifting = true;
			}

			if (stateInfo.data.surface)
			{
				int index = stateInfo.state == State::Default ? 0 : nbSurfaceStates++;
				surfaceStates[index] = stateInfo.state;
				stateSurfaces[index] = stateInfo.data.surface;
			}

			if(stateInfo.data.color != HiColor::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbColorStates++;
				colorStates[index] = stateInfo.state;
				stateColors[index] = stateInfo.data.color;
			}
		}

		// Calc size of index table for surface and color, get their index masks & shifts.

		int	surfaceIndexEntries;
		int	colorIndexEntries;

		std::tie(surfaceIndexEntries,m_stateSurfaceIndexMask,m_stateSurfaceIndexShift) = calcStateToIndexParam(nbSurfaceStates, surfaceStates);
		std::tie(colorIndexEntries,m_stateColorIndexMask,m_stateColorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);


		// Calculate memory needed for all state data

		int shiftBytes 		= _bytesNeededForContentShiftData(nbShiftingStates, shiftingStates);
		int surfaceBytes	= sizeof(Surface*) * nbSurfaceStates;
		int colorBytes		= sizeof(HiColor) * nbColorStates;
		int indexBytes		= surfaceIndexEntries+colorIndexEntries;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(shiftBytes + surfaceBytes + colorBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pCoords = _prepareForContentShiftData(pDest, nbShiftingStates, shiftingStates);
		for( int i = 0 ; i < nbShiftingStates ; i++ )
			pCoords[i] = stateShifts[i];

		pDest += shiftBytes;

		auto pSurfaces = (Surface**) pDest;
		for( int i = 0 ; i < nbSurfaceStates ; i++ )
		{
			pSurfaces[i] = stateSurfaces[i];
			pSurfaces[i]->retain();
		}

		m_pStateSurfaces = pSurfaces;
		m_nbStateSurfaces = nbSurfaceStates;

		pDest += surfaceBytes;

		auto pColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbColorStates ; i++ )
			pColors[i] = stateColors[i];

		m_pStateColors = pColors;

		pDest += colorBytes;

		m_pStateSurfaceIndexTab = pDest;
		m_pStateColorIndexTab = pDest + surfaceIndexEntries;

		generateStateToIndexTab(m_pStateSurfaceIndexTab, nbSurfaceStates, surfaceStates);
		generateStateToIndexTab(m_pStateColorIndexTab, nbColorStates, colorStates);
	}

	//____ destructor ____________________________________________________________

	TileSkin::~TileSkin()
	{
		for( int i = 0 ; i < m_nbStateSurfaces ; i++ )
			m_pStateSurfaces[i]->release();

		free(m_pStateData);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& TileSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ _render() _______________________________________________________________

	void TileSkin::_render( GfxDevice * pDevice, const RectSPX& _canvas, int scale, State state, float value, float value2, int animPos, float * pStateFractions) const
	{
		int idx = state;

		Surface * pSurf = _getSurface(state);

		if( !pSurf )
			return;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		RenderSettingsWithGradient settings(pDevice, m_layer, m_blendMode, _getColor(state), canvas, m_gradient);

		pDevice->setBlitSource(pSurf);
		pDevice->scaleTile(canvas,scale/64.f);
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX TileSkin::_defaultSize(int scale) const
	{
		SizeSPX content = align(ptsToSpx(m_padding,scale));
		SizeSPX surface;

		Surface * pSurface = m_pStateSurfaces[0];
		if (pSurface)
			surface = align(ptsToSpx(pSurface->pointSize(),scale));

		return SizeSPX::max(content, surface) + align(ptsToSpx(m_spacing, scale));
	}

	//____ _markTest() _____________________________________________________________

	bool TileSkin::_markTest( const CoordSPX& _ofs, const RectSPX& _canvas, int scale, State state, float value, float value2, int alphaOverride) const
	{
		//TODO: Take gradient and tintColor into account.

		Surface * pSurf = _getSurface(state);

		int alpha = alphaOverride == -1 ? m_markAlpha : alphaOverride;

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale));
		
		if( !canvas.contains(_ofs) )
			return false;
		
		canvas += align(ptsToSpx(m_overflow, scale));

		return markTestTileRect(_ofs, pSurf, canvas, scale, alpha);
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX TileSkin::_dirtyRect(const RectSPX& _canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (oldState == newState)
			return RectSPX();

		RectSPX canvas = _canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));

		if(_getSurface(oldState) != _getSurface(newState))
			return canvas;
		
		return StateSkin::_dirtyRect(canvas, scale, newState, oldState, newValue, oldValue, newValue2, oldValue2, 
									newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
	}

	//____ _coverage() ___________________________________________________________

	RectSPX TileSkin::_coverage(const RectSPX& geo, int scale, State state) const
	{
		Surface * pSurface = _getSurface(state);
		const HiColor& color = _getColor(state);

		if (color.a == 4096 && pSurface->isOpaque())
			return geo - align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_overflow,scale));
		else
			return RectSPX();
	}

} // namespace wg
