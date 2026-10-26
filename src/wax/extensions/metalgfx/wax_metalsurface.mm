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

#include <memory.h>

#include <wg_metalsurface.h>
#include <wg_metalbackend.h>
#include <wg_gfxutil.h>
#include <wg_blob.h>
#include <wg_gfxbase.h>
#include <wg_pixeltools.h>

#include <assert.h>
#include <algorithm>



namespace wg
{
	const TypeInfo MetalSurface::TYPEINFO = { "MetalSurface", &Surface::TYPEINFO };

	SizeI	MetalSurface::s_maxSize;

#define HANDLE_GLERROR(check) { GLenum err = check; if(err != 0) GlGfxDevice::onGlError(err, this, TYPEINFO, __func__, __FILE__, __LINE__ ); }

	//____ _premultiply() / _unpremultiply() ______________________________________
	// Colors of a pixel times or divided by its alpha (1 -> 255), in linear light for sRGB pixels.

	static inline void _premultiply(uint8_t& b, uint8_t& g, uint8_t& r, int a, bool bSRGB)
	{
		if (bSRGB)
		{
			b = HiColor::packSRGBTab[(HiColor::unpackSRGBTab[b] * a + 127) / 255];
			g = HiColor::packSRGBTab[(HiColor::unpackSRGBTab[g] * a + 127) / 255];
			r = HiColor::packSRGBTab[(HiColor::unpackSRGBTab[r] * a + 127) / 255];
		}
		else
		{
			b = uint8_t((b * a + 127) / 255);
			g = uint8_t((g * a + 127) / 255);
			r = uint8_t((r * a + 127) / 255);
		}
	}

	static inline void _unpremultiply(uint8_t& b, uint8_t& g, uint8_t& r, int a, bool bSRGB)
	{
		if (bSRGB)
		{
			b = HiColor::packSRGBTab[std::min(4096, (HiColor::unpackSRGBTab[b] * 255 + a / 2) / a)];
			g = HiColor::packSRGBTab[std::min(4096, (HiColor::unpackSRGBTab[g] * 255 + a / 2) / a)];
			r = HiColor::packSRGBTab[std::min(4096, (HiColor::unpackSRGBTab[r] * 255 + a / 2) / a)];
		}
		else
		{
			b = uint8_t(std::min(255, (b * 255 + a / 2) / a));
			g = uint8_t(std::min(255, (g * 255 + a / 2) / a));
			r = uint8_t(std::min(255, (r * 255 + a / 2) / a));
		}
	}

	//____ _isOpaque() ____________________________________________________________
	// Checks if all ARGB_8 pixels are opaque. Pixels are 32-bit words in native byte order
	// with alpha in the high byte.

	static bool _isOpaque(const uint8_t* pPixels, int pitch, int width, int height)
	{
		for (int y = 0; y < height; y++)
		{
			auto p = (const uint32_t*)(pPixels + y * pitch);
			for (int x = 0; x < width; x++)
				if ((p[x] >> 24) != 255)
					return false;
		}
		return true;
	}



	//____ maxSize() _______________________________________________________________

	SizeI MetalSurface::maxSize()
	{
		if (s_maxSize.w == 0)
		{
			GLint max = 16384;
			s_maxSize.w = max;
			s_maxSize.h = max;
		}

		return s_maxSize;
	}

	//____ create ______________________________________________________________

	MetalSurface_p	MetalSurface::create( const Blueprint& bp )
	{
        if( !_isBlueprintValid(bp, maxSize()) || !_isFormatSupported(bp.format) )
            return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp));
	}

	MetalSurface_p	MetalSurface::create( const Blueprint& bp, Blob* pBlob, int pitch )
	{
        if (!_isBlueprintValid(bp, maxSize()) || !_isFormatSupported(bp.format) )
            return MetalSurface_p();

        if ( !pBlob || (pitch > 0 && pitch % 4 != 0))
            return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp,pBlob,pitch));
	}

	MetalSurface_p	MetalSurface::create(const Blueprint& bp, const uint8_t* pPixels,
										 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
        if (!_isBlueprintValid(bp, maxSize()) || !_isFormatSupported(bp.format))
            return MetalSurface_p();

		return  MetalSurface_p(new MetalSurface(bp, pPixels, format, pitch, pPalette, paletteSize));
	};

	MetalSurface_p	MetalSurface::create(const Blueprint& bp, const uint8_t* pPixels,
										 const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if (!_isBlueprintValid(bp, maxSize()) || !_isFormatSupported(bp.format))
			return MetalSurface_p();

		return  MetalSurface_p(new MetalSurface(bp, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	};

	//____ constructor _____________________________________________________________



    MetalSurface::MetalSurface(const Blueprint& bp) : Surface( bp, PixelFormat::ARGB_8, SampleMethod::Bilinear )
    {
        _setPixelDetails();
        m_bMipmapped = bp.mipmap;
        _setupMetalTexture( nullptr, 0, nullptr, nullptr, 0, bp.palette );
    }

    MetalSurface::MetalSurface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface( bp, PixelFormat::ARGB_8, SampleMethod::Bilinear )
    {
        // The blob is in the layout specified by the blueprint, which might not be the one we end up with.

        PixelDescription srcDesc = m_pixelDescription;

        _setPixelDetails();
        m_bMipmapped = bp.mipmap;

        if( pitch == 0 )
            pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

        _setupMetalTexture(pBlob->data(), pitch, &srcDesc, bp.palette, m_paletteSize, bp.palette);
    }

	MetalSurface::MetalSurface(const Blueprint& bp, const uint8_t* pPixels, PixelFormat format,
							   int pitch, const Color8 * pPalette, int paletteSize)
	: Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		// Source pixels are in the byte order and color space of the blueprint.

		if( format == PixelFormat::Undefined )
			format = m_pixelFormat;

		auto srcDesc = Util::pixelFormatToDescription(format, bp.bigEndian);

		_setPixelDetails();
		m_bMipmapped = bp.mipmap;

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

		_fixSrcParam(srcDesc, pPalette, paletteSize);
		_setupMetalTexture( pPixels, pitch, &srcDesc, pPalette, paletteSize, bp.palette );
	}


	MetalSurface::MetalSurface(const Blueprint& bp, const uint8_t* pPixels, const PixelDescription& pixelDescription,
							   int pitch, const Color8 * pPalette, int paletteSize)
	: Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
    {
        _setPixelDetails();
        m_bMipmapped = bp.mipmap;
		
		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(pixelDescription, m_size.w);

		_fixSrcParam(pixelDescription, pPalette, paletteSize);
		_setupMetalTexture( pPixels, pitch, &pixelDescription, pPalette, paletteSize, bp.palette );
    }

	//____ _isFormatSupported() __________________________________________________

	bool MetalSurface::_isFormatSupported( PixelFormat format )
	{
		switch (format)
		{
			case PixelFormat::Undefined:			// Defaults to ARGB_8.
			case PixelFormat::XRGB_8:
			case PixelFormat::ARGB_8:
			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
			case PixelFormat::Index_8:
			case PixelFormat::Index_16:
			case PixelFormat::Alpha_8:
				return true;

			default:
				return false;
		}
	}

	//____ _setupMetalTexture() __________________________________________________________________

	void MetalSurface::_setupMetalTexture(const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc, const Color8 * pSrcPalette, int srcPaletteSize, const Color8 * pDstPalette )
	{
		m_bTextureSyncInProgress = false;

		// Create our shared buffer
		
		int     bufferLength = m_size.w * m_size.h * m_pixelSize;

		m_textureBuffer = [MetalBackend::s_metalDevice newBufferWithLength:bufferLength options:MTLResourceStorageModeShared];

		// Setup the palette if present
		
		if( m_paletteCapacity > 0 )
		{
			// Create the palette buffer and copy data. It is laid out like the palette texture,
			// which might have room for more entries than the capacity.

			// The source palette only has m_paletteSize entries, the rest is cleared.

			int paletteBufferLength = _paletteTextureWidth() * _paletteTextureHeight() * 4;

			m_paletteBuffer = [MetalBackend::s_metalDevice newBufferWithLength:paletteBufferLength options:MTLResourceStorageModeShared];
			m_pPalette = (Color8*) [m_paletteBuffer contents];
			memset(m_pPalette, 0, paletteBufferLength);
			if( pDstPalette )
				memcpy(m_pPalette, pDstPalette, m_paletteSize*4);
		}
		
		//
		
		// Copy pixel data to our shared buffer, converting it to our layout
			  
		if( pPixels )
		{
			if( srcPaletteSize == 0 )
				srcPaletteSize = m_paletteSize;
			
			auto pDst = (uint8_t*)[m_textureBuffer contents];

			PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, * pSrcPixelDesc, m_colorSpace, pitch - PixelTools::bytesPerLine(*pSrcPixelDesc, m_size.w),
								   pSrcPalette, srcPaletteSize,
								   pDst, m_pixelFormat, m_colorSpace, m_bBigEndian, 0,
								   m_pPalette, m_paletteSize, m_paletteCapacity);
		}
			   
		
		_createAndSyncTextures( pPixels != nullptr );
		
	//        setScaleMode(m_scaleMode);
	}

	//____ _paletteTextureWidth() ______________________________________________

	int MetalSurface::_paletteTextureWidth() const
	{
		// Index_16 can have 65536 entries, which is more than the widest texture.

		return std::min(m_paletteCapacity, 256);
	}

	//____ _paletteTextureHeight() _____________________________________________

	int MetalSurface::_paletteTextureHeight() const
	{
		int width = _paletteTextureWidth();
		return width == 0 ? 0 : (m_paletteCapacity + width - 1) / width;
	}

    //____ _createAndSyncTextures() __________________________________________________

    void MetalSurface::_createAndSyncTextures( bool bHasTextureData )
    {
        // Create the private texture
        
        MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];

        textureDescriptor.pixelFormat   = m_internalFormat;
        textureDescriptor.width         = m_size.w;
        textureDescriptor.height        = m_size.h;
        textureDescriptor.storageMode   = MTLStorageModePrivate;
        textureDescriptor.usage         = m_bCanvas ? (MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead) : MTLTextureUsageShaderRead;
        
        // A mipmapped texture gets levels all the way down to 1x1. MetalBackend
        // draws them, so it has to be a render target even when not a canvas.

        if(m_bMipmapped)
        {
            int mipCount = 1;

            for( int size = (m_size.w > m_size.h) ? m_size.w : m_size.h ; size > 1 ; size >>= 1 )
                mipCount++;

            textureDescriptor.mipmapLevelCount = mipCount;
            textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        }

        m_texture = [MetalBackend::s_metalDevice newTextureWithDescriptor:textureDescriptor];
        [textureDescriptor release];
        
        // Create the palette texture
        
        if( m_pPalette )
        {
            MTLTextureDescriptor *paletteDescriptor = [[MTLTextureDescriptor alloc] init];

            // Palette entries are in the color space of the surface.

            paletteDescriptor.pixelFormat   = m_colorSpace == ColorSpace::Linear ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatBGRA8Unorm_sRGB;
            paletteDescriptor.width         = _paletteTextureWidth();
            paletteDescriptor.height        = _paletteTextureHeight();
            paletteDescriptor.storageMode   = MTLStorageModePrivate;

            m_paletteTexture = [MetalBackend::s_metalDevice newTextureWithDescriptor:paletteDescriptor];

            [paletteDescriptor release];
        }

        // Copy from buffers to textures (pixels and palettes)
        
		if( bHasTextureData || m_pPalette )
		{
			@autoreleasepool {
				id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
				id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
				commandBuffer.label = @"_createAndSyncTextures Command Buffer";

				id<MTLBuffer> pixelUpload = nil;
				id<MTLBuffer> paletteUpload = nil;

				if( bHasTextureData )
					pixelUpload = _encodeTextureUpload(blitCommandEncoder, RectI(0, 0, m_size.w, m_size.h));

				if( m_pPalette )
					paletteUpload = _encodePaletteUpload(blitCommandEncoder);

				// MetalBackend draws the mip levels before the texture is first read,
				// see MetalBackend::_generateMipmaps().

				if(m_bMipmapped)
					m_bMipmapStale = true;
			
				[blitCommandEncoder endEncoding];
				blitCommandEncoder = nil;
	/*
				m_bTextureSyncInProgress = true;
			
				// Add a completion handler and commit the command buffer.
				[commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
					// Private texture is populated.
				
					m_bTextureSyncInProgress = false;
				}];
	*/
				[commandBuffer commit];
				[commandBuffer waitUntilCompleted];

				commandBuffer = nil;

				[pixelUpload release];
				[paletteUpload release];
			}
		}
    }

    //____ _setPixelDetails() __________________________________________________________

	void MetalSurface::_setPixelDetails()
	{
		// Pixels are copied straight between our shared buffer and the texture, so we
		// convert layouts that Metal has no texture format for to ones it has. The color
		// space stays and picks between the normal and _sRGB texture formats.

		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);
		[[maybe_unused]] bool bNativeByteOrder = (m_bBigEndian == (WG_IS_BIG_ENDIAN == 1));

		PixelFormat format = m_pixelFormat;

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
				m_internalFormat = MTLPixelFormatR8Unorm;
				break;

			case PixelFormat::Index_16:
				m_internalFormat = MTLPixelFormatRG8Unorm;			// Low byte in red, high byte in green, see paletteLookup() in the shaders.
				break;

			case PixelFormat::Alpha_8:
				m_internalFormat = MTLPixelFormatR8Unorm;
				break;

			default:
                GfxBase::throwError( ErrorLevel::Critical, ErrorCode::InvalidParam, "Specified PixelFormat not supported", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
				assert(false);           // Just to really catch this in a clear way during development.
				break;
		}

		m_pixelFormat = format;
		m_bBigEndian = (WG_IS_BIG_ENDIAN == 1);
		m_pixelDescription = Util::pixelFormatToDescription(format, m_bBigEndian);
        m_pixelSize = m_pPixelDescription->bits / 8;
	}

	//____ Destructor ______________________________________________________________

	MetalSurface::~MetalSurface()
	{
		// Free the stuff

        [m_texture release];
        [m_textureBuffer release];
        
        [m_paletteTexture release];
        [m_paletteBuffer release];
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

    //____ allocPixelBuffer() ______________________________________________________

    const PixelBuffer MetalSurface::allocPixelBuffer(const RectI& rect)
    {
        PixelBuffer pixbuf;

        pixbuf.format = m_pixelFormat;
        pixbuf.colorSpace = m_colorSpace;
        pixbuf.bigEndian = m_bBigEndian;
        pixbuf.palette = m_pPalette;
        pixbuf.pitch = m_size.w * m_pixelSize;
        pixbuf.pixels = ((uint8_t*)[m_textureBuffer contents]) + rect.y * pixbuf.pitch + rect.x * m_pixelSize;
        pixbuf.rect = rect;
        return pixbuf;
    }

    //____ pushPixels() ____________________________________________________________

    bool MetalSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
    {
		//TODO: Only copy pixels that are needed, not whole texture.
		
        if( !m_texture )
            return false;
        
        // Make sure we have any changes made by GPU
         
         if( m_bBufferNeedsSync )
             _syncBufferAndWait();
         else
             _waitForSyncedTexture();
        
        return true;
    }

    //____ pullPixels() _____________________________________________________________

    void MetalSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
    {
        if( m_texture && !bufferRect.isEmpty() )
            _syncTexture( bufferRect + buffer.rect.pos() );

        // Colors might have been added to or changed in the palette.

        if( m_paletteTexture )
            _syncPalette();

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
    }

    //____ freePixelBuffer() ________________________________________________________

    void MetalSurface::freePixelBuffer(const PixelBuffer& buffer)
    {
        // Do nothing.
    }

	//____ alpha() ____________________________________________________________

	int MetalSurface::alpha( CoordSPX _coord )
	{
        if( m_bBufferNeedsSync )
            _syncBufferAndWait();

		// No need to free the PixelBuffer, we know how our pixel buffer works.

		return _alpha( _coord, allocPixelBuffer() );
	}

	//____ unload() ___________________________________________________________

	bool MetalSurface::unload()
	{
		if( m_texture == 0 )
			return true;

		m_texture = nil;
        m_paletteTexture = nil;

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
        _createAndSyncTextures(true);
	}

	//____ _syncBufferAndWait() ____________________________________________

	void MetalSurface::_syncBufferAndWait()
	{
        MTLSize textureSize = { (unsigned) m_size.w, (unsigned) m_size.h,1};
        MTLOrigin textureOrigin = { 0, 0, 0};
        
        @autoreleasepool {
	        id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
			commandBuffer.label = @"_syncBufferAndWait Command Buffer";

		
	        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
	        [blitCommandEncoder     copyFromTexture:    m_texture
	                                sourceSlice:        0
	                                sourceLevel:        0
	                                sourceOrigin:       textureOrigin
	                                sourceSize:         textureSize
	                                toBuffer:           m_textureBuffer
	                                destinationOffset:  0
	                                destinationBytesPerRow: m_pixelSize * m_size.w
	                                destinationBytesPerImage: m_pixelSize * m_size.w * m_size.h ];
	        [blitCommandEncoder endEncoding];
	        blitCommandEncoder = nil;

	        [commandBuffer commit];

	        [commandBuffer waitUntilCompleted];
	        commandBuffer = nil;
        }

        if( _isPremultiplied() )
            _convertAlpha((uint8_t*)[m_textureBuffer contents], m_size.w * m_pixelSize, m_size.w, m_size.h, false);

        m_bBufferNeedsSync = false;

    }

    //____ _syncTexture() _______________________________________________

    void MetalSurface::_syncTexture(RectI region)
{
		@autoreleasepool {
			id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
			commandBuffer.label = @"_syncTexture Command Buffer";

			id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
			id<MTLBuffer> upload = _encodeTextureUpload(blitCommandEncoder, region);

	        if(m_bMipmapped)
	            m_bMipmapStale = true;
	//            [blitCommandEncoder generateMipmapsForTexture:m_texture];
        
	        [blitCommandEncoder endEncoding];
	        blitCommandEncoder = nil;
        
	/*
			 m_bTextureSyncInProgress = true;

	        // Add a completion handler and commit the command buffer.
	        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
	            // Private texture is populated.
            
	            m_bTextureSyncInProgress = false;
	        }];
	 */
	        [commandBuffer commit];
			[commandBuffer waitUntilCompleted];

	        commandBuffer = nil;
	        [upload release];
		}

//        _waitForSyncedTexture();
    }


    //____ _syncPalette() _______________________________________________

    void MetalSurface::_syncPalette()
    {
        @autoreleasepool {
	        id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
	        commandBuffer.label = @"_syncPalette Command Buffer";

	        id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
	        id<MTLBuffer> upload = _encodePaletteUpload(blitCommandEncoder);
	        [blitCommandEncoder endEncoding];
	        blitCommandEncoder = nil;

	        [commandBuffer commit];
	        [commandBuffer waitUntilCompleted];
	        commandBuffer = nil;
	        [upload release];
        }
    }

    //____ _encodeTextureUpload() _______________________________________________
    //
    // Encodes a copy of a region of our shared buffer to the texture. Surfaces with alpha
    // keep premultiplied colors in the texture, see MetalBackend::_compileRenderPipeline(),
    // so their pixels are premultiplied on the way, in a buffer of their own, unless all
    // are opaque. Returns that buffer or nil. The caller releases it once copied.

    id<MTLBuffer> MetalSurface::_encodeTextureUpload( id<MTLBlitCommandEncoder> encoder, RectI region )
    {
        int pitch = m_size.w * m_pixelSize;
        int offset = region.y * pitch + region.x * m_pixelSize;

        id<MTLBuffer> source = m_textureBuffer;
        id<MTLBuffer> premultiplied = nil;

        if( _isPremultiplied() )
        {
            const uint8_t * pPixels = ((const uint8_t*) [m_textureBuffer contents]) + offset;

            if( !_isOpaque(pPixels, pitch, region.w, region.h) )
            {
                int rowBytes = region.w * m_pixelSize;

                premultiplied = [MetalBackend::s_metalDevice newBufferWithLength:rowBytes * region.h options:MTLResourceStorageModeShared];

                auto pDest = (uint8_t*) [premultiplied contents];
                for( int y = 0 ; y < region.h ; y++ )
                    memcpy(pDest + y * rowBytes, pPixels + y * pitch, rowBytes);

                _convertAlpha(pDest, rowBytes, region.w, region.h, true);

                source = premultiplied;
                offset = 0;
                pitch = rowBytes;
            }
        }

        MTLSize textureSize = { (unsigned) region.w, (unsigned) region.h, 1 };
        MTLOrigin textureOrigin = { (unsigned) region.x, (unsigned) region.y, 0 };

        [encoder copyFromBuffer:     source
                    sourceOffset:       offset
               sourceBytesPerRow:  pitch
             sourceBytesPerImage:0
                      sourceSize:         textureSize
                       toTexture:          m_texture
                destinationSlice:   0
                destinationLevel:   0
               destinationOrigin:  textureOrigin];

        return premultiplied;
    }

    //____ _encodePaletteUpload() _______________________________________________
    //
    // Encodes a copy of the palette to the palette texture. Entries are premultiplied, like
    // the pixels of surfaces with alpha, so that interpolating between them interpolates
    // premultiplied colors. That is done in a buffer of its own, which is returned for the
    // caller to release once copied.

    id<MTLBuffer> MetalSurface::_encodePaletteUpload( id<MTLBlitCommandEncoder> encoder )
    {
        int nEntries = _paletteTextureWidth() * _paletteTextureHeight();

        id<MTLBuffer> premultiplied = [MetalBackend::s_metalDevice newBufferWithBytes:m_pPalette length:nEntries * 4 options:MTLResourceStorageModeShared];

        bool bSRGB = (m_colorSpace == ColorSpace::sRGB);
        auto pEntries = (Color8*) [premultiplied contents];

        for( int i = 0 ; i < nEntries ; i++ )
        {
            Color8& c = pEntries[i];
            if( c.a == 0 )
                c.argb = 0;
            else if( c.a < 255 )
                _premultiply(c.b, c.g, c.r, c.a, bSRGB);
        }

        MTLSize paletteSize = { (unsigned long) _paletteTextureWidth(), (unsigned long) _paletteTextureHeight(), 1 };
        MTLOrigin paletteOrigin = {0,0,0};

        [encoder copyFromBuffer:     premultiplied
                    sourceOffset:       0
               sourceBytesPerRow:  _paletteTextureWidth()*4
             sourceBytesPerImage:0
                      sourceSize:         paletteSize
                       toTexture:          m_paletteTexture
                destinationSlice:   0
                destinationLevel:   0
               destinationOrigin:  paletteOrigin];

        return premultiplied;
    }

    //____ _convertAlpha() ______________________________________________________
    //
    // Premultiplies or unpremultiplies ARGB_8 pixels in place. Pixels are 32-bit words in
    // native byte order with alpha in the high byte.

    void MetalSurface::_convertAlpha( uint8_t * pPixels, int pitch, int width, int height, bool bPremultiply ) const
    {
        bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

        for( int y = 0 ; y < height ; y++ )
        {
            auto p = (uint32_t*)(pPixels + y * pitch);

            for( int x = 0 ; x < width ; x++ )
            {
                uint32_t v = p[x];

                int a = v >> 24;
                if( a == 255 )
                    continue;

                if( a == 0 )
                    v = 0;
                else
                {
                    uint8_t b = uint8_t(v), g = uint8_t(v >> 8), r = uint8_t(v >> 16);

                    if( bPremultiply )
                        _premultiply(b, g, r, a, bSRGB);
                    else
                        _unpremultiply(b, g, r, a, bSRGB);

                    v = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | b;
                }

                p[x] = v;
            }
        }
    }

    //____ _waitForSyncedTexture() _______________________________________________

    void MetalSurface::_waitForSyncedTexture()
    {
        while( m_bTextureSyncInProgress )
            usleep(20);        // Sleep for 0.02 millisec
    }



} // namespace wg
