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
#ifndef	WG_STREAMLOGGER_DOT_H
#define	WG_STREAMLOGGER_DOT_H
#pragma once

#include <iostream>

#include <wg_object.h>
#include <wg_streamsink.h>
#include <wg_gfxstream.h>
#include <wg_streamdecoder.h>
#include <wg_backendlogger.h>

namespace wg
{

	class StreamLogger;
	typedef	StrongPtr<StreamLogger>	StreamLogger_p;
	typedef	WeakPtr<StreamLogger>	StreamLogger_wp;

	class StreamLogger : public Object, protected StreamSink::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamLogger_p	create( std::ostream& out );

		//.____ Components _______________________________________

		StreamSink		input;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior __________________________________________
		
		void	setDisplayOffset(bool bDisplay) { m_bDisplayStreamOffset = bDisplay; }
		bool	displayOffset() const { return m_bDisplayStreamOffset; }

		void	setDisplayChunkNb(bool bDisplay) { m_bDisplayChunkNb = bDisplay; }
		bool	displayChunkNb() const { return m_bDisplayChunkNb; }

		void	resetCounters();


	protected:
		StreamLogger( std::ostream& out );
		~StreamLogger();

		void	_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) override;

		bool	_logChunk();

		int		_readPrintPatches();
		void	_readPrintDataInfo(int payloadSize);



		void	_readPrintRects( const char * header, int amount );
		void	_printRect( const char * header, const RectI& rect );
		void	_printRect( const char * header, const RectF& rect );
		void	_printColor( const char * header, HiColor color );
		void	_printBorder( const char * header, const BorderI& border );
		void	_printBorder(const char* header, const Border& border);

		void	_printSPX(spx value);

		std::ostream&	m_charStream;

		StreamDecoder_p m_pDecoder;
		
		bool			m_bDisplayStreamOffset = true;
		bool			m_bDisplayChunkNb = true;
		
		int				m_spxPrintMode = 0;


		uint64_t		m_streamOffset = 0;
		int				m_chunkNb = 0;
		int				m_beginRenderNb = 0;

	};

}

#endif // WG_STREAMLOGGER_DOT_H
