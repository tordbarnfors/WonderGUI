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
#ifndef WG_STREAMPUMP_DOT_H
#define WG_STREAMPUMP_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_object.h>
#include <wg_componentptr.h>

#include <wg_streamsource.h>
#include <wg_streamsink.h>

#include <vector>
#include <functional>

namespace wg
{
	class StreamPump;
	typedef	StrongPtr<StreamPump>	StreamPump_p;
	typedef	WeakPtr<StreamPump>		StreamPump_wp;

	class StreamPump : public Object
	{
	public:

		//.____ Creation ______________________________________________________

		static StreamPump_p	create();
		static StreamPump_p	create( const StreamSource_p& pInput, const StreamSink_p& pOutput );

		//.____ Identification ________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________________

		void		setInput(const StreamSource_p& pInput);
		void		setOutput(const StreamSink_p& pOutput);

		GfxStream::ChunkId	peekChunk();
		bool		pumpChunk();
		bool		pumpUntilFrame();
		bool		pumpFrame();
		bool		pumpAllFramesOptimizeClipping( int optimizationDepth = 1024 );
		bool		pumpAll();
		int			pumpBytes( int maxBytes );		// Will pump full chunks up until maxBytes has been reached. Will likely return fewer bytes than requested.
													// Will return 0 if no more data or first chunk is larger than maxBytes.
		

	protected:

		StreamPump(const StreamSource_p& pInput, const StreamSink_p& pOutput);
		~StreamPump();

		struct CanvasFrame
		{
			const uint8_t*	pBegin;
			int				ofsClipRects;
			int				nClipRects;
		};

		struct CanvasData
		{
			CanvasData(int _id) : id(_id), framesPlayed(0) {}

			int id;				// surfaceId + canvasRef << 16;
			std::vector<CanvasFrame>	frames;
			int framesPlayed;
		};
		
		
		const uint8_t*	_findChunk(GfxStream::ChunkId id, const uint8_t* pBegin, const uint8_t* pEnd);


		bool			_pumpUntilChunk(GfxStream::ChunkId id, bool bInclusive );
		bool			_fetchUntilChunk(GfxStream::ChunkId id);


		StreamSource_p	m_pInput;
		StreamSink_p		m_pOutput;
	};
}

#endif // WG_STREAMPUMP_DOT_H
