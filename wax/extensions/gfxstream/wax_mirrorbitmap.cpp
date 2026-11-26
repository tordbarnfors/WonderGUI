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
#include <wax_mirrorbitmap.h>
#include <wax_streambackend.h>
#include <wax_base.h>

namespace wax
{

	const TypeInfo MirrorBitmap::TYPEINFO = { "MirrorBitmap", &Bitmap::TYPEINFO };


	//____ create() ___________________________________________________________

	MirrorBitmap_p MirrorBitmap::create(const Blueprint& blueprint)
	{
		if (!blueprint.encoder)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "You must provide a StreamEncoder in the blueprint.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		if (!blueprint.bitmap)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "You must provide a Bitmap in the blueprint.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return new MirrorBitmap(blueprint);
	}

	//____ constructor _____________________________________________________________

	MirrorBitmap::MirrorBitmap(const Blueprint& blueprint) : Bitmap( blueprint.bitmap->blueprint(), blueprint.bitmap->pixelFormat(), blueprint.bitmap->sampleMethod() )
	{
		m_pEncoder = blueprint.encoder;
		m_pCompressor = blueprint.compressor;
		m_pBitmap = blueprint.bitmap;
		m_canvasRef = blueprint.canvasRef;

		if( blueprint.finalizer)
		 setFinalizer( blueprint.finalizer );

		m_observerId = m_pBitmap->addObserver([this](int nRects, const RectI* pRects)
		{
			this->_sendPixels(nRects, pRects);
		} );

		if( blueprint.canvasRef == CanvasRef::None )
		{
			m_bitmapId = m_pEncoder->allocObjectId();
			_sendCreateBitmap( blueprint.streamOnCreate ? Color::Undefined : blueprint.fillColor );
		}

		if (blueprint.streamOnCreate )
		{
			RectI	rect(0,0,m_pBitmap->pixelSize());
			_sendPixels(1, &rect);
		}
	}

	//____ Destructor _________________________________________________________

	MirrorBitmap::~MirrorBitmap()
	{
		m_pBitmap->removeObserver(m_observerId);

		if( m_bitmapId != 0 )
		{
			*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::DeleteBitmap, 0, 2 };
			*m_pEncoder << m_bitmapId;

			m_pEncoder->freeObjectId(m_bitmapId);
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MirrorBitmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() _______________________________________________________________

	SizeI MirrorBitmap::maxSize()
	{
		return SizeI(4096, 4096);
	}

	//____ alpha() __________________________________________________________

	int MirrorBitmap::alpha(CoordSPX coord)
	{
		return m_pBitmap->alpha(coord);
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer MirrorBitmap::allocPixelBuffer(const RectI& rect)
	{
		return m_pBitmap->allocPixelBuffer(rect);
	}

	//____ pushPixels() ______________________________________________________

	bool MirrorBitmap::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		return m_pBitmap->pushPixels(buffer, bufferRect);
	}

	//____ pullPixels() ______________________________________________________

	void MirrorBitmap::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		m_pBitmap->pullPixels(buffer, bufferRect, bAutoNotify);
	}
	//____ freePixelBuffer() _________________________________________________

	void MirrorBitmap::freePixelBuffer(const PixelBuffer& buffer)
	{
		m_pBitmap->freePixelBuffer(buffer);
	}

	//____ fill() ____________________________________________________________

	bool MirrorBitmap::fill(Color col)
	{
		return m_pBitmap->fill(col);
	}

	bool MirrorBitmap::fill(const RectI& region, Color col)
	{
		return m_pBitmap->fill(region, col);
	}

	//____ streamAsNew() ______________________________________________________

	bool MirrorBitmap::streamAsNew(StreamEncoder* pEncoder)
	{
		_sendCreateBitmap();
		RectI	rect(0,0,m_pBitmap->pixelSize());
		_sendPixels(1, &rect);
		return true;
	}

	//____ _sendCreateBitmap() _______________________________________________

	// A fill color makes the other end fill its bitmap as ours was when created, which
	// saves sending the pixels. Undefined when the pixels follow.

	void MirrorBitmap::_sendCreateBitmap( Color fillColor )
	{
		StreamEncoder& encoder = * m_pEncoder;

		int paletteSize = m_pBitmap->paletteSize();

		encoder << GfxStream::Header{ GfxStream::ChunkId::CreateBitmap, 0, 48 };
		encoder << m_bitmapId;
		encoder << false;						// CanBeCanvas-flag. We never want the remote end to use a MirrorBitmap as canvas.
		encoder << m_pBitmap->isDynamic();
		encoder << m_pBitmap->pixelFormat();
		encoder << m_pBitmap->colorSpace();
		encoder << (uint8_t) m_pBitmap->isBigEndian();
		encoder << m_pBitmap->identity();
		encoder << m_pBitmap->isMipmapped();
		encoder << m_pBitmap->sampleMethod();
		encoder << m_pBitmap->scale();
		encoder << m_pBitmap->pixelSize();
		encoder << m_pBitmap->isTiling();
		encoder << m_pBitmap->paletteCapacity();
		encoder << paletteSize;
		encoder << fillColor;

		// The palette follows in chunks of its own, since a large one doesn't fit in one.
		// The bitmap is created on the other end once all of it has arrived.

		if (paletteSize > 0 )
			StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::BitmapPalette, m_bitmapId, m_pCompressor,
												   m_pBitmap->palette(), m_pBitmap->palette() + paletteSize);
	}

	//____ _sendPixels() _________________________________________________________

	void MirrorBitmap::_sendPixels(int nRects, const RectI* pRects)
	{
		// Calculate bounds, that is the area we will allocate a pixelBuffer for.

		auto bounds = pRects[0];

		for( int i = 1 ; i < nRects ; i++ )
			bounds.growToContain(pRects[i]);

		// We can't have more dirty rects than will fit in the block.

		int maxRects = (GfxStream::c_maxBlockSize - GfxStream::HeaderSize - 10) / 16;
		if( nRects > maxRects )
		{
			nRects = 1;
			pRects = &bounds;
		}

		// Allocate PixelBuffer and temporary buffer for pixels.

		auto pixelBuffer = m_pBitmap->allocPixelBuffer(bounds);

		auto pixelDescription = Util::pixelFormatToDescription(pixelBuffer.format, pixelBuffer.bigEndian);

		int allocSize = 0;
		for( int i = 0 ; i < nRects ; i++ )
			allocSize += pRects[i].w * pRects[i].h * pixelDescription.bits / 8;

		auto pBuffer = (uint8_t *) Base::memStackAlloc(allocSize);

		// Copy pixels from rectangles to continuous buffer memory.

		auto pDest = pBuffer;

		for (int i = 0; i < nRects; i++)
		{
			auto pLine = pixelBuffer.pixels + (pRects[i].y - pixelBuffer.rect.y) * pixelBuffer.pitch +
				((pRects[i].x - pixelBuffer.rect.x) * pixelDescription.bits) / 8;

			int lineLength = (pixelBuffer.rect.w * pixelDescription.bits) / 8;

			for (int y = 0; y < pixelBuffer.rect.h; y++)
			{
				memcpy(pDest, pLine, lineLength);
				pLine += pixelBuffer.pitch;
				pDest += lineLength;
			}
		}

		// Stream BitmapUpdate2 message and pixels.

		StreamEncoder& encoder = *m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::BitmapUpdate2, 0, 10 + nRects * 16 };
		encoder << m_canvasRef;
		encoder << uint8_t(0);
		encoder << m_bitmapId;
		encoder << (uint16_t)nRects;
		encoder << pixelBuffer.format;				// Layout of the pixels that follow.
		encoder << pixelBuffer.colorSpace;
		encoder << (uint8_t) pixelBuffer.bigEndian;

		for (int i = 0; i < nRects; i++)
			encoder << pRects[i];

		StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::BitmapPixels, m_bitmapId, m_pCompressor, pBuffer, pBuffer + allocSize);

		// Cleanup

		Base::memStackFree(allocSize);
		m_pBitmap->freePixelBuffer(pixelBuffer);
	}


} // namespace wax
