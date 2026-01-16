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
#ifndef	WG_STREAM_DOT_H
#define	WG_STREAM_DOT_H
#pragma once

#include <wg_gfxtypes.h>

#include <assert.h>
#include <cstring>


#define WG_GFXSTREAM_USE_SURFACE_UPDATE2

namespace wg
{
	class GfxStream
	{
	public:
		static constexpr int	c_maxBlockSize = 1024+256;		// Includes the block headers!	Must be at least 4096+14+block_header_size due to palette possibly included in CreateSurface-chunks.

		static constexpr int	c_maxClipRects = 256;		// Largest number of patches allowed for a drawing primitive.


		//____ Data ________________________________________________________

		struct Data
		{
			uint8_t*	pBegin;
			uint8_t*	pEnd;
		};

		//____ ChunkId ____________________________________________________

		enum class ChunkId : uint8_t    // Can't use autoExtras, change min/max/size and toString() manually!!!
		{
			OutOfData = 0,

			ProtocolVersion = 1,
			CanvasList = 2,
			Tick = 3,

			BeginRender = 4,
			EndRender = 5,
			BeginSession = 6,
			EndSession = 7,
			SetCanvas = 8,
			Objects = 9,
			Rects = 10,
			Colors = 11,
			Transforms = 12,
			Commands = 13,
			UpdateRects = 14,

			CreateSurface = 15,
			SurfaceUpdate = 16,
			SurfacePixels = 17,
			FillSurface = 18,
			CopySurface = 19,
			DeleteSurface = 20,

			CreateEdgemap = 21,
			SetEdgemapRenderSegments = 22,
			SetEdgemapColors = 23,
			EdgemapUpdate = 24,
			EdgemapSamples = 25,
			DeleteEdgemap = 26,

			SurfaceUpdate2 = 27,
			Fence = 28
		};

		const static ChunkId      ChunkId_min      = ChunkId::OutOfData;
		const static ChunkId      ChunkId_max      = ChunkId::Fence;
		const static int          ChunkId_size     = (int)ChunkId::Fence + 1;

		struct SPX
		{
			SPX() {}
			SPX(spx v) : value(v) {}
			
			operator spx() const {return value;}
			
			spx value;
		};

		enum class SpxFormat : uint8_t
		{
			Int32_dec = 0,			// 32 bit signed subpixel values.
			Uint16_dec = 1,			// 16 bit unsigned pixel values.
			Int16_int = 2,			// 16 bit signed pixel values.
			Uint8_int = 3,			// 8 bit unsigned pixel values.
//			Delta16_dec = 4,		// 16 bit delta subpixel values.
//			Delta16_int = 5,		// 16 bit delta pixel values.
//			Delta8_dec = 6,			// 8 bit delta subpixel values.
//			Delta8_int = 7			// 8 bit delta pixel values.
		};

		struct Header
		{
			ChunkId			type;
			uint8_t			dummy;		// 8 bits of future use data. Should be set to 0.
    		int				size;
		};

		struct DataInfo
		{
			int32_t		bufferSize;		// Size of buffer needed to read content
			int32_t		chunkOffset;	// Offset of this chunk.
			uint32_t	compression;	// TokenID for compression used.
			int32_t		dataStart;		// Offset at which receive data will start, which can be decompressed to buffers start.
			bool		bFirstChunk;
			bool		bLastChunk;
			bool		bPadded;
		};

		struct WriteBytes
		{
			int				bytes;
			const void *	pBuffer;
		};

		struct ReadBytes
		{
			int				bytes;
			void *			pBuffer;
		};
		
		struct ReadSpxField
		{
			int				size;
			SpxFormat		spxFormat;
			spx *			pField;
		};



		class Chunk
		{
		public:

			GfxStream::ChunkId	type() const { return m_type; }

			int			chunkSize() const
			{
				return m_size + 4;
			}

			uint8_t		dummy() const
			{
				return m_dummy;
			}

			int			dataSize() const
			{
				return m_size;
			}

			void*		data()
			{
					return ((uint8_t*)this) + 4;
			}

			Chunk *		next() const
			{
				return (Chunk*) (((uint8_t*)this) + (m_size + 4));
			}


		protected:
			GfxStream::ChunkId	m_type;
			uint8_t				m_dummy;
			uint16_t			m_size;
		};



		class iterator
		{
		public:
			iterator(void* pChunk) : m_pChunk((Chunk*)pChunk) {}


			inline Chunk operator*() const
			{
				return *m_pChunk;
			}

			inline Chunk* operator->() const
			{
				return m_pChunk;
			}

			inline iterator& operator++()
			{
				m_pChunk = m_pChunk->next();
				return *this;
			}

			inline iterator operator++(int)
			{
				iterator it = *this;
				m_pChunk = m_pChunk->next();
				return it;
			}

			inline bool operator==(const iterator& rhs) const
			{
				return m_pChunk == rhs.m_pChunk;
			}

			inline bool operator!=(const iterator& rhs) const
			{
				return m_pChunk != rhs.m_pChunk;
			}

			inline bool operator< (const iterator& rhs) const { return m_pChunk < rhs.m_pChunk; }
			inline bool operator> (const iterator& rhs) const { return m_pChunk > rhs.m_pChunk; }
			inline bool operator<=(const iterator& rhs) const { return m_pChunk <= rhs.m_pChunk; }
			inline bool operator>=(const iterator& rhs) const { return m_pChunk >= rhs.m_pChunk; }

			inline operator uint8_t*() const
			{
				return (uint8_t*) m_pChunk;
			}


		protected:
			Chunk* m_pChunk;

			virtual void	_inc() {};
	
		};

		static const int ColorSize = 8;
		static const int NinePatchSize = 16 + 8 + 10 + 10;
		static const int DataInfoSize = 18;
		static const int HeaderSize = 4;

		inline static GfxStream::ChunkId chunkType(const uint8_t* pChunk)
		{
			return (GfxStream::ChunkId) pChunk[0];
		}

		inline static int chunkSize(const uint8_t* pChunk)
		{
			return (*(uint16_t*)&pChunk[2]) + 4;
		}

		inline static int headerSize(const uint8_t* pChunk)
		{
			return 4;
		}

		inline static int dataSize(const uint8_t* pChunk)
		{
			return *(uint16_t*)&pChunk[2];
		}
		
		inline static int spxSize(SpxFormat spxFormat)
		{
			static const int size[4] = {4,2,2,1};
		
			return size[int(spxFormat)];
		}

		inline static Chunk * createChunk( void * pDest, ChunkId type, int dataSize, void * pData )
		{
			assert( reinterpret_cast<uintptr_t>(pDest) % 2 == 0 );
			assert( reinterpret_cast<uintptr_t>(pData) % 2 == 0 );
			assert( dataSize % 2 == 0 );

			((uint8_t*)pDest)[0] = (uint8_t) type;
			((uint8_t*)pDest)[1] = 0;
			((uint16_t*)pDest)[1] = dataSize;

			uint16_t * pWrite = &((uint16_t*)pData)[2];

			if( dataSize <= 16 )
			{
				uint16_t * pRead = (uint16_t*) pData;
				uint16_t * pEnd = pRead + (dataSize/2);

				while( pRead < pEnd )
					* pWrite++ = * pRead++;
			}
			else
				memcpy( pWrite, pData, dataSize );
		}


		inline static DataInfo decodeDataInfo(const uint8_t* pChunk)
		{
			DataInfo info;

			const uint16_t* pChunkData = (uint16_t*) (pChunk + headerSize(pChunk));

			info.bufferSize		= pChunkData[0] + int(pChunkData[1]) * 65536;
			info.chunkOffset	= pChunkData[2] + int(pChunkData[3]) * 65536;
			info.compression	= pChunkData[4] + int(pChunkData[5]) * 65536;
			info.dataStart		= pChunkData[6] + int(pChunkData[7]) * 65536;


			uint16_t flags = pChunkData[8];

			info.bFirstChunk = flags & 0x1;
			info.bLastChunk = (flags >> 1) & 0x1;
			info.bPadded = (flags >> 2) & 0x1;
			return info;
		};

	};

	const char * toString(GfxStream::ChunkId);
};


#endif //WG_STREAM_DOT_H
