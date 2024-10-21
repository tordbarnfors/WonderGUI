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

#include <wg_gledgemap.h>
#include <wg_gradyent.h>
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
		int colorstripsSize = (bp.size.w + bp.size.h) * bp.segments * 4 * sizeof(GLfloat);

		m_colorsOfs = samplesSize;

		int bufferSize = samplesSize + colorstripsSize;

		glGenBuffers(1, &m_bufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);

		glGenTextures(1, &m_textureId);
//		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, m_textureId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufferId);
//		glActiveTexture(GL_TEXTURE0);

//		glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);
		glBufferData(GL_TEXTURE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
//		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		// Convert and upload colorstrips

		_colorsUpdated(0, m_nbSegments);
	}

	//____ destructor ____________________________________________________________

	GlEdgemap::~GlEdgemap()
	{
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& GlEdgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _samplesUpdated() ___________________________________________________

	void GlEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		int nEdgeStrips = sampleEnd - sampleBegin;
		const int* pEdgeStrips = m_pSamples;
		int edgeStripPitch = m_nbSegments - 1;

		const spx* pEdges = m_pSamples + edgeStripPitch * edgeBegin;

		int outBytes = (nEdgeStrips - 1) * (m_nbSegments - 1) * 4 * sizeof(GLfloat);

		auto pBuffer = (GLfloat*)GfxBase::memStackAlloc(outBytes);

		auto pOut = pBuffer;

		for (int i = 0; i < nEdgeStrips - 1; i++)
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

				if ((edgeOut & 0xFFFFFFC0) <= (unsigned int)edgeIn)
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
					// 					endAdder = lastPixelCoverage - ((edgeOut & 0xFFFFFF00)-edgeIn)*increment / 256.f;
				}

				*pOut++ = edgeIn / 64.f;				// Segment begin pixel
				*pOut++ = increment;					// Segment increment
				*pOut++ = beginAdder;					// Segment begin adder
				*pOut++ = endAdder;						// Segment end adder
			}

			pEdges += edgeStripPitch;

		}

		// Upload buffer

		int ofs = edgeBegin * edgeStripPitch * 4 * sizeof(GLfloat);

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferSubData(GL_TEXTURE_BUFFER, ofs, outBytes, pBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		// Release temp buffer

		GfxBase::memStackFree(outBytes);
	}

	//____ _colorsUpdated() ____________________________________________________

	void GlEdgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		int segColors = m_size.w + m_size.h;

		int nColors = segColors * (endSegment - beginSegment);
		int tempBuffSize = nColors * sizeof(GLfloat) * 4;

		auto pTempBuffer = (GLfloat*)GfxBase::memStackAlloc(tempBuffSize);

		auto pIn = m_pColors + beginSegment * segColors;
		auto pOut = pTempBuffer;
		for (int i = 0; i < nColors; i++)
		{
			*pOut++ = pIn->r / 4096.f;
			*pOut++ = pIn->g / 4096.f;
			*pOut++ = pIn->b / 4096.f;
			*pOut++ = pIn->a / 4096.f;
			pIn++;
		}

		int bufferOffset = m_colorsOfs + beginSegment * segColors * 4 * sizeof(GLfloat);

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferSubData(GL_TEXTURE_BUFFER, bufferOffset, tempBuffSize, pTempBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		GfxBase::memStackFree(tempBuffSize);
	}

} // namespace wg

