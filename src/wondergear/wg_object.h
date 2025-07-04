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
#ifndef WG_OBJECT_DOT_H
#define WG_OBJECT_DOT_H
#pragma once

#include <atomic>

#include <wg_strongptr.h>
#include <wg_weakptr.h>
#include <wg_geartypes.h>

namespace wg
{

	class Object;
	struct TypeInfo;

	typedef StrongPtr<Object>	Object_p;
//	typedef WeakPtr<Object>		Object_wp;


	/**
	 * @brief Base class for all reference counted objects in WonderGUI.
	 *
	 * Base class for all reference counted objects in WonderGUI.
	 *
	 * Object provides the datastructures needed for smart pointers, weak pointers and
	 * destruction notifiers as well as methods for identifying object types and
	 * dynamic cast of smart pointers.
	 *
	 * Objects that are based on Object are implicitly destroyed when their last
	 * reference disappears and should never be explicitly destroyed.
	 *
	 */

	class Object
	{
		template<class T> friend class StrongPtr;
		template<class T> friend class WeakPtr;

		template<class T> friend class StrongComponentPtr;
		template<class T> friend class WeakComponentPtr;

		friend class WeakPtrHub;
	public:

		//.____ Identification _________________________________________________

		virtual const TypeInfo& typeInfo(void) const;
		const static TypeInfo	TYPEINFO;

		bool		isInstanceOf(const TypeInfo& typeInfo) const;

		//.____ Misc __________________________________________________________

		void			setFinalizer(Finalizer_p pFinalizer);
		Finalizer_p		finalizer() const;

		inline void		retain();
		inline void		release();
		
		inline int		refcount() const;
		int				weakPointers() const;

	protected:
		Object();
		virtual ~Object() {};

		inline void _incRefCount() { m_refCount++; }
		inline void _decRefCount() { m_refCount--; if( m_refCount == 0 ) _destroy(); }

		inline void _incRefCount(int amount) { m_refCount.fetch_add(amount); }
		inline void _decRefCount(int amount) { m_refCount.fetch_sub(amount); if( m_refCount == 0 ) _destroy(); }

		WeakPtrHub *	m_pWeakPtrHub = nullptr;

	private:
		virtual void 	_destroy();			// Pointers should call destroy instead of destructor.
		std::atomic_int	m_refCount;
	};

	/**
	 * @brief Increase objects reference count
	 * 
	 * Increases the reference count of the Object. 
	 * 
	 * An Objects reference count is automatically
	 * increased/decreased with every shared pointer pointing to it and is destroyed when it
	 * reaches zero.
	 * 
	 * This method allows you to increase the reference count by hand, keeping the object alive even
	 * when no smart pointers points to it anymore.
	 * 
	 * To decrease the reference count call release().
	 * 
	 */

	void Object::retain()
	{ 
		_incRefCount(); 
	}

	/**
	 * @brief Decrease objects reference count
	 * 
	 * Decreases the reference count of the Object, potentially destroying it. 
	 * 
	 * An Objects reference count is automatically
	 * increased/decreased with every shared pointer pointing to it and is destroyed when it
	 * reaches zero.
	 * 
	 * This method allows you to decrease the reference count by hand and is the companion to
	 * retain() which increases the reference count.
	 * 
	 * Never call this method without previously having called retain().
	 * 
	 */

	void Object::release()
	{ 
		_decRefCount();
	}

	/**
	 * @brief Get the reference count of the Object
	 * 
	 * The reference count is the count of how many hard references (like shared pointers) the Object has.
	 * 
	 * The Object is automatically destroyed when the reference count reaches zero.
	 *  
	 * @return The reference count of the Object. Always 1 or higher.
	 */

	int Object::refcount() const 
	{ 
		return m_refCount; 
	}

} // namespace wg
#endif //WG_OBJECT_DOT_H
