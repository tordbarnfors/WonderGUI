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
#ifndef WAX_GPUSURFACE_DOT_H
#define WAX_GPUSURFACE_DOT_H
#pragma once

#include <wax_surface.h>

namespace wax
{

	class GpuSurface;
	typedef StrongPtr<GpuSurface> GpuSurface_p;
	typedef WeakPtr<GpuSurface> GpuSurface_wp;

	//____ GpuSurface __________________________________________________________
	/**
	 * @brief Base class of surfaces that keep their pixels beside their texture.
	 *
	 * The surface has its pixels in memory the CPU reads and writes, which PixelBuffers point
	 * straight into, and a texture the GPU draws from and into. This class keeps track of which
	 * of the two is the newer and where:
	 *
	 *   m_dirtyRect is what the CPU has written that the texture doesn't have. The backend sends
	 *   it to the texture before the surface is drawn from or into, and calls _uploaded().
	 *   m_bBufferNeedsSync says that something has been rendered into the texture since the
	 *   pixels were last brought from it, see notifyRendered(). They are read back when someone
	 *   asks for them.
	 *
	 * Surfaces with alpha hold premultiplied colors in the texture and straight ones in the
	 * pixels, converted on the way.
	 *
	 * A canvas is drawn into and rarely asked for its pixels, so one that isn't buffered and is
	 * created without pixels or fill color has none until they are asked for, see _needPixels().
	 * A surface the API gives nothing for has its pixels in plain memory instead and no texture,
	 * so it can still be read and written.
	 *
	 * Subclasses pick the format of the texture, make it and what else is on the device, and do
	 * the copying between pixels and texture. DX12Surface, MetalSurface and VulkanSurface are
	 * built on it. GlSurface isn't, as it keeps no pixels unless it has to.
	 */

	class GpuSurface : public Surface
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		int					alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		using Surface::allocPixelBuffer;
		using Surface::pullPixels;
		using Surface::pushPixels;

		const PixelBuffer	allocPixelBuffer(const RectI& rect) override;
		bool				pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true) override;
		void				freePixelBuffer(const PixelBuffer& buffer) override;

		//.____ Misc __________________________________________________________

		// Used by the backends.

		bool				isIndexed() const { return m_bIndexed; }			// Index_8 or Index_16, colors come from the palette.

		// A mipmapped surface has levels all the way down to 1x1, which the backend makes from
		// level 0 before the surface is read with stale ones.

		int					mipLevels() const { return m_mipLevels; }
		bool				isMipmapStale() const { return m_bMipmapStale; }
		void				clearMipmapStale() { m_bMipmapStale = false; }

		// Called by the backend when the surface is about to be rendered into, once what the CPU
		// has written is in the texture. Our pixels are the older copy from here on, until
		// someone asks for them.

		void				notifyRendered();

	protected:

		// The pixels a surface is created from, in the layout the caller has them.

		struct PixelSource
		{
			const uint8_t *		pPixels = nullptr;			// Null for none, the rest is unused then.
			PixelDescription	description;
			int					pitch = 0;
			const Color8 *		pPalette = nullptr;
			int					paletteSize = 0;
		};

		static PixelSource	_source( const Blueprint& bp, Blob * pBlob, int pitch );
		static PixelSource	_source( const Blueprint& bp, const uint8_t * pPixels, PixelFormat format, int pitch,
									 const Color8 * pPalette, int paletteSize );
		static PixelSource	_source( const Blueprint& bp, const uint8_t * pPixels, const PixelDescription& description, int pitch,
									 const Color8 * pPalette, int paletteSize );

		// As that of Surface, and the format has to be one we have a texture for.

		static bool			_isBlueprintValid( const Blueprint& bp, SizeI maxSize );

		// pitchAlignment is what a line of our pixels has to be a multiple of for the API to copy
		// between pixels and texture, a power of two.

		GpuSurface( const Blueprint& bp, int pitchAlignment );
		~GpuSurface();

		// Called by the constructor of the subclass, as it calls what the subclass implements.
		// Settles the format, makes the palette, has the subclass make what is on the device and
		// copies in the pixels or fills with the fill color of the blueprint.

		void				_setup( const Blueprint& bp, const PixelSource& source );

		//____ For subclasses to implement ____

		// Picks the format of the texture for a surface asked to be of the given one, and returns
		// the format our pixels are kept in, which is another when the API has no texture for the
		// one asked for.

		virtual PixelFormat	_setTextureFormat( PixelFormat format ) = 0;

		// Makes the texture and what goes with it and, if bPixelBuffer, the buffer for our pixels,
		// see _createPixelBuffer(). All or none: false leaves us with nothing on the device, and
		// has been reported if it is news. Sets m_pPaletteColors for a palette based surface.

		virtual bool		_createDeviceObjects( bool bPixelBuffer ) = 0;

		// Makes the buffer for our pixels and sets m_pPixels to it, m_pitch bytes per line. What
		// it holds is whatever was in that memory. False if there is no texture to have pixels of.

		virtual bool		_createPixelBuffer() = 0;

		// Copies the texture into our pixels and waits for it, after sending m_dirtyRect to the
		// texture if there is one, as what the CPU wrote is the newer. False if it couldn't,
		// which leaves our pixels as they were.

		virtual bool		_readBack() = 0;

		// Called before our pixels are handed out to be written to, for a subclass whose uploads
		// read them when the GPU gets to it and so have to be done first.

		virtual void		_waitForUpload() {}

		// Called when m_dirtyRect has changed, for a subclass that keeps track of the surfaces
		// that have something to upload.

		virtual void		_updatePending() {}

		//____ For subclasses to use ____

		void				_addDirtyRect( const RectI& rect );
		void				_uploaded();							// m_dirtyRect is in the texture.

		bool				_needPixels();							// Makes the pixel buffer we were created without. False if we have no pixels and can't get any.
		void				_syncPixels();							// Brings what was rendered into the texture to our pixels.

		const PixelBuffer	_pixelBuffer( const RectI& rect ) const;	// Points into our pixels.

		void				_updatePaletteColors();					// Converts m_pPalette into m_pPaletteColors.

		// What is sent to the texture of a surface with alpha is premultiplied, in a buffer of its
		// own unless the pixels are the same premultiplied.

		bool				_isPremultiplied() const { return m_pixelFormat == PixelFormat::ARGB_8; }		// Texture holds premultiplied colors.
		bool				_needsPremultiplying( const RectI& rect ) const;
		void				_premultiply( const RectI& rect, uint8_t * pDest, int destPitch ) const;		// pDest is only written to.

		uint8_t *			m_pPixels = nullptr;			// Our pixels: the pixel buffer of the subclass, or m_pFallbackPixels. Null for a canvas nobody has asked for the pixels of.
		uint8_t *			m_pFallbackPixels = nullptr;	// Plain memory, when we have nothing on the device.
		int					m_pitch = 0;					// Bytes per line, a multiple of m_pitchAlignment.
		int					m_pitchAlignment;
		int					m_pixelBytes = 0;				// Bytes per pixel.
		bool				m_bIndexed = false;

		float *				m_pPaletteColors = nullptr;		// The palette as the shaders read it, see GpuTools::paletteToFloats(). Set by the subclass, only written to.

		RectI				m_dirtyRect;					// Area not yet uploaded to the texture. Empty when in sync.
		bool				m_bBufferNeedsSync = false;		// Texture has changes our pixels don't have yet.

		int					m_mipLevels = 1;
		bool				m_bMipmapStale = false;			// Level 0 has changed since the others were made from it.
	};

} // namespace wax
#endif // WAX_GPUSURFACE_DOT_H
