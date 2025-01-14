/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_streamedgemap.h>
#include <wg_streambackend.h>
#include <wg_gfxbase.h>
#include <wg_gradyent.h>
#include <wg_compress.h>

#include <cstring>

namespace wg
{

const TypeInfo StreamEdgemap::TYPEINFO = { "StreamEdgemap", &Edgemap::TYPEINFO };


//____ create() ______________________________________________________________

StreamEdgemap_p	StreamEdgemap::create(StreamEncoder* pEncoder, const Blueprint& blueprint)
{
	if (!pEncoder)
		return nullptr;

	if (!_validateBlueprint(blueprint))
		return nullptr;

	auto p = StreamEdgemap_p(new StreamEdgemap(pEncoder, blueprint));
	pEncoder->flush();
	return p;
}

StreamEdgemap_p StreamEdgemap::create(StreamEncoder* pEncoder, const Edgemap::Blueprint& blueprint, SampleOrigo origo,
	const float* pSamples, int edges, int edgePitch, int samplePitch)
{
	if (!pEncoder)
		return nullptr;

	if (!_validateBlueprint(blueprint))
		return nullptr;

	if (edges >= blueprint.segments)
		return nullptr;

	auto p = StreamEdgemap_p(new StreamEdgemap(pEncoder, blueprint));

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges + 1;
	pEncoder->flush();
	return p;
}

StreamEdgemap_p StreamEdgemap::create(StreamEncoder* pEncoder, const Edgemap::Blueprint& blueprint, SampleOrigo origo,
	const spx* pSamples, int edges, int edgePitch, int samplePitch)
{
	if (!pEncoder)
		return nullptr;

	if (!_validateBlueprint(blueprint))
		return nullptr;

	if (edges >= blueprint.segments)
		return nullptr;

	auto p = StreamEdgemap_p(new StreamEdgemap(pEncoder, blueprint));

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges + 1;
	pEncoder->flush();
	return p;
}

//____ constructor ___________________________________________________________

StreamEdgemap::StreamEdgemap(StreamEncoder* pEncoder, const Blueprint& bp) : Edgemap(bp)
{
	m_pEncoder = pEncoder;
	m_inStreamId = pEncoder->allocObjectId();

	_sendCreateEdgemap(m_pEncoder);
}

//____ destructor ____________________________________________________________

StreamEdgemap::~StreamEdgemap()
{
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& StreamEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ setRenderSegments() _____________________________________________________

bool StreamEdgemap::setRenderSegments(int nSegments)
{
	if( !Edgemap::setRenderSegments(nSegments) )
		return false;

	auto& encoder = * m_pEncoder;
		
	encoder << GfxStream::Header{ GfxStream::ChunkId::SetEdgemapRenderSegments, 0, 4 };
	encoder << m_inStreamId;
	encoder << (uint16_t) nSegments;
		
	return true;
}


//____ _samplesUpdated() _______________________________________________________

void StreamEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
{
	{
		// Stream header

		int beginBlockSize = 2 + 1 + 1 + 2 + 2;

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::EdgemapUpdate, 0, beginBlockSize };
		encoder << m_inStreamId;
		encoder << (uint8_t)edgeBegin;
		encoder << (uint8_t)edgeEnd;
		encoder << (uint16_t)sampleBegin;
		encoder << (uint16_t)sampleEnd;


		int nbEdges = m_nbSegments - 1;
		int copyEdges = edgeEnd - edgeBegin;
		int skipEdges = nbEdges - copyEdges;


		// Copy all samples to one linear buffer

		int samples = (edgeEnd - edgeBegin) * (sampleEnd - sampleBegin);
		int bufferBytes = sizeof(spx) * samples;

		spx * pBuffer = (spx*) GfxBase::memStackAlloc(bufferBytes);

		spx* pDest = pBuffer;
		spx* pSource = m_pSamples + sampleBegin * nbEdges + edgeBegin;

		for (int sample = sampleBegin; sample < sampleEnd; sample++)
		{
			for (int i = 0; i < copyEdges; i++)
				*pDest++ = *pSource++;

			pSource += skipEdges;
		}

		// Compress samples

		uint8_t * pCompressed = (uint8_t*) GfxBase::memStackAlloc(bufferBytes);

		Compression compression;
		int			size;

		std::tie(compression,size) = compressSpx( pBuffer, samples, pCompressed);

		// Stream data

		if(compression == Compression::None)
			StreamBackend::_splitAndEncode(m_pEncoder, GfxStream::ChunkId::EdgemapSamples, compression, pBuffer, ((uint8_t*)pBuffer) + bufferBytes, 4);
		else
			StreamBackend::_splitAndEncode(m_pEncoder, GfxStream::ChunkId::EdgemapSamples, compression, pCompressed, pCompressed + size, 1);

		// Release temp buffers

		GfxBase::memStackFree(bufferBytes);	// pCompressed
		GfxBase::memStackFree(bufferBytes);	// pBuffer
	}
}

//____ _colorsUpdated() ________________________________________________________

void StreamEdgemap::_colorsUpdated(int beginColor, int endColor)
{
	auto& encoder = * m_pEncoder;
	
	int maxColorsPerBlock = (GfxStream::c_maxBlockSize - 10) / sizeof(HiColor);
	
	while( beginColor < endColor )
	{
		int nColorsInBlock = std::min(maxColorsPerBlock, endColor - beginColor );

		int blockSize = 10 + nColorsInBlock * sizeof(HiColor);
		
		encoder << GfxStream::Header{ GfxStream::ChunkId::SetEdgemapColors, 0, blockSize };
		encoder << m_inStreamId;
		encoder << beginColor;
		encoder << beginColor + nColorsInBlock;

		encoder << GfxStream::WriteBytes{ int(nColorsInBlock*sizeof(HiColor)), m_pPalette + beginColor };
		
		beginColor += nColorsInBlock;
	}
}

//____ _sendCreateEdgemap() ___________________________________________________

void StreamEdgemap::_sendCreateEdgemap( StreamEncoder* pEncoder )
{
	int blockSize = 14;
	
	auto& encoder = * m_pEncoder;
	
	encoder << GfxStream::Header{ GfxStream::ChunkId::CreateEdgemap, 0, blockSize };
	encoder << m_inStreamId;
	encoder << m_size;
	encoder << (uint16_t) m_nbSegments;
	encoder << (uint16_t) m_paletteType;
	
	_colorsUpdated(0, m_paletteSize);
}

//____ _findBestPackFormat() _________________________________________________

GfxStream::SpxFormat StreamEdgemap::_findBestPackFormat( int spxMask )
{
	if( (spxMask & 0xFFDFC03F) == 0 )
		return GfxStream::SpxFormat::Uint8_int;						// Fits in uint8_t without binals.
	
	if( (spxMask & 0xFFDF0000) == 0 )
		return GfxStream::SpxFormat::Uint16_dec;					// Fits in uint16_t with binals.

	if( (spxMask & 0xFFE0003F) == 0 )
		return GfxStream::SpxFormat::Int16_int;						// Fits in int16_t without binals.

	return GfxStream::SpxFormat::Int32_dec;							// We need 32 bits.
}

} // namespace wg

