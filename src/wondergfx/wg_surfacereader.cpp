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
#include <wg_pixeltools.h>

#include <wg_compression.h>
#include <wg_lzcompression.h>
#include <wg_q565compression.h>
#include <wg_rlecompression.h>

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

		// Read palette and pixels

		int dataBytes = header.paletteBytes + header.pixelBytes;

		char * pData = GfxBase::memStackAlloc(dataBytes);
		stream.read( pData, dataBytes );

		auto pSurface = _createSurface(header, pData, _bp);

		GfxBase::memStackFree(dataBytes);
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

		return _createSurface(header, pData + headerSize, _bp);
	}

	//____ _createSurface() ____________________________________________________

	Surface_p SurfaceReader::_createSurface(const SurfaceFileHeader& header, const char * pData, const Surface::Blueprint& _bp)
	{
		if( header.versionNumber < 1 || header.versionNumber > 2 )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::Other, "Unsupported version of surface file.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		SurfaceFileLayout layout = surfaceFileLayout(header);

		if( layout.format == PixelFormat::Undefined )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::Other, "Pixel format of surface file is not supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Prepare surface blueprint

		Surface::Blueprint bp = _blueprintFromHeader(&header, layout);
		if (_addFlagsFromOtherBlueprint(bp, _bp) != 0)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided blueprint can not alter size, format or palette of loaded surface but have one or more of these parameters set.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Prepare palette. We only support uncompressed palette for the moment.

		const Color8 * pPalette = nullptr;
		if( header.paletteSize > 0 )
		{
			pPalette = (const Color8*) pData;
			bp.palette = pPalette;
		}

		const char * pPixelData = pData + header.paletteBytes;

		// Create surface

		auto pSurface = m_pFactory->createSurface(bp);
		if( !pSurface )
			return nullptr;

		auto pixbuf = pSurface->allocPixelBuffer();

		int fileLineBytes = PixelTools::bytesPerLine(layout.description, header.width);

		// Pixels can be copied straight in if the surface keeps them the same way as the file.

		bool bSameLayout = pixbuf.format == layout.format && pixbuf.colorSpace == layout.colorSpace &&
							pixbuf.bigEndian == layout.bigEndian && layout.description.bits == pSurface->pixelBits();

		// Decompress if needed.

		const char *	pPixels = pPixelData;
		int				decompressedBytes = 0;

		if (header.pixelCompression != Util::makeEndianSpecificToken( 'N','O','N','E' ))
		{
			Decompressor * pDecompressor = GfxBase::getDecompressor(header.pixelCompression);

			if( !pDecompressor )
			{
				char msg[] = "Don't know how to decompress 'XXXX'.";
				* (uint32_t*)&msg[30] = header.pixelCompression;

				GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, msg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				pSurface->freePixelBuffer(pixbuf);
				return nullptr;
			}

			const uint8_t * pBegin = (const uint8_t*) pPixelData;
			const uint8_t * pEnd = pBegin + header.pixelBytes - header.pixelDataPadding;

			if( bSameLayout && pixbuf.pitch == fileLineBytes )
			{
				pDecompressor->decompress(pixbuf.pixels, pBegin, pEnd);

				pSurface->pullPixels(pixbuf);
				pSurface->freePixelBuffer(pixbuf);
				return pSurface;
			}

			decompressedBytes = fileLineBytes * header.height + header.pixelDecompressMargin;
			char * pDecompressed = GfxBase::memStackAlloc(decompressedBytes);
			pDecompressor->decompress(pDecompressed, pBegin, pEnd);
			pPixels = pDecompressed;
		}

		// Copy or convert pixels into the surface.

		bool bOk = true;

		if( bSameLayout )
			_copyUncompressedFromMemory(pixbuf.pixels, pPixels, fileLineBytes, pixbuf.pitch, header.height);
		else
		{
			auto dstDesc = Util::pixelFormatToDescription(pixbuf.format, pixbuf.bigEndian);
			int paletteEntries = pSurface->paletteSize();

			bOk = PixelTools::copyPixels(header.width, header.height, (const uint8_t*) pPixels, layout.description, layout.colorSpace, 0,
										 pPalette, header.paletteSize, pixbuf.pixels, pixbuf.format, pixbuf.colorSpace, pixbuf.bigEndian,
										 pixbuf.pitch - PixelTools::bytesPerLine(dstDesc, header.width),
										 const_cast<Color8*>(pixbuf.palette), paletteEntries, pSurface->paletteCapacity());
		}

		if( decompressedBytes > 0 )
			GfxBase::memStackFree(decompressedBytes);

		if( !bOk )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to convert pixels of surface file to surface.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			pSurface->freePixelBuffer(pixbuf);
			return nullptr;
		}

		pSurface->pullPixels(pixbuf);
		pSurface->freePixelBuffer(pixbuf);

		return pSurface;
	}


//____ _blueprintFromHeader() _________________________________________________

Surface::Blueprint SurfaceReader::_blueprintFromHeader( const SurfaceFileHeader * pHeader, const SurfaceFileLayout& layout )
{
	return WGBP(Surface,
				_.size 			= {pHeader->width, pHeader->height},
				_.scale 		= pHeader->scale,
				_.format 		= layout.format,
				_.colorSpace	= layout.colorSpace,
				_.bigEndian		= layout.bigEndian,
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

	if (extraFlags.format != PixelFormat::Undefined || extraFlags.colorSpace != ColorSpace::Undefined)
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


} // namespace wg



