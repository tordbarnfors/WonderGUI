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
#include <wg_surfacewriter.h>
#include <wg_gfxbase.h>
#include <cstring>

namespace wg
{
	const TypeInfo SurfaceWriter::TYPEINFO = { "SurfaceWriter", &Object::TYPEINFO };


	//.____ create() _____________________________________________________________

	SurfaceWriter_p SurfaceWriter::create( const Blueprint& blueprint )
	{
		return SurfaceWriter_p( new SurfaceWriter(blueprint) );

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SurfaceWriter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ writeSurfaceToStream() _____________________________________________

	void SurfaceWriter::writeSurfaceToStream( std::ostream& stream, Surface * pSurface, int extraDataSize, char * pExtraData )
	{
		// Fill in header

		SurfaceFileHeader	header;
		int headerBytes = _prepareHeader(&header, pSurface, extraDataSize);

		// Calculate buffer needed for pixels, palette and extra data

		int bytesNeeded = _safePixelBufferSize(pSurface);

		if (pSurface->palette())
			bytesNeeded += _safePaletteBufferSize(pSurface);

		if (pExtraData && extraDataSize > 0)
			bytesNeeded += _safeExtrasBufferSize(pExtraData, pExtraData + extraDataSize);

		// Allocate buffer

		char* pBuffer = GfxBase::memStackAlloc(bytesNeeded);
		auto pWrite = pBuffer;

		// Encode data into buffer and update header info

		int bytesPixelData = _encodePixelData(pBuffer, pSurface);
		_setPixelDataInfo(&header, bytesPixelData, 0);

		pWrite += bytesPixelData;

		for (int i = 0; i < header.pixelDataPadding; i++)
			*pWrite++ = 0;

		int bytesPaletteData = 0;
		if (pSurface->palette())
		{
			int8_t	filterParams[8] = { 0,0,0,0,0,0,0,0 };

			bytesPaletteData = _encodePaletteData(pWrite, pSurface);
			_setPaletteDataInfo(&header, bytesPaletteData, 0);
			pWrite += bytesPaletteData;

			for (int i = 0; i < header.paletteDataPadding; i++)
				*pWrite++ = 0;
		}

		int bytesExtrasData = 0;
		if (pExtraData && extraDataSize > 0)
		{
			bytesExtrasData = _encodeExtrasData(pWrite, pExtraData, pExtraData + extraDataSize);
			_setExtraDataInfo(&header, bytesExtrasData, 0);
			pWrite += bytesExtrasData;
		}

		// Write header

		stream.write((char*)&header, headerBytes);

		// Write compressed data

		stream.write(pBuffer, bytesPixelData + bytesPaletteData + bytesExtrasData);

		// Release temp buffer

		GfxBase::memStackFree(bytesNeeded);
	}

	//____ _writeSurfaceToBlob() _________________________________________________

	Blob_p SurfaceWriter::writeSurfaceToBlob( Surface * pSurface, int extraDataSize, char * pExtraData )
	{
		// Fill in header

		SurfaceFileHeader	header;
		int headerBytes = _prepareHeader(&header, pSurface, extraDataSize);

		// Calculate buffer needed for pixels, palette and extra data

		int bytesNeeded = _safePixelBufferSize( pSurface );

		if (pSurface->palette())
			bytesNeeded += _safePaletteBufferSize( pSurface );

		if( pExtraData && extraDataSize > 0 )
			bytesNeeded += _safeExtrasBufferSize(pExtraData, pExtraData + extraDataSize);

		// Allocate buffer

		char * pBuffer = GfxBase::memStackAlloc(bytesNeeded);
		auto pWrite = pBuffer;

		// Encode data into buffer and update header info

		int bytesPixelData = _encodePixelData(pBuffer, pSurface );
		_setPixelDataInfo(&header, bytesPixelData, 0);

		pWrite += bytesPixelData;

		for (int i = 0; i < header.pixelDataPadding; i++)
			*pWrite++ = 0;

		int bytesPaletteData = 0;
		if (pSurface->palette())
		{
			int8_t	filterParams[8] = { 0,0,0,0,0,0,0,0 };

			bytesPaletteData = _encodePaletteData(pWrite, pSurface );
			_setPaletteDataInfo(&header, bytesPaletteData, 0);
			pWrite += bytesPaletteData;

			for (int i = 0; i < header.paletteDataPadding; i++)
				*pWrite++ = 0;
		}

		int bytesExtrasData = 0;
		if( pExtraData && extraDataSize > 0 )
		{
			bytesExtrasData = _encodeExtrasData(pWrite, pExtraData, pExtraData + extraDataSize);
			_setExtraDataInfo(&header, bytesExtrasData, 0);
			pWrite += bytesExtrasData;
		}

		// Calculate size needed for blob and create blob

		int size = headerBytes + bytesPixelData + bytesPaletteData + bytesExtrasData;

		Blob_p pBlob = Blob::create(size);

		pWrite = (char*) pBlob->data();

		// Write header

		std::memcpy(pWrite, &header, headerBytes);
		pWrite += headerBytes;

		// Write compressed data

		std::memcpy(pWrite, pBuffer, bytesPixelData + bytesPaletteData + bytesExtrasData);

		// Release temp buffer

		GfxBase::memStackFree(bytesNeeded);

		return pBlob;
	}


	//____ _prepareHeader() _____________________________________________________

	int SurfaceWriter::_prepareHeader( SurfaceFileHeader * pHeader, Surface * pSurface, bool bExtrasData )
	{
		auto bp = pSurface->blueprint();

		auto sz = pSurface->pixelSize();

		pHeader->width = bp.size.w;
		pHeader->height = bp.size.h;
		pHeader->format = bp.format;

		if( m_saveInfo.sampleMethod )
			pHeader->sampleMethod = bp.sampleMethod;

		if( m_saveInfo.buffered )
			pHeader->buffered = bp.buffered;

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

	bool  SurfaceWriter::_setPixelDataInfo(SurfaceFileHeader* pHeader, int bytesOfCompressedPixels, int decompressMargin)
	{
		if( m_pPixelCompressor )
			pHeader->pixelCompression = m_pPixelCompressor->idToken();

		pHeader->pixelDecompressMargin = decompressMargin;

		int paddedPixelBytes = bytesOfCompressedPixels + 3 & ~0x03; // Ensure 32-bit alignment

		pHeader->pixelBytes = paddedPixelBytes;
		pHeader->pixelDataPadding = (int8_t)(paddedPixelBytes - bytesOfCompressedPixels);
		return true;
	}

	//____ _setPaletteDataInfo() _________________________________________________

	bool SurfaceWriter::_setPaletteDataInfo(SurfaceFileHeader* pHeader,	int bytesOfCompressedPalette, int decompressMargin)
	{
		if( m_pPaletteCompressor )
			pHeader->paletteCompression = m_pPaletteCompressor->idToken();

		pHeader->paletteDecompressMargin = decompressMargin;

		int paddedPaletteBytes = bytesOfCompressedPalette + 3 & ~0x03; // Ensure 32-bit alignment

		pHeader->paletteBytes = paddedPaletteBytes;
		pHeader->paletteDataPadding = (int8_t)(paddedPaletteBytes - bytesOfCompressedPalette);

		return true;
	}

	//____ _setExtraDataInfo() _________________________________________________

	bool SurfaceWriter::_setExtraDataInfo(SurfaceFileHeader* pHeader, int bytesOfCompressedExtraData, int decompressMargin)
	{
		if( m_pExtrasCompressor )
			pHeader->extraDataCompression = m_pExtrasCompressor->idToken();

		pHeader->extraDataDecompressMargin = decompressMargin;
		pHeader->extraDataBytes = bytesOfCompressedExtraData;
		return true;
	}

	//____ _safePixelBufferSize() _________________________________________________

	int SurfaceWriter::_safePixelBufferSize(Surface* pSurface)
	{
		int uncompressedSize = pSurface->pixelWidth() * pSurface->pixelHeight() * pSurface->pixelBits() / 8;

		if( m_pPixelCompressor )
			return m_pPixelCompressor->maxCompressedSize(uncompressedSize);
		else
			return uncompressedSize;
	}

	//____ _safePaletteBufferSize() _________________________________________________

	int  SurfaceWriter::_safePaletteBufferSize(Surface* pSurface)
	{
		int uncompressedSize = pSurface->paletteSize() * sizeof(Color8);

		if( m_pPaletteCompressor )
			return m_pPaletteCompressor->maxCompressedSize(uncompressedSize);
		else
			return uncompressedSize;
	}

	//____ _safeExtrasBufferSize() _________________________________________________

	int  SurfaceWriter::_safeExtrasBufferSize(void* pBegin, void* pEnd)
	{
		int uncompressedSize = (int)((uint8_t*)pEnd - (uint8_t*)pBegin);

		if( m_pExtrasCompressor )
			return m_pExtrasCompressor->maxCompressedSize(uncompressedSize);
		else
			return uncompressedSize;
	}

	//____ _encodePixelData() _________________________________________________

	int SurfaceWriter::_encodePixelData(void* pDest, Surface* pSurface)
	{
		uint8_t* pWrite = (uint8_t*)pDest;

		auto pixbuf = pSurface->allocPixelBuffer();
		pSurface->pushPixels(pixbuf);

		SizeI size = pSurface->pixelSize();

		int lineBytes = size.w * (pSurface->pixelBits() / 8);

		if( m_pPixelCompressor )
		{
			int bytes = m_pPixelCompressor->compress(pWrite, pixbuf.pixels, pixbuf.pixels + lineBytes * size.h );
			pWrite += bytes;
		}
		else
		{
			if (pixbuf.pitch > lineBytes)
			{
				// Pitch is involved, we need to write line by line

				char* pPixels = (char*)pixbuf.pixels;

				for (int y = 0; y < size.h; y++)
				{
					std::memcpy(pWrite, pPixels, lineBytes);
					pWrite += lineBytes;
					pPixels += pixbuf.pitch;
				}
			}
			else
			{
				std::memcpy(pWrite, pixbuf.pixels, lineBytes * size.h);
				pWrite += lineBytes * size.h;
			}
		}

		pSurface->freePixelBuffer(pixbuf);
		return int(pWrite - (uint8_t*)pDest);
	}

	//____ _encodePaletteData() _________________________________________________

	int SurfaceWriter::_encodePaletteData(void* pDest, Surface* pSurface)
	{
		auto pPalette = pSurface->palette();
		int nColors = pSurface->paletteSize();

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

	int SurfaceWriter::_encodeExtrasData(void* pDest, void* pBegin, void* pEnd)
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

} // namespace wg

