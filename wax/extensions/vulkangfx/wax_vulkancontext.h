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
#ifndef WAX_VULKANCONTEXT_DOT_H
#define WAX_VULKANCONTEXT_DOT_H
#pragma once

#include <wax_types.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>

namespace wax
{
	class Object;
	struct TypeInfo;
	class VulkanBackend;

	//____ VulkanContext ________________________________________________________
	/**
	 * @brief The Vulkan device shared by VulkanBackends and their surfaces, edgemaps and Warpmaps.
	 *
	 * Set by VulkanBackend::setDevice(). All of Wax's work goes to one queue, in one sequence of
	 * submissions counted by a timeline semaphore, which is what lets resources move freely between
	 * backends and be released once the GPU has passed the last submission that may use them, see
	 * destroyLater().
	 *
	 * Not thread safe, like the rest of Wax.
	 */

	class VulkanContext
	{
	public:

		struct Setup
		{
			VkInstance			instance = VK_NULL_HANDLE;
			VkPhysicalDevice	physicalDevice = VK_NULL_HANDLE;
			VkDevice			device = VK_NULL_HANDLE;		// Vulkan 1.3, see VulkanBackend::setDevice() for what it needs.
			VkQueue				queue = VK_NULL_HANDLE;			// Graphics queue.
			uint32_t			queueFamily = 0;
		};

		static bool				init( const Setup& setup );
		static void				exit();							// Waits for the GPU and releases everything still held.
		static bool				isReady() { return s_setup.device != VK_NULL_HANDLE; }

		static VkInstance		instance() { return s_setup.instance; }
		static VkPhysicalDevice	physicalDevice() { return s_setup.physicalDevice; }
		static VkDevice			device() { return s_setup.device; }
		static VkQueue			queue() { return s_setup.queue; }
		static uint32_t			queueFamily() { return s_setup.queueFamily; }

		static const VkPhysicalDeviceProperties& properties() { return s_properties; }

		//.____ Errors ____________________________________________________________

		// Reports a failed call through the error handler, once if it is because the device is
		// lost, which every call then is. Returns false if result is an error.

		static bool				check( VkResult result, const char * pWhat, const Object * pObject, const TypeInfo * pClassType,
									   const char * func, const char * file, int line );

		static bool				isDeviceLost() { return s_bDeviceLost; }

		//.____ Memory ____________________________________________________________

		// Where a buffer lives. HostWrite is written by the CPU and read by the GPU, as D3D12's
		// upload heap. HostRead is also read back by the CPU, so it prefers cached memory.

		enum class Memory
		{
			HostWrite,
			HostRead,
			Device
		};

		struct Buffer
		{
			VkBuffer		buffer = VK_NULL_HANDLE;
			VkDeviceMemory	memory = VK_NULL_HANDLE;
			uint8_t *		pMapped = nullptr;				// Permanently mapped, unless Memory::Device.
			VkDeviceSize	size = 0;
		};

		struct Image
		{
			VkImage			image = VK_NULL_HANDLE;
			VkDeviceMemory	memory = VK_NULL_HANDLE;
		};

		static bool				createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, Memory memory, Buffer& buffer );
		static bool				createImage( const VkImageCreateInfo& info, Image& image );

		// Releases once the GPU is done with every submission that may use it, including what
		// backends are recording but haven't submitted yet. Clears what it is given.

		static void				destroyLater( Buffer& buffer );
		static void				destroyLater( Image& image );
		static void				destroyLater( VkImageView& view );

		//.____ Submission ________________________________________________________

		// Submits a command buffer, signalling the timeline semaphore with the next value, which
		// is returned. 0 on failure.

		static uint64_t			submit( VkCommandBuffer cmd, int nWaits = 0, const VkSemaphore * pWaits = nullptr, const VkPipelineStageFlags * pWaitStages = nullptr,
										int nSignals = 0, const VkSemaphore * pSignals = nullptr );

		static uint64_t			lastSubmitted() { return s_lastSubmitted; }
		static uint64_t			completed();						// Last value the GPU has passed.
		static void				wait( uint64_t value );				// Until the GPU has passed it.
		static void				waitIdle();							// For everything submitted so far.

		// Backends say when they start and stop recording, so that destroyLater() knows what may
		// still be used by commands not yet submitted.

		static void				beginRecording();
		static void				endRecording( uint64_t submitted );	// 0 if nothing was submitted.

		// Releases what the GPU is done with. Called by backends once a frame.

		static void				collectGarbage();

		//.____ One-shot commands _________________________________________________

		// For surfaces: records commands into a command buffer of our own, which endOneShot()
		// submits and waits for. Everything backends have recorded is submitted first, so that the
		// one-shot commands come after it, as they are meant to.

		static VkCommandBuffer	beginOneShot();
		static bool				endOneShot( VkCommandBuffer cmd );

		//.____ Backends ___________________________________________________________

		static void				addBackend( VulkanBackend * pBackend );
		static void				removeBackend( VulkanBackend * pBackend );
		static int				nbBackends() { return int(s_backends.size()); }

		static void				flushBackends();					// Submits what every backend has recorded.

		//.____ Barriers ___________________________________________________________

		// Moves an image from one layout to another, with everything before it in the queue done
		// with it and nothing after it starting before. Not cheap, but layouts change seldom.

		static void				transition( VkCommandBuffer cmd, VkImage image, VkImageLayout from, VkImageLayout to,
											uint32_t firstMip = 0, uint32_t nMips = VK_REMAINING_MIP_LEVELS );

	private:

		static uint32_t			_memoryType( uint32_t typeBits, Memory memory );
		static void				_reportDeviceLost();

		struct Garbage
		{
			uint64_t		value;			// Destroy once the GPU has passed this. 0 until known.
			VkBuffer		buffer;
			VkImage			image;
			VkImageView		view;
			VkDeviceMemory	memory;
		};

		static void				_addGarbage( const Garbage& garbage );
		static void				_destroy( const Garbage& garbage );

		static Setup						s_setup;
		static VkPhysicalDeviceProperties	s_properties;
		static VkPhysicalDeviceMemoryProperties	s_memoryProperties;

		static VkSemaphore					s_timeline;
		static uint64_t						s_lastSubmitted;

		static VkCommandPool				s_oneShotPool;

		static bool							s_bDeviceLost;

		static int							s_nRecording;		// Backends recording right now.
		static std::vector<Garbage>			s_garbage;

		static std::vector<VulkanBackend*>	s_backends;
	};

	#define WAX_VK_CHECK(call, what) VulkanContext::check(call, what, this, &TYPEINFO, __func__, __FILE__, __LINE__)

} // namespace wax
#endif // WAX_VULKANCONTEXT_DOT_H
