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

		return _createShaderModules() && _createSamplers();
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

	//____ _createShaderModules() ________________________________________________

	bool VulkanBackend::_createShaderModules()
	{
		using namespace VulkanShaders;

		auto module = [this](const Code& code) -> VkShaderModule
		{
			VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			info.codeSize = code.bytes;
			info.pCode = code.pWords;

			VkShaderModule handle = VK_NULL_HANDLE;
			if (!WAX_VK_CHECK(vkCreateShaderModule(VulkanContext::device(), &info, nullptr, &handle), "vkCreateShaderModule"))
				return VK_NULL_HANDLE;

			m_shaderModules.push_back(handle);
			return handle;
		};

		struct { PipelineKind kind; const Code* pVS; const Code* pPS; const Code* pDualPS; } shaders[] =
		{
			{ PipelineKind::Fill,				&fillVS,		&fillPS,			&fillPSDual },
			{ PipelineKind::FillAA,				&fillAAVS,		&fillAAPS,			&fillAAPSDual },
			{ PipelineKind::Blit,				&blitVS,		&blitPS,			&blitPSDual },
			{ PipelineKind::Blur,				&blitVS,		&blurPS,			&blurPSDual },		// Same geometry, so the same vertex shader.
			{ PipelineKind::PaletteBlit,		&blitVS,		&paletteBlitPS,		&paletteBlitPSDual },
			{ PipelineKind::PaletteBlur,		&blitVS,		&paletteBlurPS,		&paletteBlurPSDual },
			{ PipelineKind::Line,				&lineVS,		&linePS,			&linePSDual },
			{ PipelineKind::Segments,			&segmentsVS,	&segmentsPS,		&segmentsPSDual },
			{ PipelineKind::Shape,				&shapeVS,		&shapePS,			&shapePSDual },
			{ PipelineKind::WarpEdgemap,		&warpVS,		&warpEdgemapPS,		&warpEdgemapPSDual },
			{ PipelineKind::WarpBlit,			&warpVS,		&warpBlitPS,		&warpBlitPSDual },
			{ PipelineKind::WarpPaletteBlit,	&warpVS,		&warpPaletteBlitPS,	&warpPaletteBlitPSDual },
		};

		std::map<const Code*, VkShaderModule> vertexModules;		// Shared by kinds with the same vertex shader.

		for (auto& shader : shaders)
		{
			int i = int(shader.kind);

			auto it = vertexModules.find(shader.pVS);
			m_vertexShaders[i] = it != vertexModules.end() ? it->second : (vertexModules[shader.pVS] = module(*shader.pVS));
			m_pixelShaders[i] = module(*shader.pPS);
			m_dualSourcePixelShaders[i] = module(*shader.pDualPS);

			if (!m_vertexShaders[i] || !m_pixelShaders[i] || !m_dualSourcePixelShaders[i])
				return false;
		}

		return true;
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
			// Gets any pixels the CPU wrote into the texture before we render over it, and marks
			// our copy of them as the older one from here on. That can submit what we have
			// recorded and continue in a new command buffer.

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

	//____ _setPipeline() ________________________________________________________

	bool VulkanBackend::_setPipeline( PipelineKind kind )
	{
		if (!m_bRecording || !m_target.view)
			return false;

		VkPipeline pipeline = _pipeline(kind, m_activeBlendMode);
		if (!pipeline)
			return false;

		if (pipeline != m_activePipeline)
		{
			vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			m_activePipeline = pipeline;
		}

		return true;
	}

	//____ _pipeline() ___________________________________________________________
	//
	// The pipeline for the given kind and blend mode on the canvas we are rendering into, created
	// the first time a combination shows up.

	VkPipeline VulkanBackend::_pipeline( PipelineKind kind, BlendMode blendMode )
	{
		if (blendMode == BlendMode::Ignore)
			return VK_NULL_HANDLE;						// Nothing should be drawn.

		BlendMode mode = _normalizeBlendMode(blendMode);

		uint64_t key = (uint64_t(m_target.format) << 24) | (uint64_t(m_activeCanvasKind) << 16) | (uint64_t(kind) << 8) | uint64_t(mode);

		auto it = m_pipelines.find(key);
		if (it != m_pipelines.end())
			return it->second;							// May be null, if creation failed before.

		VkPipeline pipeline = _createPipeline(kind, mode, m_target.format, m_activeCanvasKind);
		m_pipelines[key] = pipeline;
		return pipeline;
	}

	//____ _createPipeline() _____________________________________________________
	//
	// Blending as DX12Backend::_createPipeline() has it, see there for why. Canvases with alpha
	// hold premultiplied colors. The pixel shaders pass what they draw to the blender as the mode
	// needs it, see output.hlsl, Replace, Morph and Multiply keeping the uncovered part of the pixel
	// by the second color of dual-source blending.
	//
	// An alpha only canvas is R8 here rather than A8, see VulkanSurface. The shaders write the value
	// to all channels for it, so the factors of alpha are those of color.

	VkPipeline VulkanBackend::_createPipeline( PipelineKind kind, BlendMode blendMode, VkFormat format, CanvasKind canvasKind )
	{
		bool bAlphaOnly = (canvasKind == CanvasKind::AlphaOnly);
		bool bNoAlpha = (canvasKind == CanvasKind::WithoutAlpha);
		bool bPartialCoverage = _hasPartialCoverage(kind);

		VkBool32		bBlend = VK_TRUE;
		VkBlendOp		op = VK_BLEND_OP_ADD;
		VkBlendFactor	srcBlend = VK_BLEND_FACTOR_ONE;
		VkBlendFactor	destBlend = VK_BLEND_FACTOR_ZERO;
		bool			bKeepAlpha = false;
		bool			bInvertAlpha = false;

		switch (blendMode)
		{
			case BlendMode::Replace:
				srcBlend = VK_BLEND_FACTOR_ONE;
				destBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
				bBlend = bPartialCoverage ? VK_TRUE : VK_FALSE;
				break;

			case BlendMode::Blend:
				srcBlend = VK_BLEND_FACTOR_ONE;
				destBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;

			case BlendMode::Morph:
				srcBlend = VK_BLEND_FACTOR_ONE;
				destBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
				break;

			case BlendMode::Add:
			case BlendMode::Subtract:
				op = (blendMode == BlendMode::Add) ? VK_BLEND_OP_ADD : VK_BLEND_OP_REVERSE_SUBTRACT;
				srcBlend = VK_BLEND_FACTOR_ONE;
				destBlend = VK_BLEND_FACTOR_ONE;
				bKeepAlpha = (blendMode == BlendMode::Subtract);
				break;

			case BlendMode::Multiply:
				srcBlend = bAlphaOnly ? VK_BLEND_FACTOR_DST_ALPHA : VK_BLEND_FACTOR_DST_COLOR;
				destBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
				bKeepAlpha = true;
				break;

			case BlendMode::Invert:
				srcBlend = bAlphaOnly ? VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
				destBlend = bAlphaOnly ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
				bInvertAlpha = !bAlphaOnly;
				break;

			case BlendMode::Min:
			case BlendMode::Max:
				op = (blendMode == BlendMode::Min) ? VK_BLEND_OP_MIN : VK_BLEND_OP_MAX;
				srcBlend = VK_BLEND_FACTOR_ONE;
				destBlend = VK_BLEND_FACTOR_ONE;
				bKeepAlpha = (blendMode == BlendMode::Min);
				break;

			default:
			{
				char buffer[128];
				snprintf(buffer, sizeof(buffer), "BlendMode %d is unknown, using Blend.", (int) blendMode);
				Base::throwError(ErrorLevel::Warning, ErrorCode::Other, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

				srcBlend = VK_BLEND_FACTOR_ONE;
				destBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			}
		}

		// The value of an alpha only canvas is in R, as are the alphas the shaders output for it.

		if (bAlphaOnly)
		{
			auto toColor = [](VkBlendFactor factor)
			{
				switch (factor)
				{
					case VK_BLEND_FACTOR_SRC_ALPHA:				return VK_BLEND_FACTOR_SRC_COLOR;
					case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
					case VK_BLEND_FACTOR_DST_ALPHA:				return VK_BLEND_FACTOR_DST_COLOR;
					case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
					case VK_BLEND_FACTOR_SRC1_ALPHA:			return VK_BLEND_FACTOR_SRC1_COLOR;
					case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
					default:									return factor;
				}
			};

			srcBlend = toColor(srcBlend);
			destBlend = toColor(destBlend);
			bKeepAlpha = false;
		}

		bool bDualSource = bBlend && (destBlend == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA || destBlend == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR);

		VkPipelineColorBlendAttachmentState attachment = {};
		attachment.blendEnable = bBlend;
		attachment.colorBlendOp = op;
		attachment.srcColorBlendFactor = srcBlend;
		attachment.dstColorBlendFactor = destBlend;

		if (bKeepAlpha)
		{
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		}
		else if (bInvertAlpha)
		{
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		}
		else
		{
			// Alpha can't take color factors, but none of these is one, except on an alpha only
			// canvas, which has no alpha.

			auto toAlpha = [](VkBlendFactor factor)
			{
				switch (factor)
				{
					case VK_BLEND_FACTOR_SRC_COLOR:				return VK_BLEND_FACTOR_SRC_ALPHA;
					case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					case VK_BLEND_FACTOR_DST_COLOR:				return VK_BLEND_FACTOR_DST_ALPHA;
					case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
					case VK_BLEND_FACTOR_SRC1_COLOR:			return VK_BLEND_FACTOR_SRC1_ALPHA;
					case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
					default:									return factor;
				}
			};

			attachment.alphaBlendOp = op;
			attachment.srcAlphaBlendFactor = toAlpha(srcBlend);
			attachment.dstAlphaBlendFactor = toAlpha(destBlend);
		}

		// A canvas without alpha has an alpha channel here, which is left alone. Reading it as a
		// blit source gives alpha 1 regardless, see VulkanSurface.

		attachment.colorWriteMask = bNoAlpha ? (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT)
											 : (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

		VkPipelineColorBlendStateCreateInfo blend = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blend.attachmentCount = 1;
		blend.pAttachments = &attachment;

		// Shaders.

		VkPipelineShaderStageCreateInfo stages[2] = {};

		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = m_vertexShaders[int(kind)];
		stages[0].pName = "main";

		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = bDualSource ? m_dualSourcePixelShaders[int(kind)] : m_pixelShaders[int(kind)];
		stages[1].pName = "main";

		// One vertex layout for all but warps, whose vertex shader reads the vertices of the
		// Warpmap itself. Shaders that read fewer attributes leave the last ones unused, see
		// ../hlsl/README.md.

		VkVertexInputBindingDescription vertexBinding = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

		VkVertexInputAttributeDescription attributes[4] = {
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, x) },
			{ 1, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, colorOfs) },
			{ 2, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, extrasOfs) },
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, u) }
		};

		// Only the attributes the vertex shader reads, which the validation layers would otherwise
		// complain about.

		uint32_t nAttributes;

		switch (kind)
		{
			case PipelineKind::Fill:		nAttributes = 2; break;
			case PipelineKind::Segments:	nAttributes = 4; break;
			default:						nAttributes = 3; break;
		}

		bool bWarp = _isWarp(kind);

		VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInput.vertexBindingDescriptionCount = bWarp ? 0 : 1;
		vertexInput.pVertexBindingDescriptions = &vertexBinding;
		vertexInput.vertexAttributeDescriptionCount = bWarp ? 0 : nAttributes;
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
		info.pColorBlendState = &blend;
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
		static_cast<VulkanSurface*>(pSurface)->syncTexture();
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
