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

#include <wax_gpusurface.h>
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
	 * Built on GpuSurface, which keeps our pixels and knows what the texture lacks of them. They
	 * are in a buffer the CPU can read and write, and copied to the texture when it is used.
	 *
	 * Formats, as DX12Surface has them: ARGB_8 is B8G8R8A8, XRGB_8 and the 565 formats (widened
	 * to XRGB_8) B8G8R8A8 read with alpha 1, Alpha_8 is R8 read as alpha, and palette based
	 * surfaces hold their indexes in R8_UINT or R16_UINT with the palette in a buffer of float4.
	 * sRGB surfaces have _SRGB formats. Pixels are always little endian.
	 *
	 * Between uses the texture rests in SHADER_READ_ONLY_OPTIMAL. layout() is what it is in at the
	 * end of what has been recorded, which VulkanBackend keeps up to date as it records. A texture
	 * nothing has been sent to or rendered into is still in UNDEFINED, which it can be taken out
	 * of like any other, with whatever it holds.
	 */

	class VulkanSurface : public GpuSurface
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

		//.____ Misc __________________________________________________________

		// Used by VulkanBackend.

		VkImage					image() const { return m_image.image; }
		VkImageView				sampleView() const { return m_sampleView; }		// Reads alpha 1 from XRGB, R as alpha from Alpha_8.
		VkImageView				attachmentView() const { return m_attachmentView; }	// Null unless a canvas.
		VkFormat				vkFormat() const { return m_vkFormat; }
		VkBuffer				paletteBuffer() const { return m_palette.buffer; }

		VkImageLayout			layout() const { return m_layout; }
		void					setLayout( VkImageLayout layout ) { m_layout = layout; }

		// Sends what the CPU has written since the last call to the texture, if anything. Called
		// by VulkanBackend before the surface is drawn from or into.

		void					syncTexture();

	protected:
		VulkanSurface(const Blueprint& blueprint, const PixelSource& source);
		~VulkanSurface();

		// What GpuSurface has us do.

		PixelFormat		_setTextureFormat( PixelFormat format ) override;	// Vulkan has no 565 we can count on, they are widened to XRGB_8.
		bool			_createDeviceObjects( bool bPixelBuffer ) override;	// Image, views, pixel buffer and palette buffer. All or none.
		bool			_createPixelBuffer() override;
		bool			_readBack() override;

		void			_dropDeviceObjects();

		VulkanContext::Image	m_image;
		VulkanContext::Buffer	m_pixels;			// Our pixels, read and written by the CPU. A canvas may be without, see GpuSurface::_needPixels().
		VulkanContext::Buffer	m_palette;			// Palette based surfaces only. Capacity, then the colors.

		VkImageView		m_sampleView = VK_NULL_HANDLE;
		VkImageView		m_attachmentView = VK_NULL_HANDLE;

		VkFormat		m_vkFormat = VK_FORMAT_UNDEFINED;
		VkImageLayout	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

} // namespace wax
#endif // WAX_VULKANSURFACE_DOT_H
