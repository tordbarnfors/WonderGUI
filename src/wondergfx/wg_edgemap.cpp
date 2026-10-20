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
#include <wg_edgemap.h>

#include <cstring>
#include <climits>
#include <algorithm>

namespace wg
{

	const TypeInfo Edgemap::TYPEINFO = { "Edgemap", &Object::TYPEINFO };

	Edgemap::Edgemap(const Blueprint& bp) : m_size(bp.size), m_bTiling(bp.tiling), m_nbSegments(bp.segments), m_nbRenderSegments(bp.segments)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		// Setup buffers. We always store m_size.w + 1 columns, see _mirrorFirstColumn().

		int sampleArraySize = (bp.size.w + 1) * bp.segments * sizeof(spx);
		int flatColorsSize = bp.segments * sizeof(HiColor);

		m_pBuffer = new char[sampleArraySize + flatColorsSize];
		char* pDest = m_pBuffer;

		m_pFlatColors = (HiColor*)pDest;
		pDest += flatColorsSize;

		m_pSamples = (spx*) pDest;
		pDest += sampleArraySize;

		std::fill(m_pSamples, (spx*) pDest, bp.size.h*64 );		// All edges start right below the map, making first segement fill the area.

		m_pTints = new Tint_p[bp.segments];

		// Fill in colors

		for (int i = 0; i < bp.segments; i++)
			m_pFlatColors[i] = bp.colors ? bp.colors[i] : HiColor::Transparent;

		if (bp.tints)
			setColors(0, bp.segments, bp.tints);

		m_bConstructed = true;
	}

	//____ destructor ________________________________________________________

	Edgemap::~Edgemap()
	{
		delete[] m_pTints;
		delete[] m_pBuffer;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& Edgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setRenderSegments() ________________________________________________

	bool Edgemap::setRenderSegments(int segments)
	{
		if( segments < 1 || segments > m_nbSegments )
			return false;

		m_nbRenderSegments = segments;
		return true;
	}

	//____ setColors() ____________________________________________________________
	/**
	 * @brief Set flat colors for a range of segments.
	 *
	 * Any Tints of these segments are removed.
	 */

	bool Edgemap::setColors(int begin, int end, const HiColor* pColors)
	{
		if (begin < 0 || end > m_nbSegments || begin > end || !pColors)
			return false;

		for (int i = begin; i < end; i++)
		{
			m_pFlatColors[i] = *pColors++;

			if (m_pTints[i])
			{
				m_pTints[i] = nullptr;
				m_nbTints--;
			}
		}

		if( m_bConstructed )
			_colorsUpdated(begin, end);

		return true;
	}

	/**
	 * @brief Set Tints for a range of segments.
	 *
	 * A Tint is placed in the rectangle of the Edgemap (before any flip or rotation).
	 * Entries that are nullptr leave the segment with its flat color. Flat Tints
	 * (single color, not a mix) are stored as flat colors.
	 */

	bool Edgemap::setColors(int begin, int end, const Tint_p* pTints)
	{
		if (begin < 0 || end > m_nbSegments || begin > end || !pTints)
			return false;

		for (int i = begin; i < end; i++)
		{
			Tint* pTint = pTints[i - begin];

			if (m_pTints[i])
				m_nbTints--;

			if (pTint && pTint->isFlat() && !pTint->isMix())
			{
				m_pFlatColors[i] = pTint->stops()[0].color;
				m_pTints[i] = nullptr;
			}
			else
			{
				m_pTints[i] = pTint;
				if (pTint)
					m_nbTints++;
			}
		}

		if( m_bConstructed )
			_colorsUpdated(begin, end);

		return true;
	}

	//____ importSamples() _________________________________________________________

	bool Edgemap::importSamples(SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments - 1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > samples())
			return false;

		_importSamples(origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
		return true;
	}

	bool Edgemap::importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments - 1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > samples())
			return false;

		_importSamples(origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
		return true;
	}


	//____ exportSamples() _________________________________________________________

	bool Edgemap::exportSamples(SampleOrigo origo, spx* pDestination, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		//TODO: Implement!!!

		return false;
	}

	bool Edgemap::exportSamples(SampleOrigo origo, float* pDestination, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		//TODO: Implement!!!

		return false;
	}


	//____ exportBounds() ___________________________________________________

	// NOTE: mapOffset may be negative.

	void Edgemap::exportBounds( spx * pMinMaxOutput, int nSections, int sectionWidth, int topEdge, int bottomEdge, int mapOffset, int minMaxPitch  )
	{
		int nEdges = m_nbSegments-1;
		int totalSamples = m_size.w+1;


		int section = 0;
		int sectionSamples = sectionWidth+1;

		int startSample = -mapOffset;

		while( startSample < -sectionWidth && section < nSections )
		{
			pMinMaxOutput[0] = INT_MAX;
			pMinMaxOutput[1] = INT_MIN;
			pMinMaxOutput += minMaxPitch;

			startSample += sectionWidth;
			section++;
		}

		if( startSample < 0 )
		{
			sectionSamples -= (-startSample);
			startSample = 0;
		}

		spx * pTopSamples = m_pSamples + topEdge;
		spx * pBottomSamples = m_pSamples + bottomEdge;

		while( section < nSections )
		{
			// Handle special case where we run out of samples in this section

			if( startSample + sectionSamples > m_size.w )
			{
				sectionSamples = (m_size.w - startSample) + 1;
				if( sectionSamples <= 1 )
				{
					while( section < nSections )
					{
						pMinMaxOutput[0] = INT_MAX;
						pMinMaxOutput[1] = INT_MIN;
						pMinMaxOutput += minMaxPitch;
						section++;
					}
					return;
				}
			}

			//

			int sampleOfs = startSample * nEdges;

			spx minSample = INT_MAX;
			spx maxSample = INT_MIN;

			for( int i = 0 ; i < sectionSamples ; i++ )
			{
				minSample = std::min(minSample, pTopSamples[sampleOfs]);
				maxSample = std::max(maxSample, pBottomSamples[sampleOfs]);

				sampleOfs += nEdges;
			}

			pMinMaxOutput[0] = minSample;
			pMinMaxOutput[1] = maxSample;
			pMinMaxOutput += minMaxPitch;

			section++;
			startSample += sectionSamples-1;

			sectionSamples = sectionWidth+1;
		}
	}

	//____ convertSamples() _______________________________________________________

	void Edgemap::convertSamples( spx * pDest, const spx * pSource, SampleOrigo origo, spx height, int nEdges, int nSamples,
								  int srcEdgePitch, int srcSamplePitch, int destEdgePitch, int destSamplePitch )
	{
		spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
		spx offset = 0;

		if( origo == SampleOrigo::Bottom )
			offset = height;
		else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
			offset = height/2;

		for( int edge = 0 ; edge < nEdges ; edge++ )
		{
			const spx * pSrc = pSource + srcEdgePitch * edge;
			spx * pDst = pDest + destEdgePitch * edge;

			for( int sample = 0 ; sample < nSamples ; sample++ )
			{
				* pDst = (* pSrc * mul) + offset;
				pDst += destSamplePitch;
				pSrc += srcSamplePitch;
			}
		}
	}

	void Edgemap::convertSamples( spx * pDest, const float * pSource, SampleOrigo origo, spx height, int nEdges, int nSamples,
								  int srcEdgePitch, int srcSamplePitch, int destEdgePitch, int destSamplePitch )
	{
		spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
		spx offset = 0;

		if( origo == SampleOrigo::Bottom )
			offset = height;
		else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
			offset = height/2;

		if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
			mul *= height/2;
		else
			mul *= height;

		for( int edge = 0 ; edge < nEdges ; edge++ )
		{
			const float * pSrc = pSource + srcEdgePitch * edge;
			spx * pDst = pDest + destEdgePitch * edge;

			for( int sample = 0 ; sample < nSamples ; sample++ )
			{
				* pDst = (* pSrc * mul) + offset;
				pDst += destSamplePitch;
				pSrc += srcSamplePitch;
			}
		}
	}

	//____ convertSample() ________________________________________________________

	spx Edgemap::convertSample( spx sample, SampleOrigo origo, spx height )
	{
		spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
		spx offset = 0;

		if( origo == SampleOrigo::Bottom )
			offset = height;
		else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
			offset = height/2;

		return (sample * mul) + offset;
	}

	spx Edgemap::convertSample( float sample, SampleOrigo origo, spx height )
	{
		spx out;
		convertSamples( &out, &sample, origo, height, 1, 1, 0, 0, 0, 0 );
		return out;
	}

	//____ _importSamples() ________________________________________________________

	void Edgemap::_importSamples(SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (samplePitch == 0)
			samplePitch = 1;

		if (edgePitch == 0)
			edgePitch = samplePitch * (sampleEnd - sampleBegin);

		convertSamples(m_pSamples + edgeBegin + sampleBegin * (m_nbSegments - 1), pSource, origo, m_size.h * 64,
					   edgeEnd - edgeBegin, sampleEnd - sampleBegin, edgePitch, samplePitch, 1, m_nbSegments - 1);

		_samplesUpdated(edgeBegin, edgeEnd, sampleBegin, sampleEnd);
		_mirrorFirstColumn(edgeBegin, edgeEnd, sampleBegin);
	}

	void Edgemap::_importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (samplePitch == 0)
			samplePitch = 1;

		if (edgePitch == 0)
			edgePitch = samplePitch * (sampleEnd - sampleBegin);

		convertSamples(m_pSamples + edgeBegin + sampleBegin * (m_nbSegments - 1), pSource + edgePitch * edgeBegin + samplePitch * sampleBegin,
					   origo, m_size.h * 64, edgeEnd - edgeBegin, sampleEnd - sampleBegin, edgePitch, samplePitch, 1, m_nbSegments - 1);

		_samplesUpdated(edgeBegin, edgeEnd, sampleBegin, sampleEnd);
		_mirrorFirstColumn(edgeBegin, edgeEnd, sampleBegin);
	}

	//____ _mirrorFirstColumn() __________________________________________________
	/*
		A tiling Edgemap wraps around horizontally, its last interval running from the last
		sample back to the first. We store the first column once more after the last, so that
		everything reading the samples can treat it like an ordinary Edgemap of m_size.w + 1
		columns. Called after samples starting at sampleBegin have been imported.

		The mirrored column is reported through _samplesUpdated() as a separate update
		of sample m_size.w, which lies outside the samples visible through the API.
	*/

	void Edgemap::_mirrorFirstColumn(int edgeBegin, int edgeEnd, int sampleBegin)
	{
		if (!m_bTiling || sampleBegin != 0 || edgeBegin == edgeEnd)
			return;

		int nEdges = m_nbSegments - 1;
		spx* pFirst = m_pSamples + edgeBegin;
		spx* pMirror = m_pSamples + m_size.w * nEdges + edgeBegin;

		for (int edge = edgeBegin; edge < edgeEnd; edge++)
			*pMirror++ = *pFirst++;

		_samplesUpdated(edgeBegin, edgeEnd, m_size.w, m_size.w + 1);
	}

	//____ _validateBlueprint() __________________________________________________

	bool Edgemap::_validateBlueprint(const Blueprint& bp)
	{
		//TODO: Throw errors, not just return false.

		if( bp.size.w <= 0 || bp.size.h <= 0 )
			return false;

		if( bp.segments <= 0 || bp.segments > maxSegments )
			return false;

		return true;
	}


} // namespace wg
