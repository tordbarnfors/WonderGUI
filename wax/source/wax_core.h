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
#ifndef	WAX_CORE_DOT_H
#define	WAX_CORE_DOT_H
#pragma once

#include <assert.h>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <string>
#include <vector>

#include <wax_strongptr.h>
#include <wax_weakptr.h>
#include <wax_object.h>
#include <wax_types.h>
#include <wax_gfxdevice.h>
#include <wax_gfxdevicefactory.h>
#include <wax_compression.h>

namespace wax
{
	class MemPool;
	class WeakPtrHub;
	class MemStack;
	class Object;

	class BitmapCache;
	typedef	StrongPtr<BitmapCache>		BitmapCache_p;

	class SurfaceFactory;
	typedef	StrongPtr<SurfaceFactory>	SurfaceFactory_p;

	class GfxDeviceFactory;
	typedef	StrongPtr<GfxDeviceFactory>	GfxDeviceFactory_p;


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

	//____ Context ____________________________________________________________

	class Context : public Object
	{
		friend class Core;

		std::function<void(Error&)>	pErrorHandler = nullptr;

		SurfaceFactory_p	pDefaultSurfaceFactory;
		EdgemapFactory_p	pDefaultEdgemapFactory;
		WarpmapFactory_p	pDefaultWarpmapFactory;
		GfxDeviceFactory_p	pDefaultGfxDeviceFactory;
		GfxDevice_p			pDefaultGfxDevice;
		BitmapCache_p		pDefaultBitmapCache;
	};

	typedef	StrongPtr<Context>	Context_p;
	typedef	WeakPtr<Context>	Context_wp;

	//____ Core _______________________________________________________________
	/**
	 * @brief	Static core class for Wax.
	 *
	 * A static base class for Wax that handles initialization and general
	 * housekeeping and resource allocation.
	 *
	 * The first thing that you need to do when starting Wax is to call Core::init()
	 * and the last thing you should do is to call Core::exit().
	 *
	 */

	class Core
	{
//		friend class Object_wp;
		friend class Object;

		friend class WeakPtrHub;

	public:

		//.____ Creation __________________________________________

		static bool init();
		static bool exit();

		//.____ Content _____________________________________________

		static Context_p			setContext( const Context_p& pNewContext );
		static Context_p			context() { return s_pContext; }

		static void					setErrorHandler(std::function<void(Error&)> handler);
		std::function<void(Error&)>	errorHandler();

		static BitmapCache_p		defaultBitmapCache();

		static void					setDefaultSurfaceFactory( SurfaceFactory * pFactory );
		static inline SurfaceFactory_p	defaultSurfaceFactory() { return s_pContext->pDefaultSurfaceFactory; };

		static void					setDefaultEdgemapFactory(EdgemapFactory* pFactory);
		static inline EdgemapFactory_p	defaultEdgemapFactory() { return s_pContext->pDefaultEdgemapFactory; };

		static void					setDefaultWarpmapFactory(WarpmapFactory* pFactory);
		static inline WarpmapFactory_p	defaultWarpmapFactory() { return s_pContext->pDefaultWarpmapFactory; };

		static void					setDefaultGfxDeviceFactory(GfxDeviceFactory* pFactory);
		static inline GfxDeviceFactory_p	defaultGfxDeviceFactory() { return s_pContext->pDefaultGfxDeviceFactory; };

		static void					setDefaultGfxDevice( GfxDevice * pDevice );
		static inline GfxDevice_p	defaultGfxDevice() { return s_pContext->pDefaultGfxDevice; };

		static Decompressor_p		getDecompressor( uint32_t idToken );
		static void					addDecompressor( Decompressor * pDecompressor );

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

		static constexpr int *		curveTab() { return s_curveTab; }
		static constexpr int		curveTabSize() { return c_nCurveTabEntries; }

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

		static Context_p					s_pContext;

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

	private:

		static void					_genCurveTab();

		const static int 			c_nCurveTabEntries = 1024;
		static int 					s_curveTab[c_nCurveTabEntries];

		static std::vector<Decompressor_p>	s_decompressors;
	};



} // namespace wax
#endif //WAX_CORE_DOT_H
