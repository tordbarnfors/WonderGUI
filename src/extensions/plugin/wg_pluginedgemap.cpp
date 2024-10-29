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


namespace wg
{
	const TypeInfo PluginEdgemap::TYPEINFO = { "PluginEdgemap", &Edgemap::TYPEINFO };


	//____ create ______________________________________________________________

	PluginEdgemap_p PluginEdgemap::create( wg_obj object)
	{
		return PluginEdgemap_p(new PluginEdgemap(object));
	}

	//____ constructor _____________________________________________________________

	PluginEdgemap::PluginEdgemap(wg_obj object) : Edgemap( Blueprint() )
	{
		PluginCalls::object->retain(object);

		m_cEdgemap = object;

		// We have initialized with an empty Blueprint. Now lets set correct values.

		wg_sizeI pixSize	= PluginCalls::edgemap->edgemapPixelSize(object);
		m_size				= *(SizeI*)&pixSize;
		m_nbSegments		= PluginCalls::edgemap->edgemapSegments(object);
		m_nbRenderSegments  = PluginCalls::edgemap->getRenderSegments(object);
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

	//____ setColors() ___________________________________________________________

	bool PluginEdgemap::setColors( int begin, int end, const HiColor * pColors )
	{
		return (bool) PluginCalls::edgemap->setEdgemapColors(m_cEdgemap, begin, end, (const wg_color*) pColors );
	}

	//____ setGradients() ________________________________________________________

	bool PluginEdgemap::setGradients( int begin, int end, const Gradient * pGradients )
	{
		return (bool) PluginCalls::edgemap->setEdgemapGradients(m_cEdgemap, begin, end, (const wg_gradient*) pGradients );
	}

	//____ colors() ______________________________________________________________

	const HiColor * PluginEdgemap::colors() const
	{
		return (const HiColor *) PluginCalls::edgemap->edgemapColors(m_cEdgemap);
	}

	//____ gradients() ___________________________________________________________

	const Gradient * PluginEdgemap::gradients() const
	{
		return (const Gradient *) PluginCalls::edgemap->edgemapGradients(m_cEdgemap);
	}

	//____ color() ____________________________________________________________

	HiColor PluginEdgemap::color(int segment) const
	{
		auto color = PluginCalls::edgemap->edgemapColor(m_cEdgemap, segment);
		return *(HiColor*)&color;
	}

	//____ gradient() _________________________________________________________

	Gradient PluginEdgemap::gradient(int segment) const
	{
		auto gradient = PluginCalls::edgemap->edgemapGradient(m_cEdgemap, segment);
		return *(Gradient*)&gradient;
	}

	//____ setRenderSegments() ___________________________________________________
	
	bool PluginEdgemap::setRenderSegments(int nSegments)
	{
		if( !Edgemap::setRenderSegments(nSegments) )
		   return false;

		   PluginCalls::edgemap->setRenderSegments(m_cEdgemap, nSegments);
		return true;
	}

	//____ _samplesUpdated() _____________________________________________________

	void PluginEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		int samplePitch = m_nbSegments -1;
		
		spx * pSource = m_pSamples + edgeBegin + sampleBegin * samplePitch;
		
		PluginCalls::edgemap->importSpxSamples(m_cEdgemap, WG_WAVEORIGO_TOP, pSource, edgeBegin, edgeEnd,
			sampleBegin, sampleEnd, 1, samplePitch);
	}

	//____ _colorsUpdated() ______________________________________________________

	void PluginEdgemap::_colorsUpdated(int beginColor, int endColor)
	{
		HiColor * pColors = m_pPalette + beginColor;
		
		PluginCalls::edgemap->importPaletteEntries( m_cEdgemap, beginColor, endColor, (const wg_color*) pColors);
	}


} // namespace wg
