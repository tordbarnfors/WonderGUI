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
#ifndef WG_SKINSLOTMANAGER_DOT_H
#define WG_SKINSLOTMANAGER_DOT_H
#pragma once

#include <wg_mempool.h>
#include <wg_util.h>
#include <wg_pointers.h>

#include <vector>

namespace wg
{



	struct SkinSlotPocket
	{
		class Holder
		{
		public:
			virtual bool _update(SkinSlotPocket* pPocket, int msPassed) = 0;
			virtual void _willRemovePocket(SkinSlotPocket* pPocket) = 0;
		};

		Holder*				pHolder;
		bool				bAnimated;		// Set if skin is animated beyond transitions.
		int					animationCounter;	// Count animation progress in millisec.

		Bitmask<uint8_t>	transitionFrom;	// Bitmask representation of state we transition from.
		Bitmask<uint8_t>	transitionTo;	// Bitmask representation of state we transition to.
		float				fractionalState[PrimState_Nb];
		void* instance = 0;
	};

	class SkinSlotManager;
	typedef	StrongPtr<SkinSlotManager>		SkinSlotManager_p;
	typedef	WeakPtr<SkinSlotManager>		SkinSlotManager_wp;

	class SkinSlotManager : public Object
	{

	public:
		static SkinSlotManager_p	create() { return SkinSlotManager_p(new SkinSlotManager());}
				
		void update(int msPassed);

		SkinSlotPocket* allocPocket();
		void freePocket(SkinSlotPocket* pEntry);


	private:
		SkinSlotManager();
		~SkinSlotManager();

		MemPool* 						m_pMemPool;
		std::vector<SkinSlotPocket*>	m_slotPockets;
	};
};


#endif 
