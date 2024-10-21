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

		// Setup buffers

		int sampleArraySize = (bp.size.w + 1) * bp.segments * sizeof(spx);
		int colorArraySize = (bp.size.w + bp.size.h) * bp.segments * sizeof(HiColor);	// We always reserve memory for a full set of tintmap colors.
		int gradientArraySize = bp.segments * sizeof(Gradient);							// We keep a set of gradients for backward compatiblity

		int bytes = sampleArraySize + colorArraySize + gradientArraySize;

		m_pBuffer = new char[bytes];

		m_pSamples = (spx*)m_pBuffer;
		m_pColors = (HiColor*)(m_pBuffer + sampleArraySize);
		m_pGradients = (Gradient*)(m_pBuffer + sampleArraySize + colorArraySize);

		int colorPitch = bp.size.w + bp.size.h;

		if (bp.colorstripsX || bp.colorstripsY)
		{
			if (bp.colorstripsX)
			{
				auto pInput = bp.colorstripsX;
				auto pOutput = m_pColors;

				for (int seg = 0; seg < bp.segments; seg++)
				{
					memcpy(pOutput, pInput, bp.size.w * sizeof(HiColor));
					pInput += bp.size.w;
					pOutput += colorPitch;

					m_horrTintmaps[seg] = true;
				}
			}
			else
			{
				auto pOutput = m_pColors;

				for (int seg = 0; seg < bp.segments; seg++)
				{
					for (int i = 0; i < bp.size.w; i++)
						pOutput[i] = HiColor::White;

					pOutput += colorPitch;
					m_horrTintmaps[seg] = false;
				}
			}

			if (bp.colorstripsY)
			{
				auto pInput = bp.colorstripsY;
				auto pOutput = m_pColors + bp.size.w;

				for (int seg = 0; seg < bp.segments; seg++)
				{
					memcpy(pOutput, pInput, bp.size.h * sizeof(HiColor));
					pInput += bp.size.h;
					pOutput += colorPitch;

					m_vertTintmaps[seg] = true;
				}
			}
			else
			{
				auto pOutput = m_pColors + bp.size.w;

				for (int seg = 0; seg < bp.segments; seg++)
				{
					for (int i = 0; i < bp.size.h; i++)
						pOutput[i] = HiColor::White;

					pOutput += colorPitch;
					m_vertTintmaps[seg] = false;
				}
			}
		}
		else
		{
			for (int seg = 0; seg < bp.segments; seg++)
			{
				auto pOutput = m_pColors + (seg * colorPitch);

				if (bp.tintmaps && bp.tintmaps[seg])
				{
					auto pTintmap = bp.tintmaps[seg];

					pTintmap->exportHorizontalColors(bp.size.w * 64, pOutput);
					pOutput += bp.size.w;
					pTintmap->exportVerticalColors(bp.size.h * 64, pOutput);

					if (pTintmap->isHorizontal())
						m_horrTintmaps.set(seg);

					if (pTintmap->isVertical())
						m_vertTintmaps.set(seg);

					m_pGradients[seg] = pTintmap->exportGradient();
				}
				else if (bp.gradients)
				{
					auto& gradient = bp.gradients[seg];

					auto pTintmap = Gradyent::create(gradient);

					pTintmap->exportHorizontalColors(bp.size.w * 64, pOutput);
					pOutput += bp.size.w;
					pTintmap->exportVerticalColors(bp.size.h * 64, pOutput);

					if (pTintmap->isHorizontal())
						m_horrTintmaps.set(seg);

					if (pTintmap->isVertical())
						m_vertTintmaps.set(seg);

					m_pGradients[seg] = pTintmap->exportGradient();
				}
				else if (bp.colors)
				{
					for (int i = 0; i < bp.size.w; i++)
						*pOutput++ = bp.colors[seg];

					for (int i = 0; i < bp.size.h; i++)
						*pOutput++ = HiColor::White;

					m_pGradients[seg] = Gradient(bp.colors[seg], bp.colors[seg], bp.colors[seg], bp.colors[seg]);

				}
				else
				{
					for (int i = 0; i < bp.size.w + bp.size.h; i++)
						*pOutput++ = HiColor::Transparent;

					m_pGradients[seg] = Gradient(HiColor::Transparent, HiColor::Transparent, HiColor::Transparent, HiColor::Transparent);
				}

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
		int pitch = m_size.w + m_size.h;

		auto pOutput = m_pColors + begin * pitch;

		for (int seg = begin; seg < end; seg++)
		{
			for (int i = 0; i < m_size.w; i++)
				*pOutput++ = *pColors;

			for (int i = 0; i < m_size.h; i++)
				*pOutput++ = HiColor::White;

			m_horrTintmaps.reset(seg);
			m_vertTintmaps.reset(seg);

			pColors++;
		}

		_colorsUpdated(begin, end);
		return true;
	}

	//____ setGradients() _________________________________________________________

	bool Edgemap::setGradients(int begin, int end, const Gradient* pGradients)
	{
		int pitch = m_size.w + m_size.h;

		auto pOutput = m_pColors + begin * pitch;

		for (int seg = begin; seg < end; seg++)
		{
			auto& gradient = *pGradients++;

			if (gradient.topLeft.a > 0 || gradient.topRight.a > 0 || gradient.bottomLeft.a > 0 || gradient.bottomRight.a > 0)
			{
				auto pTintmap = Gradyent::create(gradient);

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
		return true;
	}

	//____ setTintmaps() __________________________________________________________

	bool Edgemap::setTintmaps(int begin, int end, const Tintmap_p* pTintmaps)
	{
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
		return true;
	}

	//____ setColorstrips() ____________________________________________________

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
