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
#ifndef WAX_SDLGPUWARPMAP_DOT_H
#define WAX_SDLGPUWARPMAP_DOT_H
#pragma once

#include <wax_warpmap.h>
#include <wax_sdlgpucontext.h>

namespace wax
{

	class SdlGpuWarpmap;
	typedef StrongPtr<SdlGpuWarpmap> SdlGpuWarpmap_p;
	typedef WeakPtr<SdlGpuWarpmap> SdlGpuWarpmap_wp;

	//____ SdlGpuWarpmap ________________________________________________________
	/**
	 * @brief Warpmap for SdlGpuBackend.
	 *
	 * The mesh is kept on the GPU as it is, in one buffer as VulkanWarpmap has it: the vertices
	 * (the layout of Warpmap::Vertex), read by the vertex shader as a storage buffer, followed by
	 * the triangles (the layout of Warpmap::Triangle), which are the index buffer. See vs_warp.hlsl
	 * and warp.hlsl for what the shaders do with it.
	 *
	 * SDL_GPU has no buffers the CPU writes to where the GPU reads them, so a mesh that has changed
	 * is uploaded from what Warpmap keeps in memory when a session that draws it ends, all of it,
	 * cycling the buffer so that sessions still in flight keep reading what they had.
	 *
	 * Needs the device set by SdlGpuBackend::setDevice().
	 */

	class SdlGpuWarpmap : public Warpmap
	{
		friend class SdlGpuBackend;

	public:

		//.____ Creation __________________________________________

		static SdlGpuWarpmap_p	create(const Blueprint& blueprint);
		static SdlGpuWarpmap_p	create(const Blueprint& blueprint, const Vertex * pVertices, const Triangle * pTriangles);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		SdlGpuWarpmap(const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr);
		~SdlGpuWarpmap();

		void	_updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles) override;

		// For SdlGpuBackend: has the mesh uploaded, if changed.

		void	_recordUpload( SdlGpuContext::BufferUploads& uploads );

		SDL_GPUBuffer *	m_pBuffer = nullptr;
		uint32_t		m_indexOffset = 0;			// Where the triangles start in m_pBuffer, in bytes.
		bool			m_bChanged = true;
	};

} // namespace wax
#endif // WAX_SDLGPUWARPMAP_DOT_H
