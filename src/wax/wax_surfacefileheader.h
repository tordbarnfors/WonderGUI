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
	int16_t			versionNumber			= 2;
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
			uint16_t		bigEndian: 1;		// Byte order of pixels and of 16-bit palette indexes. Version 2+.
			uint16_t		linear: 1;			// Pixels (and palette) are in linear color space, otherwise sRGB. Version 2+.
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

//____ SurfaceFileLayout ______________________________________________________

// Layout of the pixels in a surface file, also for version 1 files, whose pixel formats
// include color space and byte order.

struct SurfaceFileLayout
{
	PixelFormat			format = PixelFormat::Undefined;	// Undefined if not supported.
	ColorSpace			colorSpace = ColorSpace::sRGB;
	bool				bigEndian = false;
	PixelDescription	description;						// Layout of pixels in file. 24 bits for some version 1 files.
};

inline SurfaceFileLayout surfaceFileLayout( const SurfaceFileHeader& header )
{
	SurfaceFileLayout layout;

	if( header.versionNumber >= 2 )
	{
		if( header.versionNumber > 2 || header.format > PixelFormat_max )
			return layout;

		layout.format = header.format;
		layout.colorSpace = header.linear ? ColorSpace::Linear : ColorSpace::sRGB;
		layout.bigEndian = header.bigEndian;
		layout.description = Util::pixelFormatToDescription(layout.format, layout.bigEndian);
		return layout;
	}

	// Version 1. Pixel formats are numbered as they were then.

	struct V1Format
	{
		PixelFormat format;
		ColorSpace	colorSpace;
		bool		bigEndian;
		bool		b24Bits;
	};

	static const V1Format v1Formats[] = {
		{ PixelFormat::Undefined,		ColorSpace::sRGB,	false,	false },	// Undefined
		{ PixelFormat::XRGB_8,			ColorSpace::sRGB,	false,	true },		// BGR_8
		{ PixelFormat::XRGB_8,			ColorSpace::sRGB,	false,	true },		// BGR_8_sRGB
		{ PixelFormat::XRGB_8,			ColorSpace::Linear,	false,	true },		// BGR_8_linear
		{ PixelFormat::XRGB_8,			ColorSpace::sRGB,	false,	false },	// BGRX_8
		{ PixelFormat::XRGB_8,			ColorSpace::sRGB,	false,	false },	// BGRX_8_sRGB
		{ PixelFormat::XRGB_8,			ColorSpace::Linear,	false,	false },	// BGRX_8_linear
		{ PixelFormat::ARGB_8,			ColorSpace::sRGB,	false,	false },	// BGRA_8
		{ PixelFormat::ARGB_8,			ColorSpace::sRGB,	false,	false },	// BGRA_8_sRGB
		{ PixelFormat::ARGB_8,			ColorSpace::Linear,	false,	false },	// BGRA_8_linear
		{ PixelFormat::Index_8,			ColorSpace::sRGB,	false,	false },	// Index_8
		{ PixelFormat::Index_8,			ColorSpace::sRGB,	false,	false },	// Index_8_sRGB
		{ PixelFormat::Index_8,			ColorSpace::Linear,	false,	false },	// Index_8_linear
		{ PixelFormat::Index_16,		ColorSpace::sRGB,	false,	false },	// Index_16
		{ PixelFormat::Index_16,		ColorSpace::sRGB,	false,	false },	// Index_16_sRGB
		{ PixelFormat::Index_16,		ColorSpace::Linear,	false,	false },	// Index_16_linear
		{ PixelFormat::Alpha_8,			ColorSpace::sRGB,	false,	false },	// Alpha_8
		{ PixelFormat::Undefined,		ColorSpace::sRGB,	false,	false },	// BGRA_4_linear (no longer supported)
		{ PixelFormat::RGB_565,			ColorSpace::sRGB,	false,	false },	// BGR_565
		{ PixelFormat::RGB_565,			ColorSpace::sRGB,	false,	false },	// BGR_565_sRGB
		{ PixelFormat::RGB_565,			ColorSpace::Linear,	false,	false },	// BGR_565_linear
		{ PixelFormat::BGR_565,			ColorSpace::Linear,	true,	false },	// RGB_565_bigendian
		{ PixelFormat::BGR_565,			ColorSpace::Linear,	true,	false },	// RGB_555_bigendian (lowest green bit always cleared)
		{ PixelFormat::Bitplanes_1,		ColorSpace::Linear,	true,	false },	// Bitplanes_1
		{ PixelFormat::Bitplanes_2,		ColorSpace::Linear,	true,	false },	// Bitplanes_2
		{ PixelFormat::Bitplanes_4,		ColorSpace::Linear,	true,	false },	// Bitplanes_4
		{ PixelFormat::Bitplanes_5,		ColorSpace::Linear,	true,	false },	// Bitplanes_5
		{ PixelFormat::Bitplanes_8,		ColorSpace::Linear,	true,	false },	// Bitplanes_8
		{ PixelFormat::Bitplanes_A1_1,	ColorSpace::Linear,	true,	false },	// Bitplanes_A1_1
		{ PixelFormat::Bitplanes_A1_2,	ColorSpace::Linear,	true,	false },	// Bitplanes_A1_2
		{ PixelFormat::Bitplanes_A1_4,	ColorSpace::Linear,	true,	false },	// Bitplanes_A1_4
		{ PixelFormat::Bitplanes_A1_5,	ColorSpace::Linear,	true,	false },	// Bitplanes_A1_5
		{ PixelFormat::Bitplanes_A1_8,	ColorSpace::Linear,	true,	false },	// Bitplanes_A1_8
		{ PixelFormat::XRGB_16,			ColorSpace::Linear,	false,	false },	// BGRX_16_linear
		{ PixelFormat::ARGB_16,			ColorSpace::Linear,	false,	false }		// BGRA_16_linear
	};

	int index = int(header.format);
	if( index >= int(sizeof(v1Formats)/sizeof(V1Format)) )
		return layout;

	auto& v1 = v1Formats[index];

	layout.format = v1.format;
	layout.colorSpace = v1.colorSpace;
	layout.bigEndian = v1.bigEndian;

	if( v1.b24Bits )
		layout.description = PixelDescription(24, PixelType::Chunky, 0xFF0000, 0xFF00, 0xFF, 0, false);
	else
		layout.description = Util::pixelFormatToDescription(layout.format, layout.bigEndian);

	return layout;
}

} //namespace

#endif //WG_SURFACEFILEHEADER_DOT_H
