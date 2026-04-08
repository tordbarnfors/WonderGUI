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
#include <wg_streamplayer.h>
#include <wg_gfxbase.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamPlayer::TYPEINFO = { "StreamPlayer", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamPlayer_p StreamPlayer::create(GfxBackend * pBackend, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory )
	{
		return new StreamPlayer(pBackend, pSurfaceFactory, pEdgemapFactory);
	}

	//____ constructor _____________________________________________________________

	StreamPlayer::StreamPlayer(GfxBackend * pBackend, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory) : input(this)
	{
		m_pBackend = pBackend;
		m_pSurfaceFactory = pSurfaceFactory;
		m_pEdgemapFactory = pEdgemapFactory;

		m_pDecoder = StreamDecoder::create();
	}

	//____ Destructor _________________________________________________________

	StreamPlayer::~StreamPlayer()
	{
		reset();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamPlayer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setStoreDirtyRects() _______________________________________________

	void StreamPlayer::setStoreDirtyRects(bool bStore)
	{
		if (bStore == m_bStoreDirtyRects)
			return;

		m_bStoreDirtyRects = bStore;
		if (!bStore)
			clearDirtyRects();
	}

	//____ setMaxDirtyRects() ____________________________________________________

	void StreamPlayer::setMaxDirtyRects(int max)
	{
		m_maxDirtyRects = max;
	}

	//____ dirtyRects() __________________________________________________________

	std::tuple<int, const RectSPX*> StreamPlayer::dirtyRects(CanvasRef canvas)
	{
		const PatchesSPX& patches = m_dirtyRects[(int)canvas];

		int size = patches.size();
		const RectSPX* pRects = patches.begin();
		return std::make_tuple(size, pRects);
	}

	//____ reset() _______________________________________________________________

	void StreamPlayer::reset()
	{
		m_vObjects.clear();

		m_surfaceDataBuffers.clear();
		m_edgemapDataBuffers.clear();
	}

	//____ clearDirtyRects() _____________________________________________________

	void StreamPlayer::clearDirtyRects()
	{
		for (int i = 0; i < CanvasRef_size; i++)
			m_dirtyRects[i].clear();
	}

	//____ setCanvasInfoCallback() ________________________________________________

	void StreamPlayer::setCanvasInfoCallback( const std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)>& callback )
	{
		m_canvasInfoCallback = callback;
	}

	//____ setFenceCallback() ____________________________________________________

	void StreamPlayer::setFenceCallback( const std::function<void(uint16_t fenceID, uint32_t fenceValue)>& callback )
	{
		m_fenceCallback = callback;
	}

	//____ _processStreamChunks() _____________________________________________

	void StreamPlayer::_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd)
	{
		m_pDecoder->setInput(pBegin, pEnd);

		while (_playChunk() == true);
	}

	//____ _playChunk() ____________________________________________________________

	bool StreamPlayer::_playChunk()
	{
		GfxStream::Header header;

		auto& decoder = * m_pDecoder;

		decoder >> header;

		switch (header.type)
		{

		case GfxStream::ChunkId::OutOfData:
			return false;

		case GfxStream::ChunkId::ProtocolVersion:
		{
			uint16_t	version;
			decoder >> version;

			m_streamMajorVersion = version/256;
			m_streamMinorVersion = version & 0xFF;
			break;
		}

		case GfxStream::ChunkId::CanvasList:
		{
			uint16_t nbCanvases;

			decoder >> nbCanvases;

			CanvasInfo	canvas[CanvasRef_size];

			for( int i = 0 ; i < nbCanvases ; i++ )
			{
				uint16_t 	ref;
				SizeSPX		size;
				uint16_t	scale;
				PixelFormat	format;

				decoder >> ref;
				decoder >> size;
				decoder >> scale;
				decoder >> format;

				canvas[i].ref = (CanvasRef) ref;
				canvas[i].size = size;
				canvas[i].scale = scale;
				canvas[i].format = format;
			}

			if( m_canvasInfoCallback )
				m_canvasInfoCallback(canvas, canvas+nbCanvases);

			break;
		}

		case GfxStream::ChunkId::Fence:
		{
			uint16_t	fenceID;
			uint32_t	fenceValue;

			decoder >> fenceID;
			decoder >> fenceValue;

			if( m_fenceCallback )
				m_fenceCallback(fenceID, fenceValue);

			break;
		}

        case GfxStream::ChunkId::BeginRender:
            m_pBackend->beginRender();
            break;

        case GfxStream::ChunkId::EndRender:
            m_pBackend->endRender();
            break;

		case GfxStream::ChunkId::BeginSession:
		{

			// Temporary storage for variables that is stored in smaller uints than in SessionInfo

			uint16_t	objectId;
			CanvasRef	canvasRef;
			uint8_t		dummy;
			uint16_t	nUpdateRects;

			uint16_t	nSetCanvas;
			uint16_t	nStateChanges;
			uint16_t	nLines;
			uint16_t	nFill;
			uint16_t	nBlit;
			uint16_t	nBlur;
			uint16_t	nEdgemapDraws;
			uint16_t	nTransforms;
			uint16_t	nObjects;


			decoder >> objectId;
			decoder >> canvasRef;
			decoder >> dummy;

			decoder >> nUpdateRects;

			decoder >> nSetCanvas;
			decoder >> nStateChanges;
			decoder >> nLines;
			decoder >> nFill;
			decoder >> nBlit;
			decoder >> nBlur;
			decoder >> nEdgemapDraws;

			decoder >> m_sessionInfo.nLineCoords;
			decoder >> m_sessionInfo.nLineClipRects;
			decoder >> m_sessionInfo.nRects;
			decoder >> m_sessionInfo.nColors;
			decoder >> nTransforms;
			decoder >> nObjects;

			m_sessionInfo.nSetCanvas = nSetCanvas;
			m_sessionInfo.nStateChanges = nStateChanges;
			m_sessionInfo.nLines = nLines;
			m_sessionInfo.nFill = nFill;
			m_sessionInfo.nBlit = nBlit;
			m_sessionInfo.nBlur = nBlur;
			m_sessionInfo.nEdgemapDraws = nEdgemapDraws;
			m_sessionInfo.nTransforms = nTransforms;
			m_sessionInfo.nObjects = nObjects;


			m_baseCanvasRef = canvasRef;
			m_baseCanvasSurface = objectId > 0 ? static_cast<Surface*>(m_vObjects[objectId].rawPtr()) : nullptr;


			if (nUpdateRects == 0)
			{
				SizeSPX canvasSize = m_baseCanvasSurface ? m_baseCanvasSurface->pixelSize()*64 : m_pBackend->canvasInfo(canvasRef)->size;

				m_updateRectsDataBuffer.reset(sizeof(RectSPX));

				* ((RectSPX*)m_updateRectsDataBuffer.pBuffer) =  RectSPX(0, 0, canvasSize.w, canvasSize.h);
				m_updateRectsDataBuffer.size = sizeof(RectSPX);

				if (m_bStoreDirtyRects && canvasRef != CanvasRef::None)
					m_dirtyRects[(int)canvasRef].add(canvasSize);

				m_pBackend->beginSession(m_baseCanvasRef, m_baseCanvasSurface, 0, nullptr, &m_sessionInfo);
			}
			break;
		}

		case GfxStream::ChunkId::EndSession:
		{
			m_pBackend->endSession();

			m_baseCanvasRef = CanvasRef::None;
			m_baseCanvasSurface = nullptr;
			break;
		}

		case GfxStream::ChunkId::SetCanvas:
		{
			uint16_t	objectId;
			CanvasRef	canvasRef;
			uint8_t		dummy;

			decoder >> objectId;
			decoder >> canvasRef;
			decoder >> dummy;

			if (canvasRef != CanvasRef::None)
			{
				m_pBackend->setCanvas(canvasRef);

				if (m_bStoreDirtyRects)
				{
					RectSPX * p = (RectSPX*) m_updateRectsDataBuffer.pBuffer;
					RectSPX * pEnd = (RectSPX*) m_updateRectsDataBuffer.pBuffer + m_updateRectsDataBuffer.size;

					if (m_dirtyRects[(int)canvasRef].isEmpty())
					{
						while( p < pEnd )
							m_dirtyRects[(int)canvasRef].push( * p++ );
					}
					else
					{
						while( p < pEnd )
							m_dirtyRects[(int)canvasRef].add( * p++ );
					}
				}
			}
			else
				m_pBackend->setCanvas( static_cast<Surface*>(m_vObjects[objectId].rawPtr()) );

			break;
		}

		case GfxStream::ChunkId::Objects:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			_loadIntoDataBuffer(dataInfo, m_objectsDataBuffer, header.size - dataInfo.encodedSize);

			if( m_objectsDataBuffer.size > 0 )
			{
				int nEntries = m_objectsDataBuffer.size / sizeof(uint16_t);

				m_vActionObjects.resize(nEntries);
				auto it = m_vActionObjects.begin();

				auto pSrc = (uint16_t*) m_objectsDataBuffer.pBuffer;
				auto pEnd = pSrc + nEntries;

				while(pSrc < pEnd)
				{
					uint16_t id = * pSrc++;
					* it++ = m_vObjects[id].rawPtr();
				}

				m_pBackend->setObjects(m_vActionObjects.data(), m_vActionObjects.data() + m_vActionObjects.size() );
			}

			m_pDecoder->align();
			break;
		}

		case GfxStream::ChunkId::Rects:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			_loadIntoDataBuffer(dataInfo, m_rectsDataBuffer, header.size - dataInfo.encodedSize);

			if (m_rectsDataBuffer.size > 0)
				m_pBackend->setRects((RectSPX*) m_rectsDataBuffer.pBuffer, (RectSPX*) (m_rectsDataBuffer.pBuffer + m_rectsDataBuffer.size) );

			break;
		}

		case GfxStream::ChunkId::Colors:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			_loadIntoDataBuffer(dataInfo, m_colorsDataBuffer, header.size - dataInfo.encodedSize);

			if (m_colorsDataBuffer.size > 0)
				m_pBackend->setColors((HiColor*)m_colorsDataBuffer.pBuffer, (HiColor*)(m_colorsDataBuffer.pBuffer + m_colorsDataBuffer.size) );

			break;
		}

		case GfxStream::ChunkId::Transforms:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			_loadIntoDataBuffer(dataInfo, m_transformsDataBuffer, header.size - dataInfo.encodedSize);

			if (m_transformsDataBuffer.size > 0)
				m_pBackend->setTransforms((Transform*)m_transformsDataBuffer.pBuffer, (Transform*)(m_transformsDataBuffer.pBuffer + m_transformsDataBuffer.size) );

			break;
		}


		case GfxStream::ChunkId::Commands:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			_loadIntoDataBuffer(dataInfo, m_commandsDataBuffer, header.size - dataInfo.encodedSize);

			if (m_commandsDataBuffer.size > 0)
			{
				int commandQueueVersion = m_streamMajorVersion < 3 ? 1 : 2;

				m_pBackend->processCommands( (uint16_t*) m_commandsDataBuffer.pBuffer, (uint16_t*) (m_commandsDataBuffer.pBuffer + m_commandsDataBuffer.size), commandQueueVersion );
			}

			break;
		}
		
		case GfxStream::ChunkId::UpdateRects:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			_loadIntoDataBuffer(dataInfo, m_updateRectsDataBuffer, header.size - dataInfo.encodedSize);

			if( m_updateRectsDataBuffer.size > 0 )
			{
				int nRects = m_updateRectsDataBuffer.size / sizeof(RectSPX);
				if (m_bStoreDirtyRects)
				{
					RectSPX * pRects = (RectSPX*) m_updateRectsDataBuffer.pBuffer;
					RectSPX * pEnd = pRects + nRects;

					if (m_dirtyRects[(int)m_baseCanvasRef].isEmpty())
					{
						while( pRects < pEnd )
							m_dirtyRects[(int)m_baseCanvasRef].push(*pRects++);
					}
					else
					{
						while( pRects < pEnd )
							m_dirtyRects[(int)m_baseCanvasRef].add(*pRects++);
					}
				}

				m_pBackend->beginSession(m_baseCanvasRef, m_baseCanvasSurface, nRects,(RectSPX*)m_updateRectsDataBuffer.pBuffer, &m_sessionInfo);
			}


			break;
		}


		case GfxStream::ChunkId::CreateSurface:
		{
			uint16_t	objectId;
			Surface::Blueprint	bp;

			decoder >> objectId;
			decoder >> bp.canvas;
			decoder >> bp.dynamic;
			decoder >> bp.format;
			decoder >> bp.identity;
			decoder >> bp.mipmap;
			decoder >> bp.sampleMethod;
			decoder >> bp.scale;
			decoder >> bp.size;
			decoder >> bp.tiling;
			decoder >> bp.paletteCapacity;
			decoder >> bp.paletteSize;

			bp.buffered = false;
			bp.palette = nullptr;
			
			if (bp.paletteSize > 0)
			{
				auto pPalette = (Color8*) GfxBase::memStackAlloc(bp.paletteSize*4);
				decoder >> GfxStream::ReadBytes{ bp.paletteSize*4, pPalette };
				bp.palette = pPalette;
			}

			if (m_vObjects.size() <= objectId)
				m_vObjects.resize(objectId + 16, nullptr);
			else if( m_vObjects[objectId] != nullptr )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "CreateSurface with objectId that already is in use. The old object will be replaced.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}
			
			m_vObjects[objectId] = m_pSurfaceFactory->createSurface(bp);

			if (bp.palette)
				GfxBase::memStackFree(bp.paletteSize*4);

			break;
		}

		case GfxStream::ChunkId::SurfaceUpdate:
		{
			uint16_t	objectId;
			RectI		rect;

			decoder >> objectId;
			decoder >> rect;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "SurfaceUpdate with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			auto& buffer = m_surfaceDataBuffers.emplace_back(wg_static_cast<Surface_p>(m_vObjects[objectId]));

			buffer.rects.resize(1);
			buffer.rects[0] = rect;

			m_updateObject = objectId;
			break;
		}

		case GfxStream::ChunkId::SurfaceUpdate2:
		{
			CanvasRef	canvasRef;
			uint8_t		dummy;
			uint16_t	surfaceId;
			uint16_t	nRects;

			decoder >> canvasRef;
			decoder >> dummy;
			decoder >> surfaceId;
			decoder >> nRects;


			Surface_p pSurf = (canvasRef != CanvasRef::None ) ? m_pBackend->canvasInfo(canvasRef)->pSurface : wg_static_cast<Surface_p>(m_vObjects[surfaceId]);

			auto& buffer = m_surfaceDataBuffers.emplace_back(pSurf);

			buffer.rects.resize(nRects);
			for( int i = 0 ; i < nRects ; i++ )
				decoder >> buffer.rects[i];

			m_updateObject = surfaceId;
			break;
		}


		case GfxStream::ChunkId::SurfacePixels:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			int objectId = dataInfo.objectId > 0 ? dataInfo.objectId : m_updateObject;
			auto pSurface = wg_static_cast<Surface_p>(m_vObjects[objectId]);

			auto it = m_surfaceDataBuffers.begin();
			while( it->pSurface != pSurface )
				it++;

			_loadIntoDataBuffer( dataInfo, it->buffer, header.size - dataInfo.encodedSize);

			if(it->buffer.size > 0)
			{
				// Get bounds for rect and alloc pixelBuffer

				RectI bounds = it->rects[0];
				for( size_t i = 1 ; i < it->rects.size() ; i++ )
					bounds.growToContain(it->rects[i]);

				auto pixelBuffer = pSurface->allocPixelBuffer(bounds);
				int pixelBits = pSurface->pixelBits();

				// Go through rects and copy pixels

				uint8_t * pSource = it->buffer.pBuffer;
				for( RectI& rect : it->rects )
				{
					uint8_t * pDest = pixelBuffer.pixels + (rect.y - pixelBuffer.rect.y) * pixelBuffer.pitch + (rect.x - pixelBuffer.rect.x) * pixelBits/8;
					int span = rect.w * pixelBits/8;

					for( int i = 0 ; i < rect.h ; i++ )
					{
						memcpy(pDest, pSource, span );
						pSource += span;
						pDest += pixelBuffer.pitch;
					}
				}

				// Clean up

				pSurface->pushPixels(pixelBuffer);
				pSurface->freePixelBuffer(pixelBuffer);

				m_surfaceDataBuffers.erase(it);
			}

			break;
		}

		case GfxStream::ChunkId::FillSurface:
		{
			uint16_t	objectId;
			RectI		region;
			HiColor		col;

			decoder >> objectId;
			decoder >> region;
			decoder >> col;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "FillSurface with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			static_cast<Surface*>(m_vObjects[objectId].rawPtr())->fill(region, col);
			break;
		}
/*
		case GfxStream::ChunkId::CopySurface:
		{
			uint16_t	destSurfaceId;
			uint16_t	sourceSurfaceId;
			RectI		sourceRect;
			CoordI		dest;

			decoder >> destSurfaceId;
			decoder >> sourceSurfaceId;
			decoder >> sourceRect;
			decoder >> dest;

			Surface * pDest	  = m_vSurfaces[destSurfaceId];
			Surface * pSource = m_vSurfaces[sourceSurfaceId];

			pDest->copyFrom(pSource, sourceRect, dest);
			break;
		}
*/
		case GfxStream::ChunkId::DeleteSurface:
		{
			uint16_t	objectId;

			decoder >> objectId;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr || !m_vObjects[objectId]->isInstanceOf(Surface::TYPEINFO) )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "DeleteSurface with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_vObjects[objectId] = nullptr;
			break;
		}


		case GfxStream::ChunkId::CreateEdgemap:
		{
			uint16_t	objectId;
			SizeI		size;
			uint16_t	nbSegments;
			uint16_t	paletteType;

			decoder >> objectId;
			decoder >> size;
			decoder >> nbSegments;
			decoder >> paletteType;

			if (m_vObjects.size() <= objectId)
				m_vObjects.resize(objectId + 16, nullptr);
			else if( m_vObjects[objectId] != nullptr )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "CreateEdgemap with objectId that already is in use. The old object will be replaced.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}

			Edgemap::Blueprint		bp;
			
			bp.size = size;
			bp.segments = nbSegments;
			bp.paletteType = (EdgemapPalette) paletteType;
			
			m_vObjects[objectId] = m_pEdgemapFactory->createEdgemap(bp);
			break;
		}

		case GfxStream::ChunkId::SetEdgemapRenderSegments:
		{
			uint16_t	objectId;
			uint16_t	nbSegments;

			decoder >> objectId;
			decoder >> nbSegments;

			static_cast<Edgemap*>(m_vObjects[objectId].rawPtr())->setRenderSegments(nbSegments);

			break;
		}

		case GfxStream::ChunkId::SetEdgemapColors:
		{
			uint16_t	objectId;
			int			begin;
			int			end;
			
			decoder >> objectId;
			decoder >> begin;
			decoder >> end;

			int nColors = end - begin;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "SetEdgemapColors with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);

				m_pDecoder->skip(sizeof(HiColor)*nColors);
				break;
			}

			int memAllocated = sizeof(HiColor)*nColors;
			auto * pColors = (HiColor*) GfxBase::memStackAlloc(memAllocated);
			
			for( int i = 0 ; i < nColors ; i++ )
				decoder >> pColors[i];

			static_cast<Edgemap*>(m_vObjects[objectId].rawPtr())->importPaletteEntries(begin, end, pColors);

			GfxBase::memStackFree(memAllocated);
			break;
		}
				
		case GfxStream::ChunkId::EdgemapUpdate:
		{
			uint16_t	objectId;
			uint8_t		edgeBegin;
			uint8_t		edgeEnd;
			uint16_t	sampleBegin;
			uint16_t	sampleEnd;

			decoder >> objectId;
			decoder >> edgeBegin;
			decoder >> edgeEnd;
			decoder >> sampleBegin;
			decoder >> sampleEnd;

			int nSamples = sampleEnd - sampleBegin;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "EdgemapUpdate with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
//				break;
			}

			m_edgemapDataBuffers.emplace_back(wg_static_cast<Edgemap_p>(m_vObjects[objectId]), edgeBegin, edgeEnd, sampleBegin, sampleEnd);

			m_updateObject = objectId;
			break;
		}
				
		case GfxStream::ChunkId::EdgemapSamples:
		{
			GfxStream::DataInfo dataInfo;
			decoder >> dataInfo;

			int objectId = dataInfo.objectId > 0 ? dataInfo.objectId : m_updateObject;
			auto pEdgemap = wg_static_cast<Edgemap_p>(m_vObjects[objectId]);

			auto it = m_edgemapDataBuffers.begin();
			while( it->pEdgemap != pEdgemap )
				it++;

			_loadIntoDataBuffer( dataInfo, it->buffer, header.size - dataInfo.encodedSize);

			if (it->buffer.size > 0)
			{
				pEdgemap->importSamples(SampleOrigo::Top, (spx*) it->buffer.pBuffer, it->edgeBegin, it->edgeEnd, it->sampleBegin, it->sampleEnd, 1, (it->edgeEnd - it->edgeBegin));
				m_edgemapDataBuffers.erase(it);
			}

			break;
		}

		case GfxStream::ChunkId::DeleteEdgemap:
		{
			uint16_t	objectId;

			decoder >> objectId;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr || !m_vObjects[objectId]->isInstanceOf(Edgemap::TYPEINFO) )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "DeleteEdgemap with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_vObjects[objectId] = nullptr;
			break;
		}

		default:
			// We don't know how to handle this, so let's just skip it

			m_pDecoder->skip(header.size);
			break;
		}

		return true;
	}

	//____ _loadIntoDataBuffer() ________________________________________________

	bool StreamPlayer::_loadIntoDataBuffer( GfxStream::DataInfo& dataInfo, DataBuffer& buffer, int dataSize )
	{
		auto& decoder = * m_pDecoder;

		if( dataInfo.bFirstChunk )
		{
			if( buffer.capacity < dataInfo.bufferSize )
			{
				delete [] buffer.pBuffer;
				buffer.pBuffer = new uint8_t[dataInfo.bufferSize];
				buffer.capacity = dataInfo.bufferSize;
			}
			buffer.size = 0;
		}

		buffer.size = GfxStream::loadDecompressData(dataInfo, buffer.pBuffer, decoder.readPtr(), dataSize);

		decoder.skip(dataSize + int(dataInfo.bPadded));
		return true;
	}

} //namespace wg
