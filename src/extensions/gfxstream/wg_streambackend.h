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
#ifndef	WG_STREAMBACKEND_DOT_H
#define	WG_STREAMBACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <wg_streamencoder.h>
#include <wg_remotesurface.h>
#include <wg_compressor.h>

#include <functional>

namespace wg
{

	class StreamBackend;
	typedef	StrongPtr<StreamBackend>	StreamBackend_p;
	typedef	WeakPtr<StreamBackend>	StreamBackend_wp;

	class StreamBackend : public GfxBackend
	{
		friend class RemoteSurface;
		friend class MirrorSurface;
		friend class StreamEdgemap;

	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			Compressor_p	colorCompressor;
			Compressor_p	commandCompressor;
			StreamEncoder_p	encoder;					// Mandatory!
			Finalizer_p		finalizer = nullptr;
			int				maxEdges = 15;
			Compressor_p	objectCompressor;
			Compressor_p	rectCompressor;		// Both for render rects and update rects.
			Compressor_p	transformCompressor;
		};

		//.____ Creation ____________________________________________________________

		static StreamBackend_p	create( StreamEncoder * pEncoder, int maxEdges = 15 );
		static StreamBackend_p	create( const Blueprint& blueprint );

		//.____ Identification ______________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ___________________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void	beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo = nullptr ) override;
		void	endSession() override;

		void	setCanvas( Surface * pSurface ) override;
		void	setCanvas( CanvasRef ref ) override;

		void	setObjects(Object* const * pBeg, Object* const * pEnd) override;

		void	setRects(const RectSPX* pBeg, const RectSPX* pEnd) override;
		void	setColors(const HiColor* pBeg, const HiColor* pEnd) override;
		void	setTransforms(const Transform * pBeg, const Transform * pEnd) override;

		void	processCommands( const uint16_t* pBeg, const uint16_t * pEnd) override;


		//.____ Misc ________________________________________________________________

		bool	defineCanvas( CanvasRef ref, RemoteSurface * pSurface );
		bool	defineCanvas( CanvasRef ref, const SizeI& pixelSize, PixelFormat pixelFormat, int scale = 64 );

		void	encodeCanvasList();
		void	encodeTick(int32_t microsecPassed);		// Microsec since last tick (or beginning of stream if first tick)


		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;


	protected:

		StreamBackend(StreamEncoder * pEncoder, int maxEdges );
		StreamBackend(const Blueprint& blueprint );
		~StreamBackend();

		// Static so it can be used by StreamSurface and StreamEdgemap as well.

		static void _compressSplitAndEncode( StreamEncoder * pEncoder, GfxStream::ChunkId chunkType, Compressor * pCompressor, const void * pBeg, const void * pEnd );

		std::vector<CanvasInfo>	m_definedCanvases;
		StreamEncoder_p			m_pEncoder;
		int						m_maxEdges;

		SurfaceFactory_p		m_pSurfaceFactory;
		EdgemapFactory_p		m_pEdgemapFactory;

		Compressor_p			m_pObjectCompressor;
		Compressor_p			m_pRectCompressor;		// Both for render rects and update rects.
		Compressor_p			m_pColorCompressor;
		Compressor_p			m_pTransformCompressor;
		Compressor_p			m_pCommandCompressor;

		std::vector<uint16_t>	m_objects;

	};


} // namespace wg
#endif //WG_STREAMBACKEND_DOT_H
