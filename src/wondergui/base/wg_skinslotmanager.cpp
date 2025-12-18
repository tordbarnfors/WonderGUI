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
#include <wg_skinslotmanager.h>
#include <assert.h>
#include <algorithm>

namespace wg
{

	//____ constructor _____________________________________________________________

	SkinSlotManager::SkinSlotManager()
	{
		m_pMemPool = new MemPool(16, sizeof(SkinSlotPocket));
	}

	//____ exit() _____________________________________________________________

	SkinSlotManager::~SkinSlotManager()
	{
		assert(m_pMemPool->entriesAllocated() == 0);
		delete m_pMemPool;
	}

	//____ update() ___________________________________________________________

	void SkinSlotManager::update(int msPassed)
	{
		auto deleteIt = std::remove_if(m_slotPockets.begin(),
			m_slotPockets.end(),
			[msPassed,this](SkinSlotPocket* pPocket)
			{
				bool bDelete = pPocket->pHolder->_update(pPocket, msPassed); 
				if (bDelete)
				{
					pPocket->pHolder->_willRemovePocket(pPocket);
					m_pMemPool->freeEntry(pPocket);
				}
				return bDelete; 
			}
		);

		if (deleteIt != m_slotPockets.end())
			m_slotPockets.erase(deleteIt, m_slotPockets.end());
	}

	//____ allocPocket() ______________________________________________________

	SkinSlotPocket* SkinSlotManager::allocPocket()
	{
		auto pBuff = m_pMemPool->allocEntry();

		auto pPocket = new(pBuff) SkinSlotPocket();

		m_slotPockets.push_back(pPocket);
		return pPocket;
	}

	//____ freePocket() _______________________________________________________

	void SkinSlotManager::freePocket(SkinSlotPocket* pPocket)
	{
		m_pMemPool->freeEntry(pPocket);

		auto it = std::find(m_slotPockets.begin(), m_slotPockets.end(), pPocket);
		m_slotPockets.erase(it);
	}
}
