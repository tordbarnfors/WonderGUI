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
#include <wg_surfacereader.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

#include <wg_compressor.h>
#include <wg_lzcompressor.h>
#include <wg_q565compressor.h>

#include <cstring>

namespace wg
{
	const TypeInfo SurfaceReader::TYPEINFO = { "SurfaceReader", &Object::TYPEINFO };


	//.____ create() _____________________________________________________________

	SurfaceReader_p SurfaceReader::create( const Blueprint& blueprint )
	{
		return SurfaceReader_p( new SurfaceReader(blueprint) );
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SurfaceReader::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ readSurfaceFromStream() _____________________________________________

	Surface_p SurfaceReader::readSurfaceFromStream(std::istream& stream)
	{
		Surface::Blueprint dummy;
		return readSurfaceFromStream(stream, dummy);
	}

	Surface_p SurfaceReader::readSurfaceFromStream(std::istream& stream, const Surface::Blueprint& _bp)
	{

		// Determine file format

		const char id_QOI[4] = { 'q','o','i','f' };
		const char id_Surf[4] = { 'S','U','R','F' };

		char identifier[4];
		stream.read(identifier, 4);

		if (*(uint32_t*)id_Surf == *(uint32_t*)identifier)
			return _readSurfFromStream(stream, _bp);
		else if (*(uint32_t*)id_QOI == *(uint32_t*)identifier)
			return _readQOIFromStream(stream, _bp);

		GfxBase::throwError(ErrorLevel::Error, ErrorCode::Other, "Stream is not a supported image format.", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
		return nullptr;
	}

	//____ _readSurfFromStream() ______________________________________________

	Surface_p SurfaceReader::_readSurfFromStream(std::istream & stream, const Surface::Blueprint & _bp)
	{
		SurfaceFileHeader	header;

		// Read the header size

		stream.read(((char*)(&header)) + 4, 4);

		// Read the rest of the header

		stream.read( ((char*)(&header))+8, header.headerBytes - 8);

		// Prepare surface blueprint

		Surface::Blueprint bp = _blueprintFromHeader(&header);
		if (_addFlagsFromOtherBlueprint(bp, _bp) != 0)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided blueprint can not alter format, scale or palette of loaded surface but have one or more of these parameters set.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Read and prepare palette

		int paletteBytes = header.paletteSize*sizeof(Color8) + header.paletteDecompressMargin;
		Color8 * pPalette = nullptr;
		if( paletteBytes > 0 )
		{
			pPalette = (Color8*) GfxBase::memStackAlloc(paletteBytes);

			// We only support uncompressed palette for the moment

			stream.read( (char*) pPalette, paletteBytes );

			bp.palette = pPalette;
		}

		// Create surface

		auto pSurface = m_pFactory->createSurface(bp);

		if( paletteBytes > 0 )
			GfxBase::memStackFree(paletteBytes);

		// Read pixels into PixelBuffer

		auto pixbuf = pSurface->allocPixelBuffer();


		int lineBytes = header.width * pSurface->pixelBits()/8;

		if (*(uint32_t*)&header.pixelCompression == *(uint32_t*) "NONE" )
		{
			if (pixbuf.pitch > lineBytes)
			{
				// Pitch is involved, we need to read line by line

				char* pPixels = (char*)pixbuf.pixels;

				for (int y = 0; y < header.height; y++)
				{
					stream.read(pPixels, lineBytes);
					pPixels += pixbuf.pitch;
				}
			}
			else
			{
				stream.read((char*)pixbuf.pixels, lineBytes * header.height);
			}
		}
		else
		{
			Compressor * pCompressor = _findCompressor(header.pixelCompression);

			if( !pCompressor )
			{
				char msg[] = "Don't know how to decompress 'XXXX'.";
				* (uint32_t*)&msg[30] = header.pixelCompression;

				GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, msg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				pSurface->freePixelBuffer(pixbuf);
				return nullptr;
			}

			auto pDesc = pSurface->pixelDescription();

			if (pixbuf.pitch != lineBytes)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::Other, "Can only decompress compressed pixels to surfaces that have no pitch.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				pSurface->freePixelBuffer(pixbuf);
				return nullptr;
			}

			auto pData = GfxBase::memStackAlloc(header.pixelBytes);
			stream.read(pData, header.pixelBytes);
			pCompressor->decompress(pixbuf.pixels, pData, ((const uint8_t*)pData) + header.pixelBytes);
			GfxBase::memStackFree(header.pixelBytes);

		}


		pSurface->pullPixels(pixbuf);
		pSurface->freePixelBuffer(pixbuf);

		return pSurface;
	}

	//____ _readQOIFromStream() ______________________________________________

	Surface_p SurfaceReader::_readQOIFromStream(std::istream& stream, const Surface::Blueprint& _bp)
	{
		struct qoi_header{
			uint32_t width; // image width in pixels (BE)
			uint32_t height; // image height in pixels (BE)
			uint8_t channels; // 3 = RGB, 4 = RGBA
			uint8_t colorspace; // 0 = sRGB with linear alpha
								// 1 = all channels linear
		};

		qoi_header header;

		// Read header

		stream.read(((char*)(&header)), 10);

		if (!Util::isSystemBigEndian())
		{
			header.width = Util::endianSwap(header.width);
			header.height = Util::endianSwap(header.height);
		}

		// Create surface





		return nullptr;
	}

	//____ readSurfaceFromBlob() _______________________________________________

	Surface_p SurfaceReader::readSurfaceFromBlob(const Blob* pBlob)
	{
		return readSurfaceFromMemory(static_cast<const char*>(pBlob->data()));
	}

	Surface_p SurfaceReader::readSurfaceFromBlob( const Blob * pBlob, const Surface::Blueprint& bp )
	{
		return readSurfaceFromMemory( static_cast<const char*>(pBlob->data()), bp);
	}

	//____ readSurfaceFromMemory() _____________________________________________

	Surface_p SurfaceReader::readSurfaceFromMemory(const char* pData)
	{
		Surface::Blueprint dummy;
		return readSurfaceFromMemory(pData, dummy);
	}

	Surface_p SurfaceReader::readSurfaceFromMemory(const char* pData, const Surface::Blueprint& _bp)
	{
		SurfaceFileHeader	header;

		// Read the header

		int headerSize = * (const int16_t*)&pData[6];
		std::memcpy( &header, pData, headerSize);
		pData+= headerSize;

		// Prepare surface blueprint

		Surface::Blueprint bp = _blueprintFromHeader(&header);
		if (_addFlagsFromOtherBlueprint(bp, _bp) != 0)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided blueprint can not alter size, format or palette of loaded surface but have one or more of these parameters set.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Prepare palette

		int paletteBytes = header.paletteSize*sizeof(Color8);
		if( paletteBytes > 0 )
		{
			bp.palette = (Color8*) pData;
			pData += paletteBytes;
		}

		// Create surface

		auto pSurface = m_pFactory->createSurface(bp);

		// Read pixels into PixelBuffer
		// We only support uncompressed pixels for the moment

		auto pixbuf = pSurface->allocPixelBuffer();

		int lineBytes = header.width * pSurface->pixelBits()/8;

		if (header.pixelCompression == *(uint32_t*)"NONE")
			_copyUncompressedFromMemory(pixbuf.pixels, pData, lineBytes, pixbuf.pitch, pixbuf.rect.h);
		else
		{
			Compressor * pCompressor = _findCompressor(header.pixelCompression);

			if( !pCompressor )
			{
				char msg[] = "Don't know how to decompress 'XXXX'.";
				* (uint32_t*)&msg[30] = header.pixelCompression;

				GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, msg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				pSurface->freePixelBuffer(pixbuf);
				return nullptr;
			}

			auto pDesc = pSurface->pixelDescription();

			if (pixbuf.pitch != lineBytes)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::Other, "Can only decompress compressed pixels to surfaces that have no pitch.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				pSurface->freePixelBuffer(pixbuf);
				return nullptr;
			}

			pCompressor->decompress(pixbuf.pixels, pData, ((const uint8_t*)pData) + header.pixelBytes);
		}

		pSurface->pullPixels(pixbuf);
		pSurface->freePixelBuffer(pixbuf);

		return pSurface;
	}


//____ _blueprintFromHeader() _________________________________________________

Surface::Blueprint SurfaceReader::_blueprintFromHeader( const SurfaceFileHeader * pHeader )
{
	return WGBP(Surface,
				_.size 			= {pHeader->width, pHeader->height},
				_.scale 		= pHeader->scale,
				_.format 		= pHeader->format,
				_.buffered 		= pHeader->buffered,
				_.canvas 		= pHeader->canvas,
				_.dynamic 		= pHeader->dynamic,
				_.mipmap 		= pHeader->mipmap,
				_.identity		= pHeader->identity,
				_.tiling		= pHeader->tiling,
				_.sampleMethod 	= pHeader->sampleMethod,
				_.paletteSize	= pHeader->paletteSize );
}

//____ _addFlagsFromOtherBlueprint() __________________________________________

int SurfaceReader::_addFlagsFromOtherBlueprint(Surface::Blueprint& dest, const Surface::Blueprint& extraFlags)
{
	int errorCode = 0;

	if ( extraFlags.buffered )
		dest.buffered = true;

	if (extraFlags.canvas)
		dest.canvas = true;

	if (extraFlags.palette)
		errorCode = 1;

	if (extraFlags.dynamic)
		dest.dynamic = true;

	if (extraFlags.format != PixelFormat::Undefined)
		errorCode = 2;

	if (extraFlags.identity != 0)
		dest.identity = extraFlags.identity;

	if (extraFlags.mipmap)
		dest.mipmap = true;

	if (extraFlags.sampleMethod != SampleMethod::Undefined)
		dest.sampleMethod = extraFlags.sampleMethod;

	if (extraFlags.scale != 0)
		dest.scale = extraFlags.scale;

	if (!extraFlags.size.isEmpty())
		errorCode = 3;

	
	if (extraFlags.tiling)
		dest.tiling = true;

	return errorCode;
}

//____ _copyUncompressedFromMemory() _________________________________________

void SurfaceReader::_copyUncompressedFromMemory(void* pDest, const void* pSource, int rowBytes, int pitch, int rows)
{
	if (pitch > rowBytes)
	{
		// Pitch is involved, we need to copy line by line

		char* pPixelsDest = (char*)pDest;
		const char* pPixelsSource = (const char*)pSource;
		for (int y = 0; y < rows; y++)
		{
			std::memcpy(pPixelsDest, pPixelsSource, rowBytes);
			pPixelsDest += pitch;
			pPixelsSource += rowBytes;
		}
	}
	else
	{
		std::memcpy(pDest, pSource, rowBytes * rows);
	}
}

//____ _findCompressor() ______________________________________________________

Compressor * SurfaceReader::_findCompressor( uint32_t idToken )
{
	for( auto& p : m_compressors )
		if( p->idToken() == idToken )
			return p;

	if( m_bAutoCompressors )
	{
		if( idToken == Q565Compressor::ID_TOKEN )
		{
			auto pCompressor = Q565Compressor::create( WGBP(Q565Compressor, _.decompressOnly = true ) );
			m_compressors.push_back(pCompressor);
			return pCompressor;
		}

		if( idToken == LZCompressor::ID_TOKEN )
		{
			auto pCompressor = LZCompressor::create( WGBP(LZCompressor, _.decompressOnly = true ) );
			m_compressors.push_back(pCompressor);
			return pCompressor;
		}
	}

	return nullptr;
}




} // namespace wg



