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
#include <wg_gfxbase.h>
#include <wg_bitmapcache.h>
#include <wg_pixeltools.h>
#include <wg_edgemaptools.h>
#include <wg_gfxutil.h>

#include <wg_compression.h>
#include <wg_q565compression.h>
#include <wg_lzcompression.h>
#include <wg_spxcompression.h>
#include <wg_rlecompression.h>
#include <wg_legacycompression.h>



namespace wg
{
	int					GfxBase::s_gfxInitCounter = 0;

	GfxContext_p		GfxBase::s_pGfxContext;

	int 				GfxBase::s_curveTab[c_nCurveTabEntries];

	std::vector<Compressor_p> GfxBase::s_decompressors;


	namespace PixelTools
	{
		extern uint8_t* pConv_16_linear_to_8_sRGB;
	}



	//____ init() __________________________________________________________________

	bool GfxBase::init( )
	{
		if( s_gfxInitCounter == 0 )
		{
			if( !GearBase::init() )
			   return false;

			s_pGfxContext = new GfxContext();
			s_pGfxContext->pGearContext = GearBase::context();
			
			HiColor::_initTables();
			_genCurveTab();
		}
		
		s_gfxInitCounter++;
		return true;
	}

	//____ exit() __________________________________________________________________

	bool GfxBase::exit()
	{
		if( s_gfxInitCounter > 1 )
		{
			s_gfxInitCounter--;
			return true;
		}
		
		if( s_gfxInitCounter <= 0 )
		{
			throwError(ErrorLevel::SilentError, ErrorCode::IllegalCall, "Call to GfxBase::exit() ignored, not initialized or already exited.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}
		
		s_gfxInitCounter = 0;

		if( s_pGfxContext->pDefaultBitmapCache )
			s_pGfxContext->pDefaultBitmapCache->clear();

		// We need to make sure our objects are destroyed before continuing.

		s_pGfxContext->pDefaultSurfaceFactory = nullptr;
		s_pGfxContext->pDefaultEdgemapFactory = nullptr;
		s_pGfxContext->pDefaultGfxDevice = nullptr;
		s_pGfxContext->pDefaultBitmapCache = nullptr;
		s_pGfxContext->pDefaultGfxDeviceFactory = nullptr;
		
		s_pGfxContext = nullptr;
		
		delete [] PixelTools::pConv_16_linear_to_8_sRGB;
		PixelTools::pConv_16_linear_to_8_sRGB = nullptr;

		s_decompressors.clear();

		return GearBase::exit();
	}

	//____ setContext() _______________________________________________________

	GfxContext_p GfxBase::setContext( const GfxContext_p& pNewContext )
	{
		auto pOld = s_pGfxContext;
		
		if( pNewContext )
			s_pGfxContext = pNewContext;
		else
			s_pGfxContext = new GfxContext();

		GearBase::setContext(s_pGfxContext->pGearContext);
		s_pGfxContext->pGearContext = GearBase::context();

		return pOld;
	}

	//____ defaultBitmapCache() __________________________________________________

	BitmapCache_p GfxBase::defaultBitmapCache()
	{
		if( s_pGfxContext->pDefaultBitmapCache == nullptr )
			s_pGfxContext->pDefaultBitmapCache = BitmapCache::create(16*1024*1024);

		return s_pGfxContext->pDefaultBitmapCache;
	}

	//____ setDefaultSurfaceFactory() ____________________________________________

	void GfxBase::setDefaultSurfaceFactory( SurfaceFactory * pFactory )
	{
		s_pGfxContext->pDefaultSurfaceFactory = pFactory;
	}

	//____ setDefaultEdgemapFactory() ____________________________________________

	void GfxBase::setDefaultEdgemapFactory(EdgemapFactory* pFactory)
	{
		s_pGfxContext->pDefaultEdgemapFactory = pFactory;
	}

	//____ setDefaultGfxDeviceFactory() ____________________________________________

	void GfxBase::setDefaultGfxDeviceFactory(GfxDeviceFactory* pFactory)
	{
		s_pGfxContext->pDefaultGfxDeviceFactory = pFactory;
	}

	//____ setDefaultGfxDevice() _________________________________________________

	void GfxBase::setDefaultGfxDevice( GfxDevice * pDevice )
	{
		s_pGfxContext->pDefaultGfxDevice = pDevice;
	}

	//____ setDefaultToSRGB() ____________________________________________________

	void GfxBase::setDefaultToSRGB( bool bSRGB )
	{
		s_pGfxContext->bSRGB = bSRGB;
	}

	//____ getDecompressor() _______________________________________________________

	Compressor_p GfxBase::getDecompressor( uint32_t idToken )
	{
		for( auto& p : s_decompressors )
			if( p->idToken() == idToken )
				return p;

		if( idToken == Q565Compressor::ID_TOKEN )
		{
			auto pCompressor = Q565Compressor::create( WGBP(Q565Compressor, _.decompressOnly = true) );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		if( idToken == LZCompressor::ID_TOKEN )
		{
			auto pCompressor = LZCompressor::create( WGBP(LZCompressor, _.decompressOnly = true) );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		if( idToken == SPXCompressor::ID_TOKEN )
		{
			auto pCompressor = SPXCompressor::create(WGBP(SPXCompressor, _.decompressOnly = true)  );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		if (idToken == RLECompressor::ID_TOKEN)
		{
			auto pCompressor = RLECompressor::create(WGBP(RLECompressor, _.decompressOnly = true) );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		if (idToken == U8ICompressor::ID_TOKEN)
		{
			auto pCompressor = U8ICompressor::create(WGBP(U8ICompressor, _.decompressOnly = true) );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		if (idToken == S16ICompressor::ID_TOKEN)
		{
			auto pCompressor = S16ICompressor::create(WGBP(S16ICompressor, _.decompressOnly = true) );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		if (idToken == S16BCompressor::ID_TOKEN)
		{
			auto pCompressor = S16BCompressor::create(WGBP(S16BCompressor, _.decompressOnly = true) );
			s_decompressors.push_back(pCompressor);
			return pCompressor;
		}

		return nullptr;
	}

	//____ addDecompressor() _______________________________________________________

	void GfxBase::addDecompressor( Compressor * pCompressor )
	{
		s_decompressors.push_back(pCompressor);
	}

	//____ _genCurveTab() ___________________________________________________________

	void GfxBase::_genCurveTab()
	{
		//		double factor = 3.14159265 / (2.0 * c_nCurveTabEntries);

		for (int i = 0; i < c_nCurveTabEntries; i++)
		{
			double y = 1.0 - i / (double)c_nCurveTabEntries;
			s_curveTab[i] = (int)(Util::squareRoot(1.0 - y*y)*65536.0);
		}
	}

} // namespace wg
