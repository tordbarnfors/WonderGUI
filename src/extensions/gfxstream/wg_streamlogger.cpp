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

#include <wg_streamlogger.h>
#include <wg_gradient.h>
#include <wg_gfxbackend.h>

#include <assert.h>

#include <iomanip>

namespace wg
{

	const TypeInfo StreamLogger::TYPEINFO = { "StreamLogger", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamLogger_p StreamLogger::create(std::ostream& out)
	{
		return new StreamLogger(out);
	}

	//____ constructor _____________________________________________________________

	StreamLogger::StreamLogger(std::ostream& out) : m_charStream(out), input(this)
	{
		out << std::boolalpha;

		m_pDecoder = StreamDecoder::create();
		m_pBackend = BackendLogger::create(out, nullptr);
	}

	//____ Destructor _________________________________________________________

	StreamLogger::~StreamLogger()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamLogger::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ resetCounters() ____________________________________________________

	void StreamLogger::resetCounters()
	{
		m_streamOffset = 0;
		m_chunkNb = 0;
		m_beginRenderNb = 0;
	}



	//____ _processStreamChunks() _____________________________________________

	void StreamLogger::_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd)
	{
		m_pDecoder->setInput(pBegin, pEnd);

		while (_logChunk() == true);
	}

	//____ _logChunk() ____________________________________________________________

	bool StreamLogger::_logChunk()
	{
		GfxStream::Header header;

		int chunkSize = m_pDecoder->chunkSize();
		
		*m_pDecoder >> header;

		if (header.type == GfxStream::ChunkId::OutOfData)
			return false;

		if (m_bDisplayChunkNb)
			m_charStream << std::setw(6) << std::setfill('0') << m_chunkNb << ": ";


		if( m_bDisplayStreamOffset )
			m_charStream << std::setw(8) << std::setfill('0') << m_streamOffset << " - ";

		if (header.type >= GfxStream::ChunkId_min && header.type <= GfxStream::ChunkId_max)
		{			
			m_charStream << toString(header.type);

			if (header.type == GfxStream::ChunkId::BeginRender)
			{
				m_beginRenderNb++;
				m_charStream << " [" << m_beginRenderNb << "]";
			}

			if( header.format != 0 )
				m_charStream << "(format=" << header.format << ")";

			m_charStream << std::endl;
		}

		switch (header.type)
		{
			case GfxStream::ChunkId::ProtocolVersion:
				uint16_t	version;

				*m_pDecoder >> version;
				m_charStream << "    version     = " << (version / 256) << "." << (version%256) << std::endl;
				break;

			case GfxStream::ChunkId::CanvasList:
			{
				uint16_t nbCanvases;

				*m_pDecoder >> nbCanvases;

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

					m_charStream << "    " << toString((CanvasRef) ref) << ": size = (" << size.w << "," << size.h << "), scale = " << scale << ", format = " << toString(format) << std::endl;
				}

				break;
			}

			case GfxStream::ChunkId::TimeStampMS:
				break;

			case GfxStream::ChunkId::BeginRender:
				break;

			case GfxStream::ChunkId::EndRender:
				break;

			case GfxStream::ChunkId::BeginSession:
			{
				GfxBackend::SessionInfo sessionInfo;

				// Temporary storage for variables that is stored in smaller uints than in SessionInfo

				uint16_t	objectId;
				CanvasRef	canvasRef;
				uint8_t		dummy;
				uint16_t	nUpdateRects;

				uint16_t	nSetCanvas;
				uint16_t	nStateChanges;
				uint16_t	nLines;
				uint16_t	nFill;
				uint16_t	nBlit;
				uint16_t	nBlur;
				uint16_t	nEdgemapDraws;
				int			nLineCoords;
				int			nLineClipRects;
				int			nRects;
				int			nColors;

				uint16_t	nTransforms;
				uint16_t	nObjects;

				*m_pDecoder >> objectId;
				*m_pDecoder >> canvasRef;
				*m_pDecoder >> dummy;

				*m_pDecoder >> nUpdateRects;

				*m_pDecoder >> nSetCanvas;
				*m_pDecoder >> nStateChanges;
				*m_pDecoder >> nLines;
				*m_pDecoder >> nFill;
				*m_pDecoder >> nBlit;
				*m_pDecoder >> nBlur;
				*m_pDecoder >> nEdgemapDraws;
				*m_pDecoder >> nLineCoords;

				*m_pDecoder >> nLineClipRects;
				*m_pDecoder >> nRects;
				*m_pDecoder >> nColors;
				*m_pDecoder >> nTransforms;
				*m_pDecoder >> nObjects;

				m_charStream << "    canvasObject = " << objectId << std::endl;
				m_charStream << "    canvasRef = " << toString(canvasRef) << std::endl;
				m_charStream << "    nUpdateRects " << nUpdateRects << std::endl;

				m_charStream << "    nSetCanvas = " << nSetCanvas << std::endl;
				m_charStream << "    nStateChanges = " << nStateChanges << std::endl;
				m_charStream << "    nLines = " << nLines << std::endl;
				m_charStream << "    nFill = " << nFill << std::endl;
				m_charStream << "    nBlit = " << nBlit << std::endl;
				m_charStream << "    nBlur = " << nBlur << std::endl;
				m_charStream << "    nEdgemapDraws = " << nEdgemapDraws << std::endl;
				m_charStream << "    nLineCoords = " << nLineCoords << std::endl;
				m_charStream << "    nLineClipRects = " << nLineClipRects << std::endl;
				m_charStream << "    nRects = " << nRects << std::endl;
				m_charStream << "    nColors = " << nColors << std::endl;
				m_charStream << "    nTransforms = " << nTransforms << std::endl;
				m_charStream << "    nObjects = " << nObjects << std::endl;

				break;
			}

			case GfxStream::ChunkId::EndSession:
				break;

			case GfxStream::ChunkId::SetCanvas:
			{
				uint16_t	objectId;
				CanvasRef	canvasRef;
				uint8_t		dummy;

				*m_pDecoder >> objectId;
				*m_pDecoder >> canvasRef;
				*m_pDecoder >> dummy;

				m_charStream << "    objectId = " << objectId << std::endl <<"    ref = " << toString((CanvasRef) canvasRef) << std::endl;
				break;
			}

			case GfxStream::ChunkId::Objects:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::Rects:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::Colors:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::Transforms:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::Commands:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::UpdateRects:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}


			case GfxStream::ChunkId::CreateSurface:
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


				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				m_charStream << "    canvas      = " << bp.canvas << std::endl;
				m_charStream << "    dynamic     = " << bp.dynamic << std::endl;
				m_charStream << "    format      = " << toString(bp.format) << std::endl;
				m_charStream << "    identity    = " << bp.identity << std::endl;
				m_charStream << "    mipmap      = " << bp.mipmap << std::endl;
				m_charStream << "    sampling    = " << toString(bp.sampleMethod) << std::endl;
				m_charStream << "    scale       = " << bp.scale << std::endl;
				m_charStream << "    size        = " << bp.size.w << ", " << bp.size.h << std::endl;
				m_charStream << "    tiling      = " << bp.tiling << std::endl;
				m_charStream << "    paletteCap. = " << bp.paletteCapacity << std::endl;
				m_charStream << "    paletteSize = " << bp.paletteSize << std::endl;
				break;
			}
				
			case GfxStream::ChunkId::SurfaceUpdate:
			{
				uint16_t	surfaceId;
				RectI		region;

				*m_pDecoder >> surfaceId;
				*m_pDecoder >> region;

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				_printRect( "    region     ", region );
				break;
			}

			case GfxStream::ChunkId::SurfacePixels:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::FillSurface:
			{
				uint16_t	surfaceId;
				RectI		region;
				HiColor		col;

				*m_pDecoder >> surfaceId;
				*m_pDecoder >> region;
				*m_pDecoder >> col;

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				_printRect( "    region     ", region );
				_printColor("    color      ", col );
				break;
			}

			case GfxStream::ChunkId::CopySurface:
			{
				uint16_t	destSurfaceId;
				uint16_t	sourceSurfaceId;
				RectI		sourceRect;
				CoordI		dest;

				*m_pDecoder >> destSurfaceId;
				*m_pDecoder >> sourceSurfaceId;
				*m_pDecoder >> sourceRect;
				*m_pDecoder >> dest;

				m_charStream << "    destSurface = " << destSurfaceId << std::endl;
				m_charStream << "    srcSurface  = " << sourceSurfaceId << std::endl;
				_printRect( "    source     ", sourceRect );
				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				break;
			}

			case GfxStream::ChunkId::DeleteSurface:
			{
				uint16_t	surfaceId;
				*m_pDecoder >> surfaceId;

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				break;
			}

			case GfxStream::ChunkId::CreateEdgemap:
			{

				uint16_t	edgemapId;
				SizeI		size;
				uint16_t	nbSegments;
				uint16_t	paletteType;

				*m_pDecoder >> edgemapId;
				*m_pDecoder >> size;
				*m_pDecoder >> nbSegments;
				*m_pDecoder >> paletteType;

				

				m_charStream << "    edgemapId   = " << edgemapId << std::endl;
				m_charStream << "    size        = " << size.w << ", " << size.h << std::endl;
				m_charStream << "    segments    = " << nbSegments << std::endl;
				m_charStream << "    paletteType = " << toString((EdgemapPalette) paletteType) << std::endl;
				break;
			}

			case GfxStream::ChunkId::SetEdgemapRenderSegments:
			{
				uint16_t	edgemapId;
				uint16_t	segments;

				*m_pDecoder >> edgemapId;
				*m_pDecoder >> segments;

				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				m_charStream << "    segments    = " << segments << std::endl;
				break;
			}

			case GfxStream::ChunkId::SetEdgemapColors:
			{
				uint16_t	edgemapId;
				int			begin;
				int			end;
				
				*m_pDecoder >> edgemapId;
				*m_pDecoder >> begin;
				*m_pDecoder >> end;

				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				m_charStream << "    begin       = " << begin << std::endl;
				m_charStream << "    end         = " << end << std::endl;
				break;
			}

				
			case GfxStream::ChunkId::EdgemapUpdate:
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

				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				m_charStream << "    edgeBegin   = " << edgeBegin << std::endl;
				m_charStream << "    edgeEnd     = " << edgeEnd << std::endl;
				m_charStream << "    sampleBegin = " << sampleBegin << std::endl;
				m_charStream << "    sampleEnd   = " << sampleEnd << std::endl;

				break;
			}

			case GfxStream::ChunkId::EdgemapSamples:
			{
				_readPrintDataInfo();
				m_pDecoder->skip(header.size - GfxStream::DataInfoSize);
				break;
			}

			case GfxStream::ChunkId::DeleteEdgemap:
			{
				uint16_t	edgemapId;

				*m_pDecoder >> edgemapId;
				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				break;
			}


			default:
			{
				m_charStream << "Unknown Chunk Type: " << (int) header.type << std::endl;
				m_charStream << "    size: " << (int)header.size << std::endl;

				m_pDecoder->skip(header.size);
				break;
			}				
		}

		m_streamOffset += chunkSize;
		m_chunkNb++;
		return true;
	}

	//____ readPrintPatches() _________________________________________________

	int StreamLogger::_readPrintPatches()
	{
		uint16_t	nPatches;
		RectI		patch;

		*m_pDecoder >> nPatches;

		if (nPatches == 0)
		{
			m_charStream << "    zero patches!" << std::endl;
			return 0;
		}

		*m_pDecoder >> patch;
		m_charStream << "    patches     = " << patch.x << ", " << patch.y << ", " << patch.w << ", " << patch.h << std::endl;

		for (int i = 1; i < nPatches; i++)
		{
			*m_pDecoder >> patch;
			m_charStream << "                  " << patch.x << ", " << patch.y << ", " << patch.w << ", " << patch.h << std::endl;
		}

		return nPatches;
	}

	//____ _readPrintDataInfo() _______________________________________________

	void StreamLogger::_readPrintDataInfo()
	{
		GfxStream::DataInfo dataInfo;
		*m_pDecoder >> dataInfo;

		m_charStream << "    totalSize = " << dataInfo.totalSize << " bytes" << std::endl;
		m_charStream << "    chunkOffset = " << dataInfo.chunkOffset << std::endl;
		m_charStream << "    compression = " << int(dataInfo.compression) << ", bFirstChunk = " << dataInfo.bFirstChunk << ", bLastChunk = " << dataInfo.bLastChunk << ", bPadded = " << dataInfo.bPadded << std::endl;
	}


	//____ readPrintRects() _________________________________________________

	void StreamLogger::_readPrintRects( const char * label, int nRects )
	{
		RectI		rect;

		if (nRects == 0)
		{
			m_charStream << label << " = none" << std::endl;
			return;
		}

		*m_pDecoder >> rect;
		m_charStream << label << " = " << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << std::endl;

		for (int i = 1; i < nRects; i++)
		{
			*m_pDecoder >> rect;
			m_charStream << "                  " << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << std::endl;
		}
	}

	//____ _printRect() _________________________________________________

	void StreamLogger::_printRect( const char * header, const RectI& rect )
	{
		m_charStream << header << " = " << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << std::endl;
	}

	void StreamLogger::_printRect( const char * header, const RectF& rect )
	{
		m_charStream << header << " = " << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << std::endl;
	}

	//____ _printColor() _________________________________________________

	void StreamLogger::_printColor( const char * header, HiColor color )
	{
		m_charStream << header << " = " << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << ", " << (int)color.a << std::endl;
	}

	//____ _printBorder() _________________________________________________

	void StreamLogger::_printBorder( const char * header, const BorderI& border )
	{
		m_charStream << header << " = " << border.top << ", " << border.right << ", " << border.bottom << ", " << border.left << std::endl;
	}

	void StreamLogger::_printBorder(const char* header, const Border& border)
	{
		m_charStream << header << " = " << border.top << ", " << border.right << ", " << border.bottom << ", " << border.left << std::endl;
	}

	//____ _printSPX() ___________________________________________________

	void StreamLogger::_printSPX( spx value )
	{
		switch( m_spxPrintMode )
		{
			default:
			case 0:
				m_charStream << value;
				break;
			case 1:
			{
				int integer = value / 64;
				int decimal = value % 64;
				m_charStream << integer << ":" << decimal;
				break;
			}
			case 2:
			{
				int integer = value / 64;
				int decimal = value % 64;
				m_charStream << integer;
				if( decimal != 0 )
					m_charStream << ":" << decimal;
				break;
			}
		}
	}

} //namespace wg
