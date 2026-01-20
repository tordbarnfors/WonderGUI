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
#ifndef WG_SURFACEFILEHEADER_DOT_H
#define WG_SURFACEFILEHEADER_DOT_H
#pragma once

#include <cstdint>
#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_gfxutil.h>

namespace wg
{

/*
namespace SurfaceFileToken
{
	const uint32_t SURF = _makeEndianSpecificToken( 'S','U','R','F' );
	const uint32_t NONE = _makeEndianSpecificToken( 'N','O','N','E' );
	const uint32_t Q565 = _makeEndianSpecificToken( 'Q','5','6','5' );
}

enum class PixelCompression
{
	const uint32_t SURF = _makeEndianSpecificToken( 'S','U','R','F' );
	const uint32_t NONE = _makeEndianSpecificToken( 'N','O','N','E' );
	const uint32_t Q565 = _makeEndianSpecificToken( 'Q','5','6','5' );
}
*/




//____ wgsf_header ____________________________________________________________

struct SurfaceFileHeader
{
	uint32_t		identifier				= Util::makeEndianSpecificToken( 'S','U','R','F' );
	int16_t			versionNumber			= 1;
	int16_t			headerBytes				= 0;
	int32_t			paletteBytes			= 0;		// Includes padding
	int32_t			pixelBytes				= 0;		// Includes padding
	int32_t			extraDataBytes			= 0;

	int32_t			width					= 0;
	int32_t			height					= 0;
	PixelFormat		format					= PixelFormat::Undefined;
	SampleMethod	sampleMethod			= SampleMethod::Undefined;

	union
	{
		uint16_t	flags					= 0;
		struct
		{
			uint16_t		buffered: 1;
			uint16_t		canvas: 1;
			uint16_t		dynamic: 1;
			uint16_t		tiling: 1;
			uint16_t		mipmap: 1;
		};
	};


	//	--- 32 bytes header ends here

	int16_t			reserved1				= 0;
	int16_t			scale					= 64;
	int32_t			identity				= 0;

	//	-- 40 bytes header ends here.

	uint32_t		pixelFiltering			= Util::makeEndianSpecificToken( 'N','O','N','E' );
	int16_t			filterBlockWidth		= 1;
	int16_t			filterBlockHeight		= 1;
	uint32_t		pixelCompression		= Util::makeEndianSpecificToken( 'N','O','N','E' );
	int16_t			pixelDecompressMargin	= 0;
	int8_t			pixelDataPadding		= 0;					// Number of extra bytes at end of pixel data to ensure each block starts on 32-bit alignment.
	int8_t			reserved2				= 0;
	
	//	-- 56 bytes header ends here.

	int32_t			paletteSize 			= 0;
	uint32_t		paletteFiltering		= Util::makeEndianSpecificToken( 'N','O','N','E' );
	int8_t			paletteFilteringParams[8] = { 0,0,0,0,0,0,0,0 };
	uint32_t		paletteCompression		= Util::makeEndianSpecificToken( 'N','O','N','E' );
	int16_t			paletteDecompressMargin	= 0;
	int8_t			paletteDataPadding		= 0;					// Number of extra bytes at end of pixel data to ensure each block starts on 32-bit alignment.
	int8_t			reserved3				= 0;

	//	-- 80 bytes header ends here.

	uint32_t		extraDataCompression	= Util::makeEndianSpecificToken( 'N','O','N','E' );
	int16_t			extraDataDecompressMargin = 0;
	int16_t			reserved4				= 0;

	//	-- 88 bytes header ends here.

};

} //namespace

#endif //WG_SURFACEFILEHEADER_DOT_H
