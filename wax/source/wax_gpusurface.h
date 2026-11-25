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

#include <vector>

namespace wax
{

	class GpuSurface;
	typedef StrongPtr<GpuSurface> GpuSurface_p;
	typedef WeakPtr<GpuSurface> GpuSurface_wp;

	//____ GpuSurface __________________________________________________________
	/**
	 * @brief Base class of surfaces with a texture on a GPU.
	 *
	 * A surface keeps its pixels in plain memory besides its texture, unless it is promised
	 * them back: by a restorer in its Blueprint, or by being a canvas, which whoever rendered
	 * into renders again.
	 *
	 * Pixels that are kept are what a PixelBuffer points into. What the CPU has written that
	 * the texture doesn't have is m_dirtyRect, which is sent before the surface is drawn from.
	 * After a lost device all of it is sent again.
	 *
	 * Without pixels of its own, a PixelBuffer is memory the GPU copies from, of just the
	 * rectangle asked for, which pullPixels() has copied to the texture and pushPixels() reads
	 * the texture back into, waiting for the GPU. alpha() reads a map of a byte for each pixel.
	 * A surface with restorer writes it together with the pixels, and has none as long as all
	 * that was written is opaque. A canvas reads the texture back for it, when first asked and
	 * again when it has been rendered into since. Alpha_8 and palette based surfaces that are
	 * no canvases keep their pixels with a restorer too, as an alpha map would take as much,
	 * and the alpha of a palette based one changes with its palette.
	 *
	 * Such a surface still has pixels at times: what it was created with until that has been
	 * sent, and what it is to hold while there is no device. They are let go of once they are
	 * on their way to a texture.
	 *
	 * When the device is replaced, the restorer of a surface without pixels is called to write
	 * them again, see _restore(). If there is none, or it declines, the surface is cleared and
	 * says isContentLost(). A canvas without restorer is read back first when the device it
	 * leaves still works.
	 *
	 * The GPU copies to the texture from staging memory, which the subclass makes and lets go
	 * of when the GPU has passed the copy.
	 *
	 * Surfaces with alpha hold premultiplied colors in the texture and straight ones everywhere
	 * else, converted on the way.
	 *
	 * Subclasses pick the format of the texture, make it and what else is on the device, make
	 * staging memory and do the copying between that and the texture. DX12Surface, GlSurface,
	 * MetalSurface and VulkanSurface are built on it.
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
		// has written is in the texture.

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

		GpuSurface( const Blueprint& bp );
		~GpuSurface();

		// Called by the constructor of the subclass, as it calls what the subclass implements.
		// Settles the format, makes the palette, has the subclass make what is on the device and
		// copies in the pixels or fills with the fill color of the blueprint.

		void				_setup( const Blueprint& bp, const PixelSource& source );

		// Memory the GPU copies to a texture from, mapped. A subclass adds what it has it by and
		// lets go of that in the destructor, where the GPU may still have a copy from it to do.

		struct Staging
		{
			virtual ~Staging() {}

			uint8_t *	pPixels = nullptr;
			int			pitch = 0;					// Bytes per line, as the API wants it for copying.
		};

		//____ For subclasses to implement ____

		// Picks the format of the texture for a surface asked to be of the given one, and returns
		// the format our pixels are in, which is another when the API has no texture for the one
		// asked for.

		virtual PixelFormat	_setTextureFormat( PixelFormat format ) = 0;

		// Makes the texture and what goes with it. All or none: false leaves us with nothing on
		// the device, and has been reported if it is news. Sets m_pPaletteColors for a palette
		// based surface.

		virtual bool		_createDeviceObjects() = 0;

		// Makes staging memory for the given number of pixels, holding whatever was in that
		// memory. bReadable is for what is handed out as a PixelBuffer, which may be read: memory
		// that is slow to read, as write-combined is, is only for what we write once. Null if the
		// device gives none, which has been reported if it is news.

		virtual Staging *	_allocStaging( SizeI size, bool bReadable ) = 0;

		// Has a rectangle of staging memory copied to the texture at dest, by the time the
		// backend draws from or into us. The staging memory may be deleted right after the call.
		// With m_bPullsAtOnce set it may be given pixels that aren't of _allocStaging(), as a
		// plain Staging.

		virtual void		_pull( Staging * pStaging, const RectI& rect, CoordI dest ) = 0;

		// Copies a rectangle of the texture as it is, premultiplied if it is, to pDest and waits
		// for it, once what we have had pulled is in the texture. False if it couldn't.

		virtual bool		_readBack( const RectI& rect, uint8_t * pDest, int destPitch ) = 0;

		// Called when what we have to send to the texture has changed, for a subclass that
		// keeps track of the surfaces that have something.

		virtual void		_updatePending() {}

		// Converts m_pPalette into m_pPaletteColors, when the palette has changed. For a
		// subclass that has to send the colors on to override.

		virtual void		_updatePaletteColors();

		//____ For subclasses to use ____

		// Whether our pixels stay in plain memory for as long as we live.

		bool				_keepsPixels() const { return m_bIndexed || (!m_bCanvas && (!m_restorer || m_pixelFormat == PixelFormat::Alpha_8)); }
		bool				_isOnDevice() const { return m_bOnDevice; }

		void				_addDirtyRect( const RectI& rect );

		// For the backend to call before it draws from or into us. _stageDirty() has m_dirtyRect
		// pulled from staging memory of its own. A subclass that stages in another way copies
		// with _fillStaging() and calls _dirtyStaged(). _uploaded() is for when all we have had
		// pulled is on its way to the texture.

		void				_stageDirty();
		void				_fillStaging( const RectI& rect, uint8_t * pDest, int destPitch ) const;	// pDest is only written to.
		void				_dirtyStaged();
		void				_uploaded();

		// Going off a device and onto one. _leaveDevice() is called while the texture is still
		// there. Pixels we have are to be sent again, all of them. A surface without has lost
		// its content, unless it is a canvas without restorer on a device that works, which is
		// read back into pixels it has until it is on a device again. _enterDevice() is called
		// when there is a texture again.
		//
		// _restore() is for when every surface is back on a device, as a restorer may use other
		// surfaces. It gives a surface that lost its content what it is to hold: what its
		// restorer writes, or transparent black and isContentLost(). The restorer is code of the
		// application, which can create and let go of surfaces.

		void				_leaveDevice( bool bDeviceWorks );
		void				_enterDevice();

		bool				_isRestorePending() const { return m_bRestorePending; }
		void				_restore();

		const PixelBuffer	_pixelBuffer( const RectI& rect ) const;	// Points into our pixels.

		bool				_isPremultiplied() const { return m_pixelFormat == PixelFormat::ARGB_8; }		// Texture holds premultiplied colors.

		uint8_t *			m_pPixels = nullptr;			// Our pixels, in plain memory. Null when only the texture has them.
		int					m_pitch = 0;					// Bytes per line of them.
		int					m_pixelBytes = 0;				// Bytes per pixel.
		bool				m_bIndexed = false;

		bool				m_bPullsAtOnce = false;			// Set by a subclass whose _pull() has copied when it returns, so it can be given our pixels as they are.

		float *				m_pPaletteColors = nullptr;		// The palette as the shaders read it, see GpuTools::paletteToFloats(). Set by the subclass, only written to.

		RectI				m_dirtyRect;					// Area of our pixels the texture doesn't have. Empty when in sync.

		int					m_mipLevels = 1;
		bool				m_bMipmapStale = false;			// Level 0 has changed since the others were made from it.

	private:

		// A PixelBuffer handed out of a surface without pixels of its own.

		struct Handout
		{
			uint8_t *	pPixels;				// What the PixelBuffer points to.
			Staging *	pStaging;				// Null when the device gave none, the memory is plain then.
			bool		bDeviceGone;			// Its device was let go of, so it can't be copied from.
		};

		int					_findHandout( const uint8_t * pPixels ) const;		// Index, or -1.
		bool				_needsPremultiplying( const uint8_t * pPixels, int pitch, SizeI size ) const;
		void				_convertInto( const uint8_t * pSrc, int srcPitch, SizeI size, uint8_t * pDest, int destPitch, bool bPremultiply ) const;
		bool				_stageFrom( const uint8_t * pSrc, int srcPitch, SizeI size, CoordI dest, bool bPremultiply );		// False if there was no staging memory to be had.

		void				_makeLostPixels();
		void				_releaseSparePixels();

		bool				_writesAlphaMap() const { return !m_bCanvas && !_keepsPixels() && m_pPixelDescription->A_mask != 0; }
		bool				_writeAlphaMap( const uint8_t * pPixels, int pitch, const RectI& rect );		// True if all of it is known to be opaque.
		bool				_readAlphaMap();

		bool				m_bOnDevice = false;			// We have a texture.
		bool				m_bRestorePending = false;		// The texture we had our content in is gone, see _restore().

		std::vector<Handout>	m_handouts;
		int					m_nInPlaceBuffers = 0;			// PixelBuffers out that point into our pixels.

		uint8_t *			m_pAlphaMap = nullptr;			// For alpha() of a surface without pixels, a byte for each pixel. Null while all is opaque, if we write it.
		bool				m_bAlphaMapStale = true;		// For a canvas, which reads it back.
	};

} // namespace wax
#endif // WAX_GPUSURFACE_DOT_H
