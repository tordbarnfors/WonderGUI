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

#include <wg2_smartptr.h>

#include <wg2_base.h>

WgWeakPtrImpl::WgWeakPtrImpl( WgWeakPtrTarget * pObj )
{
	if( pObj )
	{
		if( !pObj->m_pHub )
		{
			m_pHub = new WgWeakPtrHub();
			m_pHub->refCnt = 1;
			m_pHub->pObj = pObj;
			pObj->m_pHub = m_pHub;
		}
		else
		{
			m_pHub = pObj->m_pHub;
			m_pHub->refCnt++;
		}
	}
	else
	{
		m_pHub = 0;
	}
};


WgWeakPtrImpl::~WgWeakPtrImpl()
{
	if( m_pHub )
	{
		m_pHub->refCnt--;

		if( m_pHub->refCnt == 0 )
		{
			if( m_pHub->pObj )
				m_pHub->pObj->m_pHub = 0;
			delete m_pHub;
		}
	}
}



void WgWeakPtrImpl::copy( WgWeakPtrImpl const & r)
{
	if( m_pHub != r.m_pHub )
	{
		if( m_pHub )
		{
			m_pHub->refCnt--;

			if( m_pHub->refCnt == 0 )
			{
				if( m_pHub->pObj )
					m_pHub->pObj->m_pHub = 0;
				delete m_pHub;
			}
		}

		m_pHub = r.m_pHub;
		if( m_pHub )
			m_pHub->refCnt++;
	}
}
