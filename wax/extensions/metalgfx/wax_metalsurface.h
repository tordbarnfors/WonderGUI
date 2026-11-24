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
#ifndef	WAX_METALSURFACE_DOT_H
#define	WAX_METALSURFACE_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wax_gpusurface.h>

namespace wax
{

	class MetalSurface;
	typedef	StrongPtr<MetalSurface>	MetalSurface_p;
	typedef	WeakPtr<MetalSurface>	MetalSurface_wp;

	//____ MetalSurface _____________________________________________________________
	/**
	 * @brief Surface for MetalBackend.
	 *
	 * Built on GpuSurface, which keeps our pixels and knows what the texture lacks of them. They
	 * are in a shared buffer, and copied to the private texture when it is used.
	 *
	 * Formats: ARGB_8 and XRGB_8 are BGRA8Unorm, the latter read with alpha 1, the 565 formats are
	 * widened to XRGB_8, Alpha_8 is R8Unorm read as alpha, and palette based surfaces hold their
	 * indexes in R8Uint or R16Uint with the palette in a buffer of float4. sRGB surfaces have
	 * _sRGB formats.
	 */

	class MetalSurface : public GpuSurface
	{
		friend class MetalSurfaceFactory;
		friend class MetalBackend;

	public:

		//.____ Creation __________________________________________

		static MetalSurface_p	create(const Blueprint& blueprint);
		static MetalSurface_p	create(const Blueprint& blueprint, Blob* pBlob, int pitch = 0 );

		static MetalSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
									   PixelFormat format = PixelFormat::Undefined, int pitch = 0,
									   const Color8 * pPalette = nullptr, int paletteSize = 0);

		static MetalSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
									   const PixelDescription& pixelDescription, int pitch = 0,
									   const Color8 * pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI	maxSize();

		//.____ Control _______________________________________________________

		// Lets go of the texture and makes it again from our pixels. What was rendered into it
		// and hasn't been asked for since is lost.

		bool		unload();
		bool		isLoaded();
		void		reload();

		//.____ Misc __________________________________________________________

		inline	id<MTLTexture>	getTexture() const { return m_texture; }

		// What the shaders read: the texture, through a view that reads Alpha_8 from red into
		// alpha and gives XRGB_8 alpha 1, and for palette based surfaces their palette, see
		// GpuTools::paletteToFloats().

		inline	id<MTLTexture>	sampleTexture() const { return m_sampleTexture ? m_sampleTexture : m_texture; }
		inline	id<MTLBuffer>	paletteColors() const { return m_paletteColors; }

		// Sends what the CPU has written since the last call to the texture, if anything, in a
		// command buffer of its own that is committed at once. Called by MetalBackend before the
		// surface is drawn from or into, which puts it ahead of the drawing in the queue.

		void					syncTexture();

	private:

		MetalSurface(const Blueprint& blueprint, const PixelSource& source);
		~MetalSurface();

		// What GpuSurface has us do.

		PixelFormat		_setTextureFormat( PixelFormat format ) override;	// Metal has no 565 on macOS, they are widened to XRGB_8.
		bool			_createDeviceObjects( bool bPixelBuffer ) override;	// Texture, its view, pixel buffer and palette buffer. All or none.
		bool			_createPixelBuffer() override;
		bool			_readBack() override;
		void			_waitForUpload() override;							// Until the GPU has read our pixels for an upload already committed.

		bool			_createTexture();									// The texture and its view for the shaders.
		void			_dropDeviceObjects();

		id<MTLBuffer>	_encodeTextureUpload( id<MTLBlitCommandEncoder> encoder, RectI region );

		id<MTLTexture>	m_texture = nil;			// Private texture, only accessed by GPU.
		id<MTLTexture>	m_sampleTexture = nil;		// Swizzled view of it for the shaders, nil if they read it as it is.
		id<MTLBuffer>	m_textureBuffer = nil;		// Shared buffer with our pixels. A canvas may be without, see GpuSurface::_needPixels().
		id<MTLBuffer>	m_paletteColors = nil;		// The palette as the shaders read it.

		id<MTLCommandBuffer>	m_lastUpload = nil;	// Reads our pixels when the GPU gets to it, see _waitForUpload().

		MTLPixelFormat	m_internalFormat = MTLPixelFormatInvalid;
	};
} // namespace wax
#endif //WAX_METALSURFACE_DOT_H

