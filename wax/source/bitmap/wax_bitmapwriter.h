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
#ifndef WAX_BITMAPWRITER_DOT_H
#define WAX_BITMAPWRITER_DOT_H
#pragma once

#include <ostream>

#include <wax_bitmapfileheader.h>
#include <wax_bitmapfactory.h>
#include <wax_compression.h>

namespace wax
{


	class BitmapWriter;
	typedef	StrongPtr<BitmapWriter>BitmapWriter_p;
	typedef	WeakPtr<BitmapWriter>	BitmapWriter_wp;




class BitmapWriter : public Object
{
public:

	struct SaveInfo
	{
		bool identity		= true;
		bool sampleMethod 	= false;
		bool scale			= true;
		bool canvas			= false;
		bool dynamic		= false;
		bool tiling			= true;
		bool mipmap			= false;
	};
	
	struct Blueprint
	{
		Finalizer_p			finalizer = nullptr;
		Compressor_p		pixelCompressor;
		Compressor_p		paletteCompressor;
		Compressor_p		extrasCompressor;
		SaveInfo			saveInfo;
	};

	//.____ Creation __________________________________________

	static BitmapWriter_p	create() { return create( Blueprint() ); }
	static BitmapWriter_p	create( const Blueprint& blueprint );

	//.____ Identification _________________________________________________

	const TypeInfo&	typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;
	
	void			writeBitmapToStream( std::ostream& stream, Bitmap * pBitmap, int extraDataSize = 0, char * pExtraData = nullptr );
	Blob_p			writeBitmapToBlob( Bitmap * pBitmap, int extraDataSize = 0, char * pExtraData = nullptr );

	// Writes bitmap as a QOI image. Pixels are converted to 8-bit RGB or RGBA (if bitmap has alpha),
	// in the color space of the bitmap. Compressors and SaveInfo are ignored.

	bool			writeBitmapToQOIStream( std::ostream& stream, Bitmap * pBitmap );
	Blob_p			writeBitmapToQOIBlob( Bitmap * pBitmap );

protected:

	template<class BP>
	BitmapWriter(const BP& bp)
	{
		m_saveInfo = bp.saveInfo;
		m_pPixelCompressor = bp.pixelCompressor;
		m_pPaletteCompressor = bp.paletteCompressor;
		m_pExtrasCompressor = bp.extrasCompressor;

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	virtual ~BitmapWriter() {}
	
	int				_prepareHeader( BitmapFileHeader * pHeader, Bitmap * pBitmap, bool bExtrasData );
	bool			_setPixelDataInfo(	BitmapFileHeader* pHeader, int bytesOfCompressedPixels, int decompressMargin = 0 );
	bool			_setPaletteDataInfo(BitmapFileHeader* pHeader, int bytesOfCompressedPalette, int decompressMargin = 0 );
	bool			_setExtraDataInfo(BitmapFileHeader* pHeader, int bytesOfCompressedExtraData, int decompressMargin = 0);

	int				_safePixelBufferSize(Bitmap* pBitmap);
	int				_safePaletteBufferSize(Bitmap* pBitmap);
	int				_safeExtrasBufferSize(void * pBegin, void * pEnd);

	int				_encodePixelData(void* pDest, Bitmap* pBitmap);
	int				_encodePaletteData(void* pDest, Bitmap* pBitmap);
	int				_encodeExtrasData(void* pDest, void * pBegin, void * pEnd);

	int				_safeQOIBufferSize(Bitmap* pBitmap);
	int				_encodeQOI(void* pDest, Bitmap* pBitmap);
	int				_qoiChannels(Bitmap* pBitmap);


	SaveInfo			m_saveInfo;

	Compressor_p		m_pPixelCompressor;
	Compressor_p		m_pPaletteCompressor;
	Compressor_p		m_pExtrasCompressor;

};


//==============================================================================

} // namespace wax
#endif // WAX_BITMAPREADER_DOT_H

