/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <wg_animframes.h>

#include <algorithm>

namespace wg
{


	//____ setSurface() __________________________________________________________

	void AnimFrames::setSurface(Surface* pSurface)
	{
		if (pSurface != m_pSurface)
		{
			m_pSurface = pSurface;
			static_cast<AnimFrames::Holder*>(m_pHolder)->_didSetAnimSurface(this);
		}
	}

	//____ setFrameSize() __________________________________________________________

	void AnimFrames::setFrameSize(Size pointSize)
	{
		if (pointSize != m_frameSize)
		{
			m_frameSize = pointSize;
			static_cast<AnimFrames::Holder*>(m_pHolder)->_didSetAnimFrameSize(this);
		}
	}

	//____ find() _____________________________________________________________
	/**
		@brief Find frame to display for specific timestamp

		This method returns a pointer to the frame to display for the given timestamp.

		Negative values will return the first frame and values beyond the last frame
		will return the last frame.

		@timestamp Timestamp in milliseconds.

		@return Pointer to the frame for the timestamp or nullptr if component contains
				no frames.
	*/
	const AnimFrame* AnimFrames::find(int timestamp) const
	{
		if (m_entries.empty())
			return nullptr;

		const AnimFrame* pFirst = &m_entries.front();
		const AnimFrame* pLast = &m_entries.back();
		const AnimFrame* pFrame = pFirst + (pLast - pFirst);

		if (timestamp < pFirst->m_duration)
			return pFirst;

		if (timestamp >= pLast->m_timestamp)
			return pLast;

		while (true)
		{
			if (timestamp < pFrame->m_timestamp)
			{
				pLast = pFrame;
				pFrame -= (pFrame - pFirst +1) / 2;
			}
			else if (timestamp < pFrame->m_timestamp + pFrame->m_duration)
				return pFrame;
			else
			{
				pFirst = pFrame;
				pFrame += (pLast - pFrame +1) / 2;
			}
		}
	}

	//____ _didAddEntries() ___________________________________________________

	void AnimFrames::_didAddEntries(AnimFrame* pEntry, int nb)
	{
		int ofs = int(pEntry - &m_entries.front());
		int timestamp = ofs == 0 ? 0 : pEntry[-1].timestamp() + pEntry[-1].duration();

		auto it = m_entries.begin() + ofs;

		for (int i = 0; i < nb; i++, it++)
		{
			it->m_timestamp = timestamp;
			timestamp += it->m_duration;
			m_duration += it->m_duration;
		}

		while (it != m_entries.end())
		{
			it->m_timestamp = timestamp;
			timestamp += it->m_duration;
		}

		DynamicVector::_didAddEntries(pEntry, nb);
	}

	//____ _didMoveEntries() __________________________________________________

	void AnimFrames::_didMoveEntries(AnimFrame* pFrom, AnimFrame* pTo, int nb)
	{
		AnimFrame* pEntry = std::min(pFrom, pTo);

		int ofs = int(pEntry - &m_entries.front());
		int timestamp = ofs == 0 ? 0 : pEntry[-1].timestamp() + pEntry[-1].duration();
		for (auto it = m_entries.begin() + ofs; it != m_entries.end(); it++)
		{
			it->m_timestamp = timestamp;
			timestamp += it->m_duration;
		}

		DynamicVector::_didMoveEntries(pFrom, pTo, nb);
	}

	//____ _willEraseEntries() ___________________________________________________

	void AnimFrames::_willEraseEntries(AnimFrame* pEntry, int nb)
	{
		int ofs = int(pEntry - &m_entries.front());
		int timestamp = pEntry->timestamp() + pEntry->duration();

		auto it = m_entries.begin() + ofs;

		for (int i = 0; i < nb; i++, it++)
			m_duration -= it->m_duration;

		while (it != m_entries.end())
		{
			it->m_timestamp = timestamp;
			timestamp += it->m_duration;
		}

		DynamicVector::_willEraseEntries(pEntry, nb);
	}


} //namespace wg
