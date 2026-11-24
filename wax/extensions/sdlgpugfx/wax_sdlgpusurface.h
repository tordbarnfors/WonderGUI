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
#ifndef WAX_SDLGPUSURFACE_DOT_H
#define WAX_SDLGPUSURFACE_DOT_H
#pragma once

#include <wax_gpusurface.h>
#include <wax_sdlgpucontext.h>

#include <memory>
#include <vector>

namespace wax
{

	class SdlGpuSurface;
	typedef	StrongPtr<SdlGpuSurface>	SdlGpuSurface_p;
	typedef	WeakPtr<SdlGpuSurface>		SdlGpuSurface_wp;

	//____ SdlGpuSurface ________________________________________________________
	/**
	 * @brief Surface for SdlGpuBackend.
	 *
	 * Built on GpuSurface, which has our pixels, if we keep any, and knows what the texture lacks
	 * of them. What is to be sent goes by transfer buffers, which are copied to the texture in the
	 * copy pass of the session that first draws from or into us, see recordUploads().
	 *
	 * Formats: ARGB_8 is B8G8R8A8, as are XRGB_8 and the 565 formats (widened to XRGB_8), whose
	 * alpha the shaders take as 1. Alpha_8 is R8, whose value the shaders take as alpha, see
	 * ../hlsl/sdlgpu.hlsl: SDL_GPU has no swizzles to give us what DX12 and Vulkan read. Palette
	 * based surfaces hold their indexes in R8_UNORM or R16_UNORM, as SDL_GPU only samples textures
	 * of floats, with the palette in a buffer of float4. sRGB surfaces have _SRGB formats. Pixels are always little endian.
	 *
	 * Needs the device set by SdlGpuBackend::setDevice().
	 */

	class SdlGpuSurface : public GpuSurface
	{
		friend class SdlGpuBackend;

	public:

		//.____ Creation __________________________________________

		static SdlGpuSurface_p	create( const Blueprint& blueprint);
		static SdlGpuSurface_p	create( const Blueprint& blueprint, Blob* pBlob, int pitch = 0);

		static SdlGpuSurface_p	create( const Blueprint& blueprint, const uint8_t* pPixels,
										PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		static SdlGpuSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
										const PixelDescription& pixelDescription, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI			maxSize();

		//.____ Misc __________________________________________________________

		// The texture, for the owner of a canvas to show it, as copied to a window with
		// SDL_BlitGPUTexture(). Has SAMPLER usage, and COLOR_TARGET for a canvas.

		SDL_GPUTexture *		texture() const { return m_pTexture; }
		SDL_GPUTextureFormat	textureFormat() const { return m_textureFormat; }

	protected:
		SdlGpuSurface(const Blueprint& blueprint, const PixelSource& source);
		~SdlGpuSurface();

		// What GpuSurface has us do.

		PixelFormat		_setTextureFormat( PixelFormat format ) override;
		bool			_createDeviceObjects() override;					// Texture and palette buffer. All or none.
		Staging *		_allocStaging( SizeI size, bool bReadable ) override;
		void			_pull( Staging * pStaging, const RectI& rect, CoordI dest ) override;
		bool			_readBack( const RectI& rect, uint8_t * pDest, int destPitch ) override;
		void			_updatePaletteColors() override;

		void			_dropDeviceObjects();

		// For SdlGpuBackend, before the surface is drawn from or into: what the CPU has written
		// that the texture lacks, copied in a copy pass. Palette colors go by the buffer uploads.

		bool			_hasUploads() const { return !m_pulls.empty() || (m_pPixels && !m_dirtyRect.isEmpty()) || m_bPaletteChanged; }
		void			_recordUploads( SDL_GPUCopyPass * pCopyPass, SdlGpuContext::BufferUploads& bufferUploads );

		SDL_GPUBuffer *	_paletteBuffer() const { return m_pPaletteBuffer; }

		// A transfer buffer of staging memory, which lives until the last copy from it has been
		// recorded. SDL_GPU keeps it for the GPU after that.

		struct TransferBuffer
		{
			~TransferBuffer();

			void	unmap();

			SDL_GPUTransferBuffer *	pBuffer = nullptr;
			bool					bMapped = false;
		};

		struct SdlGpuStaging : public Staging
		{
			~SdlGpuStaging() { pBuffer->unmap(); }

			std::shared_ptr<TransferBuffer>	pBuffer;
		};

		// What is to be copied to the texture.

		struct Pull
		{
			std::shared_ptr<TransferBuffer>	pBuffer;
			uint32_t		offset;				// Of the first pixel in the buffer.
			int				pitch;
			SizeI			size;
			CoordI			dest;				// Where in the texture.
		};

		std::vector<Pull>		m_pulls;

		SDL_GPUTexture *		m_pTexture = nullptr;
		SDL_GPUTextureFormat	m_textureFormat = SDL_GPU_TEXTUREFORMAT_INVALID;

		SDL_GPUBuffer *			m_pPaletteBuffer = nullptr;		// Palette based surfaces only. Capacity, then the colors.
		std::vector<float>		m_paletteColors;				// What GpuSurface writes, uploaded to m_pPaletteBuffer.
		bool					m_bPaletteChanged = false;
	};

} // namespace wax
#endif // WAX_SDLGPUSURFACE_DOT_H
