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
#include <wax_bitmapreader.h>
#include <wax_base.h>
#include <wax_util.h>
#include <wax_pixeltools.h>

#include <wax_compression.h>
#include <wax_lzcompression.h>
#include <wax_q565compression.h>
#include <wax_qoicompression.h>
#include <wax_rlecompression.h>

#include <cstring>

namespace wax
{
	const TypeInfo BitmapReader::TYPEINFO = { "BitmapReader", &Object::TYPEINFO };


	//.____ create() _____________________________________________________________

	BitmapReader_p BitmapReader::create( const Blueprint& blueprint )
	{
		return BitmapReader_p( new BitmapReader(blueprint) );
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BitmapReader::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ readBitmapFromStream() _____________________________________________

	Bitmap_p BitmapReader::readBitmapFromStream(std::istream& stream)
	{
		Bitmap::Blueprint dummy;
		return readBitmapFromStream(stream, dummy);
	}

	Bitmap_p BitmapReader::readBitmapFromStream(std::istream& stream, const Bitmap::Blueprint& _bp)
	{

		// Determine file format

		const char id_QOI[4] = { 'q','o','i','f' };
		const char id_Bitmap[4] = { 'S','U','R','F' };

		char identifier[4];
		stream.read(identifier, 4);

		if (*(uint32_t*)id_Bitmap == *(uint32_t*)identifier)
			return _readBitmapFromStream(stream, _bp);
		else if (*(uint32_t*)id_QOI == *(uint32_t*)identifier)
			return _readQOIFromStream(stream, _bp);

		Base::reportError(ErrorLevel::Error, ErrorCode::Other, "Stream is not a supported image format.", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
		return nullptr;
	}

	//____ _readBitmapFromStream() ______________________________________________

	Bitmap_p BitmapReader::_readBitmapFromStream(std::istream & stream, const Bitmap::Blueprint & _bp)
	{
		BitmapFileHeader	header;

		// Read the header size

		stream.read(((char*)(&header)) + 4, 4);

		// Read the rest of the header

		stream.read( ((char*)(&header))+8, header.headerBytes - 8);

		// Read palette and pixels

		int dataBytes = header.paletteBytes + header.pixelBytes;

		char * pData = Base::memStackAlloc(dataBytes);
		stream.read( pData, dataBytes );

		auto pBitmap = _createBitmap(header, pData, _bp);

		Base::memStackFree(dataBytes);
		return pBitmap;
	}

	//____ _readQOIFromStream() ______________________________________________

	Bitmap_p BitmapReader::_readQOIFromStream(std::istream& stream, const Bitmap::Blueprint& _bp)
	{
		// Read rest of header, the magic has already been read.

		uint8_t headerBytes[c_qoiHeaderSize];
		std::memcpy(headerBytes, "qoif", 4);
		stream.read((char*) headerBytes + 4, c_qoiHeaderSize - 4);

		if( !stream.good() )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "QOI image ended before end of header.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		QOIHeader header;
		if( !_parseQOIHeader(headerBytes, header) )
			return nullptr;

		// The size of the data isn't stored anywhere, so we read it chunk by chunk
		// until all pixels have been covered, so we read the image and nothing more.

		std::vector<uint8_t>	data;
		std::streambuf *		pBuffer = stream.rdbuf();
		int64_t					pixelsLeft = int64_t(header.width) * header.height;

		while( pixelsLeft > 0 )
		{
			int tag = pBuffer->sbumpc();
			if( tag == std::char_traits<char>::eof() )
				break;

			data.push_back(uint8_t(tag));

			int extraBytes = 0;
			if( tag == 0xFE )
				extraBytes = 3;						// RGB
			else if( tag == 0xFF )
				extraBytes = 4;						// RGBA
			else if( (tag & 0xC0) == 0x80 )
				extraBytes = 1;						// Luma
			else if( (tag & 0xC0) == 0xC0 )
				pixelsLeft -= (tag & 0x3F);			// Run, one pixel more is subtracted below.

			pixelsLeft--;

			if( extraBytes > 0 )
			{
				size_t ofs = data.size();
				data.resize(ofs + extraBytes);
				if( pBuffer->sgetn((char*) &data[ofs], extraBytes) != extraBytes )
					break;
			}
		}

		// Read the end marker, which ends the stream.

		size_t ofs = data.size();
		data.resize(ofs + 8);

		if( pixelsLeft > 0 || pBuffer->sgetn((char*) &data[ofs], 8) != 8 )
		{
			stream.setstate(std::ios::eofbit | std::ios::failbit);
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "QOI image ended before all pixels were read.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return _createBitmapFromQOI(header, data.data(), data.data() + data.size(), _bp);
	}

	//____ _readQOIFromMemory() ______________________________________________

	Bitmap_p BitmapReader::_readQOIFromMemory(const char* pData, const char* pDataEnd, const Bitmap::Blueprint& _bp)
	{
		if( pDataEnd && pDataEnd - pData < c_qoiHeaderSize )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "QOI image is too small to contain a header.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		QOIHeader header;
		if( !_parseQOIHeader((const uint8_t*) pData, header) )
			return nullptr;

		auto pChunks = (const uint8_t*) pData + c_qoiHeaderSize;

		// If we don't know where the data ends, we let it end where it at most can.
		// Decompression stops at the end marker anyway.

		auto pChunksEnd = pDataEnd ? (const uint8_t*) pDataEnd : pChunks + int64_t(header.width) * header.height * 5 + 8;

		return _createBitmapFromQOI(header, pChunks, pChunksEnd, _bp);
	}

	//____ _parseQOIHeader() ___________________________________________________

	bool BitmapReader::_parseQOIHeader(const uint8_t* p, QOIHeader& header)
	{
		auto read32 = [](const uint8_t * p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]); };

		uint32_t width = read32(p + 4);
		uint32_t height = read32(p + 8);

		header.channels = p[12];
		header.colorspace = p[13];

		if( std::memcmp(p, "qoif", 4) != 0 || width == 0 || height == 0 || height >= c_qoiMaxPixels / width ||
		   (header.channels != 3 && header.channels != 4) || header.colorspace > 1 )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "Invalid or unsupported QOI header.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		header.width = int(width);
		header.height = int(height);
		return true;
	}

	//____ _createBitmapFromQOI() _____________________________________________

	Bitmap_p BitmapReader::_createBitmapFromQOI(const QOIHeader& header, const uint8_t * pChunks, const uint8_t * pChunksEnd, const Bitmap::Blueprint& _bp)
	{
		// Prepare bitmap blueprint

		auto bp = WAXBP(Bitmap,
						_.size			= { header.width, header.height },
						_.format		= header.channels == 4 ? PixelFormat::ARGB_8 : PixelFormat::XRGB_8,
						_.colorSpace	= header.colorspace == 1 ? ColorSpace::Linear : ColorSpace::sRGB );

		if (_addFlagsFromOtherBlueprint(bp, _bp) != 0)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided blueprint can not alter size, format or palette of loaded bitmap but have one or more of these parameters set.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Create bitmap

		auto pBitmap = m_pFactory ? m_pFactory->createBitmap(bp) : Base::defaultBitmapFactory()->createBitmap(bp);
		if( !pBitmap )
			return nullptr;

		auto pixbuf = pBitmap->allocPixelBuffer();

		int nPixels = header.width * header.height;
		int lineBytes = header.width * 4;

		// Decompress straight into the pixel buffer if it has the pixels as we want them,
		// otherwise to little endian ARGB_8/XRGB_8 that we convert from.

		bool bSameLayout = pixbuf.format == bp.format && pixbuf.colorSpace == bp.colorSpace && pixbuf.pitch == lineBytes;

		auto pDecompressor = QOIDecompressor::create( WAXBP(QOIDecompressor, _.bigEndian = bSameLayout ? pixbuf.bigEndian : false ) );

		uint8_t *	pDecompressed = bSameLayout ? pixbuf.pixels : (uint8_t*) Base::memStackAlloc(nPixels * 4);

		int bytes = pDecompressor->decompress(pDecompressed, nPixels * 4, pChunks, pChunksEnd);

		bool bOk = true;

		if( bytes != nPixels * 4 )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "QOI data ended before all pixels were decoded.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			bOk = false;
		}
		else if( !bSameLayout )
		{
			auto dstDesc = Util::pixelFormatToDescription(pixbuf.format, pixbuf.bigEndian);
			int paletteEntries = pBitmap->paletteSize();

			bOk = PixelTools::copyPixels(header.width, header.height, pDecompressed, bp.format, bp.colorSpace, false, 0, nullptr, 0,
										 pixbuf.pixels, pixbuf.format, pixbuf.colorSpace, pixbuf.bigEndian,
										 pixbuf.pitch - PixelTools::bytesPerLine(dstDesc, header.width),
										 const_cast<Color8*>(pixbuf.palette), paletteEntries, pBitmap->paletteCapacity());

			if( !bOk )
				Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to convert pixels of QOI image to bitmap.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		if( !bSameLayout )
			Base::memStackFree(nPixels * 4);

		if( !bOk )
		{
			pBitmap->freePixelBuffer(pixbuf);
			return nullptr;
		}

		pBitmap->pullPixels(pixbuf);
		pBitmap->freePixelBuffer(pixbuf);
		return pBitmap;
	}

	//____ readBitmapFromBlob() _______________________________________________

	Bitmap_p BitmapReader::readBitmapFromBlob(const Blob* pBlob)
	{
		Bitmap::Blueprint dummy;
		return readBitmapFromBlob(pBlob, dummy);
	}

	Bitmap_p BitmapReader::readBitmapFromBlob( const Blob * pBlob, const Bitmap::Blueprint& bp )
	{
		auto pData = static_cast<const char*>(pBlob->data());

		if( pBlob->size() >= 4 && std::memcmp(pData, "qoif", 4) == 0 )
			return _readQOIFromMemory(pData, pData + pBlob->size(), bp);

		return readBitmapFromMemory( pData, bp);
	}

	//____ readBitmapFromMemory() _____________________________________________

	Bitmap_p BitmapReader::readBitmapFromMemory(const char* pData)
	{
		Bitmap::Blueprint dummy;
		return readBitmapFromMemory(pData, dummy);
	}

	Bitmap_p BitmapReader::readBitmapFromMemory(const char* pData, const Bitmap::Blueprint& _bp)
	{
		if( std::memcmp(pData, "qoif", 4) == 0 )
			return _readQOIFromMemory(pData, nullptr, _bp);

		BitmapFileHeader	header;

		// Read the header

		int headerSize = * (const int16_t*)&pData[6];
		std::memcpy( &header, pData, headerSize);

		return _createBitmap(header, pData + headerSize, _bp);
	}

	//____ _createBitmap() ____________________________________________________

	Bitmap_p BitmapReader::_createBitmap(const BitmapFileHeader& header, const char * pData, const Bitmap::Blueprint& _bp)
	{
		if( header.versionNumber < 1 || header.versionNumber > 2 )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "Unsupported version of bitmap file.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		BitmapFileLayout layout = bitmapFileLayout(header);

		if( layout.format == PixelFormat::Undefined )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Other, "Pixel format of bitmap file is not supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Prepare bitmap blueprint

		Bitmap::Blueprint bp = _blueprintFromHeader(&header, layout);
		if (_addFlagsFromOtherBlueprint(bp, _bp) != 0)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided blueprint can not alter size, format or palette of loaded bitmap but have one or more of these parameters set.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Prepare palette, decompressing it if needed.

		const Color8 *		pPalette = nullptr;
		std::vector<Color8>	decompressedPalette;

		if( header.paletteSize > 0 )
		{
			pPalette = (const Color8*) pData;

			if (header.paletteCompression != Util::makeEndianSpecificToken( 'N','O','N','E' ))
			{
				Decompressor * pDecompressor = Base::getDecompressor(header.paletteCompression);

				if( !pDecompressor )
				{
					char msg[] = "Don't know how to decompress 'XXXX'.";
					* (uint32_t*)&msg[30] = header.paletteCompression;

					Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, msg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					return nullptr;
				}

				decompressedPalette.resize( header.paletteSize + (header.paletteDecompressMargin + 3) / 4 );
				pDecompressor->decompress(decompressedPalette.data(), pData, pData + header.paletteBytes - header.paletteDataPadding);
				pPalette = decompressedPalette.data();
			}

			bp.palette = pPalette;
		}

		const char * pPixelData = pData + header.paletteBytes;

		// Create bitmap

		auto pBitmap = m_pFactory ? m_pFactory->createBitmap(bp) : Base::defaultBitmapFactory()->createBitmap(bp);
		if( !pBitmap )
			return nullptr;

		auto pixbuf = pBitmap->allocPixelBuffer();

		int fileLineBytes = PixelTools::bytesPerLine(layout.description, header.width);

		// Pixels can be copied straight in if the bitmap keeps them the same way as the file.

		bool bSameLayout = pixbuf.format == layout.format && pixbuf.colorSpace == layout.colorSpace &&
							pixbuf.bigEndian == layout.bigEndian && layout.description.bits == pBitmap->pixelBits();

		// Decompress if needed.

		const char *	pPixels = pPixelData;
		int				decompressedBytes = 0;

		if (header.pixelCompression != Util::makeEndianSpecificToken( 'N','O','N','E' ))
		{
			Decompressor * pDecompressor = Base::getDecompressor(header.pixelCompression);

			if( !pDecompressor )
			{
				char msg[] = "Don't know how to decompress 'XXXX'.";
				* (uint32_t*)&msg[30] = header.pixelCompression;

				Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, msg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				pBitmap->freePixelBuffer(pixbuf);
				return nullptr;
			}

			const uint8_t * pBegin = (const uint8_t*) pPixelData;
			const uint8_t * pEnd = pBegin + header.pixelBytes - header.pixelDataPadding;

			if( bSameLayout && pixbuf.pitch == fileLineBytes )
			{
				pDecompressor->decompress(pixbuf.pixels, pBegin, pEnd);

				pBitmap->pullPixels(pixbuf);
				pBitmap->freePixelBuffer(pixbuf);
				return pBitmap;
			}

			decompressedBytes = fileLineBytes * header.height + header.pixelDecompressMargin;
			char * pDecompressed = Base::memStackAlloc(decompressedBytes);
			pDecompressor->decompress(pDecompressed, pBegin, pEnd);
			pPixels = pDecompressed;
		}

		// Copy or convert pixels into the bitmap.

		bool bOk = true;

		if( bSameLayout )
			_copyUncompressedFromMemory(pixbuf.pixels, pPixels, fileLineBytes, pixbuf.pitch, header.height);
		else
		{
			auto dstDesc = Util::pixelFormatToDescription(pixbuf.format, pixbuf.bigEndian);
			int paletteEntries = pBitmap->paletteSize();

			bOk = PixelTools::copyPixels(header.width, header.height, (const uint8_t*) pPixels, layout.description, layout.colorSpace, 0,
										 pPalette, header.paletteSize, pixbuf.pixels, pixbuf.format, pixbuf.colorSpace, pixbuf.bigEndian,
										 pixbuf.pitch - PixelTools::bytesPerLine(dstDesc, header.width),
										 const_cast<Color8*>(pixbuf.palette), paletteEntries, pBitmap->paletteCapacity());
		}

		if( decompressedBytes > 0 )
			Base::memStackFree(decompressedBytes);

		if( !bOk )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to convert pixels of bitmap file to bitmap.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			pBitmap->freePixelBuffer(pixbuf);
			return nullptr;
		}

		pBitmap->pullPixels(pixbuf);
		pBitmap->freePixelBuffer(pixbuf);

		return pBitmap;
	}


//____ _blueprintFromHeader() _________________________________________________

Bitmap::Blueprint BitmapReader::_blueprintFromHeader( const BitmapFileHeader * pHeader, const BitmapFileLayout& layout )
{
	return WAXBP(Bitmap,
				_.size 			= {pHeader->width, pHeader->height},
				_.scale 		= pHeader->scale,
				_.format 		= layout.format,
				_.colorSpace	= layout.colorSpace,
				_.bigEndian		= layout.bigEndian,
				_.canvas 		= pHeader->canvas,
				_.dynamic 		= pHeader->dynamic,
				_.mipmap 		= pHeader->mipmap,
				_.identity		= pHeader->identity,
				_.tiling		= pHeader->tiling,
				_.sampleMethod 	= pHeader->sampleMethod,
				_.paletteSize	= pHeader->paletteSize );
}

//____ _addFlagsFromOtherBlueprint() __________________________________________

int BitmapReader::_addFlagsFromOtherBlueprint(Bitmap::Blueprint& dest, const Bitmap::Blueprint& extraFlags)
{
	int errorCode = 0;

	if (extraFlags.restorer)
		dest.restorer = extraFlags.restorer;

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

void BitmapReader::_copyUncompressedFromMemory(void* pDest, const void* pSource, int rowBytes, int pitch, int rows)
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


} // namespace wax



