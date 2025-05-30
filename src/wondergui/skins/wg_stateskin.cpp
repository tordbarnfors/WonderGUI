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
		return m_stateData[m_stateToIndexTab[state]].contentShift;
	}

	//____ _contentBorder() _______________________________________________________

	BorderSPX StateSkin::_contentBorder(int scale, State state) const
	{
		BorderSPX b = align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_padding, scale));
		CoordSPX ofs = align(ptsToSpx(m_stateData[m_stateToIndexTab[state]].contentShift, scale));

		b.left += ofs.x;
		b.top += ofs.y;

		return b;
	}


	//____ _contentRect() __________________________________________________________

	RectSPX StateSkin::_contentRect( const RectSPX& canvas, int scale, State state ) const
	{
		return canvas - align(ptsToSpx(m_spacing, scale)) - align(ptsToSpx(m_padding,scale)) + align(ptsToSpx(m_stateData[m_stateToIndexTab[state]].contentShift,scale));
	}

	//____ _contentofs() __________________________________________________________

	CoordSPX StateSkin::_contentOfs( int scale, State state ) const
	{
		return align(ptsToSpx(Coord(m_spacing.left, m_spacing.top), scale)) +
			   align(ptsToSpx(Coord(m_padding.left, m_padding.top), scale)) + 
			   align(ptsToSpx(m_stateData[m_stateToIndexTab[state]].contentShift, scale));
	}

	//____ _dirtyRect() ______________________________________________________

	RectSPX StateSkin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos,
		float* pNewStateFractions, float* pOldStateFractions) const
	{
		if (m_stateData[m_stateToIndexTab[newState]].contentShift == m_stateData[m_stateToIndexTab[oldState]].contentShift)
			return RectSPX();
		else
			return canvas - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
	}

	//____ _bytesNeededForContentShiftData() __________________________________

	int StateSkin::_bytesNeededForContentShiftData(int nbStates, State* pStates)
	{

	}

	//____ _generateContentShiftData() ________________________________________

	void StateSkin::_generateContentShiftData(void* pDest, int nbStates, State* pStates)
	{

		uint8_t	usedBits = 0;
		for (int i = 0; i < nbStates; i++)
			usedBits |= pStates->index();

		int indexTableSize = 72;

		int indexMask = 0x7F;
		int indexShift = 0;

		if (usedBits == 0)
		{
			indexMask = 0;
			indexTableSize = 1;			// We always lookup default.
		}
		else
		{
			if ((usedBits & int(StateEnum::Disabled)) == 0)
			{
				indexMask = 0x3F;
				indexTableSize = 64;
				if ((usedBits & (int(StateEnum::Hovered) | int(StateEnum::Pressed))) == 0)
				{ 
					indexMask = 0xF;
					indexTableSize = 16; 
					if ((usedBits & int(StateEnum::Focused) == 0))
					{
						indexMask = 0x7;
						indexTableSize = 8;
						if ((usedBits & int(StateEnum::Checked) == 0))
						{
							indexMask = 0x3;
							indexTableSize = 4;
							if ((usedBits & int(StateEnum::Selected) == 0))
							{
								indexMask = 0x1;
								indexTableSize = 2;
							}
						}
					}
				}
			}

			if ((usedBits & int(StateEnum::Flagged)) == 0)
			{
				indexShift++;
				indexTableSize /= 2;

				if ((usedBits & int(StateEnum::Selected)) == 0)
				{
					indexShift++;
					indexTableSize /= 2;

					if ((usedBits & int(StateEnum::Checked)) == 0)
					{
						indexShift++;
						indexTableSize /= 2;

						if ((usedBits & int(StateEnum::Focused)) == 0)
						{
							indexShift++;
							indexTableSize /= 2;

							if ((usedBits & (int(StateEnum::Hovered) | int(StateEnum::Pressed))) == 0)
							{
								indexShift += 2;
								indexTableSize /= 4;
							}
						}
					}
				}
			}



		}




	}




	//____ _generateStateToIndexTab() _________________________________________

	void StateSkin::_generateStateToIndexTab(int nbStates, State* pStates)
	{
		for (int i = 0; i < State::NbStates; i++)
		{
			State wanted = (StateEnum) i;
			int index = wanted.bestMatch(nbStates, pStates);
			assert(index >= 0 && index < nbStates);
			m_stateToIndexTab[i] = index;
		}
	}


} // namespace wg
