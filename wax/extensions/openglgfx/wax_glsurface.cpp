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

#include <wax_glsurface.h>
#include <wax_glbackend.h>
#include <wax_util.h>
#include <wax_blob.h>
#include <wax_base.h>

#include <cstring>


#define GL_SILENCE_DEPRECATION


namespace wax
{
	const TypeInfo GlSurface::TYPEINFO = { "GlSurface", &GpuSurface::TYPEINFO };

	SizeI	GlSurface::s_maxSize;

#define HANDLE_GLERROR(check) { GLenum err = check; if(err != 0) GlBackend::onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__ ); }

	// Our pixels are little endian, see GpuSurface. Packed pixel types describe a pixel as a
	// word of the CPU, with the same register order as our PixelFormats, so on a big endian
	// one OpenGL swaps the bytes on the way in and out.

	static const GLint c_swapBytes = (WAX_IS_BIG_ENDIAN == 1) ? GL_TRUE : GL_FALSE;

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

		return GlSurface_p(new GlSurface(bp, PixelSource()));
	}

	GlSurface_p GlSurface::create(const Blueprint& bp, Blob* pBlob, int pitch)
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return GlSurface_p();

		return GlSurface_p(new GlSurface(bp, _source(bp, pBlob, pitch)));
	}

	GlSurface_p	GlSurface::create(const Blueprint& bp, const uint8_t* pPixels,
								  PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return GlSurface_p();

		return GlSurface_p(new GlSurface(bp, _source(bp, pPixels, format, pitch, pPalette, paletteSize)));
	}

	GlSurface_p	GlSurface::create(const Blueprint& bp, const uint8_t* pPixels,
								  const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(bp, maxSize()) )
			return GlSurface_p();

		return GlSurface_p(new GlSurface(bp, _source(bp, pPixels, pixelDescription, pitch, pPalette, paletteSize)));
	}

	//____ constructor _____________________________________________________________

	GlSurface::GlSurface( const Blueprint& bp, const PixelSource& source ) : GpuSurface(bp)
	{
		m_bPullsAtOnce = true;			// glTexSubImage2D() has copied when it returns.

		_setup(bp, source);
	}

	//____ Destructor ______________________________________________________________

	GlSurface::~GlSurface()
	{
		if( m_framebufferId != 0 )
			glDeleteFramebuffers(1, &m_framebufferId);

		if( m_texture != 0 )
			glDeleteTextures(1, &m_texture);

		if( m_paletteTexture != 0 )
		{
			glDeleteTextures(1, &m_paletteTexture);
			glDeleteBuffers(1, &m_paletteBuffer);
		}

		delete [] m_pPaletteColors;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setTextureFormat() ___________________________________________________
	//
	// Picks the GL texture format and the format of the pixels we upload and read back. GL has
	// a texture for each of our formats, so the pixels are in the one asked for.
	//
	// An sRGB surface gets an sRGB texture, which the GPU decodes to linear when sampling. An
	// sRGB 565 surface is stored as GL_SRGB8, since there is no sRGB 565 texture format.

	PixelFormat GlSurface::_setTextureFormat( PixelFormat format )
	{
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

		switch (format)
		{
			case PixelFormat::XRGB_8:
				m_internalFormat = bSRGB ? GL_SRGB8 : GL_RGB8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_INT_8_8_8_8_REV;		// Blue in the low bits, X in the high.
				break;

			case PixelFormat::ARGB_8:
				m_internalFormat = bSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
				m_accessFormat = GL_BGRA;
				m_pixelDataType = GL_UNSIGNED_INT_8_8_8_8_REV;		// Blue in the low bits, alpha in the high.
				break;

			case PixelFormat::RGB_565:
				m_internalFormat = bSRGB ? GL_SRGB8 : GL_RGB565;
				m_accessFormat = GL_RGB;
				m_pixelDataType = GL_UNSIGNED_SHORT_5_6_5;			// Red in the high bits, blue in the low.
				break;

			case PixelFormat::BGR_565:
				m_internalFormat = bSRGB ? GL_SRGB8 : GL_RGB565;
				m_accessFormat = GL_RGB;
				m_pixelDataType = GL_UNSIGNED_SHORT_5_6_5_REV;		// Red in the low bits, blue in the high.
				break;

			case PixelFormat::Index_8:
				m_internalFormat = GL_R8UI;							// Read as integers, see ps_blit_palette.hlsl.
				m_accessFormat = GL_RED_INTEGER;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				break;

			case PixelFormat::Index_16:
				m_internalFormat = GL_R16UI;
				m_accessFormat = GL_RED_INTEGER;
				m_pixelDataType = GL_UNSIGNED_SHORT;
				break;

			case PixelFormat::Alpha_8:
				m_internalFormat = GL_R8;
				m_accessFormat = GL_RED;
				m_pixelDataType = GL_UNSIGNED_BYTE;
				break;

			default:
				break;			// create() turns other formats away.
		}

		return format;
	}

	//____ _createDeviceObjects() ________________________________________________
	//
	// The texture, without pixels, and for a palette based surface the buffer texture the
	// shaders read its palette from.

	bool GlSurface::_createDeviceObjects()
	{
		HANDLE_GLERROR(glGetError());

		GLint oldBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);

		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		GLint wrapMode = m_bTiling ? GL_REPEAT : GL_CLAMP_TO_EDGE;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

		glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0, m_accessFormat, m_pixelDataType, nullptr);

		// The shaders read the alpha of Alpha_8 sources, which GL keeps in red. Reading through
		// a swizzle doesn't change what is rendered into the texture as a canvas. XRGB_8 and the
		// 565 formats have textures without alpha, which read alpha 1.

		if( m_pixelFormat == PixelFormat::Alpha_8 )
		{
			GLint swizzle[4] = { GL_ZERO, GL_ZERO, GL_ZERO, GL_RED };
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
		}

		if( m_bIndexed )
		{
			// Indexes are integers the shaders read with texelFetch(), see ps_blit_palette.hlsl.

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

			GLint oldBuffer;
			glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &oldBuffer);

			glGenBuffers(1, &m_paletteBuffer);
			glGenTextures(1, &m_paletteTexture);

			glBindBuffer(GL_TEXTURE_BUFFER, m_paletteBuffer);
			glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(m_paletteCapacity + 1) * 4 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_TEXTURE_BUFFER, 0);

			glBindTexture(GL_TEXTURE_BUFFER, m_paletteTexture);
			glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_paletteBuffer);
			glBindTexture(GL_TEXTURE_BUFFER, GLuint(oldBuffer));

			// Sent to the buffer by _updatePaletteColors(), which GpuSurface calls when the
			// palette is set and when it may have changed.

			m_pPaletteColors = new float[size_t(m_paletteCapacity + 1) * 4];
		}
		else if( m_sampleMethod == SampleMethod::Bilinear )
		{
			// A mipmap filter on a texture without mipmaps leaves it incomplete, and it samples
			// as black, so GlBackend makes them before the first use.

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		m_bMipmapStale = (m_mipLevels > 1);

		glBindTexture(GL_TEXTURE_2D, oldBinding);

		HANDLE_GLERROR(glGetError());
		return true;
	}

	//____ _allocStaging() _______________________________________________________
	//
	// Plain memory, as glTexSubImage2D() copies from anywhere.

	GpuSurface::Staging * GlSurface::_allocStaging( SizeI size, bool bReadable )
	{
		auto pStaging = new GlStaging();

		pStaging->pitch = (size.w * m_pixelBytes + 3) & ~3;
		pStaging->pPixels = new uint8_t[size_t(pStaging->pitch) * size.h];

		return pStaging;
	}

	//____ _pull() _______________________________________________________________
	//
	// Lines are pitch bytes apart, which a row length in pixels says for every pitch that is a
	// whole number of pixels. Ours are, being multiples of four.

	void GlSurface::_pull( Staging * pStaging, const RectI& rect, CoordI dest )
	{
		if( m_texture == 0 )
			return;

		GLint oldBinding;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		const uint8_t * pSrc = pStaging->pPixels + rect.y * pStaging->pitch + rect.x * m_pixelBytes;

		glPixelStorei(GL_UNPACK_ROW_LENGTH, pStaging->pitch / m_pixelBytes);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, c_swapBytes);
		glTexSubImage2D(GL_TEXTURE_2D, 0, dest.x, dest.y, rect.w, rect.h, m_accessFormat, m_pixelDataType, pSrc);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		glBindTexture(GL_TEXTURE_2D, oldBinding);

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;

		HANDLE_GLERROR(glGetError());
	}

	//____ _updatePending() ______________________________________________________
	//
	// Nothing waits to be sent with OpenGL: what is written to our pixels goes to the texture
	// right away.

	void GlSurface::_updatePending()
	{
		_stageDirty();
	}

	//____ _updatePaletteColors() ________________________________________________

	void GlSurface::_updatePaletteColors()
	{
		GpuSurface::_updatePaletteColors();

		if( !m_pPaletteColors || m_paletteBuffer == 0 )
			return;

		glBindBuffer(GL_TEXTURE_BUFFER, m_paletteBuffer);
		glBufferSubData(GL_TEXTURE_BUFFER, 0, GLsizeiptr(m_paletteCapacity + 1) * 4 * sizeof(GLfloat), m_pPaletteColors);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}

	//____ _bindReadFramebuffer() ________________________________________________
	//
	// Binds a framebuffer with our texture to read from. False if GL won't have one of a
	// texture of our format, which it doesn't promise for all of them.

	bool GlSurface::_bindReadFramebuffer( GLint& oldFramebuffer )
	{
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldFramebuffer);

		if( m_framebufferId == 0 )
		{
			glGenFramebuffers(1, &m_framebufferId);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferId);
			glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);
		}
		else
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferId);

		if( glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, oldFramebuffer);
			return false;
		}

		return true;
	}

	//____ _readBack() ___________________________________________________________
	//
	// OpenGL before 4.5 can read a whole texture, or a rectangle of a framebuffer. So part of
	// us is read through a framebuffer, and if there is none to be had, by reading all of us.
	//
	// What GlBackend has recorded and not yet executed isn't in the texture, which only shows
	// to code that asks for pixels between its beginSession() and endSession().

	bool GlSurface::_readBack( const RectI& rect, uint8_t * pDest, int destPitch )
	{
		if( m_texture == 0 )
			return false;

		HANDLE_GLERROR(glGetError());

		GLint oldAlignment;
		glGetIntegerv(GL_PACK_ALIGNMENT, &oldAlignment);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_SWAP_BYTES, c_swapBytes);

		GLint oldFramebuffer;

		if( rect == RectI(m_size) )
		{
			GLint oldBinding;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);

			glPixelStorei(GL_PACK_ROW_LENGTH, destPitch / m_pixelBytes);
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glGetTexImage(GL_TEXTURE_2D, 0, m_accessFormat, m_pixelDataType, pDest);
			glBindTexture(GL_TEXTURE_2D, oldBinding);
		}
		else if( _bindReadFramebuffer(oldFramebuffer) )
		{
			glPixelStorei(GL_PACK_ROW_LENGTH, destPitch / m_pixelBytes);
			glReadPixels(rect.x, rect.y, rect.w, rect.h, m_accessFormat, m_pixelDataType, pDest);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, oldFramebuffer);
		}
		else
		{
			GLint oldBinding;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);

			int			pitch = m_size.w * m_pixelBytes;
			uint8_t *	pAll = new uint8_t[size_t(pitch) * m_size.h];

			glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glGetTexImage(GL_TEXTURE_2D, 0, m_accessFormat, m_pixelDataType, pAll);
			glBindTexture(GL_TEXTURE_2D, oldBinding);

			for( int y = 0; y < rect.h; y++ )
				memcpy( pDest + y * destPitch, pAll + (rect.y + y) * pitch + rect.x * m_pixelBytes, rect.w * m_pixelBytes );

			delete [] pAll;
		}

		glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, oldAlignment);

		GLenum err = glGetError();

		if( err != 0 )
		{
			GlBackend::onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		return true;
	}

} // namespace wax
