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
#ifndef	WAX_GLSURFACE_DOT_H
#define	WAX_GLSURFACE_DOT_H
#pragma once

#define GL_GLEXT_PROTOTYPES 1



#ifdef WIN32
#	include <GL/glew.h>
#else
#	ifdef __APPLE__
#		define GL_SILENCE_DEPRECATION
#		include <OpenGL/gl3.h>
#	else
#		include <GL/glew.h>
#	endif
#endif

#include <wax_gpusurface.h>

namespace wax
{

	class GlSurface;
	typedef	StrongPtr<GlSurface>	GlSurface_p;
	typedef	WeakPtr<GlSurface>	GlSurface_wp;

	//____ GlSurface _____________________________________________________________
	/**
	 * @brief Surface with an OpenGL texture.
	 *
	 * What it keeps of its pixels and how they are read and written is that of GpuSurface.
	 * OpenGL has copied the pixels when it is given them, so what is written is in the
	 * texture when pullPixels() returns, and it needs no staging memory of its own kind.
	 *
	 * Creating a surface and reading or writing its pixels takes the OpenGL context it is
	 * used in to be current.
	 */

	class GlSurface : public GpuSurface
	{
		friend class GlSurfaceFactory;
		friend class GlBackend;

	public:

		//.____ Creation __________________________________________

		static GlSurface_p	create(const Blueprint& blueprint);
		static GlSurface_p	create(const Blueprint& blueprint, Blob* pBlob, int pitch = 0 );

		static GlSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
									   PixelFormat format = PixelFormat::Undefined, int pitch = 0,
									   const Color8 * pPalette = nullptr, int paletteSize = 0 );

		static GlSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
									   const PixelDescription& pixelDescription, int pitch = 0,
									   const Color8 * pPalette = nullptr, int paletteSize = 0 );


		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI	maxSize();

		//.____ Misc __________________________________________________________

		inline	GLuint	getTexture() const { return m_texture; }
		inline	GLuint	getPaletteTexture() const { return m_paletteTexture; }		// Buffer texture of float4, see _updatePaletteColors().

	private:
		GlSurface(const Blueprint& blueprint, const PixelSource& source);
		~GlSurface();

		PixelFormat	_setTextureFormat( PixelFormat format ) override;
		bool		_createDeviceObjects() override;
		Staging *	_allocStaging( SizeI size, bool bReadable ) override;
		void		_pull( Staging * pStaging, const RectI& rect, CoordI dest ) override;
		bool		_readBack( const RectI& rect, uint8_t * pDest, int destPitch ) override;
		void		_updatePending() override;
		void		_updatePaletteColors() override;

		struct GlStaging : Staging
		{
			~GlStaging() { delete [] pPixels; }
		};

		bool		_bindReadFramebuffer( GLint& oldFramebuffer );

		GLuint 		m_texture = 0;			// GL texture handle.
		GLint		m_internalFormat = 0;	// Format of the texture, like GL_SRGB8_ALPHA8 or GL_RGB565.
		GLenum		m_accessFormat = 0;		// Format of pixels we upload and read back, like GL_BGRA or GL_RGB.
		GLenum		m_pixelDataType = 0;

		GLuint 		m_paletteTexture = 0;	// Buffer texture of the palette, see _updatePaletteColors().
		GLuint		m_paletteBuffer = 0;	// The buffer it reads.

		GLuint		m_framebufferId = 0;	// For reading part of the texture back, made when first needed.

		static SizeI	s_maxSize;
	};
} // namespace wax
#endif //WAX_GLSURFACE_DOT_H
