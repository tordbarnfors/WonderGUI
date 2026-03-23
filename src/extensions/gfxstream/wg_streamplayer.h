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
#ifndef	WG_STREAMPLAYER_DOT_H
#define	WG_STREAMPLAYER_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_gfxstream.h>
#include <wg_streamsink.h>
#include <wg_streamdecoder.h>
#include <wg_gfxbackend.h>
#include <wg_patches.h>
#include <wg_compressor.h>

#include <vector>
#include <tuple>

namespace wg
{

	class StreamPlayer;
	typedef	StrongPtr<StreamPlayer>	StreamPlayer_p;
	typedef	WeakPtr<StreamPlayer>	StreamPlayer_wp;

	class StreamPlayer : public Object, protected StreamSink::Holder
	{
	public:

		//.____ Blueprint ___________________________________________________________
/*

	TODO: Implement!!!

		struct Blueprint
		{
			bool						autoCompressors = true;		// Create and use known compressors on demand.
			std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)> canvasInfoCallback;
			std::vector<Compressor_p>	compressors;
			GfxBackend_p				backend;
			EdgemapFactory_p			edgemapFactory;
			Finalizer_p					finalizer = nullptr;
			int							maxDirtyRects = 64;
			bool						storeDirtyRects = false;
			SurfaceFactory_p			surfaceFactory;
		};
*/
		//.____ Creation ____________________________________________________________

		static StreamPlayer_p	create(GfxBackend * pBackend, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory);

		//.____ Components __________________________________________________________

		StreamSink		input;

		//.____ Identification ______________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc ________________________________________________________________

		void	setStoreDirtyRects(bool bStore);
		void	setMaxDirtyRects(int max);

		std::tuple<int, const RectI*> dirtyRects(CanvasRef canvas);
		void	clearDirtyRects();

		void	setCanvasInfoCallback( const std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)>& callback );
		
		void	reset();
		
		const std::vector<Object_p>& objects() const { return m_vObjects; }

	protected:
		StreamPlayer(GfxBackend * pDevice, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory);
		~StreamPlayer();

		void	_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) override;
		bool	_playChunk();
		Compressor * _findCompressor( uint32_t idToken );

		struct DataBuffer
		{
			~DataBuffer() { delete [] pBuffer; }

			void release() { delete [] pBuffer; pBuffer = nullptr; capacity = 0; size = 0; }

			void reset(int minCapacity)
			{
				if( minCapacity > capacity )
				{
					delete [] pBuffer;
					pBuffer = new uint8_t[minCapacity];
					capacity = minCapacity;
					size = 0;
				}
			}

			uint8_t *	pBuffer = nullptr;
			int			capacity = 0;
			int			size = 0;			// Stays zero until buffer filled and data decompressed.
		};

//		bool	_loadChunkIntoDataBuffer( DataBuffer& buffer, int chunkDataSize );
		bool	_loadIntoDataBuffer( GfxStream::DataInfo& dataInfo, DataBuffer& buffer, int chunkDataSize );


		StreamDecoder_p		m_pDecoder;
		GfxBackend_p		m_pBackend;
		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;

		std::vector<Object_p>	m_vObjects;		// Surfaces and Edgemaps.

		std::vector<Object*>	m_vActionObjects;

		DataBuffer				m_objectsDataBuffer;
		DataBuffer				m_rectsDataBuffer;
		DataBuffer				m_transformsDataBuffer;
		DataBuffer				m_colorsDataBuffer;
		DataBuffer				m_commandsDataBuffer;


		struct SurfaceDataBuffer
		{
			SurfaceDataBuffer(const Surface_p& pSurface) : pSurface(pSurface) {}

			Surface_p			pSurface;
			std::vector<RectI>	rects;
			DataBuffer			buffer;
		};

		struct EdgemapDataBuffer
		{
			EdgemapDataBuffer(const Edgemap_p& pEgemap, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
			  :	pEdgemap(pEdgemap),
				edgeBegin(edgeBegin),
				edgeEnd(edgeEnd),
				sampleBegin(sampleBegin),
				sampleEnd(sampleEnd) {}

			Edgemap_p			pEdgemap;

			int					edgeBegin;
			int					edgeEnd;
			int					sampleBegin;
			int					sampleEnd;

			DataBuffer			buffer;
		};

		std::vector<SurfaceDataBuffer>	m_surfaceDataBuffers;
		std::vector<EdgemapDataBuffer>	m_edgemapDataBuffers;


		GfxBackend::SessionInfo	m_sessionInfo;			// Temporary for BeginSession/UpdateRects
		DataBuffer			m_updateRectsDataBuffer;

		CanvasRef			m_baseCanvasRef;
		Surface_p			m_baseCanvasSurface;

		
		// For multi-chunk drawing operations (DrawSegments, FlipDrawSegments, DrawWave and FlipDrawWave), telling which one we are receiving edge samples for.
		
		GfxStream::ChunkId			m_drawTypeInProgress = GfxStream::ChunkId::OutOfData;
		
		char *	m_pTempBuffer = nullptr;
		int		m_bytesLoaded;
		int		m_bufferSize;
		

		std::vector<Compressor_p>		m_compressors;
		bool				m_bAutoCompressors = true;

		bool				m_bStoreDirtyRects = false;
		int					m_maxDirtyRects = 64;
		PatchesSPX			m_dirtyRects[CanvasRef_size];

		std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)>	m_canvasInfoCallback;
	};

}

#endif // WG_STREAMPLAYER_DOT_H
