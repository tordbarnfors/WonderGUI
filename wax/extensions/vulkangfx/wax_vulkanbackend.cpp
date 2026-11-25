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

#include <wax_vulkanbackend.h>
#include <wax_vulkansurface.h>
#include <wax_vulkansurfacefactory.h>
#include <wax_vulkanedgemap.h>
#include <wax_vulkanedgemapfactory.h>
#include <wax_vulkanwarpmap.h>
#include <wax_vulkanwarpmapfactory.h>
#include <wax_vulkanshaders.h>

#include <wax_base.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <type_traits>

namespace wax
{
	const TypeInfo VulkanBackend::TYPEINFO = { "VulkanBackend", &GpuBackend::TYPEINFO };

	// Where the constant blocks are in the push constants, see constants.hlsl.

	static const uint32_t c_canvasConstantsOfs = 0;
	static const uint32_t c_pixelConstantsOfs = 32;
	static const uint32_t c_warpConstantsOfs = 48;
	static const uint32_t c_warpConstantsSize = 24;			// The padding of WarpConstants is HLSL's, not ours.
	static const uint32_t c_pushConstantsSize = c_warpConstantsOfs + c_warpConstantsSize;

	// Bindings of the descriptor set, see ../hlsl/README.md.

	enum Binding : uint32_t
	{
		Colors = 0,				// t0
		Extras = 1,				// t1
		BlitSource = 2,			// t2
		EdgemapOrPalette = 3,	// t3
		WarpVertices = 4,		// t4
		Sampler = 5				// s0
	};

	//____ create() ______________________________________________________________

	VulkanBackend_p VulkanBackend::create()
	{
		if (!VulkanContext::isReady())
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No Vulkan device set. Call VulkanBackend::setDevice() before creating a backend.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return VulkanBackend_p(new VulkanBackend());
	}

	//____ setDevice() ___________________________________________________________

	bool VulkanBackend::setDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, uint32_t queueFamily)
	{
		if (device == VulkanContext::device() && device != VK_NULL_HANDLE)
			return true;

		if (VulkanContext::nbBackends() > 0)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::IllegalCall,
				"Can't change Vulkan device while VulkanBackends exist.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if (device == VK_NULL_HANDLE)
		{
			VulkanContext::exit();
			return true;
		}

		VulkanContext::Setup setup;
		setup.instance = instance;
		setup.physicalDevice = physicalDevice;
		setup.device = device;
		setup.queue = queue;
		setup.queueFamily = queueFamily;

		return VulkanContext::init(setup);
	}

	//____ requiredDeviceExtensions() ____________________________________________

	const char * const * VulkanBackend::requiredDeviceExtensions( int& count )
	{
		static const char * const s_extensions[] = { VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME };

		count = int(sizeof(s_extensions) / sizeof(s_extensions[0]));
		return s_extensions;
	}

	//____ constructor ___________________________________________________________

	VulkanBackend::VulkanBackend()
	{
		m_pSurfaceFactory = VulkanSurfaceFactory::create();
		m_pEdgemapFactory = VulkanEdgemapFactory::create();
		m_pWarpmapFactory = VulkanWarpmapFactory::create();

		VulkanContext::addBackend(this);

		m_bDeviceObjectsOK = _createDeviceObjects();

		if (!m_bDeviceObjectsOK && !VulkanContext::isDeviceLost())
			Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the Vulkan objects of the backend, nothing will render.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	//____ destructor ____________________________________________________________

	VulkanBackend::~VulkanBackend()
	{
		// Our objects must not be destroyed while the GPU still uses them.

		if (m_bRecording)
		{
			_endRendering();

			uint64_t submitted = _submitCommandBuffer();
			m_bRecording = false;
			VulkanContext::endRecording(submitted);
		}

		VulkanContext::removeBackend(this);

		for (auto& frame : m_frames)
			VulkanContext::wait(frame.submitted);

		_destroyDeviceObjects();

		VulkanContext::collectGarbage();
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& VulkanBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _createDeviceObjects() ________________________________________________

	bool VulkanBackend::_createDeviceObjects()
	{
		VkDevice device = VulkanContext::device();

		m_vkCmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSetKHR) vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");

		if (!m_vkCmdPushDescriptorSet)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"The Vulkan device has no VK_KHR_push_descriptor, which VulkanBackend needs.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		// A command pool per frame, reset as a whole when the frame starts over.

		for (auto& frame : m_frames)
		{
			VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			poolInfo.queueFamilyIndex = VulkanContext::queueFamily();

			if (!WAX_VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &frame.pool), "vkCreateCommandPool"))
				return false;
		}

		// One descriptor set of push descriptors and one block of push constants for all
		// pipelines, as DX12Backend has one root signature.

		VkDescriptorSetLayoutBinding bindings[6] = {};

		const VkDescriptorType types[6] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
											VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_SAMPLER };

		for (uint32_t i = 0; i < 6; i++)
		{
			bindings[i].binding = i;
			bindings[i].descriptorType = types[i];
			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		VkDescriptorSetLayoutCreateInfo setInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		setInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		setInfo.bindingCount = 6;
		setInfo.pBindings = bindings;

		if (!WAX_VK_CHECK(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &m_descriptorSetLayout), "vkCreateDescriptorSetLayout"))
			return false;

		VkPushConstantRange range = {};
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		range.offset = 0;
		range.size = c_pushConstantsSize;

		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &m_descriptorSetLayout;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &range;

		if (!WAX_VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_pipelineLayout), "vkCreatePipelineLayout"))
			return false;

		// Shader modules are made when a pipeline first needs them, see _shaderModule().

		m_shaderModules.assign(VulkanShaders::nbShaders, VK_NULL_HANDLE);

		return _createSamplers();
	}

	//____ _destroyDeviceObjects() _______________________________________________

	void VulkanBackend::_destroyDeviceObjects()
	{
		VkDevice device = VulkanContext::device();
		if (!device)
			return;

		for (auto& entry : m_pipelines)
			if (entry.second)
				vkDestroyPipeline(device, entry.second, nullptr);

		m_pipelines.clear();
		m_activePipeline = VK_NULL_HANDLE;

		for (auto module : m_shaderModules)
			if (module)
				vkDestroyShaderModule(device, module, nullptr);

		m_shaderModules.clear();

		for (auto& sampler : m_samplers)
		{
			if (sampler)
				vkDestroySampler(device, sampler, nullptr);
			sampler = VK_NULL_HANDLE;
		}

		if (m_pipelineLayout)
			vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
		if (m_descriptorSetLayout)
			vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);

		m_pipelineLayout = VK_NULL_HANDLE;
		m_descriptorSetLayout = VK_NULL_HANDLE;

		for (auto& frame : m_frames)
		{
			if (frame.pool)
				vkDestroyCommandPool(device, frame.pool, nullptr);		// Frees its command buffers.

			frame.pool = VK_NULL_HANDLE;
			frame.cmdBuffers.clear();

			VulkanContext::destroyLater(frame.vertices);
			VulkanContext::destroyLater(frame.colors);
			VulkanContext::destroyLater(frame.extras);
		}

		m_buffers = FrameBuffers();
	}

	//____ _shaderModule() _______________________________________________________
	//
	// The module of a shader, by the name GpuBackend has for it, made the first time a pipeline
	// needs it. Null if there is no such shader, which is reported, or the module couldn't be made.

	VkShaderModule VulkanBackend::_shaderModule( const char * pName )
	{
		for (int i = 0; pName && i < VulkanShaders::nbShaders; i++)
		{
			const VulkanShaders::Shader& shader = VulkanShaders::shaders[i];

			if (std::strcmp(shader.pName, pName) != 0)
				continue;

			if (!m_shaderModules[i])
			{
				VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
				info.codeSize = shader.bytes;
				info.pCode = shader.pWords;

				WAX_VK_CHECK(vkCreateShaderModule(VulkanContext::device(), &info, nullptr, &m_shaderModules[i]), "vkCreateShaderModule");
			}

			return m_shaderModules[i];
		}

		char buffer[128];
		snprintf(buffer, sizeof(buffer), "Shader %s is missing, run compile_shaders.py.", pName ? pName : "without a name");
		Base::throwError(ErrorLevel::Error, ErrorCode::Internal, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return VK_NULL_HANDLE;
	}

	//____ _createSamplers() _____________________________________________________

	bool VulkanBackend::_createSamplers()
	{
		for (int i = 0; i < 8; i++)
		{
			bool bClip = (i & 4) != 0;
			bool bTiling = (i & 2) != 0;
			bool bBilinear = (i & 1) != 0;

			VkSamplerAddressMode addressMode = bTiling ? VK_SAMPLER_ADDRESS_MODE_REPEAT
										   : bClip ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			VkSamplerCreateInfo info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			info.magFilter = bBilinear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
			info.minFilter = bBilinear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
			info.mipmapMode = bBilinear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = addressMode;
			info.addressModeV = addressMode;
			info.addressModeW = addressMode;
			info.mipLodBias = 0.f;
			info.anisotropyEnable = VK_FALSE;
			info.maxAnisotropy = 1.f;
			info.compareEnable = VK_FALSE;
			info.minLod = 0.f;
			info.maxLod = VK_LOD_CLAMP_NONE;
			info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;		// Only used by the clip samplers.
			info.unnormalizedCoordinates = VK_FALSE;

			if (!WAX_VK_CHECK(vkCreateSampler(VulkanContext::device(), &info, nullptr, &m_samplers[i]), "vkCreateSampler"))
				return false;
		}

		return true;
	}

	//____ beginRender() _________________________________________________________

	void VulkanBackend::beginRender()
	{
		if (m_bRecording)
			return;

		// With no device to render with, nothing gets recorded this frame, and nothing is written
		// into buffers.

		if (!m_bDeviceObjectsOK || VulkanContext::isDeviceLost())
		{
			m_buffers = FrameBuffers();
			return;
		}

		m_frameIndex = (m_frameIndex + 1) % c_nbFrames;
		Frame& frame = m_frames[m_frameIndex];

		// The GPU must be done with the frame we are about to reuse.

		VulkanContext::wait(frame.submitted);
		VulkanContext::collectGarbage();

		vkResetCommandPool(VulkanContext::device(), frame.pool, 0);
		frame.nCmdBuffersUsed = 0;

		if (!_beginCommandBuffer())
		{
			m_buffers = FrameBuffers();
			return;
		}

		m_bRecording = true;
		VulkanContext::beginRecording();

		// Vertices, colors and extras are written for the whole frame, not per session. The
		// buffers may not exist yet, beginSession() creates or grows them as needed.

		m_buffers.pVertexBeg = (Vertex*) frame.vertices.pMapped;
		m_buffers.pVertexPtr = m_buffers.pVertexBeg;
		m_buffers.pVertexEnd = m_buffers.pVertexBeg ? m_buffers.pVertexBeg + frame.vertexCapacity : nullptr;

		m_buffers.pColorBeg = (Color*) frame.colors.pMapped;
		m_buffers.pColorPtr = m_buffers.pColorBeg;
		m_buffers.pColorEnd = m_buffers.pColorBeg ? m_buffers.pColorBeg + frame.colorCapacity : nullptr;

		m_buffers.pExtrasBeg = (Extras*) frame.extras.pMapped;
		m_buffers.pExtrasPtr = m_buffers.pExtrasBeg;
		m_buffers.pExtrasEnd = m_buffers.pExtrasBeg ? m_buffers.pExtrasBeg + frame.extrasCapacity : nullptr;
	}

	//____ endRender() ___________________________________________________________

	void VulkanBackend::endRender()
	{
		if (!m_bRecording)
		{
			m_waitSemaphores.clear();
			m_waitStages.clear();
			m_signalSemaphores.clear();
			return;
		}

		_endRendering();

		uint64_t submitted = _submitCommandBuffer();

		m_bRecording = false;
		VulkanContext::endRecording(submitted);

		m_waitSemaphores.clear();
		m_waitStages.clear();
		m_signalSemaphores.clear();
	}

	//____ addWaitSemaphore() ____________________________________________________

	void VulkanBackend::addWaitSemaphore( VkSemaphore semaphore, VkPipelineStageFlags stage )
	{
		m_waitSemaphores.push_back(semaphore);
		m_waitStages.push_back(stage);
	}

	//____ addSignalSemaphore() __________________________________________________

	void VulkanBackend::addSignalSemaphore( VkSemaphore semaphore )
	{
		m_signalSemaphores.push_back(semaphore);
	}

	//____ _beginCommandBuffer() _________________________________________________

	bool VulkanBackend::_beginCommandBuffer()
	{
		Frame& frame = m_frames[m_frameIndex];

		if (frame.nCmdBuffersUsed == int(frame.cmdBuffers.size()))
		{
			VkCommandBufferAllocateInfo alloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			alloc.commandPool = frame.pool;
			alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			alloc.commandBufferCount = 1;

			VkCommandBuffer cmd = VK_NULL_HANDLE;
			if (!WAX_VK_CHECK(vkAllocateCommandBuffers(VulkanContext::device(), &alloc, &cmd), "vkAllocateCommandBuffers"))
				return false;

			frame.cmdBuffers.push_back(cmd);
		}

		m_cmd = frame.cmdBuffers[frame.nCmdBuffersUsed++];

		VkCommandBufferBeginInfo begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (!WAX_VK_CHECK(vkBeginCommandBuffer(m_cmd, &begin), "vkBeginCommandBuffer"))
		{
			m_cmd = VK_NULL_HANDLE;
			return false;
		}

		// A new command buffer has none of the state of the last one.

		m_activePipeline = VK_NULL_HANDLE;
		m_bRendering = false;
		return true;
	}

	//____ _submitCommandBuffer() ________________________________________________

	uint64_t VulkanBackend::_submitCommandBuffer()
	{
		if (!m_cmd)
			return 0;

		VkCommandBuffer cmd = m_cmd;
		m_cmd = VK_NULL_HANDLE;

		if (!WAX_VK_CHECK(vkEndCommandBuffer(cmd), "vkEndCommandBuffer"))
			return 0;

		uint64_t value = VulkanContext::submit(cmd, int(m_waitSemaphores.size()), m_waitSemaphores.data(), m_waitStages.data(),
											   int(m_signalSemaphores.size()), m_signalSemaphores.data());

		if (value)
			m_frames[m_frameIndex].submitted = value;

		return value;
	}

	//____ _flush() ______________________________________________________________
	//
	// Submits what we have recorded so far and goes on recording into a new command buffer of the
	// same frame, with the state of the session put back. Called when a surface needs the GPU to
	// catch up with us before it does something of its own, see VulkanContext::beginOneShot().
	// Semaphores of the owner wait for endRender().

	void VulkanBackend::_flush()
	{
		if (!m_bRecording)
			return;

		_endRendering();

		std::vector<VkSemaphore> waits, signals;
		std::vector<VkPipelineStageFlags> stages;

		waits.swap(m_waitSemaphores);
		stages.swap(m_waitStages);
		signals.swap(m_signalSemaphores);

		_submitCommandBuffer();

		waits.swap(m_waitSemaphores);
		stages.swap(m_waitStages);
		signals.swap(m_signalSemaphores);

		if (!_beginCommandBuffer())
		{
			m_bRecording = false;
			VulkanContext::endRecording(0);
			return;
		}

		if (m_bInSession)
			_bindSessionState();
	}

	//____ _bindSessionState() ___________________________________________________
	//
	// Everything a session needs that a fresh command buffer doesn't have.

	void VulkanBackend::_bindSessionState()
	{
		if (!m_bRecording)
			return;

		Frame& frame = m_frames[m_frameIndex];

		if (frame.vertices.buffer)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(m_cmd, 0, 1, &frame.vertices.buffer, &offset);
		}

		if (frame.colors.buffer)
			_pushBuffer(Binding::Colors, frame.colors.buffer);

		if (frame.extras.buffer)
			_pushBuffer(Binding::Extras, frame.extras.buffer);

		_setConstants(ConstantBlock::Canvas);
		_setConstants(ConstantBlock::Pixel);
		_setConstants(ConstantBlock::Warp);

		if (m_target.view)
		{
			VkViewport viewport = { 0.f, 0.f, float(m_target.size.w), float(m_target.size.h), 0.f, 1.f };
			vkCmdSetViewport(m_cmd, 0, 1, &viewport);
			_resetScissor();
		}

		m_activePipeline = VK_NULL_HANDLE;
		m_bBlitSourceBound = false;
	}

	//____ _isRecording() ________________________________________________________

	bool VulkanBackend::_isRecording() const
	{
		return m_bRecording && !VulkanContext::isDeviceLost();
	}

	//____ _isNativeSurface() ____________________________________________________

	bool VulkanBackend::_isNativeSurface( const Object * pObject ) const
	{
		return pObject && _isOfType(pObject->typeInfo(), VulkanSurface::TYPEINFO);
	}

	//____ _isNativeEdgemap() ____________________________________________________

	bool VulkanBackend::_isNativeEdgemap( const Object * pObject ) const
	{
		return pObject && _isOfType(pObject->typeInfo(), VulkanEdgemap::TYPEINFO);
	}

	//____ _isNativeWarpmap() ____________________________________________________

	bool VulkanBackend::_isNativeWarpmap( const Object * pObject ) const
	{
		return pObject && _isOfType(pObject->typeInfo(), VulkanWarpmap::TYPEINFO);
	}

	//____ _beginSessionOnDevice() _______________________________________________

	void VulkanBackend::_beginSessionOnDevice( Surface * pCanvas )
	{
		m_bDefaultCanvasInUse = false;
		m_pTargetSurface = nullptr;
		m_target = Target();
	}

	//____ _endSessionOnDevice() _________________________________________________

	void VulkanBackend::_endSessionOnDevice()
	{
		_endRendering();
		_releaseCanvas();
	}

	//____ _releaseCanvas() ______________________________________________________
	//
	// Canvases go back to rest when we are done with them: surfaces where they can be read as blit
	// source, the default canvas where its owner wants it.

	void VulkanBackend::_releaseCanvas()
	{
		_endRendering();

		if (m_pTargetSurface)
		{
			VulkanContext::transition(m_cmd, m_pTargetSurface->image(), m_pTargetSurface->layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_pTargetSurface->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_pTargetSurface = nullptr;
		}

		if (m_bDefaultCanvasInUse)
		{
			VulkanContext::transition(m_cmd, m_defaultImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, m_defaultRestingLayout);
			m_bDefaultCanvasInUse = false;
		}

		m_target = Target();
	}

	//____ _reserveFrameBuffers() ________________________________________________
	//
	// If what is left of a buffer is too little, it is replaced by a bigger one and writing starts
	// over at its beginning. Draws recorded so far keep reading the old one, which is released once
	// the GPU is done with them. Buffers at least double when they grow, so a frame settles on a
	// size after a few frames rather than growing a little every session.

	bool VulkanBackend::_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects )
	{
		Frame& frame = m_frames[m_frameIndex];

		auto reserve = [](VulkanContext::Buffer& buffer, int& capacity, auto*& pBeg, auto*& pPtr, auto*& pEnd, int needed, VkBufferUsageFlags usage) -> bool
		{
			typedef typename std::remove_reference<decltype(*pBeg)>::type Entry;

			if (needed < 1)
				needed = 1;

			if (pPtr && pEnd - pPtr >= needed)
				return true;

			int newCapacity = std::max(capacity * 2, needed);

			VulkanContext::Buffer newBuffer;
			if (!VulkanContext::createBuffer(VkDeviceSize(newCapacity) * sizeof(Entry), usage, VulkanContext::Memory::HostWrite, newBuffer))
				return false;

			VulkanContext::destroyLater(buffer);

			buffer = newBuffer;
			capacity = newCapacity;

			pBeg = (Entry*) newBuffer.pMapped;
			pPtr = pBeg;
			pEnd = pBeg + newCapacity;
			return true;
		};

		bool bOK = reserve(frame.vertices, frame.vertexCapacity, m_buffers.pVertexBeg, m_buffers.pVertexPtr, m_buffers.pVertexEnd, nVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		bOK = reserve(frame.colors, frame.colorCapacity, m_buffers.pColorBeg, m_buffers.pColorPtr, m_buffers.pColorEnd, nColors, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) && bOK;
		bOK = reserve(frame.extras, frame.extrasCapacity, m_buffers.pExtrasBeg, m_buffers.pExtrasPtr, m_buffers.pExtrasEnd, nExtras, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) && bOK;

		_bindSessionState();
		return bOK;
	}

	//____ _bindCanvas() _________________________________________________________

	bool VulkanBackend::_bindCanvas( Surface * pCanvas, SizeI& size, CanvasKind& kind )
	{
		if (!m_bRecording)
			return false;

		VulkanSurface * pSurface = static_cast<VulkanSurface*>(pCanvas);

		if (pSurface && (!pSurface->image() || !pSurface->attachmentView()))
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Surface can not be used as canvas, rendering to default canvas instead.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			pSurface = nullptr;
		}

		// Whatever we rendered into before is done now, unless it is the default canvas, which
		// stays ready until the session ends.

		_endRendering();

		if (m_pTargetSurface && m_pTargetSurface != pSurface)
		{
			VulkanContext::transition(m_cmd, m_pTargetSurface->image(), m_pTargetSurface->layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_pTargetSurface->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		m_pTargetSurface = nullptr;
		m_target = Target();
		m_activePipeline = VK_NULL_HANDLE;		// Pipelines are made for the format of the canvas.

		if (pSurface)
		{
			// Gets any pixels the CPU wrote into the texture before we render over it.

			_recordUploads(pSurface);
			pSurface->notifyRendered();

			if (!m_bRecording)
				return false;

			VulkanContext::transition(m_cmd, pSurface->image(), pSurface->layout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			pSurface->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			m_pTargetSurface = pSurface;
			m_target.image = pSurface->image();
			m_target.view = pSurface->attachmentView();
			m_target.format = pSurface->vkFormat();
			m_target.size = pSurface->pixelSize();

			kind = _canvasKind(pSurface);
		}
		else
		{
			if (!m_defaultImage || !m_defaultView)
				return false;

			if (!m_bDefaultCanvasInUse)
			{
				VulkanContext::transition(m_cmd, m_defaultImage, m_defaultRestingLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				m_bDefaultCanvasInUse = true;
			}

			m_target.image = m_defaultImage;
			m_target.view = m_defaultView;
			m_target.format = m_defaultFormat;
			m_target.size = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };

			kind = CanvasKind::WithAlpha;
		}

		size = m_target.size;

		VkViewport viewport = { 0.f, 0.f, float(size.w), float(size.h), 0.f, 1.f };
		vkCmdSetViewport(m_cmd, 0, 1, &viewport);
		_resetScissor();

		return true;
	}

	//____ _beginRendering() _____________________________________________________

	void VulkanBackend::_beginRendering()
	{
		if (m_bRendering || !m_bRecording || !m_target.view)
			return;

		VkRenderingAttachmentInfo color = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		color.imageView = m_target.view;
		color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;			// Only the update rects are drawn, the rest stays.
		color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo info = { VK_STRUCTURE_TYPE_RENDERING_INFO };
		info.renderArea = { { 0, 0 }, { uint32_t(m_target.size.w), uint32_t(m_target.size.h) } };
		info.layerCount = 1;
		info.colorAttachmentCount = 1;
		info.pColorAttachments = &color;

		vkCmdBeginRendering(m_cmd, &info);
		m_bRendering = true;
	}

	//____ _endRendering() _______________________________________________________

	void VulkanBackend::_endRendering()
	{
		if (!m_bRendering)
			return;

		vkCmdEndRendering(m_cmd);
		m_bRendering = false;
	}

	//____ _bindPipeline()_______________________________________________________
	//
	// Pipelines are created the first time a combination of canvas format, kind and blend mode
	// shows up.

	bool VulkanBackend::_bindPipeline( PipelineKind kind, BlendMode blendMode )
	{
		if (!m_bRecording || !m_target.view)
			return false;

		uint64_t key = _pipelineKey(m_target.format, m_activeCanvasKind, kind, blendMode);

		auto it = m_pipelines.find(key);
		if (it == m_pipelines.end())
			it = m_pipelines.emplace(key, _createPipeline(kind, _blendState(kind, blendMode, m_activeCanvasKind), m_target.format)).first;		// Also when null, so a failure is only tried once.

		VkPipeline pipeline = it->second;

		if (!pipeline)
			return false;

		if (pipeline != m_activePipeline)
		{
			vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			m_activePipeline = pipeline;
		}

		return true;
	}

	//____ _createPipeline() _____________________________________________________
	//
	// Blending is that of GpuBackend::_blendState(), in Vulkan terms. An alpha only canvas is R8
	// here rather than A8, see VulkanSurface.

	VkPipeline VulkanBackend::_createPipeline( PipelineKind kind, const BlendState& blend, VkFormat format )
	{
		static const VkBlendFactor factors[] = {								// In the order of BlendFactor.
			VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
			VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, VK_BLEND_FACTOR_SRC1_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA };

		static const VkBlendOp ops[] = {										// In the order of BlendOp.
			VK_BLEND_OP_ADD, VK_BLEND_OP_REVERSE_SUBTRACT, VK_BLEND_OP_MIN, VK_BLEND_OP_MAX };

		static_assert(sizeof(factors) / sizeof(factors[0]) == size_t(BlendFactor::Size) && sizeof(ops) / sizeof(ops[0]) == size_t(BlendOp::Size),
					  "A Vulkan value for each blend factor and operation");

		VkPipelineColorBlendAttachmentState attachment = {};
		attachment.blendEnable = blend.bEnable ? VK_TRUE : VK_FALSE;
		attachment.colorBlendOp = ops[int(blend.colorOp)];
		attachment.srcColorBlendFactor = factors[int(blend.srcColor)];
		attachment.dstColorBlendFactor = factors[int(blend.dstColor)];
		attachment.alphaBlendOp = ops[int(blend.alphaOp)];
		attachment.srcAlphaBlendFactor = factors[int(blend.srcAlpha)];
		attachment.dstAlphaBlendFactor = factors[int(blend.dstAlpha)];

		attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

		if (blend.bWriteAlpha)
			attachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo blendInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blendInfo.attachmentCount = 1;
		blendInfo.pAttachments = &attachment;

		// Shaders.

		VkShaderModule vertexShader = _shaderModule(_vertexShaderName(kind));
		VkShaderModule pixelShader = _shaderModule(_pixelShaderName(kind, blend.bDualSource));

		if (!vertexShader || !pixelShader)
			return VK_NULL_HANDLE;

		VkPipelineShaderStageCreateInfo stages[2] = {};

		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = vertexShader;
		stages[0].pName = "main";

		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = pixelShader;
		stages[1].pName = "main";

		// One vertex layout for all but warps, whose vertex shader reads the vertices of the
		// Warpmap itself. Only the attributes the vertex shader reads, see _nbVertexAttributes(),
		// which the validation layers would otherwise complain about.

		VkVertexInputBindingDescription vertexBinding = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

		VkVertexInputAttributeDescription attributes[3] = {
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, x) },
			{ 1, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, colorOfs) },
			{ 2, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, extrasOfs) }
		};

		uint32_t nAttributes = uint32_t(_nbVertexAttributes(kind));

		VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInput.vertexBindingDescriptionCount = nAttributes > 0 ? 1 : 0;
		vertexInput.pVertexBindingDescriptions = &vertexBinding;
		vertexInput.vertexAttributeDescriptionCount = nAttributes;
		vertexInput.pVertexAttributeDescriptions = attributes;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewport = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewport.viewportCount = 1;
		viewport.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo raster = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.cullMode = VK_CULL_MODE_NONE;
		raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
		raster.lineWidth = 1.f;

		VkPipelineMultisampleStateCreateInfo multisample = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamic = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamicStates;

		VkPipelineRenderingCreateInfo rendering = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		rendering.colorAttachmentCount = 1;
		rendering.pColorAttachmentFormats = &format;

		VkGraphicsPipelineCreateInfo info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		info.pNext = &rendering;
		info.stageCount = 2;
		info.pStages = stages;
		info.pVertexInputState = &vertexInput;
		info.pInputAssemblyState = &inputAssembly;
		info.pViewportState = &viewport;
		info.pRasterizationState = &raster;
		info.pMultisampleState = &multisample;
		info.pColorBlendState = &blendInfo;
		info.pDynamicState = &dynamic;
		info.layout = m_pipelineLayout;

		VkPipeline pipeline = VK_NULL_HANDLE;

		if (!WAX_VK_CHECK(vkCreateGraphicsPipelines(VulkanContext::device(), VK_NULL_HANDLE, 1, &info, nullptr, &pipeline), "vkCreateGraphicsPipelines"))
			return VK_NULL_HANDLE;

		return pipeline;
	}

	//____ _setConstants() _______________________________________________________

	void VulkanBackend::_setConstants( ConstantBlock block )
	{
		if (!m_bRecording || !m_cmd)
			return;

		static_assert(sizeof(CanvasConstants) == 32 && sizeof(PixelConstants) == 16, "Constant blocks must match constants.hlsl");

		VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		switch (block)
		{
			case ConstantBlock::Canvas:
				vkCmdPushConstants(m_cmd, m_pipelineLayout, stages, c_canvasConstantsOfs, sizeof(CanvasConstants), &m_canvasConstants);
				break;

			case ConstantBlock::Pixel:
				vkCmdPushConstants(m_cmd, m_pipelineLayout, stages, c_pixelConstantsOfs, sizeof(PixelConstants), &m_pixelConstants);
				break;

			case ConstantBlock::Warp:
				vkCmdPushConstants(m_cmd, m_pipelineLayout, stages, c_warpConstantsOfs, c_warpConstantsSize, &m_warpConstants);
				break;
		}
	}

	//____ _draw() _______________________________________________________________

	void VulkanBackend::_draw( int firstVertex, int nVertices )
	{
		if (!m_bRecording || !m_target.view || nVertices <= 0)
			return;

		_beginRendering();
		vkCmdDraw(m_cmd, uint32_t(nVertices), 1, uint32_t(firstVertex), 0);
	}

	//____ _setScissor() _________________________________________________________

	void VulkanBackend::_setScissor( const RectI& rect )
	{
		if (!m_bRecording)
			return;

		// Vulkan doesn't take scissors reaching outside the canvas.

		int x1 = std::max(rect.x, 0);
		int y1 = std::max(rect.y, 0);
		int x2 = std::min(rect.x + rect.w, m_target.size.w);
		int y2 = std::min(rect.y + rect.h, m_target.size.h);

		VkRect2D scissor = { { x1, y1 }, { uint32_t(std::max(x2 - x1, 0)), uint32_t(std::max(y2 - y1, 0)) } };
		vkCmdSetScissor(m_cmd, 0, 1, &scissor);
	}

	//____ _resetScissor() _______________________________________________________

	void VulkanBackend::_resetScissor()
	{
		if (!m_bRecording)
			return;

		VkRect2D scissor = { { 0, 0 }, { uint32_t(m_target.size.w), uint32_t(m_target.size.h) } };
		vkCmdSetScissor(m_cmd, 0, 1, &scissor);
	}

	//____ _pushBuffer() _________________________________________________________

	void VulkanBackend::_pushBuffer( uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range )
	{
		VkDescriptorBufferInfo info = { buffer, offset, range };

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pBufferInfo = &info;

		m_vkCmdPushDescriptorSet(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &write);
	}

	//____ _pushImage() __________________________________________________________

	void VulkanBackend::_pushImage( uint32_t binding, VkImageView view )
	{
		VkDescriptorImageInfo info = { VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write.pImageInfo = &info;

		m_vkCmdPushDescriptorSet(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &write);
	}

	//____ _pushSampler() ________________________________________________________

	void VulkanBackend::_pushSampler( uint32_t binding, VkSampler sampler )
	{
		VkDescriptorImageInfo info = { sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };

		VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &info;

		m_vkCmdPushDescriptorSet(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &write);
	}

	//____ _prepareBlitSource() __________________________________________________

	void VulkanBackend::_prepareBlitSource( Surface * pSurface )
	{
		_recordUploads(static_cast<VulkanSurface*>(pSurface));
	}

	//____ _recordUploads() ______________________________________________________
	//
	// Has what the CPU has written to a surface copied to its texture, in our command buffer, so
	// in front of what we record from here on and with no wait. Copies can't be inside a render
	// pass, which the next draw begins again.
	//
	// Recorded but unsubmitted work of other backends might read the texture. It goes in front
	// of our copies, as it would have had it been submitted already.

	void VulkanBackend::_recordUploads( VulkanSurface * pSurface )
	{
		if (!m_bRecording || !pSurface->hasUploads())
			return;

		_endRendering();

		VulkanContext::flushBackends(this);

		pSurface->recordUploads(m_cmd);
	}

	//____ _bindBlitTexture() ____________________________________________________

	bool VulkanBackend::_bindBlitTexture( Surface * pSurface )
	{
		auto pSource = static_cast<VulkanSurface*>(pSurface);

		if (!m_bRecording || !pSource->image() || !pSource->sampleView())
			return false;

		// Mip levels that no longer match level 0 are made anew before anything reads them.

		if (pSource->isMipmapStale())
		{
			_endRendering();
			_generateMipmaps(pSource);
		}

		if (pSource->layout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			_endRendering();
			VulkanContext::transition(m_cmd, pSource->image(), pSource->layout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			pSource->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		_pushImage(Binding::BlitSource, pSource->sampleView());

		// A palette based source has its palette where edgemaps go, which is free while we blit.
		// Drawing an edgemap clears m_bBlitSourceBound, so it is put back afterwards.

		if (pSource->isIndexed())
		{
			if (!pSource->paletteBuffer())
				return false;

			_pushBuffer(Binding::EdgemapOrPalette, pSource->paletteBuffer());
		}

		return true;
	}

	//____ _bindBlitSampler() ____________________________________________________

	void VulkanBackend::_bindBlitSampler( Surface * pSurface, bool bClip )
	{
		if (!m_bRecording)
			return;

		int index = (bClip ? 4 : 0) + (pSurface->isTiling() ? 2 : 0) + (pSurface->sampleMethod() == SampleMethod::Bilinear ? 1 : 0);

		_pushSampler(Binding::Sampler, m_samplers[index]);
	}

	//____ _bindEdgemap() ________________________________________________________

	bool VulkanBackend::_bindEdgemap( GpuEdgemap * pEdgemap )
	{
		auto pVulkanEdgemap = static_cast<VulkanEdgemap*>(pEdgemap);

		if (!m_bRecording || !pVulkanEdgemap->_buffer())
			return false;

		_pushBuffer(Binding::EdgemapOrPalette, pVulkanEdgemap->_buffer());
		return true;
	}

	//____ _drawWarpmap() ________________________________________________________

	void VulkanBackend::_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches )
	{
		auto pVulkanWarpmap = static_cast<VulkanWarpmap*>(pWarpmap);

		if (!m_bRecording || !m_target.view || !pVulkanWarpmap->m_buffer.buffer || pWarpmap->triangleCount() == 0)
			return;

		_pushBuffer(Binding::WarpVertices, pVulkanWarpmap->m_buffer.buffer);
		vkCmdBindIndexBuffer(m_cmd, pVulkanWarpmap->m_buffer.buffer, VkDeviceSize(pVulkanWarpmap->m_indexOffset), VK_INDEX_TYPE_UINT16);

		_beginRendering();

		for (int i = 0; i < nPatches; i++)
		{
			_setScissor(pPatches[i]);
			vkCmdDrawIndexed(m_cmd, uint32_t(pWarpmap->triangleCount() * 3), 1, 0, 0, 0);
		}

		_resetScissor();
	}

	//____ _generateMipmaps() ____________________________________________________
	//
	// Makes each mip level from the one above it with a linear blit, which averages the four
	// texels under each, as the mipmap shaders of DX12Backend do. Outside of rendering.

	void VulkanBackend::_generateMipmaps( VulkanSurface * pSurface )
	{
		int nLevels = pSurface->mipLevels();
		VkImage image = pSurface->image();

		if (nLevels < 2 || !image)
		{
			pSurface->clearMipmapStale();
			return;
		}

		VulkanContext::transition(m_cmd, image, pSurface->layout(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1);
		VulkanContext::transition(m_cmd, image, pSurface->layout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, uint32_t(nLevels - 1));

		SizeI size = pSurface->pixelSize();

		for (int level = 1; level < nLevels; level++)
		{
			int srcW = std::max(size.w >> (level - 1), 1), srcH = std::max(size.h >> (level - 1), 1);
			int dstW = std::max(size.w >> level, 1), dstH = std::max(size.h >> level, 1);

			VkImageBlit blit = {};
			blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, uint32_t(level - 1), 0, 1 };
			blit.srcOffsets[1] = { srcW, srcH, 1 };
			blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, uint32_t(level), 0, 1 };
			blit.dstOffsets[1] = { dstW, dstH, 1 };

			vkCmdBlitImage(m_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			VulkanContext::transition(m_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, uint32_t(level), 1);
		}

		VulkanContext::transition(m_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		pSurface->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		pSurface->clearMipmapStale();
	}

	//____ setDefaultCanvas() ____________________________________________________

	bool VulkanBackend::setDefaultCanvas( VkImage image, VkImageView view, VkFormat format, VkImageLayout restingLayout, SizeSPX size, int scale )
	{
		m_defaultImage = image;
		m_defaultView = view;
		m_defaultFormat = format;
		m_defaultRestingLayout = restingLayout;

		m_defaultCanvas.ref = CanvasRef::Default;
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;

		// Wax has no name for R8G8B8A8, so it is reported as ARGB_8. Channel order is the one thing
		// about the default canvas nobody can ask us, and the color space, which they can, is then
		// right.

		switch (format)
		{
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_UNORM:
				m_defaultCanvas.format = PixelFormat::ARGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::Linear;
				break;

			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_R8G8B8A8_SRGB:
				m_defaultCanvas.format = PixelFormat::ARGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::sRGB;
				break;

			default:
				m_defaultCanvas.format = PixelFormat::Undefined;
				break;
		}

		return true;
	}

	//____ canvasInfo() __________________________________________________________

	const CanvasInfo* VulkanBackend::canvasInfo(CanvasRef ref) const
	{
		if (ref == CanvasRef::Default)
			return &m_defaultCanvas;

		Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only Default canvas is supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return &m_dummyCanvas;
	}

	//____ surfaceFactory() ______________________________________________________

	SurfaceFactory_p VulkanBackend::surfaceFactory()
	{
		return m_pSurfaceFactory;
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p VulkanBackend::edgemapFactory()
	{
		return m_pEdgemapFactory;
	}

	//____ warpmapFactory() ______________________________________________________

	WarpmapFactory_p VulkanBackend::warpmapFactory()
	{
		return m_pWarpmapFactory;
	}

	//____ canBeBlitSource() _____________________________________________________

	bool VulkanBackend::canBeBlitSource(const TypeInfo& type) const
	{
		return _isOfType(type, VulkanSurface::TYPEINFO);
	}

	//____ canBeCanvas() _________________________________________________________

	bool VulkanBackend::canBeCanvas(const TypeInfo& type) const
	{
		return _isOfType(type, VulkanSurface::TYPEINFO);
	}

	//____ waitForCompletion() ___________________________________________________

	void VulkanBackend::waitForCompletion()
	{
		// Waiting for work that is still being recorded would mean nothing, so what we have goes
		// to the GPU first.

		_flush();

		for (auto& frame : m_frames)
			VulkanContext::wait(frame.submitted);
	}

} // namespace wax
