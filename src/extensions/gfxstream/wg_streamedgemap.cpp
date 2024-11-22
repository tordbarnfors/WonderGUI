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
#include <wg_gfxbase.h>
#include <wg_gradyent.h>

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
		
	encoder << GfxStream::Header{ GfxStream::ChunkId::SetEdgemapRenderSegments, GfxStream::SpxFormat::Int32_dec, 4 };
	encoder << m_inStreamId;
	encoder << (uint16_t) nSegments;
		
	return true;
}


//____ _samplesUpdated() _______________________________________________________

void StreamEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
{
	{
		// Send header

		int beginBlockSize = 2 + 1 + 1 + 2 + 2;

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::BeginEdgemapUpdate, GfxStream::SpxFormat::Int32_dec, beginBlockSize };
		encoder << m_inStreamId;
		encoder << (uint8_t)edgeBegin;
		encoder << (uint8_t)edgeEnd;
		encoder << (uint16_t)sampleBegin;
		encoder << (uint16_t)sampleEnd;

		int nbEdges = m_nbSegments - 1;
		int copyEdges = edgeEnd - edgeBegin;
		int skipEdges = nbEdges - copyEdges;

		// Check sample max/min and max/min diff against old samples
		// and find best spxFormat for transfer.

		int spxMask = 0;

		{
			const int add = (1 << 21);
			spx* pNew = m_pSamples + sampleBegin * nbEdges + edgeBegin;

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				for (int i = 0; i < copyEdges; i++)
				{
					int value = *pNew++;
					spxMask |= value + add;
				}
				pNew += skipEdges;
			}
		}

		GfxStream::SpxFormat spxFormat = _findBestPackFormat(spxMask);

		// Compress bytes for transfer

		int bufferBytes = GfxStream::spxSize(spxFormat) * (edgeEnd - edgeBegin) * (sampleEnd - sampleBegin);

		int8_t* pBuffer = (int8_t*)GfxBase::memStackAlloc(bufferBytes);


		switch (spxFormat)
		{
		case GfxStream::SpxFormat::Int32_dec:
		{
			// No compression needed, just replace old

			spx* pPacked = reinterpret_cast<spx*>(pBuffer);	
			spx* pNew = m_pSamples + sampleBegin * nbEdges + edgeBegin;

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				for (int i = 0; i < copyEdges; i++)
					*pPacked++ = *pNew++;

				pNew += skipEdges;
			}

			break;
		}

		case GfxStream::SpxFormat::Uint16_dec:
		{
			uint16_t* pPacked = reinterpret_cast<uint16_t*>(pBuffer);
			spx* pNew = m_pSamples + sampleBegin * nbEdges + edgeBegin;

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				for (int i = 0; i < copyEdges; i++)
					*pPacked++ = (uint16_t)*pNew++;

				pNew += skipEdges;
			}
			break;
		}

		case GfxStream::SpxFormat::Int16_int:
		{
			int16_t* pPacked = reinterpret_cast<int16_t*>(pBuffer);
			spx* pNew = m_pSamples + sampleBegin * nbEdges + edgeBegin;

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				for (int i = 0; i < copyEdges; i++)
					*pPacked++ = (int16_t)((*pNew++) >> 6);

				pNew += skipEdges;
			}
			break;
		}

		case GfxStream::SpxFormat::Uint8_int:
		{
			uint8_t* pPacked = reinterpret_cast<uint8_t*>(pBuffer);
			spx* pNew = m_pSamples + sampleBegin * nbEdges + edgeBegin;

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				for (int i = 0; i < copyEdges; i++)
					*pPacked++ = (uint8_t)((*pNew++) >> 6);

				pNew += skipEdges;
			}
			break;
		}
		}

		// Stream data

		const int8_t* pSrc = pBuffer;

		int dataSize = bufferBytes;

		int maxDataSize = ((int)(GfxStream::c_maxBlockSize - sizeof(GfxStream::Header))) & 0xFFFC;		// We can not have fractional spx in buffer.

		while (dataSize > 0)
		{
			uint16_t chunkSize = std::min(dataSize, maxDataSize);
			dataSize -= chunkSize;

			encoder << GfxStream::Header{ GfxStream::ChunkId::EdgemapSamples, spxFormat, uint16_t((chunkSize + 1) & 0xFFFE) };

			encoder << GfxStream::WriteBytes{ chunkSize, (void*)pSrc };
			pSrc += chunkSize;
		}

		encoder.align();

		// Stream footer

		encoder << GfxStream::Header{ GfxStream::ChunkId::EndEdgemapUpdate, GfxStream::SpxFormat::Int32_dec, 0 };

		// Release temp buffer

		GfxBase::memStackFree(bufferBytes);
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
		
		encoder << GfxStream::Header{ GfxStream::ChunkId::SetEdgemapColors, GfxStream::SpxFormat::Int32_dec, blockSize };
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
	
	encoder << GfxStream::Header{ GfxStream::ChunkId::CreateEdgemap, GfxStream::SpxFormat::Int32_dec, blockSize };
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

