/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free Glware; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Glware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_METALSURFACE_DOT_H
#define	WG_METALSURFACE_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wg_surface.h>

namespace wg
{

	class MetalSurface;
	typedef	StrongPtr<MetalSurface>	MetalSurface_p;
	typedef	WeakPtr<MetalSurface>	MetalSurface_wp;

	//____ MetalSurface _____________________________________________________________

	class MetalSurface : public Surface
	{
        
        friend class MetalSurfaceFactory;
		friend class MetalGfxDevice;
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

		//.____ Content _______________________________________________________

		int		    alpha(CoordSPX coord) override;

        //.____ Control _______________________________________________________

		using Surface::allocPixelBuffer;

        const PixelBuffer allocPixelBuffer(const RectI& rect) override;
        bool        pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
        void        pullPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
        void        freePixelBuffer(const PixelBuffer& buffer) override;

		bool		unload();
		bool		isLoaded();
		void		reload();


		//.____ Misc __________________________________________________________

		inline	id<MTLTexture>	getTexture() const { return m_texture; }
		inline	id<MTLTexture>	getPaletteTexture() const { return m_paletteTexture; }

	private:

		MetalSurface(const Blueprint& blueprint);
		MetalSurface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		MetalSurface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize);
		MetalSurface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize);


		~MetalSurface();

		void		    _setPixelDetails( PixelFormat format );
		void 			_setupMetalTexture(const void * pPixels, int pitch, PixelFormat srcFormat, const PixelDescription * pSrcPixelDesc,
										   const Color * pSrcPalette, const Color * pDstPalette, int srcPaletteSize );

        void            _createAndSyncTextures( bool bHasTextureData );
        
        void            _syncBufferAndWait();
        void            _syncTexture( RectI region );
        void            _waitForSyncedTexture();

		int			    m_bPendingReads = false;					// Set if there are queued commands that will use surface as source. Active MetalGfxDevice needs to be flushed before we modify.

        bool            m_bBufferNeedsSync = false;                 // Texture has changes that have not been synced to buffer yet.
        std::atomic<bool>  m_bTextureSyncInProgress;           // Texture is being updated with changes from buffer.


		id<MTLTexture>	m_paletteTexture = nil;	    // Private texture with palette data, only accessed by GPU.
        id<MTLBuffer>   m_paletteBuffer = nil;         // Shared buffer, containing palette data accessible to CPU.
        
		id<MTLTexture>	m_texture = nil;			// Private texture, only accessed by GPU.
        id<MTLBuffer>   m_textureBuffer = nil;      // Shared buffer, containing texture data accessible to CPU.
        
        MTLPixelFormat  m_internalFormat;

		uint32_t	    m_pixelSize;		// Size in bytes of a pixel.
		static SizeI	s_maxSize;

        bool            m_bMipmapStale = false;

	};
} // namespace wg
#endif //WG_METALSURFACE_DOT_H

