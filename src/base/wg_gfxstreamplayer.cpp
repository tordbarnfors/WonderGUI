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

#include <wg_gfxstreamplayer.h>
#include <wg_base.h>
#include <assert.h>

namespace wg
{

	const TypeInfo GfxStreamPlayer::TYPEINFO = { "GfxStreamPlayer", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	GfxStreamPlayer_p GfxStreamPlayer::create(CGfxInStream& in, GfxDevice * pDevice, SurfaceFactory * pFactory)
	{
		return new GfxStreamPlayer(in, pDevice, pFactory);
	}

	//____ constructor _____________________________________________________________

	GfxStreamPlayer::GfxStreamPlayer(CGfxInStream& in, GfxDevice * pDevice, SurfaceFactory * pFactory)
	{
		m_pStream = in.ptr();
		m_pDevice = pDevice;
		m_pSurfaceFactory = pFactory;
	}

	//____ Destructor _________________________________________________________

	GfxStreamPlayer::~GfxStreamPlayer()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GfxStreamPlayer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ isEmpty() __________________________________________________________

	bool GfxStreamPlayer::isEmpty() const
	{
		return m_pStream->isEmpty();
	}

	//____ peekChunk() ________________________________________________________

	GfxStream::Header GfxStreamPlayer::peekChunk() const
	{
		return m_pStream->peek();
	}

	//____ playAll() ___________________________________________________________

	void GfxStreamPlayer::playAll()
	{
		while (playChunk() == true);
	}

	//____ playChunk() ____________________________________________________________

	bool GfxStreamPlayer::playChunk()
	{
		GfxStream::Header header;

		*m_pStream >> header;

		switch (header.type)
		{

		case GfxChunkId::OutOfData:
			return false;

		case GfxChunkId::BeginRender:
			m_pDevice->beginRender();
			break;

		case GfxChunkId::EndRender:
			m_pDevice->endRender();
			break;

		case GfxChunkId::SetCanvas:
		{
			uint16_t	surfaceId;
			uint16_t	bResetClipList;
			*m_pStream >> surfaceId;
			*m_pStream >> bResetClipList;

			if( surfaceId > 0 )
				m_pDevice->setCanvas(m_vSurfaces[surfaceId], (bool) bResetClipList );
			else
				m_pDevice->setCanvas(nullptr, (bool) bResetClipList);

			break;
		}

		case GfxChunkId::SetClip:
		{
			int nRectangles = header.size / 8;

			for (int i = 0; i < nRectangles; i++)
				*m_pStream >> m_clipRects[i];

			m_pDevice->setClipList(nRectangles, m_clipRects);
			break;
		}

		case GfxChunkId::SetTintColor:
		{
			Color	col;
			*m_pStream >> col;
			m_pDevice->setTintColor(col);
			break;
		}

		case GfxChunkId::SetBlendMode:
		{
			BlendMode	blendMode;
			*m_pStream >> blendMode;
			m_pDevice->setBlendMode(blendMode);
			break;
		}

		case GfxChunkId::SetBlitSource:
		{
			uint16_t	surfaceId;
			*m_pStream >> surfaceId;

			m_pDevice->setBlitSource(m_vSurfaces[surfaceId]);
			break;
		}

		case GfxChunkId::Fill:
		{
			RectI	rect;
			Color	col;

			*m_pStream >> rect;
			*m_pStream >> col;

			m_pDevice->fill(rect, col);
			break;
		}

		case GfxChunkId::FillSubpixel:
		{
			RectF	rect;
			Color	col;

			*m_pStream >> rect;
			*m_pStream >> col;

			m_pDevice->fill(rect, col);
			break;
		}

		case GfxChunkId::PlotPixels:
		{
			int nPixels = header.size / 8;

			int bufferSize = header.size*3/2;			// Pixels needs to be expanded, but not colors
			char * pBuffer = reinterpret_cast<char*>(Base::memStackAlloc(bufferSize));

			// Load all data to end of buffer

			*m_pStream >> GfxStream::DataChunk{ header.size, pBuffer + bufferSize - header.size };

			// Unpack Coordinates

			CoordI * pDest = (CoordI*)pBuffer;
			int16_t * pSrc = (int16_t*) (pBuffer + bufferSize - header.size);

			for (int i = 0; i < nPixels; i++)
			{
				pDest[i].x = *pSrc++;
				pDest[i].y = *pSrc++;
			}

			//

			m_pDevice->plotPixels(nPixels, (CoordI*)pBuffer, (Color*)(pBuffer + header.size / 2));

			Base::memStackRelease(bufferSize);
			break;
		}

		case GfxChunkId::DrawLineFromTo:
		{
			CoordI	begin;
			CoordI	end;
			Color	color;
			float	thickness;

			*m_pStream >> begin;
			*m_pStream >> end;
			*m_pStream >> color;
			*m_pStream >> thickness;

			m_pDevice->drawLine(begin, end, color, thickness);
			break;
		}

		case GfxChunkId::DrawLineStraight:
		{
			CoordI		begin;
			Direction	dir;
			uint16_t	length;
			Color		color;
			float		thickness;

			*m_pStream >> begin;
			*m_pStream >> dir;
			*m_pStream >> length;
			*m_pStream >> color;
			*m_pStream >> thickness;

			m_pDevice->drawLine(begin, dir, length, color, thickness);
			break;
		}

		case GfxChunkId::Blit:
		{
			CoordI		dest;
			RectI		source;

			*m_pStream >> dest;
			*m_pStream >> source;

			m_pDevice->blit(dest,source);
			break;
		}

		case GfxChunkId::StretchBlit:
		{
			RectI		dest;
			RectF		source;

			*m_pStream >> dest;
			*m_pStream >> source;

			m_pDevice->stretchBlit(dest, source);
			break;
		}

		case GfxChunkId::SimpleTransformBlit:
		{
			RectI dest;
			CoordI src;
			int transform[2][2];

			*m_pStream >> dest;
			*m_pStream >> src;
			*m_pStream >> transform;

			m_pDevice->_transformBlit(dest, src, transform);
			break;
		}

		case GfxChunkId::ComplexTransformBlit:
		{
			RectI dest;
			CoordF src;
			float transform[2][2];

			*m_pStream >> dest;
			*m_pStream >> src;
			*m_pStream >> transform;

			m_pDevice->_transformBlit(dest, src, transform);
			break;
		}

		case GfxChunkId::TransformDrawSegments:
		{
			RectI		dest;
			uint16_t	nSegments;
			uint16_t	nEdgeStrips;

			*m_pStream >> dest;
			*m_pStream >> nSegments;
			*m_pStream >> nEdgeStrips;
			*m_pStream >> m_seg.transform;

			for (int i = 0; i < nSegments; i++)
				* m_pStream >> m_seg.colors[i];

			int nTotalSamples = (nSegments - 1)*nEdgeStrips;

			m_seg.dest = dest;
			m_seg.nSegments = nSegments;
			m_seg.nEdgeStrips = nEdgeStrips;
			m_seg.pEdgeStrips = new int[nTotalSamples];
			m_seg.nLoadedSamples = 0;
			m_seg.nTotalSamples = nTotalSamples;

			break;
		}

		case GfxChunkId::EdgeSamples:
		{
			int nSamples = header.size/2;

			assert(nSamples <= m_seg.nTotalSamples - m_seg.nLoadedSamples);

			// Stream the compressed samples to end of destination and unpack them.

			*m_pStream >> GfxStream::DataChunk{ header.size, ((char*)(&m_seg.pEdgeStrips[m_seg.nLoadedSamples])) + nSamples*2 };

			int * wp = &m_seg.pEdgeStrips[m_seg.nLoadedSamples];
			int16_t * rp = ((int16_t*)&m_seg.pEdgeStrips[m_seg.nLoadedSamples]) + nSamples;

			for (int i = 0; i < nSamples; i++)
				* wp++ = ((int)(*rp++)) << 4;

			// Increase counter and possibly render the segment

			m_seg.nLoadedSamples += nSamples;
			if (m_seg.nLoadedSamples == m_seg.nTotalSamples)
			{
				m_pDevice->_transformDrawSegments(m_seg.dest, m_seg.nSegments, m_seg.colors, m_seg.nEdgeStrips, m_seg.pEdgeStrips, (m_seg.nSegments - 1), m_seg.transform);
				delete [] m_seg.pEdgeStrips;
			}
			break;
		}

		case GfxChunkId::CreateSurface:
		{
			uint16_t	surfaceId;
			PixelFormat	type;
			SizeI		size;
			uint16_t	flags;

			*m_pStream >> surfaceId;
			*m_pStream >> type;
			*m_pStream >> size;
			*m_pStream >> flags;


			Color * pClut = nullptr;

			if (header.size > 4096)
			{
				pClut = (Color*) Base::memStackAlloc(4096);
				*m_pStream >> GfxStream::DataChunk{ 4096, pClut };
			}

			if (m_vSurfaces.size() <= surfaceId)
				m_vSurfaces.resize(surfaceId + 1, nullptr);

			m_vSurfaces[surfaceId] = m_pSurfaceFactory->createSurface(size, type, flags | SurfaceFlag::WriteOnly, pClut);

			if (pClut)
				Base::memStackRelease(4096);

			break;
		}

		case GfxChunkId::DeleteSurface:
		{
			uint16_t	surfaceId;

			*m_pStream >> surfaceId;

			m_vSurfaces[surfaceId] = nullptr;
			break;
		}

		case GfxChunkId::SetSurfaceScaleMode:
		{
			uint16_t	surfaceId;
			ScaleMode	scaleMode;

			*m_pStream >> surfaceId;
			*m_pStream >> scaleMode;

			m_vSurfaces[surfaceId]->setScaleMode(scaleMode);
			break;
		}

		case GfxChunkId::BeginSurfaceUpdate:
		{
			uint16_t	surfaceId;
			RectI		rect;

			*m_pStream >> surfaceId;
			*m_pStream >> rect;

			m_pUpdatingSurface = m_vSurfaces[surfaceId];
			m_pWritePixels = m_pUpdatingSurface->lockRegion(AccessMode::WriteOnly, rect);
			break;
		}

		case GfxChunkId::SurfaceData:
		{
			*m_pStream >> GfxStream::DataChunk{ header.size, m_pWritePixels };
			m_pWritePixels += header.size;
			break;
		}

		case GfxChunkId::EndSurfaceUpdate:
		{
			m_pUpdatingSurface->unlock();
			m_pUpdatingSurface = nullptr;
			break;
		}

		case GfxChunkId::FillSurface:
		{
			uint16_t	surfaceId;
			RectI		region;
			Color		col;

			*m_pStream >> surfaceId;
			*m_pStream >> region;
			*m_pStream >> col;

			m_vSurfaces[surfaceId]->fill(col, region);
			break;
		}

		case GfxChunkId::CopySurface:
		{
			uint16_t	destSurfaceId;
			uint16_t	sourceSurfaceId;
			RectI		sourceRect;
			CoordI		dest;

			*m_pStream >> destSurfaceId;
			*m_pStream >> sourceSurfaceId;
			*m_pStream >> sourceRect;
			*m_pStream >> dest;

			Surface * pDest	  = m_vSurfaces[destSurfaceId];
			Surface * pSource = m_vSurfaces[sourceSurfaceId];

			pDest->copyFrom(pSource, sourceRect, dest);
			break;
		}

		default:
			// We don't know how to handle this, so let's just skip it

			m_pStream->skip(header.size);
			break;
		}

		return true;
	}

	//____ playFrame() _________________________________________________________

	bool GfxStreamPlayer::playFrame()
	{
		GfxStream::Header	header = peekChunk();

		while (header.type != GfxChunkId::OutOfData)
		{
			playChunk();
			if (header.type == GfxChunkId::EndRender)
				return true;
			header = peekChunk();
		}
		return false;
	}


} //namespace wg
