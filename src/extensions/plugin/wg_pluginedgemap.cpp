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

#include <wg_plugincalls.h>
#include <wg_pluginedgemap.h>
#include <wg_base.h>

namespace wg
{
	const TypeInfo PluginEdgemap::TYPEINFO = { "PluginEdgemap", &Edgemap::TYPEINFO };


	//____ create ______________________________________________________________

	PluginEdgemap_p PluginEdgemap::create( wg_obj object)
	{
		return PluginEdgemap_p(new PluginEdgemap(object));
	}

	//____ constructor _____________________________________________________________

	PluginEdgemap::PluginEdgemap(wg_obj object)
	{
		PluginCalls::object->retain(object);

		m_cEdgemap = object;

		// We have initialized with an empty Blueprint. Now lets set correct values.

		wg_sizeI pixSize	= PluginCalls::edgemap->edgemapPixelSize(object);
		m_size				= *(SizeI*)&pixSize;
		m_nbSegments		= PluginCalls::edgemap->edgemapSegments(object);
		m_nbRenderSegments  = PluginCalls::edgemap->getRenderSegments(object);
		m_paletteType		= (EdgemapPalette) PluginCalls::edgemap->edgemapPaletteType(object);
	}

	//____ Destructor ______________________________________________________________

	PluginEdgemap::~PluginEdgemap()
	{
		PluginCalls::object->release(m_cEdgemap);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginEdgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setRenderSegments() ___________________________________________________

	bool PluginEdgemap::setRenderSegments(int nSegments)
	{
		if( !Edgemap::setRenderSegments(nSegments) )
		   return false;

		   PluginCalls::edgemap->setRenderSegments(m_cEdgemap, nSegments);
		return true;
	}

	//____ setColors() ___________________________________________________________

	bool PluginEdgemap::setColors( int begin, int end, const HiColor * pColors )
	{
		return (bool) PluginCalls::edgemap->setEdgemapColors( m_cEdgemap, begin, end, reinterpret_cast<const wg_color *>(pColors));
	}

	bool PluginEdgemap::setColors( int begin, int end, const Gradient * pGradients)
	{
		return (bool) PluginCalls::edgemap->setEdgemapColorsFromGradients( m_cEdgemap, begin, end, reinterpret_cast<const wg_gradient *>(pGradients));

	}

	bool PluginEdgemap::setColors( int begin, int end, const Tintmap_p * pTintmaps )
	{
		if (m_paletteType == EdgemapPalette::Flat)
			return false;

		//TODO: Also check so that the tintmaps don't tint a direction we don't have colorstrips for.

		int entries = end - begin;
		int nHorr = m_paletteType == EdgemapPalette::ColorstripX || m_paletteType == EdgemapPalette::ColorstripXY ? m_size.w * entries: 0;
		int nVert = m_paletteType == EdgemapPalette::ColorstripY || m_paletteType == EdgemapPalette::ColorstripXY ? m_size.h * entries: 0;

		int mem = (nHorr + nVert) * sizeof(HiColor);

		auto pDest = (HiColor *) Base::memStackAlloc( mem );

		const wg_color * pColorStripX = nullptr;
		const wg_color * pColorStripY = nullptr;

		if (nHorr > 0 )
		{
			pColorStripX = reinterpret_cast<const wg_color *>(pDest);
			auto pMaps = pTintmaps;

			for (int i = 0 ; i < entries ; i++)
			{
				Tintmap* pMap = *pMaps++;

				pMap->exportHorizontalColors(m_size.w, pDest);
				pDest += m_size.w;
			}
		}

		if (nVert > 0)
		{
			pColorStripY = reinterpret_cast<const wg_color *>(pDest);
			auto pMaps = pTintmaps;

			for (int i = 0 ; i < entries ; i++)
			{
				Tintmap* pMap = *pMaps++;

				pMap->exportVerticalColors(m_size.h, pDest);
				pDest += m_size.h;
			}
		}

		bool retVal = (bool) PluginCalls::edgemap->setEdgemapColorsFromStrips( m_cEdgemap, begin, end, pColorStripX, pColorStripY );
		Base::memStackFree( mem );
		return retVal;
	}

	bool PluginEdgemap::setColors( int begin, int end, const HiColor * pColorstripsX, const HiColor * pColorstripsY)
	{
		return (bool) PluginCalls::edgemap->setEdgemapColorsFromStrips( m_cEdgemap, begin, end,
					reinterpret_cast<const wg_color *>(pColorstripsX), reinterpret_cast<const wg_color *>(pColorstripsY));
	}

	//____ importPaletteEntries() ________________________________________________

	bool PluginEdgemap::importPaletteEntries( int begin, int end, const HiColor * pColors )
	{
		return (bool) PluginCalls::edgemap->importPaletteEntries( m_cEdgemap, begin, end, reinterpret_cast<const wg_color *>(pColors) );
	}

	//____ flatColors() __________________________________________________________

	const HiColor*  PluginEdgemap::flatColors() const
	{
		const wg_color * pColors = PluginCalls::edgemap->edgemapFlatColors( m_cEdgemap );
		return reinterpret_cast<const HiColor *>(pColors);
	}

	//____ colorstripsX() ________________________________________________________

	const HiColor*  PluginEdgemap::colorstripsX() const
	{
		const wg_color * pColors = PluginCalls::edgemap->edgemapColorstripsX( m_cEdgemap );
		return reinterpret_cast<const HiColor *>(pColors);
	}

	//____ colorstripsY() ________________________________________________________

	const HiColor*  PluginEdgemap::colorstripsY() const
	{
		const wg_color * pColors = PluginCalls::edgemap->edgemapColorstripsY( m_cEdgemap );
		return reinterpret_cast<const HiColor *>(pColors);
	}

	//____ exportLegacyPalette() _________________________________________________

	void PluginEdgemap::exportLegacyPalette( HiColor * pDest ) const
	{
		// Deprecated method!

		assert(false);
	}

	//____ _importSamples() ______________________________________________________

	void  PluginEdgemap::_importSamples(SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
								   int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		PluginCalls::edgemap->importSpxSamples(m_cEdgemap, (wg_sampleOrigo) origo, (const wg_spx*) pSource, edgeBegin, edgeEnd,
			sampleBegin, sampleEnd, edgePitch, samplePitch);

	}

	void  PluginEdgemap::_importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
								   int sampleBegin, int sampleEnd, int edgePitch, int samplePitch)
	{
		PluginCalls::edgemap->importFloatSamples(m_cEdgemap, (wg_sampleOrigo) origo, pSource, edgeBegin, edgeEnd,
			sampleBegin, sampleEnd, edgePitch, samplePitch);
	}

	//____ _samplesUpdated() _____________________________________________________

	void PluginEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		// Should never get here!

		assert(false);
/*
		int samplePitch = m_nbSegments -1;

		spx * pSource = m_pSamples + edgeBegin + sampleBegin * samplePitch;
		
		PluginCalls::edgemap->importSpxSamples(m_cEdgemap, WG_WAVEORIGO_TOP, pSource, edgeBegin, edgeEnd,
			sampleBegin, sampleEnd, 1, samplePitch);
 */
	}

	//____ _colorsUpdated() ______________________________________________________

	void PluginEdgemap::_colorsUpdated(int beginColor, int endColor)
	{
		// Should never get here!

		assert(false);

/*
		HiColor * pColors = m_pPalette + beginColor;

		PluginCalls::edgemap->importPaletteEntries( m_cEdgemap, beginColor, endColor, (const wg_color*) pColors);
 */
	}


} // namespace wg
