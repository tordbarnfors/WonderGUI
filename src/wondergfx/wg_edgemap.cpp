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

#include <wg_edgemap.h>
#include <wg_gradyent.h>

#include <cstring>

namespace wg
{

	const TypeInfo Edgemap::TYPEINFO = { "Edgemap", &Object::TYPEINFO };

	//____ constructor _________________________________________________________

	Edgemap::Edgemap(const Blueprint& bp) : m_size(bp.size), m_nbSegments(bp.segments), m_nbRenderSegments(bp.segments)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		// Check what kind of color storage we should have.

		bool bStripX = false; 
		bool bStripY = false;

		if( bp.paletteType != EdgemapPalette::Undefined )
		{
			switch(bp.paletteType)
			{
				default:
				case EdgemapPalette::Flat:
					break;
					
				case EdgemapPalette::ColorstripX:
					bStripX = true;
					break;
					
				case EdgemapPalette::ColorstripY:
					bStripY = true;
					break;
					
				case EdgemapPalette::ColorstripXY:
					bStripX = true;
					bStripY = true;
					break;
			}
		}
		else if (bp.colorstripsX || bp.colorstripsY)
		{
			bStripX = bp.colorstripsX;
			bStripY = bp.colorstripsY;
		}
		else if (bp.tintmaps)
		{
			for (int i = 0; i < bp.segments; i++)
			{
				if (bp.tintmaps[i] && bp.tintmaps[i]->isHorizontal() )
					bStripX = true;

				if (bp.tintmaps[i] && bp.tintmaps[i]->isVertical())
					bStripY = true;
			}
		}

		if( bp.paletteType == EdgemapPalette::Undefined )
		{
			if( bStripX && bStripY )
				m_paletteType = EdgemapPalette::ColorstripXY;
			else if( bStripX )
				m_paletteType = EdgemapPalette::ColorstripX;
			else if( bStripY )
				m_paletteType = EdgemapPalette::ColorstripY;
			else
				m_paletteType = EdgemapPalette::Flat;
		}
		else
			m_paletteType = bp.paletteType;
		
				
		// Setup buffers

		int sampleArraySize = (bp.size.w + 1) * bp.segments * sizeof(spx);

		int stripSizeX = bStripX ? (bp.size.w) * bp.segments * sizeof(HiColor) : 0;
		int stripSizeY = bStripY ? (bp.size.h) * bp.segments * sizeof(HiColor) : 0;
		int flatColorsSize = (bStripX == false && bStripY == false) ? bp.segments * sizeof(HiColor) : 0;

		int bytes = sampleArraySize + stripSizeX + stripSizeY + flatColorsSize;

		m_pBuffer = new char[bytes];
		char* pDest = m_pBuffer;


		m_pSamples = (spx*) pDest;
		pDest += sampleArraySize;

		m_pPalette = (HiColor*) pDest;
		m_paletteSize = (stripSizeX + stripSizeY + flatColorsSize) / sizeof(HiColor);

		if (stripSizeX > 0)
		{
			m_pColorstripsX = (HiColor *) pDest;
			pDest += stripSizeX;
		}

		if (stripSizeY > 0)
		{
			m_pColorstripsY = (HiColor*)pDest;
			pDest += stripSizeY;
		}

		if (flatColorsSize)
		{
			m_pFlatColors = (HiColor*)pDest;
			pDest += flatColorsSize;
		}

		// Fill in colorstrips

		if( m_pColorstripsX || m_pColorstripsY)
		{
			// We need to fill in colorstrips

			if (bp.colorstripsX || bp.colorstripsY)
			{
				// We have colorstrips defined. This overrides everything else, so we ignore tintmaps, gradients and colors.

				if (bp.colorstripsX)
				{
					auto pInput = bp.colorstripsX;
					auto pOutput = m_pColorstripsX;

					for (int seg = 0; seg < bp.segments; seg++)
					{
						memcpy(pOutput, pInput, bp.size.w * sizeof(HiColor));

						pInput += bp.size.w;
						pOutput += bp.size.w;
					}
				}

				if (bp.colorstripsY)
				{
					auto pInput = bp.colorstripsY;
					auto pOutput = m_pColorstripsY + bp.size.w;

					for (int seg = 0; seg < bp.segments; seg++)
					{
						memcpy(pOutput, pInput, bp.size.h * sizeof(HiColor));

						pInput += bp.size.h;
						pOutput += bp.size.h;
					}
				}
			}
			else if( bp.tintmaps )
			{
				// We need to fill in colorstrips from tintmaps and possibly colors.

				for (int seg = 0; seg < bp.segments; seg++)
				{
					if (bp.tintmaps[seg])
					{
						auto pTintmap = bp.tintmaps[seg];

						if (m_pColorstripsX)
							pTintmap->exportHorizontalColors(bp.size.w * 64, m_pColorstripsX + seg * m_size.w);

						if (m_pColorstripsY)
							pTintmap->exportVerticalColors(bp.size.w * 64, m_pColorstripsY + seg * m_size.h);
					}
					else if (bp.colors)
					{
						if (m_pColorstripsX)
						{
							auto pOutput = m_pColorstripsX + seg * m_size.w;
							for (int i = 0; i < bp.size.w; i++)
								*pOutput++ = bp.colors[seg];


							if (m_pColorstripsY)
							{
								auto pOutput = m_pColorstripsY + seg * m_size.h;

								for (int i = 0; i < bp.size.h; i++)
									*pOutput++ = HiColor::White;
							}
						}
						else
						{
							auto pOutput = m_pColorstripsY + seg * m_size.h;
							for (int i = 0; i < bp.size.h; i++)
								*pOutput++ = bp.colors[seg];

						}
					}
					else
					{
						if (m_pColorstripsX)
						{
							auto pOutput = m_pColorstripsX + seg * m_size.w;
							for (int i = 0; i < bp.size.w; i++)
								*pOutput++ = HiColor::Transparent;
						}

						if (m_pColorstripsY)
						{
							auto pOutput = m_pColorstripsY + seg * m_size.h;
							for (int i = 0; i < bp.size.h; i++)
								*pOutput++ = HiColor::Transparent;
						}
					}
				}

			}
			else if( bp.gradients )
			{
				// We have gradients so we fill in our colorstrips from them.

				setColors(0, bp.segments, bp.gradients);
			}
		}
		else if( bp.colors )
		{
			// Flat colors
			
			setColors(0, bp.segments, bp.colors);
		}

		m_bConstructed = true;
	}
	
	//____ destructor ________________________________________________________

	Edgemap::~Edgemap()
	{
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

	bool Edgemap::setColors(int begin, int end, const HiColor* pColors)
	{
		if (m_pFlatColors)
		{
			for (int i = begin; i < end; i++)
				m_pFlatColors[i] = *pColors++;

			if( m_bConstructed )
				_colorsUpdated(begin, end);
		}
		else if (m_pColorstripsX)
		{
			// Set whole horizontal colorstrips to our color.

			HiColor* pDest = m_pColorstripsX + begin * m_size.w;
			for (int seg = begin; seg < end; seg++)
			{
				for (int i = 0; i < m_size.w; i++)
					*pDest++ = *pColors;
				pColors++;
			}

			if( m_bConstructed )
				_colorsUpdated((m_pColorstripsX - m_pPalette) + begin * m_size.w, (m_pColorstripsX - m_pPalette) + end * m_size.w);

			// If we also have vertical colorstrip, we need to set its colors to White.

			if (m_pColorstripsY)
			{
				HiColor* pDest = m_pColorstripsY + begin * m_size.h;

				for (int seg = begin; seg < end; seg++)
				{
					for (int i = 0; i < m_size.h; i++)
						*pDest++ = HiColor::White;
				}

				if( m_bConstructed )
					_colorsUpdated((m_pColorstripsY - m_pPalette) + begin * m_size.h, (m_pColorstripsY - m_pPalette) + end * m_size.h);
			}
		}
		else
		{
			// We only have vertical colorstrips, so we set our new colors in them.

			HiColor* pDest = m_pColorstripsY + begin * m_size.h;
			for (int seg = begin; seg < end; seg++)
			{
				for (int i = 0; i < m_size.h; i++)
					*pDest++ = *pColors;
				pColors++;
			}

			if( m_bConstructed )
				_colorsUpdated((m_pColorstripsY - m_pPalette) + begin * m_size.h, (m_pColorstripsY - m_pPalette) + end * m_size.h);
		}

		return true;
	}


	bool Edgemap::setColors(int begin, int end, const Gradient* pGradients)
	{
		if (m_pFlatColors)
			return false;

		for (int seg = begin; seg < end; seg++)
		{
			auto pGradyent = Gradyent::create(*pGradients++);

			if (m_pColorstripsX)
				pGradyent->exportHorizontalColors(m_size.w, m_pColorstripsX + seg * m_size.w);

			if (m_pColorstripsY)
				pGradyent->exportVerticalColors(m_size.w, m_pColorstripsY + seg * m_size.h);
		}

		if( m_pColorstripsX && m_bConstructed )
			_colorsUpdated((m_pColorstripsX - m_pPalette) + begin * m_size.w, (m_pColorstripsX - m_pPalette) + end * m_size.w);

		if (m_pColorstripsY && m_bConstructed )
			_colorsUpdated((m_pColorstripsY - m_pPalette) + begin * m_size.h, (m_pColorstripsY - m_pPalette) + end * m_size.h);

		return true;
	}

	bool Edgemap::setColors(int begin, int end, const Tintmap_p* pTintmaps)
	{
		if (m_pFlatColors)
			return false;

		//TODO: Also check so that the tintmaps don't tint a direction we don't have colorstrips for.


		if (m_pColorstripsX)
		{
			auto pMaps = pTintmaps;
			HiColor* pDest = m_pColorstripsX + begin * m_size.w;

			for (int seg = begin; seg < end; seg++)
			{
				Tintmap* pMap = *pMaps++;

				pMap->exportHorizontalColors(m_size.w, pDest);
				pDest += m_size.w;
			}

			_colorsUpdated((m_pColorstripsX-m_pPalette) + begin * m_size.w, (m_pColorstripsX - m_pPalette) + end * m_size.w);
		}

		if (m_pColorstripsY)
		{
			auto pMaps = pTintmaps;
			HiColor* pDest = m_pColorstripsY + begin * m_size.h;

			for (int seg = begin; seg < end; seg++)
			{
				Tintmap* pMap = *pMaps++;

				pMap->exportVerticalColors(m_size.h, pDest);
				pDest += m_size.h;
			}

			_colorsUpdated((m_pColorstripsY - m_pPalette) + begin * m_size.h, (m_pColorstripsY - m_pPalette) + end * m_size.h);
		}

		return true;
	}

	bool Edgemap::setColors(int begin, int end, const HiColor* pColorstripsX, const HiColor* pColorstripsY)
	{
		if ((pColorstripsX && !m_pColorstripsX) || (pColorstripsY && !m_pColorstripsY))
			return false;

		if (pColorstripsX)
		{
			int nColors = (end - begin) * m_size.w;

			HiColor* pDest = m_pColorstripsX + begin * m_size.w;
			for (int i = 0; i < nColors; i++)
					*pDest++ = *pColorstripsX++;

			_colorsUpdated((m_pColorstripsX-m_pPalette) + begin * m_size.w, (m_pColorstripsX - m_pPalette) + end * m_size.w);
		}

		if (pColorstripsY)
		{
			int nColors = (end - begin) * m_size.h;

			HiColor* pDest = m_pColorstripsY + begin * m_size.h;
			for (int i = 0; i < nColors; i++)
				*pDest++ = *pColorstripsY++;

			_colorsUpdated((m_pColorstripsY - m_pPalette) + begin * m_size.h, (m_pColorstripsY - m_pPalette) + end * m_size.h);
		}

		return true;
	}

	//____ importPaletteEntries() ________________________________________________

	void Edgemap::importPaletteEntries( int begin, int end, const HiColor * pColors )
	{
		memcpy( m_pPalette + begin, pColors, (end - begin)*sizeof(HiColor) );
		_colorsUpdated(begin, end);
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

		if (bp.paletteType == EdgemapPalette::Flat && (bp.gradients || bp.tintmaps || bp.colorstripsX || bp.colorstripsY))
			return false;

		if (bp.paletteType == EdgemapPalette::ColorstripX && bp.colorstripsY)
			return false;

		if (bp.paletteType == EdgemapPalette::ColorstripY && bp.colorstripsX)
			return false;

		if (bp.paletteType == EdgemapPalette::Undefined)
		{
			if (bp.tintmaps && (bp.colorstripsX || bp.colorstripsY || bp.gradients))
				return false;

			if (bp.colors && (bp.gradients || bp.colorstripsX || bp.colorstripsY))
				return false;
		}

		return true;
	}


} // namespace wg
