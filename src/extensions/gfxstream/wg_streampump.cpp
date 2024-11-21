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

#include <wg_streampump.h>
#include <wg_surface.h>
#include <wg_patches.h>
#include <wg_gfxbase.h>

#include <vector>
#include <cstdint>
#include <cstring>

using namespace std;

namespace wg
{
	const TypeInfo StreamPump::TYPEINFO = { "StreamPump", &Object::TYPEINFO };



	//____ create() _______________________________________________________________

	StreamPump_p StreamPump::create()
	{
		StreamPump_p p(new StreamPump( StreamSource_p(), StreamSink_p()));
		return p;
	}

	StreamPump_p	StreamPump::create( const StreamSource_p& pInput, const StreamSink_p& pOutput)
	{
		StreamPump_p p(new StreamPump(pInput, pOutput));
		return p;
	}


	//____ constructor _____________________________________________________________

	StreamPump::StreamPump(const StreamSource_p& pInput, const StreamSink_p& pOutput)
	{
		m_pInput = pInput;
		m_pOutput = pOutput;
	}

	//____ Destructor ______________________________________________________________

	StreamPump::~StreamPump()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamPump::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setInput() _________________________________________________________

	void StreamPump::setInput(const StreamSource_p& pStream)
	{
		m_pInput = pStream;
	}

	//____ setOutput() ________________________________________________________

	void StreamPump::setOutput(const StreamSink_p& pStream)
	{
		m_pOutput = pStream;
	}

	//____ peekChunk() ________________________________________________________

	GfxChunkId StreamPump::peekChunk()
	{
		if (!m_pInput)
			return GfxChunkId::OutOfData;

		return m_pInput->peekChunk();
	}

	//____ pumpChunk() ________________________________________________________

	bool StreamPump::pumpChunk()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int	nSegments;
		const DataSegment* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		if (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();
			int size = GfxStream::chunkSize( pSegments[0].pBegin );

			m_pOutput->processChunks( pSegments[0].pBegin, pSegments[0].pBegin + size );
			m_pInput->discardChunks(size);

			return true;
		}

		return false;
	}

	//____ pumpUntilFrame() ________________________________________________________

	bool StreamPump::pumpUntilFrame()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		return _pumpUntilChunk(GfxChunkId::BE_BeginRender, false);
	}

	//____ pumpFrame() ________________________________________________________

	bool StreamPump::pumpFrame()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		if ( _fetchUntilChunk(GfxChunkId::BE_EndRender) )
			return _pumpUntilChunk(GfxChunkId::BE_EndRender, true);

		return false;
	}

	//____ _pumpUntilChunk() ___________________________________________________

	bool StreamPump::_pumpUntilChunk(GfxChunkId id, bool bInclusive)
	{
		int	nSegments;
		const DataSegment* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int	bytesProcessed = 0;
			for (int i = 0; i < nSegments; i++)
			{
				const uint8_t* pBegin = pSegments[i].pBegin;
				const uint8_t* pEnd = _findChunk(id, pSegments[i].pBegin, pSegments[i].pEnd);

				m_pOutput->processChunks(pBegin, pEnd);
				bytesProcessed += pEnd - pBegin;

				if (pEnd != pSegments[i].pEnd)
				{
					if (bInclusive)
					{
						bytesProcessed += GfxStream::chunkSize(pEnd);
						m_pOutput->processChunks(pEnd, pEnd + GfxStream::chunkSize(pEnd) );
					}

					m_pInput->discardChunks(bytesProcessed);
					return true;
				}
			}

			m_pInput->discardChunks(bytesProcessed);
			m_pInput->fetchChunks();
		}

		return false;
	}

	//____ _fetchUntilChunk() _________________________________________________

	bool StreamPump::_fetchUntilChunk(GfxChunkId id)
	{
		int	nSegments;
		const DataSegment* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			for (int i = 0; i < nSegments; i++)
			{
				if (_findChunk(id, pSegments[i].pBegin, pSegments[i].pEnd) != pSegments[i].pEnd)
					return true;
			}
			
			if( !m_pInput->fetchChunks() )
				break;
		}

		return false;
	}

	//____ pumpAllFramesOptimizeClipping() ____________________________________________________

	bool StreamPump::pumpAllFramesOptimizeClipping( int optimizationDepth )
	{
		// Fetch all data

		while (m_pInput->fetchChunks());

		// Show all chunks

		int	nSegments;
		const DataSegment* pSegments;
		std::tie(nSegments, pSegments) = m_pInput->showChunks();

		// Find end of last complete frame, that will be the scope of our work.
		
		const uint8_t* pLastFoundEndRender = nullptr;
		int nFrames = 0;
		int nFullSegments = 0;

		for (int i = 0; i < nSegments; i++)
		{
			auto p = _findChunk(GfxChunkId::EndRender, pSegments[i].pBegin, pSegments[i].pEnd );
			while (p != pSegments[i].pEnd)
			{
				nFullSegments = i;
				nFrames++;
				pLastFoundEndRender = p;
				p = _findChunk(GfxChunkId::EndRender, p + GfxStream::chunkSize(p), pSegments[i].pEnd);
			}
		}

		// Early out if we have one frame or less.

		if (nFrames == 0)
			return false;

		if (nFrames == 1)
			return _pumpUntilChunk(GfxChunkId::EndRender, true);

		// Build up our canvas info

		std::vector<CanvasData>	canvases;
		int totalRects = 0;

		for (int seg = 0; seg <= nFullSegments; seg++)
		{
			const uint8_t* pEnd = (seg == nFullSegments) ? pLastFoundEndRender : pSegments[seg].pEnd;
			auto p = _findChunk(GfxChunkId::BeginCanvasUpdate, pSegments[seg].pBegin, pEnd);
			while (p != pEnd)
			{
				// Get canvasRef and surfaceId from chunk, create our ID

				const uint8_t* pChunkData = p + GfxStream::headerSize(p);

				uint16_t	surfaceId	= *(const uint16_t*)pChunkData;
				CanvasRef	canvasRef	= (CanvasRef) pChunkData[2];
				totalRects += pChunkData[3];

				int id = surfaceId + (int(canvasRef) << 16);

				// See if we already have this canvas, otherwise add it to our vector.

				int idx = 0;
				for( ; idx < (int) canvases.size() ; idx++ )
				{
					if (canvases[idx].id == id)
						break;
				}

				if( idx == (int) canvases.size() )
					canvases.push_back( id );

				// Add our frame to the list

				canvases[idx].frames.push_back({ p, 0, 0 } );

				// 

				p = _findChunk(GfxChunkId::BeginCanvasUpdate, p + GfxStream::chunkSize(p), pEnd);
			}
		}

		// Copy our rectangles from the stream, possibly mask clipping rectangles

		std::vector<RectSPX>	clipRects;

		for (auto& canvas : canvases)
		{
			int maskBegin = int(clipRects.size());				// Where canvases we mask against begin

			for (int i = int(canvas.frames.size()) - 1; i >= 0; i--)
			{
				// Only optimize against last X frames of updates for this canvas.

				if( i + optimizationDepth < canvas.frames.size() )
					maskBegin = canvas.frames[i+optimizationDepth].ofsClipRects;
				
				
				int maskEnd = int(clipRects.size());				// Where we stop masking for this frame
				
				// Get data from header

				const uint8_t* pChunk = canvas.frames[i].pBegin;

				const uint8_t* pData = pChunk + GfxStream::headerSize(pChunk);

				int nRects = (GfxStream::dataSize(pChunk) - 4) / 16;
				uint16_t * pRects = (uint16_t*)(pData + 4);

				// Check if this frame completely redraws the canvas.
				
				if (nRects == 0)
				{
					//TODO: We should possibly mask against later frames and not automatically redraw whole canvas.


					canvas.frames[i].ofsClipRects = 0;							// Signal that this is a complete redraw.
					canvas.frames[i].nClipRects = 0;

					// All earlier frames can be skipped 

					while (i > 0)
					{
						i--;
						canvas.frames[i].ofsClipRects = maskBegin;
						canvas.frames[i].nClipRects = -1;
					}

					break;
				}


				// Mask and copy rectangles for this frame

				int startOfs = int(clipRects.size());
		
				// For last frame we just copy the rects

				if (i == int(canvas.frames.size()) - 1)
				{
					for (int r = 0; r < nRects; r++)
					{
						RectI rect;
						rect.x = pRects[0] + (int(pRects[1]) << 16);
						rect.y = pRects[2] + (int(pRects[3]) << 16);
						rect.w = pRects[4] + (int(pRects[5]) << 16);
						rect.h = pRects[6] + (int(pRects[7]) << 16);
						pRects += 8;

						if( rect.w > 0 && rect.h > 0 )
							clipRects.push_back(rect);
					}
				}
				else
				{
					// For all other frames we mask against all existing rectangles

					for (int r = 0; r < nRects; r++)
					{
						RectI rect;
						rect.x = pRects[0] + (int(pRects[1]) << 16);
						rect.y = pRects[2] + (int(pRects[3]) << 16);
						rect.w = pRects[4] + (int(pRects[5]) << 16);
						rect.h = pRects[6] + (int(pRects[7]) << 16);
						pRects += 8;
						
						if (rect.w > 0 && rect.h > 0)
							_maskAddRect(clipRects, maskBegin, maskEnd, rect);
					}
				}

				canvas.frames[i].ofsClipRects = startOfs;

				int nRectsLeft = int(clipRects.size()) - startOfs;

				if( nRects > 0 && nRectsLeft == 0 )
					nRectsLeft = -1;		// Signal that the whole frame should be skipped.

				canvas.frames[i].nClipRects = nRectsLeft;
			}
		}

		// Play stream, replacing BeginRenderCanvas with our opimized one and
		// filtering set/push clip-rect against canvas cliplist.
				
		int			bytesToDiscard = 0;

		for (int seg = 0; seg <= nFullSegments; seg++)
		{
			const uint8_t* pBegin = pSegments[seg].pBegin;
			const uint8_t* pEnd = (seg == nFullSegments) ? pLastFoundEndRender + GfxStream::chunkSize(pLastFoundEndRender) : pSegments[seg].pEnd;

			bytesToDiscard += pEnd - pBegin;

			const uint8_t* p = pBegin;


			while( p < pEnd )
			{
				// Process any chunks before BeginCanvasUpdate
					
				const uint8_t* p2 = p;

				while (p != pEnd )
				{
					auto chunkType = GfxStream::chunkType(p);

					assert(chunkType != GfxChunkId::EndCanvasUpdate);

					if (chunkType == GfxChunkId::BeginCanvasUpdate)
						break;
					p += GfxStream::chunkSize(p);
				}
				
				if (p != p2)
				{
					m_pOutput->processChunks(p2, p);
				}
				
				if (p != pEnd)
				{
					_optimizeCanvasUpdate(p, pEnd, canvases, clipRects, [&](const uint8_t*& pBegin, const uint8_t*& pEnd)
					{
						seg++;
						pBegin = pSegments[seg].pBegin;
						pEnd = (seg == nFullSegments) ? pLastFoundEndRender + GfxStream::chunkSize(pLastFoundEndRender) : pSegments[seg].pEnd;
						
						bytesToDiscard += pEnd - pBegin;
					} );
				}
			}
		}

		// Discard our processed chunks

		m_pInput->discardChunks(bytesToDiscard);
		return true;
	}

	//____ _optimizeCanvasUpdate() _______________________________________________

	void StreamPump::_optimizeCanvasUpdate( const uint8_t *& pBegin, const uint8_t *& pEnd, 
										   std::vector<CanvasData>& canvases, std::vector<RectSPX>	clipRects,
										   std::function<void(const uint8_t*& pBegin, const uint8_t*& pEnd)> fetch )
	{
		GfxChunkId chunkType;

		int			nActiveUpdateRects = 0;
		RectSPX *	pActiveUpdateRects = nullptr;

			
		// We are now pointing at our BeginCanvasUpdate
		// Find the optimized rects for our canvas

		auto pChunkData = pBegin + GfxStream::headerSize(pBegin);

		uint16_t	surfaceId = *(uint16_t*) pChunkData;
		CanvasRef	canvasRef = (CanvasRef) pChunkData[2];

		int id = surfaceId + (int(canvasRef) << 16);

		CanvasData* pCanvasData = canvases.data();
		while ( pCanvasData->id != id)
			pCanvasData++;

		CanvasFrame* pFrameData = &pCanvasData->frames[pCanvasData->framesPlayed++];
		
		//
	
		if( pFrameData->nClipRects == -1 )
		{
			// No updates for this canvas, just skip it, but
			// make sure to not skip any canvasUpdate inside.
						
			pBegin += GfxStream::chunkSize(pBegin);
			while( true )
			{
				if( pBegin == pEnd )
					fetch( pBegin, pEnd );
				
				auto chunkType = GfxStream::chunkType(pBegin);
				
				if (chunkType == GfxChunkId::EndCanvasUpdate)
				{
					pBegin += GfxStream::chunkSize(pBegin);
					return;
				}
				
				if( chunkType == GfxChunkId::BeginCanvasUpdate )
					_optimizeCanvasUpdate(pBegin, pEnd, canvases, clipRects, fetch);
				else
					pBegin += GfxStream::chunkSize(pBegin);
			}
		}
		else
		{
			if (pFrameData->nClipRects > 0)
			{
				// Use our (possibly) modified clip rects

				nActiveUpdateRects = pFrameData->nClipRects;
				pActiveUpdateRects = clipRects.data() + pFrameData->ofsClipRects;

	//						assert(nActiveUpdateRects < 256 );
				
				uint8_t * pTempChunk = (uint8_t*) GfxBase::memStackAlloc(16+16*nActiveUpdateRects);
				
				// Create our own BeginCanvasUpdate

				int dataSize = nActiveUpdateRects * sizeof(RectSPX) + 4;
				
				pTempChunk[0] = (uint8_t) GfxChunkId::BeginCanvasUpdate;
				pTempChunk[1] = 31;		// Force long header.
				*(uint16_t*)(pTempChunk + 2) = dataSize;
				*(uint16_t*)(pTempChunk + 4) = surfaceId;
				pTempChunk[6] = (uint8_t) canvasRef;
				pTempChunk[7] = 0;						// Dummy, used to be number of rects.

				std::memcpy(pTempChunk + 8, pActiveUpdateRects, nActiveUpdateRects * sizeof(RectSPX));
				
				// Process our created chunk

				m_pOutput->processChunks(pTempChunk, pTempChunk + 4 + dataSize);
				
				GfxBase::memStackFree(16+16*nActiveUpdateRects);
			}
			else
			{
				// Full canvas redraw, just process BeginCanvasUpdate as it is.

				m_pOutput->processChunks(pBegin, pBegin + GfxStream::chunkSize(pBegin) );
			}

			pBegin += GfxStream::chunkSize(pBegin);

			// BeginCanvasUpdate chunk processed, now proceed with content.
			
			while( true )
			{
				
				// Fetch more data if needed
				
				if( pBegin == pEnd )
				{
					fetch( pBegin, pEnd );
				}
				
				// Process all chunks until we find a chunk that needs special treatment

				{
					auto p = pBegin;
					chunkType = GfxStream::chunkType(p);
					
					while( chunkType != GfxChunkId::BeginCanvasUpdate && chunkType != GfxChunkId::EndCanvasUpdate &&
						  chunkType != GfxChunkId::PushClipList && chunkType != GfxChunkId::SetClipList )
					{
						p += GfxStream::chunkSize(p);
						
						if( p == pEnd )
						{
							m_pOutput->processChunks(pBegin, p );
							fetch( pBegin, pEnd );
							p = pBegin;
						}
						
						chunkType = GfxStream::chunkType(p);
					}
					
					if( p != pBegin )
					{
						m_pOutput->processChunks(pBegin, p );
						pBegin = p;
					}
					
				}
				
				// Process special chunks
				
				if( chunkType == GfxChunkId::EndCanvasUpdate )
				{
					auto pChunkEnd = pBegin + GfxStream::chunkSize(pBegin);
					m_pOutput->processChunks(pBegin, pChunkEnd );
					pBegin = pChunkEnd;
					return;
				}
				
				if( chunkType == GfxChunkId::BeginCanvasUpdate )
				{
					_optimizeCanvasUpdate(pBegin, pEnd, canvases, clipRects, fetch);
				}
				else
				{
					// Push or set cliplist
					
					// Get info from the chunk

					int nSrcRect = GfxStream::dataSize(pBegin) / sizeof(RectSPX);
					const RectSPX* pSrcRect = (const RectSPX*)(pBegin + GfxStream::headerSize(pBegin));

					// Create our own PushClipList/SetClipList, start by filtering and pushing rectangles.

					int nBytesAllocated = 16+(16*nSrcRect+16*nActiveUpdateRects)*2;		// This could in theory be too low...
																						// In theory we could need nSrcRect*nActiveUpdateRects.
					
					uint8_t * pTempChunk = (uint8_t*) GfxBase::memStackAlloc(nBytesAllocated);

					RectSPX* pDst = (RectSPX*)(pTempChunk + 4);

					for (int i = 0; i < nSrcRect; i++)
					{
						for (int j = 0; j < nActiveUpdateRects; j++)
						{
							if (pSrcRect[i].isOverlapping(pActiveUpdateRects[j]) )
								*pDst++ = RectSPX::overlap(pSrcRect[i], pActiveUpdateRects[j]);
						}
					}

					int nDstRect = int(pDst - (RectSPX*)(pTempChunk + 4));

					// Create the header

					pTempChunk[0] = (uint8_t) chunkType;
					pTempChunk[1] = 31;		// Force long header.
					*((uint16_t*)(pTempChunk + 2)) = nDstRect*sizeof(RectSPX);

					// Process our created chunk

					m_pOutput->processChunks( pTempChunk, (uint8_t*) pDst );

					GfxBase::memStackFree(nBytesAllocated);

					pBegin += GfxStream::chunkSize(pBegin);
				}
			}
		}
		
	}


	//____ pumpAll() __________________________________________________________

	bool StreamPump::pumpAll()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int	nSegments;
		const DataSegment* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int	bytesProcessed = 0;
			for (int i = 0; i < nSegments; i++)
			{
				m_pOutput->processChunks(pSegments[i].pBegin, pSegments[i].pEnd);
				bytesProcessed += pSegments[i].pEnd - pSegments[i].pBegin;
			}

			m_pInput->discardChunks(bytesProcessed);

			m_pInput->fetchChunks();
		}

		return true;
	}

	//____ pumpBytes() __________________________________________________________

	int StreamPump::pumpBytes( int maxBytes )
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int bytesLeft = maxBytes;
		bool done = false;
		
		while (!done)
		{
			if (!m_pInput->hasChunks())
			{
				m_pInput->fetchChunks();
				if( !m_pInput->hasChunks())
					return maxBytes - bytesLeft;				// Out of data!
			}

			int	nSegments;
			const DataSegment* pSegments;
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int	bytesProcessed = 0;
			for (int i = 0; i < nSegments && !done; i++)
			{
				const uint8_t* pBegin = pSegments[i].pBegin;
				const uint8_t* pEnd = pSegments[i].pEnd;
				const uint8_t* p = pBegin;

				if( bytesLeft >= (pEnd - pBegin) )
					p = pEnd;
				else
				{
					int chunkSize = GfxStream::chunkSize(p);

					while( p + chunkSize - pBegin <= bytesLeft )
					{
						p += chunkSize;
						chunkSize = GfxStream::chunkSize(p);
					}

					done = true;
				}

				if( p > pBegin )
				{
					m_pOutput->processChunks(pBegin, p);
					bytesProcessed += p - pBegin;
					bytesLeft -= p - pBegin;
				}
				
			}

			m_pInput->discardChunks(bytesProcessed);
		}

		return maxBytes - bytesLeft;
	}


	//____ _findChunk() _______________________________________________________

	const uint8_t* StreamPump::_findChunk(GfxChunkId id, const uint8_t * pBegin, const uint8_t * pEnd)
	{
		const uint8_t* p = pBegin;

		while (p != pEnd)
		{
			GfxChunkId chunkId = GfxStream::chunkType(p);
			if (chunkId == id)
				break;
			p += GfxStream::chunkSize(p);
		}

		return p;
	}


	//____ _maskAddRect() __________________________________________________________________

	void StreamPump::_maskAddRect(std::vector<RectI>& vRects, int startOffset, int endOffset, const RectI& rect)
	{
		// We mask in reverse order as an optimization.
		
		for (int i = endOffset - 1; i >= startOffset; --i)
		{
			RectI* pR = vRects.data() + i;

			// Bail out early if no intersection at all.

			if ((rect.x >= pR->x + pR->w) || (rect.x + rect.w <= pR->x) ||
				(rect.y >= pR->y + pR->h) || (rect.y + rect.h <= pR->y) )
				continue;															// No intersection.

			// Check for total coverage

			if ((rect.x >= pR->x) && (rect.x + rect.w <= pR->x + pR->w) &&
				(rect.y >= pR->y) && (rect.y + rect.h <= pR->y + pR->h) )
				return;  															// rect totally covered by pR


			// Clip newR against pR.

			RectI newR = rect;

			RectI mask = *pR;

			// Cut off upper part

			if (newR.y < mask.y)
			{
				RectI xR(newR.x, newR.y, newR.w, mask.y - newR.y);
				_maskAddRect(vRects, startOffset, i - 1, xR );

				newR.h -= xR.h;
				newR.y += xR.h;
			}

			// Cut off lower part

			if (newR.y + newR.h > mask.y + mask.h )
			{
				RectI	xR;
				xR.x = newR.x;
				xR.y = mask.y + mask.h;
				xR.w = newR.w;
				xR.h = (newR.y + newR.h) - (mask.y + mask.h);
				_maskAddRect(vRects, startOffset, i - 1, xR);

				newR.h -= xR.h;
			}

			if (newR.h > 0)
			{
				// Cut off left part

				if (newR.x < mask.x)
				{
					RectI	xR;
					xR.x = newR.x;
					xR.y = newR.y;
					xR.w = mask.x - newR.x;
					xR.h = newR.h;

					_maskAddRect(vRects, startOffset, i - 1, xR);
				}

				// Cut off right part

				if (newR.x + newR.w > mask.x + mask.w)
				{
					RectI	xR;
					xR.x = mask.x + mask.w;
					xR.y = newR.y;
					xR.w = (newR.x + newR.w) - (mask.x + mask.w);
					xR.h = newR.h;

					_maskAddRect(vRects, startOffset, i - 1, xR);
				}
			}

			// We have split our rectangle into visible pieces and masked them one by one.
			// There is nothing left of original rectangle.

			return;	

		}

		// If we haven't returned yet we have a patch left to add.

		vRects.push_back(rect);
	}


} // namespace
