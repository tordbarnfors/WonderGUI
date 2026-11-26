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

#include <wax_sdlgpuwarpmap.h>
#include <wax_base.h>

#include <algorithm>

namespace wax
{

	const TypeInfo SdlGpuWarpmap::TYPEINFO = { "SdlGpuWarpmap", &Warpmap::TYPEINFO };

	//____ create() ______________________________________________________________

	SdlGpuWarpmap_p SdlGpuWarpmap::create(const Blueprint& bp)
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return SdlGpuWarpmap_p(new SdlGpuWarpmap(bp));
	}

	SdlGpuWarpmap_p SdlGpuWarpmap::create(const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles)
	{
		if (!pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles))
			return nullptr;

		return SdlGpuWarpmap_p(new SdlGpuWarpmap(bp, pVertices, pTriangles));
	}

	//____ constructor ___________________________________________________________

	SdlGpuWarpmap::SdlGpuWarpmap(const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles) : Warpmap(bp, pVertices, pTriangles)
	{
		static_assert(sizeof(Vertex) == 6 * sizeof(int32_t) && sizeof(Triangle) == 3 * sizeof(uint16_t), "Vertices and triangles are uploaded as they are");

		if (!SdlGpuContext::isReady())
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No SDL_GPU device set. SdlGpuBackend::setDevice() must be called before any warpmap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Triangles start at an aligned offset after the vertices.

		m_indexOffset = (uint32_t(m_nVertices) * sizeof(Vertex) + 255) & ~uint32_t(255);

		SDL_GPUBufferCreateInfo info = {};
		info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_BUFFERUSAGE_INDEX;
		info.size = std::max(m_indexOffset + uint32_t(m_nTriangles) * uint32_t(sizeof(Triangle)), uint32_t(256));

		m_pBuffer = SDL_CreateGPUBuffer(SdlGpuContext::device(), &info);

		WAX_SDL_CHECK(m_pBuffer != nullptr, "Creating the buffer of a warpmap");
	}

	//____ destructor ____________________________________________________________

	SdlGpuWarpmap::~SdlGpuWarpmap()
	{
		if (m_pBuffer && SdlGpuContext::device())
			SDL_ReleaseGPUBuffer(SdlGpuContext::device(), m_pBuffer);		// Once the GPU is done with it.
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& SdlGpuWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _updated() ____________________________________________________________
	//
	// Periods are given when drawing.

	void SdlGpuWarpmap::_updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles)
	{
		if (nVertices > 0 || nTriangles > 0)
			m_bChanged = true;
	}

	//____ _recordUpload() _______________________________________________________
	//
	// The vertices cycle the buffer, after which the triangles are written into the new one.

	void SdlGpuWarpmap::_recordUpload( SdlGpuContext::BufferUploads& uploads )
	{
		if (!m_bChanged || !m_pBuffer || m_nTriangles == 0)
			return;

		uploads.add(m_pBuffer, 0, m_pVertices, uint32_t(m_nVertices) * uint32_t(sizeof(Vertex)), true);
		uploads.add(m_pBuffer, m_indexOffset, m_pTriangles, uint32_t(m_nTriangles) * uint32_t(sizeof(Triangle)), false);

		m_bChanged = false;
	}

} // namespace wax
