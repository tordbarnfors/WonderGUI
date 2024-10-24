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
#include <wg_gfxdevice.h>

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

		if (header.type == GfxChunkId::OutOfData)
			return false;

		if (m_bDisplayChunkNb)
			m_charStream << std::setw(6) << std::setfill('0') << m_chunkNb << ": ";


		if( m_bDisplayStreamOffset )
			m_charStream << std::setw(8) << std::setfill('0') << m_streamOffset << " - ";

		if (header.type >= GfxChunkId_min && header.type <= GfxChunkId_max)
		{			
			m_charStream << toString(header.type);

			if (header.type == GfxChunkId::BeginRender)
			{
				m_beginRenderNb++;
				m_charStream << " [" << m_beginRenderNb << "]";
			}


			switch( header.spxFormat )
			{
				case GfxStream::SpxFormat::Int32_dec:
					break;
				case GfxStream::SpxFormat::Uint16_dec:
					m_charStream << " (spx16_dec)";
					break;
				case GfxStream::SpxFormat::Int16_int:
					m_charStream << " (spx16_int)";
					break;
				case GfxStream::SpxFormat::Uint8_int:
					m_charStream << " (spx8_int)";
					break;
			}

			m_charStream << std::endl;
		}

		switch (header.type)
		{
			case GfxChunkId::ProtocolVersion:
				uint16_t	version;

				*m_pDecoder >> version;
				m_charStream << "    version     = " << (version / 256) << "." << (version%256) << std::endl;
				break;

			case GfxChunkId::CanvasList:
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

					m_charStream << "    ref = " << toString((CanvasRef) ref) << ", size = (" << size.w << "," << size.h << "), scale = " << scale << ", format = " << toString(format) << std::endl;
				}

				break;
			}

			case GfxChunkId::BeginRender:
				break;

			case GfxChunkId::EndRender:
				break;

			case GfxChunkId::Flush:
				break;

			case GfxChunkId::BeginCanvasUpdate:
			{
				uint16_t	surfaceId;
				CanvasRef	canvasRef;
				uint8_t		dummy;
					
				*m_pDecoder >> surfaceId;
				*m_pDecoder >> canvasRef;
				*m_pDecoder >> dummy;
				
				int nUpdateRects = (header.size - 4) / (GfxStream::spxSize(header.spxFormat)*4);

				if( surfaceId > 0 )
					m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				else
					m_charStream << "    canvasRef   = " << toString(canvasRef) << std::endl;
				
				_readPrintRects( "    clip rects ", nUpdateRects );
				break;
			}
				
			case GfxChunkId::EndCanvasUpdate:
				break;

			case GfxChunkId::SetClipList:
			{
				int		nRects = header.size / 16;
				m_charStream << "    number of rects: " << nRects << std::endl;

				_readPrintRects( "    clip rects ", nRects );
				break;
			}

			case GfxChunkId::ResetClipList:
			{
				break;
			}

			case GfxChunkId::PushClipList:
			{
				int		nRects = header.size / 16;
				m_charStream << "    number of rects: " << nRects << std::endl;

				_readPrintRects( "    clip rects     ", nRects );
				break;
			}

			case GfxChunkId::PopClipList:
			{
				break;
			}

			case GfxChunkId::SetTintColor:
			{
				HiColor	col;
				*m_pDecoder >> col;

				_printColor( "    color      ", col );
				break;
			}

			case GfxChunkId::SetTintGradient:
			{
				RectI rect;
				Gradient gradient;
				
				*m_pDecoder >> rect;
				*m_pDecoder >> gradient;

				_printRect(  "    rect         ", rect );
				_printGradient( gradient );
				break;
			}

			case GfxChunkId::ClearTintGradient:
			{
				break;
			}

			case GfxChunkId::SetBlendMode:
			{
				BlendMode	blendMode;
				*m_pDecoder >> blendMode;

				m_charStream << "    mode        = " << toString(blendMode) << std::endl;
				break;
			}

			case GfxChunkId::SetBlitSource:
			{
				uint16_t	surfaceId;
				*m_pDecoder >> surfaceId;

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				break;
			}

			case GfxChunkId::SetMorphFactor:
			{
				float	factor;
				*m_pDecoder >> factor;

				m_charStream << "    factor      = " << factor << std::endl;
				break;
			}
				
			case GfxChunkId::SetFixedBlendColor:
			{
				HiColor		color;
				*m_pDecoder >> color;

				_printColor( "    color      ", color );
				break;
			}

			case GfxChunkId::SetRenderLayer:
			{
				uint16_t	layer;
				*m_pDecoder >> layer;

				m_charStream << "    layer       = " << layer << std::endl;
				break;
			}

			case GfxChunkId::Fill:
			{
				HiColor	col;

				*m_pDecoder >> col;

				_printColor( "    color      ", col );
				break;
			}

			case GfxChunkId::FillRect:
			{
				RectSPX	rect;
				HiColor	col;

				*m_pDecoder >> rect;
				*m_pDecoder >> col;

				_printRect( "    dest       ", rect );
				_printColor( "    color      ", col );
				break;
			}

			case GfxChunkId::PlotPixels:
			{
				m_pDecoder->skip(header.size);

				m_charStream << "    number of pixels: " << header.size/16 << std::endl;
				break;
			}

			case GfxChunkId::DrawLineFromTo:
			{
				CoordSPX	begin;
				CoordSPX	end;
				HiColor		color;
				GfxStream::SPX	thickness;

				*m_pDecoder >> begin;
				*m_pDecoder >> end;
				*m_pDecoder >> color;
				*m_pDecoder >> thickness;
				m_pDecoder->align();

				m_charStream << "    begin       = " << begin.x << ", " << begin.y << std::endl;
				m_charStream << "    end         = " << end.x << ", " << end.y << std::endl;
				
				_printColor( "    color      ", color );
				m_charStream << "    thickness   = " << thickness << std::endl;
				break;
			}


			case GfxChunkId::DrawLineStraight:
			{
				CoordSPX	begin;
				Direction	dir;
				GfxStream::SPX	length;
				GfxStream::SPX	thickness;
				HiColor		color;

				*m_pDecoder >> begin;
				*m_pDecoder >> dir;
				*m_pDecoder >> length;
				*m_pDecoder >> thickness;
				*m_pDecoder >> color;

				m_charStream << "    begin       = " << begin.x << ", " << begin.y << std::endl;
				m_charStream << "    direction   = " << toString(dir) << std::endl;
				m_charStream << "    length      = " << length << std::endl;

				_printColor( "    color      ", color );
				m_charStream << "    thickness   = " << thickness << std::endl;
				break;
			}

			case GfxChunkId::DrawLineStraightDeprecated:
			{
				CoordSPX	begin;
				Direction	dir;
				spx			length;
				HiColor		color;
				spx			thickness;

				*m_pDecoder >> begin;
				*m_pDecoder >> dir;
				*m_pDecoder >> length;
				*m_pDecoder >> color;
				*m_pDecoder >> thickness;

				m_charStream << "    begin       = " << begin.x << ", " << begin.y << std::endl;
				m_charStream << "    direction   = " << toString(dir) << std::endl;
				m_charStream << "    length      = " << length << std::endl;

				_printColor( "    color      ", color );
				m_charStream << "    thickness   = " << thickness << std::endl;
				break;
			}

			case GfxChunkId::Blit:
			{
				CoordSPX	dest;

				*m_pDecoder >> dest;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				break;
			}
				
			case GfxChunkId::BlitRect:
			{
				CoordSPX	dest;
				RectSPX		source;

				*m_pDecoder >> dest;
				*m_pDecoder >> source;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				_printRect( "    source     ", source );
				break;
			}

			case GfxChunkId::FlipBlit:
			{
				CoordSPX	dest;
				GfxFlip		flip;

				*m_pDecoder >> dest;
				*m_pDecoder >> flip;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				m_charStream << "    flip        = " << toString(flip) << std::endl;

				break;
			}
				
			case GfxChunkId::FlipBlitRect:
			{
				CoordSPX	dest;
				RectSPX		source;
				GfxFlip		flip;

				*m_pDecoder >> dest;
				*m_pDecoder >> source;
				*m_pDecoder >> flip;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				_printRect( "    source     ", source );
				m_charStream << "    flip        = " << toString(flip) << std::endl;
				break;
			}

			case GfxChunkId::StretchBlit:
			{
				RectSPX		dest;

				*m_pDecoder >> dest;

				_printRect("    dest       ", dest);
				break;
			}

			case GfxChunkId::StretchBlitRect:
			{
				RectSPX		dest;
				RectSPX		source;

				*m_pDecoder >> dest;
				*m_pDecoder >> source;

				_printRect( "    dest       ", dest);
				_printRect( "    source     ", source );
				break;
			}

			case GfxChunkId::StretchFlipBlit:
			{
				RectSPX		dest;
				GfxFlip		flip;

				*m_pDecoder >> dest;
				*m_pDecoder >> flip;

				_printRect("    dest       ", dest);
				m_charStream << "    flip        = " << toString(flip) << std::endl;
				break;
			}

			case GfxChunkId::StretchFlipBlitRect:
			{
				RectSPX		dest;
				RectSPX		source;
				GfxFlip		flip;

				*m_pDecoder >> dest;
				*m_pDecoder >> source;
				*m_pDecoder >> flip;

				_printRect("    dest       ", dest);
				_printRect(     "    source     ", source );
				m_charStream << "    flip        = " << toString(flip) << std::endl;
				break;
			}

			case GfxChunkId::PrecisionBlit:
			{
				RectSPX		dest;
				RectF		source;

				*m_pDecoder >> dest;
				*m_pDecoder >> source;

				_printRect(     "    dest       ", source );
				_printRect(     "    source     ", source );
				break;
			}
					
			case GfxChunkId::TransformBlit:
			{
				RectSPX		dest;
				CoordF		source;
				float		transform[2][2];

				*m_pDecoder >> dest;
				*m_pDecoder >> source;
				*m_pDecoder >> transform[0][0];
				*m_pDecoder >> transform[0][1];
				*m_pDecoder >> transform[1][0];
				*m_pDecoder >> transform[1][1];

				_printRect(     "    dest       ", source );
				m_charStream << "    source      = " << source.x << ", " << source.y << std::endl;
				m_charStream << "    transform   = [" << transform[0][0] << ", " << transform[0][1] << "]" << std::endl;
				m_charStream << "                  [" << transform[1][0] << ", " << transform[1][1] << "]" << std::endl;
				break;
			}
				
			case GfxChunkId::RotScaleBlit:
			{
				RectSPX		dest;
				float		rotationDegrees;
				float		scale;
				CoordF		srcCenter;
				CoordF		destCenter;
				
				*m_pDecoder >> dest;
				*m_pDecoder >> rotationDegrees;
				*m_pDecoder >> scale;
				*m_pDecoder >> srcCenter;
				*m_pDecoder >> destCenter;

				_printRect("    dest       ", dest);
				m_charStream << "    rotation    = " << rotationDegrees << " degrees" << std::endl;
				m_charStream << "    scale       = " << scale << std::endl;
				m_charStream << "    srcCenter   = " << srcCenter.x << ", " << srcCenter.y << std::endl;
				m_charStream << "    destCenter  = " << destCenter.x << ", " << destCenter.y << std::endl;

				break;
			}

			case GfxChunkId::Tile:
			{
				RectSPX		dest;
				CoordSPX	shift;
				
				*m_pDecoder >> dest;
				*m_pDecoder >> shift;

				_printRect( "    dest       ", dest );
				m_charStream << "    shift       = " << shift.x << ", " << shift.y << std::endl;
				break;
			}

			case GfxChunkId::FlipTile:
			{
				RectSPX		dest;
				GfxFlip		flip;
				CoordSPX	shift;
				
				*m_pDecoder >> dest;
				*m_pDecoder >> flip;
				*m_pDecoder >> shift;

				_printRect( "    dest       ", dest );
				m_charStream << "    flip        = " << toString(flip) << std::endl;
				m_charStream << "    shift       = " << shift.x << ", " << shift.y << std::endl;
				break;
			}

			case GfxChunkId::ScaleTile:
			{
				RectSPX		dest;
				float		scale;
				CoordSPX	shift;
				
				*m_pDecoder >> dest;
				*m_pDecoder >> scale;
				*m_pDecoder >> shift;

				_printRect( "    dest       ", dest );
				m_charStream << "    scale       = " << scale << std::endl;
				m_charStream << "    shift       = " << shift.x << ", " << shift.y << std::endl;
				break;
			}

			case GfxChunkId::ScaleFlipTile:
			{
				RectSPX		dest;
				float		scale;
				GfxFlip		flip;
				CoordSPX	shift;
				
				*m_pDecoder >> dest;
				*m_pDecoder >> scale;
				*m_pDecoder >> flip;
				*m_pDecoder >> shift;

				_printRect( "    dest       ", dest );
				m_charStream << "    scale       = " << scale << std::endl;
				m_charStream << "    flip        = " << toString(flip) << std::endl;
				m_charStream << "    shift       = " << shift.x << ", " << shift.y << std::endl;
				break;
			}
				
			case GfxChunkId::FlipDrawWave:
			case GfxChunkId::DrawWave:
			{
				RectI		dest;
				WaveLine	topBorder;
				WaveLine	bottomBorder;
				HiColor		frontFill;
				HiColor		backFill;
				GfxFlip		flip;			// Onlu used when m_drawTypeInProgress == flipDrawWave.
				
				*m_pDecoder >> dest;
				*m_pDecoder >> topBorder.length;
				*m_pDecoder >> topBorder.thickness;
				*m_pDecoder >> topBorder.color;
				*m_pDecoder >> topBorder.hold;
				
				*m_pDecoder >> bottomBorder.length;
				*m_pDecoder >> bottomBorder.thickness;
				*m_pDecoder >> bottomBorder.color;
				*m_pDecoder >> bottomBorder.hold;
				
				*m_pDecoder >> frontFill;
				*m_pDecoder >> backFill;
				
				if (header.type == GfxChunkId::FlipDrawWave)
					*m_pDecoder >> flip;
				
				
				_printRect(     "    dest                  ", dest );
				m_charStream << "    topBorder.length       = " << topBorder.length << std::endl;
				m_charStream << "    topBorder.thickness    = " << topBorder.thickness << std::endl;
				_printColor(    "    topBorder.color       ", topBorder.color );
				m_charStream << "    topBorder.hold         = " << topBorder.hold << std::endl;
				m_charStream << std::endl;
				
				m_charStream << "    bottomBorder.length    = " << bottomBorder.length << std::endl;
				m_charStream << "    bottomBorder.thickness = " << bottomBorder.thickness << std::endl;
				_printColor(    "    bottomBorder.color    ", bottomBorder.color );
				m_charStream << "    bottomBorder.hold      = " << bottomBorder.hold << std::endl;
				m_charStream << std::endl;
				
				_printColor(    "    frontFill             ", frontFill );
				_printColor(    "    backFill              ", backFill );
				m_charStream << std::endl;
				
				break;
			}
				
			case GfxChunkId::DrawElipse:
			{
				RectSPX	canvas;
				spx 	thickness;
				HiColor	color;
				spx		outlineThickness;
				HiColor	outlineColor;

				*m_pDecoder >> canvas;
				*m_pDecoder >> thickness;
				*m_pDecoder >> color;
				*m_pDecoder >> outlineThickness;
				*m_pDecoder >> outlineColor;

				_printRect(     "    canvas          ", canvas );
				m_charStream << "    thickness        = " << thickness << std::endl;
				_printColor(    "    color           ", color );
				m_charStream << "    outlineThickness = " << outlineThickness << std::endl;
				_printColor(    "    outlineColor    ", outlineColor );
				break;
			}

			case GfxChunkId::DrawPieChart:
			{
				float sliceSizes[32];
				HiColor sliceColors[32];
				
				RectSPX	canvas;
				float 	start;
				int 	nSlices;
				float 	hubSize;
				HiColor hubColor;
				HiColor backColor;
				bool	bRectangular;
				
				*m_pDecoder >> canvas;
				*m_pDecoder >> start;
				*m_pDecoder >> nSlices;
				*m_pDecoder >> hubSize;
				*m_pDecoder >> hubColor;
				*m_pDecoder >> backColor;
				*m_pDecoder >> bRectangular;

				assert(nSlices <= 32);
				
				*m_pDecoder >> GfxStream::ReadBytes{ nSlices*4, sliceSizes };
				*m_pDecoder >> GfxStream::ReadBytes{ nSlices*8, sliceColors };

				_printRect(     "    canvas          ", canvas );
				m_charStream << "    start           = " << start << std::endl;
				m_charStream << "    nSlices         = " << nSlices << std::endl;
				m_charStream << "    hubSize         = " << hubSize << std::endl;
				_printColor(    "    hubColor       ", hubColor );
				_printColor(    "    backColor      ", backColor );
				m_charStream << "    rectangular     = " << bRectangular << std::endl;
				
				for( int i = 0 ; i < nSlices ; i++ )
				{
					HiColor& col = sliceColors[i];
					
					m_charStream << "    slice " << (i+1) << "    size: " << sliceSizes[i] << "    color: " << (int)col.a << ", " << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << std::endl;
				}
				break;
			}

			case GfxChunkId::DrawSegments:
			{
				m_charStream << "    no data printout implemented yet." << std::endl;
				m_pDecoder->skip(header.size);
				break;
			}

			case GfxChunkId::FlipDrawSegments:
			{
				m_charStream << "    no data printout implemented yet." << std::endl;
				m_pDecoder->skip(header.size);
				break;
			}
				
			case GfxChunkId::BlitNinePatch:
			{
				RectSPX		dstRect;
				BorderSPX 	dstFrame;
				NinePatch 	patch;
				int			scale;
					
				*m_pDecoder >> dstRect;
				*m_pDecoder >> dstFrame;

				*m_pDecoder >> patch.block;
				*m_pDecoder >> patch.frame;

				*m_pDecoder >> patch.rigidPartXOfs;
				*m_pDecoder >> patch.rigidPartXLength;
				*m_pDecoder >> patch.rigidPartXSections;

				*m_pDecoder >> patch.rigidPartYOfs;
				*m_pDecoder >> patch.rigidPartYLength;
				*m_pDecoder >> patch.rigidPartYSections;

				*m_pDecoder >> scale;

				_printRect(     "    dstRect        ", dstRect );
				_printBorder(   "    dstFrame       ", dstFrame );
				_printRect(     "    patch.block    ", patch.block );
				_printBorder(   "    patch.frame    ", patch.frame );
				m_charStream << "    (rigid parts not printed)" << std::endl;
				m_charStream << "    scale          = " << scale << std::endl;
				break;
			}
				
			case GfxChunkId::EdgeSamples:
			{
				m_pDecoder->skip(header.size);
				m_charStream << "    nSamples    = " << (int)header.size/GfxStream::spxSize(header.spxFormat) << std::endl;
				break;
			}

			case GfxChunkId::CreateSurfaceDeprecated:
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

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				m_charStream << "    canvas      = " << bp.canvas << std::endl;
				m_charStream << "    dynamic     = " << bp.dynamic << std::endl;
				m_charStream << "    format      = " << toString(bp.format) << std::endl;
				m_charStream << "    identity    = " << bp.identity << std::endl;
				m_charStream << "    mipmap      = " << bp.mipmap << std::endl;
				m_charStream << "    sampleMethod= " << toString(bp.sampleMethod) << std::endl;
				m_charStream << "    scale       = " << bp.scale << std::endl;
				m_charStream << "    size        = " << bp.size.w << ", " << bp.size.h << std::endl;
				m_charStream << "    tiling      = " << bp.tiling << std::endl;
				break;
			}

			case GfxChunkId::CreateSurface:
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
				
			case GfxChunkId::BeginSurfaceUpdate:
			{
				uint16_t	surfaceId;
				RectI		region;

				*m_pDecoder >> surfaceId;
				*m_pDecoder >> region;

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				_printRect( "    region     ", region );
				break;
			}

			case GfxChunkId::SurfacePixels:
			{
				m_pDecoder->skip(header.size);

				m_charStream << "    size: " << header.size << " bytes." << std::endl;
				break;
			}

			case GfxChunkId::EndSurfaceUpdate:
			{
				break;
			}

			case GfxChunkId::FillSurface:
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

			case GfxChunkId::CopySurface:
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

			case GfxChunkId::DeleteSurface:
			{
				uint16_t	surfaceId;
				*m_pDecoder >> surfaceId;

				m_charStream << "    surfaceId   = " << surfaceId << std::endl;
				break;
			}

			case GfxChunkId::CreateEdgemap:
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

			case GfxChunkId::SetEdgemapRenderSegments:
			{
				uint16_t	edgemapId;
				uint16_t	segments;

				*m_pDecoder >> edgemapId;
				*m_pDecoder >> segments;

				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				m_charStream << "    segments    = " << segments << std::endl;
				break;
			}

			case GfxChunkId::SetEdgemapColors:
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

				
			case GfxChunkId::BeginEdgemapUpdate:
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

			case GfxChunkId::EdgemapSamples:
			{
				m_pDecoder->skip(header.size);

				m_charStream << "    size: " << header.size << " bytes." << std::endl;
				break;
			}

			case GfxChunkId::EndEdgemapUpdate:
				break;

			case GfxChunkId::DeleteEdgemap:
			{
				uint16_t	edgemapId;

				*m_pDecoder >> edgemapId;
				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				break;
			}

			case GfxChunkId::DrawEdgemap:
			{
				CoordSPX	dest;
				uint16_t	edgemapId;

				*m_pDecoder >> dest;
				*m_pDecoder >> edgemapId;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				break;
			}

			case GfxChunkId::FlipDrawEdgemap:
			{
				CoordSPX	dest;
				uint16_t	edgemapId;
				GfxFlip		flip;

				*m_pDecoder >> dest;
				*m_pDecoder >> edgemapId;
				*m_pDecoder >> flip;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << std::endl;
				m_charStream << "    edgemapId  = " << edgemapId << std::endl;
				m_charStream << "    flip        = " << toString(flip) << std::endl;
				break;
			}


/*
			case GfxChunkId::SimpleTransformBlit:
			{
				RectI		dest;
				CoordI		src;
				int			transform[2][2];

				*m_pDecoder >> dest;
				*m_pDecoder >> src;
				*m_pDecoder >> transform;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << ", " << dest.w << ", " << dest.h << std::endl;
				m_charStream << "    src         = " << src.x << ", " << src.y << std::endl;
				m_charStream << "    transform   = [ " << transform[0][0] << ", " << transform[0][1] << " ]" << std::endl;
				m_charStream << "                  [ " << transform[1][0] << ", " << transform[1][1] << " ]" << std::endl;
				break;
			}

			case GfxChunkId::ComplexTransformBlit:
			{
				RectI		dest;
				CoordF		src;
				float		transform[2][2];

				*m_pDecoder >> dest;
				*m_pDecoder >> src;
				*m_pDecoder >> transform;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << ", " << dest.w << ", " << dest.h << std::endl;
				m_charStream << "    src         = " << src.x << ", " << src.y << std::endl;
				m_charStream << "    transform   = [ " << transform[0][0] << ", " << transform[0][1] << " ]" << std::endl;
				m_charStream << "                  [ " << transform[1][0] << ", " << transform[1][1] << " ]" << std::endl;
				break;
			}

			case GfxChunkId::TransformDrawSegments:
			{
				RectI		dest;
				uint16_t	nSegments;
				uint16_t	nEdgeStrips;
				int			transform[2][2];


				(*m_pDecoder) >> dest;
				(*m_pDecoder) >> nSegments;
				(*m_pDecoder) >> nEdgeStrips;
				(*m_pDecoder) >> transform;

				m_charStream << "    dest        = " << dest.x << ", " << dest.y << ", " << dest.w << ", " << dest.h << std::endl;
				m_charStream << "    nSegments   = " << nSegments << std::endl;
				m_charStream << "    nEdgeStrips = " << nEdgeStrips << std::endl;
				m_charStream << "    transform   = [ " << transform[0][0] << ", " << transform[0][1] << " ]" << std::endl;
				m_charStream << "                  [ " << transform[1][0] << ", " << transform[1][1] << " ]" << std::endl;


				m_charStream << "    seg colors  = ";
				for (int i = 0; i < nSegments; i++)
				{
					HiColor color;
					(*m_pDecoder) >> color;

					m_charStream << "{ " << (int)color.a << ", " << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << " } ";

				}
				m_charStream << std::endl;
				break;
			}
*/

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

	//____ _printGradient() _________________________________________________

	void StreamLogger::_printGradient( const Gradient& gradient )
	{
		_printColor("        topLeft      ", gradient.topLeft);
		_printColor("        topRight     ", gradient.topRight);
		_printColor("        bottomRight  ", gradient.bottomRight);
		_printColor("        bottomLeft   ", gradient.bottomLeft);
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
