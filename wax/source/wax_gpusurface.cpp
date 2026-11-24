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

	GpuSurface::GpuSurface( const Blueprint& bp, int pitchAlignment ) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;
		m_pitchAlignment = pitchAlignment;
	}

	//____ destructor ____________________________________________________________

	GpuSurface::~GpuSurface()
	{
		delete [] m_pFallbackPixels;
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

		m_pitch = (m_size.w * m_pixelBytes + m_pitchAlignment - 1) & ~(m_pitchAlignment - 1);

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

		// A canvas that nobody has given anything to start out with or said to be buffered gets
		// no pixel buffer until its pixels are asked for, see _needPixels(). That is half its
		// memory and a good part of the time it takes to make it, which matters for canvas
		// layers, made for every canvas update.

		bool bPixelBuffer = source.pPixels || !bp.fillColor.isUndefined() || !m_bCanvas || m_bBuffered;

		// A surface with nothing on the device is of no use to the GPU, but it still has to
		// behave like a surface: hand out pixel buffers, be readable, be writable.

		if( !_createDeviceObjects(bPixelBuffer) )
		{
			m_pFallbackPixels = new uint8_t[size_t(m_pitch) * m_size.h];
			m_pPixels = m_pFallbackPixels;
		}

		// Without pixels and without a fill color our pixels are left as the memory came and
		// nothing is sent to the texture, as the content of such a surface is unspecified.

		if( m_pPixels && source.pPixels )
		{
			// A source without a palette size, or palette based without a palette, is taken to
			// have as many colors as we do.

			bool bSourceIndexed = (source.description.type == PixelType::Index || source.description.type == PixelType::Bitplanes);

			int paletteSize = (source.paletteSize == 0 || (bSourceIndexed && !source.pPalette)) ? m_paletteSize : source.paletteSize;

			PixelTools::copyPixels(m_size.w, m_size.h, source.pPixels, source.description, m_colorSpace,
								   source.pitch - PixelTools::bytesPerLine(source.description, m_size.w),
								   source.pPalette, paletteSize,
								   m_pPixels, m_pixelFormat, m_colorSpace, m_bBigEndian, m_pitch - m_size.w * m_pixelBytes,
								   m_pPalette, m_paletteSize, m_paletteCapacity);

			_addDirtyRect( RectI(m_size) );
		}
		else if( m_pPixels && !bp.fillColor.isUndefined() )
		{
			PixelTools::fillBitmap(m_pPixels, m_pixelFormat, m_colorSpace, m_bBigEndian, m_pitch, RectI(m_size),
								   bp.fillColor, m_pPalette, m_paletteSize);

			_addDirtyRect( RectI(m_size) );
		}

		_updatePaletteColors();
	}

	//____ alpha() _______________________________________________________________

	int GpuSurface::alpha(CoordSPX coord)
	{
		if( !_needPixels() )
			return 0;

		_syncPixels();

		// We only read, so an upload reading the same pixels is no concern.

		return _alpha(coord, _pixelBuffer(RectI(m_size)));
	}

	//____ allocPixelBuffer() ____________________________________________________
	//
	// Our pixels are in memory the CPU can reach, so this just points into it, once a canvas
	// that was created without has got them. Note that the pitch can be padded.

	const PixelBuffer GpuSurface::allocPixelBuffer(const RectI& rect)
	{
		_needPixels();
		_waitForUpload();			// Whoever gets it may write to it.

		return _pixelBuffer(rect);
	}

	//____ pushPixels() __________________________________________________________
	//
	// The buffer is our pixels, so it is already up to date unless something has been rendered
	// into us since we last looked.

	bool GpuSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		//TODO: Only read back the lines asked for, not the whole texture.

		if( !_needPixels() )
			return false;

		_syncPixels();
		return true;
	}

	//____ pullPixels() __________________________________________________________

	void GpuSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		_addDirtyRect( bufferRect + buffer.rect.pos() );

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
		// Nothing to free, it points into our pixels.
	}

	//____ notifyRendered() ______________________________________________________

	void GpuSurface::notifyRendered()
	{
		m_bBufferNeedsSync = true;

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

	//____ _uploaded() ___________________________________________________________

	void GpuSurface::_uploaded()
	{
		m_dirtyRect = RectI();
		_updatePending();

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;
	}

	//____ _needPixels() _________________________________________________________
	//
	// Called before our pixels are handed out or read. A canvas created without a pixel buffer,
	// see _setup(), gets it here. What it has is in the texture, so the buffer starts out as the
	// older copy, to be read back into before it is read.

	bool GpuSurface::_needPixels()
	{
		if( m_pPixels )
			return true;

		if( !_createPixelBuffer() )
			return false;

		m_bBufferNeedsSync = true;
		return true;
	}

	//____ _syncPixels() _________________________________________________________
	//
	// This stalls until the GPU is done, but only happens when someone asks for the pixels of a
	// surface that has been rendered into.

	void GpuSurface::_syncPixels()
	{
		if( !m_bBufferNeedsSync || !m_pPixels )
			return;

		if( !_readBack() )
			return;						// Leaves the flag set, so we try again rather than hand out stale pixels.

		m_bBufferNeedsSync = false;

		// The texture holds premultiplied colors.

		if( _isPremultiplied() )
		{
			GpuTools::convertAlpha( m_pPixels, m_pitch, m_pPixels, m_pitch, m_size.w, m_size.h, false,
									m_colorSpace == ColorSpace::sRGB, c_bSwapBytes );
		}

		m_dirtyRect = RectI();			// Our pixels and the texture are identical now.
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

	//____ _needsPremultiplying() ________________________________________________

	bool GpuSurface::_needsPremultiplying( const RectI& rect ) const
	{
		return _isPremultiplied() &&
			   !GpuTools::isSamePremultiplied( m_pPixels + rect.y * m_pitch + rect.x * m_pixelBytes, m_pitch, rect.w, rect.h, c_bSwapBytes );
	}

	//____ _premultiply() ________________________________________________________

	void GpuSurface::_premultiply( const RectI& rect, uint8_t * pDest, int destPitch ) const
	{
		GpuTools::convertAlpha( m_pPixels + rect.y * m_pitch + rect.x * m_pixelBytes, m_pitch, pDest, destPitch, rect.w, rect.h, true,
								m_colorSpace == ColorSpace::sRGB, c_bSwapBytes );
	}

} // namespace wax
