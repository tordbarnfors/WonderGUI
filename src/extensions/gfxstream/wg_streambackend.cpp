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

#include <cstring>

#include <wg_streambackend.h>
#include <wg_streamsurface.h>
#include <wg_streamedgemap.h>
#include <wg_streamsurfacefactory.h>
#include <wg_streamedgemapfactory.h>
#include <wg_gfxbase.h>

#include <assert.h>

namespace wg
{

	const TypeInfo StreamBackend::TYPEINFO = { "StreamBackend", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamBackend_p StreamBackend::create(StreamEncoder * pEncoder, int maxEdges )
	{
		return new StreamBackend(pEncoder, maxEdges);
	}

	//____ constructor _____________________________________________________________

	StreamBackend::StreamBackend(StreamEncoder * pEncoder, int maxEdges)
	{
		m_pEncoder = pEncoder;
		m_maxEdges = maxEdges;

		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::ProtocolVersion, GfxStream::SpxFormat::Int32_dec, 2 };
		(*m_pEncoder) << (uint16_t) 0x0200;

	}

	//____ Destructor _________________________________________________________

	StreamBackend::~StreamBackend()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _________________________________________________________

	void StreamBackend::beginRender()
	{
		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::BE_BeginRender, GfxStream::SpxFormat::Int32_dec, 0 };
	}

	//____ endRender() ___________________________________________________________

	void StreamBackend::endRender()
	{
		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::BE_EndRender, GfxStream::SpxFormat::Int32_dec, 0 };

		m_pEncoder->flush();
	}

	//____ beginSession() ________________________________________________________

	void StreamBackend::beginSession( const SessionInfo * pSession )
	{
		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::BE_BeginSession, GfxStream::SpxFormat::Int32_dec, 44 };

		(*m_pEncoder) << (uint16_t) pSession->nCanvases;
		(*m_pEncoder) << 			pSession->canvasSize;
		(*m_pEncoder) << (uint16_t) pSession->nUpdateRects;

		(*m_pEncoder) << (uint16_t) pSession->nStateChanges;
		(*m_pEncoder) << (uint16_t) pSession->nLines;
		(*m_pEncoder) << (uint16_t) pSession->nFill;
		(*m_pEncoder) << (uint16_t) pSession->nBlit;
		(*m_pEncoder) << (uint16_t) pSession->nBlur;
		(*m_pEncoder) << (uint16_t) pSession->nEdgemapDraws;
		(*m_pEncoder) << (int32_t) pSession->nLineCoords;

		(*m_pEncoder) << (int32_t) pSession->nLineClipRects;
		(*m_pEncoder) << (int32_t) pSession->nRects;
		(*m_pEncoder) << (int32_t) pSession->nColors;
		(*m_pEncoder) << (uint16_t) pSession->nTransforms;
		(*m_pEncoder) << (uint16_t) pSession->nObjects;

		if( pSession->nUpdateRects > 0 )
			_splitAndEncode( GfxChunkId::BE_UpdateRects, pSession->pUpdateRects, pSession->pUpdateRects + pSession->nUpdateRects, sizeof(RectI) );
	}

	//____ endSession() __________________________________________________________

	void StreamBackend::endSession()
	{
		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::BE_EndSession, GfxStream::SpxFormat::Int32_dec, 0 };
	}

	//____ setCanvas() ___________________________________________________________

	void StreamBackend::setCanvas( Surface * pSurface )
	{
		uint16_t size = 2 + 2;

		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::BE_SetCanvas, {}, size };
		(*m_pEncoder) << (pSurface ? static_cast<StreamSurface*>(pSurface)->inStreamId() : (uint16_t) 0);
		(*m_pEncoder) << CanvasRef::None;
		(*m_pEncoder) << (uint8_t) 0;				// Filler to align.
	}

	void StreamBackend::setCanvas( CanvasRef ref )
	{
		uint16_t size = 2 + 1 + 1;

		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::BE_SetCanvas, {}, size };
		(*m_pEncoder) << (uint16_t) 0;				// Surface ID
		(*m_pEncoder) << ref;
		(*m_pEncoder) << (uint8_t) 0;				// Filler to align.
	}

	//____ setObjects() __________________________________________________________

	void StreamBackend::setObjects(Object** pBeg, Object** pEnd)
	{
		m_objects.resize(pEnd-pBeg);

		auto it = m_objects.begin();

		for( Object ** p = pBeg ; p < pEnd ; p++ )
		{
			Object * pObj = * p;

			if( pObj->typeInfo() == StreamSurface::TYPEINFO )
				* it++ = static_cast<StreamSurface*>(pObj)->inStreamId();
			else if( pObj->typeInfo() == StreamEdgemap::TYPEINFO )
				* it++ = static_cast<StreamEdgemap*>(pObj)->inStreamId();
			else
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Unknown object type in object list.", this, &StreamBackend::TYPEINFO, __func__, __FILE__, __LINE__);				
				return;
			}
		}

		_splitAndEncode( GfxChunkId::BE_Objects, m_objects.data(), m_objects.data() + m_objects.size(), sizeof(uint16_t) );
	}

	//____ setRects() ___________________________________________________________

	void StreamBackend::setRects(RectSPX* pBeg, RectSPX* pEnd)
	{
		_splitAndEncode( GfxChunkId::BE_Rects, pBeg, pEnd, sizeof(RectSPX) );
	}

	//____ setColors() ___________________________________________________________

	void StreamBackend::setColors(HiColor* pBeg, HiColor* pEnd)
	{
		_splitAndEncode( GfxChunkId::BE_Colors, pBeg, pEnd, sizeof(HiColor) );
	}

	//____ setTransforms() _______________________________________________________

	void StreamBackend::setTransforms(Transform * pBeg, Transform * pEnd)
	{
		_splitAndEncode( GfxChunkId::BE_Transforms, pBeg, pEnd, sizeof(Transform) );
	}

	//____ processCommands() _____________________________________________________

	void StreamBackend::processCommands( int32_t* pBeg, int32_t * pEnd)
	{
		_splitAndEncode( GfxChunkId::BE_Commands, pBeg, pEnd, sizeof(int32_t) );
	}

	//____ defineCanvas() ________________________________________________________

	bool StreamBackend::defineCanvas( CanvasRef ref, StreamSurface * pSurface )
	{
		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](CanvasInfo& entry) { return (ref == entry.ref); });

		if (it == m_definedCanvases.end())
		{
			if (pSurface)
				m_definedCanvases.push_back(CanvasInfo(ref, pSurface, pSurface->pixelSize() * 64, pSurface->pixelFormat(), pSurface->scale()));
		}
		else
		{
			if (pSurface)
			{
				it->pSurface = pSurface;
				it->size = pSurface->pixelSize() * 64;
				it->scale = pSurface->scale();
			}
			else
			{
				m_definedCanvases.erase(it);
			}
		}

		return true;
	}

	bool StreamBackend::defineCanvas( CanvasRef ref, const SizeI& pixelSize, PixelFormat pixelFormat, int scale )
	{
		auto it = std::find_if( m_definedCanvases.begin(), m_definedCanvases.end(), [ref] (CanvasInfo& entry) { return (ref == entry.ref); } );

		if( it == m_definedCanvases.end() )
		{
			if( !pixelSize.isEmpty() )
				m_definedCanvases.push_back( CanvasInfo( ref, nullptr, pixelSize * 64, pixelFormat, scale ) );
		}
		else
		{
			if( !pixelSize.isEmpty() )
			{
				it->pSurface = nullptr;
				it->size = pixelSize * 64;
				it->scale = scale;
			}
			else
			{
				m_definedCanvases.erase(it);
			}
		}

		return true;
	}

	//____ canvasInfo() __________________________________________________________

	const CanvasInfo * StreamBackend::canvasInfo(CanvasRef ref) const
	{
		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](const CanvasInfo& entry) { return (ref == entry.ref); });

		if (it != m_definedCanvases.end())
			return &(*it);

		return nullptr;
	}

	//____ encodeCanvasList() ______________________________________________________

	void StreamBackend::encodeCanvasList()
	{
		(*m_pEncoder) << GfxStream::Header{ GfxChunkId::CanvasList, GfxStream::SpxFormat::Int32_dec, (uint16_t)(m_definedCanvases.size()*14)+2 };

		(*m_pEncoder) << (uint16_t) m_definedCanvases.size();

		for( auto& canvas : m_definedCanvases )
		{
			(*m_pEncoder) << (uint16_t) canvas.ref;
			(*m_pEncoder) << canvas.size;
			(*m_pEncoder) << (uint16_t) canvas.scale;
			(*m_pEncoder) << canvas.format;
		}
	}

	//____ surfaceFactory() ______________________________________________________

	SurfaceFactory_p StreamBackend::surfaceFactory()
	{
		if( !m_pSurfaceFactory )
			m_pSurfaceFactory = StreamSurfaceFactory::create(m_pEncoder);

		return m_pSurfaceFactory;
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p StreamBackend::edgemapFactory()
	{
		if( !m_pEdgemapFactory )
			m_pEdgemapFactory = StreamEdgemapFactory::create(m_pEncoder);

		return m_pEdgemapFactory;
	}

	//____ maxEdges() ____________________________________________________________

	int StreamBackend::maxEdges() const
	{
		return m_maxEdges;
	}

	//____ surfaceType() _________________________________________________________

	const TypeInfo& StreamBackend::surfaceType(void) const
	{
		return StreamSurface::TYPEINFO;
	}

	//____ _splitAndEncode() _____________________________________________________

	void StreamBackend::_splitAndEncode( GfxChunkId chunkType, const void * _pBeg, const void * _pEnd, int entrySize )
	{
		// Content
		// int32_t		totalSize in bytes
		// int32_t		chunkOffset in bytes
		// bool			bFirstChunk
		// bool			bLastChunk
		//
		// All measures in bytes

		char * pBeg = (char *) _pBeg;
		char * pEnd = (char *) _pEnd;
		char * p = pBeg;

		int maxBytesInChunk = ((GfxStream::c_maxBlockSize - 4 - 12) / entrySize) * entrySize;

		while( p < pEnd )
		{
			int bytesOfData = std::min(int(pEnd-p),maxBytesInChunk);

			(*m_pEncoder) << GfxStream::Header{ chunkType, GfxStream::SpxFormat::Int32_dec, (uint16_t) bytesOfData + 12 };

			(*m_pEncoder) << (int32_t) (pEnd - pBeg);
			(*m_pEncoder) << (int32_t) (p - pBeg);
//			(*m_pEncoder) << (int32_t) bytesOfData;
			(*m_pEncoder) << (bool) (p == pBeg);
			(*m_pEncoder) << (bool) (p + bytesOfData == pEnd);

			(*m_pEncoder) << GfxStream::WriteBytes{ bytesOfData, (void *) p };

			p += bytesOfData;
		}
	}


} // namespace wg
