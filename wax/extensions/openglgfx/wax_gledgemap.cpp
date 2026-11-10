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
#include <wax_gledgemap.h>
#include <wax_base.h>

#include <cstring>

namespace wax
{

	const TypeInfo GlEdgemap::TYPEINFO = { "GlEdgemap", &GpuEdgemap::TYPEINFO };


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

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
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

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	GlEdgemap::GlEdgemap(const Blueprint& bp) : GpuEdgemap(bp)
	{
		GLint oldTexture;
		glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &oldTexture);

		glGenBuffers(1, &m_bufferId);
		glGenTextures(1, &m_textureId);

		// A buffer only exists once it has been bound.

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		glBindTexture(GL_TEXTURE_BUFFER, m_textureId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufferId);
		glBindTexture(GL_TEXTURE_BUFFER, GLuint(oldTexture));

		_createContent();
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

	//____ _allocBuffer() ________________________________________________________

	float * GlEdgemap::_allocBuffer( size_t bytes )
	{
		m_newContent.assign(bytes / sizeof(float), 0.f);
		return m_newContent.data();
	}

	//____ _commitBuffer() _______________________________________________________

	void GlEdgemap::_commitBuffer()
	{
		m_content.swap(m_newContent);		// Keeps where the content is, which GpuEdgemap points at.
		m_newContent.clear();
		m_newContent.shrink_to_fit();
		m_bChanged = true;
	}

	//____ _samplesUpdated() _____________________________________________________

	void GlEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		GpuEdgemap::_samplesUpdated(edgeBegin, edgeEnd, sampleBegin, sampleEnd);
		m_bChanged = true;
	}

	//____ _colorsUpdated() ______________________________________________________

	void GlEdgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		GpuEdgemap::_colorsUpdated(beginSegment, endSegment);
		m_bChanged = true;
	}

	//____ _upload() _____________________________________________________________
	//
	// Called by GlBackend when it renders a draw that reads the edgemap, which is when the content
	// should be read, see the Wax design. The whole of it, as most changes touch most of it.

	void GlEdgemap::_upload()
	{
		if (!m_bChanged)
			return;

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(m_content.size() * sizeof(float)), m_content.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		m_bChanged = false;
	}

} // namespace wax

