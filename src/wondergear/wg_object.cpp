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

#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_gearbase.h>

namespace wg
{

	const TypeInfo	Object::TYPEINFO = { "Object", nullptr };

	WeakPtrHub * WeakPtrHub::getHub(Object * pObj)
	{
		if( !pObj)
			return nullptr;

		WeakPtrHub * pHub;
		if( !pObj->m_pWeakPtrHub )
		{
			pHub = GearBase::_allocWeakPtrHub();
			pHub->refCnt = 1;
			pHub->pObj = pObj;
			pHub->pFinalizer = nullptr;
			pObj->m_pWeakPtrHub = pHub;
		}
		else
		{
			pHub = pObj->m_pWeakPtrHub;
			pHub->refCnt++;
		}
		return pHub;
	}

	void WeakPtrHub::releaseHub(WeakPtrHub * pHub)
	{
		if( pHub )
		{
			pHub->refCnt--;

			if( pHub->refCnt == 0 && !pHub->pFinalizer )
			{
				if( pHub->pObj )
					pHub->pObj->m_pWeakPtrHub = nullptr;
				GearBase::_freeWeakPtrHub(pHub);
			}
		}
	}

	void WeakPtrHub::objectWillDestroy(WeakPtrHub * pHub)
	{
		if (pHub->pFinalizer)
		{
			pHub->pFinalizer(pHub->pObj);
			pHub->pObj = nullptr;
			pHub->pFinalizer = nullptr;
			if (pHub->refCnt == 0)
				GearBase::_freeWeakPtrHub(pHub);
		}
		else
			pHub->pObj = nullptr;
	}

	void WeakPtrHub::setFinalizer(Object * pObj, Finalizer_p pFinalizer)
	{
		WeakPtrHub * pHub;
		if (pObj->m_pWeakPtrHub)
		{
			pHub = pObj->m_pWeakPtrHub;
		}
		else
		{
			pHub = GearBase::_allocWeakPtrHub();
			pHub->refCnt = 0;
			pHub->pObj = pObj;
			pObj->m_pWeakPtrHub = pHub;
		}

		pHub->pFinalizer = pFinalizer;
	}

	Finalizer_p WeakPtrHub::getFinalizer(Object * pObj)
	{
		if (pObj->m_pWeakPtrHub)
			return pObj->m_pWeakPtrHub->pFinalizer;

		return nullptr;
	}

	//____ Constructor ________________________________________________________

	Object::Object() : m_refCount(0)
	{
		GearBase::_objectWasCreated(this);
	}

	//____ isInstanceOf() _____________________________________________________
	/**
	 * @brief	Check if the object is an instance or subclass of specified class.
	 *
	 * Check if the object is an instance or subclass of specified class.
	 *
	 * @param typeInfo		Reference to TypeInfo. This needs to be a reference returned by
	 * 						a call to Object::typeInfo() or the TYPEINFO member of a subclass of Object.
	 *
	 * This method compares the specified class type to the TYPEINFO member of all classes implemented by the
	 * object. This is needed when checking if the object implements a class other than the leaf class.
	 * When just checking for a leaf class, a direct comparison between object->typeInfo() and Foo::TYPEINFO
	 * is faster.
	 *
	 * @return True if the object implements the specied class.
	 *
	 */

	bool Object::isInstanceOf( const TypeInfo& _typeInfo ) const
	{
		const TypeInfo* p = &typeInfo();

		while (p != nullptr)
		{
			if (p == &_typeInfo)
				return true;
			p = p->pSuperClass;
		}
		return false;
	}

	//____ typeInfo() _________________________________________________________
	/**
	 * @brief Get class information about the object.
	 *
	 * Get class information about the object.
	 *
	 * The TypeInfo is a static struct with class information for the object.
	 * The struct is the same for all objects of same class, so if two objects share
	 * the same TypeInfo struct they are of the same class.
	 *
	 * @return Reference to the TypeInfo struct for the class of the object.
	 */

	const TypeInfo& Object::typeInfo(void) const
	{
		return TYPEINFO;
	}

	void Object::_destroy()
	{
		GearBase::_objectWillDestroy(this);

		if (m_pWeakPtrHub)
			WeakPtrHub::objectWillDestroy(m_pWeakPtrHub);
		delete this;
	}

	//____ setFinalizer() _____________________________________________________
	/**
	* @brief Set a callback for when object is destroyed.
	*
	* Set a callback that will be called when the object is destroyed.
	*
	* @param pFinalizer		Pointer to a function of format "void FunctionName(Object*)", where the Object pointer points to the object being destroyed.
	*
	* A WonderGUI Object will automatically be destroyed when there no longer is any reference to it (besides any weak pointer).
	* Setting a Finalizer function allows us to be notified right before the objects destructor is called.
	*
	* Each object can have only one finalizer, so setting a finalizer will remove any previous finalizer for that object.
	*
	* Finalizers comes with a cost. A small, extra memory allocation is performed for each object with a finalizer.
	*
	* @return Nothing, method always succeeds.
	*/

	void Object::setFinalizer(Finalizer_p pFinalizer)
	{
		WeakPtrHub::setFinalizer(this, pFinalizer);
	}

	//____ finalizer() ________________________________________________________
	/**
	* @brief Get the Finalizer for this Object.
	*
	* Get the Finalizer (if any) set for this Object.
	*
	* A WonderGUI Object will automatically be destroyed when there no longer is any reference to it (besides any weak pointer).
	* The finalizer is a function that is called right before the objects destructor, thus notifying us of its immediate demise.
	*
	* @return Pointer to the finalizer if one has been set, otherwise null.
	*/

	Finalizer_p Object::finalizer() const
	{
		if (m_pWeakPtrHub)
			return m_pWeakPtrHub->pFinalizer;

		return nullptr;
	}

	//____ weakPointers() ________________________________________________________
/**
* @brief Get number of weak pointers pointing to this Object.
*
* Get number of weak pointers pointing to this Object.
*
* @return Number of weak pointers pointing to this Object.
*/

	int Object::weakPointers() const
	{
		if (m_pWeakPtrHub)
			return m_pWeakPtrHub->refCnt;

		return 0;
	}


} // namespace wg
