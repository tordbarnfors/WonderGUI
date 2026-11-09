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

#include <wax_vulkanedgemap.h>
#include <wax_base.h>

namespace wax
{

	const TypeInfo VulkanEdgemap::TYPEINFO = { "VulkanEdgemap", &GpuEdgemap::TYPEINFO };

	//____ create() ______________________________________________________________

	VulkanEdgemap_p	VulkanEdgemap::create(const Blueprint& blueprint)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		return VulkanEdgemap_p(new VulkanEdgemap(blueprint));
	}

	VulkanEdgemap_p VulkanEdgemap::create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint) || edges >= blueprint.segments)
			return nullptr;

		auto p = VulkanEdgemap_p(new VulkanEdgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	VulkanEdgemap_p VulkanEdgemap::create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint) || edges >= blueprint.segments)
			return nullptr;

		auto p = VulkanEdgemap_p(new VulkanEdgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	VulkanEdgemap::VulkanEdgemap(const Blueprint& bp) : GpuEdgemap(bp)
	{
		// Without a buffer VulkanBackend draws nothing for the edgemap.

		if( !VulkanContext::isReady() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No Vulkan device set. VulkanBackend::setDevice() must be called before any edgemap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		_createContent();
	}

	//____ destructor ____________________________________________________________

	VulkanEdgemap::~VulkanEdgemap()
	{
		VulkanContext::destroyLater( m_buffer );
		VulkanContext::destroyLater( m_newBuffer );
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& VulkanEdgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _allocBuffer() ________________________________________________________
	//
	//TODO: There is one buffer and two frames in flight, so updating samples can change what
	// the GPU is still reading for the previous frame, as for DX12Edgemap.

	float * VulkanEdgemap::_allocBuffer( size_t bytes )
	{
		VulkanContext::destroyLater( m_newBuffer );

		if( !VulkanContext::createBuffer( bytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VulkanContext::Memory::HostWrite, m_newBuffer ) )
		{
			if( !VulkanContext::isDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for edgemap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return (float*) m_newBuffer.pMapped;
	}

	//____ _commitBuffer() _______________________________________________________

	void VulkanEdgemap::_commitBuffer()
	{
		VulkanContext::destroyLater( m_buffer );		// The GPU may still be reading it.

		m_buffer = m_newBuffer;
		m_newBuffer = VulkanContext::Buffer();
	}

} // namespace wax
