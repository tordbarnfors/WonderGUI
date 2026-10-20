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
#include <wg_edgemapwave.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

#include <algorithm>
#include <cstring>

namespace wg
{
	const TypeInfo EdgemapWave::TYPEINFO = { "EdgemapWave", &EdgemapShape::TYPEINFO };


	//.____ create() _____________________________________________________________

	EdgemapWave_p EdgemapWave::create( const Blueprint& blueprint )
	{
		return EdgemapWave_p( new EdgemapWave(blueprint) );
	}

	//____ constructor ___________________________________________________________

	EdgemapWave::EdgemapWave( const Blueprint& bp )
	: 	m_color(bp.color),
		m_outlineColor(bp.outlineColor),
		m_pTint(bp.tint),
		m_bOptimizeDirtRange(bp.optimize),
		m_pOutlineTint(bp.outlineTint),
		m_origo(bp.origo),
		m_pFactory(bp.factory),
		m_size(Util::roundToPixels(bp.size))
	{
		m_nbSamples = m_size.w + 1;

		m_pieces.resize(1);
		m_pieces[0].pos = Util::roundToPixels(bp.pos) * 64;
		m_pieces[0].flip = bp.flip;

		m_bHasOutlines = bp.topOutlineThickness > 0 || bp.bottomOutlineThickness > 0;

		m_topBrush.thickness = std::clamp(bp.topOutlineThickness, 0, 64*64);
		m_bottomBrush.thickness = std::clamp(bp.bottomOutlineThickness, 0, 64*64);

		// Generate brush slope

		if( m_bHasOutlines )
		{
			_generateInfluenceSlope( m_topBrush );
			_generateInfluenceSlope( m_bottomBrush );
		}

		//

		m_topSamplesPadding = (bp.topOutlineThickness/2 / 64) + 1;
		m_bottomSamplesPadding = (bp.bottomOutlineThickness/2 / 64) + 1;

		_setupBuffer(m_nbSamples, m_bOptimizeDirtRange);

		_regenEdgemap();

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ destructor ____________________________________________________________

	EdgemapWave::~EdgemapWave()
	{
		delete [] m_pBuffer;
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& EdgemapWave::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setSize() ______________________________________________________________

	bool EdgemapWave::setSize( SizeSPX _size )
	{
		SizeI size = Util::roundToPixels(_size);

		if( size.w <= 0 || size.h <= 0 )
		{
			return false;
		}

		if( size == m_size )
			return true;

		_setupBuffer(size.w + 1, m_bOptimizeDirtRange);
		m_size = size;

		_regenEdgemap();

		return true;
	}

	//____ setPos() _______________________________________________________________
	/**
	 * @brief Set the top-left corner of the wave on the canvas, after flip.
	 *
	 * The position is rounded to whole pixels.
	 */

	void EdgemapWave::setPos( CoordSPX pos )
	{
		m_pieces[0].pos = Util::roundToPixels(pos) * 64;
	}

	//____ setFlip() ______________________________________________________________
	/**
	 * @brief Set how the wave is flipped when drawn.
	 *
	 * The wave runs along x before it is flipped, so a flip that rotates it by 90 or 270
	 * degrees makes it run along y. The position stays the top-left corner.
	 */

	void EdgemapWave::setFlip( GfxFlip flip )
	{
		m_pieces[0].flip = flip;
	}

	//____ _setupBuffer() _______________________________________________________

	void EdgemapWave::_setupBuffer(int nbSamples, bool bClearSamples)
	{
		delete [] m_pBuffer;

		int sampleBufferSize = ((nbSamples+m_topSamplesPadding*2)+(nbSamples+m_bottomSamplesPadding*2))*sizeof(spx);

		m_pBuffer = new uint8_t[sampleBufferSize];
		uint8_t * pBuffer = m_pBuffer;

		spx * pSamples = (spx*) pBuffer;

		m_pTopSamples = pSamples + m_topSamplesPadding;
		m_pBottomSamples = pSamples + (nbSamples + m_topSamplesPadding*2) + m_bottomSamplesPadding;

		m_nbSamples = nbSamples;

		m_dirtBegin	= 0;
		m_dirtEnd 	= m_nbSamples;

		if( bClearSamples )
			memset( pSamples, 0, sampleBufferSize);
	}

	//____ setColor() _____________________________________________________________

	bool EdgemapWave::setColor( HiColor col )
	{
		if( !col.isValid() )
			return false;

		m_color = col;
		_updateEdgemapColors();
		return true;
	}


	//____ setTint() ___________________________________________________________
	/**
	 * @brief Set a Tint for the fill, overriding the fill color.
	 *
	 * The Tint is placed in the rectangle of the wave, before flip.
	 */

	void EdgemapWave::setTint( Tint * pTint )
	{
		m_pTint = pTint;
		_updateEdgemapColors();
	}

	//____ setOutlineColor() _____________________________________________________

	bool EdgemapWave::setOutlineColor( HiColor col )
	{
		if( !col.isValid() )
			return false;

		m_outlineColor = col;
		_updateEdgemapColors();
		return true;
	}

	//____ setOutlineTint() ____________________________________________________
	/**
	 * @brief Set a Tint for the outlines, overriding the outline color.
	 *
	 * The Tint is placed in the rectangle of the wave, before flip.
	 */

	void EdgemapWave::setOutlineTint( Tint * pTint )
	{
		m_pOutlineTint = pTint;
		_updateEdgemapColors();
	}


	//____ setSamples() __________________________________________________________

	void EdgemapWave::setSamples( int sampleBegin, int sampleEnd, spx * pTopSamples, spx * pBottomSamples )
	{
		limit(sampleBegin, 0, m_nbSamples);
		limit(sampleEnd, sampleBegin, m_nbSamples);

		if( sampleBegin == sampleEnd )
			return;

		if( m_bOptimizeDirtRange )
		{
			if( pTopSamples )
				_optimizeRangeSetSamples(m_pTopSamples + sampleBegin, m_pTopSamples + sampleEnd, pTopSamples, m_pTopSamples);

			if( pBottomSamples )
				_optimizeRangeSetSamples(m_pBottomSamples + sampleBegin, m_pBottomSamples + sampleEnd, pBottomSamples, m_pBottomSamples);
		}
		else
		{
			if( pTopSamples )
				Edgemap::convertSamples(m_pTopSamples + sampleBegin, pTopSamples, m_origo, m_size.h*64, 1, sampleEnd-sampleBegin, 0, 1, 0, 1);

			if( pBottomSamples )
				Edgemap::convertSamples(m_pBottomSamples + sampleBegin, pBottomSamples, m_origo, m_size.h*64, 1, sampleEnd-sampleBegin, 0, 1, 0, 1);

			m_dirtBegin = std::min(sampleBegin,m_dirtBegin);
			m_dirtEnd = std::max(sampleEnd,m_dirtEnd);
		}
	}

	void EdgemapWave::setSamples( int sampleBegin, int sampleEnd, float * pTopSamples, float * pBottomSamples )
	{
		limit(sampleBegin, 0, m_nbSamples);
		limit(sampleEnd, sampleBegin, m_nbSamples);

		if( sampleBegin == sampleEnd )
			return;

		if( m_bOptimizeDirtRange )
		{
			if( pTopSamples )
				_optimizeRangeSetSamples(m_pTopSamples + sampleBegin, m_pTopSamples + sampleEnd, pTopSamples, m_pTopSamples);

			if( pBottomSamples )
				_optimizeRangeSetSamples(m_pBottomSamples + sampleBegin, m_pBottomSamples + sampleEnd, pBottomSamples, m_pBottomSamples);
		}
		else
		{
			if( pTopSamples )
				Edgemap::convertSamples(m_pTopSamples + sampleBegin, pTopSamples, m_origo, m_size.h*64, 1, sampleEnd-sampleBegin, 0, 1, 0, 1);

			if( pBottomSamples )
				Edgemap::convertSamples(m_pBottomSamples + sampleBegin, pBottomSamples, m_origo, m_size.h*64, 1, sampleEnd-sampleBegin, 0, 1, 0, 1);

			m_dirtBegin = std::min(sampleBegin,m_dirtBegin);
			m_dirtEnd = std::max(sampleEnd,m_dirtEnd);
		}
	}

	//____ _optimizeRangeSetSamples() ____________________________________________

	void EdgemapWave::_optimizeRangeSetSamples( spx * _pDestBegin, spx * _pDestEnd, const spx * pSource, spx * pSampleStart )
	{
		spx * pDestBegin = _pDestBegin;
		spx * pDestEnd = _pDestEnd;

		int nbSamples = int(pDestEnd - pDestBegin);
		int allocBytes= nbSamples * sizeof(spx);
		spx * pConverted = (spx*) GfxBase::memStackAlloc(allocBytes);
		Edgemap::convertSamples(pConverted, pSource, m_origo, m_size.h*64, 1, nbSamples, 0, 1, 0, 1);

		int bufferBeg = 0;
		int bufferEnd = nbSamples;

		while( pDestBegin < pDestEnd && pConverted[bufferBeg] == pDestBegin[0] )
		{
			pDestBegin++;
			bufferBeg++;
		}

		while( pDestBegin < pDestEnd && pConverted[bufferEnd-1] == pDestEnd[-1] )
		{
			pDestEnd--;
			bufferEnd--;
		}

		if( pDestEnd > pDestBegin )
		{
			memcpy( pDestBegin, pConverted + bufferBeg, (pDestEnd-pDestBegin)*sizeof(spx) );

			m_dirtBegin = std::min(int(pDestBegin - pSampleStart),m_dirtBegin);
			m_dirtEnd = std::max(int(pDestEnd - pSampleStart),m_dirtEnd);
		}

		GfxBase::memStackFree(allocBytes);
	}

	void EdgemapWave::_optimizeRangeSetSamples( spx * _pDestBegin, spx * _pDestEnd, const float * pSource, spx * pSampleStart )
	{
		spx * pDestBegin = _pDestBegin;
		spx * pDestEnd = _pDestEnd;

		int nbSamples = int(pDestEnd - pDestBegin);
		int allocBytes= nbSamples * sizeof(spx);
		spx * pConverted = (spx*) GfxBase::memStackAlloc(allocBytes);
		Edgemap::convertSamples(pConverted, pSource, m_origo, m_size.h*64, 1, nbSamples, 0, 1, 0, 1);

		int bufferBeg = 0;
		int bufferEnd = nbSamples;

		while( pDestBegin < pDestEnd && pConverted[bufferBeg] == pDestBegin[0] )
		{
			pDestBegin++;
			bufferBeg++;
		}

		while( pDestBegin < pDestEnd && pConverted[bufferEnd-1] == pDestEnd[-1] )
		{
			pDestEnd--;
			bufferEnd--;
		}

		if( pDestEnd > pDestBegin )
		{
			memcpy( pDestBegin, pConverted + bufferBeg, (pDestEnd-pDestBegin)*sizeof(spx) );

			m_dirtBegin = std::min(int(pDestBegin - pSampleStart),m_dirtBegin);
			m_dirtEnd = std::max(int(pDestEnd - pSampleStart),m_dirtEnd);
		}

		GfxBase::memStackFree(allocBytes);
	}


	//____ setFlatTopLine() ______________________________________________________

	void EdgemapWave::setFlatTopLine( int sampleBegin, int sampleEnd, spx sample )
	{
		spx out = Edgemap::convertSample(sample, m_origo, m_size.h*64);

		_setFlatLine(m_pTopSamples, sampleBegin, sampleEnd, out);
	}

	void EdgemapWave::setFlatTopLine( int sampleBegin, int sampleEnd, float sample )
	{
		spx out = Edgemap::convertSample(sample, m_origo, m_size.h*64);

		_setFlatLine(m_pTopSamples, sampleBegin, sampleEnd, out);
	}

	//____ setFlatBottomLine() ___________________________________________________

	void EdgemapWave::setFlatBottomLine( int sampleBegin, int sampleEnd, spx sample )
	{
		spx out = Edgemap::convertSample(sample, m_origo, m_size.h*64);

		_setFlatLine(m_pBottomSamples, sampleBegin, sampleEnd, out);
	}

	void EdgemapWave::setFlatBottomLine( int sampleBegin, int sampleEnd, float sample )
	{
		spx out = Edgemap::convertSample(sample, m_origo, m_size.h*64);

		_setFlatLine(m_pBottomSamples, sampleBegin, sampleEnd, out);
	}

	//____ refresh() _____________________________________________________________

	void EdgemapWave::refresh()
	{
		if( m_dirtEnd - m_dirtBegin <= 0  )
			return;

		Edgemap * pEdgemap = m_pieces[0].pEdgemap;

		// Copy first/last samples to padding if changed.

		if( m_dirtBegin == 0 )
		{
			spx value = m_pTopSamples[0];
			spx * p = m_pTopSamples - m_topSamplesPadding;
			while( p != m_pTopSamples )
				* p++ = value;

			value = m_pBottomSamples[0];
			p = m_pBottomSamples - m_bottomSamplesPadding;
			while( p != m_pBottomSamples )
				* p++ = value;
		}

		if( m_dirtEnd == m_nbSamples )
		{
			spx value = m_pTopSamples[m_nbSamples-1];
			spx * p = m_pTopSamples + m_nbSamples + m_topSamplesPadding -1;
			while( p >= m_pTopSamples + m_nbSamples )
				* p-- = value;

			value = m_pBottomSamples[m_nbSamples-1];
			p = m_pBottomSamples + m_nbSamples + m_bottomSamplesPadding -1;
			while( p >= m_pBottomSamples + m_nbSamples )
				* p-- = value;
		}

		int padding = std::max(m_topSamplesPadding,m_bottomSamplesPadding);

		int dirtBegin = std::max(0,m_dirtBegin-padding);
		int dirtEnd = std::min(m_nbSamples,m_dirtEnd+padding);
		int dirtSize = dirtEnd - dirtBegin;

		if( !m_bHasOutlines )
		{
			int allocated = (dirtSize)*2*sizeof(spx);

			spx * pBuffer = (spx*) GfxBase::memStackAlloc(allocated);

			memcpy(pBuffer, m_pTopSamples + dirtBegin, dirtSize*sizeof(spx) );
			memcpy(pBuffer+dirtSize, m_pBottomSamples + dirtBegin, dirtSize*sizeof(spx) );

			// Loop through new edges and fix possible issues.

 			for (int i = 0; i < dirtSize; i++)
			{
				// Flip lines if they cross

				if( pBuffer[i] > pBuffer[i+dirtSize])
					std::swap( pBuffer[i], pBuffer[i+dirtSize]);
			}


			// Update edgemap.

			pEdgemap->importSamples(SampleOrigo::Top, pBuffer, 0, 1, dirtBegin, dirtEnd);
			pEdgemap->importSamples(SampleOrigo::Top, pBuffer + dirtSize, 1, 2, dirtBegin, dirtEnd);

			GfxBase::memStackFree(allocated);
		}
		else
		{
			int allocated = (dirtSize)*4*sizeof(spx);

			spx * pBuffer = (spx*) GfxBase::memStackAlloc(allocated);

			_drawLine(m_pTopSamples + dirtBegin, pBuffer, pBuffer + dirtSize, dirtSize, m_topBrush );
			_drawLine(m_pBottomSamples + dirtBegin, pBuffer+dirtSize*2, pBuffer + dirtSize*3, dirtSize, m_bottomBrush );

			// Loop through new edges and fix possible issues.

			for (int i = 0; i < dirtSize; i++)
			{
				// Flip lines if they cross

				if( pBuffer[i] > pBuffer[i+dirtSize*2])
				{
					std::swap( pBuffer[i], pBuffer[i+dirtSize*2]);
					std::swap( pBuffer[i+dirtSize], pBuffer[i+dirtSize*3]);
				}

				// Handle outline overlapping

				if (pBuffer[i+dirtSize*2] < pBuffer[i+dirtSize] )
				{
					pBuffer[i+dirtSize*2] = pBuffer[i+dirtSize];
					if (pBuffer[i+dirtSize*3] < pBuffer[i+dirtSize*2] )
						pBuffer[i+dirtSize*3] = pBuffer[i+dirtSize*2];
				}
			}

			// Update edgemap.

			pEdgemap->importSamples(SampleOrigo::Top, pBuffer, 0, 2, dirtBegin, dirtEnd);
			pEdgemap->importSamples(SampleOrigo::Top, pBuffer + dirtSize*2, 2, 4, dirtBegin, dirtEnd);

			GfxBase::memStackFree(allocated);
		}

		m_dirtBegin = m_nbSamples;
		m_dirtEnd = 0;
	}

	//____ _setFlatLine() _______________________________________________________

	void EdgemapWave::_setFlatLine( spx * pSamples, int sampleBegin, int sampleEnd, spx sample )
	{
		limit(sampleBegin, 0, m_nbSamples);
		limit(sampleEnd, sampleBegin, m_nbSamples);

		while( sampleBegin < sampleEnd && pSamples[sampleBegin] == sample )
			sampleBegin++;

		while( sampleEnd > sampleBegin && pSamples[sampleEnd-1] == sample )
			sampleEnd--;

		spx * p = pSamples + sampleBegin;
		for( int i = 0 ; i < sampleEnd - sampleBegin ; i++ )
			* p++ = sample;

		if( sampleBegin < m_dirtBegin )
			m_dirtBegin = sampleBegin;

		if( sampleEnd > m_dirtEnd )
			m_dirtEnd = sampleEnd;
	}


	//____ _drawLine() __________________________________________________________

	void EdgemapWave::_drawLine(spx * pSrc, spx * pDestTop, spx * pDestBottom, int nPoints, Brush& brush)
	{

		for (int i = 0; i < nPoints; i++)
		{
			// Start with top and bottom for current point

			int top = pSrc[i] - brush.influenceSlope[0];
			int bottom = pSrc[i] + brush.influenceSlope[0];

			// Check brush's coverage from previous points

			spx * pInfluence = brush.influenceSlope;
			spx * _pSrc = pSrc + i;


			while( * pInfluence != 0 )
			{
				spx inc = (_pSrc[-1] - _pSrc[0])/4;
				spx sample = * _pSrc--;

				for( int j = 0 ; j < 4 ; j++ )
				{
					sample += inc;

					spx topCover = sample - *pInfluence;
					spx bottomCover = sample + *pInfluence;

					if (topCover < top)
						top = topCover;
					else if (bottomCover > bottom)
						bottom = bottomCover;

					pInfluence++;
				}
			}

			// Check brush's coverage from following points

			pInfluence = brush.influenceSlope;
			_pSrc = pSrc + i;

			while( * pInfluence != 0 )
			{
				spx inc = (_pSrc[1] - _pSrc[0])/4;
				spx sample = * _pSrc++;

				for( int j = 0 ; j < 4 ; j++ )
				{
					sample += inc;

					spx topCover = sample - *pInfluence;
					spx bottomCover = sample + *pInfluence;

					if (topCover < top)
						top = topCover;
					else if (bottomCover > bottom)
						bottom = bottomCover;

					pInfluence++;
				}
			}

			// Save traced values

			*pDestTop++ = top;
			*pDestBottom++ = bottom;
		}
	}

	//____ _generateInfluenceSlope() ____________________________________________

	void EdgemapWave::_generateInfluenceSlope(Brush& brush)
	{
		auto pCurveTab = GfxBase::curveTab();
		int curveTabSize = GfxBase::curveTabSize();

		int radius = brush.thickness / 2;

		brush.influenceSlope[0] = radius;

		int nSteps = radius * 4 / 64;

		for( int i = 0 ; i < nSteps ; i++ )
			brush.influenceSlope[i] = (pCurveTab[curveTabSize -1 -(curveTabSize * i*(64/4) / radius)] * radius) >> 16;

		brush.influenceSlope[nSteps] 	= 0;
		brush.influenceSlope[nSteps+1] 	= 0;
		brush.influenceSlope[nSteps+2] 	= 0;
		brush.influenceSlope[nSteps+3] 	= 0;
	}

	//____ _updateEdgemapColors() ______________________________________________________

	void EdgemapWave::_updateEdgemapColors()
	{
		Edgemap * pEdgemap = m_pieces[0].pEdgemap;

		if( pEdgemap == nullptr )
		{
			_regenEdgemap();
			return;
		}

		HiColor	colors[5];
		int nSegments = _generateColorPalette(colors);
		pEdgemap->setColors( 0, nSegments, colors );

		if( m_pTint || m_pOutlineTint )
		{
			Tint_p	tints[5];
			_generateTintPalette(tints);
			pEdgemap->setColors( 0, nSegments, tints );
		}
	}

	//____ _regenEdgemap() ______________________________________________________

	void EdgemapWave::_regenEdgemap()
	{
		EdgemapFactory_p pFactory = _factoryOrDefault(m_pFactory);

		HiColor	colors[5];
		Tint_p	tints[5];

		int nSegments = _generateColorPalette( colors );
		_generateTintPalette( tints );

		bool bTints = m_pTint || m_pOutlineTint;

		m_pieces[0].pEdgemap = pFactory->createEdgemap( WGBP(Edgemap, _.colors = colors, _.tints = bTints ? tints : nullptr, _.size = m_size, _.segments = nSegments ) );
	}

	//____ _generateColorPalette() _______________________________________________

	int EdgemapWave::_generateColorPalette( HiColor * pDest )
	{
		HiColor * p = pDest;
		* p++ = Color::Transparent;
		if( m_bHasOutlines )
			* p++ = m_outlineColor;
		* p++ = m_color;
		if( m_bHasOutlines )
			* p++ = m_outlineColor;
		* p++ = Color::Transparent;

		return int(p - pDest);
	}

	//____ _generateTintPalette() ______________________________________________

	int EdgemapWave::_generateTintPalette( Tint_p * pDest )
	{
		Tint_p * p = pDest;
		* p++ = nullptr;
		if( m_bHasOutlines )
			* p++ = m_pOutlineTint;
		* p++ = m_pTint;
		if( m_bHasOutlines )
			* p++ = m_pOutlineTint;
		* p++ = nullptr;

		return int( p - pDest );
	}
}


