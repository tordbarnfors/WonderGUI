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
#include <wg_gfxstream.h>
#include <wg_gfxbase.h>

namespace wg
{
const char * toString(GfxStream::ChunkId i)
{
	static const char * names[] = {
		"OutOfData",
		"ProtocolVersion",
		"CanvasList",
		"Tick",
		"BeginRender",
		"EndRender",
		"BeginSession",
		"EndSession",
		"SetCanvas",
		"Objects",
		"Rects",
		"Colors",
		"Transforms",
		"Commands",
		"UpdateRects",
		"CreateSurface",
		"SurfaceUpdate",
		"SurfacePixels",
		"FillSurface",
		"CopySurface",
		"DeleteSurface",
		"CreateEdgemap",
		"SetEdgemapRenderSegments",
		"SetEdgemapColors",
		"EdgemapUpdate",
		"EdgemapSamples",
		"DeleteEdgemap",
		"SurfaceUpdate2",
		"Fence"
	};

	return names[(int)i];
}

//____ readDataInfo() _______________________________________________________

GfxStream::DataInfo GfxStream::readDataInfo(const uint8_t* _pChunkData)
{
	DataInfo info;

	const uint16_t* pChunkData = (const uint16_t*) _pChunkData;

	if( (pChunkData[4] & 0xFF) < 4 )			// Old style DataInfo.
	{
		int totalSize = pChunkData[0] + int(pChunkData[1]) * 65536;
		int chunkOfs = pChunkData[2] + int(pChunkData[3]) * 65536;
		int compression = pChunkData[4] & 0xFF;
		int flags = pChunkData[4] >> 8;

		info.bufferSize = totalSize;
		info.objectId = 0;			// Will this work????

		info.compression = Util::makeEndianSpecificToken('N','O','N','E');
		info.dataStart = 0;
		info.chunkOffset = chunkOfs;
		info.chunkComp = compression;

		info.bFirstChunk = flags & 0x1;
		info.bLastChunk = (flags >> 1) & 0x1;
		info.bPadded = (flags >> 2) & 0x1;

		info.encodedSize = 10;
	}
	else
	{
		info.bufferSize		= pChunkData[0] + int(pChunkData[1]) * 65536;
		info.chunkOffset	= pChunkData[2] + int(pChunkData[3]) * 65536;
		info.compression	= pChunkData[4] + int(pChunkData[5]) * 65536;
		info.dataStart		= pChunkData[6] + int(pChunkData[7]) * 65536;
		info.objectId		= pChunkData[8];

		uint16_t flags = pChunkData[9];

		info.bFirstChunk = flags & 0x1;
		info.bLastChunk = (flags >> 1) & 0x1;
		info.bPadded = (flags >> 2) & 0x1;

		info.chunkComp = 0;
		info.encodedSize = 20;
	}

	return info;
};

//____ loadDecompressData() ________________________________________________

int GfxStream::loadDecompressData( DataInfo& dataInfo, uint8_t * pBuffer, const uint8_t * pSource, int bytes )
{

	bytes -= dataInfo.bPadded;

	auto wp = (spx*)(pBuffer + dataInfo.chunkOffset);

	switch( dataInfo.chunkComp )
	{
		case 0:
			std::memcpy(wp, pSource, bytes);
			break;
		case 1:
		{
			auto rp = (uint8_t*)pSource;
			for (int i = 0; i < bytes; i++)
				*wp++ = ((spx)(*rp++)) << 6;
			break;
		}
		case 2:
		{
			auto rp = (uint16_t*)pSource;
			for (int i = 0; i < bytes / 2; i++)
				*wp++ = (spx)(*rp++);
			break;
		}
		case 3:
		{
			auto rp = (uint16_t*)pSource;
			for (int i = 0; i < bytes / 2; i++)
				*wp++ = ((spx)(*rp++)) << 6;
			break;
		}
	}

	if( dataInfo.bLastChunk )
	{
		int size;

		if(dataInfo.compression != Util::makeEndianSpecificToken('N','O','N','E') )
		{
			auto pCompressor = GfxBase::getDecompressor(dataInfo.compression);

			if( pCompressor )
				return pCompressor->decompress(pBuffer, pBuffer + dataInfo.dataStart, pBuffer + dataInfo.bufferSize);
			else
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Don't know how to decompress data in chunk, add matching compressor to Base.", nullptr, nullptr, __func__, __FILE__, __LINE__ );
				return -1;
			}
		}
		else
		{
			return dataInfo.bufferSize;
		}
	}

	return 0;
}


}
