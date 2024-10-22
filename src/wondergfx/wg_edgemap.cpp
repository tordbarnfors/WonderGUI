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

		if (bp.colorstripsX || bp.colorstripsY)
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


		// Setup buffers

		int sampleArraySize = (bp.size.w + 1) * bp.segments * sizeof(spx);

		int stripSizeX = bStripX ? (bp.size.w) * bp.segments * sizeof(HiColor) : 0;
		int stripSizeY = bStripY ? (bp.size.h) * bp.segments * sizeof(HiColor) : 0;
		int flatColorsSize = (bStripX == false && bStripY == false) ? bp.segments * sizeof(HiColor) : 0;
		int gradientArraySize = bp.segments * sizeof(Gradient);							// We keep a set of gradients for backward compatiblity

		int bytes = sampleArraySize + stripSizeX + stripSizeY + flatColorsSize + gradientArraySize;

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

		m_pGradients = (Gradient*) pDest;


		// Fill in colorstrips

		if( m_pColorstripsX || m_pColorstripsY)
		{
			// We need to fill in colorstrips

			if (bp.colorstripsX || bp.colorstripsY)
			{
				// We have colorstrips defined. This overrides everything else, so we ignore tintmaps, gradients and colors.
				// Gradients are used for saved gradients.

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

						if( !bp.colorstripsX)
							m_pGradients[seg] = Gradient(pInput[0], pInput[bp.size.h - 1], pInput[bp.size.h - 1], pInput[0]);

						pInput += bp.size.h;
						pOutput += bp.size.h;
					}
				}

				if (bp.gradients)
				{
					for (int seg = 0; seg < bp.segments; seg++)
						m_pGradients[seg] = bp.gradients[seg];
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

						m_pGradients[seg] = pTintmap->exportGradient();
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

						m_pGradients[seg] = Gradient(bp.colors[seg], bp.colors[seg], bp.colors[seg], bp.colors[seg]);
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

						m_pGradients[seg] = Gradient(HiColor::Transparent, HiColor::Transparent, HiColor::Transparent, HiColor::Transparent);
					}
				}

			}
			else
			{
				// We have gradients so we fill in our colorstrips from them.

				for (int seg = 0; seg < bp.segments; seg++)
				{
					auto& gradient = bp.gradients[seg];

					auto pTintmap = Gradyent::create(gradient);

					if (m_pColorstripsX)
						pTintmap->exportHorizontalColors(bp.size.w * 64, m_pColorstripsX + seg * m_size.w);

					if (m_pColorstripsY)
						pTintmap->exportVerticalColors(bp.size.w * 64, m_pColorstripsY + seg * m_size.h);

					m_pGradients[seg] = pTintmap->exportGradient();
				}
			}
		}
		else
		{
			// We just have flat colors so we simply copy them

			memcpy(m_pFlatColors, bp.colors, bp.segments * sizeof(HiColor));

			for (int i = 0; i < bp.segments; i++)
			{
				m_pFlatColors[i] = bp.colors[i];
				m_pGradients[i] = Gradient(bp.colors[i]);
			}

		}

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
/*
		int pitch = m_size.w + m_size.h;

		auto pOutput = m_pColors + begin * pitch;

		for (int seg = begin; seg < end; seg++)
		{
			for (int i = 0; i < m_size.w; i++)
				*pOutput++ = *pColors;

			for (int i = 0; i < m_size.h; i++) 
				*pOutput++ = HiColor::White;

			pColors++;
		}

		_colorsUpdated(begin, end);
*/
		return true;
	}

	//____ setGradients() ______________________________________________________

	bool Edgemap::setGradients(int begin, int end, const Gradient* pGradients)
	{
		return true;
	}


	//____ setTintmaps() __________________________________________________________

	bool Edgemap::setTintmaps(int begin, int end, const Tintmap_p* pTintmaps)
	{
/*
		int pitch = m_size.w + m_size.h;

		auto pOutput = m_pColors + begin * pitch;

		for (int seg = begin; seg < end; seg++)
		{
			auto pTintmap = *pTintmaps++;

			if (pTintmap)
			{
				pTintmap->exportHorizontalColors(m_size.w * 64, pOutput);
				pOutput += m_size.w;
				pTintmap->exportVerticalColors(m_size.h * 64, pOutput);
				pOutput += m_size.h;

				m_horrTintmaps[seg] = pTintmap->isHorizontal();
				m_vertTintmaps[seg] = pTintmap->isVertical();

				m_pGradients[seg] = pTintmap->exportGradient();
			}
			else
			{
				for (int i = 0; i < m_size.w + m_size.h; i++)
					*pOutput++ = HiColor::Transparent;

				m_pGradients[seg] = Gradient(HiColor::Transparent, HiColor::Transparent, HiColor::Transparent, HiColor::Transparent);
			}
		}

		_colorsUpdated(begin, end);
*/
		return true;
	}

	//____ setColorstrips() ____________________________________________________

/*
	bool Edgemap::setColorstrips(int begin, int end, const HiColor* pStripsX, const HiColor* pStripsY)
	{
		int colorPitch = m_size.w + m_size.h;

		if (pStripsX)
		{
			auto pInput = pStripsX;
			auto pOutput = m_pColors + begin * colorPitch;

			for (int seg = begin; seg < end; seg++)
			{
				memcpy(pOutput, pInput, m_size.w * sizeof(HiColor));
				pInput += m_size.w;
				pOutput += colorPitch;

				m_horrTintmaps[seg] = true;
			}
		}
		else
		{
			auto pOutput = m_pColors + begin * colorPitch;

			for (int seg = begin; seg < end; seg++)
			{
				for (int i = 0; i < m_size.w; i++)
					pOutput[i] = HiColor::White;

				pOutput += colorPitch;

				m_horrTintmaps[seg] = false;
			}
		}

		if (pStripsY)
		{
			auto pInput = pStripsY;
			auto pOutput = m_pColors + begin * colorPitch + m_size.w;

			for (int seg = begin; seg < end; seg++)
			{
				memcpy(pOutput, pInput, m_size.h * sizeof(HiColor));
				pInput += m_size.h;
				pOutput += colorPitch;

				m_vertTintmaps[seg] = true;
			}
		}
		else
		{
			auto pOutput = m_pColors + begin * colorPitch + m_size.w;

			for (int seg = begin; seg < end; seg++)
			{
				for (int i = 0; i < m_size.h; i++)
					pOutput[i] = HiColor::White;

				pOutput += colorPitch;

				m_vertTintmaps[seg] = false;
			}
		}

		_colorsUpdated(begin, end);

		return true;
	}
*/

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

		if( int(bp.colors != nullptr) + int(bp.gradients != nullptr) + int(bp.tintmaps != nullptr) != 1 )
			return false;

		return true;
	}


} // namespace wg
