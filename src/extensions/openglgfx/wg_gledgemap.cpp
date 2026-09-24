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
#include <wg_gledgemap.h>
#include <wg_gfxbase.h>

#include <cstring>

namespace wg
{

	const TypeInfo GlEdgemap::TYPEINFO = { "GlEdgemap", &Edgemap::TYPEINFO };


	//____ create() ______________________________________________________________

	GlEdgemap_p	GlEdgemap::create(const Blueprint& blueprint)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		return GlEdgemap_p(new GlEdgemap(blueprint));
	}

	GlEdgemap_p GlEdgemap::create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = GlEdgemap_p(new GlEdgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	GlEdgemap_p GlEdgemap::create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = GlEdgemap_p(new GlEdgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	GlEdgemap::GlEdgemap(const Blueprint& bp) : Edgemap(bp)
	{
		// Create OpenGL buffer

		int samplesSize = bp.size.w * (bp.segments - 1) * 4 * sizeof(GLfloat);

		m_paletteOfs = samplesSize;

		glGenBuffers(1, &m_bufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_BUFFER, m_textureId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufferId);

		// Buffer is allocated when colors are uploaded, since tints decide its size.

		_colorsUpdated(0, m_nbSegments);
	}

	//____ destructor ____________________________________________________________

	GlEdgemap::~GlEdgemap()
	{
		glDeleteTextures( 1, &m_textureId );
		glDeleteBuffers(1, &m_bufferId);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& GlEdgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _samplesUpdated() ___________________________________________________

	void GlEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		int columnBegin = sampleBegin == 0 ? 0 : sampleBegin - 1;
		int columnEnd = sampleEnd > m_size.w ? m_size.w : sampleEnd;

		int nPixelColumns = columnEnd - columnBegin;

		int edgeStripPitch = m_nbSegments - 1;

		const spx* pEdges = m_pSamples + edgeStripPitch * columnBegin;

		int outBytes = nPixelColumns * (m_nbSegments - 1) * 4 * sizeof(GLfloat);

		auto pBuffer = (GLfloat*)GfxBase::memStackAlloc(outBytes);

		auto pOut = pBuffer;

		for (int i = 0; i < nPixelColumns; i++)
		{
			for (int j = 0; j < m_nbSegments - 1; j++)
			{
				int edgeIn = pEdges[j];
				int edgeOut = pEdges[edgeStripPitch + j];

				if (edgeIn > edgeOut)
					std::swap(edgeIn, edgeOut);

				float increment = edgeOut == edgeIn ? 100.f : 64.f / (edgeOut - edgeIn);
				float beginAdder;
				float endAdder;

				if ((edgeOut & 0xFFFFFFC0) == (edgeIn & 0xFFFFFFC0))
				{
					float firstPixelCoverage = ((64 - (edgeOut & 0x3F)) + (edgeOut - edgeIn) / 2) / 64.f;

					beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
					endAdder = beginAdder;
				}
				else
				{
					int height = 64 - (edgeIn & 0x3F);
					int width = (int)(increment * height);
					float firstPixelCoverage = (height * width) / (2 * 4096.f);
					float lastPixelCoverage = 1.f - (edgeOut & 0x3F) * increment * (edgeOut & 0x3F) / (2 * 4096.f);

					beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
					endAdder = lastPixelCoverage - (1.f - (edgeOut & 0x3F) * increment / 64.f);
				}

				*pOut++ = edgeIn / 64.f;				// Segment begin pixel
				*pOut++ = increment;					// Segment increment
				*pOut++ = beginAdder;					// Segment begin adder
				*pOut++ = endAdder;						// Segment end adder
			}

			pEdges += edgeStripPitch;

		}

		// Upload buffer

		int ofs = columnBegin * edgeStripPitch * 4 * sizeof(GLfloat);

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferSubData(GL_TEXTURE_BUFFER, ofs, outBytes, pBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		// Release temp buffer

		GfxBase::memStackFree(outBytes);
	}

	//____ _colorsUpdated() ____________________________________________________
	/*
		Rebuilds and uploads flat colors, tint table and tint blocks for all segments,
		since tint blocks can change size. Grows the buffer if needed.
	*/

	void GlEdgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		RectSPX rect(0, 0, m_size.w * 64, m_size.h * 64);

		TintTools::DecodedTint	decoded[maxSegments];
		int						blockOfs[maxSegments];

		int nEntries = m_nbSegments * 2;					// Flat colors and tint table.

		for (int seg = 0; seg < m_nbSegments; seg++)
		{
			if (m_pTints[seg])
			{
				uint16_t	words[TintTools::c_maxEncodedTintWords];
				HiColor		colors[TintTools::c_maxEncodedTintColors];
				int			nColors;

				TintTools::encodeTint(m_pTints[seg], rect, words, colors, nColors);
				const HiColor* pColors = colors;
				TintTools::decodeTint(words, pColors, decoded[seg]);

				blockOfs[seg] = m_paletteOfs / 16 + nEntries;
				nEntries += TintTools::gpuTintBlockSize(decoded[seg]);
			}
			else
				blockOfs[seg] = -1;
		}

		int paletteBytes = nEntries * 4 * sizeof(GLfloat);

		auto pBuffer = (GLfloat*)GfxBase::memStackAlloc(paletteBytes);
		auto pOut = pBuffer;

		for (int seg = 0; seg < m_nbSegments; seg++)
		{
			const HiColor& col = m_pFlatColors[seg];
			*pOut++ = col.r / 4096.f;
			*pOut++ = col.g / 4096.f;
			*pOut++ = col.b / 4096.f;
			*pOut++ = col.a / 4096.f;
		}

		for (int seg = 0; seg < m_nbSegments; seg++)
		{
			*pOut++ = float(blockOfs[seg]);
			*pOut++ = 0.f;
			*pOut++ = 0.f;
			*pOut++ = 0.f;
		}

		for (int seg = 0; seg < m_nbSegments; seg++)
		{
			if (blockOfs[seg] >= 0)
			{
				TintTools::writeGpuTintBlock(decoded[seg], pOut);
				pOut += TintTools::gpuTintBlockSize(decoded[seg]) * 4;
			}
		}

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);

		bool bReallocated = false;
		if (m_paletteOfs + paletteBytes > m_bufferSize)
		{
			m_bufferSize = m_paletteOfs + paletteBytes;
			glBufferData(GL_TEXTURE_BUFFER, m_bufferSize, nullptr, GL_DYNAMIC_DRAW);
			bReallocated = true;
		}

		glBufferSubData(GL_TEXTURE_BUFFER, m_paletteOfs, paletteBytes, pBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		GfxBase::memStackFree(paletteBytes);

		// A reallocated buffer has lost its samples.

		if (bReallocated && m_nbSegments > 1)
			_samplesUpdated(0, m_nbSegments - 1, 0, m_size.w + 1);
	}

} // namespace wg

