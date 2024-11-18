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
		return std::tie(size, pRects);
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

		case GfxChunkId::OutOfData:
			return false;

		case GfxChunkId::ProtocolVersion:
		{
			uint16_t	version;
			*m_pDecoder >> version;

			//TODO: Something if version isn't supported.

			break;
		}

		case GfxChunkId::CanvasList:
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

        case GfxChunkId::BE_BeginRender:
            m_pBackend->beginRender();
            break;

        case GfxChunkId::BE_EndRender:
            m_pBackend->endRender();
            break;

		case GfxChunkId::BE_BeginSession:
		{

			// Temporary storage for variables that is stored in smaller uints than in SessionInfo

			uint16_t	nCanvases;
			uint16_t	nUpdateRects;

			uint16_t	nStateChanges;
			uint16_t	nLines;
			uint16_t	nFill;
			uint16_t	nBlit;
			uint16_t	nBlur;
			uint16_t	nEdgemapDraws;

			*m_pDecoder >> nCanvases;
			*m_pDecoder >> m_sessionInfo.canvasSize;
			*m_pDecoder >> nUpdateRects;
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
			*m_pDecoder >> m_sessionInfo.nTransforms;
			*m_pDecoder >> m_sessionInfo.nObjects;

			m_sessionInfo.nCanvases = nCanvases;
			m_sessionInfo.nUpdateRects = nUpdateRects;
			m_sessionInfo.nStateChanges = nStateChanges;
			m_sessionInfo.nLines = nLines;
			m_sessionInfo.nFill = nFill;
			m_sessionInfo.nBlit = nBlit;
			m_sessionInfo.nBlur = nBlur;
			m_sessionInfo.nEdgemapDraws = nEdgemapDraws;

			if (nUpdateRects == 0)
			{
				m_sessionInfo.pUpdateRects = nullptr;
				m_pBackend->beginSession(&m_sessionInfo);
			}
		}

		case GfxChunkId::BE_EndSession:
		{
			m_pBackend->endSession();
		}

		case GfxChunkId::BE_SetCanvas:
		{
			uint16_t	surfaceId;
			CanvasRef	canvasRef;
			uint8_t		dummy;

			*m_pDecoder >> surfaceId;
			*m_pDecoder >> canvasRef;
			*m_pDecoder >> dummy;

			if (canvasRef != CanvasRef::None)
				m_pBackend->setCanvas(canvasRef);
			else
				m_pBackend->setCanvas(m_vSurfaces[surfaceId]);
		}

		case GfxChunkId::BE_Objects:
			break;

		case GfxChunkId::BE_Rects:
		{
			int32_t		totalSize;
			int32_t		offset;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vRects.resize(totalSize / sizeof(RectSPX));

			int bytes = (header.size - 10);
			char* pDest = ((char*)m_vRects.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->setRects(&(*m_vRects.begin()), &(*m_vRects.end()));

			break;
		}

		case GfxChunkId::BE_Colors:
		{
			int32_t		totalSize;
			int32_t		offset;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vColors.resize(totalSize / sizeof(HiColor));

			int bytes = (header.size - 10);
			char* pDest = ((char*)m_vColors.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->setColors(&(*m_vColors.begin()), &(*m_vColors.end()));

			break;
		}

		case GfxChunkId::BE_Transforms:
		{
			int32_t		totalSize;
			int32_t		offset;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vTransforms.resize(totalSize / sizeof(Transform));

			int bytes = (header.size - 10);
			char* pDest = ((char*)m_vTransforms.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->setTransforms(&(*m_vTransforms.begin()), &(*m_vTransforms.end()));

			break;
		}


		case GfxChunkId::BE_Commands:
		{
			int32_t		totalSize;
			int32_t		offset;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vCommands.resize(totalSize / sizeof(int32_t));

			int bytes = (header.size - 10);
			char* pDest = ((char*)m_vCommands.data()) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
				m_pBackend->processCommands(&(*m_vCommands.begin()), &(*m_vCommands.end()));

			break;
		}
		
		case GfxChunkId::BE_UpdateRects:
		{
			int32_t		totalSize;
			int32_t		offset;
			bool		bFirstChunk;
			bool		bLastChunk;

			*m_pDecoder >> totalSize;
			*m_pDecoder >> offset;
			*m_pDecoder >> bFirstChunk;
			*m_pDecoder >> bLastChunk;

			if (bFirstChunk)
				m_vUpdateRects.resize(totalSize / sizeof(RectSPX));

			int bytes = (header.size - 10);
			char* pDest = ((char*)m_sessionInfo.pUpdateRects) + offset;

			*m_pDecoder >> GfxStream::ReadBytes{ bytes, pDest };

			if (bLastChunk)
			{
				m_sessionInfo.pUpdateRects = m_vUpdateRects.data();
				m_pBackend->beginSession(&m_sessionInfo);
			}

			break;
		}


		case GfxChunkId::CreateSurface:
		{
			uint16_t	surfaceId;
			Surface::Blueprint	bp;

			*m_pDecoder >> surfaceId;
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

			if (m_vSurfaces.size() <= surfaceId)
				m_vSurfaces.resize(surfaceId + 16, nullptr);
			else if( m_vSurfaces[surfaceId] != nullptr )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "CreateSurface with surfaceId that already is in use. The old surface will be replaced.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}
			
			m_vSurfaces[surfaceId] = m_pSurfaceFactory->createSurface(bp);

			if (bp.palette)
				GfxBase::memStackFree(bp.paletteSize*4);

			break;
		}

		case GfxChunkId::BeginSurfaceUpdate:
		{
			uint16_t	surfaceId;
			RectI		rect;

			*m_pDecoder >> surfaceId;
			*m_pDecoder >> rect;

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "BeginSurfaceUpdate with invalid SurfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			
			m_pUpdatingSurface = m_vSurfaces[surfaceId];
			m_pixelBuffer = m_pUpdatingSurface->allocPixelBuffer(rect);

			m_pWritePixels = m_pixelBuffer.pixels;

			m_surfaceBytesLeft = rect.w * rect.h * m_pUpdatingSurface->pixelBits()/8;
			break;
		}

		case GfxChunkId::SurfacePixels:
		{
            int line = int(m_pWritePixels - m_pixelBuffer.pixels) / m_pixelBuffer.pitch;
            int ofs = int(m_pWritePixels - m_pixelBuffer.pixels) % m_pixelBuffer.pitch;

            int bytesPerLine = m_pixelBuffer.rect.w * m_pUpdatingSurface->pixelBits()/8;

            int chunkBytesLeft = header.size;

			if( chunkBytesLeft > m_surfaceBytesLeft )
				chunkBytesLeft = m_surfaceBytesLeft;		// Last chunk was padded with an extra byte.

			m_surfaceBytesLeft -= chunkBytesLeft;


            while( chunkBytesLeft > 0 )
            {
                int toRead = std::min(chunkBytesLeft, bytesPerLine - ofs);
                *m_pDecoder >> GfxStream::ReadBytes{ toRead, m_pWritePixels };
                chunkBytesLeft -= toRead;

                if(toRead + ofs == bytesPerLine)
                {
                    m_pWritePixels += m_pixelBuffer.pitch - ofs;
                    ofs = 0;
                }
                else
                {
                    m_pWritePixels += toRead;
                    ofs += toRead;
                }

            }
			m_pDecoder->align();
			break;
		}

		case GfxChunkId::EndSurfaceUpdate:
		{
			assert(m_surfaceBytesLeft == 0);

			m_pUpdatingSurface->pullPixels(m_pixelBuffer);
			m_pUpdatingSurface->freePixelBuffer(m_pixelBuffer);
			m_pUpdatingSurface = nullptr;
			break;
		}

		case GfxChunkId::FillSurface:
		{
			uint16_t	surfaceId;
			RectI		region;
			HiColor		col;

			*m_pDecoder >> surfaceId;
			*m_pDecoder >> region;
			*m_pDecoder >> col;

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "FillSurface with invalid surfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_vSurfaces[surfaceId]->fill(region, col);
			break;
		}
/*
		case GfxChunkId::CopySurface:
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
		case GfxChunkId::DeleteSurface:
		{
			uint16_t	surfaceId;

			*m_pDecoder >> surfaceId;

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "DeleteSurface with invalid surfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_vSurfaces[surfaceId] = nullptr;
			break;
		}


		case GfxChunkId::CreateEdgemap:
		{
			uint16_t	edgemapId;
			SizeI		size;
			uint16_t	nbSegments;
			uint16_t	paletteType;

			*m_pDecoder >> edgemapId;
			*m_pDecoder >> size;
			*m_pDecoder >> nbSegments;
			*m_pDecoder >> paletteType;

			if (m_vEdgemaps.size() <= edgemapId)
				m_vEdgemaps.resize(edgemapId + 16, nullptr);

			Edgemap::Blueprint		bp;
			
			bp.size = size;
			bp.segments = nbSegments;
			bp.paletteType = (EdgemapPalette) paletteType;
			
			m_vEdgemaps[edgemapId] = m_pEdgemapFactory->createEdgemap(bp);
			break;
		}

		case GfxChunkId::SetEdgemapRenderSegments:
		{
			uint16_t	edgemapId;
			uint16_t	nbSegments;

			*m_pDecoder >> edgemapId;
			*m_pDecoder >> nbSegments;

			m_vEdgemaps[edgemapId]->setRenderSegments(nbSegments);
			
			break;
		}

		case GfxChunkId::SetEdgemapColors:
		{
			uint16_t	edgemapId;
			int			begin;
			int			end;
			
			*m_pDecoder >> edgemapId;
			*m_pDecoder >> begin;
			*m_pDecoder >> end;

			int nColors = end - begin;

			int memAllocated = sizeof(HiColor)*nColors;
			auto * pColors = (HiColor*) GfxBase::memStackAlloc(memAllocated);
			
			for( int i = 0 ; i < nColors ; i++ )
				*m_pDecoder >> pColors[i];

			m_vEdgemaps[edgemapId]->importPaletteEntries(begin, end, pColors);
			GfxBase::memStackFree(memAllocated);
			break;
		}
				
		case GfxChunkId::BeginEdgemapUpdate:
		{
			uint16_t	edgemapId;
			uint8_t		edgeBegin;
			uint8_t		edgeEnd;
			uint16_t	sampleBegin;
			uint16_t	sampleEnd;

			*m_pDecoder >> edgemapId;
			*m_pDecoder >> edgeBegin;
			*m_pDecoder >> edgeEnd;
			*m_pDecoder >> sampleBegin;
			*m_pDecoder >> sampleEnd;

			int nEdges = edgeEnd - edgeBegin;
			int nSamples = sampleEnd - sampleBegin;
			
			m_pUpdatingEdgemap = m_vEdgemaps[edgemapId];
			m_pWaveSampleBuffer = new spx[nEdges*nSamples+1];

			m_pWaveWriteSamples = m_pWaveSampleBuffer;

			m_waveSamplesLeft = nEdges*nSamples;

			m_waveUpdateEdgeBegin 	= edgeBegin;
			m_waveUpdateEdgeEnd 	= edgeEnd;
			m_waveUpdateSampleBegin	= sampleBegin;
			m_waveUpdateSampleEnd	= sampleEnd;

			break;
		}
				
		case GfxChunkId::EdgemapSamples:
		{
			int bytes = header.size;

			int nbSamples = bytes / GfxStream::spxSize(header.spxFormat);
			
			if( nbSamples > m_waveSamplesLeft )
				nbSamples = m_waveSamplesLeft;
			
			m_waveSamplesLeft -= nbSamples;
			
			*m_pDecoder >> GfxStream::ReadSpxField{ nbSamples, header.spxFormat, m_pWaveWriteSamples };
			m_pWaveWriteSamples += nbSamples;
			
			m_pDecoder->align();
			break;
		}

		case GfxChunkId::EndEdgemapUpdate:
		{
			m_pUpdatingEdgemap->importSamples(SampleOrigo::Top, m_pWaveSampleBuffer, m_waveUpdateEdgeBegin, m_waveUpdateEdgeEnd, m_waveUpdateSampleBegin, m_waveUpdateSampleEnd, 1, (m_waveUpdateEdgeEnd-m_waveUpdateEdgeBegin));
						
			m_pUpdatingEdgemap = nullptr;
			
			delete [] m_pWaveSampleBuffer;
			m_pWaveSampleBuffer = nullptr;
			break;
		}
				

		case GfxChunkId::DeleteEdgemap:
		{
			uint16_t	edgemapId;

			*m_pDecoder >> edgemapId;

			m_vEdgemaps[edgemapId] = nullptr;
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
		m_vSurfaces.clear();
		m_vEdgemaps.clear();
	}


} //namespace wg
