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

#include <wg_surfacestreamer.h>
#include <wg_streambackend.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo SurfaceStreamer::TYPEINFO = { "SurfaceStreamer", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	SurfaceStreamer_p SurfaceStreamer::create(const Blueprint& blueprint)
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

		return new SurfaceStreamer(blueprint);
	}

	//____ constructor _____________________________________________________________

	SurfaceStreamer::SurfaceStreamer(const Blueprint& blueprint)
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

	SurfaceStreamer::~SurfaceStreamer()
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

	const TypeInfo& SurfaceStreamer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _sendCreateSurface() _______________________________________________

	void SurfaceStreamer::_sendCreateSurface()
	{
		StreamEncoder& encoder = * m_pEncoder;

		int paletteSize = m_pSurface->paletteSize();

		uint16_t blockSize = 38 + paletteSize * sizeof(Color8);

		encoder << GfxStream::Header{ GfxStream::ChunkId::CreateSurface, 0, blockSize };
		encoder << m_surfaceId;
		encoder << m_pSurface->canBeCanvas();
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

	void SurfaceStreamer::_sendPixels(int nRects, const RectI* pRects)
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

		// Copy pixels from rectangels to continous buffer memory.

		auto pDest = pBuffer;

		for( int i = 0 ; i < nRects ; i++ )
		{
			auto pLine = pixelBuffer.pixels + (pRects[i].y - pixelBuffer.rect.y) * pixelBuffer.pitch +
							((pRects[i].x - pixelBuffer.rect.x) * pixelDescription.bits)/8;

			int lineLength = (pixelBuffer.rect.w * pixelDescription.bits) / 8;

			for( int y = 0 ; y < pixelBuffer.rect.h ; y++ )
			{
				memcpy( pDest, pLine, lineLength );
				pLine += pixelBuffer.pitch;
				pDest += lineLength;
			}
		}

		// Stream SurfaceUpdate2 message and pixels.

		StreamEncoder& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::SurfaceUpdate2, 0, 6 + nRects*16 };
		encoder << m_canvasRef;
		encoder << uint8_t(0);
		encoder << m_surfaceId;
		encoder << (uint16_t) nRects;

		for( int i = 0 ; i < nRects ; i++ )
			encoder << pRects[i];

		StreamBackend::_splitAndEncode(m_pEncoder, GfxStream::ChunkId::SurfacePixels, Compression::None, pBuffer, pBuffer + allocSize, 1);

		// Cleanup

		GfxBase::memStackFree(allocSize);
		m_pSurface->freePixelBuffer(pixelBuffer);
	}


} // namespace wg
