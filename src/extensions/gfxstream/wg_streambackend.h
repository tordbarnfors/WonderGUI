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

#ifndef	WG_STREAMBACKEND_DOT_H
#define	WG_STREAMBACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <wg_streamencoder.h>
#include <wg_streamsurface.h>

#include <functional>

namespace wg
{

	class StreamBackend;
	typedef	StrongPtr<StreamBackend>	StreamBackend_p;
	typedef	WeakPtr<StreamBackend>	StreamBackend_wp;

	class StreamBackend : public GfxBackend
	{
	public:

		//.____ Creation __________________________________________

		static StreamBackend_p	create( StreamEncoder * pEncoder, int maxEdges = 15 );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void	beginSession( const SessionInfo * pSession ) override;
		void	endSession() override;

		void	setCanvas( Surface * pSurface ) override;
		void	setCanvas( CanvasRef ref ) override;

		void	setObjects(Object** pBeg, Object** pEnd) override;

		void	setCoords(spx* pBeg, spx* pEnd) override;
		void	setColors(HiColor* pBeg, HiColor* pEnd) override;
		void	setTransforms(Transform * pBeg, Transform * pEnd) override;

		void	processCommands( int32_t* pBeg, int32_t * pEnd) override;


		//.____ Misc _________________________________________________________

		bool	defineCanvas( CanvasRef ref, StreamSurface * pSurface );
		bool	defineCanvas( CanvasRef ref, const SizeI& pixelSize, PixelFormat pixelFormat, int scale = 64 );

		void	encodeCanvasList();

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		const TypeInfo& surfaceType(void) const override;



	protected:

		StreamBackend(StreamEncoder * pEncoder, int maxEdges );
		~StreamBackend();

		void _splitAndEncode( GfxChunkId chunkType, const void * pBeg, const void * pEnd, int entrySize );

		std::vector<CanvasInfo>	m_definedCanvases;
		StreamEncoder_p			m_pEncoder;
		int						m_maxEdges;

		SurfaceFactory_p		m_pSurfaceFactory;
		EdgemapFactory_p		m_pEdgemapFactory;
	};


} // namespace wg
#endif //WG_STREAMBACKEND_DOT_H
