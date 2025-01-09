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

#include <wg_glsurface.h>
#include <wg_glbackend.h>
#include <wg_gfxutil.h>
#include <wg_pixeltools.h>
#include <wg_blob.h>
#include <wg_gfxbase.h>
#include <assert.h>


#define GL_SILENCE_DEPRECATION


namespace wg
{
	const TypeInfo GlSurface::TYPEINFO = { "GlSurface", &Surface::TYPEINFO };

	SizeI	GlSurface::s_maxSize;

#define HANDLE_GLERROR(check) { GLenum err = check; if(err != 0) GlBackend::onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__ ); }

    unsigned int    GlSurface::g_texturePixels = 0;
    unsigned int    GlSurface::g_backingPixels = 0;

	//____ maxSize() _______________________________________________________________

	SizeI GlSurface::maxSize()
	{
		if (s_maxSize.w == 0)
		{
			GLint max = 1024;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
			s_maxSize.w = max;
			s_maxSize.h = max;
		}

		return s_maxSize;
	}

	//____ Create ______________________________________________________________

	GlSurface_p GlSurface::create(const Blueprint& bp)
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return GlSurface_p();

		return GlSurface_p(new GlSurface(bp));
	}

	GlSurface_p GlSurface::create(const Blueprint& bp, Blob* pBlob, int pitch)
	{
		if (!_isBlueprintValid(bp, maxSize()) )
			return GlSurface_p();

        int realPitch = pitch > 0 ? pitch : bp.size.w * Util::pixelFormatToDescription( bp.format ).bits/8;
        
		if ( !pBlob || realPitch % 4 != 0 )
			return GlSurface_p();


		return GlSurface_p(new GlSurface(bp, pBlob, pitch));
	}

	GlSurface_p	GlSurface::create(const Blueprint& bp, const uint8_t* pPixels,
								  PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if (!_isBlueprintValid(bp, maxSize()))
			return GlSurface_p();

		return  GlSurface_p(new GlSurface(bp, pPixels, format, pitch, pPalette, paletteSize));
	};

	GlSurface_p	GlSurface::create(const Blueprint& bp, const uint8_t* pPixels,
								  const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if (!_isBlueprintValid(bp, maxSize()))
			return GlSurface_p();

		return  GlSurface_p(new GlSurface(bp, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	};


	//____ constructor _____________________________________________________________


	GlSurface::GlSurface( const Blueprint& bp ) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Bilinear)
	{
		HANDLE_GLERROR(glGetError());

		_setPixelDetails(m_pixelFormat);
		m_pPalette = nullptr;

		if (bp.buffered || m_pPixelDescription->bits <= 8)
        {
            g_backingPixels += m_size.w*m_size.h;
            m_pitch = ((m_size.w * m_pPixelDescription->bits / 8) + 3) & 0xFFFFFFFC;
            m_pBlob = Blob::create(m_pitch * m_size.h + m_paletteCapacity*sizeof(Color8));

            if (m_paletteCapacity > 0)
			{
				m_pPalette = (Color8*)((uint8_t*)m_pBlob->data() + m_pitch * m_size.h);
				memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8));
            }
        }
        else
		{
			m_pitch = 0;

			if (m_paletteCapacity > 0)
			{
				m_pPalette = new Color8[m_paletteCapacity];
				memcpy(m_pPalette, bp.palette, m_paletteSize);
			}

			if (m_pPixelDescription->A_mask > 0 && bp.canvas == false)
				m_pAlphaMap = new uint8_t[m_size.w * m_size.h];
		}

		_setupGlTexture(nullptr, 0);

		HANDLE_GLERROR(glGetError());
	}


	GlSurface::GlSurface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Bilinear)
	{
		// Set general information


		_setPixelDetails(m_pixelFormat);
		m_pPalette = const_cast<Color8*>(bp.palette);

		if (pitch == 0)
			pitch = bp.size.w * m_pPixelDescription->bits / 8;

        if ((bp.buffered) || m_pPixelDescription->bits <= 8)
        {
            g_backingPixels += m_size.w*m_size.h;
            m_pitch = pitch;
            m_pBlob = pBlob;
        }
        else
		{
			m_pitch = 0;

			if (m_paletteCapacity > 0)
			{
				m_pPalette = new Color8[m_paletteCapacity];
				memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8));
			}

			if (m_pPixelDescription->A_mask > 0 && bp.canvas == false)
			{
				m_pAlphaMap = new uint8_t[m_size.w * m_size.h];

				// Setup a fake PixelBuffer for call to _updateAlphaMap
				PixelBuffer buf;
				buf.format = m_pixelFormat;
				buf.palette = m_pPalette;
				buf.pitch = pitch;
				buf.pixels = (uint8_t*) pBlob->data();
				buf.rect = m_size;

				_updateAlphaMap( buf, m_size );
			}
		}

		//TODO: Support pitch
		
		_setupGlTexture(pBlob->data(), pitch);
	}

	GlSurface::GlSurface(const Blueprint& bp, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	: GlSurface(bp, pPixels, format == PixelFormat::Undefined ? (bp.format == PixelFormat::Undefined ? Util::pixelFormatToDescription(PixelFormat::BGRA_8)
																 : Util::pixelFormatToDescription(bp.format)) : Util::pixelFormatToDescription(format), pitch, pPalette, paletteSize)
	{
		// Calling constructor with PixelDescription could be slow (but should be ok, once we optimize copyPixels() to take fast route).
	}

	GlSurface::GlSurface(const Blueprint& bp, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	: Surface(bp, PixelFormat::BGRA_8, SampleMethod::Bilinear)
	{
		//TODO: Not just default to BGRA_8 if PixelFormat not specified in Blueprint. Instead we should take the most suitable PixelFormat based on pPixelDescription (same for SoftSurface, MetalSurface etc.)
		
		_setPixelDetails(m_pixelFormat);
		m_pPalette = nullptr;
		
		if( pitch == 0 )
			pitch = bp.size.w * pixelDescription.bits/8;
		
        if (bp.buffered || m_pPixelDescription->bits <= 8)
        {
            g_backingPixels += m_size.w*m_size.h;

			// Create our blob
			
            m_pitch = ((m_size.w * m_pPixelDescription->bits / 8) + 3) & 0xFFFFFFFC;
            m_pBlob = Blob::create(m_pitch * m_size.h + m_paletteCapacity*sizeof(Color8));

			// Setup palette

			if (m_paletteCapacity > 0)
			{
				m_pPalette = (Color8*)((uint8_t*)m_pBlob->data() + m_pitch * m_size.h);
				memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8));
			}

			// Copy pixels
			
			_fixSrcParam(pixelDescription, pPalette, paletteSize);
			
			int srcPitchAdd = (pitch == 0) ? 0 : pitch - pixelDescription.bits/8 * m_size.w;
			
			PixelTools::copyPixels(m_size.w, m_size.h, pPixels, pixelDescription, srcPitchAdd,
					   (uint8_t*) m_pBlob->data(), m_pixelFormat, m_pitch - m_size.w * m_pPixelDescription->bits / 8,
					   pPalette, m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);

			// Setup GL-texture
			
            _setupGlTexture(m_pBlob->data(), m_pitch);
        }
        else
		{
			m_pitch = 0;

			// Setup palette, not part of Blob since we should keep this.

			if (m_paletteCapacity > 0)
			{
				m_pPalette = new Color8[m_paletteCapacity];
				memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8));
			}
			
			// Convert/copy pixels to temporary buffer.

			//TODO: Skip temporary buffer and copy if source format is same as destination (and pitch/4 == 0).

			int tempBufPitch = ((m_size.w * m_pPixelDescription->bits / 8)+3) & 0xFFFFFFFC;
			uint8_t * pTempPixelBuffer = new uint8_t[tempBufPitch * m_size.h];

			// Copy pixels
		
			PixelTools::copyPixels(m_size.w, m_size.h, pPixels, pixelDescription, pitch - m_size.w * pixelDescription.bits/8,
					   pTempPixelBuffer, m_pixelFormat, tempBufPitch - m_size.w * m_pPixelDescription->bits/8,
					   pPalette, m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);

			// Setup GL-texture

            _setupGlTexture(pTempPixelBuffer, tempBufPitch);

			//
			
			if (m_pPixelDescription->A_mask > 0 && bp.canvas == false)
			{
				m_pAlphaMap = new uint8_t[m_size.w * m_size.h];

				// Setup a fake PixelBuffer for call to _updateAlphaMap
				PixelBuffer buf;
				buf.format = m_pixelFormat;
				buf.palette = m_pPalette;
				buf.pitch = tempBufPitch;
				buf.pixels = pTempPixelBuffer;
				buf.rect = m_size;

				_updateAlphaMap(buf, m_size);
			}

			// Delete our temp buffer
			
			delete [] pTempPixelBuffer;

		}
	}


	//____ _setupGlTexture() __________________________________________________

	void GlSurface::_setupGlTexture(void* pPixelsToUpload, int pitch)
	{
		GLint oldBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);

        g_texturePixels += m_size.w*m_size.h;

		// Create and bind texture

		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		// Set tiling

		GLint mode = m_bTiling ? GL_REPEAT : GL_CLAMP_TO_EDGE;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

		m_bTiling = m_bTiling;

		// Push pixels

		HANDLE_GLERROR(glGetError());
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch/m_pixelSize);
        glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0, m_accessFormat, m_pixelDataType, pPixelsToUpload);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		HANDLE_GLERROR(glGetError());

		//

		if( m_bCanvas )
		{
			glGenFramebuffers(1, &m_framebufferId);

			GLint			oldFrameBuffer;

			glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldFrameBuffer);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferId);
			glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, oldFrameBuffer);
		}

		//

		if (m_pPalette)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);


			HANDLE_GLERROR(glGetError());

			glGenTextures(1, &m_paletteTexture);
			glBindTexture(GL_TEXTURE_2D, m_paletteTexture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

			glTexImage2D(GL_TEXTURE_2D, 0, m_pPixelDescription->colorSpace == ColorSpace::Linear ? GL_RGBA8 : GL_SRGB8_ALPHA8, 256, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_pPalette);

			HANDLE_GLERROR(glGetError());
		}
		else
		{
			switch (m_sampleMethod)
			{
			case SampleMethod::Bilinear:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_bMipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;

			case SampleMethod::Nearest:
			default:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			}

			if (m_bMipmapped)
				m_bMipmapStale = true;
		}

		//

		glBindTexture(GL_TEXTURE_2D, oldBinding);

		HANDLE_GLERROR(glGetError());
	}

	//____ _setPixelDetails() ____________________________________________________

	void GlSurface::_setPixelDetails( PixelFormat format )
	{
		switch (format)
		{
			case PixelFormat::BGR_8:
				m_internalFormat = GfxBase::defaultToSRGB() ? GL_SRGB8 : GL_RGB8;
				m_accessFormat = GL_BGR;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 3;
				break;

			case PixelFormat::BGR_8_sRGB:
				m_internalFormat = GL_SRGB8;
				m_accessFormat = GL_BGR;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 3;
				break;

			case PixelFormat::BGR_8_linear:
				m_internalFormat = GL_RGB8;
				m_accessFormat = GL_BGR;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 3;
				break;

			case PixelFormat::BGRX_8:
				m_internalFormat = GfxBase::defaultToSRGB() ? GL_SRGB8 : GL_RGB8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 4;
				break;

			case PixelFormat::BGRX_8_sRGB:
				m_internalFormat = GL_SRGB8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 4;
				break;

			case PixelFormat::BGRX_8_linear:
				m_internalFormat = GL_RGB8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 4;
				break;

			case PixelFormat::BGRA_8:
				m_internalFormat = GfxBase::defaultToSRGB() ? GL_SRGB8_ALPHA8 : GL_RGBA8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 4;
				break;

			case PixelFormat::BGRA_8_sRGB:
				m_internalFormat = GL_SRGB8_ALPHA8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 4;
				break;

			case PixelFormat::BGRA_8_linear:
				m_internalFormat = GL_RGBA8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 4;
				break;


			case PixelFormat::BGR_565_linear:
				m_internalFormat = GL_RGB565;					// NOTE: We lose one bit of precision on green here...
				m_accessFormat = GL_RGB;
				m_pixelDataType = GL_UNSIGNED_SHORT_5_6_5_REV;		// or should we use GL_UNSIGNED_SHORT_5_6_5_REV?
				m_pixelSize = 2;
				break;

			case PixelFormat::BGRA_4_linear:
				m_internalFormat = GL_RGBA4;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_SHORT_4_4_4_4;	// or should we use GL_UNSIGNED_SHORT_4_4_4_4_REV?
				m_pixelSize = 2;
				break;

			case PixelFormat::Index_8:
			case PixelFormat::Index_8_sRGB:
			case PixelFormat::Index_8_linear:
				m_internalFormat = GL_R8;
				m_accessFormat = GL_RED;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 1;
				break;

			case PixelFormat::Alpha_8:
				m_internalFormat = GL_R8;
				m_accessFormat = GL_RED;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				m_pixelSize = 1;
				break;

			default:
				assert(false);           // Should never get here, just avoiding compiler warnings.
				break;

		}
	}

	//____ Destructor ______________________________________________________________

	GlSurface::~GlSurface()
	{
        
        g_texturePixels -= m_size.w*m_size.h;

        if( m_pBlob )
            g_backingPixels -= m_size.w*m_size.h;

		if( m_bCanvas )
			glDeleteFramebuffers(1, &m_framebufferId);

		// Free the stuff

		glDeleteTextures( 1, &m_texture );

		if (m_pPalette)
		{
			glDeleteTextures(1, &m_paletteTexture);
			if (!m_pBlob)
				delete[] m_pPalette;		// Palette is not part of the blob.
		}

		if (m_pAlphaMap)
		{
			delete[] m_pAlphaMap;
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer GlSurface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer pixbuf;

		if (m_pBlob)
		{
			pixbuf.format = m_pixelFormat;
			pixbuf.palette = m_pPalette;
			pixbuf.pitch = m_pitch;
			pixbuf.pixels = ((uint8_t*)m_pBlob->data()) + rect.y * m_pitch + rect.x * m_pixelSize;
			pixbuf.rect = rect;
		}
		else
		{
			pixbuf.format = m_pixelFormat;
			pixbuf.palette = m_pPalette;
			pixbuf.pitch = ((rect.w * m_pPixelDescription->bits / 8) + 3) & 0xFFFFFFFC;
            pixbuf.pixels = new uint8_t[rect.h * pixbuf.pitch];
			pixbuf.rect = rect;
		}
		return pixbuf;
	}

	//____ pushPixels() _______________________________________________________

	bool GlSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		if (m_pBlob)
		{
			if (m_bBackingBufferStale)
				_readBackTexture(m_pBlob->data());

			return true;
		}
        else if( bufferRect.size() == m_size )
		{
            // This is OpenGL pre 4.5, we can only read back a whole texture :(
            
            _readBackTexture(buffer.pixels);
			return false;
		}
        else
            return false;
	}

	//____ pullPixels() _______________________________________________________

	void GlSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		// Handle special case where we use _copyFrom() in constructor
		// before texture has been created.

		if (m_texture == 0)
			return;

		//

		GLint oldBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		RectI texRect = { buffer.rect.pos() + bufferRect.pos(), bufferRect.size() };
		uint8_t* pSrc = buffer.pixels + bufferRect.y * buffer.pitch + bufferRect.x * m_pixelSize;

		glPixelStorei(GL_UNPACK_ROW_LENGTH, buffer.pitch/m_pixelSize);
		glTexSubImage2D(GL_TEXTURE_2D, 0, texRect.x, texRect.y, texRect.w, texRect.h, m_accessFormat, m_pixelDataType, pSrc);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glBindTexture(GL_TEXTURE_2D, oldBinding);
		m_bMipmapStale = m_bMipmapped;

		HANDLE_GLERROR(glGetError());

		if (m_pAlphaMap)
			_updateAlphaMap(buffer, bufferRect);

		Surface::pullPixels(buffer, bufferRect);
	}

	//____ freePixelBuffer() ____________________________________________________

	void GlSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
		if (!m_pBlob)
			delete [] buffer.pixels;
	}

	//____ alpha() ____________________________________________________________

	int GlSurface::alpha( CoordSPX _coord )
	{
//		if (m_bBackingBufferStale)
//			_refreshBackingBuffer();

		if( m_pBlob )
		{
			PixelBuffer buf;
			buf.rect = m_size;
			buf.palette = m_pPalette;
			buf.pitch = m_pitch;
			buf.format = m_pixelFormat;
			buf.pixels = (uint8_t*) m_pBlob->data();
			
			return _alpha(_coord, buf);
		}
		else if (m_pAlphaMap)
		{
			//TODO: Take endianess into account.
			//TODO: Take advantage of subpixel precision and interpolate alpha value if surface set to interpolate.

			CoordI coord(((_coord.x + 32) / 64) % m_size.w, ((_coord.y + 32) / 64) % m_size.h);

			return HiColor::unpackLinearTab[m_pAlphaMap[coord.y * m_size.w + coord.x]];
		}
		else if( m_bCanvas )
		{
			GLint		oldViewport[4];
			GLint		oldFrameBuffer;

			glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldFrameBuffer);

			glGetIntegerv(GL_VIEWPORT, oldViewport);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferId);
			if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::OpenGL, "Can not read pixel from surface for unknow reason, returning 4096 as alpha value.", this, &TYPEINFO, __func__, __FILE__, __LINE__);

				return 4096;
			}

			glViewport(0, 0, m_size.w, m_size.h);

			CoordI coord(((_coord.x + 32) / 64) % m_size.w, ((_coord.y + 32) / 64) % m_size.h);

			uint32_t	myPixel;
			glReadPixels(coord.x, coord.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &myPixel);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, oldFrameBuffer);
			glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

			return HiColor::unpackLinearTab[ myPixel >> 24 ];

		}
		else
			return 4096;
	}

	//____ unload() ___________________________________________________________

	bool GlSurface::unload()
	{
		if (!m_pBlob)
			return false;

		if( m_texture == 0 )
			return true;

		glDeleteTextures( 1, &m_texture );
		m_texture = 0;

		HANDLE_GLERROR(glGetError());
		return true;
	}

	//____ isLoaded() _________________________________________________________

	bool GlSurface::isLoaded()
	{
		return (m_texture != 0);
	}

	//____ reloaded() _________________________________________________________

	void GlSurface::reload()
	{
		if (m_texture != 0)
			return;

		HANDLE_GLERROR(glGetError());

		GLint oldBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);

		glGenTextures( 1, &m_texture );
		glBindTexture( GL_TEXTURE_2D, m_texture );

		switch (m_sampleMethod)
		{
		case SampleMethod::Bilinear:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;

		case SampleMethod::Nearest:
		default:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		}

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

		glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
					 m_accessFormat, m_pixelDataType, m_pBlob->data() );

		glBindTexture( GL_TEXTURE_2D, oldBinding );

		m_bMipmapStale = m_bMipmapped;

		HANDLE_GLERROR(glGetError());
	}

	//____ _updateAlphaMap() __________________________________________________

	void GlSurface::_updateAlphaMap(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		PixelTools::extractAlphaChannel(m_pixelFormat, buffer.pixels, buffer.pitch,
										{ bufferRect.x,bufferRect.y, bufferRect.w, bufferRect.h },
										m_pAlphaMap + buffer.rect.y * m_size.w + buffer.rect.x, m_size.w, m_pPalette);
	}

	//____ _readBackTexture() ____________________________________________

	void GlSurface::_readBackTexture(void * pDest)
	{
		HANDLE_GLERROR(glGetError());

		GLenum	type;

		switch (m_pixelFormat)
		{
		case PixelFormat::BGR_8:
		case PixelFormat::BGR_8_sRGB:
		case PixelFormat::BGR_8_linear:
			type = GL_UNSIGNED_BYTE;
			break;
		case PixelFormat::BGRA_8:
		case PixelFormat::BGRA_8_sRGB:
		case PixelFormat::BGRA_8_linear:
		case PixelFormat::BGRX_8:
		case PixelFormat::BGRX_8_sRGB:
		case PixelFormat::BGRX_8_linear:
			type = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case PixelFormat::BGRA_4_linear:
			type = GL_UNSIGNED_SHORT_4_4_4_4_REV;
			break;
		case PixelFormat::BGR_565_linear:
			type = GL_UNSIGNED_SHORT_5_6_5_REV;
			break;
		case PixelFormat::Alpha_8:
			type = GL_UNSIGNED_BYTE;
			break;
		case PixelFormat::Index_8:
		case PixelFormat::Index_8_sRGB:
		case PixelFormat::Index_8_linear:
			type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(false);   // Should never get here! This code is just to avoid compiler warnings.
			break;
		}

		GLint oldBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);

		glBindTexture(GL_TEXTURE_2D, m_texture);
		glPixelStorei(GL_PACK_ROW_LENGTH, m_size.w);
		glGetTexImage(GL_TEXTURE_2D, 0, m_accessFormat, type, pDest);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glBindTexture(GL_TEXTURE_2D, oldBinding);

		HANDLE_GLERROR(glGetError());

		m_bBackingBufferStale = false;
	}




} // namespace wg
