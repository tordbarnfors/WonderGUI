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

#include <wax_qoicompression.h>
#include <wax_util.h>
#include <wax_core.h>

#include <climits>

namespace wax
{
const TypeInfo QOICompressor::TYPEINFO = { "QOICompressor", &Compressor::TYPEINFO };
const TypeInfo QOIDecompressor::TYPEINFO = { "QOIDecompressor", &Decompressor::TYPEINFO };

const uint32_t QOICompressor::ID_TOKEN = Util::makeEndianSpecificToken('Q', 'O', 'I', 'L');
const uint32_t QOICompressor::ID_TOKEN_BIG_ENDIAN = Util::makeEndianSpecificToken('Q', 'O', 'I', 'B');
const uint32_t QOIDecompressor::ID_TOKEN = Util::makeEndianSpecificToken('Q', 'O', 'I', 'L');
const uint32_t QOIDecompressor::ID_TOKEN_BIG_ENDIAN = Util::makeEndianSpecificToken('Q', 'O', 'I', 'B');

static const uint8_t	c_opIndex	= 0x00;
static const uint8_t	c_opDiff	= 0x40;
static const uint8_t	c_opLuma	= 0x80;
static const uint8_t	c_opRun		= 0xC0;
static const uint8_t	c_opRGB		= 0xFE;
static const uint8_t	c_opRGBA	= 0xFF;

static const uint8_t	c_endMarker[8] = { 0,0,0,0,0,0,0,1 };

// Pixels are handled as 0xAARRGGBB internally, whatever their byte order in memory.

//____ _loadPixel() ___________________________________________________________

template<bool BIGENDIAN>
static inline uint32_t _loadPixel( const uint8_t * p )
{
	if( BIGENDIAN )
		return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
	else
		return (uint32_t(p[3]) << 24) | (uint32_t(p[2]) << 16) | (uint32_t(p[1]) << 8) | uint32_t(p[0]);
}

//____ _storePixel() __________________________________________________________

template<bool BIGENDIAN>
static inline void _storePixel( uint8_t * p, uint32_t pixel )
{
	if( BIGENDIAN )
	{
		p[0] = uint8_t(pixel >> 24);
		p[1] = uint8_t(pixel >> 16);
		p[2] = uint8_t(pixel >> 8);
		p[3] = uint8_t(pixel);
	}
	else
	{
		p[0] = uint8_t(pixel);
		p[1] = uint8_t(pixel >> 8);
		p[2] = uint8_t(pixel >> 16);
		p[3] = uint8_t(pixel >> 24);
	}
}

//____ _hash() ________________________________________________________________

static inline int _hash( uint32_t pixel )
{
	uint32_t a = pixel >> 24;
	uint32_t r = (pixel >> 16) & 0xFF;
	uint32_t g = (pixel >> 8) & 0xFF;
	uint32_t b = pixel & 0xFF;

	return int(r * 3 + g * 5 + b * 7 + a * 11) & 63;
}

//____ _encode() ______________________________________________________________

template<bool BIGENDIAN>
static int _encode( uint8_t * pDest, const uint8_t * pBegin, const uint8_t * pEnd, uint32_t alphaMask )
{
	uint32_t	index[64] = {};
	uint32_t	prev = 0xFF000000;
	int			run = 0;

	uint8_t * pWrite = pDest;

	for( const uint8_t * pRead = pBegin ; pRead < pEnd ; pRead += 4 )
	{
		uint32_t pixel = _loadPixel<BIGENDIAN>(pRead) | alphaMask;

		if( pixel == prev )
		{
			run++;
			if( run == 62 )
			{
				*pWrite++ = c_opRun | (run - 1);
				run = 0;
			}
			continue;
		}

		if( run > 0 )
		{
			*pWrite++ = c_opRun | (run - 1);
			run = 0;
		}

		int indexPos = _hash(pixel);

		if( index[indexPos] == pixel )
			*pWrite++ = c_opIndex | indexPos;
		else
		{
			index[indexPos] = pixel;

			uint8_t r = uint8_t(pixel >> 16);
			uint8_t g = uint8_t(pixel >> 8);
			uint8_t b = uint8_t(pixel);

			if( (pixel & 0xFF000000) == (prev & 0xFF000000) )
			{
				int8_t vr = int8_t(r - uint8_t(prev >> 16));
				int8_t vg = int8_t(g - uint8_t(prev >> 8));
				int8_t vb = int8_t(b - uint8_t(prev));

				int8_t vgr = int8_t(vr - vg);
				int8_t vgb = int8_t(vb - vg);

				if( vr > -3 && vr < 2 && vg > -3 && vg < 2 && vb > -3 && vb < 2 )
					*pWrite++ = c_opDiff | ((vr + 2) << 4) | ((vg + 2) << 2) | (vb + 2);
				else if( vgr > -9 && vgr < 8 && vg > -33 && vg < 32 && vgb > -9 && vgb < 8 )
				{
					*pWrite++ = c_opLuma | (vg + 32);
					*pWrite++ = ((vgr + 8) << 4) | (vgb + 8);
				}
				else
				{
					*pWrite++ = c_opRGB;
					*pWrite++ = r;
					*pWrite++ = g;
					*pWrite++ = b;
				}
			}
			else
			{
				*pWrite++ = c_opRGBA;
				*pWrite++ = r;
				*pWrite++ = g;
				*pWrite++ = b;
				*pWrite++ = uint8_t(pixel >> 24);
			}
		}

		prev = pixel;
	}

	if( run > 0 )
		*pWrite++ = c_opRun | (run - 1);

	for( int i = 0 ; i < 8 ; i++ )
		*pWrite++ = c_endMarker[i];

	return int(pWrite - pDest);
}

//____ _decode() ______________________________________________________________
//
// Decodes until end marker, end of source or until maxPixels have been written.
// Reads all of a chunk before writing its pixels, so decompression can be done
// in place, with the compressed data at the end of the destination buffer.

template<bool BIGENDIAN>
static int _decode( uint8_t * pDest, int maxPixels, const uint8_t * pRead, const uint8_t * pEnd )
{
	uint32_t	index[64] = {};
	uint32_t	pixel = 0xFF000000;
	int			pixelsLeft = maxPixels;

	uint8_t * pWrite = pDest;

	while( pRead < pEnd && pixelsLeft > 0 )
	{
		uint8_t	tag = *pRead++;
		int		nPixels = 1;

		if( tag == c_opRGB )
		{
			if( pEnd - pRead < 3 )
				break;

			pixel = (pixel & 0xFF000000) | (uint32_t(pRead[0]) << 16) | (uint32_t(pRead[1]) << 8) | uint32_t(pRead[2]);
			pRead += 3;
		}
		else if( tag == c_opRGBA )
		{
			if( pEnd - pRead < 4 )
				break;

			pixel = (uint32_t(pRead[3]) << 24) | (uint32_t(pRead[0]) << 16) | (uint32_t(pRead[1]) << 8) | uint32_t(pRead[2]);
			pRead += 4;
		}
		else
		{
			switch( tag & 0xC0 )
			{
				case c_opIndex:
				{
					// Index 0 directly followed by index 0 is never issued, so this is the end marker.

					if( tag == 0 && pEnd - pRead >= 7 && pRead[0] == 0 && pRead[1] == 0 && pRead[2] == 0 &&
						pRead[3] == 0 && pRead[4] == 0 && pRead[5] == 0 && pRead[6] == 1 )
						return int(pWrite - pDest);

					pixel = index[tag];
					break;
				}

				case c_opDiff:
				{
					uint32_t r = (pixel >> 16) + ((tag >> 4) & 0x03) - 2;
					uint32_t g = (pixel >> 8) + ((tag >> 2) & 0x03) - 2;
					uint32_t b = pixel + (tag & 0x03) - 2;

					pixel = (pixel & 0xFF000000) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
					break;
				}

				case c_opLuma:
				{
					if( pRead == pEnd )
						return int(pWrite - pDest);

					uint8_t data = *pRead++;
					int vg = (tag & 0x3F) - 32;

					uint32_t r = (pixel >> 16) + vg - 8 + ((data >> 4) & 0x0F);
					uint32_t g = (pixel >> 8) + vg;
					uint32_t b = pixel + vg - 8 + (data & 0x0F);

					pixel = (pixel & 0xFF000000) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
					break;
				}

				default:	// c_opRun
					nPixels = (tag & 0x3F) + 1;
					break;
			}
		}

		index[_hash(pixel)] = pixel;

		if( nPixels > pixelsLeft )
			nPixels = pixelsLeft;

		pixelsLeft -= nPixels;

		for( int i = 0 ; i < nPixels ; i++ )
		{
			_storePixel<BIGENDIAN>(pWrite, pixel);
			pWrite += 4;
		}
	}

	return int(pWrite - pDest);
}


//____ create() _____________________________________________________________

QOICompressor_p QOICompressor::create()
{
	return QOICompressor_p( new QOICompressor( Blueprint() ) );
}

QOICompressor_p QOICompressor::create( const Blueprint& blueprint )
{
	if( blueprint.format != PixelFormat::ARGB_8 && blueprint.format != PixelFormat::XRGB_8 )
	{
		Core::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "QOI compression is only supported for pixel formats ARGB_8 and XRGB_8.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
		return nullptr;
	}

	return QOICompressor_p( new QOICompressor(blueprint) );
}

//____ constructor ____________________________________________________________

QOICompressor::QOICompressor( const Blueprint& blueprint )
{
	m_format = blueprint.format;
	m_bBigEndian = blueprint.bigEndian;

	if( blueprint.finalizer )
		setFinalizer(blueprint.finalizer);
}

//____ typeInfo() _________________________________________________________

const TypeInfo& QOICompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t QOICompressor::idToken() const
{
	return m_bBigEndian ? ID_TOKEN_BIG_ENDIAN : ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int QOICompressor::maxCompressedSize( int uncompressedSize )
{
	// Worst case scenario is an RGBA chunk (5 bytes) for every pixel, plus the end marker.

	return (uncompressedSize + 3) / 4 * 5 + 8;
}

//____ compress() _____________________________________________________________

int QOICompressor::compress( void * pDest, const void * pBegin, const void * pEnd )
{
	if( (((const uint8_t*) pEnd) - ((const uint8_t*) pBegin)) % 4 != 0 )
	{
		Core::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Size of data to compress is not a multiple of 4 bytes.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	uint32_t alphaMask = m_format == PixelFormat::XRGB_8 ? 0xFF000000 : 0;

	if( m_bBigEndian )
		return _encode<true>( (uint8_t*) pDest, (const uint8_t*) pBegin, (const uint8_t*) pEnd, alphaMask );
	else
		return _encode<false>( (uint8_t*) pDest, (const uint8_t*) pBegin, (const uint8_t*) pEnd, alphaMask );
}


//____ create() _____________________________________________________________

QOIDecompressor_p QOIDecompressor::create()
{
	return QOIDecompressor_p( new QOIDecompressor( Blueprint() ) );
}

QOIDecompressor_p QOIDecompressor::create( const Blueprint& blueprint )
{
	return QOIDecompressor_p( new QOIDecompressor(blueprint) );
}

//____ constructor ____________________________________________________________

QOIDecompressor::QOIDecompressor( const Blueprint& blueprint )
{
	m_bBigEndian = blueprint.bigEndian;

	if( blueprint.finalizer )
		setFinalizer(blueprint.finalizer);
}

//____ typeInfo() _________________________________________________________

const TypeInfo& QOIDecompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t QOIDecompressor::idToken() const
{
	return m_bBigEndian ? ID_TOKEN_BIG_ENDIAN : ID_TOKEN;
}

//____ decompress() ___________________________________________________________

int QOIDecompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	return decompress( pDest, INT_MAX, pBegin, pEnd );
}

int QOIDecompressor::decompress( void * pDest, int maxBytes, const void * pBegin, const void * pEnd )
{
	if( m_bBigEndian )
		return _decode<true>( (uint8_t*) pDest, maxBytes / 4, (const uint8_t*) pBegin, (const uint8_t*) pEnd );
	else
		return _decode<false>( (uint8_t*) pDest, maxBytes / 4, (const uint8_t*) pBegin, (const uint8_t*) pEnd );
}

}
