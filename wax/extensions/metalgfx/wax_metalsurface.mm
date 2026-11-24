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
	//
	// Metal copies between buffer and texture with any number of bytes per line, so our lines
	// have no padding.

	MetalSurface::MetalSurface(const Blueprint& bp, const PixelSource& source) : GpuSurface(bp, 1)
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
	// Pixels are copied straight between our shared buffer and the texture, so layouts that
	// Metal has no texture format for are kept as one it has. The color space stays and picks
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

	bool MetalSurface::_createDeviceObjects( bool bPixelBuffer )
	{
		if( MetalBackend::s_metalDevice == nil )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Metal device set. Call MetalBackend::setMetalDevice() before creating any surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		bool bOK = _createTexture();

		// Our pixels, unless they can wait until they are asked for.

		if( bOK && bPixelBuffer )
			bOK = _createPixelBuffer();

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

	//____ _createPixelBuffer() ________________________________________________
	//
	// Our pixels, which are also where what is rendered into us is read back to. Metal gives
	// us the buffer zeroed.

	bool MetalSurface::_createPixelBuffer()
	{
		if( m_texture == nil )
			return false;

		m_textureBuffer = [MetalBackend::s_metalDevice newBufferWithLength:size_t(m_pitch) * m_size.h options:MTLResourceStorageModeShared];

		if( m_textureBuffer == nil )
			return false;

		m_pPixels = (uint8_t*) [m_textureBuffer contents];
		return true;
	}

	//____ _dropDeviceObjects() ________________________________________________
	//
	// A command buffer that still reads any of it keeps it until it is done.

	void MetalSurface::_dropDeviceObjects()
	{
		[m_lastUpload release];
		[m_sampleTexture release];
		[m_texture release];
		[m_textureBuffer release];
		[m_paletteColors release];

		m_lastUpload = nil;
		m_sampleTexture = nil;
		m_texture = nil;
		m_textureBuffer = nil;
		m_paletteColors = nil;

		m_bMipmapStale = false;
		m_pPixels = m_pFallbackPixels;
		m_pPaletteColors = nullptr;
	}

	//____ unload() ___________________________________________________________

	bool MetalSurface::unload()
	{
		[m_sampleTexture release];
		[m_texture release];

		m_sampleTexture = nil;
		m_texture = nil;

		return true;
	}

	//____ isLoaded() _________________________________________________________

	bool MetalSurface::isLoaded()
	{
		return (m_texture != nil);
	}

	//____ reload() _________________________________________________________

	void MetalSurface::reload()
	{
		if( m_texture != nil || MetalBackend::s_metalDevice == nil || m_pFallbackPixels )
			return;

		if( !_createTexture() )
		{
			unload();
			return;
		}

		// Everything we have goes to the new texture.

		m_bBufferNeedsSync = false;

		if( m_pPixels )
			_addDirtyRect( RectI(m_size) );
	}

	//____ syncTexture() _______________________________________________________
	//
	// The copy reads our pixels when the GPU gets to it, unless they had to be premultiplied
	// first, so the CPU must not write to them until it is done. allocPixelBuffer() waits for
	// that, see _waitForUpload().

	void MetalSurface::syncTexture()
	{
		if( m_dirtyRect.isEmpty() || m_texture == nil || m_textureBuffer == nil )
			return;

		@autoreleasepool {
			id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
			commandBuffer.label = @"MetalSurface upload";

			id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
			id<MTLBuffer> premultiplied = _encodeTextureUpload(blitCommandEncoder, m_dirtyRect);
			[blitCommandEncoder endEncoding];

			[commandBuffer commit];

			[premultiplied release];			// The command buffer keeps it until it is done.

			[m_lastUpload release];
			m_lastUpload = [commandBuffer retain];
		}

		// Level 0 has changed, MetalBackend draws the others from it before the surface is read,
		// see MetalBackend::_generateMipmaps().

		_uploaded();
	}

	//____ _waitForUpload() ____________________________________________________

	void MetalSurface::_waitForUpload()
	{
		if( m_lastUpload == nil )
			return;

		[m_lastUpload waitUntilCompleted];
		[m_lastUpload release];
		m_lastUpload = nil;
	}

	//____ _readBack() _________________________________________________________
	//
	// Copies our texture into our pixels. This stalls until the GPU is done, but only happens
	// when someone actually asks to read a canvas surface.

	bool MetalSurface::_readBack()
	{
		if( m_texture == nil || m_textureBuffer == nil )
			return false;

		// Pixels written by the CPU since are the newer ones, so they go in on top of what was
		// rendered before we read it all back.

		syncTexture();

		MTLSize textureSize = { (unsigned) m_size.w, (unsigned) m_size.h, 1 };
		MTLOrigin textureOrigin = { 0, 0, 0 };

		@autoreleasepool {
			id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
			commandBuffer.label = @"MetalSurface read back";

			id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
			[blitCommandEncoder		copyFromTexture:	m_texture
									sourceSlice:		0
									sourceLevel:		0
									sourceOrigin:		textureOrigin
									sourceSize:			textureSize
									toBuffer:			m_textureBuffer
									destinationOffset:	0
									destinationBytesPerRow: m_pitch
									destinationBytesPerImage: m_pitch * m_size.h ];
			[blitCommandEncoder endEncoding];

			[commandBuffer commit];
			[commandBuffer waitUntilCompleted];
		}

		// Any upload of ours was ahead of this in the queue, so it is done too.

		[m_lastUpload release];
		m_lastUpload = nil;

		return true;
	}

	//____ _encodeTextureUpload() _______________________________________________
	//
	// Encodes a copy of a region of our pixels to the texture. Surfaces with alpha keep
	// premultiplied colors in the texture, see MetalBackend::_compileRenderPipeline(), so their
	// pixels are premultiplied on the way, in a buffer of their own, unless they are the same
	// premultiplied. Returns that buffer or nil. The caller releases it once committed.

	id<MTLBuffer> MetalSurface::_encodeTextureUpload( id<MTLBlitCommandEncoder> encoder, RectI region )
	{
		int pitch = m_pitch;
		int offset = region.y * m_pitch + region.x * m_pixelBytes;

		id<MTLBuffer> source = m_textureBuffer;
		id<MTLBuffer> premultiplied = nil;

		if( _needsPremultiplying(region) )
		{
			int rowBytes = region.w * m_pixelBytes;

			premultiplied = [MetalBackend::s_metalDevice newBufferWithLength:size_t(rowBytes) * region.h options:MTLResourceStorageModeShared];

			if( premultiplied != nil )
			{
				_premultiply( region, (uint8_t*) [premultiplied contents], rowBytes );

				source = premultiplied;
				offset = 0;
				pitch = rowBytes;
			}
			else
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for premultiplied pixels, colors of transparent pixels will be off.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		MTLSize textureSize = { (unsigned) region.w, (unsigned) region.h, 1 };
		MTLOrigin textureOrigin = { (unsigned) region.x, (unsigned) region.y, 0 };

		[encoder copyFromBuffer:	source
				   sourceOffset:	offset
			  sourceBytesPerRow:	pitch
			sourceBytesPerImage:	0
					 sourceSize:	textureSize
					  toTexture:	m_texture
			   destinationSlice:	0
			   destinationLevel:	0
			  destinationOrigin:	textureOrigin];

		return premultiplied;
	}

} // namespace wax
