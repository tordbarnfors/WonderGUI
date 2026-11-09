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
#ifndef WAX_VULKANEDGEMAP_DOT_H
#define WAX_VULKANEDGEMAP_DOT_H
#pragma once

#include <wax_gpuedgemap.h>
#include <wax_vulkancontext.h>

namespace wax
{

	class VulkanEdgemap;
	typedef StrongPtr<VulkanEdgemap> VulkanEdgemap_p;
	typedef WeakPtr<VulkanEdgemap> VulkanEdgemap_wp;

	//____ VulkanEdgemap ________________________________________________________
	/**
	 * @brief Edgemap for VulkanBackend.
	 *
	 * Its content is laid out by GpuEdgemap, in a buffer the GPU reads where it is, mapped for
	 * the CPU to write to.
	 *
	 * Needs the device set by VulkanBackend::setDevice().
	 */

	class VulkanEdgemap : public GpuEdgemap
	{
		friend class VulkanBackend;

	public:

		//.____ Creation __________________________________________

		static VulkanEdgemap_p	create(const Blueprint& blueprint);
		static VulkanEdgemap_p	create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);
		static VulkanEdgemap_p	create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		VulkanEdgemap(const Blueprint& bp);
		~VulkanEdgemap();

		float *		_allocBuffer( size_t bytes ) override;
		void		_commitBuffer() override;

		VkBuffer	_buffer() const { return m_buffer.buffer; }

		VulkanContext::Buffer	m_buffer;
		VulkanContext::Buffer	m_newBuffer;		// Between _allocBuffer() and _commitBuffer().
	};

} // namespace wax
#endif // WAX_VULKANEDGEMAP_DOT_H
