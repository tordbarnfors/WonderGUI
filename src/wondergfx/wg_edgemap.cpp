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

	Edgemap::Edgemap(const Blueprint& bp) : m_size(bp.size), m_nbSegments(bp.segments), m_nbRenderSegments(bp.segments)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		// Setup buffers

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
		if (pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments - 1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > (m_size.w + 1))
			return false;

		_importSamples(origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
		return true;
	}

	bool Edgemap::importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments - 1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > (m_size.w + 1))
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

	//____ _importSamples() ________________________________________________________

	void Edgemap::_importSamples(SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (samplePitch == 0)
			samplePitch = 1;


		if (edgePitch == 0)
			edgePitch = samplePitch * (sampleEnd - sampleBegin);

		int destSamplePitch = m_nbSegments - 1;
		int destEdgePitch = 1;


		spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
		spx offset = 0;

		if (origo == SampleOrigo::Bottom)
			offset = m_size.h * 64;
		else if (origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp)
			offset = m_size.h * 32;

		for (int edge = edgeBegin; edge < edgeEnd; edge++)
		{
			const spx* pSrc = pSource + edgePitch * (edge - edgeBegin);
			spx* pDst = m_pSamples + edge + sampleBegin * (m_nbSegments - 1);

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				*pDst = (*pSrc * mul) + offset;
				pDst += destSamplePitch;
				pSrc += samplePitch;
			}
		}

		_samplesUpdated(edgeBegin, edgeEnd, sampleBegin, sampleEnd);
	}

	void Edgemap::_importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
		int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		if (samplePitch == 0)
			samplePitch = 1;


		if (edgePitch == 0)
			edgePitch = samplePitch * (sampleEnd - sampleBegin);

		int destSamplePitch = m_nbSegments - 1;
		int destEdgePitch = 1;


		spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
		spx offset = 0;

		if (origo == SampleOrigo::Bottom)
			offset = m_size.h * 64;
		else if (origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp)
			offset = m_size.h * 32;

		if (origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp)
			mul *= m_size.h * 32;
		else
			mul *= m_size.h * 64;

		for (int edge = edgeBegin; edge < edgeEnd; edge++)
		{
			const float* pSrc = pSource + edgePitch * edge + samplePitch * sampleBegin;
			spx* pDst = m_pSamples + edge + sampleBegin * (m_nbSegments - 1);

			for (int sample = sampleBegin; sample < sampleEnd; sample++)
			{
				*pDst = (*pSrc * mul) + offset;
				pDst += destSamplePitch;
				pSrc += samplePitch;
			}
		}

		_samplesUpdated(edgeBegin, edgeEnd, sampleBegin, sampleEnd);
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
