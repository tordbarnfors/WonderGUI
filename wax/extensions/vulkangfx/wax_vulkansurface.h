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
#ifndef WAX_VULKANSURFACE_DOT_H
#define WAX_VULKANSURFACE_DOT_H
#pragma once

#include <wax_surface.h>
#include <wax_vulkancontext.h>

namespace wax
{

	class VulkanSurface;
	typedef	StrongPtr<VulkanSurface>	VulkanSurface_p;
	typedef	WeakPtr<VulkanSurface>		VulkanSurface_wp;

	//____ VulkanSurface ________________________________________________________
	/**
	 * @brief Surface for VulkanBackend.
	 *
	 * Pixels are kept in a buffer the CPU can read and write, which PixelBuffers point into, and
	 * copied to the texture when it is used, as DX12Surface does. Surfaces with alpha hold
	 * premultiplied colors in the texture. What is rendered into a canvas surface is read back
	 * when someone asks for its pixels.
	 *
	 * Formats, as DX12Surface has them: ARGB_8 is B8G8R8A8, XRGB_8 and the 565 formats (widened
	 * to XRGB_8) B8G8R8A8 read with alpha 1, Alpha_8 is R8 read as alpha, and palette based
	 * surfaces hold their indexes in R8_UINT or R16_UINT with the palette in a buffer of float4.
	 * sRGB surfaces have _SRGB formats. Pixels are always little endian.
	 *
	 * Between uses the texture rests in SHADER_READ_ONLY_OPTIMAL. layout() is what it is in at the
	 * end of what has been recorded, which VulkanBackend keeps up to date as it records.
	 */

	class VulkanSurface : public Surface
	{
		friend class VulkanBackend;

	public:

		//.____ Creation __________________________________________

		static VulkanSurface_p	create( const Blueprint& blueprint);
		static VulkanSurface_p	create( const Blueprint& blueprint, Blob* pBlob, int pitch = 0);

		static VulkanSurface_p	create( const Blueprint& blueprint, const uint8_t* pPixels,
										PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		static VulkanSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
										const PixelDescription& pixelDescription, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI			maxSize();

		//.____ Content _______________________________________________________

		int						alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		using Surface::allocPixelBuffer;
		using Surface::pullPixels;
		using Surface::pushPixels;

		const PixelBuffer		allocPixelBuffer(const RectI& rect) override;
		bool					pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void					pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true) override;
		void					freePixelBuffer(const PixelBuffer& buffer) override;

		//.____ Misc __________________________________________________________

		// Used by VulkanBackend.

		VkImage					image() const { return m_image.image; }
		VkImageView				sampleView() const { return m_sampleView; }		// Reads alpha 1 from XRGB, R as alpha from Alpha_8.
		VkImageView				attachmentView() const { return m_attachmentView; }	// Null unless a canvas.
		VkFormat				vkFormat() const { return m_vkFormat; }
		VkBuffer				paletteBuffer() const { return m_palette.buffer; }
		bool					isIndexed() const { return m_bIndexed; }

		VkImageLayout			layout() const { return m_layout; }
		void					setLayout( VkImageLayout layout ) { m_layout = layout; }

		int						mipLevels() const { return m_mipLevels; }
		bool					isMipmapStale() const { return m_bMipmapStale; }
		void					clearMipmapStale() { m_bMipmapStale = false; }

		void					syncTexture();			// Uploads pending pixel changes, if any.
		void					notifyRendered();		// Called by VulkanBackend before it renders into us.

	protected:
		VulkanSurface(const Blueprint& blueprint);
		VulkanSurface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		VulkanSurface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize);
		VulkanSurface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize);

		~VulkanSurface();

		void			_setupTexture( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize );

		bool			_setPixelDetails();			// Settles on a layout Vulkan can hold.
		bool			_createDeviceObjects();		// Image, views, pixel buffer and palette buffer. All or none.
		void			_dropDeviceObjects();

		void			_copyInPixels( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, int srcPaletteSize );

		void			_addDirtyRect( const RectI& rect );
		void			_syncBufferAndWait();		// Brings anything rendered into us back to our pixels.
		void			_updatePaletteBuffer();		// Converts m_pPalette into the palette buffer.

		bool			_isPremultiplied() const { return m_pixelFormat == PixelFormat::ARGB_8; }

		VulkanContext::Image	m_image;
		VulkanContext::Buffer	m_pixels;			// Our pixels, read and written by the CPU.
		VulkanContext::Buffer	m_palette;			// Palette based surfaces only. Capacity, then the colors.

		VkImageView		m_sampleView = VK_NULL_HANDLE;
		VkImageView		m_attachmentView = VK_NULL_HANDLE;

		VkFormat		m_vkFormat = VK_FORMAT_UNDEFINED;
		VkImageLayout	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		uint8_t *		m_pPixels = nullptr;		// m_pixels mapped, or m_pFallbackPixels.
		uint8_t *		m_pFallbackPixels = nullptr;	// Plain memory, when Vulkan wouldn't give us a buffer.
		int				m_pitch = 0;				// Bytes per line, aligned for copies.
		int				m_pixelBytes = 0;			// Bytes per pixel.
		bool			m_bIndexed = false;

		RectI			m_dirtyRect;				// Area not yet uploaded to the texture. Empty when in sync.
		bool			m_bBufferNeedsSync = false;	// Texture has changes our pixels don't have yet.

		int				m_mipLevels = 1;
		bool			m_bMipmapStale = false;		// Level 0 has changed since the others were made from it.
	};

} // namespace wax
#endif // WAX_VULKANSURFACE_DOT_H
