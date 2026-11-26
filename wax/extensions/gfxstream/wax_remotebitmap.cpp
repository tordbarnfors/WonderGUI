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
#include <memory.h>

#include <wax_remotebitmap.h>
#include <wax_util.h>
#include <wax_blob.h>
#include <wax_pixeltools.h>
#include <wax_base.h>
#include <wax_streambackend.h>

#include <assert.h>
#include <cstring>
#include <algorithm>



namespace wax
{
	const TypeInfo RemoteBitmap::TYPEINFO = { "RemoteBitmap", &Bitmap::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI RemoteBitmap::maxSize()
	{
		return SizeI(4096,4096);
	}

	//____ create ______________________________________________________________

	RemoteBitmap_p RemoteBitmap::create(StreamEncoder * pEncoder, const Blueprint& blueprint)
	{
		return RemoteBitmap_p(new RemoteBitmap(pEncoder, blueprint));
	}

	RemoteBitmap_p RemoteBitmap::create(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return RemoteBitmap_p(new RemoteBitmap(pEncoder, blueprint, pBlob, pitch));
	}

	RemoteBitmap_p RemoteBitmap::create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return RemoteBitmap_p(new RemoteBitmap(pEncoder, blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	RemoteBitmap_p RemoteBitmap::create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
												PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		return RemoteBitmap_p(new RemoteBitmap(pEncoder, blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}



	//____ constructor _____________________________________________________________

	RemoteBitmap::RemoteBitmap( StreamEncoder * pEncoder, const Blueprint& bp) : Bitmap(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod() )
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;
		m_fillColor = bp.fillColor;
		_initPalette(bp);

		_sendCreateBitmap(pEncoder);
		m_pEncoder->flush();
	}

	RemoteBitmap::RemoteBitmap( StreamEncoder * pEncoder, const Blueprint& bp, Blob* pBlob, int pitch )
		: Bitmap(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;
		_initPalette(bp);

		_sendCreateBitmap(pEncoder);
		_sendPixels(m_pEncoder, bp.size, (uint8_t*) pBlob->data(), pitch);
		m_pEncoder->flush();
	}

	RemoteBitmap::RemoteBitmap(StreamEncoder* pEncoder, const Blueprint& bp, const uint8_t* pPixels, PixelFormat srcFormat, int pitch, const Color8* pPalette, int paletteSize)
		: Bitmap(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;
		_initPalette(bp);

		// Source pixels are in the byte order and color space of the blueprint, which are ours.

		if (srcFormat == PixelFormat::Undefined)
			srcFormat = m_pixelFormat;

		auto srcDesc = Util::pixelFormatToDescription(srcFormat, m_bBigEndian);

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

		if (srcFormat == m_pixelFormat)
		{
			_sendCreateBitmap(pEncoder);
			_sendPixels(m_pEncoder, m_size, pPixels, pitch);
		}
		else
		{
			int srcPitchAdd = pitch - PixelTools::bytesPerLine(srcDesc, m_size.w);

			int dstPitch = PixelTools::bytesPerLine(*m_pPixelDescription, m_size.w);

			auto pBlob = Blob::create(dstPitch * m_size.h);

			PixelTools::copyPixels(m_size.w, m_size.h, pPixels, srcDesc, m_colorSpace, srcPitchAdd, pPalette, paletteSize,
				(uint8_t*)pBlob->data(), m_pixelFormat, m_colorSpace, m_bBigEndian, 0,
				m_pPalette, m_paletteSize, m_paletteCapacity);

			// Conversion might have built our palette, so we create the bitmap first now.

			_sendCreateBitmap(pEncoder);
			_sendPixels(m_pEncoder, m_size, (uint8_t*)pBlob->data(), dstPitch);
		}

		m_pEncoder->flush();
	}

	RemoteBitmap::RemoteBitmap( StreamEncoder * pEncoder, const Blueprint& bp, const uint8_t * pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize )
		: Bitmap(bp, pEncoder->defaultPixelFormat(), pEncoder->defaultSampleMethod())
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_bDynamic = bp.dynamic;
		_initPalette(bp);

		// We always convert the data for streaming.
		// Conversion might build our palette, so the bitmap is created after it.
		// (but we could optimize and skip conversion if format already is correct)

		int srcPitchAdd = pitch == 0 ? 0 : pitch - PixelTools::bytesPerLine(pixelDescription, m_size.w);
		int dstPitch = PixelTools::bytesPerLine(*m_pPixelDescription, m_size.w);

		auto pBlob = Blob::create(dstPitch*m_size.h);

		PixelTools::copyPixels(m_size.w, m_size.h, pPixels, pixelDescription, m_colorSpace, srcPitchAdd, pPalette, paletteSize,
							 (uint8_t*) pBlob->data(), m_pixelFormat, m_colorSpace, m_bBigEndian, 0,
							 m_pPalette, m_paletteSize, m_paletteCapacity);

		_sendCreateBitmap(pEncoder);
		_sendPixels(m_pEncoder, m_size, (uint8_t*) pBlob->data(), dstPitch);
		m_pEncoder->flush();
	}

	
	//____ Destructor ______________________________________________________________

	RemoteBitmap::~RemoteBitmap()
	{
		_sendDeleteBitmap();
		m_pEncoder->freeObjectId(m_inStreamId);
		m_pEncoder->flush();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RemoteBitmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer RemoteBitmap::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;

		buf.pitch = ((rect.w + 3) & 0xFFFFFFFC) * m_pPixelDescription->bits / 8;
		buf.pixels = new uint8_t[buf.pitch*rect.h];
		buf.palette = m_pPalette;
		buf.format = m_pixelFormat;
		buf.colorSpace = m_colorSpace;
		buf.bigEndian = m_bBigEndian;
		buf.rect = rect;

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool RemoteBitmap::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
			return false;
	}

	//____ pullPixels() _______________________________________________________

	void RemoteBitmap::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		_sendPixels(m_pEncoder, buffer.rect, buffer.pixels, buffer.pitch);
		m_pEncoder->flush();

		Bitmap::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() __________________________________________________

	void RemoteBitmap::freePixelBuffer(const PixelBuffer& buffer)
	{
			delete[] buffer.pixels;
	}

	//____ alpha() ____________________________________________________________

	int RemoteBitmap::alpha( CoordSPX coord )
	{
		return 4096;
	}

	//____ fill() _____________________________________________________________

	bool RemoteBitmap::fill(Color color)
	{
		return fill(RectI(0, 0, pixelSize()), color );
	}

	bool RemoteBitmap::fill(const RectI& region, Color color )
	{
		*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::FillBitmap, 0, 2+16+8 };
		*m_pEncoder << m_inStreamId;
		*m_pEncoder << region;
		*m_pEncoder << color;
		m_pEncoder->flush();

		return true;
	}

	//____ recreateInStream() ______________________________________________________

	bool RemoteBitmap::recreateInStream(StreamEncoder* pEncoder)
	{
		_sendCreateBitmap(pEncoder);
		pEncoder->flush();
		return true;
	}


	//____ _initPalette() _____________________________________________________

	void RemoteBitmap::_initPalette(const Blueprint& bp)
	{
		if (m_paletteCapacity == 0)
			return;

		m_palette.resize(m_paletteCapacity, Color8(0, 0, 0, 0));
		if (bp.palette)
			memcpy(m_palette.data(), bp.palette, m_paletteSize * sizeof(Color8));
		m_pPalette = m_palette.data();
	}

	//____ _sendCreateBitmap() _______________________________________________

	void RemoteBitmap::_sendCreateBitmap(StreamEncoder* pEncoder)
	{
		*pEncoder << GfxStream::Header{ GfxStream::ChunkId::CreateBitmap, 0, 48 };
		*pEncoder << m_inStreamId;
		*pEncoder << m_bCanvas;
		*pEncoder << m_bDynamic;
		*pEncoder << m_pixelFormat;
		*pEncoder << m_colorSpace;
		*pEncoder << (uint8_t) m_bBigEndian;
		*pEncoder << m_id;
		*pEncoder << m_bMipmapped;
		*pEncoder << m_sampleMethod;
		*pEncoder << m_scale;
		*pEncoder << m_size;
		*pEncoder << m_bTiling;
		*pEncoder << m_paletteCapacity;
		*pEncoder << m_paletteSize;
		*pEncoder << m_fillColor;

		// The palette follows in chunks of its own, since a large one doesn't fit in one.
		// The bitmap is created on the other end once all of it has arrived.

		if (m_pPalette && m_paletteSize > 0)
			StreamBackend::_compressSplitAndEncode(pEncoder, GfxStream::ChunkId::BitmapPalette, m_inStreamId, nullptr, m_pPalette, m_pPalette + m_paletteSize);
	}

	//____ _sendPixels() _________________________________________________________

	void RemoteBitmap::_sendPixels(StreamEncoder* pEncoder, RectI rect, const uint8_t * pSource, int pitch)
	{
		//TODO: Avoid copy if pitch matches and no compression.

		int	pixelSize = m_pPixelDescription->bits / 8;
		int dataSize = rect.w * rect.h * pixelSize;

		int allocSize = dataSize;		// Increase if needed by future compression

		auto pBuffer = (uint8_t *) Base::memStackAlloc(allocSize);

		auto pLine = pSource;
		auto pDest = pBuffer;
		int lineLength = rect.w * pixelSize;

		for( int y = 0 ; y < rect.h ; y++ )
		{
			memcpy( pDest, pLine, lineLength );
			pLine += pitch;
			pDest += lineLength;
		}

		*pEncoder << GfxStream::Header{ GfxStream::ChunkId::BitmapUpdate2, 0, 10 + 16 };
		*pEncoder << CanvasRef::None;
		*pEncoder << uint8_t(0);
		*pEncoder << m_inStreamId;
		*pEncoder << (uint16_t) 1;
		*pEncoder << m_pixelFormat;				// Layout of the pixels that follow.
		*pEncoder << m_colorSpace;
		*pEncoder << (uint8_t) m_bBigEndian;
		*pEncoder << rect;

		StreamBackend::_compressSplitAndEncode(pEncoder, GfxStream::ChunkId::BitmapPixels, m_inStreamId, nullptr, pBuffer, pBuffer + dataSize);

		Base::memStackFree(allocSize);
	}

	//____ _sendDeleteBitmap() _______________________________________________

	void RemoteBitmap::_sendDeleteBitmap()
	{
		*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::DeleteBitmap, 0, 2 };
		*m_pEncoder << m_inStreamId;

	}

} // namespace wax
