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
#ifndef WAX_VULKANWARPMAP_DOT_H
#define WAX_VULKANWARPMAP_DOT_H
#pragma once

#include <wax_warpmap.h>
#include <wax_vulkancontext.h>

namespace wax
{

	class VulkanWarpmap;
	typedef StrongPtr<VulkanWarpmap> VulkanWarpmap_p;
	typedef WeakPtr<VulkanWarpmap> VulkanWarpmap_wp;

	//____ VulkanWarpmap ________________________________________________________
	/**
	 * @brief Warpmap for VulkanBackend.
	 *
	 * The mesh is kept on the GPU as it is, in one buffer mapped for the CPU to write to, as
	 * DX12Warpmap has it: the vertices (the layout of Warpmap::Vertex), read by the vertex shader
	 * as a storage buffer, followed by the triangles (the layout of Warpmap::Triangle), which are
	 * the index buffer. See vs_warp.hlsl and warp.hlsl for what the shaders do with it.
	 *
	 * Needs the device set by VulkanBackend::setDevice().
	 */

	class VulkanWarpmap : public Warpmap
	{
		friend class VulkanBackend;

	public:

		//.____ Creation __________________________________________

		static VulkanWarpmap_p	create(const Blueprint& blueprint);
		static VulkanWarpmap_p	create(const Blueprint& blueprint, const Vertex * pVertices, const Triangle * pTriangles);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		VulkanWarpmap(const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr);
		~VulkanWarpmap();

		void	_updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles) override;

		VulkanContext::Buffer	m_buffer;
		size_t					m_indexOffset = 0;			// Where the triangles start in m_buffer, in bytes.
	};

} // namespace wax
#endif // WAX_VULKANWARPMAP_DOT_H
