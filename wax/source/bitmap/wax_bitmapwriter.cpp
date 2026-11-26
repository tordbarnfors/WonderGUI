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
#include <wax_bitmapwriter.h>
#include <wax_base.h>
#include <wax_util.h>
#include <wax_pixeltools.h>
#include <wax_qoicompression.h>
#include <cstring>

namespace wax
{
	const TypeInfo BitmapWriter::TYPEINFO = { "BitmapWriter", &Object::TYPEINFO };


	//.____ create() _____________________________________________________________

	BitmapWriter_p BitmapWriter::create( const Blueprint& blueprint )
	{
		return BitmapWriter_p( new BitmapWriter(blueprint) );

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BitmapWriter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ writeBitmapToStream() _____________________________________________

	void BitmapWriter::writeBitmapToStream( std::ostream& stream, Bitmap * pBitmap, int extraDataSize, char * pExtraData )
	{
		// Fill in header

		BitmapFileHeader	header;
		int headerBytes = _prepareHeader(&header, pBitmap, extraDataSize);

		// Calculate buffer needed for pixels, palette and extra data

		int bytesNeeded = _safePixelBufferSize(pBitmap) + 3;		// + alignment padding

		if (pBitmap->palette())
			bytesNeeded += _safePaletteBufferSize(pBitmap) + 3;		// + alignment padding

		if (pExtraData && extraDataSize > 0)
			bytesNeeded += _safeExtrasBufferSize(pExtraData, pExtraData + extraDataSize);

		// Allocate buffer

		char* pBuffer = Base::memStackAlloc(bytesNeeded);
		auto pWrite = pBuffer;

		// Encode data into buffer and update header info

		// Palette data

		int bytesPaletteData = 0;
		if (pBitmap->palette())
		{
			int8_t	filterParams[8] = { 0,0,0,0,0,0,0,0 };

			bytesPaletteData = _encodePaletteData(pWrite, pBitmap);
			_setPaletteDataInfo(&header, bytesPaletteData, 0);
			pWrite += bytesPaletteData;

			for (int i = 0; i < header.paletteDataPadding; i++)
				*pWrite++ = 0;

			bytesPaletteData = header.paletteBytes;		// Include padding
		}

		// Pixel data

		int bytesPixelData = _encodePixelData(pWrite, pBitmap);
		_setPixelDataInfo(&header, bytesPixelData, 0);

		pWrite += bytesPixelData;

		for (int i = 0; i < header.pixelDataPadding; i++)
			*pWrite++ = 0;

		bytesPixelData = header.pixelBytes;		// Include padding

		// Extra data

		int bytesExtrasData = 0;
		if (pExtraData && extraDataSize > 0)
		{
			bytesExtrasData = _encodeExtrasData(pWrite, pExtraData, pExtraData + extraDataSize);
			_setExtraDataInfo(&header, bytesExtrasData, 0);
			pWrite += bytesExtrasData;
		}

		// Write header

		stream.write((char*)&header, headerBytes);

		// Write compressed data, including padding

		stream.write(pBuffer, pWrite - pBuffer);

		// Release temp buffer

		Base::memStackFree(bytesNeeded);
	}

	//____ _writeBitmapToBlob() _________________________________________________

	Blob_p BitmapWriter::writeBitmapToBlob( Bitmap * pBitmap, int extraDataSize, char * pExtraData )
	{
		// Fill in header

		BitmapFileHeader	header;
		int headerBytes = _prepareHeader(&header, pBitmap, extraDataSize);

		// Calculate buffer needed for pixels, palette and extra data

		int bytesNeeded = _safePixelBufferSize( pBitmap ) + 3;		// + alignment padding

		if (pBitmap->palette())
			bytesNeeded += _safePaletteBufferSize( pBitmap ) + 3;		// + alignment padding

		if( pExtraData && extraDataSize > 0 )
			bytesNeeded += _safeExtrasBufferSize(pExtraData, pExtraData + extraDataSize);

		// Allocate buffer

		char * pBuffer = Base::memStackAlloc(bytesNeeded);
		auto pWrite = pBuffer;

		// Encode data into buffer and update header info

		int bytesPaletteData = 0;
		if (pBitmap->palette())
		{
			int8_t	filterParams[8] = { 0,0,0,0,0,0,0,0 };

			bytesPaletteData = _encodePaletteData(pWrite, pBitmap );
			_setPaletteDataInfo(&header, bytesPaletteData, 0);
			pWrite += bytesPaletteData;

			for (int i = 0; i < header.paletteDataPadding; i++)
				*pWrite++ = 0;

			bytesPaletteData = header.paletteBytes;		// Include padding
		}

		int bytesPixelData = _encodePixelData(pWrite, pBitmap );
		_setPixelDataInfo(&header, bytesPixelData, 0);

		pWrite += bytesPixelData;

		for (int i = 0; i < header.pixelDataPadding; i++)
			*pWrite++ = 0;

		bytesPixelData = header.pixelBytes;		// Include padding

		int bytesExtrasData = 0;
		if( pExtraData && extraDataSize > 0 )
		{
			bytesExtrasData = _encodeExtrasData(pWrite, pExtraData, pExtraData + extraDataSize);
			_setExtraDataInfo(&header, bytesExtrasData, 0);
			pWrite += bytesExtrasData;
		}

		// Calculate size needed for blob and create blob

		int dataBytes = int(pWrite - pBuffer);		// Includes padding
		int size = headerBytes + dataBytes;

		Blob_p pBlob = Blob::create(size);

		pWrite = (char*) pBlob->data();

		// Write header

		std::memcpy(pWrite, &header, headerBytes);
		pWrite += headerBytes;

		// Write compressed data

		std::memcpy(pWrite, pBuffer, dataBytes);

		// Release temp buffer

		Base::memStackFree(bytesNeeded);

		return pBlob;
	}


	//____ writeBitmapToQOIStream() __________________________________________

	bool BitmapWriter::writeBitmapToQOIStream( std::ostream& stream, Bitmap * pBitmap )
	{
		int bufferSize = _safeQOIBufferSize(pBitmap);
		if( bufferSize == 0 )
			return false;

		char * pBuffer = Base::memStackAlloc(bufferSize);

		int bytes = _encodeQOI(pBuffer, pBitmap);
		stream.write(pBuffer, bytes);

		Base::memStackFree(bufferSize);
		return stream.good();
	}

	//____ writeBitmapToQOIBlob() ____________________________________________

	Blob_p BitmapWriter::writeBitmapToQOIBlob( Bitmap * pBitmap )
	{
		int bufferSize = _safeQOIBufferSize(pBitmap);
		if( bufferSize == 0 )
			return nullptr;

		char * pBuffer = Base::memStackAlloc(bufferSize);

		int bytes = _encodeQOI(pBuffer, pBitmap);

		Blob_p pBlob = Blob::create(bytes);
		std::memcpy(pBlob->data(), pBuffer, bytes);

		Base::memStackFree(bufferSize);
		return pBlob;
	}

	//____ _prepareHeader() _____________________________________________________

	int BitmapWriter::_prepareHeader( BitmapFileHeader * pHeader, Bitmap * pBitmap, bool bExtrasData )
	{
		auto bp = pBitmap->blueprint();

		auto sz = pBitmap->pixelSize();

		pHeader->width = bp.size.w;
		pHeader->height = bp.size.h;
		pHeader->format = bp.format;
		pHeader->bigEndian = bp.bigEndian;
		pHeader->linear = (bp.colorSpace == ColorSpace::Linear);

		if( m_saveInfo.sampleMethod )
			pHeader->sampleMethod = bp.sampleMethod;

		if( m_saveInfo.canvas )
			pHeader->canvas = bp.canvas;

		if( m_saveInfo.dynamic )
			pHeader->dynamic = bp.dynamic;

		if( m_saveInfo.tiling )
			pHeader->tiling = bp.tiling;

		if( m_saveInfo.mipmap )
			pHeader->mipmap = bp.mipmap;

		if( m_saveInfo.scale )
			pHeader->scale = bp.scale;

		if( m_saveInfo.identity )
			pHeader->identity = bp.identity;

		if( bp.palette )
			pHeader->paletteSize = bp.paletteSize;

		int headerBytes;
		if( bExtrasData )
			headerBytes = 88;
		else if( bp.palette )
			headerBytes = 80;
		else if( m_pPixelCompressor )
			headerBytes = 56;
		else if( (m_saveInfo.scale && bp.scale != 64) || (m_saveInfo.identity && bp.identity != 0) )
			headerBytes = 40;
		else
			headerBytes = 32;

		pHeader->headerBytes = headerBytes;

		return headerBytes;
	}

	//____ _setPixelDataInfo() _________________________________________________

	bool  BitmapWriter::_setPixelDataInfo(BitmapFileHeader* pHeader, int bytesOfCompressedPixels, int decompressMargin)
	{
		if( m_pPixelCompressor )
			pHeader->pixelCompression = m_pPixelCompressor->idToken();

		pHeader->pixelDecompressMargin = decompressMargin;

		int paddedPixelBytes = (bytesOfCompressedPixels + 3) & ~0x03; // Ensure 32-bit alignment

		pHeader->pixelBytes = paddedPixelBytes;
		pHeader->pixelDataPadding = (int8_t)(paddedPixelBytes - bytesOfCompressedPixels);
		return true;
	}

	//____ _setPaletteDataInfo() _________________________________________________

	bool BitmapWriter::_setPaletteDataInfo(BitmapFileHeader* pHeader,	int bytesOfCompressedPalette, int decompressMargin)
	{
		if( m_pPaletteCompressor )
			pHeader->paletteCompression = m_pPaletteCompressor->idToken();

		pHeader->paletteDecompressMargin = decompressMargin;

		int paddedPaletteBytes = (bytesOfCompressedPalette + 3) & ~0x03; // Ensure 32-bit alignment

		pHeader->paletteBytes = paddedPaletteBytes;
		pHeader->paletteDataPadding = (int8_t)(paddedPaletteBytes - bytesOfCompressedPalette);

		return true;
	}

	//____ _setExtraDataInfo() _________________________________________________

	bool BitmapWriter::_setExtraDataInfo(BitmapFileHeader* pHeader, int bytesOfCompressedExtraData, int decompressMargin)
	{
		if( m_pExtrasCompressor )
			pHeader->extraDataCompression = m_pExtrasCompressor->idToken();

		pHeader->extraDataDecompressMargin = decompressMargin;
		pHeader->extraDataBytes = bytesOfCompressedExtraData;
		return true;
	}

	//____ _safePixelBufferSize() _________________________________________________

	int BitmapWriter::_safePixelBufferSize(Bitmap* pBitmap)
	{
		int uncompressedSize = PixelTools::bytesPerLine(*pBitmap->pixelDescription(), pBitmap->pixelWidth()) * pBitmap->pixelHeight();

		// Includes up to 3 bytes of padding for 32-bit alignment.

		if( m_pPixelCompressor )
			return m_pPixelCompressor->maxCompressedSize(uncompressedSize) + 3;
		else
			return uncompressedSize + 3;
	}

	//____ _safePaletteBufferSize() _________________________________________________

	int  BitmapWriter::_safePaletteBufferSize(Bitmap* pBitmap)
	{
		int uncompressedSize = pBitmap->paletteSize() * sizeof(Color8);

		// Includes up to 3 bytes of padding for 32-bit alignment.

		if( m_pPaletteCompressor )
			return m_pPaletteCompressor->maxCompressedSize(uncompressedSize) + 3;
		else
			return uncompressedSize + 3;
	}

	//____ _safeExtrasBufferSize() _________________________________________________

	int  BitmapWriter::_safeExtrasBufferSize(void* pBegin, void* pEnd)
	{
		int uncompressedSize = (int)((uint8_t*)pEnd - (uint8_t*)pBegin);

		if( m_pExtrasCompressor )
			return m_pExtrasCompressor->maxCompressedSize(uncompressedSize);
		else
			return uncompressedSize;
	}

	//____ _encodePixelData() _________________________________________________

	int BitmapWriter::_encodePixelData(void* pDest, Bitmap* pBitmap)
	{
		uint8_t* pWrite = (uint8_t*)pDest;

		auto pixbuf = pBitmap->allocPixelBuffer();
		pBitmap->pushPixels(pixbuf);

		SizeI size = pBitmap->pixelSize();
		auto desc = pBitmap->pixelDescription();
		int lineBytes = PixelTools::bytesPerLine(*desc, size.w);

		// Pixels are saved in the format, color space and byte order of the bitmap,
		// which is not necessarily how the PixelBuffer has them.

		const uint8_t * pPixels = pixbuf.pixels;
		int pitch = pixbuf.pitch;
		int convertedBytes = 0;

		if( pixbuf.format != pBitmap->pixelFormat() || pixbuf.colorSpace != pBitmap->colorSpace() || pixbuf.bigEndian != pBitmap->isBigEndian() )
		{
			convertedBytes = lineBytes * size.h;
			uint8_t * pConverted = (uint8_t*) Base::memStackAlloc(convertedBytes);

			auto bufDesc = Util::pixelFormatToDescription(pixbuf.format, pixbuf.bigEndian);
			int paletteEntries = pBitmap->paletteSize();

			PixelTools::copyPixels(size.w, size.h, pixbuf.pixels, bufDesc, pixbuf.colorSpace, pixbuf.pitch - PixelTools::bytesPerLine(bufDesc, size.w),
								   pixbuf.palette, pBitmap->paletteSize(), pConverted, pBitmap->pixelFormat(), pBitmap->colorSpace(), pBitmap->isBigEndian(), 0,
								   const_cast<Color8*>(pBitmap->palette()), paletteEntries, pBitmap->paletteCapacity());

			pPixels = pConverted;
			pitch = lineBytes;
		}
		else if( m_pPixelCompressor && pitch > lineBytes )
		{
			// Compressors need the pixels without the padding at the end of each line.

			convertedBytes = lineBytes * size.h;
			uint8_t * pPacked = (uint8_t*) Base::memStackAlloc(convertedBytes);

			for (int y = 0; y < size.h; y++)
				std::memcpy(pPacked + y * lineBytes, pPixels + y * pitch, lineBytes);

			pPixels = pPacked;
			pitch = lineBytes;
		}

		if( m_pPixelCompressor )
		{
			int bytes = m_pPixelCompressor->compress(pWrite, pPixels, pPixels + lineBytes * size.h );
			pWrite += bytes;
		}
		else
		{
			if (pitch > lineBytes)
			{
				// Pitch is involved, we need to write line by line

				const uint8_t* p = pPixels;

				for (int y = 0; y < size.h; y++)
				{
					std::memcpy(pWrite, p, lineBytes);
					pWrite += lineBytes;
					p += pitch;
				}
			}
			else
			{
				std::memcpy(pWrite, pPixels, lineBytes * size.h);
				pWrite += lineBytes * size.h;
			}
		}

		if( convertedBytes > 0 )
			Base::memStackFree(convertedBytes);

		pBitmap->freePixelBuffer(pixbuf);

		return int(pWrite - (uint8_t*)pDest);
	}

	//____ _encodePaletteData() _________________________________________________

	int BitmapWriter::_encodePaletteData(void* pDest, Bitmap* pBitmap)
	{
		auto pPalette = pBitmap->palette();
		int nColors = pBitmap->paletteSize();

		if( pPalette == nullptr || nColors == 0 )
			return 0;

		if( m_pPaletteCompressor )
			return m_pPaletteCompressor->compress(pDest, pPalette, pPalette + nColors );
		else
		{
			std::memcpy( pDest, pPalette, nColors * sizeof(Color8) );
			return nColors * sizeof(Color8);
		}
	}

	//____ _encodeExtrasData() _________________________________________________

	int BitmapWriter::_encodeExtrasData(void* pDest, void* pBegin, void* pEnd)
	{
		if( m_pExtrasCompressor )
			return m_pExtrasCompressor->compress(pDest, pBegin, pEnd);
		else
		{
			int dataSize = int((uint8_t*)pEnd - (uint8_t*)pBegin);
			std::memcpy( pDest, pBegin, dataSize );
			return dataSize;
		}
	}

	//____ _safeQOIBufferSize() _________________________________________________

	int BitmapWriter::_safeQOIBufferSize(Bitmap* pBitmap)
	{
		SizeI size = pBitmap->pixelSize();

		if( size.w <= 0 || size.h <= 0 || uint32_t(size.h) >= c_qoiMaxPixels / uint32_t(size.w) )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Bitmap is empty or too large to be saved as a QOI image.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return 0;
		}

		// Worst case scenario is a 5 byte RGBA chunk per pixel, plus header and end marker.

		return c_qoiHeaderSize + size.w * size.h * 5 + 8;
	}

	//____ _qoiChannels() _______________________________________________________

	int BitmapWriter::_qoiChannels(Bitmap* pBitmap)
	{
		if( pBitmap->pixelDescription()->A_mask != 0 )
			return 4;

		auto pPalette = pBitmap->palette();
		if( pPalette )
		{
			int paletteSize = pBitmap->paletteSize();
			for( int i = 0 ; i < paletteSize ; i++ )
			{
				if( pPalette[i].a != 255 )
					return 4;
			}
		}

		return 3;
	}

	//____ _encodeQOI() _________________________________________________________

	int BitmapWriter::_encodeQOI(void* pDest, Bitmap* pBitmap)
	{
		uint8_t* pWrite = (uint8_t*)pDest;

		SizeI size = pBitmap->pixelSize();
		int channels = _qoiChannels(pBitmap);
		ColorSpace colorSpace = pBitmap->colorSpace();

		// Write header

		auto write32 = [](uint8_t * p, uint32_t v) { p[0] = uint8_t(v >> 24); p[1] = uint8_t(v >> 16); p[2] = uint8_t(v >> 8); p[3] = uint8_t(v); };

		std::memcpy(pWrite, "qoif", 4);
		write32(pWrite + 4, uint32_t(size.w));
		write32(pWrite + 8, uint32_t(size.h));
		pWrite[12] = uint8_t(channels);
		pWrite[13] = colorSpace == ColorSpace::Linear ? 1 : 0;
		pWrite += c_qoiHeaderSize;

		// Get pixels as ARGB_8 or XRGB_8, which the QOI compressor needs.

		auto pixbuf = pBitmap->allocPixelBuffer();
		pBitmap->pushPixels(pixbuf);

		PixelFormat format = channels == 4 ? PixelFormat::ARGB_8 : PixelFormat::XRGB_8;
		int lineBytes = size.w * 4;

		bool bSameLayout = (pixbuf.format == PixelFormat::ARGB_8 || pixbuf.format == format) && pixbuf.colorSpace == colorSpace &&
							pixbuf.pitch == lineBytes;

		const uint8_t * pPixels = pixbuf.pixels;
		bool bBigEndian = pixbuf.bigEndian;
		int convertedBytes = 0;

		if( !bSameLayout )
		{
			convertedBytes = lineBytes * size.h;
			uint8_t * pConverted = (uint8_t*) Base::memStackAlloc(convertedBytes);

			auto bufDesc = Util::pixelFormatToDescription(pixbuf.format, pixbuf.bigEndian);
			int dummyPaletteEntries = 0;

			PixelTools::copyPixels(size.w, size.h, pixbuf.pixels, pixbuf.format, pixbuf.colorSpace, pixbuf.bigEndian, pixbuf.pitch - PixelTools::bytesPerLine(bufDesc, size.w),
								   pixbuf.palette, pBitmap->paletteSize(), pConverted, format, colorSpace, false, 0,
								   nullptr, dummyPaletteEntries, 0);

			pPixels = pConverted;
			bBigEndian = false;
		}

		// Compress. Alpha of XRGB_8 is ignored by the compressor, as QOI wants it for 3 channels.

		auto pCompressor = QOICompressor::create( WAXBP(QOICompressor, _.format = format, _.bigEndian = bBigEndian ) );

		pWrite += pCompressor->compress(pWrite, pPixels, pPixels + lineBytes * size.h);

		if( convertedBytes > 0 )
			Base::memStackFree(convertedBytes);

		pBitmap->freePixelBuffer(pixbuf);

		return int(pWrite - (uint8_t*)pDest);
	}

} // namespace wax
