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

#include <wax_vulkanwarpmap.h>
#include <wax_base.h>

#include <cstring>
#include <algorithm>

namespace wax
{

	const TypeInfo VulkanWarpmap::TYPEINFO = { "VulkanWarpmap", &Warpmap::TYPEINFO };

	//____ create() ______________________________________________________________

	VulkanWarpmap_p VulkanWarpmap::create(const Blueprint& bp)
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return VulkanWarpmap_p(new VulkanWarpmap(bp));
	}

	VulkanWarpmap_p VulkanWarpmap::create(const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles)
	{
		if (!pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles))
			return nullptr;

		return VulkanWarpmap_p(new VulkanWarpmap(bp, pVertices, pTriangles));
	}

	//____ constructor ___________________________________________________________

	VulkanWarpmap::VulkanWarpmap(const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles) : Warpmap(bp, pVertices, pTriangles)
	{
		static_assert(sizeof(Vertex) == 6 * sizeof(int32_t) && sizeof(Triangle) == 3 * sizeof(uint16_t), "Vertices and triangles are uploaded as they are");

		if (!VulkanContext::isReady())
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No Vulkan device set. VulkanBackend::setDevice() must be called before any warpmap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Triangles start at an aligned offset after the vertices.

		m_indexOffset = (size_t(m_nVertices) * sizeof(Vertex) + 255) & ~size_t(255);
		size_t bytes = std::max(m_indexOffset + size_t(m_nTriangles) * sizeof(Triangle), size_t(256));

		if (!VulkanContext::createBuffer(bytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VulkanContext::Memory::HostWrite, m_buffer))
		{
			if (!VulkanContext::isDeviceLost())
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for warpmap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		//TODO: As for DX12Warpmap, there is one buffer and two frames in flight, so updating the
		// mesh can change what the GPU is still reading for the previous frame.

		_updated(0, m_nVertices, 0, m_nTriangles);
	}

	//____ destructor ____________________________________________________________

	VulkanWarpmap::~VulkanWarpmap()
	{
		VulkanContext::destroyLater(m_buffer);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& VulkanWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _updated() ____________________________________________________________
	//
	// Copies what changed to the buffer. Periods are given when drawing.

	void VulkanWarpmap::_updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles)
	{
		if (!m_buffer.pMapped)
			return;

		if (nVertices > 0)
			std::memcpy(m_buffer.pMapped + size_t(firstVertex) * sizeof(Vertex), m_pVertices + firstVertex, size_t(nVertices) * sizeof(Vertex));

		if (nTriangles > 0)
			std::memcpy(m_buffer.pMapped + m_indexOffset + size_t(firstTriangle) * sizeof(Triangle), m_pTriangles + firstTriangle, size_t(nTriangles) * sizeof(Triangle));
	}

} // namespace wax
