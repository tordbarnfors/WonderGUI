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
#include <wg_mirrorsurface.h>
#include <wg_streambackend.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo MirrorSurface::TYPEINFO = { "MirrorSurface", &Surface::TYPEINFO };


	//____ create() ___________________________________________________________

	MirrorSurface_p MirrorSurface::create(const Blueprint& blueprint)
	{
		if (!blueprint.encoder)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "You must provide a StreamEncoder in the blueprint.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		if (!blueprint.surface)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "You must provide a Surface in the blueprint.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return new MirrorSurface(blueprint);
	}

	//____ constructor _____________________________________________________________

	MirrorSurface::MirrorSurface(const Blueprint& blueprint) : Surface( blueprint.surface->blueprint(), blueprint.surface->pixelFormat(), blueprint.surface->sampleMethod() )
	{
		m_pEncoder = blueprint.encoder;
		m_pSurface = blueprint.surface;
		m_canvasRef = blueprint.canvasRef;

		if( blueprint.finalizer)
		 setFinalizer( blueprint.finalizer );

		m_observerId = m_pSurface->addObserver([this](int nRects, const RectI* pRects)
		{
			this->_sendPixels(nRects, pRects);
		} );

		if( blueprint.canvasRef == CanvasRef::None )
		{
			m_surfaceId = m_pEncoder->allocObjectId();
			_sendCreateSurface();
		}

		if (blueprint.streamOnCreate )
		{
			RectI	rect(0,0,m_pSurface->pixelSize());
			_sendPixels(1, &rect);
		}
	}

	//____ Destructor _________________________________________________________

	MirrorSurface::~MirrorSurface()
	{
		m_pSurface->removeObserver(m_observerId);

		if( m_surfaceId != 0 )
		{
			*m_pEncoder << GfxStream::Header{ GfxStream::ChunkId::DeleteSurface, 0, 2 };
			*m_pEncoder << m_surfaceId;

			m_pEncoder->freeObjectId(m_surfaceId);
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MirrorSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() _______________________________________________________________

	SizeI MirrorSurface::maxSize()
	{
		return SizeI(4096, 4096);
	}

	//____ alpha() __________________________________________________________

	int MirrorSurface::alpha(CoordSPX coord)
	{
		return m_pSurface->alpha(coord);
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer MirrorSurface::allocPixelBuffer(const RectI& rect)
	{
		return m_pSurface->allocPixelBuffer(rect);
	}

	//____ pushPixels() ______________________________________________________

	bool MirrorSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		return m_pSurface->pushPixels(buffer, bufferRect);
	}

	//____ pullPixels() ______________________________________________________

	void MirrorSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		m_pSurface->pullPixels(buffer, bufferRect, bAutoNotify);
	}
	//____ freePixelBuffer() _________________________________________________

	void MirrorSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
		m_pSurface->freePixelBuffer(buffer);
	}

	//____ fill() ____________________________________________________________

	bool MirrorSurface::fill(HiColor col)
	{
		return m_pSurface->fill(col);
	}

	bool MirrorSurface::fill(const RectI& region, HiColor col)
	{
		return m_pSurface->fill(region, col);
	}

	//____ streamAsNew() ______________________________________________________

	bool MirrorSurface::streamAsNew(StreamEncoder* pEncoder)
	{
		_sendCreateSurface();
		RectI	rect(0,0,m_pSurface->pixelSize());
		_sendPixels(1, &rect);
		return true;
	}

	//____ _sendCreateSurface() _______________________________________________

	void MirrorSurface::_sendCreateSurface()
	{
		StreamEncoder& encoder = * m_pEncoder;

		int paletteSize = m_pSurface->paletteSize();

		uint16_t blockSize = 38 + paletteSize * sizeof(Color8);

		encoder << GfxStream::Header{ GfxStream::ChunkId::CreateSurface, 0, blockSize };
		encoder << m_surfaceId;
		encoder << false;						// CanBeCanvas-flag. We never want the remote end to use a MirrorSurface as canvas.
		encoder << m_pSurface->isDynamic();
		encoder << m_pSurface->pixelFormat();
		encoder << m_pSurface->identity();
		encoder << m_pSurface->isMipmapped();
		encoder << m_pSurface->sampleMethod();
		encoder << m_pSurface->scale();
		encoder << m_pSurface->pixelSize();
		encoder << m_pSurface->isTiling();
		encoder << m_pSurface->paletteCapacity();
		encoder << m_pSurface->paletteSize();

		if (paletteSize > 0 )
			encoder << GfxStream::WriteBytes{ paletteSize * int(sizeof(Color8)), m_pSurface->palette() };
	}

	//____ _sendPixels() _________________________________________________________

	void MirrorSurface::_sendPixels(int nRects, const RectI* pRects)
	{
		// Calculate bounds, that is the area we will allocate a pixelBuffer for.

		auto bounds = pRects[0];

		for( int i = 1 ; i < nRects ; i++ )
			bounds.growToContain(pRects[i]);

		// We can't have more dirty rects than will fit in the block.

		int maxRects = (GfxStream::c_maxBlockSize - GfxStream::HeaderSize - 6) / 16;
		if( nRects > maxRects )
		{
			nRects = 1;
			pRects = &bounds;
		}

		// Allocate PixelBuffer and temporary buffer for pixels.

		auto pixelBuffer = m_pSurface->allocPixelBuffer(bounds);

		auto pixelDescription = Util::pixelFormatToDescription(pixelBuffer.format);

		int allocSize = 0;
		for( int i = 0 ; i < nRects ; i++ )
			allocSize += pRects[i].w * pRects[i].h * pixelDescription.bits / 8;

		auto pBuffer = (uint8_t *) GfxBase::memStackAlloc(allocSize);

#ifdef WG_GFXSTREAM_USE_SURFACE_UPDATE2

		//TODO: We can get too many rectangels to fit in the chunk!

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

		// Stream SurfaceUpdate2 message and pixels.

		StreamEncoder& encoder = *m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::SurfaceUpdate2, 0, 6 + nRects * 16 };
		encoder << m_canvasRef;
		encoder << uint8_t(0);
		encoder << m_surfaceId;
		encoder << (uint16_t)nRects;

		for (int i = 0; i < nRects; i++)
			encoder << pRects[i];

		StreamBackend::_splitAndEncode(m_pEncoder, GfxStream::ChunkId::SurfacePixels, Compression::None, pBuffer, pBuffer + allocSize, 1);

#else
		// Send pixels using old SurfaceUpdate method for compatibility with old firmware.
		// NO SUPPORT FOR CANVASREF!

		StreamEncoder& encoder = *m_pEncoder;

		for (int i = 0; i < nRects; i++)
		{
			encoder << GfxStream::Header{ GfxStream::ChunkId::SurfaceUpdate, 0, 18 };
			encoder << m_surfaceId;
			encoder << pRects[i];

			uint8_t * pDest = pBuffer;

			uint8_t * pLine = pixelBuffer.pixels + (pRects[i].y - pixelBuffer.rect.y) * pixelBuffer.pitch +
				((pRects[i].x - pixelBuffer.rect.x) * pixelDescription.bits) / 8;

			int lineLength = (pRects[i].w * pixelDescription.bits) / 8;

			for (int y = 0; y < pixelBuffer.rect.h; y++)
			{
				memcpy(pDest, pLine, lineLength);
				pLine += pixelBuffer.pitch;
				pDest += lineLength;
			}

			StreamBackend::_splitAndEncode(m_pEncoder, GfxStream::ChunkId::SurfacePixels, Compression::None, pBuffer, pDest, 4);
		}
#endif

		// Cleanup

		GfxBase::memStackFree(allocSize);
		m_pSurface->freePixelBuffer(pixelBuffer);
	}


} // namespace wg
