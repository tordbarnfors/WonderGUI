/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wax_metalsurface.h>
#include <wax_metalbackend.h>
#include <wax_base.h>

#include <cstring>

namespace wax
{
	const TypeInfo MetalSurface::TYPEINFO = { "MetalSurface", &GpuSurface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI MetalSurface::maxSize()
	{
		return SizeI(16384, 16384);
	}

	//____ create ______________________________________________________________

	MetalSurface_p	MetalSurface::create( const Blueprint& bp )
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp, PixelSource()));
	}

	MetalSurface_p	MetalSurface::create( const Blueprint& bp, Blob* pBlob, int pitch )
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp, _source(bp, pBlob, pitch)));
	}

	MetalSurface_p	MetalSurface::create(const Blueprint& bp, const uint8_t* pPixels,
										 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp, _source(bp, pPixels, format, pitch, pPalette, paletteSize)));
	}

	MetalSurface_p	MetalSurface::create(const Blueprint& bp, const uint8_t* pPixels,
										 const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp, _source(bp, pPixels, pixelDescription, pitch, pPalette, paletteSize)));
	}

	//____ constructor _____________________________________________________________

	MetalSurface::MetalSurface(const Blueprint& bp, const PixelSource& source) : GpuSurface(bp)
	{
		_setup(bp, source);
	}

	//____ Destructor ______________________________________________________________

	MetalSurface::~MetalSurface()
	{
		_dropDeviceObjects();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setTextureFormat() _____________________________________________________
	//
	// Pixels are copied straight between buffers and the texture, so layouts that Metal has
	// no texture format for are kept as one it has. The color space stays and picks
	// between the normal and _sRGB texture formats.

	PixelFormat MetalSurface::_setTextureFormat( PixelFormat format )
	{
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);
		[[maybe_unused]] bool bNativeByteOrder = (m_bBigEndian == (WAX_IS_BIG_ENDIAN == 1));

		switch (format)
		{
			case PixelFormat::RGB_565:
#if TARGET_OS_IPHONE
				if( !bSRGB && bNativeByteOrder && !m_bCanvas )
				{
					m_internalFormat = MTLPixelFormatB5G6R5Unorm;		// Red in the high bits, blue in the low.
					break;
				}
#endif
				[[fallthrough]];

			case PixelFormat::BGR_565:
				format = PixelFormat::XRGB_8;
				[[fallthrough]];

			case PixelFormat::XRGB_8:
			case PixelFormat::ARGB_8:
				m_internalFormat = bSRGB ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
				break;

			case PixelFormat::Index_8:
				m_internalFormat = MTLPixelFormatR8Uint;			// Read as integers, see ps_blit_palette.hlsl.
				break;

			case PixelFormat::Index_16:
				m_internalFormat = MTLPixelFormatR16Uint;
				break;

			case PixelFormat::Alpha_8:
				m_internalFormat = MTLPixelFormatR8Unorm;
				break;

			default:
				m_internalFormat = MTLPixelFormatInvalid;			// create() turns other formats away.
				break;
		}

		return format;
	}

	//____ _createDeviceObjects() ______________________________________________

	bool MetalSurface::_createDeviceObjects()
	{
		if( MetalBackend::s_metalDevice == nil )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Metal device set. Call MetalBackend::setMetalDevice() before creating any surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		bool bOK = _createTexture();

		// One float4 per palette entry, plus one in front holding the capacity, which the shaders
		// keep indexes below.

		if( bOK && m_bIndexed )
		{
			m_paletteColors = [MetalBackend::s_metalDevice newBufferWithLength:size_t(m_paletteCapacity + 1) * 4 * sizeof(float) options:MTLResourceStorageModeShared];

			bOK = (m_paletteColors != nil && m_pPalette);

			if( bOK )
			{
				m_pPaletteColors = (float*) [m_paletteColors contents];
				_updatePaletteColors();
			}
		}

		if( !bOK )
		{
			_dropDeviceObjects();

			Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the Metal objects of a surface, it will have no texture.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		return bOK;
	}

	//____ _createTexture() ____________________________________________________

	bool MetalSurface::_createTexture()
	{
		MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];

		textureDescriptor.pixelFormat   = m_internalFormat;
		textureDescriptor.width         = m_size.w;
		textureDescriptor.height        = m_size.h;
		textureDescriptor.storageMode   = MTLStorageModePrivate;
		textureDescriptor.usage         = m_bCanvas ? (MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead) : MTLTextureUsageShaderRead;

		// The shaders read alpha from Alpha_8 and need alpha 1 from XRGB_8, see sampleTexture().
		// Neither is what Metal reads from the texture, so they get a swizzled view, which a
		// render target can't be itself.

		bool bSwizzled = (m_pixelFormat == PixelFormat::Alpha_8 || m_pixelFormat == PixelFormat::XRGB_8);

		// MetalBackend draws the mip levels, so the texture has to be a render target even when
		// not a canvas.

		if( m_mipLevels > 1 )
		{
			textureDescriptor.mipmapLevelCount = m_mipLevels;
			textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
		}

		if( bSwizzled )
			textureDescriptor.usage |= MTLTextureUsagePixelFormatView;

		m_texture = [MetalBackend::s_metalDevice newTextureWithDescriptor:textureDescriptor];
		[textureDescriptor release];

		if( m_texture == nil )
			return false;

		if( bSwizzled )
		{
			MTLTextureSwizzleChannels swizzle;

			if( m_pixelFormat == PixelFormat::Alpha_8 )
				swizzle = MTLTextureSwizzleChannelsMake(MTLTextureSwizzleZero, MTLTextureSwizzleZero, MTLTextureSwizzleZero, MTLTextureSwizzleRed);
			else
				swizzle = MTLTextureSwizzleChannelsMake(MTLTextureSwizzleRed, MTLTextureSwizzleGreen, MTLTextureSwizzleBlue, MTLTextureSwizzleOne);

			m_sampleTexture = [m_texture newTextureViewWithPixelFormat:m_internalFormat
														   textureType:MTLTextureType2D
																levels:NSMakeRange(0, m_texture.mipmapLevelCount)
																slices:NSMakeRange(0, 1)
															   swizzle:swizzle];

			if( m_sampleTexture == nil )
				return false;
		}

		return true;
	}

	//____ _dropDeviceObjects() ________________________________________________
	//
	// A command buffer that still reads any of it keeps it until it is done.

	void MetalSurface::_dropDeviceObjects()
	{
		_dropPulls();

		[m_sampleTexture release];
		[m_texture release];
		[m_paletteColors release];

		m_sampleTexture = nil;
		m_texture = nil;
		m_paletteColors = nil;

		m_bMipmapStale = false;
		m_pPaletteColors = nullptr;
	}

	//____ _dropPulls() ________________________________________________________

	void MetalSurface::_dropPulls()
	{
		for( auto& pull : m_pulls )
			[pull.buffer release];

		m_pulls.clear();
	}

	//____ MetalStaging ________________________________________________________

	MetalSurface::MetalStaging::~MetalStaging()
	{
		[buffer release];
	}

	//____ _allocStaging() _____________________________________________________
	//
	// A shared buffer the GPU copies to our private texture from. Metal copies between buffer
	// and texture with any number of bytes per line, so the lines have no padding, and shared
	// memory reads as fast as any, so what is handed out is the same as what isn't.

	MetalSurface::Staging * MetalSurface::_allocStaging( SizeI size, bool /*bReadable*/ )
	{
		if( m_texture == nil || MetalBackend::s_metalDevice == nil )
			return nullptr;

		int pitch = size.w * m_pixelBytes;

		id<MTLBuffer> buffer = [MetalBackend::s_metalDevice newBufferWithLength:size_t(pitch) * size.h options:MTLResourceStorageModeShared];

		if( buffer == nil )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for pixels on their way to the texture of a surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		auto pStaging = new MetalStaging();

		pStaging->buffer = buffer;
		pStaging->pPixels = (uint8_t*) [buffer contents];
		pStaging->pitch = pitch;

		return pStaging;
	}

	//____ _pull() _____________________________________________________________
	//
	// The copy is encoded when MetalBackend is about to use us, see syncTexture(). We keep the
	// buffer until then, as whoever has the staging memory may let go of it before.

	void MetalSurface::_pull( Staging * pStaging, const RectI& rect, CoordI dest )
	{
		if( m_texture == nil || rect.isEmpty() )
			return;

		auto p = static_cast<MetalStaging*>(pStaging);

		Pull pull;

		pull.buffer = [p->buffer retain];
		pull.offset = rect.y * p->pitch + rect.x * m_pixelBytes;
		pull.pitch = p->pitch;
		pull.size = rect.size();
		pull.dest = dest;

		m_pulls.push_back(pull);
	}

	//____ syncTexture() _______________________________________________________
	//
	// The copies read their buffers when the GPU gets to them, which the command buffer keeps
	// until it is done. Nothing on the GPU reads our pixels, so they can be written to at any
	// time.

	void MetalSurface::syncTexture()
	{
		if( m_texture == nil )
			return;

		_stageDirty();				// Our own pixels that have changed, by a buffer made for it.

		if( m_pulls.empty() )
			return;

		@autoreleasepool {
			id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
			commandBuffer.label = @"MetalSurface upload";

			id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];

			for( auto& pull : m_pulls )
			{
				MTLSize textureSize = { (unsigned) pull.size.w, (unsigned) pull.size.h, 1 };
				MTLOrigin textureOrigin = { (unsigned) pull.dest.x, (unsigned) pull.dest.y, 0 };

				[blitCommandEncoder copyFromBuffer:	pull.buffer
									  sourceOffset:	pull.offset
								 sourceBytesPerRow:	pull.pitch
							   sourceBytesPerImage:	0
										sourceSize:	textureSize
										 toTexture:	m_texture
								  destinationSlice:	0
								  destinationLevel:	0
								 destinationOrigin:	textureOrigin];
			}

			[blitCommandEncoder endEncoding];

			[commandBuffer commit];
		}

		_dropPulls();				// The command buffer keeps the buffers until it is done.

		// Level 0 has changed, MetalBackend draws the others from it before the surface is read,
		// see MetalBackend::_generateMipmaps().

		_uploaded();
	}

	//____ _readBack() _________________________________________________________
	//
	// Copies a rectangle of our texture to pDest, by a buffer made for it. This stalls until the
	// GPU is done, but only happens when someone asks for the pixels of a canvas.

	bool MetalSurface::_readBack( const RectI& rect, uint8_t * pDest, int destPitch )
	{
		if( m_texture == nil )
			return false;

		// Pixels written by the CPU since are the newer ones, so they go in on top of what was
		// rendered before we read it back. They are ahead of us in the queue.

		syncTexture();

		int pitch = rect.w * m_pixelBytes;

		id<MTLBuffer> buffer = [MetalBackend::s_metalDevice newBufferWithLength:size_t(pitch) * rect.h options:MTLResourceStorageModeShared];

		if( buffer == nil )
			return false;

		MTLSize textureSize = { (unsigned) rect.w, (unsigned) rect.h, 1 };
		MTLOrigin textureOrigin = { (unsigned) rect.x, (unsigned) rect.y, 0 };

		@autoreleasepool {
			id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
			commandBuffer.label = @"MetalSurface read back";

			id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
			[blitCommandEncoder		copyFromTexture:	m_texture
									sourceSlice:		0
									sourceLevel:		0
									sourceOrigin:		textureOrigin
									sourceSize:			textureSize
									toBuffer:			buffer
									destinationOffset:	0
									destinationBytesPerRow: pitch
									destinationBytesPerImage: pitch * rect.h ];
			[blitCommandEncoder endEncoding];

			[commandBuffer commit];
			[commandBuffer waitUntilCompleted];
		}

		const uint8_t * pSrc = (const uint8_t*) [buffer contents];

		for( int y = 0; y < rect.h; y++ )
			memcpy( pDest + y * destPitch, pSrc + y * pitch, pitch );

		[buffer release];
		return true;
	}

} // namespace wax
