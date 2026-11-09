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

	const TypeInfo VulkanSurface::TYPEINFO = { "VulkanSurface", &Surface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI VulkanSurface::maxSize()
	{
		int max = VulkanContext::isReady() ? int(VulkanContext::properties().limits.maxImageDimension2D) : 4096;
		return SizeI(max, max);
	}

	//____ create() ______________________________________________________________

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint)
	{
		return VulkanSurface_p(new VulkanSurface(blueprint));
	}

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return VulkanSurface_p(new VulkanSurface(blueprint, pBlob, pitch));
	}

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		return VulkanSurface_p(new VulkanSurface(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	VulkanSurface_p VulkanSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		return VulkanSurface_p(new VulkanSurface(blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}

	//____ constructor ________________________________________________________________
	//
	// Default sample method is Bilinear, as for the other GPU surfaces.

	VulkanSurface::VulkanSurface(const Blueprint& bp) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;
		_setupTexture( nullptr, 0, nullptr, nullptr, bp.palette, 0 );
	}

	VulkanSurface::VulkanSurface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;

		// The blob holds the layout that was asked for, which is not always the one we end up
		// with, so take note of it before _setupTexture() settles that.

		PixelDescription srcDesc = m_pixelDescription;

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

		_setupTexture( pBlob ? pBlob->data() : nullptr, pitch, &srcDesc, bp.palette, bp.palette, 0 );
	}

	VulkanSurface::VulkanSurface(const Blueprint& bp, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;

		// Source pixels are in the byte order and color space of the blueprint.

		if( format == PixelFormat::Undefined )
			format = m_pixelFormat;

		auto srcDesc = Util::pixelFormatToDescription(format, bp.bigEndian);

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

		_fixSrcParam(srcDesc, pPalette, paletteSize);
		_setupTexture( pPixels, pitch, &srcDesc, pPalette, bp.palette, paletteSize );
	}

	VulkanSurface::VulkanSurface(const Blueprint& bp, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(pixelDescription, m_size.w);

		_fixSrcParam(pixelDescription, pPalette, paletteSize);
		_setupTexture( pPixels, pitch, &pixelDescription, pPalette, bp.palette, paletteSize );
	}

	//____ destructor ____________________________________________________________

	VulkanSurface::~VulkanSurface()
	{
		_dropDeviceObjects();

		delete [] m_pFallbackPixels;
		delete [] m_pPalette;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& VulkanSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setPixelDetails() ______________________________________________________
	//
	// Settles on a pixel layout Vulkan can hold, which is not always the one that was asked for,
	// the same ones as DX12Surface. The 565 formats are widened to XRGB_8, and pixels are always
	// little endian. PixelTools does the conversion when pixels are copied in. The color space
	// stays: an sRGB surface gets an _SRGB format, so the hardware converts to linear when a texel
	// is sampled and back when a pixel is written, as our shaders work in linear.

	bool VulkanSurface::_setPixelDetails()
	{
		bool bSupported = true;
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

		PixelFormat format = m_pixelFormat;

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
			// color space is the palette's, see _updatePaletteBuffer().

			case PixelFormat::Index_8:
				m_vkFormat = VK_FORMAT_R8_UINT;
				break;

			case PixelFormat::Index_16:
				m_vkFormat = VK_FORMAT_R16_UINT;
				break;

			default:
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer), "Pixel format %s is not supported by VulkanBackend. The surface will have no texture.", toString(format));
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

				m_vkFormat = VK_FORMAT_UNDEFINED;
				bSupported = false;
				break;
			}
		}

		m_pixelFormat = format;
		m_bBigEndian = false;
		m_pixelDescription = Util::pixelFormatToDescription(format, m_bBigEndian);
		m_pixelBytes = m_pPixelDescription->bits / 8;
		m_bIndexed = (m_vkFormat == VK_FORMAT_R8_UINT || m_vkFormat == VK_FORMAT_R16_UINT);

		return bSupported;
	}

	//____ _setupTexture() _____________________________________________________

	void VulkanSurface::_setupTexture( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize )
	{
		bool bFormatSupported = _setPixelDetails();

		// Surface leaves the palette to us. It has to exist before any pixels are copied in, since
		// converting to a palette based format fills it in.

		if( m_pPixelDescription->type == PixelType::Index && m_paletteCapacity > 0 )
		{
			m_pPalette = new Color8[m_paletteCapacity];
			memset( m_pPalette, 0, sizeof(Color8) * m_paletteCapacity );

			if( pDstPalette && m_paletteSize > 0 )
				memcpy( m_pPalette, pDstPalette, sizeof(Color8) * m_paletteSize );
		}

		if( m_pixelBytes <= 0 || m_size.w <= 0 || m_size.h <= 0 )
		{
			char buffer[256];
			snprintf(buffer, sizeof(buffer), "Can't create a %dx%d surface with %d bits per pixel.", m_size.w, m_size.h, m_pPixelDescription->bits);
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Lines start on four byte boundaries, which is a whole number of pixels for all our
		// formats, as copies between buffer and image need.

		m_pitch = (m_size.w * m_pixelBytes + 3) & ~3;

		if( !bFormatSupported || !_createDeviceObjects() )
		{
			if( VulkanContext::isReady() && bFormatSupported && !VulkanContext::isDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the Vulkan objects of a surface, it will have no texture.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			else if( !VulkanContext::isReady() )
				Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Vulkan device set. Call VulkanBackend::setDevice() before creating any surface.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);

			// A surface with no texture is of no use to the GPU, but it still has to behave like
			// a surface: hand out pixel buffers, be readable, be writable.

			m_pFallbackPixels = new uint8_t[size_t(m_pitch) * m_size.h];
			m_pPixels = m_pFallbackPixels;
		}

		memset( m_pPixels, 0, size_t(m_pitch) * m_size.h );

		_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );
		_updatePaletteBuffer();
	}

	//____ _createDeviceObjects() ______________________________________________

	bool VulkanSurface::_createDeviceObjects()
	{
		if( !VulkanContext::isReady() || VulkanContext::isDeviceLost() )
			return false;

		VkDevice device = VulkanContext::device();

		// A mipmapped surface gets levels all the way down to 1x1, made from level 0 by
		// VulkanBackend when the surface is read with stale ones. Palette based surfaces can't be
		// mipmapped, an average of indexes means nothing.

		m_mipLevels = 1;

		if( m_bMipmapped && !m_bIndexed )
		{
			for( int size = std::max(m_size.w, m_size.h); size > 1; size >>= 1 )
				m_mipLevels++;
		}

		bool bCanvas = m_bCanvas && !m_bIndexed;

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

		if( bCanvas )
			info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if( !VulkanContext::createImage( info, m_image ) )
			return false;

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

		bool bOK = VulkanContext::check( vkCreateImageView( device, &viewInfo, nullptr, &m_sampleView ), "vkCreateImageView", this, &TYPEINFO, __func__, __FILE__, __LINE__ );

		// Render target views must not swizzle.

		if( bOK && bCanvas )
		{
			viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
			viewInfo.subresourceRange.levelCount = 1;

			bOK = VulkanContext::check( vkCreateImageView( device, &viewInfo, nullptr, &m_attachmentView ), "vkCreateImageView", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
		}

		// Our pixels, which are also where what is rendered into us is read back to.

		if( bOK )
			bOK = VulkanContext::createBuffer( VkDeviceSize(m_pitch) * m_size.h, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
											   VulkanContext::Memory::HostRead, m_pixels );

		// One float4 per palette entry, plus one in front holding the capacity, so that an index
		// past the end of the palette - there is nothing stopping one - can be caught.

		if( bOK && m_bIndexed )
		{
			bOK = m_pPalette && VulkanContext::createBuffer( VkDeviceSize(m_paletteCapacity + 1) * 4 * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
															  VulkanContext::Memory::HostWrite, m_palette );
			if( bOK )
			{
				memset( m_palette.pMapped, 0, size_t(m_paletteCapacity + 1) * 4 * sizeof(float) );
				((float*) m_palette.pMapped)[0] = float(m_paletteCapacity);
			}
		}

		if( !bOK )
		{
			_dropDeviceObjects();
			return false;
		}

		m_pPixels = m_pixels.pMapped;
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
		m_mipLevels = 1;
		m_bMipmapStale = false;
		m_pPixels = m_pFallbackPixels;
	}

	//____ _updatePaletteBuffer() ______________________________________________
	//
	// The palette goes to the GPU converted to linear, the same way SoftSurface unpacks it, and
	// premultiplied, like the pixels of surfaces with alpha, so the shader can use the colors as
	// they are and interpolate between them. The buffer is read by the GPU where it is, so a
	// frame in flight may see new colors.

	void VulkanSurface::_updatePaletteBuffer()
	{
		if( !m_palette.pMapped || !m_pPalette )
			return;

		bool bLinear = (m_colorSpace == ColorSpace::Linear);

		float * p = ((float*) m_palette.pMapped) + 4;			// Past the capacity.

		for( int i = 0; i < m_paletteCapacity; i++ )
		{
			HiColor col = m_pPalette[i];

			if( bLinear )
			{
				p[0] = col.r / 4096.f;
				p[1] = col.g / 4096.f;
				p[2] = col.b / 4096.f;
				p[3] = col.a / 4096.f;
			}
			else
				col.toLinearFloat(p);

			p[0] *= p[3];
			p[1] *= p[3];
			p[2] *= p[3];
			p += 4;
		}
	}

	//____ _copyInPixels() _____________________________________________________

	void VulkanSurface::_copyInPixels( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, int srcPaletteSize )
	{
		if( !m_pPixels )
			return;

		if( pPixels && pSrcPixelDesc )
		{
			if( srcPaletteSize == 0 )
				srcPaletteSize = m_paletteSize;

			int dstLineMargin = m_pitch - m_size.w * m_pixelBytes;

			PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, * pSrcPixelDesc, m_colorSpace, pitch - PixelTools::bytesPerLine(*pSrcPixelDesc, m_size.w),
								   pSrcPalette, srcPaletteSize,
								   m_pPixels, m_pixelFormat, m_colorSpace, m_bBigEndian, dstLineMargin,
								   m_pPalette, m_paletteSize, m_paletteCapacity);
		}

		// Everything we have goes to the texture on first use.

		m_dirtyRect = RectI(0, 0, m_size.w, m_size.h);
	}

	//____ _addDirtyRect() _____________________________________________________

	void VulkanSurface::_addDirtyRect( const RectI& rect )
	{
		if( rect.isEmpty() )
			return;

		m_dirtyRect = m_dirtyRect.isEmpty() ? rect : RectI::bounds(m_dirtyRect, rect);
	}

	//____ syncTexture() _______________________________________________________
	//
	// Uploads whatever has changed since the last call and waits for it. Called by VulkanBackend
	// before the surface is used as blit source or rendered into.
	//
	//TODO: This stalls, as DX12Surface does. Fine for surfaces filled while loading, which is the
	// normal case, but a surface updated every frame wants something better.

	void VulkanSurface::syncTexture()
	{
		if( m_dirtyRect.isEmpty() || !m_image.image || VulkanContext::isDeviceLost() )
			return;

		// Full lines, so the source offset stays aligned.

		int y1 = m_dirtyRect.y;
		int y2 = m_dirtyRect.y + m_dirtyRect.h;

		// Surfaces with alpha keep premultiplied colors in the texture. Our pixels are the ones
		// handed out, so they are premultiplied in a buffer of their own, unless all opaque.

		VkBuffer source = m_pixels.buffer;
		VkDeviceSize sourceOffset = VkDeviceSize(y1) * m_pitch;

		VulkanContext::Buffer premultiplied;

		if( _isPremultiplied() && !GpuTools::isOpaque(m_pPixels + sourceOffset, m_pitch, m_size.w, y2 - y1) )
		{
			size_t nBytes = size_t(m_pitch) * (y2 - y1);

			if( VulkanContext::createBuffer( nBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VulkanContext::Memory::HostWrite, premultiplied ) )
			{
				memcpy( premultiplied.pMapped, m_pPixels + sourceOffset, nBytes );
				GpuTools::convertAlpha( premultiplied.pMapped, m_pitch, m_size.w, y2 - y1, true, m_colorSpace == ColorSpace::sRGB );

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
			region.imageOffset = { 0, y1, 0 };
			region.imageExtent = { uint32_t(m_size.w), uint32_t(y2 - y1), 1 };

			vkCmdCopyBufferToImage( cmd, source, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

			VulkanContext::transition( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, restLayout );
			m_layout = restLayout;

			VulkanContext::endOneShot( cmd );
		}

		VulkanContext::destroyLater( premultiplied );

		m_dirtyRect = RectI();

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;
	}

	//____ notifyRendered() ____________________________________________________
	//
	// Called by VulkanBackend when this surface is about to be rendered into. Our pixels are the
	// older copy from here on, until someone asks for them.

	void VulkanSurface::notifyRendered()
	{
		syncTexture();			// Anything the CPU wrote goes in before we render over it.

		m_bBufferNeedsSync = true;

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;
	}

	//____ _syncBufferAndWait() ________________________________________________
	//
	// Brings whatever was rendered into our texture back into our pixels. This stalls until the
	// GPU is done, but only happens when someone actually asks to read a canvas surface.

	void VulkanSurface::_syncBufferAndWait()
	{
		if( !m_bBufferNeedsSync || VulkanContext::isDeviceLost() || !m_image.image || !m_pixels.buffer )
			return;

		// Pixels written by the CPU since are the newer ones, so they go in on top of what was
		// rendered before we read it all back.

		syncTexture();

		VkCommandBuffer cmd = VulkanContext::beginOneShot();
		if( !cmd )
			return;			// Leaves the flag set, so we try again rather than hand out stale pixels.

		VkImageLayout layout = m_layout;

		VulkanContext::transition( cmd, m_image.image, layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = uint32_t(m_pitch / m_pixelBytes);
		region.bufferImageHeight = 0;
		region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { uint32_t(m_size.w), uint32_t(m_size.h), 1 };

		vkCmdCopyImageToBuffer( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_pixels.buffer, 1, &region );

		VulkanContext::transition( cmd, m_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, layout );

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

		if( !VulkanContext::endOneShot( cmd ) )
			return;

		m_bBufferNeedsSync = false;

		// The texture holds premultiplied colors, see syncTexture().

		if( _isPremultiplied() )
			GpuTools::convertAlpha( m_pPixels, m_pitch, m_size.w, m_size.h, false, m_colorSpace == ColorSpace::sRGB );

		m_dirtyRect = RectI();			// Our pixels and the texture are identical now.
	}

	//____ alpha() _______________________________________________________________

	int VulkanSurface::alpha(CoordSPX coord)
	{
		if( !m_pPixels )
			return 0;

		if( m_bBufferNeedsSync )
			_syncBufferAndWait();

		return _alpha(coord, allocPixelBuffer());
	}

	//____ allocPixelBuffer() _________________________________________________
	//
	// Our pixels always live in memory the CPU can reach, so this just points into it. Note that
	// the pitch can be padded.

	const PixelBuffer VulkanSurface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;

		buf.format = m_pixelFormat;
		buf.colorSpace = m_colorSpace;
		buf.bigEndian = m_bBigEndian;
		buf.palette = m_pPalette;
		buf.pitch = m_pitch;
		buf.pixels = m_pPixels ? m_pPixels + rect.y * m_pitch + rect.x * m_pixelBytes : nullptr;
		buf.rect = rect;

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool VulkanSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		// The buffer is our pixels, so it is already up to date unless something has been
		// rendered into us since we last looked.

		if( m_bBufferNeedsSync )
			_syncBufferAndWait();

		return m_pPixels != nullptr;
	}

	//____ pullPixels() _______________________________________________________

	void VulkanSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		_addDirtyRect( bufferRect + buffer.rect.pos() );

		// Surface::copy() into a palette based surface can append colors to the palette through
		// PixelTools::copyPixels() and then calls pullPixels(), so the palette is converted again,
		// as DX12Surface does.

		if( m_bIndexed )
			_updatePaletteBuffer();

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() __________________________________________________

	void VulkanSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
	}

} // namespace wax
