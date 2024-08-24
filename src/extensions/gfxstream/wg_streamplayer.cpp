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

#include <wg_streamplayer.h>
#include <wg_gfxbase.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamPlayer::TYPEINFO = { "StreamPlayer", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamPlayer_p StreamPlayer::create(GfxDevice * pDevice, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory )
	{
		return new StreamPlayer(pDevice, pSurfaceFactory, pEdgemapFactory);
	}

	//____ constructor _____________________________________________________________

	StreamPlayer::StreamPlayer(GfxDevice * pDevice, SurfaceFactory * pSurfaceFactory, EdgemapFactory * pEdgemapFactory) : input(this)
	{
		m_pDevice = pDevice;
		m_pSurfaceFactory = pSurfaceFactory;
		m_pEdgemapFactory = pEdgemapFactory;

		m_pDecoder = StreamDecoder::create();

		m_clipListBuffer.pRects = new RectI[c_clipListBufferSize];
		m_clipListBuffer.nRects = 0;
		m_clipListBuffer.capacity = c_clipListBufferSize;
	}

	//____ Destructor _________________________________________________________

	StreamPlayer::~StreamPlayer()
	{
		delete[] m_clipListBuffer.pRects;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamPlayer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setStoreDirtyRects() _______________________________________________

	void StreamPlayer::setStoreDirtyRects(bool bStore)
	{
		if (bStore == m_bStoreDirtyRects)
			return;

		m_bStoreDirtyRects = bStore;
		if (!bStore)
			clearDirtyRects();
	}

	//____ setMaxDirtyRects() ____________________________________________________

	void StreamPlayer::setMaxDirtyRects(int max)
	{
		m_maxDirtyRects = max;
	}

	//____ dirtyRects() __________________________________________________________

	std::tuple<int, const RectSPX*> StreamPlayer::dirtyRects(CanvasRef canvas)
	{
		const PatchesSPX& patches = m_dirtyRects[(int)canvas];

		int size = patches.size();
		const RectSPX* pRects = patches.begin();
		return std::tie(size, pRects);
	}

	//____ clearDirtyRects() _____________________________________________________

	void StreamPlayer::clearDirtyRects()
	{
		for (int i = 0; i < CanvasRef_size; i++)
			m_dirtyRects[i].clear();
	}

	//____ setCanvasInfoCallback() ________________________________________________

	void StreamPlayer::setCanvasInfoCallback( const std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)>& callback )
	{
		m_canvasInfoCallback = callback;
	}

	//____ _processStreamChunks() _____________________________________________

	void StreamPlayer::_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd)
	{
		m_pDecoder->setInput(pBegin, pEnd);

		while (_playChunk() == true);
	}


	//____ _playChunk() ____________________________________________________________

	bool StreamPlayer::_playChunk()
	{
		GfxStream::Header header;

		*m_pDecoder >> header;

		switch (header.type)
		{

		case GfxChunkId::OutOfData:
			return false;

		case GfxChunkId::ProtocolVersion:
		{
			uint16_t	version;
			*m_pDecoder >> version;

			//TODO: Something if version isn't supported.

			break;
		}

		case GfxChunkId::CanvasList:
		{
			uint16_t nbCanvases;

			*m_pDecoder >> nbCanvases;

			CanvasInfo	canvas[CanvasRef_size];

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

				canvas[i].ref = (CanvasRef) ref;
				canvas[i].size = size;
				canvas[i].scale = scale;
				canvas[i].format = format;
			}

			if( m_canvasInfoCallback )
				m_canvasInfoCallback(canvas, canvas+nbCanvases);
		}

        case GfxChunkId::BeginRender:
            m_pDevice->beginRender();
            break;

        case GfxChunkId::EndRender:
			assert(m_clipListBufferStack.empty() && m_clipListInfoStack.empty() && m_clipListBuffer.nRects == 0);
            m_pDevice->endRender();
            break;

        case GfxChunkId::Flush:
            m_pDevice->flush();
            break;

		case GfxChunkId::BeginCanvasUpdate:
		{
			uint16_t	surfaceId;
			CanvasRef	canvasRef;
			uint8_t		dummy;

			*m_pDecoder >> surfaceId;
 			*m_pDecoder >> canvasRef;
			*m_pDecoder >> dummy;

			int nUpdateRects = (header.size - 4) / 16;

			auto pRects = _pushClipListCanvas(nUpdateRects);
			
			if( surfaceId > 0 )
			{
				if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
				{
					GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "BeginCanvasUpdate with invalid surfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
					break;
				}
				
				m_pDevice->beginCanvasUpdate(m_vSurfaces[surfaceId], nUpdateRects, pRects );
			}
			else
				m_pDevice->beginCanvasUpdate(canvasRef, nUpdateRects, pRects );

			break;
		}

		case GfxChunkId::EndCanvasUpdate:
		{

			const CanvasInfo& canvasInfo = m_pDevice->canvas();

			if (m_bStoreDirtyRects && canvasInfo.ref != CanvasRef::None )
			{
				m_pDevice->resetClipList();								// Returns cliplist to that of canvas.

				int nDirtyRects = m_pDevice->clipListSize();
				const RectSPX* pDirtyRects = m_pDevice->clipList();

				PatchesSPX& patches = m_dirtyRects[(int)canvasInfo.ref];
				if (patches.isEmpty())
				{
					if (nDirtyRects > m_maxDirtyRects)
					{
						patches.clear();
						patches.push(m_pDevice->clipBounds());
					}
					else
					{
						for (int i = 0; i < nDirtyRects; i++)
							patches.push(pDirtyRects[i]);
					}
				}
				else
				{
					for (int i = 0; i < nDirtyRects; i++)
						patches.add(pDirtyRects[i]);

					if (patches.size() > m_maxDirtyRects)
					{
						RectSPX u = patches.bounds();
						patches.clear();
						patches.push(u);
					}
				}
			}

			m_pDevice->endCanvasUpdate();
			_popClipListCanvas();
			break;
		}

		case GfxChunkId::SetClipList:
		{
			int nRects = header.size / 16;

			auto pRects = _setClipList(nRects);

			m_pDevice->setClipList(nRects, pRects);
			break;
		}

		case GfxChunkId::ResetClipList:
			m_pDevice->resetClipList();
			break;

		case GfxChunkId::PushClipList:
		{
			int nRects = header.size / 16;

			auto pRects = _pushClipList(nRects);
			m_pDevice->pushClipList(nRects, pRects);
			break;
		}

		case GfxChunkId::PopClipList:
			_popClipList();
			m_pDevice->popClipList();
			break;

		case GfxChunkId::SetTintColor:
		{
			HiColor	col;
			*m_pDecoder >> col;
			m_pDevice->setTintColor(col);
			break;
		}

		case GfxChunkId::SetTintGradient:
		{
			RectI rect;
			Gradient gradient;

			*m_pDecoder >> rect;
			*m_pDecoder >> gradient;

			m_pDevice->setTintGradient(rect, gradient );
		   break;
		}

		case GfxChunkId::ClearTintGradient:
			m_pDevice->clearTintGradient();
			break;

		case GfxChunkId::SetBlendMode:
		{
			BlendMode	blendMode;
			*m_pDecoder >> blendMode;
			m_pDevice->setBlendMode(blendMode);
			break;
		}

		case GfxChunkId::SetBlitSource:
		{
			uint16_t	surfaceId;
			*m_pDecoder >> surfaceId;

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "SetBlitSource wiht invalid surfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_pDevice->setBlitSource(m_vSurfaces[surfaceId]);
			break;
		}

		case GfxChunkId::SetMorphFactor:
		{
			float	factor;

			*m_pDecoder >> factor;

			m_pDevice->setRenderLayer(factor);
			break;
		}

		case GfxChunkId::SetFixedBlendColor:
		{
			HiColor	col;

			*m_pDecoder >> col;

			m_pDevice->setFixedBlendColor(col);
			break;
		}

		case GfxChunkId::SetRenderLayer:
		{
			uint16_t	layer;

			*m_pDecoder >> layer;

			m_pDevice->setRenderLayer(layer);
			break;
		}

		case GfxChunkId::Fill:
		{
			HiColor	col;

			*m_pDecoder >> col;

			m_pDevice->fill(col);
			break;
		}

		case GfxChunkId::FillRect:
		{
			RectSPX	rect;
			HiColor	col;

			*m_pDecoder >> rect;
			*m_pDecoder >> col;

			m_pDevice->fill(rect, col);
			break;
		}

		case GfxChunkId::PlotPixels:
		{
			int nPixels = header.size / 16;

			int bufferSize = header.size;
			char * pBuffer = reinterpret_cast<char*>(GfxBase::memStackAlloc(bufferSize));

			// Load all data to buffer

			*m_pDecoder >> GfxStream::ReadBytes{ header.size, pBuffer };

			//

			m_pDevice->plotPixels(nPixels, (CoordI*)pBuffer, (HiColor*)(pBuffer + header.size / 2));

			GfxBase::memStackFree(bufferSize);
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

			m_pDevice->drawLine(begin, end, color, thickness.value);
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

			m_pDevice->drawLine(begin, dir, length, color, thickness);
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

				m_pDevice->drawLine(begin, dir, length, color, thickness);
				break;
			}


		case GfxChunkId::Blit:
		{
			CoordSPX	dest;

			*m_pDecoder >> dest;

			m_pDevice->blit(dest);
			break;
		}

		case GfxChunkId::BlitRect:
		{
			CoordSPX	dest;
			RectSPX		source;

			*m_pDecoder >> dest;
			*m_pDecoder >> source;

			m_pDevice->blit(dest,source);
			break;
		}

		case GfxChunkId::FlipBlit:
		{
			CoordSPX	dest;
			GfxFlip		flip;

			*m_pDecoder >> dest;
			*m_pDecoder >> flip;

			m_pDevice->flipBlit(dest,flip);
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

			m_pDevice->flipBlit(dest,source,flip);
			break;
		}

		case GfxChunkId::StretchBlit:
		{
			RectSPX		dest;

			*m_pDecoder >> dest;

			m_pDevice->stretchBlit(dest);
			break;
		}

		case GfxChunkId::StretchBlitRect:
		{
			RectSPX		dest;
			RectSPX		source;

			*m_pDecoder >> dest;
			*m_pDecoder >> source;

			m_pDevice->stretchBlit(dest, source);
			break;
		}

		case GfxChunkId::StretchFlipBlit:
		{
			RectSPX		dest;
			GfxFlip		flip;

			*m_pDecoder >> dest;
			*m_pDecoder >> flip;

			m_pDevice->stretchFlipBlit(dest, flip);
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

			m_pDevice->stretchFlipBlit(dest, source, flip);
			break;
		}

		case GfxChunkId::PrecisionBlit:
		{
			RectSPX		dest;
			RectF		source;

			*m_pDecoder >> dest;
			*m_pDecoder >> source;

			m_pDevice->precisionBlit(dest, source);
			break;
		}

		case GfxChunkId::TransformBlit:
		{
			RectSPX		dest;
			CoordF		source;
			Transform	transform;

			*m_pDecoder >> dest;
			*m_pDecoder >> source;
			*m_pDecoder >> transform.xx;
			*m_pDecoder >> transform.xy;
			*m_pDecoder >> transform.yx;
			*m_pDecoder >> transform.yy;

			m_pDevice->transformBlit(dest, source, transform);
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

			m_pDevice->rotScaleBlit(dest, rotationDegrees, scale, srcCenter, destCenter );
			break;
		}

		case GfxChunkId::Tile:
		{
			RectSPX		dest;
			CoordSPX	shift;

			*m_pDecoder >> dest;
			*m_pDecoder >> shift;

			m_pDevice->tile(dest, shift);
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

			m_pDevice->flipTile(dest, flip, shift);
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

			m_pDevice->scaleTile(dest, scale, shift);
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

			m_pDevice->scaleFlipTile(dest, scale, flip, shift);
			break;
		}

		case GfxChunkId::DrawWave:
		case GfxChunkId::FlipDrawWave:
		{
			m_drawTypeInProgress = header.type;

			*m_pDecoder >> m_wave.dest;
			*m_pDecoder >> m_wave.topBorder.length;
			*m_pDecoder >> m_wave.topBorder.thickness;
			*m_pDecoder >> m_wave.topBorder.color;
			*m_pDecoder >> m_wave.topBorder.hold;

			*m_pDecoder >> m_wave.bottomBorder.length;
			*m_pDecoder >> m_wave.bottomBorder.thickness;
			*m_pDecoder >> m_wave.bottomBorder.color;
			*m_pDecoder >> m_wave.bottomBorder.hold;

			*m_pDecoder >> m_wave.frontFill;
			*m_pDecoder >> m_wave.backFill;

			if (header.type == GfxChunkId::FlipDrawWave)
				*m_pDecoder >> m_wave.flip;

			int nTotalSamples = m_wave.topBorder.length + m_wave.bottomBorder.length;

			int bufferSize = sizeof(int) * nTotalSamples;

			m_pTempBuffer = new char[bufferSize];
			m_bytesLoaded = 0;
			m_bufferSize = bufferSize;

			m_wave.topBorder.pWave = (int*) m_pTempBuffer;
			m_wave.bottomBorder.pWave = reinterpret_cast<int*>(m_pTempBuffer) + m_wave.topBorder.length;
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


			m_pDevice->drawElipse(canvas, thickness, color, outlineThickness, outlineColor);
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

			m_pDevice->drawPieChart(canvas, start, nSlices, sliceSizes, sliceColors, hubSize, hubColor, backColor, bRectangular );
			break;
		}

		case GfxChunkId::DrawSegments:
		case GfxChunkId::FlipDrawSegments:
		{
			m_drawTypeInProgress = header.type;

			RectI		dest;
			uint16_t	nSegments;
			uint16_t	nEdgeStrips;
			GfxFlip		flip = GfxFlip::None;
			TintMode	tintMode;

			*m_pDecoder >> dest;
			*m_pDecoder >> nSegments;
			*m_pDecoder >> nEdgeStrips;
			if (header.type == GfxChunkId::FlipDrawSegments)
				*m_pDecoder >> flip;
			*m_pDecoder >> tintMode;

			int nTotalSamples = (nSegments - 1)*nEdgeStrips;

			int nColors;
			switch( tintMode )
			{
				case TintMode::None:
				case TintMode::Flat:
					nColors = nSegments;
					break;
				case TintMode::GradientX:
				case TintMode::GradientY:
					nColors = nSegments*2;
					break;
				case TintMode::GradientXY:
					nColors = nSegments*4;
					break;
			}

			int bufferSize = sizeof(HiColor)*nColors+sizeof(int)*nTotalSamples;

			m_pTempBuffer = new char[bufferSize];
			m_bytesLoaded = sizeof(HiColor)*nColors;
			m_bufferSize = bufferSize;

			m_seg.dest = dest;
			m_seg.nSegments = nSegments;
			m_seg.pSegmentColors = (HiColor*) m_pTempBuffer;
			m_seg.nEdgeStrips = nEdgeStrips;
			m_seg.pEdgeStrips = (int*) &m_pTempBuffer[sizeof(HiColor)*nColors];
			m_seg.edgeStripPitch = nSegments-1;
			m_seg.flip = flip;
			m_seg.tintMode = tintMode;

			for (int i = 0; i < nColors; i++)
				*m_pDecoder >> m_seg.pSegmentColors[i];

			break;
		}


		case GfxChunkId::EdgeSamples:
		{
			int nSPX = header.size / GfxStream::spxSize(header.spxFormat);

			assert(m_pTempBuffer != nullptr && nSPX*4-4 <= m_bufferSize - m_bytesLoaded);

			// Stream the compressed samples to end of destination and unpack them.

			
			
			if( m_bytesLoaded + nSPX*4 > m_bufferSize )
				nSPX--;											// Last chunk was padded with one byte for alignment.
						
			*m_pDecoder >> GfxStream::ReadSpxField{ nSPX, header.spxFormat, (spx*) &m_pTempBuffer[m_bytesLoaded] };
			m_pDecoder->align();

			// Increase counter and possibly render the segment

			m_bytesLoaded += nSPX*4;
			if (m_bytesLoaded == m_bufferSize)
			{
				switch (m_drawTypeInProgress)
				{
				case GfxChunkId::DrawSegments:
					m_pDevice->drawSegments(m_seg.dest, m_seg.nSegments, m_seg.pSegmentColors, m_seg.nEdgeStrips, m_seg.pEdgeStrips, m_seg.edgeStripPitch, m_seg.tintMode);
					break;
				case GfxChunkId::FlipDrawSegments:
					m_pDevice->flipDrawSegments(m_seg.dest, m_seg.nSegments, m_seg.pSegmentColors, m_seg.nEdgeStrips, m_seg.pEdgeStrips, m_seg.edgeStripPitch, m_seg.flip, m_seg.tintMode);
					break;
				case GfxChunkId::DrawWave:
					m_pDevice->drawWave(m_wave.dest, &m_wave.topBorder, &m_wave.bottomBorder, m_wave.frontFill, m_wave.backFill);
					break;
				case GfxChunkId::FlipDrawWave:
					m_pDevice->flipDrawWave(m_wave.dest, &m_wave.topBorder, &m_wave.bottomBorder, m_wave.frontFill, m_wave.backFill, m_wave.flip);
					break;
				default:
					assert(false);
				}

				delete[] m_pTempBuffer;
				m_pTempBuffer = nullptr;
				m_drawTypeInProgress = GfxChunkId::OutOfData;
			}
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

			m_pDevice->blitNinePatch(dstRect, dstFrame, patch, scale);
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

			bp.buffered = false;
			bp.palette = nullptr;

			if (header.size > 1024)
			{
				auto pPalette = (Color8*) GfxBase::memStackAlloc(1024);
				*m_pDecoder >> GfxStream::ReadBytes{ 1024, pPalette };
				bp.palette = pPalette;
			}

			if (m_vSurfaces.size() <= surfaceId)
				m_vSurfaces.resize(surfaceId + 16, nullptr);

			m_vSurfaces[surfaceId] = m_pSurfaceFactory->createSurface(bp);

			if (bp.palette)
				GfxBase::memStackFree(1024);

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

			bp.buffered = false;
			bp.palette = nullptr;
			
			if (bp.paletteSize > 0)
			{
				auto pPalette = (Color8*) GfxBase::memStackAlloc(bp.paletteSize*4);
				*m_pDecoder >> GfxStream::ReadBytes{ bp.paletteSize*4, pPalette };
				bp.palette = pPalette;
			}

			if (m_vSurfaces.size() <= surfaceId)
				m_vSurfaces.resize(surfaceId + 16, nullptr);
			else if( m_vSurfaces[surfaceId] != nullptr )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "CreateSurface with surfaceId that already is in use. The old surface will be replaced.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}
			
			m_vSurfaces[surfaceId] = m_pSurfaceFactory->createSurface(bp);

			if (bp.palette)
				GfxBase::memStackFree(bp.paletteSize*4);

			break;
		}

		case GfxChunkId::BeginSurfaceUpdate:
		{
			uint16_t	surfaceId;
			RectI		rect;

			*m_pDecoder >> surfaceId;
			*m_pDecoder >> rect;

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "BeginSurfaceUpdate with invalid SurfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			
			m_pUpdatingSurface = m_vSurfaces[surfaceId];
			m_pixelBuffer = m_pUpdatingSurface->allocPixelBuffer(rect);

			m_pWritePixels = m_pixelBuffer.pixels;

			m_surfaceBytesLeft = rect.w * rect.h * m_pUpdatingSurface->pixelBits()/8;
			break;
		}

		case GfxChunkId::SurfacePixels:
		{
            int line = int(m_pWritePixels - m_pixelBuffer.pixels) / m_pixelBuffer.pitch;
            int ofs = int(m_pWritePixels - m_pixelBuffer.pixels) % m_pixelBuffer.pitch;

            int bytesPerLine = m_pixelBuffer.rect.w * m_pUpdatingSurface->pixelBits()/8;

            int chunkBytesLeft = header.size;

			if( chunkBytesLeft > m_surfaceBytesLeft )
				chunkBytesLeft = m_surfaceBytesLeft;		// Last chunk was padded with an extra byte.

			m_surfaceBytesLeft -= chunkBytesLeft;


            while( chunkBytesLeft > 0 )
            {
                int toRead = std::min(chunkBytesLeft, bytesPerLine - ofs);
                *m_pDecoder >> GfxStream::ReadBytes{ toRead, m_pWritePixels };
                chunkBytesLeft -= toRead;

                if(toRead + ofs == bytesPerLine)
                {
                    m_pWritePixels += m_pixelBuffer.pitch - ofs;
                    ofs = 0;
                }
                else
                {
                    m_pWritePixels += toRead;
                    ofs += toRead;
                }

            }
			m_pDecoder->align();
			break;
		}

		case GfxChunkId::EndSurfaceUpdate:
		{
			assert(m_surfaceBytesLeft == 0);

			m_pUpdatingSurface->pullPixels(m_pixelBuffer);
			m_pUpdatingSurface->freePixelBuffer(m_pixelBuffer);
			m_pUpdatingSurface = nullptr;
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

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "FillSurface with invalid surfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_vSurfaces[surfaceId]->fill(region, col);
			break;
		}
/*
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

			Surface * pDest	  = m_vSurfaces[destSurfaceId];
			Surface * pSource = m_vSurfaces[sourceSurfaceId];

			pDest->copyFrom(pSource, sourceRect, dest);
			break;
		}
*/
		case GfxChunkId::DeleteSurface:
		{
			uint16_t	surfaceId;

			*m_pDecoder >> surfaceId;

			if( surfaceId > m_vSurfaces.size() || m_vSurfaces[surfaceId] == nullptr )
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "DeleteSurface with invalid surfaceId", this, &TYPEINFO, __func__, __FILE__, __LINE__);
				break;
			}

			m_vSurfaces[surfaceId] = nullptr;
			break;
		}


		case GfxChunkId::CreateEdgemap:
		{

			uint16_t	edgemapId;
			SizeI		size;
			uint16_t	nbSegments;
			bool		bHasColors;
			bool		bHasGradients;

			*m_pDecoder >> edgemapId;
			*m_pDecoder >> size;
			*m_pDecoder >> nbSegments;
			*m_pDecoder >> bHasColors;
			*m_pDecoder >> bHasGradients;

			int nbColors = bHasColors ? nbSegments : 0;
			int nbGradients = bHasGradients ? nbSegments : 0;

			int memSize = nbColors * sizeof(HiColor) + nbGradients * sizeof(Gradient);
			
			HiColor * pColors = nullptr;
			Gradient * pGradients = nullptr;
			
			if( memSize > 0 )
			{
				char * pMem = (char *) GfxBase::memStackAlloc(memSize);
				
				if( bHasColors )
				{
					pColors = (HiColor*) pMem;
					for( int i = 0 ; i < nbSegments ; i++ )
						*m_pDecoder >> pColors[i];

					pMem += nbColors * sizeof(HiColor);
				}
				
				if( bHasGradients )
				{
					pGradients = (Gradient*) pMem;
					for( int i = 0 ; i < nbSegments ; i++ )
						*m_pDecoder >> pGradients[i];
				}
			}

			if (m_vEdgemaps.size() <= edgemapId)
				m_vEdgemaps.resize(edgemapId + 16, nullptr);

			Edgemap::Blueprint		bp;
			
			bp.size = size;
			bp.colors = pColors;
			bp.gradients = pGradients;
			bp.segments = nbSegments;
			
			m_vEdgemaps[edgemapId] = m_pEdgemapFactory->createEdgemap(bp);

			GfxBase::memStackFree(memSize);
						
			break;
		}

		case GfxChunkId::SetEdgemapRenderSegments:
		{
			uint16_t	edgemapId;
			uint16_t	nbSegments;

			*m_pDecoder >> edgemapId;
			*m_pDecoder >> nbSegments;

			m_vEdgemaps[edgemapId]->setRenderSegments(nbSegments);
			
			break;
		}

			case GfxChunkId::SetEdgemapColors:
			{
				uint16_t	edgemapId;
				uint16_t	begin;
				uint16_t	end;
				
				*m_pDecoder >> edgemapId;
				*m_pDecoder >> begin;
				*m_pDecoder >> end;

				int nColors = end - begin;

				int memAllocated = sizeof(HiColor)*nColors;
				auto * pColors = (HiColor*) GfxBase::memStackAlloc(memAllocated);
				
				for( int i = 0 ; i < nColors ; i++ )
					*m_pDecoder >> pColors[i];

				m_vEdgemaps[edgemapId]->setColors(begin, end, pColors);
				GfxBase::memStackFree(memAllocated);
				break;
			}
				
			case GfxChunkId::SetEdgemapGradients:
			{
				uint16_t	edgemapId;
				uint16_t	begin;
				uint16_t	end;
				
				*m_pDecoder >> edgemapId;
				*m_pDecoder >> begin;
				*m_pDecoder >> end;

				int nGradients = end - begin;

				int memAllocated = sizeof(Gradient)*nGradients;
				auto * pGradients = (Gradient*) GfxBase::memStackAlloc(memAllocated);
				
				for( int i = 0 ; i < nGradients ; i++ )
					*m_pDecoder >> pGradients[i];

				m_vEdgemaps[edgemapId]->setGradients(begin, end, pGradients);
				GfxBase::memStackFree(memAllocated);
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

			int nEdges = edgeEnd - edgeBegin;
			int nSamples = sampleEnd - sampleBegin;
			
			m_pUpdatingEdgemap = m_vEdgemaps[edgemapId];
			m_pWaveSampleBuffer = new spx[nEdges*nSamples+1];

			m_pWaveWriteSamples = m_pWaveSampleBuffer;

			m_waveSamplesLeft = nEdges*nSamples;

			m_waveUpdateEdgeBegin 	= edgeBegin;
			m_waveUpdateEdgeEnd 	= edgeEnd;
			m_waveUpdateSampleBegin	= sampleBegin;
			m_waveUpdateSampleEnd	= sampleEnd;

			break;
		}
				
		case GfxChunkId::EdgemapSamples:
		{
			int bytes = header.size;

			int nbSamples = bytes / GfxStream::spxSize(header.spxFormat);
			
			if( nbSamples > m_waveSamplesLeft )
				nbSamples = m_waveSamplesLeft;
			
			m_waveSamplesLeft -= nbSamples;
			
			*m_pDecoder >> GfxStream::ReadSpxField{ nbSamples, header.spxFormat, m_pWaveWriteSamples };
			m_pWaveWriteSamples += nbSamples;
			
			m_pDecoder->align();
			break;
		}

		case GfxChunkId::EndEdgemapUpdate:
		{
			m_pUpdatingEdgemap->importSamples(SampleOrigo::Top, m_pWaveSampleBuffer, m_waveUpdateEdgeBegin, m_waveUpdateEdgeEnd, m_waveUpdateSampleBegin, m_waveUpdateSampleEnd);
						
			m_pUpdatingEdgemap = nullptr;
			
			delete [] m_pWaveSampleBuffer;
			m_pWaveSampleBuffer = nullptr;
			break;
		}
				

		case GfxChunkId::DeleteEdgemap:
		{
			uint16_t	edgemapId;

			*m_pDecoder >> edgemapId;

			m_vEdgemaps[edgemapId] = nullptr;
			break;
		}

		case GfxChunkId::DrawEdgemap:
		{
			CoordSPX	dest;
			uint16_t	edgemapId;

			*m_pDecoder >> dest;
			*m_pDecoder >> edgemapId;

			m_pDevice->drawEdgemap(dest, m_vEdgemaps[edgemapId]);
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

			m_pDevice->flipDrawEdgemap(dest, m_vEdgemaps[edgemapId], flip);
			break;
		}
				
				
		default:
			// We don't know how to handle this, so let's just skip it

			m_pDecoder->skip(header.size);
			break;
		}

		return true;
	}

	//____ reset() _______________________________________________________________

	void StreamPlayer::reset()
	{
		m_vSurfaces.clear();
	}


	//____ _pushClipListCanvas() ____________________________________________________

	RectI * StreamPlayer::_pushClipListCanvas(int nRects)
	{
		m_clipListInfoStack.push_back(m_clipList);
		m_clipList.bCanvas = false;
		RectI * pRects = _setClipList(nRects, true);
		m_clipList.bCanvas = true;
		return pRects;
	}

	//____ _popClipListCanvas() _____________________________________________________

	void StreamPlayer::_popClipListCanvas()
	{
		while( !m_clipList.bCanvas )
			_popClipList();

		_popClipList();
	}

	//____ _pushClipList() ____________________________________________________

	RectI * StreamPlayer::_pushClipList(int nRects)
	{
		m_clipListInfoStack.push_back(m_clipList);
		m_clipList.bCanvas = false;
		return _setClipList(nRects, true);
	}

	//____ _setClipList() _____________________________________________________

	RectI * StreamPlayer::_setClipList(int nRects, bool bPush)
	{
		assert(nRects <= c_clipListBufferSize);

		// We overwrite previous clipList unless we start a new canvas or it is the
		// cliplist of the canvas.

		if( m_clipList.bCanvas )
			m_clipListInfoStack.push_back(m_clipList);		// Force an extra push so ClipListInfo for our canvas is preserved.

		if( !bPush && !m_clipList.bCanvas )
			m_clipListBuffer.nRects -= m_clipList.nRects;

		// Reserve a new buffer if needed

		if (m_clipListBuffer.capacity < m_clipListBuffer.nRects + nRects)
		{
			m_clipListBufferStack.push_back(m_clipListBuffer);

			m_clipListBuffer.pRects = new RectI[c_clipListBufferSize];
			m_clipListBuffer.nRects = 0;
			m_clipListBuffer.capacity = c_clipListBufferSize;
		}

		// Puts values in clipList-buffer

		auto pRects = m_clipListBuffer.pRects + m_clipListBuffer.nRects;

		*m_pDecoder >> GfxStream::ReadBytes{ nRects * 16, pRects };

		m_clipListBuffer.nRects += nRects;
		m_clipList.nRects = nRects;
		m_clipList.bCanvas = false;
		return pRects;
	}

	//____ _popClipList() _____________________________________________________

	void StreamPlayer::_popClipList()
	{
		assert(m_clipListBuffer.nRects >= m_clipList.nRects);

		if (m_clipListBuffer.nRects == m_clipList.nRects && !m_clipListBufferStack.empty() )
		{
			delete[] m_clipListBuffer.pRects;
			m_clipListBuffer = m_clipListBufferStack.back();
			m_clipListBufferStack.pop_back();
		}
		else
		{
			m_clipListBuffer.nRects -= m_clipList.nRects;
		}

		m_clipList = m_clipListInfoStack.back();
		m_clipListInfoStack.pop_back();
	}


} //namespace wg
