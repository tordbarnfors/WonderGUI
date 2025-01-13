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

#include <wg_streamsurface.h>
#include <wg_gfxutil.h>
#include <wg_blob.h>
#include <wg_pixeltools.h>
#include <wg_gfxbase.h>
#include <wg_streambackend.h>

#include <assert.h>
#include <cstring>
#include <algorithm>



namespace wg
{
	const TypeInfo StreamSurface::TYPEINFO = { "StreamSurface", &Surface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI StreamSurface::maxSize()
	{
		return SizeI(4096,4096);
	}

	//____ create ______________________________________________________________

	StreamSurface_p StreamSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint)
	{
		return StreamSurface_p(new StreamSurface(pEncoder, blueprint));
	}

	StreamSurface_p StreamSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return StreamSurface_p(new StreamSurface(pEncoder, blueprint, pBlob, pitch));
	}

	StreamSurface_p StreamSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return StreamSurface_p(new StreamSurface(pEncoder, blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	StreamSurface_p StreamSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
												PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return StreamSurface_p(new StreamSurface(pEncoder, blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}



	//____ constructor _____________________________________________________________

	StreamSurface::StreamSurface( StreamEncoder * pEncoder, const Blueprint& bp) : Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod() )
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_pitch = ((bp.size.w + 3) & 0xFFFFFFFC)*m_pPixelDescription->bits / 8;
		m_bDynamic = bp.dynamic;

		if (m_pPixelDescription->bits <= 8 || bp.buffered)
		{
			m_pBlob = Blob::create(m_pitch*bp.size.h + m_paletteCapacity*sizeof(Color8) );
			std::memset(m_pBlob->data(), 0, m_pitch*bp.size.h);

			if (bp.palette)
			{
				m_pPalette = (Color8*)((uint8_t*)m_pBlob->data() + m_pitch * bp.size.h);
				memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8) );
			}
			else
				m_pPalette = nullptr;

			m_pAlphaLayer = nullptr;
		}
		else
		{
			if (m_pPixelDescription->A_mask == 0)
				m_pAlphaLayer = nullptr;
			else
			{
				m_pAlphaLayer = new uint8_t[bp.size.w*bp.size.h];
				std::memset(m_pAlphaLayer, 0, bp.size.w*bp.size.h);
			}
		}

		_sendCreateSurface(pEncoder);
		m_pEncoder->flush();
	}

	StreamSurface::StreamSurface( StreamEncoder * pEncoder, const Blueprint& bp, Blob* pBlob, int pitch )
		: Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_pitch = pitch;
		m_bDynamic = bp.dynamic;


		if (m_pPixelDescription->bits <= 8 || bp.buffered)
		{
			m_pBlob = pBlob;
			m_pPalette = const_cast<Color8*>(bp.palette);
			m_pAlphaLayer = nullptr;
		}
		else
		{
			if (m_pPixelDescription->A_mask == 0)
				m_pAlphaLayer = nullptr;
			else
				m_pAlphaLayer = _genAlphaLayer((char*)pBlob->data(), pitch);
		}

		_sendCreateSurface(pEncoder);
		_sendPixels(m_pEncoder, bp.size, (uint8_t*) pBlob->data(), pitch);
		m_pEncoder->flush();
	}

	StreamSurface::StreamSurface( StreamEncoder * pEncoder, const Blueprint& bp, const uint8_t * pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize )
		: Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_pitch = ((bp.size.w + 3) & 0xFFFFFFFC)*m_pPixelDescription->bits / 8;
		m_bDynamic = bp.dynamic;

		// We always convert the data even if we throw it away, since we need to stream the converted data.
		// (but we could optimize and skip conversion if format already is correct)

		m_pBlob = Blob::create(m_pitch*m_size.h + m_paletteCapacity*sizeof(Color8) );

		if( m_paletteCapacity > 0 )
		{
			m_pPalette = (Color8*)((uint8_t*)m_pBlob->data() + m_pitch * bp.size.h);
			memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8));
		}
				
		int srcPitchAdd = pitch == 0 ? 0 : pitch - pixelDescription.bits/8 * m_size.w;

		PixelTools::copyPixels(m_size.w, m_size.h, pPixels, pixelDescription, srcPitchAdd,
							 (uint8_t*) m_pBlob->data(), m_pixelFormat, m_pitch - m_pPixelDescription->bits/8 * m_size.w, pPalette,
							 m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);
		
		_sendCreateSurface(pEncoder);
		_sendPixels(m_pEncoder, m_size, (uint8_t*) m_pBlob->data(), m_pitch);
		m_pEncoder->flush();

		if (m_pPixelDescription->bits <= 8 || bp.buffered)
			m_pAlphaLayer = nullptr;
		else
		{
			m_pBlob = nullptr;
			m_pPalette = nullptr;

			if (m_pPixelDescription->A_mask == 0)
				m_pAlphaLayer = nullptr;
			else
				m_pAlphaLayer = _genAlphaLayer((char*)pPixels, pitch);
		}
	}

	StreamSurface::StreamSurface( StreamEncoder * pEncoder, const Blueprint& bp, const uint8_t * pPixels, PixelFormat srcFormat, int pitch, const Color8 * pPalette, int paletteSize )
		: Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_pitch = ((bp.size.w + 3) & 0xFFFFFFFC)*m_pPixelDescription->bits / 8;
		m_bDynamic = bp.dynamic;


		// We always convert the data even if we throw it away, since we need to stream the converted data.
		// (but we could optimize and skip conversion if format already is correct)

		m_pBlob = Blob::create(m_pitch*m_size.h + m_paletteCapacity*sizeof(Color8) );

		if( m_paletteCapacity > 0 )
		{
			m_pPalette = (Color8*)((uint8_t*)m_pBlob->data() + m_pitch * bp.size.h);
			memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8));
		}
		
		int srcPitchAdd = pitch == 0 ? 0 : pitch - Util::pixelFormatToDescription(srcFormat).bits/8 * m_size.w;

		_sendCreateSurface(pEncoder);
		PixelTools::copyPixels(m_size.w, m_size.h, pPixels, srcFormat, srcPitchAdd,
							 (uint8_t*) m_pBlob->data(), m_pixelFormat, m_pitch - m_pPixelDescription->bits/8 * m_size.w, pPalette,
							 m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);
		
		_sendPixels(m_pEncoder, m_size, (uint8_t*) m_pBlob->data(), m_pitch);
		m_pEncoder->flush();

		if (m_pPixelDescription->bits <= 8 || bp.buffered)
			m_pAlphaLayer = nullptr;
		else
		{
			m_pBlob = nullptr;
			m_pPalette = nullptr;

			if (m_pPixelDescription->A_mask == 0)
				m_pAlphaLayer = nullptr;
			else
				m_pAlphaLayer = _genAlphaLayer((char*)pPixels, pitch);
		}
	}

	//____ Destructor ______________________________________________________________

	StreamSurface::~StreamSurface()
	{
		_sendDeleteSurface();
		m_pEncoder->freeObjectId(m_inStreamId);
		m_pEncoder->flush();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer StreamSurface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;

		if (m_pBlob)
		{
			buf.pixels = (uint8_t*) m_pBlob->data() + m_pitch * rect.y + rect.x * m_pPixelDescription->bits / 8;
			buf.palette = m_pPalette;
			buf.format = m_pixelFormat;
			buf.rect = rect;
			buf.pitch = m_pitch;
		}
		else
		{
			buf.pitch = ((rect.w + 3) & 0xFFFFFFFC) * m_pPixelDescription->bits / 8;
			buf.pixels = new uint8_t[buf.pitch*rect.h];
			buf.palette = m_pPalette;
			buf.format = m_pixelFormat;
			buf.rect = rect;
		}

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool StreamSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		if (m_pBlob)
			return true;
		else
			return false;
	}

	//____ pullPixels() _______________________________________________________

	void StreamSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		_sendPixels(m_pEncoder, buffer.rect, buffer.pixels, buffer.pitch);
		m_pEncoder->flush();
	}

	//____ freePixelBuffer() __________________________________________________

	void StreamSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
		if (!m_pBlob)
			delete[] buffer.pixels;
	}

	//____ alpha() ____________________________________________________________

	int StreamSurface::alpha( CoordSPX coord )
	{
		PixelBuffer	buffer;

		if (m_pAlphaLayer)
		{
			buffer.format = PixelFormat::Alpha_8;
			buffer.palette = nullptr;
			buffer.pitch = m_size.w;
			buffer.pixels = m_pAlphaLayer;
			buffer.rect = { 0,0,m_size };
		}
		else
		{
			buffer.format = m_pixelFormat;
			buffer.palette = m_pPalette;
			buffer.pitch = m_pitch;
			buffer.pixels = (uint8_t*) m_pBlob->data();
			buffer.rect = { 0,0,m_size };
		}

		return _alpha(coord, buffer);
	}

	//____ fill() _____________________________________________________________

	bool StreamSurface::fill(HiColor color)
	{
		return fill(RectI(0, 0, pixelSize()), color );
	}

	bool StreamSurface::fill(const RectI& region, HiColor color )
	{
		// Stream the call

		*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::FillSurface, 2+16+8 };
		*m_pEncoder << m_inStreamId;
		*m_pEncoder << region;
		*m_pEncoder << color;
		m_pEncoder->flush();

		// Update local copy or alpha channel (if any)

		if (m_pBlob)
		{
			PixelTools::fillBitmap((uint8_t*) m_pBlob->data(), m_pixelFormat, m_pitch, region, color);
		}
		else if (m_pAlphaLayer)
		{
			for (int y = region.y ; y < region.y + region.h ; y++ )
			{
				uint8_t * p = m_pAlphaLayer + y * m_size.w + region.x;
				for( int x = 0 ; x < region.h ; x++ )
					* p++ = color.a;
			}
		}

		//
		return true;
	}

	//____ streamAsNew() ______________________________________________________

	bool StreamSurface::streamAsNew(StreamEncoder* pEncoder)
	{
		if (!m_pBlob)
			return false;

		_sendCreateSurface(pEncoder);

		_sendPixels(pEncoder, m_size, (uint8_t*)m_pBlob->data(), m_pitch);
		pEncoder->flush();
		return true;
	}


	//____ _sendCreateSurface() _______________________________________________

	void StreamSurface::_sendCreateSurface(StreamEncoder* pEncoder)
	{
		uint16_t blockSize = 38 + m_paletteSize*4;

		*pEncoder << GfxStream::Header{ GfxStream::ChunkId::CreateSurface, 0, blockSize };
		*pEncoder << m_inStreamId;
		*pEncoder << m_bCanvas;
		*pEncoder << m_bDynamic;
		*pEncoder << m_pixelFormat;
		*pEncoder << m_id;
		*pEncoder << m_bMipmapped;
		*pEncoder << m_sampleMethod;
		*pEncoder << m_scale;
		*pEncoder << m_size;
		*pEncoder << m_bTiling;
		*pEncoder << m_paletteCapacity;
		*pEncoder << m_paletteSize;

		if (m_pPalette)
			* pEncoder << GfxStream::WriteBytes{ m_paletteSize*4, m_pPalette };
	}

	//____ _genAlphaLayer() _________________________________________________________

	uint8_t * StreamSurface::_genAlphaLayer(const char * pSource, int pitch)
	{
		uint8_t * pAlphaLayer = new uint8_t[m_size.w*m_size.h];

		//TODO: Support more than 8 bit alpha.

		uint8_t * pDest = pAlphaLayer;
		int pixelPitch = m_pPixelDescription->bits / 8;
		for (int y = 0; y < m_size.h; y++)
		{
			const char * pPixelAlpha = pSource + y*pitch + pixelPitch -1;

			for (int x = 0; x < m_size.w; x++)
			{
				*pDest++ = *pPixelAlpha;
				pPixelAlpha += pixelPitch;
			}
		}
		return pAlphaLayer;
	}

	//____ _sendPixels() _________________________________________________________

	void StreamSurface::_sendPixels(StreamEncoder* pEncoder, RectI rect, const uint8_t * pSource, int pitch)
	{
		int	pixelSize = m_pPixelDescription->bits / 8;
		int dataSize = rect.w * rect.h * pixelSize;

		int allocSize = dataSize;		// Increase if needed by future compression

		auto pBuffer = (uint8_t *) GfxBase::memStackAlloc(allocSize);

		auto pLine = pSource;
		auto pDest = pBuffer;
		int lineLength = rect.w * pixelSize;

		for( int y = 0 ; y < rect.h ; y++ )
		{
			memcpy( pDest, pLine, lineLength );
			pLine += pitch;
			pDest += lineLength;
		}

		*pEncoder << GfxStream::Header{ GfxStream::ChunkId::SurfaceUpdate, 0, 18 };
		*pEncoder << m_inStreamId;
		*pEncoder << rect;

		StreamBackend::_splitAndEncode(pEncoder, GfxStream::ChunkId::SurfacePixels, pBuffer, pBuffer + dataSize, pixelSize);

		GfxBase::memStackFree(allocSize);
	}

	//____ _sendDeleteSurface() _______________________________________________

	void StreamSurface::_sendDeleteSurface()
	{
		*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::DeleteSurface, 0, 2 };
		*m_pEncoder << m_inStreamId;

	}

} // namespace wg
