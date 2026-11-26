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

#include <wax_vulkanbitmap.h>
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

	const TypeInfo VulkanBitmap::TYPEINFO = { "VulkanBitmap", &GpuBitmap::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI VulkanBitmap::maxSize()
	{
		int max = VulkanContext::isReady() ? int(VulkanContext::properties().limits.maxImageDimension2D) : 4096;
		return SizeI(max, max);
	}

	//____ create() ______________________________________________________________

	VulkanBitmap_p VulkanBitmap::create(const Blueprint& blueprint)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanBitmap_p();

		return VulkanBitmap_p(new VulkanBitmap(blueprint, PixelSource()));
	}

	VulkanBitmap_p VulkanBitmap::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanBitmap_p();

		return VulkanBitmap_p(new VulkanBitmap(blueprint, _source(blueprint, pBlob, pitch)));
	}

	VulkanBitmap_p VulkanBitmap::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanBitmap_p();

		return VulkanBitmap_p(new VulkanBitmap(blueprint, _source(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize)));
	}

	VulkanBitmap_p VulkanBitmap::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return VulkanBitmap_p();

		return VulkanBitmap_p(new VulkanBitmap(blueprint, _source(blueprint, pPixels, format, pitch, pPalette, paletteSize)));
	}

	//____ constructor ________________________________________________________________

	VulkanBitmap::VulkanBitmap(const Blueprint& bp, const PixelSource& source) : GpuBitmap(bp)
	{
		_setup(bp, source);
	}

	//____ destructor ____________________________________________________________

	VulkanBitmap::~VulkanBitmap()
	{
		_dropDeviceObjects();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& VulkanBitmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setTextureFormat() _____________________________________________________
	//
	// The same formats as DX12Bitmap. The 565 formats are widened to XRGB_8, which PixelTools
	// does when pixels are copied in. The color space stays: an sRGB bitmap gets an _SRGB
	// format, so the hardware converts to linear when a texel is sampled and back when a pixel
	// is written, as our shaders work in linear.

	PixelFormat VulkanBitmap::_setTextureFormat( PixelFormat format )
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

	bool VulkanBitmap::_createDeviceObjects()
	{
		if( !VulkanContext::isReady() )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Vulkan device set. Call VulkanBackend::setDevice() before creating any bitmap.",
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
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the Vulkan objects of a bitmap, it will have no texture.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		return bOK;
	}

	//____ _dropDeviceObjects() ________________________________________________

	void VulkanBitmap::_dropDeviceObjects()
	{
		m_pulls.clear();

		VulkanContext::destroyLater( m_sampleView );
		VulkanContext::destroyLater( m_attachmentView );
		VulkanContext::destroyLater( m_image );
		VulkanContext::destroyLater( m_palette );

		m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_bMipmapStale = false;
		m_pPaletteColors = nullptr;
	}

	//____ StagingBuffer _______________________________________________________

	VulkanBitmap::StagingBuffer::~StagingBuffer()
	{
		VulkanContext::destroyLater( buffer );
	}

	//____ _allocStaging() _____________________________________________________
	//
	// A buffer the GPU copies to our image from, mapped. Lines start on four byte boundaries,
	// which is a whole number of pixels for all our formats, as copies between buffer and image
	// need. What is handed out as a PixelBuffer may be read, so it prefers cached memory.

	VulkanBitmap::Staging * VulkanBitmap::_allocStaging( SizeI size, bool bReadable )
	{
		if( !m_image.image || !VulkanContext::isReady() || VulkanContext::isDeviceLost() )
			return nullptr;

		int pitch = (size.w * m_pixelBytes + 3) & ~3;

		auto pBuffer = std::make_shared<StagingBuffer>();

		if( !VulkanContext::createBuffer( VkDeviceSize(pitch) * size.h, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
										  bReadable ? VulkanContext::Memory::HostRead : VulkanContext::Memory::HostWrite, pBuffer->buffer ) )
		{
			if( !VulkanContext::isDeviceLost() )
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for pixels on their way to the texture of a bitmap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);

			return nullptr;
		}

		auto pStaging = new VulkanStaging();

		pStaging->pBuffer = pBuffer;
		pStaging->pPixels = pBuffer->buffer.pMapped;
		pStaging->pitch = pitch;

		return pStaging;
	}

	//____ _pull() _____________________________________________________________
	//
	// The copy is recorded when VulkanBackend is about to use us, see recordUploads(). The
	// buffer is shared between the staging memory and us, as whoever has that may let go of it
	// before then, and is destroyed when the GPU is done with what the last of us had recorded.

	void VulkanBitmap::_pull( Staging * pStaging, const RectI& rect, CoordI dest )
	{
		if( !m_image.image || rect.isEmpty() )
			return;

		auto p = static_cast<VulkanStaging*>(pStaging);

		Pull pull;

		pull.pBuffer = p->pBuffer;
		pull.offset = VkDeviceSize(rect.y) * p->pitch + rect.x * m_pixelBytes;
		pull.pitch = p->pitch;
		pull.size = rect.size();
		pull.dest = dest;

		// A copy that starts at an offset that isn't a multiple of four is more than Vulkan has
		// promised to do with every queue, so such a rectangle goes by a buffer of its own.

		if( pull.offset % 4 != 0 )
		{
			auto pOwn = static_cast<VulkanStaging*>( _allocStaging( rect.size(), false ) );

			if( !pOwn )
				return;

			int lineBytes = rect.w * m_pixelBytes;

			for( int y = 0; y < rect.h; y++ )
				memcpy( pOwn->pPixels + y * pOwn->pitch, p->pPixels + (rect.y + y) * p->pitch + rect.x * m_pixelBytes, lineBytes );

			pull.pBuffer = pOwn->pBuffer;
			pull.offset = 0;
			pull.pitch = pOwn->pitch;

			delete pOwn;
		}

		m_pulls.push_back(pull);
	}

	//____ recordUploads() _____________________________________________________
	//
	// Records the copies of what the CPU has written since the last call into the given command
	// buffer, which has to be outside a render pass. No wait: the copies happen where they are
	// in the queue, after what was recorded before them and in front of what comes after.

	void VulkanBitmap::recordUploads( VkCommandBuffer cmd )
	{
		if( !cmd || !m_image.image || VulkanContext::isDeviceLost() )
			return;

		_stageDirty();				// Our own pixels that have changed, by a buffer made for it.

		if( m_pulls.empty() )
			return;

		VkImageLayout restLayout = m_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : m_layout;

		// A partial upload must keep what is outside it, so only a fresh image may be
		// discarded.

		VulkanContext::transition( cmd, m_image.image, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

		for( auto& pull : m_pulls )
		{
			VkBufferImageCopy region = {};
			region.bufferOffset = pull.offset;
			region.bufferRowLength = uint32_t(pull.pitch / m_pixelBytes);
			region.bufferImageHeight = 0;
			region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			region.imageOffset = { pull.dest.x, pull.dest.y, 0 };
			region.imageExtent = { uint32_t(pull.size.w), uint32_t(pull.size.h), 1 };

			vkCmdCopyBufferToImage( cmd, pull.pBuffer->buffer.buffer, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
		}

		VulkanContext::transition( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, restLayout );
		m_layout = restLayout;

		// Lets go of the buffers, which are destroyed when the GPU is done with what is being
		// recorded.

		m_pulls.clear();

		_uploaded();
	}

	//____ _readBack() _________________________________________________________
	//
	// Copies a rectangle of our texture to pDest, by a buffer made for it. This stalls until the
	// GPU is done, but only happens when someone asks for the pixels of a canvas.

	bool VulkanBitmap::_readBack( const RectI& rect, uint8_t * pDest, int destPitch )
	{
		if( VulkanContext::isDeviceLost() || !m_image.image )
			return false;

		int pitch = (rect.w * m_pixelBytes + 3) & ~3;

		VulkanContext::Buffer buffer;

		if( !VulkanContext::createBuffer( VkDeviceSize(pitch) * rect.h, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VulkanContext::Memory::HostRead, buffer ) )
			return false;

		// Has every backend submit what it has recorded, so it comes before us.

		VkCommandBuffer cmd = VulkanContext::beginOneShot();

		if( !cmd )
		{
			VulkanContext::destroyLater( buffer );
			return false;
		}

		// Pixels written by the CPU since are the newer ones, so they go in on top of what was
		// rendered before we read it back, in the same command buffer.

		recordUploads( cmd );

		// A texture nothing has been sent to or rendered into has nothing to give us but what
		// its memory holds, and comes to rest like the others.

		VkImageLayout restLayout = m_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : m_layout;

		VulkanContext::transition( cmd, m_image.image, m_layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = uint32_t(pitch / m_pixelBytes);
		region.bufferImageHeight = 0;
		region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.imageOffset = { rect.x, rect.y, 0 };
		region.imageExtent = { uint32_t(rect.w), uint32_t(rect.h), 1 };

		vkCmdCopyImageToBuffer( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.buffer, 1, &region );

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

		bool bOK = VulkanContext::endOneShot( cmd );			// Waits for it.

		if( bOK )
		{
			int lineBytes = rect.w * m_pixelBytes;

			for( int y = 0; y < rect.h; y++ )
				memcpy( pDest + y * destPitch, buffer.pMapped + y * pitch, lineBytes );
		}

		VulkanContext::destroyLater( buffer );
		return bOK;
	}

} // namespace wax
