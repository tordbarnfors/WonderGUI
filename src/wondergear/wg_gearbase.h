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
#ifndef	WG_GEARBASE_DOT_H
#define	WG_GEARBASE_DOT_H
#pragma once

#include <functional>
#include <unordered_map>
#include <atomic>
#include <string>

#include <wg_strongptr.h>
#include <wg_weakptr.h>
#include <wg_object.h>
#include <wg_geartypes.h>

namespace wg
{
	class MemPool;
	class WeakPtrHub;
	class MemStack;
	class Object;

	class Error
	{
	public:
		ErrorLevel	severity;
		ErrorCode		code;
		const char *	message;
		const Object *	pObject;
		const char *	classname;
		const char *	function;
		const char *	file;
		int				line;
	};

	//

	class GearContext : public Object
	{
		friend class GearBase;
		
		std::function<void(Error&)>	pErrorHandler = nullptr;
	};

	typedef StrongPtr<GearContext>	GearContext_p;
	typedef WeakPtr<GearContext>	GearContext_wp;

	/**
	 * @brief	Static base class for WonderGUI.
	 *
	 * A static base class for WonderGUI that handles initialization and general
	 * housekeeping and resource allocation.
	 *
	 * The first thing that you need to do when starting WonderGUI is to call Base::init()
	 * and the last thing you should do is to call Base::exit().
	 *
	 */

	class GearBase
	{
//		friend class Object_wp;
		friend class Object;

		friend class WeakPtrHub;

	public:

		//.____ Creation __________________________________________

		static bool init();
		static bool exit();

		//.____ Content _____________________________________________
		
		static GearContext_p		setContext( const GearContext_p& pNewContext );
		static GearContext_p		context() { return s_pGearContext; }

		
		static void					setErrorHandler(std::function<void(Error&)> handler);
		std::function<void(Error&)>	errorHandler();
				
		//.____ Misc ________________________________________________

		const static TypeInfo	TYPEINFO;

		static bool			isInitialized() { return s_pPtrPool != nullptr; }


		static char *		memStackAlloc( int bytes );
		static void			memStackFree( int bytes );

		static void			throwError( ErrorLevel severity, ErrorCode code, const char * pMsg, const Object * pObject, const TypeInfo * pClassType, const char * pFunction, const char * pFile, int line );

		static void			beginObjectTracking();
		static void			endObjectTracking();
		static bool			isObjectTracking() { return s_bTrackingObjects;  }

		static void			printObjects(std::ostream& stream);

		//.____ Internal ____________________________________________________________

		template<typename T>
		static T _trackObj(const T& pObj, const char* pFileName, int lineNb)
		{
			_trackObject(pObj, pFileName, lineNb);
			return pObj;
		}

		static void			_trackObject(Object* pObject, const char* pFileName, int lineNb);

	protected:
		static int					s_initCounter;

		static WeakPtrHub *	_allocWeakPtrHub();
		static void			_freeWeakPtrHub(WeakPtrHub * pHub);

        inline static void	_objectWasCreated(Object* pObject) { s_objectsCreated.fetch_add(1, std::memory_order_relaxed); if (s_bTrackingObjects) _trackObject(pObject, nullptr, -1); }
        inline static void	_objectWillDestroy(Object* pObject) { s_objectsDestroyed.fetch_add(1, std::memory_order_relaxed); if (s_bTrackingObjects) s_trackedObjects.erase(pObject); }

		static GearContext_p				s_pGearContext;
		
		static MemPool *					s_pPtrPool;
		static MemStack *					s_pMemStack;
		
		struct ObjectInfo
		{
			// Filename and line where this objects create() method was called.

			const char*		pFileName;
			int				lineNb;
			unsigned int	serialNb;
		};

		static std::atomic<unsigned int>	s_objectsCreated;
		static std::atomic<unsigned int>	s_objectsDestroyed;

		static bool			s_bTrackingObjects;
		static std::unordered_map<Object*, ObjectInfo>	s_trackedObjects;
	};



} // namespace wg
#endif //WG_BASE_DOT_H
