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
#include <wg_blockingcapsule.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

#include <algorithm>

namespace wg
{
	using namespace Util;

	const TypeInfo BlockingCapsule::TYPEINFO = { "BlockingCapsule", &Capsule::TYPEINFO };


	//____ create() _______________________________________________________________

	BlockingCapsule_p BlockingCapsule::create() 
	{ 
		return BlockingCapsule_p(new BlockingCapsule()); 
	}

	BlockingCapsule_p BlockingCapsule::create(const Blueprint& blueprint)
	{ 
		return BlockingCapsule_p(new BlockingCapsule(blueprint));
	}


	//____ constructor ____________________________________________________________

	BlockingCapsule::BlockingCapsule()
	{
	}

	//____ Destructor _____________________________________________________________

	BlockingCapsule::~BlockingCapsule()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BlockingCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setActive() ___________________________________________________________

	void BlockingCapsule::setActive(bool bActive)
	{
		m_bActive = bActive;
	}

	//____ setInverted() _________________________________________________________

	void BlockingCapsule::setInverted(bool bInverted)
	{
		m_bInverted = bInverted;
	}

	//____ setAutoDeactivate() ___________________________________________________

	void BlockingCapsule::setAutoDeactivate(bool bAutoDeactivate)
	{
		m_bAutoDeactivate = bAutoDeactivate;
	}

	//____ _findWidget() _________________________________________________________

	Widget*  BlockingCapsule::_findWidget(const CoordSPX& ofs, SearchMode mode)
	{
		Widget * pFound = Capsule::_findWidget(ofs,mode);
		
		if( !m_bActive || pFound == this || mode != SearchMode::ActionTarget)
			return pFound;


		bool bInList = false;			// Set if area or found widget or its ancestor is in list.

		// Check against blocked rectangles.
		
		Coord ptsOfs = Coord(ofs)/m_scale;
		
		for( auto& rect : blockedAreas )
		{
			if( rect.contains(ptsOfs) )
			{
				bInList = true;
				break;
			}
		}
				
		// Check against blocked widgets.
		
		if( !bInList )
		{
			for( auto& weakptr : blockedWidgets )
			{
				Widget * pTarget = weakptr.rawPtr();
				if( pTarget == nullptr )
					continue;
				
				Widget * pWidget = pFound;
				while( pWidget )
				{
					if( pWidget == pTarget )
					{
						bInList = true;
						break;
					}
					
					pWidget = pWidget->parent();
				}
			}
		}
			
		bool bBlock = m_bInverted ? !bInList : bInList;
		
		if( bBlock )
			return this;
		else
			return pFound;
	}

	//____ _receive() ____________________________________________________________

	void BlockingCapsule::_receive(Msg* pMsg)
	{
		if( pMsg->type() == MsgType::MouseClick && m_bActive && m_bAutoDeactivate && !pMsg->reposted() )
		{
			m_bActive = false;
			Base::msgRouter()->post( DeactivateMsg::create(this) );
		}
		
		Capsule::_receive(pMsg);
	}

} // namespace wg
