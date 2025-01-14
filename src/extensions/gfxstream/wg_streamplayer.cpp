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

#include <wg_streamplayer.h>
#include <wg_gfxbase.h>
#include <wg_compress.h>
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

		m_clipListBuffer.pRects = new RectI[c_clipListBufferSize];
		m_clipListBuffer.nRects = 0;
		m_clipListBuffer.capacity = c_clipListBufferSize;
	}

	//____ Destructor _________________________________________________________

	StreamPlayer::~StreamPlayer()
	{
		delete[] m_clipListBuffer.pRects;
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

		*m_pDecoder >> header;

		switch (header.type)
		{

		case GfxStream::ChunkId::OutOfData:
			return false;

		case GfxStream::ChunkId::ProtocolVersion:
		{
			uint16_t	version;
			*m_pDecoder >> version;

			//TODO: Something if version isn't supported.

			break;
		}

		case GfxStream::ChunkId::CanvasList:
		{
			uint16_t nbCanvases;

			*m_pDecoder >> nbCanvases;

			CanvasInfo	canvas[CanvasRef_size];

			for( int i = 0 ; i < nbCanvases ; i++ )
			{
				uint16_t 	ref;
				SizeSPX		size;
				uint16_t	scale;
				PixelFormat	format;

				*m_pDecoder >> ref;
				*m_pDecoder >> size;
				*m_pDecoder >> scale;
				*m_pDecoder >> format;

				canvas[i].ref = (CanvasRef) ref;
				canvas[i].size = size;
				canvas[i].scale = scale;
				canvas[i].format = format;
			}

			if( m_canvasInfoCallback )
				m_canvasInfoCallback(canvas, canvas+nbCanvases);
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


			*m_pDecoder >> objectId;
			*m_pDecoder >> canvasRef;
			*m_pDecoder >> dummy;

			*m_pDecoder >> nUpdateRects;

			*m_pDecoder >> nSetCanvas;
			*m_pDecoder >> nStateChanges;
			*m_pDecoder >> nLines;
			*m_pDecoder >> nFill;
			*m_pDecoder >> nBlit;
			*m_pDecoder >> nBlur;
			*m_pDecoder >> nEdgemapDraws;

			*m_pDecoder >> m_sessionInfo.nLineCoords;
			*m_pDecoder >> m_sessionInfo.nLineClipRects;
			*m_pDecoder >> m_sessionInfo.nRects;
			*m_pDecoder >> m_sessionInfo.nColors;
			*m_pDecoder >> nTransforms;
			*m_pDecoder >> nObjects;

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

			*m_pDecoder >> objectId;
			*m_pDecoder >> canvasRef;
			*m_pDecoder >> dummy;

			if (canvasRef != CanvasRef::None)
			{
				m_pBackend->setCanvas(canvasRef);

				if (m_bStoreDirtyRects)
				{
					if (m_dirtyRects[(int)canvasRef].isEmpty())
					{
						for (auto& rect : m_vUpdateRects)
							m_dirtyRects[(int)canvasRef].push(rect);
					}
					else
					{
						for (auto& rect : m_vUpdateRects)
							m_dirtyRects[(int)canvasRef].add(rect);
					}
				}
			}
			else
				m_pBackend->setCanvas( static_cast<Surface*>(m_vObjects[objectId].rawPtr()) );

			break;
		}

		case GfxStream::ChunkId::Objects:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression	compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vActionObjects.resize(totalSize / sizeof(uint16_t));

			int nEntries = bytes / sizeof(uint16_t);

			uint16_t * pBuffer = (uint16_t*) GfxBase::memStackAlloc(bytes);
			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pBuffer };

			auto it = m_vActionObjects.begin() + (offset / sizeof(uint16_t));

			for(int i = 0 ; i < nEntries ; i++)
			{
				uint16_t id = pBuffer[i];
				* it++ = m_vObjects[id].rawPtr();
			}

			GfxBase::memStackFree(bytes);

			if (bLastChunk)
				m_pBackend->setObjects(m_vActionObjects.data(), m_vActionObjects.data() + m_vActionObjects.size() );

			m_pDecoder->align();
			break;
		}

		case GfxStream::ChunkId::Rects:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression	compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vRects.resize(totalSize / sizeof(RectSPX));

			char* pDest = ((char*)m_vRects.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->setRects(m_vRects.data(), m_vRects.data() + m_vRects.size());

			m_pDecoder->align();
			break;
		}

		case GfxStream::ChunkId::Colors:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vColors.resize(totalSize / sizeof(HiColor));

			char* pDest = ((char*)m_vColors.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->setColors(m_vColors.data(), m_vColors.data() + m_vColors.size());

			m_pDecoder->align();
			break;
		}

		case GfxStream::ChunkId::Transforms:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vTransforms.resize(totalSize / sizeof(Transform));

			char* pDest = ((char*)m_vTransforms.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->setTransforms(m_vTransforms.data(), m_vTransforms.data() + m_vTransforms.size() );

			m_pDecoder->align();
			break;
		}


		case GfxStream::ChunkId::Commands:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression	compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vCommands.resize(totalSize / sizeof(uint16_t));

			char* pDest = ((char*)m_vCommands.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->processCommands(m_vCommands.data(), m_vCommands.data() + m_vCommands.size());

			m_pDecoder->align();
			break;
		}
		
		case GfxStream::ChunkId::UpdateRects:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression	compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vUpdateRects.resize(totalSize / sizeof(RectSPX));

			char* pDest = ((char*)m_vUpdateRects.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->beginSession(m_baseCanvasRef, m_baseCanvasSurface, m_vUpdateRects.size(), m_vUpdateRects.data(), &m_sessionInfo);

			m_pDecoder->align();
			break;
		}


		case GfxStream::ChunkId::CreateSurface:
		{
			uint16_t	objectId;
			Surface::Blueprint	bp;

			*m_pDecoder >> objectId;
			*m_pDecoder >> bp.canvas;
			*m_pDecoder >> bp.dynamic;
			*m_pDecoder >> bp.format;
			*m_pDecoder >> bp.identity;
			*m_pDecoder >> bp.mipmap;
			*m_pDecoder >> bp.sampleMethod;
			*m_pDecoder >> bp.scale;
			*m_pDecoder >> bp.size;
			*m_pDecoder >> bp.tiling;
			*m_pDecoder >> bp.paletteCapacity;
			*m_pDecoder >> bp.paletteSize;

			bp.buffered = false;
			bp.palette = nullptr;
			
			if (bp.paletteSize > 0)
			{
				auto pPalette = (Color8*) GfxBase::memStackAlloc(bp.paletteSize*4);
				*m_pDecoder >> GfxStream::ReadBytes{ bp.paletteSize*4, pPalette };
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

			*m_pDecoder >> objectId;
			*m_pDecoder >> rect;

			if( objectId > m_vObjects.size() || m_vObjects[objectId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "SurfaceUpdate with invalid objectId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_pUpdatingSurface = wg_static_cast<Surface_p>(m_vObjects[objectId]);
			m_updatingRect = rect;
			break;
		}

		case GfxStream::ChunkId::SurfacePixels:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression	compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_pSurfaceDataBuffer = new uint8_t[totalSize];

			uint8_t* pDest = m_pSurfaceDataBuffer + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
			{
				auto pixelBuffer = m_pUpdatingSurface->allocPixelBuffer(m_updatingRect);

				int lineLength = pixelBuffer.rect.w * (m_pUpdatingSurface->pixelBits()/8);

				uint8_t * pSource = m_pSurfaceDataBuffer;
				uint8_t * pDest = pixelBuffer.pixels;

				for( int y = 0 ; y < m_updatingRect.h ; y++ )
				{
					memcpy(pDest, pSource, lineLength );
					pSource += lineLength;
					pDest += pixelBuffer.pitch;
				}

				m_pUpdatingSurface->pushPixels(pixelBuffer);
				m_pUpdatingSurface->freePixelBuffer(pixelBuffer);

				delete [] m_pSurfaceDataBuffer;
				m_pSurfaceDataBuffer = nullptr;
			}

			m_pDecoder->align();
			break;
		}

		case GfxStream::ChunkId::FillSurface:
		{
			uint16_t	objectId;
			RectI		region;
			HiColor		col;

			*m_pDecoder >> objectId;
			*m_pDecoder >> region;
			*m_pDecoder >> col;

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

			*m_pDecoder >> destSurfaceId;
			*m_pDecoder >> sourceSurfaceId;
			*m_pDecoder >> sourceRect;
			*m_pDecoder >> dest;

			Surface * pDest	  = m_vSurfaces[destSurfaceId];
			Surface * pSource = m_vSurfaces[sourceSurfaceId];

			pDest->copyFrom(pSource, sourceRect, dest);
			break;
		}
*/
		case GfxStream::ChunkId::DeleteSurface:
		{
			uint16_t	objectId;

			*m_pDecoder >> objectId;

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

			*m_pDecoder >> objectId;
			*m_pDecoder >> size;
			*m_pDecoder >> nbSegments;
			*m_pDecoder >> paletteType;

			if (m_vObjects.size() <= objectId)
				m_vObjects.resize(objectId + 16, nullptr);

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

			*m_pDecoder >> objectId;
			*m_pDecoder >> nbSegments;

			static_cast<Edgemap*>(m_vObjects[objectId].rawPtr())->setRenderSegments(nbSegments);

			break;
		}

		case GfxStream::ChunkId::SetEdgemapColors:
		{
			uint16_t	objectId;
			int			begin;
			int			end;
			
			*m_pDecoder >> objectId;
			*m_pDecoder >> begin;
			*m_pDecoder >> end;

			int nColors = end - begin;

			int memAllocated = sizeof(HiColor)*nColors;
			auto * pColors = (HiColor*) GfxBase::memStackAlloc(memAllocated);
			
			for( int i = 0 ; i < nColors ; i++ )
				*m_pDecoder >> pColors[i];

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

			*m_pDecoder >> objectId;
			*m_pDecoder >> edgeBegin;
			*m_pDecoder >> edgeEnd;
			*m_pDecoder >> sampleBegin;
			*m_pDecoder >> sampleEnd;

			int nEdges = edgeEnd - edgeBegin;
			int nSamples = sampleEnd - sampleBegin;
			
			m_pUpdatingEdgemap = wg_static_cast<Edgemap_p>(m_vObjects[objectId]);

			m_edgemapUpdateEdgeBegin 	= edgeBegin;
			m_edgemapUpdateEdgeEnd 		= edgeEnd;
			m_edgemapUpdateSampleBegin	= sampleBegin;
			m_edgemapUpdateSampleEnd	= sampleEnd;

			break;
		}
				
		case GfxStream::ChunkId::EdgemapSamples:
		{
			int32_t		totalSize;
			int32_t		offset;
			uint16_t	bytes;
			Compression	compression;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bytes;
			*m_pDecoder >> compression;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_pEdgemapDataBuffer = new uint8_t[totalSize];

			uint8_t* pDest = m_pEdgemapDataBuffer + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
			{
				if( compression == Compression::None )
				{
					m_pUpdatingEdgemap->importSamples(SampleOrigo::Top, (spx *) m_pEdgemapDataBuffer, m_edgemapUpdateEdgeBegin, m_edgemapUpdateEdgeEnd, m_edgemapUpdateSampleBegin, m_edgemapUpdateSampleEnd, 1, (m_edgemapUpdateEdgeEnd-m_edgemapUpdateEdgeBegin));
				}
				else
				{
					int nEdges = m_edgemapUpdateEdgeEnd - m_edgemapUpdateEdgeBegin;
					int nSamples = m_edgemapUpdateSampleEnd - m_edgemapUpdateSampleBegin;


					int decompressedSize = (nEdges*nSamples)*sizeof(spx);
					auto pBuffer = (spx *) GfxBase::memStackAlloc(decompressedSize);

					decompress(compression, m_pEdgemapDataBuffer, totalSize, pBuffer);

					m_pUpdatingEdgemap->importSamples(SampleOrigo::Top, pBuffer, m_edgemapUpdateEdgeBegin, m_edgemapUpdateEdgeEnd, m_edgemapUpdateSampleBegin, m_edgemapUpdateSampleEnd, 1, (m_edgemapUpdateEdgeEnd-m_edgemapUpdateEdgeBegin));

					GfxBase::memStackFree(decompressedSize);
				}

				delete [] m_pEdgemapDataBuffer;
				m_pEdgemapDataBuffer = nullptr;
				m_pUpdatingEdgemap = nullptr;
			}

			m_pDecoder->align();
			break;
		}

		case GfxStream::ChunkId::DeleteEdgemap:
		{
			uint16_t	objectId;

			*m_pDecoder >> objectId;

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

	//____ reset() _______________________________________________________________

	void StreamPlayer::reset()
	{
		m_vObjects.clear();
	}


} //namespace wg
