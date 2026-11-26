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

#include <wax_sdlgpusurface.h>

#include <wax_base.h>

#include <cstring>
#include <cstdio>

namespace wax
{

	const TypeInfo SdlGpuSurface::TYPEINFO = { "SdlGpuSurface", &GpuSurface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI SdlGpuSurface::maxSize()
	{
		int max = SdlGpuContext::maxTextureSize();
		return SizeI(max, max);
	}

	//____ create() ______________________________________________________________

	SdlGpuSurface_p SdlGpuSurface::create(const Blueprint& blueprint)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return SdlGpuSurface_p();

		return SdlGpuSurface_p(new SdlGpuSurface(blueprint, PixelSource()));
	}

	SdlGpuSurface_p SdlGpuSurface::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return SdlGpuSurface_p();

		return SdlGpuSurface_p(new SdlGpuSurface(blueprint, _source(blueprint, pBlob, pitch)));
	}

	SdlGpuSurface_p SdlGpuSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return SdlGpuSurface_p();

		return SdlGpuSurface_p(new SdlGpuSurface(blueprint, _source(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize)));
	}

	SdlGpuSurface_p SdlGpuSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return SdlGpuSurface_p();

		return SdlGpuSurface_p(new SdlGpuSurface(blueprint, _source(blueprint, pPixels, format, pitch, pPalette, paletteSize)));
	}

	//____ constructor ________________________________________________________________

	SdlGpuSurface::SdlGpuSurface(const Blueprint& bp, const PixelSource& source) : GpuSurface(bp)
	{
		_setup(bp, source);
	}

	//____ destructor ____________________________________________________________

	SdlGpuSurface::~SdlGpuSurface()
	{
		_dropDeviceObjects();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SdlGpuSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setTextureFormat() _____________________________________________________
	//
	// As VulkanSurface has them, but Alpha_8 has no swizzle to read it as alpha, see the class
	// description. The 565 formats are widened to XRGB_8, which PixelTools does when pixels are
	// copied in. An sRGB surface gets an _SRGB format, so the hardware converts to linear when a
	// texel is sampled and back when a pixel is written, as our shaders work in linear.

	PixelFormat SdlGpuSurface::_setTextureFormat( PixelFormat format )
	{
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

		switch( format )
		{
			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
				format = PixelFormat::XRGB_8;
				[[fallthrough]];

			case PixelFormat::XRGB_8:
			case PixelFormat::ARGB_8:
				m_textureFormat = bSRGB ? SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB : SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
				break;

			case PixelFormat::Alpha_8:
				m_textureFormat = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
				break;

			// The texture holds the indexes, which the pixel shader looks up in the palette. The
			// color space is the palette's, see GpuTools::paletteToFloats(). SDL_GPU doesn't let
			// us sample a texture of integers, so the indexes are read as fractions of the
			// largest, see _updatePaletteColors().

			case PixelFormat::Index_8:
				m_textureFormat = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
				break;

			case PixelFormat::Index_16:
				m_textureFormat = SDL_GPU_TEXTUREFORMAT_R16_UNORM;
				break;

			default:
				m_textureFormat = SDL_GPU_TEXTUREFORMAT_INVALID;		// create() turns other formats away.
				break;
		}

		return format;
	}

	//____ _createDeviceObjects() ______________________________________________
	//
	// A canvas is rendered into, and so is a mipmapped surface, by SDL_GenerateMipmapsForGPUTexture().

	bool SdlGpuSurface::_createDeviceObjects()
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if( !pDevice )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No SDL_GPU device set. Call SdlGpuBackend::setDevice() before creating any surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		SDL_GPUTextureCreateInfo info = {};
		info.type = SDL_GPU_TEXTURETYPE_2D;
		info.format = m_textureFormat;
		info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
		info.width = uint32_t(m_size.w);
		info.height = uint32_t(m_size.h);
		info.layer_count_or_depth = 1;
		info.num_levels = uint32_t(m_mipLevels);
		info.sample_count = SDL_GPU_SAMPLECOUNT_1;

		if( m_bCanvas || m_mipLevels > 1 )
			info.usage |= SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;

		if( !SDL_GPUTextureSupportsFormat(pDevice, info.format, info.type, info.usage) )
		{
			char buffer[256];
			snprintf(buffer, sizeof(buffer), "The SDL_GPU device (%s) has no texture of the format a surface of pixel format %d needs%s.",
					 SDL_GetGPUDeviceDriver(pDevice), int(m_pixelFormat), m_bCanvas ? " for a canvas" : (m_mipLevels > 1 ? " for mipmapping" : ""));
			Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_pTexture = SDL_CreateGPUTexture(pDevice, &info);

		bool bOK = WAX_SDL_CHECK(m_pTexture != nullptr, "SDL_CreateGPUTexture");

		// One float4 per palette entry, plus one in front holding the capacity, so that an index
		// past the end of the palette - there is nothing stopping one - can be caught.

		if( bOK && m_bIndexed )
		{
			uint32_t bytes = uint32_t(m_paletteCapacity + 1) * 4 * sizeof(float);

			SDL_GPUBufferCreateInfo bufferInfo = {};
			bufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
			bufferInfo.size = bytes;

			m_pPaletteBuffer = m_pPalette ? SDL_CreateGPUBuffer(pDevice, &bufferInfo) : nullptr;
			bOK = WAX_SDL_CHECK(m_pPaletteBuffer != nullptr, "SDL_CreateGPUBuffer");

			if( bOK )
			{
				m_paletteColors.assign(bytes / sizeof(float), 0.f);
				m_pPaletteColors = m_paletteColors.data();
				_updatePaletteColors();
			}
		}

		if( !bOK )
		{
			_dropDeviceObjects();

			Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the SDL_GPU objects of a surface, it will have no texture.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		return bOK;
	}

	//____ _dropDeviceObjects() ________________________________________________
	//
	// SDL_GPU lets go of them when the GPU is done with them.

	void SdlGpuSurface::_dropDeviceObjects()
	{
		m_pulls.clear();

		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if( pDevice )
		{
			if( m_pTexture )
				SDL_ReleaseGPUTexture(pDevice, m_pTexture);

			if( m_pPaletteBuffer )
				SDL_ReleaseGPUBuffer(pDevice, m_pPaletteBuffer);
		}

		m_pTexture = nullptr;
		m_pPaletteBuffer = nullptr;

		m_bMipmapStale = false;
		m_pPaletteColors = nullptr;
		m_paletteColors.clear();
		m_bPaletteChanged = false;
	}

	//____ _updatePaletteColors() ______________________________________________
	//
	// The header of the palette has its capacity first, which GpuTools::paletteToFloats() writes,
	// and for our shaders the largest index next, which turns what they read from the texture back
	// into the index, see ../hlsl/sdlgpu.hlsl.

	void SdlGpuSurface::_updatePaletteColors()
	{
		GpuSurface::_updatePaletteColors();

		if( m_pPaletteColors )
		{
			m_pPaletteColors[1] = (m_pixelFormat == PixelFormat::Index_16) ? 65535.f : 255.f;
			m_bPaletteChanged = true;
		}
	}

	//____ TransferBuffer ______________________________________________________

	SdlGpuSurface::TransferBuffer::~TransferBuffer()
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if( pBuffer && pDevice )
		{
			unmap();
			SDL_ReleaseGPUTransferBuffer(pDevice, pBuffer);
		}
	}

	void SdlGpuSurface::TransferBuffer::unmap()
	{
		if( bMapped && SdlGpuContext::device() )
			SDL_UnmapGPUTransferBuffer(SdlGpuContext::device(), pBuffer);

		bMapped = false;
	}

	//____ _allocStaging() _____________________________________________________
	//
	// A transfer buffer, mapped. Lines start on four byte boundaries, which is a whole number of
	// pixels for all our formats, except on Metal, where they must follow each other without
	// padding, see SdlGpuContext::tightUploadRows(). Transfer buffers for uploading are all of one
	// kind, read or not.
	//
	// What is handed out as a PixelBuffer stays mapped while it is copied from, which all devices
	// of SDL_GPU allow, as they keep transfer buffers mapped or map them where they are.

	SdlGpuSurface::Staging * SdlGpuSurface::_allocStaging( SizeI size, bool bReadable )
	{
		(void) bReadable;

		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if( !m_pTexture || !pDevice )
			return nullptr;

		int pitch = SdlGpuContext::tightUploadRows() ? size.w * m_pixelBytes : (size.w * m_pixelBytes + 3) & ~3;

		SDL_GPUTransferBufferCreateInfo info = {};
		info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		info.size = uint32_t(pitch) * uint32_t(size.h);

		auto pBuffer = std::make_shared<TransferBuffer>();

		pBuffer->pBuffer = SDL_CreateGPUTransferBuffer(pDevice, &info);

		uint8_t * pMapped = pBuffer->pBuffer ? (uint8_t*) SDL_MapGPUTransferBuffer(pDevice, pBuffer->pBuffer, false) : nullptr;

		if( !pMapped )
		{
			WAX_SDL_CHECK(false, "Making a transfer buffer for pixels on their way to the texture of a surface");
			return nullptr;
		}

		pBuffer->bMapped = true;

		auto pStaging = new SdlGpuStaging();

		pStaging->pBuffer = pBuffer;
		pStaging->pPixels = pMapped;
		pStaging->pitch = pitch;

		return pStaging;
	}

	//____ _pull() _____________________________________________________________
	//
	// The copy is recorded when SdlGpuBackend is about to use us, see _recordUploads(). The
	// transfer buffer is shared between the staging memory and us, as whoever has that may let go
	// of it before then.

	void SdlGpuSurface::_pull( Staging * pStaging, const RectI& rect, CoordI dest )
	{
		if( !m_pTexture || rect.isEmpty() )
			return;

		auto p = static_cast<SdlGpuStaging*>(pStaging);

		Pull pull;

		pull.pBuffer = p->pBuffer;
		pull.offset = uint32_t(rect.y) * uint32_t(p->pitch) + uint32_t(rect.x * m_pixelBytes);
		pull.pitch = p->pitch;
		pull.size = rect.size();
		pull.dest = dest;

		// Vulkan only promises copies from offsets that are a multiple of four, and Metal reads
		// rows as long as the rectangle, see SdlGpuContext::tightUploadRows(), so such a
		// rectangle goes by a buffer of its own.

		bool bRowsTooLong = SdlGpuContext::tightUploadRows() && p->pitch != rect.w * m_pixelBytes;

		if( pull.offset % 4 != 0 || bRowsTooLong )
		{
			auto pOwn = static_cast<SdlGpuStaging*>( _allocStaging( rect.size(), false ) );

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

	//____ _recordUploads() ____________________________________________________
	//
	// Records the copies of what the CPU has written since the last call into a copy pass, which
	// comes before the draws of its command buffer.

	void SdlGpuSurface::_recordUploads( SDL_GPUCopyPass * pCopyPass, SdlGpuContext::BufferUploads& bufferUploads )
	{
		if( !m_pTexture )
			return;

		_stageDirty();				// Our own pixels that have changed, by a transfer buffer made for it.

		if( !m_pulls.empty() )
		{
			for( auto& pull : m_pulls )
			{
				SDL_GPUTextureTransferInfo src = {};
				src.transfer_buffer = pull.pBuffer->pBuffer;
				src.offset = pull.offset;
				src.pixels_per_row = uint32_t(pull.pitch / m_pixelBytes);
				src.rows_per_layer = uint32_t(pull.size.h);

				SDL_GPUTextureRegion dst = {};
				dst.texture = m_pTexture;
				dst.x = uint32_t(pull.dest.x);
				dst.y = uint32_t(pull.dest.y);
				dst.w = uint32_t(pull.size.w);
				dst.h = uint32_t(pull.size.h);
				dst.d = 1;

				// A partial upload must keep what is outside it, so no cycling.

				SDL_UploadToGPUTexture(pCopyPass, &src, &dst, false);
			}

			// The transfer buffers go once the GPU is done with the copies.

			m_pulls.clear();

			_uploaded();
		}

		if( m_bPaletteChanged && m_pPaletteBuffer )
		{
			bufferUploads.add( m_pPaletteBuffer, 0, m_paletteColors.data(), uint32_t(m_paletteColors.size() * sizeof(float)), true );
			m_bPaletteChanged = false;
		}
	}

	//____ _readBack() _________________________________________________________
	//
	// Copies a rectangle of our texture to pDest, by a transfer buffer made for it. This stalls
	// until the GPU is done, but only happens when someone asks for the pixels of a canvas. Every
	// session on a surface has been submitted when it ended, so what was rendered into us is in
	// front of us.

	bool SdlGpuSurface::_readBack( const RectI& rect, uint8_t * pDest, int destPitch )
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if( !pDevice || !m_pTexture )
			return false;

		int lineBytes = rect.w * m_pixelBytes;

		SDL_GPUTransferBufferCreateInfo info = {};
		info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
		info.size = uint32_t(lineBytes) * uint32_t(rect.h);

		SDL_GPUTransferBuffer * pTransfer = SDL_CreateGPUTransferBuffer(pDevice, &info);

		if( !WAX_SDL_CHECK(pTransfer != nullptr, "SDL_CreateGPUTransferBuffer") )
			return false;

		SDL_GPUCommandBuffer * pCmd = SDL_AcquireGPUCommandBuffer(pDevice);

		if( !WAX_SDL_CHECK(pCmd != nullptr, "SDL_AcquireGPUCommandBuffer") )
		{
			SDL_ReleaseGPUTransferBuffer(pDevice, pTransfer);
			return false;
		}

		SDL_GPUCopyPass * pCopyPass = SDL_BeginGPUCopyPass(pCmd);

		// Pixels written by the CPU since are the newer ones, so they go in on top of what was
		// rendered before we read it back.

		SdlGpuContext::BufferUploads bufferUploads;
		_recordUploads(pCopyPass, bufferUploads);
		bufferUploads.record(pCopyPass);

		SDL_GPUTextureRegion src = {};
		src.texture = m_pTexture;
		src.x = uint32_t(rect.x);
		src.y = uint32_t(rect.y);
		src.w = uint32_t(rect.w);
		src.h = uint32_t(rect.h);
		src.d = 1;

		SDL_GPUTextureTransferInfo dst = {};
		dst.transfer_buffer = pTransfer;
		dst.offset = 0;
		dst.pixels_per_row = uint32_t(rect.w);
		dst.rows_per_layer = uint32_t(rect.h);

		SDL_DownloadFromGPUTexture(pCopyPass, &src, &dst);
		SDL_EndGPUCopyPass(pCopyPass);

		bool bOK = SdlGpuContext::submitAndWait(pCmd);

		if( bOK )
		{
			auto pMapped = (const uint8_t*) SDL_MapGPUTransferBuffer(pDevice, pTransfer, false);

			bOK = WAX_SDL_CHECK(pMapped != nullptr, "SDL_MapGPUTransferBuffer");

			if( bOK )
			{
				for( int y = 0; y < rect.h; y++ )
					memcpy( pDest + y * destPitch, pMapped + y * lineBytes, lineBytes );

				SDL_UnmapGPUTransferBuffer(pDevice, pTransfer);
			}
		}

		SDL_ReleaseGPUTransferBuffer(pDevice, pTransfer);
		return bOK;
	}

} // namespace wax
