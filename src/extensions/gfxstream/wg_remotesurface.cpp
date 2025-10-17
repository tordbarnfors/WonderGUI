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

#include <wg_remotesurface.h>
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
	const TypeInfo RemoteSurface::TYPEINFO = { "RemoteSurface", &Surface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI RemoteSurface::maxSize()
	{
		return SizeI(4096,4096);
	}

	//____ create ______________________________________________________________

	RemoteSurface_p RemoteSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint)
	{
		return RemoteSurface_p(new RemoteSurface(pEncoder, blueprint));
	}

	RemoteSurface_p RemoteSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return RemoteSurface_p(new RemoteSurface(pEncoder, blueprint, pBlob, pitch));
	}

	RemoteSurface_p RemoteSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return RemoteSurface_p(new RemoteSurface(pEncoder, blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	RemoteSurface_p RemoteSurface::create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
												PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return RemoteSurface_p(new RemoteSurface(pEncoder, blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}



	//____ constructor _____________________________________________________________

	RemoteSurface::RemoteSurface( StreamEncoder * pEncoder, const Blueprint& bp) : Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod() )
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;

		_sendCreateSurface(pEncoder);
		m_pEncoder->flush();
	}

	RemoteSurface::RemoteSurface( StreamEncoder * pEncoder, const Blueprint& bp, Blob* pBlob, int pitch )
		: Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		//TODO: Support for palette!

		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;

		_sendCreateSurface(pEncoder);
		_sendPixels(m_pEncoder, bp.size, (uint8_t*) pBlob->data(), pitch);
		m_pEncoder->flush();
	}

	RemoteSurface::RemoteSurface(StreamEncoder* pEncoder, const Blueprint& bp, const uint8_t* pPixels, PixelFormat srcFormat, int pitch, const Color8* pPalette, int paletteSize)
		: Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		//TODO: Support for palette!

		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;

		_sendCreateSurface(pEncoder);

		if (srcFormat == m_pixelFormat)
		{
			_sendPixels(m_pEncoder, m_size, pPixels, pitch);
		}
		else
		{
			int srcPitchAdd = pitch == 0 ? 0 : pitch - Util::pixelFormatToDescription(srcFormat).bits / 8 * m_size.w;

			int dstPitch = m_size.w * m_pPixelDescription->bits / 8;

			auto pBlob = Blob::create(dstPitch * m_size.h);

			PixelTools::copyPixels(m_size.w, m_size.h, pPixels, srcFormat, srcPitchAdd,
				(uint8_t*)pBlob->data(), m_pixelFormat, 0, pPalette,
				m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);

			_sendPixels(m_pEncoder, m_size, (uint8_t*)pBlob->data(), dstPitch);
		}

		m_pEncoder->flush();
	}

	RemoteSurface::RemoteSurface( StreamEncoder * pEncoder, const Blueprint& bp, const uint8_t * pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize )
		: Surface(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		//TODO: Support for palette!

		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;

		_sendCreateSurface(pEncoder);

		// We always convert the data for streaming.
		// (but we could optimize and skip conversion if format already is correct)

		int srcPitchAdd = pitch == 0 ? 0 : pitch - pixelDescription.bits / 8 * m_size.w;
		int dstPitch = m_size.w * m_pPixelDescription->bits / 8;

		auto pBlob = Blob::create(dstPitch*m_size.h);

		PixelTools::copyPixels(m_size.w, m_size.h, pPixels, pixelDescription, srcPitchAdd,
							 (uint8_t*) pBlob->data(), m_pixelFormat, 0, pPalette,
							 m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);
		
		_sendPixels(m_pEncoder, m_size, (uint8_t*) pBlob->data(), dstPitch);
		m_pEncoder->flush();
	}

	
	//____ Destructor ______________________________________________________________

	RemoteSurface::~RemoteSurface()
	{
		_sendDeleteSurface();
		m_pEncoder->freeObjectId(m_inStreamId);
		m_pEncoder->flush();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RemoteSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer RemoteSurface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;

		buf.pitch = ((rect.w + 3) & 0xFFFFFFFC) * m_pPixelDescription->bits / 8;
		buf.pixels = new uint8_t[buf.pitch*rect.h];
		buf.palette = m_pPalette;
		buf.format = m_pixelFormat;
		buf.rect = rect;

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool RemoteSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
			return false;
	}

	//____ pullPixels() _______________________________________________________

	void RemoteSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		_sendPixels(m_pEncoder, buffer.rect, buffer.pixels, buffer.pitch);
		m_pEncoder->flush();

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() __________________________________________________

	void RemoteSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
			delete[] buffer.pixels;
	}

	//____ alpha() ____________________________________________________________

	int RemoteSurface::alpha( CoordSPX coord )
	{
		return 4096;
	}

	//____ fill() _____________________________________________________________

	bool RemoteSurface::fill(HiColor color)
	{
		return fill(RectI(0, 0, pixelSize()), color );
	}

	bool RemoteSurface::fill(const RectI& region, HiColor color )
	{
		*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::FillSurface, 2+16+8 };
		*m_pEncoder << m_inStreamId;
		*m_pEncoder << region;
		*m_pEncoder << color;
		m_pEncoder->flush();

		return true;
	}

	//____ recreateInStream() ______________________________________________________

	bool RemoteSurface::recreateInStream(StreamEncoder* pEncoder)
	{
		_sendCreateSurface(pEncoder);
		pEncoder->flush();
		return true;
	}


	//____ _sendCreateSurface() _______________________________________________

	void RemoteSurface::_sendCreateSurface(StreamEncoder* pEncoder)
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

	//____ _sendPixels() _________________________________________________________

	void RemoteSurface::_sendPixels(StreamEncoder* pEncoder, RectI rect, const uint8_t * pSource, int pitch)
	{
		//TODO: Avoid copy if pitch matches and no compression.

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

		StreamBackend::_splitAndEncode(pEncoder, GfxStream::ChunkId::SurfacePixels, Compression::None, pBuffer, pBuffer + dataSize, pixelSize);

		GfxBase::memStackFree(allocSize);
	}

	//____ _sendDeleteSurface() _______________________________________________

	void RemoteSurface::_sendDeleteSurface()
	{
		*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::DeleteSurface, 0, 2 };
		*m_pEncoder << m_inStreamId;

	}

} // namespace wg
