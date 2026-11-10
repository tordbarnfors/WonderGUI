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

#include <wax_metalsurface.h>
#include <wax_metalbackend.h>
#include <wax_util.h>
#include <wax_blob.h>
#include <wax_base.h>
#include <wax_pixeltools.h>

#include <assert.h>
#include <algorithm>



namespace wax
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
			// Create the palette buffer and copy data. The source palette only has m_paletteSize
			// entries, the rest is cleared.

			int paletteBufferLength = m_paletteCapacity * 4;

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

        // The shaders read alpha from Alpha_8 and need alpha 1 from XRGB_8, see sampleTexture().
        // Neither is what Metal reads from the texture, so they get a swizzled view, which a
        // render target can't be itself.

        bool bSwizzled = (m_pixelFormat == PixelFormat::Alpha_8 || m_pixelFormat == PixelFormat::XRGB_8);
        
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

        if( bSwizzled )
            textureDescriptor.usage |= MTLTextureUsagePixelFormatView;

        m_texture = [MetalBackend::s_metalDevice newTextureWithDescriptor:textureDescriptor];
        [textureDescriptor release];

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
        }

        // The palette as the shaders read it.

        if( m_pPalette )
        {
            if( m_paletteColors == nil )
                m_paletteColors = [MetalBackend::s_metalDevice newBufferWithLength:size_t(m_paletteCapacity + 1) * 4 * sizeof(float) options:MTLResourceStorageModeShared];

            _updatePaletteColors();
        }

        // Copy pixels from the buffer to the texture.

		if( bHasTextureData )
		{
			@autoreleasepool {
				id<MTLCommandBuffer> commandBuffer = [MetalBackend::s_metalCommandQueue commandBuffer];
				id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
				commandBuffer.label = @"_createAndSyncTextures Command Buffer";

				id<MTLBuffer> pixelUpload = _encodeTextureUpload(blitCommandEncoder, RectI(0, 0, m_size.w, m_size.h));

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
		[[maybe_unused]] bool bNativeByteOrder = (m_bBigEndian == (WAX_IS_BIG_ENDIAN == 1));

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
				m_internalFormat = MTLPixelFormatR8Uint;			// Read as integers, see ps_blit_palette.hlsl.
				break;

			case PixelFormat::Index_16:
				m_internalFormat = MTLPixelFormatR16Uint;			// In native byte order, as the buffer is.
				break;

			case PixelFormat::Alpha_8:
				m_internalFormat = MTLPixelFormatR8Unorm;
				break;

			default:
                Base::throwError( ErrorLevel::Critical, ErrorCode::InvalidParam, "Specified PixelFormat not supported", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
				assert(false);           // Just to really catch this in a clear way during development.
				break;
		}

		m_pixelFormat = format;
		m_bBigEndian = (WAX_IS_BIG_ENDIAN == 1);
		m_pixelDescription = Util::pixelFormatToDescription(format, m_bBigEndian);
        m_pixelSize = m_pPixelDescription->bits / 8;
	}

	//____ Destructor ______________________________________________________________

	MetalSurface::~MetalSurface()
	{
		// Free the stuff

        [m_sampleTexture release];
        [m_texture release];
        [m_textureBuffer release];

        [m_paletteBuffer release];
        [m_paletteColors release];
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

        if( m_paletteColors )
            _updatePaletteColors();

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

		[m_sampleTexture release];
		m_sampleTexture = nil;
		[m_texture release];
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

    //____ _updatePaletteColors() _____________________________________________
    //
    // The palette goes to the GPU converted to linear, the same way SoftSurface unpacks it, and
    // premultiplied, like the pixels of surfaces with alpha, so the shaders can use the colors as
    // they are and interpolate between them. One float4 per entry, plus one in front holding the
    // capacity, which the shaders keep indexes below. Done as DX12Surface does it.
    //
    // The buffer is read by the GPU where it is. Should the palette change while a frame using it
    // is still in flight, that frame may see the new colors.

    void MetalSurface::_updatePaletteColors()
    {
        if( !m_paletteColors || !m_pPalette )
            return;

        float * p = (float *) [m_paletteColors contents];

        p[0] = float(m_paletteCapacity);		// Exact, far below where floats lose integers.
        p[1] = 0.f;
        p[2] = 0.f;
        p[3] = 0.f;
        p += 4;

        // Palette entries are in the color space of the surface.

        bool bLinear = (m_colorSpace == ColorSpace::Linear);

        for( int i = 0 ; i < m_paletteCapacity ; i++ )
        {
            HiColor col = m_pPalette[i];		// Same color space, just more bits.

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



} // namespace wax
