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

#ifndef	WG_STREAMPLAYER_DOT_H
#define	WG_STREAMPLAYER_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_gfxstream.h>
#include <wg_streamsink.h>
#include <wg_streamdecoder.h>
#include <wg_gfxbackend.h>
#include <wg_patches.h>

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

		//.____ Creation __________________________________________

		static StreamPlayer_p	create(GfxBackend * pBackend, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory);

		//.____ Components _______________________________________

		StreamSink		input;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________

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

		StreamDecoder_p		m_pDecoder;
		GfxBackend_p		m_pBackend;
		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;

		std::vector<Object_p>	m_vObjects;		// Surfaces and Edgemaps.

		std::vector<RectSPX>	m_vRects;
		std::vector<HiColor>	m_vColors;
		std::vector<Transform>	m_vTransforms;
		std::vector<Object*>	m_vActionObjects;
		std::vector<uint16_t>	m_vCommands;

		Surface_p			m_pUpdatingSurface;
		RectI				m_updatingRect;
		uint8_t *			m_pSurfaceDataBuffer = nullptr;

		Edgemap_p			m_pUpdatingEdgemap;
		uint8_t *			m_pEdgemapDataBuffer = nullptr;

		int					m_edgemapUpdateEdgeBegin;
		int					m_edgemapUpdateEdgeEnd;
		int					m_edgemapUpdateSampleBegin;
		int					m_edgemapUpdateSampleEnd;
		
		GfxBackend::SessionInfo	m_sessionInfo;			// Temporary for BeginSession/UpdateRects
		std::vector<RectSPX>	m_vUpdateRects;

		CanvasRef			m_baseCanvasRef;
		Surface_p			m_baseCanvasSurface;

		
		// For multi-chunk drawing operations (DrawSegments, FlipDrawSegments, DrawWave and FlipDrawWave), telling which one we are receiving edge samples for.
		
		GfxStream::ChunkId			m_drawTypeInProgress = GfxStream::ChunkId::OutOfData;
		
		char *	m_pTempBuffer = nullptr;
		int		m_bytesLoaded;
		int		m_bufferSize;

		struct ClipRectsBuffer
		{
			RectI*	pRects;
			int		nRects;
			int		capacity;
		};

		struct ClipListInfo
		{
			int nRects;
			bool bCanvas;
		};
		
		constexpr static int c_clipListBufferSize = 1024;		// Ridiculously high for most cases, but needed by StreamAnalyzer when rewinding a stream.

		std::vector<ClipListInfo>	m_clipListInfoStack;		// Number of rects for each clipList pushed.

		ClipRectsBuffer		m_clipListBuffer = { nullptr, 0, 0 };
		ClipListInfo		m_clipList = { 0, false };

		std::vector<ClipRectsBuffer>	m_clipListBufferStack;

		bool				m_bStoreDirtyRects = false;
		int					m_maxDirtyRects = 64;
		PatchesSPX			m_dirtyRects[CanvasRef_size];

		std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)>	m_canvasInfoCallback;
	};

}

#endif // WG_STREAMPLAYER_DOT_H
