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

#include <wax_sdlgpuedgemap.h>
#include <wax_base.h>

namespace wax
{

	const TypeInfo SdlGpuEdgemap::TYPEINFO = { "SdlGpuEdgemap", &GpuEdgemap::TYPEINFO };

	//____ create() ______________________________________________________________

	SdlGpuEdgemap_p	SdlGpuEdgemap::create(const Blueprint& blueprint)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		return SdlGpuEdgemap_p(new SdlGpuEdgemap(blueprint));
	}

	SdlGpuEdgemap_p SdlGpuEdgemap::create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint) || edges >= blueprint.segments)
			return nullptr;

		auto p = SdlGpuEdgemap_p(new SdlGpuEdgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	SdlGpuEdgemap_p SdlGpuEdgemap::create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint) || edges >= blueprint.segments)
			return nullptr;

		auto p = SdlGpuEdgemap_p(new SdlGpuEdgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	SdlGpuEdgemap::SdlGpuEdgemap(const Blueprint& bp) : GpuEdgemap(bp)
	{
		// Without a buffer SdlGpuBackend draws nothing for the edgemap.

		if( !SdlGpuContext::isReady() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No SDL_GPU device set. SdlGpuBackend::setDevice() must be called before any edgemap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		_createContent();
	}

	//____ destructor ____________________________________________________________

	SdlGpuEdgemap::~SdlGpuEdgemap()
	{
		if( m_pBuffer && SdlGpuContext::device() )
			SDL_ReleaseGPUBuffer( SdlGpuContext::device(), m_pBuffer );		// Once the GPU is done with it.
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& SdlGpuEdgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _allocBuffer() ________________________________________________________

	float * SdlGpuEdgemap::_allocBuffer( size_t bytes )
	{
		m_newContent.assign(bytes / sizeof(float), 0.f);
		return m_newContent.data();
	}

	//____ _commitBuffer() _______________________________________________________
	//
	// A buffer of the new size, which SDL_GPU keeps for the GPU as long as it reads the old one.

	void SdlGpuEdgemap::_commitBuffer()
	{
		m_content.swap(m_newContent);		// Keeps where the content is, which GpuEdgemap points at.
		m_newContent.clear();
		m_newContent.shrink_to_fit();
		m_bChanged = true;

		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if( m_pBuffer )
			SDL_ReleaseGPUBuffer( pDevice, m_pBuffer );

		SDL_GPUBufferCreateInfo info = {};
		info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
		info.size = uint32_t(m_content.size() * sizeof(float));

		m_pBuffer = SDL_CreateGPUBuffer( pDevice, &info );

		WAX_SDL_CHECK( m_pBuffer != nullptr, "Creating the buffer of an edgemap" );
	}

	//____ _samplesUpdated() _____________________________________________________

	void SdlGpuEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		GpuEdgemap::_samplesUpdated(edgeBegin, edgeEnd, sampleBegin, sampleEnd);
		m_bChanged = true;
	}

	//____ _colorsUpdated() ______________________________________________________

	void SdlGpuEdgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		GpuEdgemap::_colorsUpdated(beginSegment, endSegment);
		m_bChanged = true;
	}

	//____ _recordUpload() _______________________________________________________
	//
	// The whole of it, as most changes touch most of it.

	void SdlGpuEdgemap::_recordUpload( SdlGpuContext::BufferUploads& uploads )
	{
		if( !m_bChanged || !m_pBuffer )
			return;

		uploads.add( m_pBuffer, 0, m_content.data(), uint32_t(m_content.size() * sizeof(float)), true );
		m_bChanged = false;
	}

} // namespace wax
