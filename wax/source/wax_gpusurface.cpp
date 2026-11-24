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

#include <wax_gpusurface.h>
#include <wax_gputools.h>

#include <wax_blob.h>
#include <wax_util.h>
#include <wax_pixeltools.h>

#include <cstring>

namespace wax
{

	const TypeInfo GpuSurface::TYPEINFO = { "GpuSurface", &Surface::TYPEINFO };

	// Our pixels are little endian, as the texture formats are laid out by byte. The premultiply
	// code reads them as words.

	static const bool c_bSwapBytes = (WAX_IS_BIG_ENDIAN == 1);

	//____ _source() _____________________________________________________________
	//
	// A blob holds pixels in the layout of the blueprint, with its palette.

	GpuSurface::PixelSource GpuSurface::_source( const Blueprint& bp, Blob * pBlob, int pitch )
	{
		PixelSource source;

		if( pBlob )
			source = _source( bp, (const uint8_t*) pBlob->data(), PixelFormat::Undefined, pitch, bp.palette, 0 );

		return source;
	}

	// Pixels are in the byte order of the blueprint, and of its format unless one is given.

	GpuSurface::PixelSource GpuSurface::_source( const Blueprint& bp, const uint8_t * pPixels, PixelFormat format, int pitch,
												 const Color8 * pPalette, int paletteSize )
	{
		if( format == PixelFormat::Undefined )
			format = bp.format == PixelFormat::Undefined ? PixelFormat::ARGB_8 : bp.format;

		return _source( bp, pPixels, Util::pixelFormatToDescription(format, bp.bigEndian), pitch, pPalette, paletteSize );
	}

	GpuSurface::PixelSource GpuSurface::_source( const Blueprint& bp, const uint8_t * pPixels, const PixelDescription& description, int pitch,
												 const Color8 * pPalette, int paletteSize )
	{
		PixelSource source;

		source.pPixels = pPixels;
		source.description = description;
		source.pitch = pitch != 0 ? pitch : PixelTools::bytesPerLine(description, bp.size.w);
		source.pPalette = pPalette;
		source.paletteSize = paletteSize;

		return source;
	}

	//____ _isBlueprintValid() ___________________________________________________

	bool GpuSurface::_isBlueprintValid( const Blueprint& bp, SizeI maxSize )
	{
		switch( bp.format )
		{
			case PixelFormat::Undefined:			// Defaults to ARGB_8.
			case PixelFormat::XRGB_8:
			case PixelFormat::ARGB_8:
			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
			case PixelFormat::Index_8:
			case PixelFormat::Index_16:
			case PixelFormat::Alpha_8:
				return Surface::_isBlueprintValid(bp, maxSize);

			default:
				return false;
		}
	}

	//____ constructor ___________________________________________________________
	//
	// Default sample method is Bilinear. Code that creates surfaces without specifying one
	// expects that, and Nearest samples on texel corners for 1:1 blits, which float precision in
	// tall textures makes land on the texel above every now and then.

	GpuSurface::GpuSurface( const Blueprint& bp ) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;
	}

	//____ destructor ____________________________________________________________

	GpuSurface::~GpuSurface()
	{
		for( auto& handout : m_handouts )
		{
			if( handout.pStaging )
				delete handout.pStaging;
			else
				delete [] handout.pPixels;
		}

		delete [] m_pPixels;
		delete [] m_pAlphaMap;
		delete [] m_pPalette;
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& GpuSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setup() ______________________________________________________________

	void GpuSurface::_setup( const Blueprint& bp, const PixelSource& source )
	{
		// We may end up with another layout than was asked for, so format, description and size
		// of a pixel follow what the subclass has a texture for.

		m_pixelFormat = _setTextureFormat( m_pixelFormat );
		m_bBigEndian = false;
		m_pixelDescription = Util::pixelFormatToDescription(m_pixelFormat, m_bBigEndian);
		m_pixelBytes = m_pPixelDescription->bits / 8;
		m_bIndexed = (m_pPixelDescription->type == PixelType::Index);

		m_pitch = (m_size.w * m_pixelBytes + 3) & ~3;

		// Palette based surfaces can't be mipmapped, an average of indexes means nothing.

		if( m_bMipmapped && !m_bIndexed )
		{
			for( int size = m_size.w > m_size.h ? m_size.w : m_size.h; size > 1; size >>= 1 )
				m_mipLevels++;
		}

		// Surface leaves the palette to us. It has to exist before any pixels are copied in, since
		// converting to a palette based format fills it in. It holds as many entries as the
		// palette capacity Surface has worked out.

		if( m_bIndexed && m_paletteCapacity > 0 )
		{
			m_pPalette = new Color8[m_paletteCapacity];
			memset( m_pPalette, 0, sizeof(Color8) * m_paletteCapacity );

			if( bp.palette && m_paletteSize > 0 )
				memcpy( m_pPalette, bp.palette, sizeof(Color8) * m_paletteSize );
		}

		m_bOnDevice = _createDeviceObjects();

		// Without pixels and without a fill color nothing is written and nothing is sent to the
		// texture, as the content of such a surface is unspecified.

		bool bContent = source.pPixels || !bp.fillColor.isUndefined();

		// A canvas or a surface with restorer has no pixels of its own, which is half its memory
		// and a good part of the time it takes to make it. That matters for canvas layers, made
		// for every canvas update. What it is created with is still written to pixels, which it
		// has until they are sent. That is a copy more than writing straight to staging memory,
		// but lets a backend send what many surfaces were created with in one go. A surface with
		// nothing on the device is of no use to the GPU, but it still has to behave like a
		// surface: hand out pixel buffers, be readable, be writable.

		if( _keepsPixels() || !m_bOnDevice || bContent )
			m_pPixels = new uint8_t[size_t(m_pitch) * m_size.h];

		if( bContent )
		{
			PixelBuffer buffer = GpuSurface::allocPixelBuffer( RectI(m_size) );

			if( buffer.pixels && source.pPixels )
			{
				// A source without a palette size, or palette based without a palette, is taken to
				// have as many colors as we do.

				bool bSourceIndexed = (source.description.type == PixelType::Index || source.description.type == PixelType::Bitplanes);

				int paletteSize = (source.paletteSize == 0 || (bSourceIndexed && !source.pPalette)) ? m_paletteSize : source.paletteSize;

				PixelTools::copyPixels(m_size.w, m_size.h, source.pPixels, source.description, m_colorSpace,
									   source.pitch - PixelTools::bytesPerLine(source.description, m_size.w),
									   source.pPalette, paletteSize,
									   buffer.pixels, m_pixelFormat, m_colorSpace, m_bBigEndian, buffer.pitch - m_size.w * m_pixelBytes,
									   m_pPalette, m_paletteSize, m_paletteCapacity);
			}
			else if( buffer.pixels )
			{
				PixelTools::fillBitmap(buffer.pixels, m_pixelFormat, m_colorSpace, m_bBigEndian, buffer.pitch, RectI(m_size),
									   bp.fillColor, m_pPalette, m_paletteSize);
			}

			GpuSurface::pullPixels( buffer, RectI(m_size), false );
			GpuSurface::freePixelBuffer( buffer );
		}

		_updatePaletteColors();
	}

	//____ alpha() _______________________________________________________________

	int GpuSurface::alpha(CoordSPX coord)
	{
		if( m_pPixels )
			return _alpha(coord, _pixelBuffer(RectI(m_size)));

		if( m_pPixelDescription->A_mask == 0 )
			return 4096;

		// We have no pixels, so there is a map of a byte for each. The alpha of a canvas is read
		// back from the texture when first asked for and when it has changed since. A surface
		// with restorer has written its map with the pixels, unless all of them were opaque.

		if( m_bCanvas )
		{
			if( !_readAlphaMap() )
				return 0;
		}
		else if( !m_pAlphaMap )
			return 4096;

		PixelBuffer	buf;

		buf.format = PixelFormat::Alpha_8;
		buf.colorSpace = m_colorSpace;
		buf.bigEndian = false;
		buf.palette = nullptr;
		buf.pitch = m_size.w;
		buf.pixels = m_pAlphaMap;
		buf.rect = RectI(m_size);

		return _alpha(coord, buf);
	}

	//____ allocPixelBuffer() ____________________________________________________
	//
	// Points into our pixels if we have them. If not it is memory of its own for the rectangle
	// asked for, which the GPU can copy from. Note that the pitch can be padded either way.

	const PixelBuffer GpuSurface::allocPixelBuffer(const RectI& rect)
	{
		if( !m_pPixels && !m_bOnDevice )
			_makeLostPixels();

		if( m_pPixels )
		{
			m_nInPlaceBuffers++;
			return _pixelBuffer(rect);
		}

		// Plain memory when the device gives none, which makes what is written to it take a
		// copy more to get to the texture.

		Handout handout;

		handout.pStaging = (m_bOnDevice && !rect.isEmpty()) ? _allocStaging( rect.size(), true ) : nullptr;
		handout.bDeviceGone = false;

		int pitch;

		if( handout.pStaging )
		{
			handout.pPixels = handout.pStaging->pPixels;
			pitch = handout.pStaging->pitch;
		}
		else
		{
			pitch = (rect.w * m_pixelBytes + 3) & ~3;
			handout.pPixels = new uint8_t[size_t(pitch) * rect.h + 1];
		}

		m_handouts.push_back(handout);

		PixelBuffer	buf;

		buf.format = m_pixelFormat;
		buf.colorSpace = m_colorSpace;
		buf.bigEndian = m_bBigEndian;
		buf.palette = m_pPalette;
		buf.pitch = pitch;
		buf.pixels = handout.pPixels;
		buf.rect = rect;

		return buf;
	}

	//____ pushPixels() __________________________________________________________
	//
	// A buffer that points into our pixels is up to date as it is. One of a surface without
	// gets the rectangle read back from the texture, which stalls until the GPU is done.

	bool GpuSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		bool bHandout = _findHandout(buffer.pixels) >= 0;

		if( m_pPixels && !bHandout )
			return true;

		if( bufferRect.isEmpty() )
			return true;

		RectI		rect = bufferRect + buffer.rect.pos();
		uint8_t *	pDest = buffer.pixels + bufferRect.y * buffer.pitch + bufferRect.x * m_pixelBytes;
		int			lineBytes = rect.w * m_pixelBytes;

		if( !m_pPixels && !m_bOnDevice )
			_makeLostPixels();

		// Handed out while only the texture had our content, which our pixels have now.

		if( m_pPixels )
		{
			for( int y = 0; y < rect.h; y++ )
				memcpy( pDest + y * buffer.pitch, m_pPixels + (rect.y + y) * m_pitch + rect.x * m_pixelBytes, lineBytes );

			return true;
		}

		if( !_readBack( rect, pDest, buffer.pitch ) )
			return false;

		// The texture holds premultiplied colors.

		if( _isPremultiplied() )
		{
			GpuTools::convertAlpha( pDest, buffer.pitch, pDest, buffer.pitch, rect.w, rect.h, false,
									m_colorSpace == ColorSpace::sRGB, c_bSwapBytes );
		}

		return true;
	}

	//____ pullPixels() __________________________________________________________

	void GpuSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		RectI			rect = bufferRect + buffer.rect.pos();
		const uint8_t *	pSrc = buffer.pixels + bufferRect.y * buffer.pitch + bufferRect.x * m_pixelBytes;

		int handout = _findHandout(buffer.pixels);

		if( !rect.isEmpty() && buffer.pixels )
		{
			if( !m_pPixels && !m_bOnDevice )
				_makeLostPixels();

			bool bOpaque = _writesAlphaMap() && _writeAlphaMap( pSrc, buffer.pitch, rect );

			if( m_pPixels )
			{
				// Handed out while only the texture had our content.

				if( handout >= 0 )
				{
					for( int y = 0; y < rect.h; y++ )
						memcpy( m_pPixels + (rect.y + y) * m_pitch + rect.x * m_pixelBytes, pSrc + y * buffer.pitch, rect.w * m_pixelBytes );
				}

				_addDirtyRect( rect );
			}
			else
			{
				// Copied to the texture from where it is, unless it needs premultiplying or is
				// in memory the GPU can't copy from. Then it goes by staging memory of its own.

				Staging *	pStaging = (handout >= 0 && !m_handouts[handout].bDeviceGone) ? m_handouts[handout].pStaging : nullptr;
				bool		bPremultiply = !bOpaque && _needsPremultiplying( pSrc, buffer.pitch, rect.size() );

				if( pStaging && !bPremultiply )
					_pull( pStaging, bufferRect, rect.pos() );
				else
					_stageFrom( pSrc, buffer.pitch, rect.size(), rect.pos(), bPremultiply );

				m_bAlphaMapStale = true;
			}

			// With all of it written, nothing is missing any more.

			if( rect == RectI(m_size) )
			{
				m_bContentLost = false;
				m_bRestorePending = false;
			}
		}

		// KLUDGE: Surface::copy() into a palette based surface can append colors to the palette
		// through PixelTools::copyPixels(), then calls pullPixels(), so we convert the whole
		// palette again on every pullPixels() to catch that. The intended solution is Surface
		// methods for updating (parts of) the palette, API not yet designed. Replace this once
		// they exist.

		if( m_bIndexed )
			_updatePaletteColors();

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() _____________________________________________________

	void GpuSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
		int handout = _findHandout(buffer.pixels);

		if( handout >= 0 )
		{
			// The GPU may still have a copy from the memory to do, which is for the subclass to
			// keep it for.

			if( m_handouts[handout].pStaging )
				delete m_handouts[handout].pStaging;
			else
				delete [] m_handouts[handout].pPixels;

			m_handouts.erase( m_handouts.begin() + handout );
		}
		else if( m_nInPlaceBuffers > 0 )
		{
			m_nInPlaceBuffers--;
			_releaseSparePixels();
		}
	}

	//____ notifyRendered() ______________________________________________________

	void GpuSurface::notifyRendered()
	{
		m_bAlphaMapStale = true;

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;
	}

	//____ _addDirtyRect() _______________________________________________________

	void GpuSurface::_addDirtyRect( const RectI& rect )
	{
		if( rect.isEmpty() )
			return;

		//TODO: Keep a list of rectangles instead of growing one to cover them all.

		m_dirtyRect = m_dirtyRect.isEmpty() ? rect : RectI::bounds(m_dirtyRect, rect);
		_updatePending();
	}

	//____ _stageDirty() _________________________________________________________

	void GpuSurface::_stageDirty()
	{
		if( m_dirtyRect.isEmpty() || !m_pPixels || !m_bOnDevice )
			return;

		const uint8_t * pSrc = m_pPixels + m_dirtyRect.y * m_pitch + m_dirtyRect.x * m_pixelBytes;

		// Stays to be sent if there was no staging memory to be had.

		if( _stageFrom( pSrc, m_pitch, m_dirtyRect.size(), m_dirtyRect.pos(), _needsPremultiplying(pSrc, m_pitch, m_dirtyRect.size()) ) )
			_dirtyStaged();
	}

	//____ _fillStaging() ________________________________________________________

	void GpuSurface::_fillStaging( const RectI& rect, uint8_t * pDest, int destPitch ) const
	{
		const uint8_t * pSrc = m_pPixels + rect.y * m_pitch + rect.x * m_pixelBytes;

		_convertInto( pSrc, m_pitch, rect.size(), pDest, destPitch, _needsPremultiplying(pSrc, m_pitch, rect.size()) );
	}

	//____ _dirtyStaged() ________________________________________________________

	void GpuSurface::_dirtyStaged()
	{
		m_dirtyRect = RectI();
		_releaseSparePixels();
	}

	//____ _uploaded() ___________________________________________________________

	void GpuSurface::_uploaded()
	{
		if( m_mipLevels > 1 )
			m_bMipmapStale = true;

		_updatePending();
	}

	//____ _leaveDevice() ________________________________________________________

	void GpuSurface::_leaveDevice( bool bDeviceWorks )
	{
		if( !m_bOnDevice )
			return;

		if( !m_pPixels )
		{
			// Only the texture has our content. What was rendered into a canvas is read back if
			// there is a device to read from and nobody has promised to put it back. That isn't
			// done for a surface with restorer: it would be every such texture at once, with
			// colors of low alpha losing precision on the way.

			if( !m_pRestorer && bDeviceWorks )
			{
				m_pPixels = new uint8_t[size_t(m_pitch) * m_size.h];

				if( _readBack( RectI(m_size), m_pPixels, m_pitch ) )
				{
					if( _isPremultiplied() )
					{
						GpuTools::convertAlpha( m_pPixels, m_pitch, m_pPixels, m_pitch, m_size.w, m_size.h, false,
												m_colorSpace == ColorSpace::sRGB, c_bSwapBytes );
					}
				}
				else
				{
					delete [] m_pPixels;
					m_pPixels = nullptr;
				}
			}

			// Otherwise it is gone. Whoever rendered into a canvas gets to know right away, and
			// a restorer is asked once there is a device again.

			if( !m_pPixels )
			{
				m_bRestorePending = true;

				if( !m_pRestorer )
					m_bContentLost = true;
			}
		}

		// Memory handed out stays for whoever has it, but it is of a device we are no longer on.

		for( auto& handout : m_handouts )
			handout.bDeviceGone = true;

		m_bAlphaMapStale = true;
		m_bOnDevice = false;

		m_dirtyRect = m_pPixels ? RectI(m_size) : RectI();			// Sent when we are on a device again.
	}

	//____ _enterDevice() ________________________________________________________

	void GpuSurface::_enterDevice()
	{
		m_bOnDevice = true;

		_releaseSparePixels();
		_updatePending();
	}

	//____ _restore() ____________________________________________________________

	void GpuSurface::_restore()
	{
		if( !m_bRestorePending || !m_bOnDevice )
			return;

		m_bRestorePending = false;

		if( m_pRestorer && m_pRestorer(this) )
		{
			m_bContentLost = false;
			return;
		}

		// Nobody put anything back, so we are transparent black, which has to be sent as a new
		// texture holds anything. If we were written to while there was no device, we are that.

		m_bContentLost = true;

		if( !m_pPixels )
			_makeLostPixels();

		_updatePending();
	}

	//____ _pixelBuffer() ________________________________________________________

	const PixelBuffer GpuSurface::_pixelBuffer( const RectI& rect ) const
	{
		PixelBuffer	buf;

		buf.format = m_pixelFormat;
		buf.colorSpace = m_colorSpace;
		buf.bigEndian = m_bBigEndian;
		buf.palette = m_pPalette;
		buf.pitch = m_pitch;
		buf.pixels = m_pPixels ? m_pPixels + rect.y * m_pitch + rect.x * m_pixelBytes : nullptr;
		buf.rect = rect;

		return buf;
	}

	//____ _updatePaletteColors() ________________________________________________
	//
	// An sRGB texture would have had the hardware convert the colors, but only those read from
	// the texture, and ours are indexes.
	//
	// The buffer is read by the GPU where it is. Should the palette change while a frame using it
	// is still in flight, that frame may see the new colors.

	void GpuSurface::_updatePaletteColors()
	{
		if( m_pPaletteColors && m_pPalette )
			GpuTools::paletteToFloats( m_pPalette, m_paletteCapacity, m_colorSpace, m_pPaletteColors );
	}

	//____ _findHandout() ________________________________________________________

	int GpuSurface::_findHandout( const uint8_t * pPixels ) const
	{
		for( int i = 0; i < int(m_handouts.size()); i++ )
		{
			if( m_handouts[i].pPixels == pPixels )
				return i;
		}

		return -1;
	}

	//____ _needsPremultiplying() ________________________________________________
	//
	// Pixels that are all opaque or transparent black are the same premultiplied.

	bool GpuSurface::_needsPremultiplying( const uint8_t * pPixels, int pitch, SizeI size ) const
	{
		return _isPremultiplied() && !GpuTools::isSamePremultiplied( pPixels, pitch, size.w, size.h, c_bSwapBytes );
	}

	//____ _convertInto() ________________________________________________________
	//
	// Copies pixels as the texture is to have them. pDest is only written to.

	void GpuSurface::_convertInto( const uint8_t * pSrc, int srcPitch, SizeI size, uint8_t * pDest, int destPitch, bool bPremultiply ) const
	{
		if( bPremultiply )
		{
			GpuTools::convertAlpha( pSrc, srcPitch, pDest, destPitch, size.w, size.h, true, m_colorSpace == ColorSpace::sRGB, c_bSwapBytes );
		}
		else
		{
			int lineBytes = size.w * m_pixelBytes;

			for( int y = 0; y < size.h; y++ )
				memcpy( pDest + y * destPitch, pSrc + y * srcPitch, lineBytes );
		}
	}

	//____ _stageFrom() __________________________________________________________
	//
	// Has pixels from anywhere copied to the texture, by staging memory made for it. A subclass
	// that has copied when _pull() returns needs none for pixels that are sent as they are.

	bool GpuSurface::_stageFrom( const uint8_t * pSrc, int srcPitch, SizeI size, CoordI dest, bool bPremultiply )
	{
		if( m_bPullsAtOnce && !bPremultiply )
		{
			Staging direct;

			direct.pPixels = const_cast<uint8_t*>(pSrc);
			direct.pitch = srcPitch;

			_pull( &direct, RectI(0, 0, size.w, size.h), dest );
			return true;
		}

		Staging * pStaging = _allocStaging( size, false );

		if( !pStaging )
			return false;

		_convertInto( pSrc, srcPitch, size, pStaging->pPixels, pStaging->pitch, bPremultiply );
		_pull( pStaging, RectI(0, 0, size.w, size.h), dest );

		delete pStaging;
		return true;
	}

	//____ _makeLostPixels() _____________________________________________________
	//
	// Pixels for a surface that has neither them nor a texture with its content: transparent
	// black, to be sent when there is a texture to send them to.

	void GpuSurface::_makeLostPixels()
	{
		size_t bytes = size_t(m_pitch) * m_size.h;

		m_pPixels = new uint8_t[bytes];
		memset( m_pPixels, 0, bytes );

		if( _writesAlphaMap() )
		{
			if( !m_pAlphaMap )
				m_pAlphaMap = new uint8_t[size_t(m_size.w) * m_size.h];

			memset( m_pAlphaMap, 0, size_t(m_size.w) * m_size.h );
		}

		m_bAlphaMapStale = true;
		m_dirtyRect = RectI(m_size);
	}

	//____ _releaseSparePixels() _________________________________________________
	//
	// A surface that keeps no pixels lets go of those it has once they are on their way to its
	// texture and nobody has a PixelBuffer pointing into them.

	void GpuSurface::_releaseSparePixels()
	{
		if( !m_pPixels || _keepsPixels() || !m_bOnDevice || !m_dirtyRect.isEmpty() || m_nInPlaceBuffers > 0 )
			return;

		delete [] m_pPixels;
		m_pPixels = nullptr;

		m_bAlphaMapStale = true;
	}

	//____ _writeAlphaMap() ______________________________________________________
	//
	// Brings the alpha map up to date with pixels written to the given rectangle of us. There
	// is no map as long as all that has been written is opaque, which is most of what isn't
	// drawn by hand. Returns true when the pixels are known to be opaque.

	bool GpuSurface::_writeAlphaMap( const uint8_t * pPixels, int pitch, const RectI& rect )
	{
		if( !m_pAlphaMap )
		{
			// ARGB_8 is all we get here, with alpha in the last byte of each pixel.

			bool bOpaque = true;

			for( int y = 0; y < rect.h && bOpaque; y++ )
			{
				const uint8_t * p = pPixels + y * pitch + 3;

				for( int x = 0; x < rect.w; x++ )
				{
					if( p[x * 4] != 255 )
					{
						bOpaque = false;
						break;
					}
				}
			}

			if( bOpaque )
				return true;

			m_pAlphaMap = new uint8_t[size_t(m_size.w) * m_size.h];
			memset( m_pAlphaMap, 255, size_t(m_size.w) * m_size.h );
		}

		PixelTools::extractAlphaChannel( m_pixelFormat, m_bBigEndian, pPixels, pitch, RectI(0, 0, rect.w, rect.h),
										 m_pAlphaMap + rect.y * m_size.w + rect.x, m_size.w, nullptr );
		return false;
	}

	//____ _readAlphaMap() _______________________________________________________
	//
	// This stalls until the GPU is done, but only happens when someone asks for the alpha of a
	// canvas that has changed since it was last asked for.

	bool GpuSurface::_readAlphaMap()
	{
		if( m_pAlphaMap && !m_bAlphaMapStale )
			return true;

		if( !m_bOnDevice )
			return false;

		int			pitch = m_size.w * m_pixelBytes;
		uint8_t *	pPixels = new uint8_t[size_t(pitch) * m_size.h];

		bool bRead = _readBack( RectI(m_size), pPixels, pitch );

		if( bRead )
		{
			if( !m_pAlphaMap )
				m_pAlphaMap = new uint8_t[size_t(m_size.w) * m_size.h];

			PixelTools::extractAlphaChannel( m_pixelFormat, m_bBigEndian, pPixels, pitch, RectI(m_size), m_pAlphaMap, m_size.w, nullptr );
			m_bAlphaMapStale = false;
		}

		delete [] pPixels;
		return bRead;
	}

} // namespace wax
