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
#ifndef	WG_COMPONENTPTR_DOT_H
#define	WG_COMPONENTPTR_DOT_H
#pragma once

#include <wg_object.h>

namespace wg
{

	template<class Comp> class ComponentPtr
	{

	public:

	//.____ Creation ___________________________________________________________

		ComponentPtr()
		{
		}

		ComponentPtr(Object* pObject, Comp& component)
		{
			pObject->retain();
			m_pObject = pObject;
			m_pComponent = &component;
		}

		ComponentPtr(const ComponentPtr<Comp>& _p)
		{
			m_pObject = _p.m_pObject;
			m_pComponent = _p.m_pComponent;
			if (m_pObject)
				m_pObject->retain();
		};


		template<typename _Tp1> ComponentPtr( const ComponentPtr<_Tp1>& _p)
		{
			m_pObject = _p.m_pObject;
			m_pComponent = _p.m_pComponent;
			if (m_pObject)
				m_pObject->retain();
		}

		~ComponentPtr()
		{
			if (m_pObject)
				m_pObject->release();
		}

		//.____ Misc ___________________________________________________________

		inline Object_p object() const { return m_pObject; }
		inline Object* rawObjectPtr() const { return m_pObject; }
		inline Comp* rawComponentPtr() const { return m_pComponent; }

		void		clear() { if (m_pObject) m_pObject->release(); m_pObject = nullptr; m_pComponent = nullptr; }

		//.____ Operators ______________________________________________________

		ComponentPtr<Comp>& operator=( const ComponentPtr<Comp>& _p)
		{
			if( m_pComponent != _p.m_pComponent )
			{
				if( m_pObject )
					m_pObject->release();

				m_pObject = _p.m_pObject;
				m_pComponent = _p.m_pComponent;
				if (m_pObject)
					m_pObject->retain();
			}
			return *this;
		}

		template<typename _Tp1> ComponentPtr<Comp>& operator=( const ComponentPtr<_Tp1>& _p)
		{
			if (m_pComponent != _p.m_pComponent)
			{
				if (m_pObject)
					m_pObject->release();

				m_pObject = _p.m_pObject;
				m_pComponent = _p.m_pComponent;
				if (m_pObject)
					m_pObject->retain();
			}
			return *this;
		}

		operator  Comp*() const { return m_pComponent; }

		inline Comp & operator*() const { return * this->m_pComponent; }
		inline Comp * operator->() const{ return this->m_pComponent; }

		template<typename _Tp1> bool operator==( const ComponentPtr<_Tp1>& _p) const { return this->m_pComponent == _p.m_pComponent; }
		template<typename _Tp1> bool operator!=( const ComponentPtr<_Tp1>& _p) const { return this->m_pComponent != _p.m_pComponent; }
		template<typename _Tp1> bool operator<(const ComponentPtr<_Tp1>& other) const { return m_pComponent < other.m_pComponent; }
		template<typename _Tp1> bool operator>(const ComponentPtr<_Tp1>& other) const { return m_pComponent > other.m_pComponent; }
		template<typename _Tp1> bool operator<=(const ComponentPtr<_Tp1>& other) const { return m_pComponent <= other.m_pComponent; }
		template<typename _Tp1> bool operator>=(const ComponentPtr<_Tp1>& other) const { return m_pComponent >= other.m_pComponent; }

		inline operator bool() const { return m_pObject != 0; }

protected:
		Object* m_pObject = nullptr;
		Comp *	m_pComponent = nullptr;
	};




} // namespace wg
#endif //WG_COMPONENTPTR_DOT_H

