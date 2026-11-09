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

#include <wax_vulkancontext.h>
#include <wax_vulkanbackend.h>
#include <wax_base.h>

#include <algorithm>
#include <cstdio>

namespace wax
{
	VulkanContext::Setup					VulkanContext::s_setup;
	VkPhysicalDeviceProperties				VulkanContext::s_properties = {};
	VkPhysicalDeviceMemoryProperties		VulkanContext::s_memoryProperties = {};
	VkSemaphore								VulkanContext::s_timeline = VK_NULL_HANDLE;
	uint64_t								VulkanContext::s_lastSubmitted = 0;
	VkCommandPool							VulkanContext::s_oneShotPool = VK_NULL_HANDLE;
	bool									VulkanContext::s_bDeviceLost = false;
	int										VulkanContext::s_nRecording = 0;
	std::vector<VulkanContext::Garbage>		VulkanContext::s_garbage;
	std::vector<VulkanBackend*>				VulkanContext::s_backends;

	static const TypeInfo s_typeInfo = { "VulkanContext", nullptr };

	//____ init() ________________________________________________________________

	bool VulkanContext::init( const Setup& setup )
	{
		if( isReady() )
			exit();

		if( !setup.device || !setup.physicalDevice || !setup.queue )
			return false;

		s_setup = setup;
		s_bDeviceLost = false;
		s_lastSubmitted = 0;

		vkGetPhysicalDeviceProperties( setup.physicalDevice, &s_properties );
		vkGetPhysicalDeviceMemoryProperties( setup.physicalDevice, &s_memoryProperties );

		// One timeline semaphore counts all our submissions.

		VkSemaphoreTypeCreateInfo typeInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
		typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		typeInfo.initialValue = 0;

		VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semInfo.pNext = &typeInfo;

		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = setup.queueFamily;

		if( !check( vkCreateSemaphore( setup.device, &semInfo, nullptr, &s_timeline ), "vkCreateSemaphore", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) ||
			!check( vkCreateCommandPool( setup.device, &poolInfo, nullptr, &s_oneShotPool ), "vkCreateCommandPool", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
		{
			exit();
			return false;
		}

		return true;
	}

	//____ exit() ________________________________________________________________

	void VulkanContext::exit()
	{
		if( !isReady() )
			return;

		if( !s_bDeviceLost )
			vkDeviceWaitIdle( s_setup.device );

		for( auto& garbage : s_garbage )
			_destroy( garbage );

		s_garbage.clear();

		if( s_oneShotPool )
			vkDestroyCommandPool( s_setup.device, s_oneShotPool, nullptr );

		if( s_timeline )
			vkDestroySemaphore( s_setup.device, s_timeline, nullptr );

		s_oneShotPool = VK_NULL_HANDLE;
		s_timeline = VK_NULL_HANDLE;
		s_setup = Setup();
		s_lastSubmitted = 0;
		s_nRecording = 0;
	}

	//____ check() _______________________________________________________________

	bool VulkanContext::check( VkResult result, const char * pWhat, const Object * pObject, const TypeInfo * pClassType,
							   const char * func, const char * file, int line )
	{
		if( result >= 0 )
			return true;

		if( result == VK_ERROR_DEVICE_LOST )
		{
			_reportDeviceLost();
			return false;
		}

		if( s_bDeviceLost )
			return false;			// Anything may fail once the device is gone, and that has been reported.

		char buffer[256];
		snprintf( buffer, sizeof(buffer), "%s failed, VkResult = %d", pWhat, int(result) );
		Base::throwError( ErrorLevel::Error, ErrorCode::RenderFailure, buffer, pObject, pClassType, func, file, line );
		return false;
	}

	//____ _reportDeviceLost() ___________________________________________________

	void VulkanContext::_reportDeviceLost()
	{
		if( s_bDeviceLost )
			return;

		s_bDeviceLost = true;

		Base::throwError( ErrorLevel::Error, ErrorCode::RenderFailure,
			"Vulkan device lost. Vulkan rendering stops, a new device can't be set yet.",
			nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ );
	}

	//____ _memoryType() _________________________________________________________

	uint32_t VulkanContext::_memoryType( uint32_t typeBits, Memory memory )
	{
		// In order of preference. Host visible memory is kept mapped and must be coherent, as we
		// never flush it.

		VkMemoryPropertyFlags wanted[3] = {};
		int nWanted = 0;

		switch( memory )
		{
			case Memory::HostWrite:
				wanted[nWanted++] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				break;

			case Memory::HostRead:
				wanted[nWanted++] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
				wanted[nWanted++] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				break;

			case Memory::Device:
				wanted[nWanted++] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				wanted[nWanted++] = 0;
				break;
		}

		for( int w = 0; w < nWanted; w++ )
		{
			for( uint32_t i = 0; i < s_memoryProperties.memoryTypeCount; i++ )
			{
				if( (typeBits & (1u << i)) && (s_memoryProperties.memoryTypes[i].propertyFlags & wanted[w]) == wanted[w] )
					return i;
			}
		}

		return UINT32_MAX;
	}

	//____ createBuffer() ________________________________________________________

	bool VulkanContext::createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, Memory memory, Buffer& buffer )
	{
		buffer = Buffer();

		if( !isReady() || s_bDeviceLost )
			return false;

		if( size == 0 )
			size = 16;

		VkBufferCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = size;
		info.usage = usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if( !check( vkCreateBuffer( s_setup.device, &info, nullptr, &buffer.buffer ), "vkCreateBuffer", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
			return false;

		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements( s_setup.device, buffer.buffer, &req );

		VkMemoryAllocateInfo alloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		alloc.allocationSize = req.size;
		alloc.memoryTypeIndex = _memoryType( req.memoryTypeBits, memory );

		if( alloc.memoryTypeIndex == UINT32_MAX ||
			!check( vkAllocateMemory( s_setup.device, &alloc, nullptr, &buffer.memory ), "vkAllocateMemory", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) ||
			!check( vkBindBufferMemory( s_setup.device, buffer.buffer, buffer.memory, 0 ), "vkBindBufferMemory", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
		{
			_destroy( { 0, buffer.buffer, VK_NULL_HANDLE, VK_NULL_HANDLE, buffer.memory } );
			buffer = Buffer();
			return false;
		}

		if( memory != Memory::Device )
		{
			void * pMapped = nullptr;

			if( !check( vkMapMemory( s_setup.device, buffer.memory, 0, VK_WHOLE_SIZE, 0, &pMapped ), "vkMapMemory", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
			{
				_destroy( { 0, buffer.buffer, VK_NULL_HANDLE, VK_NULL_HANDLE, buffer.memory } );
				buffer = Buffer();
				return false;
			}

			buffer.pMapped = (uint8_t*) pMapped;
		}

		buffer.size = size;
		return true;
	}

	//____ createImage() _________________________________________________________

	bool VulkanContext::createImage( const VkImageCreateInfo& info, Image& image )
	{
		image = Image();

		if( !isReady() || s_bDeviceLost )
			return false;

		if( !check( vkCreateImage( s_setup.device, &info, nullptr, &image.image ), "vkCreateImage", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
			return false;

		VkMemoryRequirements req;
		vkGetImageMemoryRequirements( s_setup.device, image.image, &req );

		VkMemoryAllocateInfo alloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		alloc.allocationSize = req.size;
		alloc.memoryTypeIndex = _memoryType( req.memoryTypeBits, Memory::Device );

		if( alloc.memoryTypeIndex == UINT32_MAX ||
			!check( vkAllocateMemory( s_setup.device, &alloc, nullptr, &image.memory ), "vkAllocateMemory", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) ||
			!check( vkBindImageMemory( s_setup.device, image.image, image.memory, 0 ), "vkBindImageMemory", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
		{
			_destroy( { 0, VK_NULL_HANDLE, image.image, VK_NULL_HANDLE, image.memory } );
			image = Image();
			return false;
		}

		return true;
	}

	//____ destroyLater() ________________________________________________________

	void VulkanContext::destroyLater( Buffer& buffer )
	{
		if( buffer.buffer || buffer.memory )
			_addGarbage( { 0, buffer.buffer, VK_NULL_HANDLE, VK_NULL_HANDLE, buffer.memory } );

		buffer = Buffer();
	}

	void VulkanContext::destroyLater( Image& image )
	{
		if( image.image || image.memory )
			_addGarbage( { 0, VK_NULL_HANDLE, image.image, VK_NULL_HANDLE, image.memory } );

		image = Image();
	}

	void VulkanContext::destroyLater( VkImageView& view )
	{
		if( view )
			_addGarbage( { 0, VK_NULL_HANDLE, VK_NULL_HANDLE, view, VK_NULL_HANDLE } );

		view = VK_NULL_HANDLE;
	}

	//____ _addGarbage() _________________________________________________________
	//
	// Whatever is being recorded may use it, and gets submitted with a value we don't know yet.
	// Once nothing is being recorded, everything that may use it has been submitted.

	void VulkanContext::_addGarbage( const Garbage& garbage )
	{
		if( !isReady() )
			return;

		Garbage entry = garbage;
		entry.value = s_nRecording > 0 ? 0 : s_lastSubmitted;

		s_garbage.push_back( entry );
	}

	//____ _destroy() ____________________________________________________________

	void VulkanContext::_destroy( const Garbage& garbage )
	{
		VkDevice device = s_setup.device;

		if( garbage.view )
			vkDestroyImageView( device, garbage.view, nullptr );
		if( garbage.image )
			vkDestroyImage( device, garbage.image, nullptr );
		if( garbage.buffer )
			vkDestroyBuffer( device, garbage.buffer, nullptr );
		if( garbage.memory )
			vkFreeMemory( device, garbage.memory, nullptr );		// Unmaps it too.
	}

	//____ collectGarbage() ______________________________________________________

	void VulkanContext::collectGarbage()
	{
		if( s_garbage.empty() )
			return;

		uint64_t done = completed();

		auto it = std::remove_if( s_garbage.begin(), s_garbage.end(), [done]( const Garbage& garbage )
		{
			// A lost device runs nothing more, so nothing is in use.

			if( (garbage.value != 0 || s_nRecording == 0) && (garbage.value <= done || s_bDeviceLost) )
			{
				_destroy( garbage );
				return true;
			}
			return false;
		});

		s_garbage.erase( it, s_garbage.end() );
	}

	//____ submit() ______________________________________________________________

	uint64_t VulkanContext::submit( VkCommandBuffer cmd, int nWaits, const VkSemaphore * pWaits, const VkPipelineStageFlags * pWaitStages,
									int nSignals, const VkSemaphore * pSignals )
	{
		if( !isReady() || s_bDeviceLost )
			return 0;

		uint64_t value = s_lastSubmitted + 1;

		VkSemaphore signals[8];
		uint64_t signalValues[8] = {};

		int n = 0;
		for( int i = 0; i < nSignals && n < 7; i++ )
			signals[n++] = pSignals[i];

		signals[n] = s_timeline;
		signalValues[n] = value;
		n++;

		uint64_t waitValues[8] = {};

		VkTimelineSemaphoreSubmitInfo timelineInfo = { VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
		timelineInfo.waitSemaphoreValueCount = uint32_t(nWaits);
		timelineInfo.pWaitSemaphoreValues = waitValues;
		timelineInfo.signalSemaphoreValueCount = uint32_t(n);
		timelineInfo.pSignalSemaphoreValues = signalValues;

		VkSubmitInfo info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		info.pNext = &timelineInfo;
		info.waitSemaphoreCount = uint32_t(nWaits);
		info.pWaitSemaphores = pWaits;
		info.pWaitDstStageMask = pWaitStages;
		info.commandBufferCount = cmd ? 1 : 0;
		info.pCommandBuffers = &cmd;
		info.signalSemaphoreCount = uint32_t(n);
		info.pSignalSemaphores = signals;

		if( !check( vkQueueSubmit( s_setup.queue, 1, &info, VK_NULL_HANDLE ), "vkQueueSubmit", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
			return 0;

		s_lastSubmitted = value;
		return value;
	}

	//____ completed() ___________________________________________________________

	uint64_t VulkanContext::completed()
	{
		if( !s_timeline )
			return 0;

		if( s_bDeviceLost )
			return s_lastSubmitted;

		uint64_t value = 0;
		if( !check( vkGetSemaphoreCounterValue( s_setup.device, s_timeline, &value ), "vkGetSemaphoreCounterValue", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
			return s_lastSubmitted;

		return value;
	}

	//____ wait() ________________________________________________________________

	void VulkanContext::wait( uint64_t value )
	{
		if( !s_timeline || s_bDeviceLost || value == 0 )
			return;

		VkSemaphoreWaitInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
		info.semaphoreCount = 1;
		info.pSemaphores = &s_timeline;
		info.pValues = &value;

		// Not forever: a GPU that never gets there is lost, and the driver should say so well
		// before ten seconds.

		check( vkWaitSemaphores( s_setup.device, &info, 10ull * 1000 * 1000 * 1000 ), "vkWaitSemaphores", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ );
	}

	//____ waitIdle() ____________________________________________________________

	void VulkanContext::waitIdle()
	{
		wait( s_lastSubmitted );
	}

	//____ beginRecording() ______________________________________________________

	void VulkanContext::beginRecording()
	{
		s_nRecording++;
	}

	//____ endRecording() ________________________________________________________

	void VulkanContext::endRecording( uint64_t submitted )
	{
		if( s_nRecording > 0 )
			s_nRecording--;

		if( s_nRecording > 0 )
			return;

		// Everything that may use garbage of unknown value has now been submitted.

		uint64_t value = std::max( submitted, s_lastSubmitted );

		for( auto& garbage : s_garbage )
		{
			if( garbage.value == 0 )
				garbage.value = value;
		}
	}

	//____ beginOneShot() ________________________________________________________

	VkCommandBuffer VulkanContext::beginOneShot()
	{
		if( !isReady() || s_bDeviceLost )
			return VK_NULL_HANDLE;

		// What backends have recorded comes before us in the queue, as it was meant to happen
		// before whatever we are about to do.

		flushBackends();

		VkCommandBufferAllocateInfo alloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		alloc.commandPool = s_oneShotPool;
		alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc.commandBufferCount = 1;

		VkCommandBuffer cmd = VK_NULL_HANDLE;

		if( !check( vkAllocateCommandBuffers( s_setup.device, &alloc, &cmd ), "vkAllocateCommandBuffers", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
			return VK_NULL_HANDLE;

		VkCommandBufferBeginInfo begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if( !check( vkBeginCommandBuffer( cmd, &begin ), "vkBeginCommandBuffer", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ ) )
		{
			vkFreeCommandBuffers( s_setup.device, s_oneShotPool, 1, &cmd );
			return VK_NULL_HANDLE;
		}

		return cmd;
	}

	//____ endOneShot() __________________________________________________________

	bool VulkanContext::endOneShot( VkCommandBuffer cmd )
	{
		if( !cmd )
			return false;

		bool bOK = check( vkEndCommandBuffer( cmd ), "vkEndCommandBuffer", nullptr, &s_typeInfo, __func__, __FILE__, __LINE__ );

		uint64_t value = bOK ? submit( cmd ) : 0;

		if( value )
			wait( value );

		vkFreeCommandBuffers( s_setup.device, s_oneShotPool, 1, &cmd );
		return value != 0 && !s_bDeviceLost;
	}

	//____ addBackend() __________________________________________________________

	void VulkanContext::addBackend( VulkanBackend * pBackend )
	{
		s_backends.push_back( pBackend );
	}

	//____ removeBackend() _______________________________________________________

	void VulkanContext::removeBackend( VulkanBackend * pBackend )
	{
		s_backends.erase( std::remove( s_backends.begin(), s_backends.end(), pBackend ), s_backends.end() );
	}

	//____ flushBackends() _______________________________________________________

	void VulkanContext::flushBackends()
	{
		for( auto pBackend : s_backends )
			pBackend->_flush();
	}

	//____ transition() __________________________________________________________

	void VulkanContext::transition( VkCommandBuffer cmd, VkImage image, VkImageLayout from, VkImageLayout to, uint32_t firstMip, uint32_t nMips )
	{
		if( from == to || !cmd || !image )
			return;

		VkImageMemoryBarrier2 barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.oldLayout = from;
		barrier.newLayout = to;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = firstMip;
		barrier.subresourceRange.levelCount = nMips;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkDependencyInfo dep = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dep.imageMemoryBarrierCount = 1;
		dep.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2( cmd, &dep );
	}

} // namespace wax
