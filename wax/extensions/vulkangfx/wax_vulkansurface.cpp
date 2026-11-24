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

#include <wax_vulkansurface.h>
#include <wax_vulkanbackend.h>
#include <wax_gputools.h>

#include <wax_base.h>
#include <wax_util.h>
#include <wax_pixeltools.h>

#include <algorithm>
#include <cstring>
#include <cstdio>

namespace wax
{

	const TypeInfo VulkanSurface::TYPEINFO = { "VulkanSurface", &GpuSurface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI VulkanSurface::maxSize()
	{
		int max = VulkanContext::isReady() ? int(VulkanContext::properties().limits.maxImageDimension2D) : 4096;
		return SizeI(max, max);
	}

	//____ create() ______________________________________________________________

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanSurface_p();

		return VulkanSurface_p(new VulkanSurface(blueprint, PixelSource()));
	}

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanSurface_p();

		return VulkanSurface_p(new VulkanSurface(blueprint, _source(blueprint, pBlob, pitch)));
	}

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanSurface_p();

		return VulkanSurface_p(new VulkanSurface(blueprint, _source(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize)));
	}

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanSurface_p();

		return VulkanSurface_p(new VulkanSurface(blueprint, _source(blueprint, pPixels, format, pitch, pPalette, paletteSize)));
	}

	//____ constructor ________________________________________________________________
	//
	// Lines start on four byte boundaries, which is a whole number of pixels for all our
	// formats, as copies between buffer and image need.

	VulkanSurface::VulkanSurface(const Blueprint& bp, const PixelSource& source) : GpuSurface(bp, 4)
	{
		_setup(bp, source);
	}

	//____ destructor ____________________________________________________________

	VulkanSurface::~VulkanSurface()
	{
		_dropDeviceObjects();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& VulkanSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setTextureFormat() _____________________________________________________
	//
	// The same formats as DX12Surface. The 565 formats are widened to XRGB_8, which PixelTools
	// does when pixels are copied in. The color space stays: an sRGB surface gets an _SRGB
	// format, so the hardware converts to linear when a texel is sampled and back when a pixel
	// is written, as our shaders work in linear.

	PixelFormat VulkanSurface::_setTextureFormat( PixelFormat format )
	{
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

		switch( format )
		{
			// XRGB is held in a BGRA texture, read with alpha 1 whatever the unused byte holds,
			// see sampleView(), and rendered into without touching alpha.

			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
				format = PixelFormat::XRGB_8;
				[[fallthrough]];

			case PixelFormat::XRGB_8:
			case PixelFormat::ARGB_8:
				m_vkFormat = bSRGB ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8A8_UNORM;
				break;

			// Vulkan has no A8 without an extension, so alpha is kept in R and read as alpha. The
			// shaders write the value to all channels of an alpha only canvas, see output.hlsl.

			case PixelFormat::Alpha_8:
				m_vkFormat = VK_FORMAT_R8_UNORM;
				break;

			// The texture holds the indexes, which the pixel shader looks up in the palette. The
			// color space is the palette's, see GpuTools::paletteToFloats().

			case PixelFormat::Index_8:
				m_vkFormat = VK_FORMAT_R8_UINT;
				break;

			case PixelFormat::Index_16:
				m_vkFormat = VK_FORMAT_R16_UINT;
				break;

			default:
				m_vkFormat = VK_FORMAT_UNDEFINED;			// create() turns other formats away.
				break;
		}

		return format;
	}

	//____ _createDeviceObjects() ______________________________________________

	bool VulkanSurface::_createDeviceObjects( bool bPixelBuffer )
	{
		if( !VulkanContext::isReady() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Vulkan device set. Call VulkanBackend::setDevice() before creating any surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if( VulkanContext::isDeviceLost() )
			return false;

		VkDevice device = VulkanContext::device();

		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = m_vkFormat;
		info.extent = { uint32_t(m_size.w), uint32_t(m_size.h), 1 };
		info.mipLevels = uint32_t(m_mipLevels);
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if( m_bCanvas )
			info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		bool bOK = VulkanContext::createImage( info, m_image );

		m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		// The view shaders sample. XRGB reads alpha 1 and Alpha_8 its R as alpha, so the shaders
		// see them as DX12 has them.

		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = m_image.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_vkFormat;
		viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, uint32_t(m_mipLevels), 0, 1 };

		if( m_pixelFormat == PixelFormat::XRGB_8 )
			viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_ONE };
		else if( m_pixelFormat == PixelFormat::Alpha_8 )
			viewInfo.components = { VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_R };

		if( bOK )
			bOK = VulkanContext::check( vkCreateImageView( device, &viewInfo, nullptr, &m_sampleView ), "vkCreateImageView", this, &TYPEINFO, __func__, __FILE__, __LINE__ );

		// Render target views must not swizzle.

		if( bOK && m_bCanvas )
		{
			viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
			viewInfo.subresourceRange.levelCount = 1;

			bOK = VulkanContext::check( vkCreateImageView( device, &viewInfo, nullptr, &m_attachmentView ), "vkCreateImageView", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
		}

		// Our pixels, unless they can wait until they are asked for.

		if( bOK && bPixelBuffer )
			bOK = _createPixelBuffer();

		// One float4 per palette entry, plus one in front holding the capacity, so that an index
		// past the end of the palette - there is nothing stopping one - can be caught.

		if( bOK && m_bIndexed )
		{
			bOK = m_pPalette && VulkanContext::createBuffer( VkDeviceSize(m_paletteCapacity + 1) * 4 * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
															  VulkanContext::Memory::HostWrite, m_palette );
			if( bOK )
			{
				m_pPaletteColors = (float*) m_palette.pMapped;
				_updatePaletteColors();
			}
		}

		if( !bOK )
		{
			_dropDeviceObjects();

			if( !VulkanContext::isDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the Vulkan objects of a surface, it will have no texture.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		return bOK;
	}

	//____ _createPixelBuffer() ________________________________________________
	//
	// Our pixels, which are also where what is rendered into us is read back to.

	bool VulkanSurface::_createPixelBuffer()
	{
		if( !m_image.image || !VulkanContext::isReady() || VulkanContext::isDeviceLost() )
			return false;

		if( !VulkanContext::createBuffer( VkDeviceSize(m_pitch) * m_size.h, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
										  VulkanContext::Memory::HostRead, m_pixels ) )
			return false;

		m_pPixels = (uint8_t*) m_pixels.pMapped;
		return true;
	}

	//____ _dropDeviceObjects() ________________________________________________

	void VulkanSurface::_dropDeviceObjects()
	{
		VulkanContext::destroyLater( m_sampleView );
		VulkanContext::destroyLater( m_attachmentView );
		VulkanContext::destroyLater( m_image );
		VulkanContext::destroyLater( m_pixels );
		VulkanContext::destroyLater( m_palette );

		m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_bMipmapStale = false;
		m_pPixels = m_pFallbackPixels;
		m_pPaletteColors = nullptr;
	}

	//____ syncTexture() _______________________________________________________
	//
	// Uploads whatever has changed since the last call and waits for it. Called by VulkanBackend
	// before the surface is used as blit source or rendered into.
	//
	//TODO: This stalls, as DX12Surface did. Fine for surfaces filled while loading, which is the
	// normal case, but a surface updated every frame wants something better.

	void VulkanSurface::syncTexture()
	{
		if( m_dirtyRect.isEmpty() || !m_image.image || !m_pixels.buffer || VulkanContext::isDeviceLost() )
			return;

		// Full lines, so the source offset stays aligned.

		RectI lines( 0, m_dirtyRect.y, m_size.w, m_dirtyRect.h );

		// Surfaces with alpha keep premultiplied colors in the texture. Our pixels are the ones
		// handed out, so they are premultiplied in a buffer of their own, unless they are the
		// same premultiplied.

		VkBuffer source = m_pixels.buffer;
		VkDeviceSize sourceOffset = VkDeviceSize(lines.y) * m_pitch;

		VulkanContext::Buffer premultiplied;

		if( _needsPremultiplying(lines) )
		{
			if( VulkanContext::createBuffer( VkDeviceSize(m_pitch) * lines.h, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VulkanContext::Memory::HostWrite, premultiplied ) )
			{
				_premultiply( lines, (uint8_t*) premultiplied.pMapped, m_pitch );

				source = premultiplied.buffer;
				sourceOffset = 0;
			}
			else if( !VulkanContext::isDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for premultiplied pixels, colors of transparent pixels will be off.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		VkCommandBuffer cmd = VulkanContext::beginOneShot();

		if( cmd )
		{
			VkImageLayout restLayout = m_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : m_layout;

			// A partial upload must keep what is outside it, so only a fresh image may be
			// discarded.

			VulkanContext::transition( cmd, m_image.image, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

			VkBufferImageCopy region = {};
			region.bufferOffset = sourceOffset;
			region.bufferRowLength = uint32_t(m_pitch / m_pixelBytes);
			region.bufferImageHeight = 0;
			region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			region.imageOffset = { 0, lines.y, 0 };
			region.imageExtent = { uint32_t(lines.w), uint32_t(lines.h), 1 };

			vkCmdCopyBufferToImage( cmd, source, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

			VulkanContext::transition( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, restLayout );
			m_layout = restLayout;

			VulkanContext::endOneShot( cmd );
		}

		VulkanContext::destroyLater( premultiplied );

		_uploaded();
	}

	//____ _readBack() _________________________________________________________
	//
	// Copies our texture into our pixels. This stalls until the GPU is done, but only happens
	// when someone actually asks to read a canvas surface.

	bool VulkanSurface::_readBack()
	{
		if( VulkanContext::isDeviceLost() || !m_image.image || !m_pixels.buffer )
			return false;

		// Pixels written by the CPU since are the newer ones, so they go in on top of what was
		// rendered before we read it all back.

		syncTexture();

		VkCommandBuffer cmd = VulkanContext::beginOneShot();
		if( !cmd )
			return false;

		// A texture nothing has been sent to or rendered into has nothing to give us but what
		// its memory holds, and comes to rest like the others.

		VkImageLayout restLayout = m_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : m_layout;

		VulkanContext::transition( cmd, m_image.image, m_layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = uint32_t(m_pitch / m_pixelBytes);
		region.bufferImageHeight = 0;
		region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { uint32_t(m_size.w), uint32_t(m_size.h), 1 };

		vkCmdCopyImageToBuffer( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_pixels.buffer, 1, &region );

		VulkanContext::transition( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, restLayout );
		m_layout = restLayout;

		// The copy has to be visible to the host when we read it.

		VkMemoryBarrier2 barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;

		VkDependencyInfo dep = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dep.memoryBarrierCount = 1;
		dep.pMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2( cmd, &dep );

		return VulkanContext::endOneShot( cmd );
	}

} // namespace wax
