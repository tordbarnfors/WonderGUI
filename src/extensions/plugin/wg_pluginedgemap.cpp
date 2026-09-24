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

		// Our copies of flat colors and tints. The Edgemap destructor frees them.

		m_pBuffer			= new char[m_nbSegments * sizeof(HiColor)];
		m_pFlatColors		= (HiColor*) m_pBuffer;
		m_pTints			= new Tint_p[m_nbSegments];

		const wg_color * pFlatColors = PluginCalls::edgemap->edgemapFlatColors(object);

		for( int i = 0 ; i < m_nbSegments ; i++ )
		{
			m_pFlatColors[i] = pFlatColors ? reinterpret_cast<const HiColor*>(pFlatColors)[i] : HiColor::Transparent;
			m_pTints[i] = PluginCalls::_localTint( PluginCalls::edgemap->edgemapTint(object, i) );
			if( m_pTints[i] )
				m_nbTints++;
		}

		m_bConstructed = true;
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
		if( !Edgemap::setColors(begin, end, pColors) )
			return false;

		return (bool) PluginCalls::edgemap->setEdgemapColors( m_cEdgemap, begin, end, reinterpret_cast<const wg_color *>(pColors));
	}

	bool PluginEdgemap::setColors( int begin, int end, const Tint_p * pTints )
	{
		if( !Edgemap::setColors(begin, end, pTints) )
			return false;

		// Host copies of the tints, released once the host has its own references.

		wg_obj	hostTints[Edgemap::maxSegments];

		for( int i = begin ; i < end ; i++ )
			hostTints[i-begin] = PluginCalls::_hostTint( pTints[i-begin] );

		bool retVal = (bool) PluginCalls::edgemap->setEdgemapTints( m_cEdgemap, begin, end, hostTints );

		for( int i = begin ; i < end ; i++ )
		{
			if( hostTints[i-begin] )
				PluginCalls::object->release( hostTints[i-begin] );
		}

		return retVal;
	}

	//____ exportBounds() ________________________________________________________

	void PluginEdgemap::exportBounds( spx * pMinMaxOutput, int nSections, int sectionWidth,
				 int topEdge, int bottomEdge, int mapOffset, int minMaxPitch )
	{
		PluginCalls::edgemap->exportBounds( m_cEdgemap, pMinMaxOutput, nSections, sectionWidth, topEdge, bottomEdge, mapOffset, minMaxPitch );
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

	void PluginEdgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		// Nothing to do, setColors() passes changes on to the host.
	}


} // namespace wg
