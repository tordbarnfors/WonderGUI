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
#include <wg_metalgfxdevice.h>
#include <wg_gfxutil.h>
#include <wg_blob.h>
#include <wg_gfxbase.h>
#include <wg_pixeltools.h>

#include <assert.h>



namespace wg
{
	const TypeInfo MetalSurface::TYPEINFO = { "MetalSurface", &Surface::TYPEINFO };

	SizeI	MetalSurface::s_maxSize;

#define HANDLE_GLERROR(check) { GLenum err = check; if(err != 0) GlGfxDevice::onGlError(err, this, TYPEINFO, __func__, __FILE__, __LINE__ ); }



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
        if( !_isBlueprintValid(bp, maxSize()) )
            return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp));
	}

	MetalSurface_p	MetalSurface::create( const Blueprint& bp, Blob* pBlob, int pitch )
	{
        if (!_isBlueprintValid(bp, maxSize()) )
            return MetalSurface_p();

        if ( !pBlob || (pitch > 0 && pitch % 4 != 0))
            return MetalSurface_p();

		return MetalSurface_p(new MetalSurface(bp,pBlob,pitch));
	}

	MetalSurface_p	MetalSurface::create(const Blueprint& bp, const uint8_t* pPixels,
										 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
        if (!_isBlueprintValid(bp, maxSize()))
            return MetalSurface_p();

		return  MetalSurface_p(new MetalSurface(bp, pPixels, format, pitch, pPalette, paletteSize));
	};

	MetalSurface_p	MetalSurface::create(const Blueprint& bp, const uint8_t* pPixels,
										 const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if (!_isBlueprintValid(bp, maxSize()))
			return MetalSurface_p();

		return  MetalSurface_p(new MetalSurface(bp, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	};

	//____ constructor _____________________________________________________________



    MetalSurface::MetalSurface(const Blueprint& bp) : Surface( bp, PixelFormat::BGRA_8, SampleMethod::Bilinear )
    {
        _setPixelDetails(m_pixelFormat);
        m_bMipmapped = bp.mipmap;
        _setupMetalTexture( nullptr, 0, PixelFormat::Undefined, nullptr, nullptr, bp.palette, 0 );

    }

    MetalSurface::MetalSurface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface( bp, PixelFormat::BGRA_8, SampleMethod::Bilinear )
    {
        // Set general information

        _setPixelDetails(m_pixelFormat);
        m_bMipmapped = bp.mipmap;
        _setupMetalTexture(pBlob->data(), pitch, m_pixelFormat, nullptr, bp.palette, bp.palette, 0);
    }

	MetalSurface::MetalSurface(const Blueprint& bp, const uint8_t* pPixels, PixelFormat format,
							   int pitch, const Color8 * pPalette, int paletteSize)
	: Surface(bp, PixelFormat::BGRA_8, SampleMethod::Bilinear)
	{
		_setPixelDetails(m_pixelFormat);
		m_bMipmapped = bp.mipmap;
		
		auto srcDesc = Util::pixelFormatToDescription(format);

		if( pitch == 0 )
			pitch = bp.size.w * srcDesc.bits/8;
		
		_fixSrcParam(format, pPalette, paletteSize);
		_setupMetalTexture( pPixels, pitch, format, nullptr, pPalette, bp.palette, paletteSize );
	}


	MetalSurface::MetalSurface(const Blueprint& bp, const uint8_t* pPixels, const PixelDescription& pixelDescription,
							   int pitch, const Color8 * pPalette, int paletteSize)
	: Surface(bp, PixelFormat::BGRA_8, SampleMethod::Bilinear)
    {
        _setPixelDetails(m_pixelFormat);
        m_bMipmapped = bp.mipmap;
		
		if( pitch == 0 )
			pitch = bp.size.w * pixelDescription.bits/8;


		_fixSrcParam(pixelDescription, pPalette, paletteSize);
		_setupMetalTexture( pPixels, pitch, PixelFormat::Undefined, &pixelDescription, pPalette, bp.palette, paletteSize );
    }


	//____ _setupMetalTexture() __________________________________________________________________

	void MetalSurface::_setupMetalTexture(const void * pPixels, int pitch, PixelFormat srcFormat, const PixelDescription * pSrcPixelDesc, const Color * pSrcPalette, const Color * pDstPalette, int srcPaletteSize )
	{
		m_bTextureSyncInProgress = false;

		// Create our shared buffer
		
		int     bufferLength = m_size.w * m_size.h * m_pixelSize;

		m_textureBuffer = [MetalGfxDevice::s_metalDevice newBufferWithLength:bufferLength options:MTLResourceStorageModeShared];

		// Setup the palette if present
		
		if( pDstPalette )
		{
			// Create the palette buffer and copy data

			m_paletteBuffer = [MetalGfxDevice::s_metalDevice newBufferWithBytes:pDstPalette length:m_paletteCapacity*4 options:MTLResourceStorageModeShared];
			m_pPalette = (Color*) [m_paletteBuffer contents];
		}
		
		//
		
		// Copy pixel data to our shared buffer
			  
		if( pPixels )
		{
			if( srcPaletteSize == 0 )
				srcPaletteSize = m_paletteSize;
			
			auto pDst = (uint8_t*)[m_textureBuffer contents];

			if( srcFormat != PixelFormat::Undefined )
			{
				auto& srcDesc = Util::pixelFormatToDescription(srcFormat);

				PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, srcFormat, pitch - m_size.w * srcDesc.bits/8,
									   pDst, m_pixelFormat, 0, pSrcPalette,
									   m_pPalette, srcPaletteSize, m_paletteSize, m_paletteCapacity);
			}
			else
			{
				PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, * pSrcPixelDesc, pitch - m_size.w * pSrcPixelDesc->bits/8,
									   pDst, m_pixelFormat, 0, pSrcPalette,
									   m_pPalette, srcPaletteSize, m_paletteSize, m_paletteCapacity);
			}
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
        
        if(m_bMipmapped)
        {
            int heightLevels = ceil(log2(m_size.h));
            int widthLevels = ceil(log2(m_size.w));
            int mipCount = (heightLevels > widthLevels) ? heightLevels : widthLevels;
            
            textureDescriptor.mipmapLevelCount = mipCount;
        }

        m_texture = [MetalGfxDevice::s_metalDevice newTextureWithDescriptor:textureDescriptor];
        [textureDescriptor release];
        
        // Create the palette texture
        
        if( m_pPalette )
        {
            MTLTextureDescriptor *paletteDescriptor = [[MTLTextureDescriptor alloc] init];

            paletteDescriptor.pixelFormat   = m_pPixelDescription->colorSpace == ColorSpace::Linear ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatBGRA8Unorm_sRGB;
            paletteDescriptor.width         = m_paletteCapacity;
            paletteDescriptor.height        = 1;
            paletteDescriptor.storageMode   = MTLStorageModePrivate;

            m_paletteTexture = [MetalGfxDevice::s_metalDevice newTextureWithDescriptor:paletteDescriptor];
            
            [paletteDescriptor release];
        }

        // Copy from buffers to textures (pixels and palettes)
        
		if( bHasTextureData || m_pPalette )
		{
			id<MTLCommandBuffer> commandBuffer = [MetalGfxDevice::s_metalCommandQueue commandBuffer];
			id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
			commandBuffer.label = @"_createAndSyncTextures Command Buffer";
			
			if( bHasTextureData )
			{
				MTLSize textureSize = { (unsigned) m_size.w, (unsigned) m_size.h, 1};
				MTLOrigin textureOrigin = {0,0,0};


				[blitCommandEncoder copyFromBuffer:     m_textureBuffer
									sourceOffset:       0
									sourceBytesPerRow:  m_size.w * m_pixelSize
									sourceBytesPerImage:0
									sourceSize:         textureSize
									toTexture:          m_texture
									destinationSlice:   0
									destinationLevel:   0
									destinationOrigin:  textureOrigin];
			}
			
			if( m_pPalette )
			{
				MTLSize paletteSize = { (unsigned long) m_paletteSize, 1, 1 };
				MTLOrigin paletteOrigin = {0,0,0};

				[blitCommandEncoder copyFromBuffer:     m_paletteBuffer
									sourceOffset:       0
									sourceBytesPerRow:  m_paletteCapacity*4
									sourceBytesPerImage:0
									sourceSize:         paletteSize
									toTexture:          m_paletteTexture
									destinationSlice:   0
									destinationLevel:   0
									destinationOrigin:  paletteOrigin];
			}

			if(m_bMipmapped)
				[blitCommandEncoder generateMipmapsForTexture:m_texture];
			
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
		}
    }

    //____ _setPixelDetails() __________________________________________________________

	void MetalSurface::_setPixelDetails( PixelFormat format )
	{
		switch (format)
		{
            case PixelFormat::BGR_8:
                format = PixelFormat::BGRX_8;
            case PixelFormat::BGRX_8:
			case PixelFormat::BGRA_8:
				m_internalFormat = GfxBase::defaultToSRGB() ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
				break;
                
            case PixelFormat::BGR_8_sRGB:
                format = PixelFormat::BGRX_8_sRGB;
            case PixelFormat::BGRX_8_sRGB:
			case PixelFormat::BGRA_8_sRGB:
				m_internalFormat = MTLPixelFormatBGRA8Unorm_sRGB;
				break;

            case PixelFormat::BGR_8_linear:
                format = PixelFormat::BGRX_8_linear;
            case PixelFormat::BGRX_8_linear:
			case PixelFormat::BGRA_8_linear:
				m_internalFormat = MTLPixelFormatBGRA8Unorm;
				break;

			case PixelFormat::BGR_565_linear:
#if TARGET_OS_IPHONE
				m_internalFormat = MTLPixelFormatB5G6R5Unorm;
#else
                format = PixelFormat::BGRX_8_linear;
                m_internalFormat = MTLPixelFormatBGRA8Unorm;
#endif
				break;
                
            case PixelFormat::BGRA_4_linear:
                format = PixelFormat::BGRA_8_linear;
                m_internalFormat = MTLPixelFormatBGRA8Unorm;
                break;
                
			case PixelFormat::Index_8:
			case PixelFormat::Index_8_sRGB:
			case PixelFormat::Index_8_linear:
				m_internalFormat = MTLPixelFormatR8Unorm;
				break;

			case PixelFormat::Alpha_8:
				m_internalFormat = MTLPixelFormatR8Unorm;
				break;

			default:
                GfxBase::throwError( ErrorLevel::Critical, ErrorCode::InvalidParam, "Specified PixelFormat not supported", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
				assert(false);           // Should never get here, just avoiding compiler warnings.
				break;
		}

		m_pPixelDescription = &Util::pixelFormatToDescription(format);
		m_pixelFormat = format;											// We do convert form BGR to BGRX
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

    void MetalSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect)
    {
        if( m_texture && !bufferRect.isEmpty() )
            _syncTexture( bufferRect + buffer.rect.pos() );
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
		// Flush any active device to make sure our texture is up-to-date

		if (MetalGfxDevice::s_pActiveDevice)
			MetalGfxDevice::s_pActiveDevice->flush();

		//
        MTLSize textureSize = { (unsigned) m_size.w, (unsigned) m_size.h,1};
        MTLOrigin textureOrigin = { 0, 0, 0};
        
        id<MTLCommandBuffer> commandBuffer = [MetalGfxDevice::s_metalCommandQueue commandBuffer];
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
        m_bBufferNeedsSync = false;

    }

    //____ _syncTexture() _______________________________________________

    void MetalSurface::_syncTexture(RectI region)
{
		//        if (m_bPendingReads)
		//            MetalGfxDevice::s_pActiveDevice->flush();
		//        if (MetalGfxDevice::s_pActiveDevice)
		//            MetalGfxDevice::s_pActiveDevice->flush();

		//

		MTLSize textureSize = { (unsigned) region.w, (unsigned) region.h,1};
		MTLOrigin textureOrigin = { (unsigned) region.x, (unsigned) region.y,0};

		int     sourceOffset = region.y * m_size.w * m_pixelSize + region.x * m_pixelSize;

		id<MTLCommandBuffer> commandBuffer = [MetalGfxDevice::s_metalCommandQueue commandBuffer];
		commandBuffer.label = @"_syncTexture Command Buffer";

		id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
		[blitCommandEncoder copyFromBuffer:     m_textureBuffer
							  sourceOffset:       sourceOffset
						 sourceBytesPerRow:  m_size.w * m_pixelSize
					   sourceBytesPerImage:0
								sourceSize:         textureSize
								 toTexture:          m_texture
						  destinationSlice:   0
						  destinationLevel:   0
						 destinationOrigin:  textureOrigin];

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
        
//        _waitForSyncedTexture();
    }


    //____ _waitForSyncedTexture() _______________________________________________

    void MetalSurface::_waitForSyncedTexture()
    {
        while( m_bTextureSyncInProgress )
            usleep(20);        // Sleep for 0.02 millisec
    }



} // namespace wg
