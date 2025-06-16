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
#ifndef WG_STATESKIN_DOT_H
#define WG_STATESKIN_DOT_H
#pragma once

#include <wg_skin.h>

#include <initializer_list>
#include <utility>
#include <vector>

namespace wg
{

	class StateSkin : public Skin
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior _______________________________________________________

		Coord			contentShift(State state) const;

		//.____ Internal ________________________________________________________

//		BorderSPX		_padding(int scale, State state) const override;

		BorderSPX		_contentBorder(int scale, State state) const override;

		CoordSPX		_contentOfs(int scale, State state) const override;
		RectSPX			_contentRect(const RectSPX& canvas, int scale, State state) const override;

		RectSPX			_dirtyRect(	const RectSPX& canvas, int scale, State newState, State oldState, float newValue = 1.f, float oldValue = 1.f,
									float newValue2 = -1.f, float oldValue2 = -1.f, int newAnimPos = 0, int oldAnimPos = 0,
									float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

	protected:
		StateSkin() { m_bIgnoresState = false; }
		template<class BP> 
		StateSkin(const BP& bp) : Skin(bp)
		{
			m_bIgnoresState = false;
		}

		~StateSkin() {}


		int				_bytesNeededForContentShiftData(int nbStates, const State* pStates);
		Coord * 		_prepareForContentShiftData(void * pDest, int nbStates, const State * pStates);

		Coord 			_getContentShift(State state) const
		{
						int idxTabEntry = (state.index() & m_contentShiftIndexMask) >> m_contentShiftIndexShift;
						int entry = m_pContentShiftIndexTab[idxTabEntry];
						return m_pContentShiftTable[entry];
		}

		// Mask and shift values to apply to stateIndex in order to make m_pContentShiftIndexTab
		// shorter when bits least or most significant bits of index can be ignored.

		uint8_t			m_contentShiftIndexMask;		
		uint8_t			m_contentShiftIndexShift;

		// 

		uint8_t*		m_pContentShiftIndexTab;		// Table with index values into m_pContentShiftTable for each mode (72) or less.
		Coord*			m_pContentShiftTable;			// Contains content shift values used.
	};


} // namespace wg
#endif //WG_STATESKIN_DOT_H
