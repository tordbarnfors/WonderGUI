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
#include <wax_core.h>
#include <wax_memstack.h>
#include <wax_mempool.h>
#include <wax_object.h>
#include <wax_bitmapcache.h>
#include <wax_pixeltools.h>
#include <wax_edgemaptools.h>
#include <wax_util.h>

#include <wax_compression.h>
#include <wax_q565compression.h>
#include <wax_qoicompression.h>
#include <wax_lzcompression.h>
#include <wax_spxcompression.h>
#include <wax_rlecompression.h>

#include <iostream>
#include <algorithm>
#include <cassert>

namespace wax
{
	const TypeInfo	Core::TYPEINFO = { "Core", nullptr };

	int							Core::s_initCounter = 0;

	Context_p					Core::s_pContext;

	MemPool *					Core::s_pPtrPool = nullptr;
	MemStack *					Core::s_pMemStack = nullptr;

	std::atomic<unsigned int>	Core::s_objectsCreated;
	std::atomic<unsigned int>	Core::s_objectsDestroyed;


	bool						Core::s_bTrackingObjects = false;

	std::unordered_map<Object*, Core::ObjectInfo>	Core::s_trackedObjects;

	int 						Core::s_curveTab[c_nCurveTabEntries];

	std::vector<Decompressor_p> Core::s_decompressors;


	namespace PixelTools
	{
		void _initTables();
		void _releaseTables();
	}


	//____ init() __________________________________________________________________

	bool Core::init()
	{
		if( s_initCounter == 0 )
		{
			s_objectsCreated = 0;
			s_objectsDestroyed = 0;
			s_pPtrPool = new MemPool( 128, sizeof( WeakPtrHub ) );
			s_pMemStack = new MemStack( 4096 );

			s_pContext = new Context();

			HiColor::_initTables();
			PixelTools::_initTables();
			_genCurveTab();
		}

		s_initCounter++;
		return true;
	}

	//____ exit() __________________________________________________________________

	bool Core::exit()
	{
		if( s_initCounter <= 0 )
		{
			throwError(ErrorLevel::SilentError, ErrorCode::IllegalCall, "Call to Core::exit() ignored, not initialized or already exited.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if( s_initCounter > 1 )
		{
			s_initCounter--;
			return true;
		}

		if( s_pContext->pDefaultBitmapCache )
			s_pContext->pDefaultBitmapCache->clear();

		// We need to make sure our objects are destroyed before continuing.

		s_pContext->pDefaultSurfaceFactory = nullptr;
		s_pContext->pDefaultEdgemapFactory = nullptr;
		s_pContext->pDefaultWarpmapFactory = nullptr;
		s_pContext->pDefaultDevice = nullptr;
		s_pContext->pDefaultBitmapCache = nullptr;
		s_pContext->pDefaultDeviceFactory = nullptr;

		PixelTools::_releaseTables();

		s_decompressors.clear();

		if( !s_pPtrPool->isEmpty() )
		{
			throwError(ErrorLevel::SilentError, ErrorCode::SystemIntegrity, "Some weak pointers still in use. Can not exit Wax.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
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
			throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "Some objects still alive after Wax exit. Might cause problems when they go out of scope. Forgotten to clear pointers?\nHint: Enable object tracking to find out which ones.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);

		s_pContext = nullptr;
		return true;
	}

	//____ setContext() _______________________________________________________

	Context_p Core::setContext( const Context_p& pNewContext )
	{
		Context_p p = s_pContext;

		if( pNewContext )
			s_pContext = pNewContext;
		else
			s_pContext = new Context();
		return p;
	}

	//____ throwError() _________________________________________________________

	void Core::throwError(ErrorLevel severity, ErrorCode code, const char * msg, const Object * pObject, const TypeInfo* pClassType, const char * function, const char * file, int line)
	{
		if (s_pContext != nullptr && s_pContext->pErrorHandler)
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

			s_pContext->pErrorHandler(error);
		}
	}

	//____ beginObjectTracking() _________________________________________________

	void Core::beginObjectTracking()
	{
		s_bTrackingObjects = true;
	}

	//____ endObjectTracking() __________________________________________________

	void Core::endObjectTracking()
	{
		s_bTrackingObjects = false;
		s_trackedObjects.clear();
	}

	//____ printObjects() ________________________________________________

	void Core::printObjects(std::ostream& stream)
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


	//____ setErrorHandler() _________________________________________________________

	void Core::setErrorHandler(std::function<void(Error&)> handler)
	{
		s_pContext->pErrorHandler = handler;
	}

	//____ errorHandler() ____________________________________________________________

	std::function<void(Error&)>	Core::errorHandler()
	{
		return s_pContext->pErrorHandler;
	}

	//____ memStackAlloc() ________________________________________________________

	char * Core::memStackAlloc( int bytes )
	{
		assert(s_pMemStack != nullptr);
		return s_pMemStack->allocBytes(bytes);
	}

	//____ memStackFree() ______________________________________________________

	void Core::memStackFree( int bytes )
	{	assert(s_pMemStack != nullptr);
		return s_pMemStack->releaseBytes(bytes);
	}

	//____ _trackObject() ________________________________________________________

	void Core::_trackObject(Object* pObject, const char* pFileName, int lineNb)
	{
        if( s_bTrackingObjects )
            s_trackedObjects[pObject] = { pFileName, lineNb, s_objectsCreated };
	}

	//____ _allocWeakPtrHub() ______________________________________________________

	WeakPtrHub * Core::_allocWeakPtrHub()
	{
		assert( s_pPtrPool != nullptr );
		WeakPtrHub * pHub = (WeakPtrHub*) s_pPtrPool->allocEntry();

		new (pHub) WeakPtrHub();

		return pHub;
	}

	//____ _freeWeakPtrHub() _______________________________________________________

	void Core::_freeWeakPtrHub( WeakPtrHub * pHub )
	{
		assert( s_pPtrPool != nullptr );
		pHub->~WeakPtrHub();
		s_pPtrPool->freeEntry( pHub );
	}


//____ defaultBitmapCache() __________________________________________________

	BitmapCache_p Core::defaultBitmapCache()
	{
		if( s_pContext->pDefaultBitmapCache == nullptr )
			s_pContext->pDefaultBitmapCache = BitmapCache::create(16*1024*1024);

		return s_pContext->pDefaultBitmapCache;
	}

	//____ setDefaultSurfaceFactory() ____________________________________________

	void Core::setDefaultSurfaceFactory( SurfaceFactory * pFactory )
	{
		s_pContext->pDefaultSurfaceFactory = pFactory;
	}

	//____ setDefaultEdgemapFactory() ____________________________________________

	void Core::setDefaultEdgemapFactory(EdgemapFactory* pFactory)
	{
		s_pContext->pDefaultEdgemapFactory = pFactory;
	}

	//____ setDefaultWarpmapFactory() ____________________________________________

	void Core::setDefaultWarpmapFactory(WarpmapFactory* pFactory)
	{
		s_pContext->pDefaultWarpmapFactory = pFactory;
	}

	//____ setDefaultDeviceFactory() ____________________________________________

	void Core::setDefaultDeviceFactory(DeviceFactory* pFactory)
	{
		s_pContext->pDefaultDeviceFactory = pFactory;
	}

	//____ setDefaultDevice() _________________________________________________

	void Core::setDefaultDevice( Device * pDevice )
	{
		s_pContext->pDefaultDevice = pDevice;
	}

	//____ getDecompressor() _______________________________________________________

	Decompressor_p Core::getDecompressor( uint32_t idToken )
	{
		for( auto& p : s_decompressors )
			if( p->idToken() == idToken )
				return p;

		Decompressor_p pDecompressor;

		if( idToken == Q565Decompressor::ID_TOKEN )
			pDecompressor = Q565Decompressor::create();
		else if( idToken == LZDecompressor::ID_TOKEN )
			pDecompressor = LZDecompressor::create();
		else if( idToken == SPXDecompressor::ID_TOKEN )
			pDecompressor = SPXDecompressor::create();
		else if (idToken == RLEDecompressor::ID_TOKEN)
			pDecompressor = RLEDecompressor::create();
		else if (idToken == QOIDecompressor::ID_TOKEN || idToken == QOIDecompressor::ID_TOKEN_BIG_ENDIAN)
		{
			QOIDecompressor::Blueprint bp;
			bp.bigEndian = (idToken == QOIDecompressor::ID_TOKEN_BIG_ENDIAN);
			pDecompressor = QOIDecompressor::create(bp);
		}

		if( pDecompressor )
			s_decompressors.push_back(pDecompressor);

		return pDecompressor;
	}

	//____ addDecompressor() _______________________________________________________

	void Core::addDecompressor( Decompressor * pDecompressor )
	{
		s_decompressors.push_back(pDecompressor);
	}

	//____ _genCurveTab() ___________________________________________________________

	void Core::_genCurveTab()
	{
		//		double factor = 3.14159265 / (2.0 * c_nCurveTabEntries);

		for (int i = 0; i < c_nCurveTabEntries; i++)
		{
			double y = 1.0 - i / (double)c_nCurveTabEntries;
			s_curveTab[i] = (int)(Util::squareRoot(1.0 - y*y)*65536.0);
		}
	}

} // namespace wax
