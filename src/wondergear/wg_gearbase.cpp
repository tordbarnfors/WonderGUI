
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


#include <wg_gearbase.h>
#include <wg_memstack.h>
#include <wg_mempool.h>
#include <wg_object.h>

#include <iostream>
#include <algorithm>
#include <cassert>

namespace wg
{
	const TypeInfo	GearBase::TYPEINFO = { "GearBase", nullptr };

	int							GearBase::s_initCounter = 0;

	GearContext_p				GearBase::s_pGearContext;

	MemPool *					GearBase::s_pPtrPool = nullptr;
	MemStack *					GearBase::s_pMemStack = nullptr;

	std::atomic<unsigned int>	GearBase::s_objectsCreated;
	std::atomic<unsigned int>	GearBase::s_objectsDestroyed;


	bool						GearBase::s_bTrackingObjects = false;

	std::unordered_map<Object*, GearBase::ObjectInfo>	GearBase::s_trackedObjects;


	//____ init() __________________________________________________________________

	bool GearBase::init()
	{

		if( s_initCounter == 0 )
		{
			s_objectsCreated = 0;
			s_objectsDestroyed = 0;
			s_pPtrPool = new MemPool( 128, sizeof( WeakPtrHub ) );
			s_pMemStack = new MemStack( 4096 );

			s_pGearContext = new GearContext();
		}

		s_initCounter++;
		return true;
	}

	//____ exit() __________________________________________________________________

	bool GearBase::exit()
	{
		if( s_initCounter <= 0 )
		{
			throwError(ErrorLevel::SilentError, ErrorCode::IllegalCall, "Call to GearBase::exit() ignored, not initialized or already exited.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if( s_initCounter > 1 )
		{
			s_initCounter--;
			return true;
		}


		if( !s_pPtrPool->isEmpty() )
		{
			throwError(ErrorLevel::SilentError, ErrorCode::SystemIntegrity, "Some weak pointers still in use. Can not exit WonderGUI.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		s_initCounter = 0;

		if( !s_pMemStack->isEmpty() )
		{
			throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "Memstack still contains data. Not all allocations have been correctly released.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		delete s_pPtrPool;
		s_pPtrPool = nullptr;

		delete s_pMemStack;
		s_pMemStack = nullptr;

		// Our context should be the only object alive at this point. Still needed for the error-handler.

		if (s_objectsCreated != s_objectsDestroyed )
			throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "Some objects still alive after wondergui exit. Might cause problems when they go out of scope. Forgotten to clear pointers?\nHint: Enable object tracking to find out which ones.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);

		s_pGearContext = nullptr;
		return true;
	}

	//____ throwError() _________________________________________________________

	void GearBase::throwError(ErrorLevel severity, ErrorCode code, const char * msg, const Object * pObject, const TypeInfo* pClassType, const char * function, const char * file, int line)
	{
		if (s_pGearContext != nullptr && s_pGearContext->pErrorHandler)
		{
			Error	error;

			error.code = code;
			error.message = msg;
			error.pObject = pObject;
			error.classname = pClassType ? pClassType->className : nullptr;
			error.function = function;
			error.file = file;
			error.line = line;
			error.severity = severity;

			s_pGearContext->pErrorHandler(error);
		}
	}

	//____ beginObjectTracking() _________________________________________________

	void GearBase::beginObjectTracking()
	{
		s_bTrackingObjects = true;
	}

	//____ endObjectTracking() __________________________________________________

	void GearBase::endObjectTracking()
	{
		s_bTrackingObjects = false;
		s_trackedObjects.clear();
	}

	//____ printObjects() ________________________________________________

	void GearBase::printObjects(std::ostream& stream)
	{

		stream << "Objects created: " << s_objectsCreated << std::endl;
		stream << "Objects destroyed: " << s_objectsDestroyed << std::endl;
		stream << "Objects alive: " << (s_objectsCreated - s_objectsDestroyed) << std::endl;

        if( !s_trackedObjects.empty() )
        {
            stream << std::endl;
            stream << "Tracked objects: " << s_trackedObjects.size() << std::endl;
            stream << "---------------------------------------" << std::endl;

			for (auto& tracked : s_trackedObjects)
			{
				const char* pClassName = tracked.first->typeInfo().className;

				stream << "#" << tracked.second.serialNb << " - " << pClassName << " @ 0x" << tracked.first << " with " << tracked.first->refcount() << " references.";

				if (tracked.second.pFileName)
					stream << " Tracked from " << tracked.second.pFileName << ":" << tracked.second.lineNb;

				stream << std::endl;
			}
		}
	}


	//____ setContext() _______________________________________________________

	GearContext_p GearBase::setContext( const GearContext_p& pNewContext )
	{
		GearContext_p p = s_pGearContext;

		if( pNewContext )
			s_pGearContext = pNewContext;
		else
			s_pGearContext = new GearContext();
		return p;
	}

	//____ setErrorHandler() _________________________________________________________

	void GearBase::setErrorHandler(std::function<void(Error&)> handler)
	{
		s_pGearContext->pErrorHandler = handler;
	}

	//____ errorHandler() ____________________________________________________________

	std::function<void(Error&)>	GearBase::errorHandler()
	{
		return s_pGearContext->pErrorHandler;
	}

	//____ memStackAlloc() ________________________________________________________

	char * GearBase::memStackAlloc( int bytes )
	{
		assert(s_pMemStack != nullptr);
		return s_pMemStack->allocBytes(bytes);
	}

	//____ memStackFree() ______________________________________________________

	void GearBase::memStackFree( int bytes )
	{	assert(s_pMemStack != nullptr);
		return s_pMemStack->releaseBytes(bytes);
	}

	//____ _trackObject() ________________________________________________________

	void GearBase::_trackObject(Object* pObject, const char* pFileName, int lineNb)
	{
        if( s_bTrackingObjects )
            s_trackedObjects[pObject] = { pFileName, lineNb, s_objectsCreated };
	}

	//____ _allocWeakPtrHub() ______________________________________________________

	WeakPtrHub * GearBase::_allocWeakPtrHub()
	{
		assert( s_pPtrPool != nullptr );
		WeakPtrHub * pHub = (WeakPtrHub*) s_pPtrPool->allocEntry();

		new (pHub) WeakPtrHub();

		return pHub;
	}

	//____ _freeWeakPtrHub() _______________________________________________________

	void GearBase::_freeWeakPtrHub( WeakPtrHub * pHub )
	{
		assert( s_pPtrPool != nullptr );
		pHub->~WeakPtrHub();
		s_pPtrPool->freeEntry( pHub );
	}


} // namespace wg
