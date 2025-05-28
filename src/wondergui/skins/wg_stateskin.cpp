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
#include <wg_util.h>

namespace wg
{

	using namespace Util;

	const TypeInfo StateSkin::TYPEINFO = { "StateSkin", &Skin::TYPEINFO };


	//____ typeInfo() _________________________________________________________

	const TypeInfo& StateSkin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ clearContentShift() ________________________________________________

	void StateSkin::clearContentShift()
	{
		m_contentShiftStateMask = 1;			// Mode normal is set by default

		for( int i = 0 ; i < State::NbStates ; i++ )
			m_contentShift[i] = { 0,0 };

		m_bContentShifting = false;
	}

	//____ setContentShift() ______________________________________________________

	void StateSkin::setContentShift(State state, Coord shift)
	{
		int index = state;
		m_contentShift[index] = shift;
		m_contentShiftStateMask.setBit(index);

		m_bContentShifting = true;				// Making it easy for us, just assume something will be shifting when this method is called.

		_updateContentShift();
	}

	//____ setContentShift() _____________________________________________________

	void StateSkin::setContentShift(std::initializer_list< std::pair<State, Coord> > stateShifts)
	{
		for (auto& shift : stateShifts)
		{
			int index = shift.first;
			m_contentShift[index] = shift.second;
			m_contentShiftStateMask.setBit(index);
		}

		m_bContentShifting = true;				// Making it easy for us, just assume something will be shifting when this method is called.

		_updateContentShift();
	}

	//____ contentShift() ________________________________________________

	Coord StateSkin::contentShift(State state) const
	{
		int index = state;
		if (m_contentShiftStateMask.bit(index))
			return m_contentShift[index];

		return Coord();
	}

	//____ _padding() _______________________________________________________
/*
	BorderSPX StateSkin::_padding(int scale, State state) const
	{
		BorderSPX b = align(ptsToSpx(m_padding,scale));
		CoordSPX ofs = align(ptsToSpx(m_contentShift[state], scale));

		b.left += ofs.x;
		b.top += ofs.y;

		return b;
	}
*/

	//____ _contentBorder() _______________________________________________________

	BorderSPX StateSkin::_contentBorder(int scale, State state) const
	{
		BorderSPX b = align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_padding, scale));
		CoordSPX ofs = align(ptsToSpx(m_contentShift[state], scale));

		b.left += ofs.x;
		b.top += ofs.y;

		return b;
	}


	//____ _contentRect() __________________________________________________________

	RectSPX StateSkin::_contentRect( const RectSPX& canvas, int scale, State state ) const
	{
		return canvas - align(ptsToSpx(m_spacing, scale)) - align(ptsToSpx(m_padding,scale)) + align(ptsToSpx(m_contentShift[state],scale));
	}

	//____ _contentofs() __________________________________________________________

	CoordSPX StateSkin::_contentOfs( int scale, State state ) const
	{
		return align(ptsToSpx(Coord(m_spacing.left, m_spacing.top), scale)) +
			   align(ptsToSpx(Coord(m_padding.left, m_padding.top), scale)) + 
			   align(ptsToSpx(m_contentShift[state], scale));
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX StateSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (m_contentShift[newState] == m_contentShift[oldState])
			return RectSPX();
		else
			return canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
	}

	//____ _updateContentShift() _________________________________________________

	void StateSkin::_updateContentShift()
	{
		for (int i = 0; i < State::NbStates; i++)
		{
			if ( !m_contentShiftStateMask.bit(i) )
			{
				int bestAlternative = bestStateIndexMatch(i, m_contentShiftStateMask);
				m_contentShift[i] = m_contentShift[bestAlternative];
			}
		}
	}

} // namespace wg
