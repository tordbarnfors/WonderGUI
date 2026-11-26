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
#ifndef	WAX_PARTPTR_DOT_H
#define	WAX_PARTPTR_DOT_H
#pragma once

#include <wax_object.h>

namespace wax
{

	template<class Comp> class PartPtr
	{

	public:

	//.____ Creation ___________________________________________________________

		PartPtr()
		{
		}

		PartPtr(Object* pObject, Comp& part)
		{
			pObject->retain();
			m_pObject = pObject;
			m_pPart = &part;
		}

		PartPtr(const PartPtr<Comp>& _p)
		{
			m_pObject = _p.m_pObject;
			m_pPart = _p.m_pPart;
			if (m_pObject)
				m_pObject->retain();
		};


		template<typename _Tp1> PartPtr( const PartPtr<_Tp1>& _p)
		{
			m_pObject = _p.m_pObject;
			m_pPart = _p.m_pPart;
			if (m_pObject)
				m_pObject->retain();
		}

		~PartPtr()
		{
			if (m_pObject)
				m_pObject->release();
		}

		//.____ Misc ___________________________________________________________

		inline Object_p object() const { return m_pObject; }
		inline Object* rawObjectPtr() const { return m_pObject; }
		inline Comp* rawPartPtr() const { return m_pPart; }

		void		clear() { if (m_pObject) m_pObject->release(); m_pObject = nullptr; m_pPart = nullptr; }

		//.____ Operators ______________________________________________________

		PartPtr<Comp>& operator=( const PartPtr<Comp>& _p)
		{
			if( m_pPart != _p.m_pPart )
			{
				if( m_pObject )
					m_pObject->release();

				m_pObject = _p.m_pObject;
				m_pPart = _p.m_pPart;
				if (m_pObject)
					m_pObject->retain();
			}
			return *this;
		}

		template<typename _Tp1> PartPtr<Comp>& operator=( const PartPtr<_Tp1>& _p)
		{
			if (m_pPart != _p.m_pPart)
			{
				if (m_pObject)
					m_pObject->release();

				m_pObject = _p.m_pObject;
				m_pPart = _p.m_pPart;
				if (m_pObject)
					m_pObject->retain();
			}
			return *this;
		}

		operator  Comp*() const { return m_pPart; }

		inline Comp & operator*() const { return * this->m_pPart; }
		inline Comp * operator->() const{ return this->m_pPart; }

		template<typename _Tp1> bool operator==( const PartPtr<_Tp1>& _p) const { return this->m_pPart == _p.m_pPart; }
		template<typename _Tp1> bool operator!=( const PartPtr<_Tp1>& _p) const { return this->m_pPart != _p.m_pPart; }
		template<typename _Tp1> bool operator<(const PartPtr<_Tp1>& other) const { return m_pPart < other.m_pPart; }
		template<typename _Tp1> bool operator>(const PartPtr<_Tp1>& other) const { return m_pPart > other.m_pPart; }
		template<typename _Tp1> bool operator<=(const PartPtr<_Tp1>& other) const { return m_pPart <= other.m_pPart; }
		template<typename _Tp1> bool operator>=(const PartPtr<_Tp1>& other) const { return m_pPart >= other.m_pPart; }

		inline operator bool() const { return m_pObject != 0; }

protected:
		Object* m_pObject = nullptr;
		Comp *	m_pPart = nullptr;
	};




} // namespace wax
#endif //WAX_PARTPTR_DOT_H

