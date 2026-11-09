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
#ifndef WAX_VULKANBACKEND_DOT_H
#define WAX_VULKANBACKEND_DOT_H
#pragma once

#include <wax_gpubackend.h>
#include <wax_vulkancontext.h>

#include <map>
#include <vector>

namespace wax
{
	class VulkanSurface;

	class VulkanBackend;
	typedef	StrongPtr<VulkanBackend>	VulkanBackend_p;
	typedef	WeakPtr<VulkanBackend>		VulkanBackend_wp;

	//____ VulkanBackend ________________________________________________________
	/**
	 * @brief Backend rendering with Vulkan 1.3.
	 *
	 * Draws with the HLSL shaders DX12Backend has, compiled to SPIR-V, and is built on GpuBackend,
	 * which turns the commands into what the shaders read. This class does what needs Vulkan: render
	 * targets through dynamic rendering, pipelines, binding resources with push descriptors and
	 * recording draws.
	 *
	 * Any number of backends can exist at the same time, e.g. one per window, but they all share
	 * one device and one queue, set with setDevice() before the first backend or surface is
	 * created. The device must be Vulkan 1.3 with these enabled:
	 *
	 *   Features:		dynamicRendering, synchronization2, shaderDemoteToHelperInvocation,
	 *					timelineSemaphore (core in 1.3, but they have to be turned on) and
	 *					dualSrcBlend.
	 *   Extensions:	VK_KHR_push_descriptor.
	 *
	 * Rendering happens between beginRender() and endRender(), which submits what has been
	 * recorded. Two frames can be in flight.
	 */

	class VulkanBackend : public GpuBackend
	{
		friend class VulkanContext;

	public:

		//.____ Creation __________________________________________

		static VulkanBackend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		//.____ Device _______________________________________________________

		// Sets the device and queue shared by all backends, surfaces, edgemaps and Warpmaps.
		// Must be called before any of them is created. setDevice() with a null device lets go of
		// it, which is only allowed once they are all gone. Replacing a device that is in use isn't
		// supported yet.

		static bool				setDevice( VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, uint32_t queueFamily );

		// What the device needs to have turned on, see above.

		static const char * const *	requiredDeviceExtensions( int& count );

		static bool				isDeviceLost() { return VulkanContext::isDeviceLost(); }

		//.____ Misc _________________________________________________________

		// The default canvas is an image of the owner of the backend, typically of a window. It
		// is in restingLayout between sessions, and in COLOR_ATTACHMENT_OPTIMAL while a session
		// renders into it. Supported formats are B8G8R8A8 and R8G8B8A8, UNORM or SRGB.

		bool	setDefaultCanvas( VkImage image, VkImageView view, VkFormat format, VkImageLayout restingLayout, SizeSPX size, int scale );

		const CanvasInfo*	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

		// The command buffer being recorded, for the owner of the backend to add commands of its
		// own between endSession() and endRender(), such as copying the default canvas to a swap
		// chain image. Null when not recording.

		VkCommandBuffer		commandBuffer() const { return m_bRecording ? m_cmd : VK_NULL_HANDLE; }

		// Semaphores endRender() waits for and signals, set by the owner before endRender(), for
		// presenting. Cleared by endRender().

		void	addWaitSemaphore( VkSemaphore semaphore, VkPipelineStageFlags stage );
		void	addSignalSemaphore( VkSemaphore semaphore );

	protected:
		VulkanBackend();
		~VulkanBackend();

		//.____ GpuBackend ___________________________________________________

		bool		_isRecording() const override;

		bool		_isNativeSurface( const Object * pObject ) const override;
		bool		_isNativeEdgemap( const Object * pObject ) const override;
		bool		_isNativeWarpmap( const Object * pObject ) const override;

		void		_beginSessionOnDevice( Surface * pCanvas ) override;
		void		_endSessionOnDevice() override;

		bool		_reserveFrameBuffers( int nVertices, int nColors, int nExtras ) override;

		bool		_bindCanvas( Surface * pCanvas, SizeI& size, CanvasKind& kind ) override;
		bool		_setPipeline( PipelineKind kind ) override;
		void		_setConstants( ConstantBlock block ) override;
		void		_draw( int firstVertex, int nVertices ) override;
		void		_setScissor( const RectI& rect ) override;
		void		_resetScissor() override;

		void		_prepareBlitSource( Surface * pSurface ) override;
		bool		_bindBlitTexture( Surface * pSurface ) override;
		void		_bindBlitSampler( Surface * pSurface, bool bClip ) override;
		bool		_bindEdgemap( GpuEdgemap * pEdgemap ) override;
		void		_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches ) override;

		//.____ Internal ______________________________________________________

		bool		_createDeviceObjects();
		void		_destroyDeviceObjects();
		bool		_createShaderModules();
		bool		_createSamplers();

		bool		_beginCommandBuffer();
		uint64_t	_submitCommandBuffer();					// Ends and submits it, returns the timeline value.
		void		_flush();								// Submits what we have recorded and goes on recording.
		void		_bindSessionState();					// Everything a fresh command buffer needs for a session.

		void		_beginRendering();						// Into the bound canvas, unless already.
		void		_endRendering();						// For barriers and copies, which can't be inside.

		void		_releaseCanvas();						// Puts the bound canvas back to rest.
		void		_generateMipmaps( VulkanSurface * pSurface );

		void		_pushBuffer( uint32_t binding, VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE );
		void		_pushImage( uint32_t binding, VkImageView view );
		void		_pushSampler( uint32_t binding, VkSampler sampler );

		VkPipeline	_pipeline( PipelineKind kind, BlendMode mode );
		VkPipeline	_createPipeline( PipelineKind kind, BlendMode mode, VkFormat format, CanvasKind canvasKind );

		static VkFormat		_defaultCanvasFormat( VkFormat format, PixelFormat& pixelFormat, ColorSpace& colorSpace );

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		// Frames in flight, each with its own command buffers and buffers for what the shaders
		// read, which grow when a session needs more than there is room for, see
		// _reserveFrameBuffers().

		struct Frame
		{
			VkCommandPool					pool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer>	cmdBuffers;				// More than one if flushed during the frame.
			int								nCmdBuffersUsed = 0;
			uint64_t						submitted = 0;			// Timeline value of the last submission.

			VulkanContext::Buffer			vertices;
			VulkanContext::Buffer			colors;
			VulkanContext::Buffer			extras;
			int								vertexCapacity = 0;		// In entries.
			int								colorCapacity = 0;
			int								extrasCapacity = 0;
		};

		static const int	c_nbFrames = 2;

		Frame				m_frames[c_nbFrames];
		int					m_frameIndex = 0;

		VkCommandBuffer		m_cmd = VK_NULL_HANDLE;
		bool				m_bRecording = false;
		bool				m_bDeviceObjectsOK = false;

		std::vector<VkSemaphore>			m_waitSemaphores;
		std::vector<VkPipelineStageFlags>	m_waitStages;
		std::vector<VkSemaphore>			m_signalSemaphores;

		// The canvas being rendered into.

		struct Target
		{
			VkImage			image = VK_NULL_HANDLE;
			VkImageView		view = VK_NULL_HANDLE;
			VkFormat		format = VK_FORMAT_UNDEFINED;
			SizeI			size;
		};

		Target				m_target;
		VulkanSurface *		m_pTargetSurface = nullptr;		// Null for the default canvas.
		bool				m_bRendering = false;			// Inside vkCmdBeginRendering().
		bool				m_bDefaultCanvasInUse = false;	// Moved out of its resting layout this session.

		// Default canvas.

		VkImage				m_defaultImage = VK_NULL_HANDLE;
		VkImageView			m_defaultView = VK_NULL_HANDLE;
		VkFormat			m_defaultFormat = VK_FORMAT_UNDEFINED;
		VkImageLayout		m_defaultRestingLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CanvasInfo			m_defaultCanvas;
		CanvasInfo			m_dummyCanvas;

		// Pipelines are created as combinations of canvas format, kind and blend mode show up.

		VkDescriptorSetLayout			m_descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout				m_pipelineLayout = VK_NULL_HANDLE;
		std::map<uint64_t, VkPipeline>	m_pipelines;
		VkPipeline						m_activePipeline = VK_NULL_HANDLE;

		VkShaderModule		m_vertexShaders[int(PipelineKind::Size)] = {};
		VkShaderModule		m_pixelShaders[int(PipelineKind::Size)] = {};
		VkShaderModule		m_dualSourcePixelShaders[int(PipelineKind::Size)] = {};
		std::vector<VkShaderModule>	m_shaderModules;		// All of them, for destroying.

		// Samplers, indexed (clip ? 4 : 0) + (tiling ? 2 : 0) + (bilinear ? 1 : 0) as DX12Backend
		// has them. The clip ones return transparent black outside the texture.

		VkSampler			m_samplers[8] = {};

		PFN_vkCmdPushDescriptorSetKHR	m_vkCmdPushDescriptorSet = nullptr;
	};

} // namespace wax
#endif // WAX_VULKANBACKEND_DOT_H
