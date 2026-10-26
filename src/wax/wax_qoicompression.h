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
#ifndef	WG_QOICOMPRESSION_DOT_H
#define	WG_QOICOMPRESSION_DOT_H
#pragma once

#include <wg_compression.h>
#include <wg_gfxtypes.h>


/*
*	QOI compression of 32-bit pixels (ARGB_8 or XRGB_8).
*
*	The compressed data is binary compatible with the data of a QOI image
*	(qoiformat.org), that is a QOI file consists of its 14 byte header followed
*	by exactly what the compressor produces. The header is not included, since
*	surface files and graphics streams keep the size of the image elsewhere.
*
*	QOI chunks:
*
*	11111110 r g b			RGB, alpha unchanged.
*	11111111 r g b a		RGBA.
*	00 iiiiii				Pixel from index. Index of a pixel is (r*3 + g*5 + b*7 + a*11) % 64.
*	01 rr gg bb				Difference to previous pixel, -2 to +1 per channel (bias 2).
*	10 gggggg rrrr bbbb		Green difference -32 to +31 (bias 32), red and blue difference to
*							the green difference -8 to +7 (bias 8).
*	11 nnnnnn				Repeat previous pixel 1 - 62 times (bias 1).
*
*	Quirks of the format that are followed:
*
*	- Previous pixel starts as {r:0, g:0, b:0, a:255} and the index as all zeroes (also alpha).
*	- The 8-bit tags RGB and RGBA take precedence over the 2-bit run tag, which
*	  is why runs are limited to 62.
*	- Differences wrap around, so 255 + 1 = 0 and 0 - 1 = 255.
*	- The same index is never issued twice in a row, a run is used instead.
*	- The stream ends with an end marker of seven 0x00 bytes and one 0x01 byte.
*	  Since two index chunks with index 0 never follow each other, the end marker
*	  can't be mistaken for pixels. The decompressor stops at the end marker, so
*	  the end of the data doesn't need to be known exactly.
*	- With 3 channels (QOI images without alpha, which we use for XRGB_8) alpha
*	  stays at 255 throughout the stream.
*
*	QOI knows nothing about pixel formats, only of r, g, b and a. Which byte of the
*	uncompressed pixels is red, green, blue and alpha is given by the pixel format
*	and byte order the compressor is set up for. The ID token tells the byte order
*	('QOIL' for little endian, 'QOIB' for big endian), so the decompressor can put
*	r, g, b and a back where they were.
*/


namespace wg
{
	// A QOI image file is a 14 byte header ('qoif', width and height as big endian
	// uint32_t, channels and colorspace as uint8_t) followed by the compressed data.

	const int		c_qoiHeaderSize = 14;
	const uint32_t	c_qoiMaxPixels = 400000000;		// Same limit as the reference implementation.

	class QOICompressor;
	typedef StrongPtr<QOICompressor>	QOICompressor_p;
	typedef WeakPtr<QOICompressor>		QOICompressor_wp;

	class QOIDecompressor;
	typedef StrongPtr<QOIDecompressor>	QOIDecompressor_p;
	typedef WeakPtr<QOIDecompressor>	QOIDecompressor_wp;


	//____ QOICompressor _________________________________________________________

	class QOICompressor : public Compressor
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			PixelFormat		format = PixelFormat::ARGB_8;			// ARGB_8 or XRGB_8. Alpha is ignored for XRGB_8 and compressed as 255.
			bool			bigEndian = (WG_IS_BIG_ENDIAN == 1);	// Byte order of the pixels.
			Finalizer_p		finalizer = nullptr;
		};

		//.____ Creation __________________________________________________________

		static QOICompressor_p		create();
		static QOICompressor_p		create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;

		const static uint32_t	ID_TOKEN;				// Little endian pixels.
		const static uint32_t	ID_TOKEN_BIG_ENDIAN;	// Big endian pixels.

		//.____ Misc ____________________________________________________

		int		maxCompressedSize( int uncompressedSize ) override;

		int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

		PixelFormat	format() const { return m_format; }
		bool		isBigEndian() const { return m_bBigEndian; }
		int			channels() const { return m_format == PixelFormat::XRGB_8 ? 3 : 4; }		// As in the header of a QOI image.

	protected:
		QOICompressor( const Blueprint& blueprint );
		virtual ~QOICompressor() {};

		PixelFormat	m_format;
		bool		m_bBigEndian;
	};


	//____ QOIDecompressor ______________________________________________________

	class QOIDecompressor : public Decompressor
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			bool			bigEndian = (WG_IS_BIG_ENDIAN == 1);	// Byte order of the pixels.
			Finalizer_p		finalizer = nullptr;
		};

		//.____ Creation __________________________________________________________

		static QOIDecompressor_p	create();
		static QOIDecompressor_p	create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;

		const static uint32_t	ID_TOKEN;				// Little endian pixels.
		const static uint32_t	ID_TOKEN_BIG_ENDIAN;	// Big endian pixels.

		//.____ Misc ____________________________________________________

		int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;
		int		decompress( void * pDest, int maxBytes, const void * pSrcBegin, const void * pSrcEnd );	// Stops after maxBytes of pixels.

		bool	isBigEndian() const { return m_bBigEndian; }

	protected:
		QOIDecompressor( const Blueprint& blueprint );
		virtual ~QOIDecompressor() {};

		bool		m_bBigEndian;
	};

}


#endif //WG_QOICOMPRESSION_DOT_H
