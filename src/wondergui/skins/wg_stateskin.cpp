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

#include <wg_stateskin.h>

namespace wg
{

	using namespace Util;

	const TypeInfo StateSkin::TYPEINFO = { "StateSkin", &Skin::TYPEINFO };


	//____ typeInfo() _________________________________________________________

	const TypeInfo& StateSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ contentShift() ________________________________________________

	Coord StateSkin::contentShift(State state) const
	{
		return _getContentShift(state);
	}

	//____ _contentBorder() _______________________________________________________

	BorderSPX StateSkin::_contentBorder(int scale, State state) const
	{
		BorderSPX b = align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_padding, scale));
		CoordSPX ofs = align(ptsToSpx(_getContentShift(state), scale));

		b.left += ofs.x;
		b.top += ofs.y;

		return b;
	}


	//____ _contentRect() __________________________________________________________

	RectSPX StateSkin::_contentRect( const RectSPX& canvas, int scale, State state ) const
	{
		return canvas - align(ptsToSpx(m_spacing, scale)) - align(ptsToSpx(m_padding,scale)) + align(ptsToSpx(_getContentShift(state),scale));
	}

	//____ _contentofs() __________________________________________________________

	CoordSPX StateSkin::_contentOfs( int scale, State state ) const
	{
		return align(ptsToSpx(Coord(m_spacing.left, m_spacing.top), scale)) +
			   align(ptsToSpx(Coord(m_padding.left, m_padding.top), scale)) + 
			   align(ptsToSpx(_getContentShift(state), scale));
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX StateSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if ( _getContentShift(newState) == _getContentShift(oldState) )
			return RectSPX();
		else
			return canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
	}

	//____ _bytesNeededForContentShiftData() __________________________________

	int StateSkin::_bytesNeededForContentShiftData(int nbStates, const State* pStates)
	{
		int indexTableSize;
		int indexMask;
		int indexShift;

		std::tie(indexTableSize,indexMask,indexShift) = calcStateToIndexParam(nbStates,pStates);

		return sizeof(Coord) * nbStates + alignUp8(indexTableSize);
	}

	//____ _prepareForContentShiftData() ___________________________________

	Coord * StateSkin::_prepareForContentShiftData(void * pDest, int nbStates, const State * pStates)
	{
		int indexTableSize;
		int indexMask;
		int indexShift;

		std::tie(indexTableSize,indexMask,indexShift) = calcStateToIndexParam(nbStates,pStates);

		uint8_t * pIndex = (uint8_t*) pDest;
		Coord * pShiftData = (Coord*) (pIndex + alignUp8(indexTableSize));

		generateStateToIndexTab(pIndex, nbStates, pStates);

		m_contentShiftIndexMask = (uint8_t) indexMask;
		m_contentShiftIndexShift = (uint8_t) indexShift;

		m_pContentShiftIndexTab = pIndex;
		m_pContentShiftTable = pShiftData;

		return pShiftData;
	}


} // namespace wg
