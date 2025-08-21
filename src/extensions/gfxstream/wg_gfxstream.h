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

#ifndef	WG_STREAM_DOT_H
#define	WG_STREAM_DOT_H
#pragma once

#include <wg_gfxtypes.h>

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

		enum class ChunkId : uint8_t    //. autoExtras
		{
			OutOfData = 0,

			ProtocolVersion = 1,
			CanvasList = 2,
			TimeStampMS = 3,

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
		};

		const static ChunkId      ChunkId_min      = ChunkId::OutOfData;
		const static ChunkId      ChunkId_max      = ChunkId::DeleteEdgemap;
		const static int          ChunkId_size     = (int)ChunkId::DeleteEdgemap + 1;

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
			uint8_t			format;		// Type specific information about the format of the content. Used by some types.
    		int				size;
		};

		struct DataInfo
		{
			int32_t		totalSize;
			int32_t		chunkOffset;
			Compression	compression;
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

			uint8_t		format() const
			{
				return m_format;
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
			uint8_t				m_format;
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
		static const int GradientSize = 8 * 4;
		static const int NinePatchSize = 16 + 8 + 10 + 10;
		static const int DataInfoSize = 10;

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

	};

	const char * toString(GfxStream::ChunkId);
};


#endif //WG_STREAM_DOT_H
