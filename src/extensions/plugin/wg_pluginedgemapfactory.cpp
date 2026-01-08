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
#include <memory.h>

#include <wg_plugincalls.h>
#include <wg_pluginedgemap.h>
#include <wg_pluginedgemapfactory.h>

#include <wg_gfxbase.h>


namespace wg
{

	const TypeInfo PluginEdgemapFactory::TYPEINFO = { "PluginEdgemapFactory", &EdgemapFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	PluginEdgemapFactory::PluginEdgemapFactory( wg_obj cFactory) : m_cFactory(cFactory)
	{
		/*
		 TODO: Solve this in a more correct way:
		 
		 (explanation below is from PluginSurfaceFactory, but principle is the same)
		 
		 We skip retaining/releasing the hosts SurfaceFactory used by us to make curent
		 Softube close down sequence work where PluginSurfaceFactory (due to a static pointer)
		 is destroyed AFTER the PluginCalls-structure has been cleared.
		 We therefore can not call release() in our destructor.

		 We therefore rely on the hosts real SurfaceFactory not being deleted until
		 this class is not used by us anymore. This is safe to assume in the Softube GUI
		 framework, but could cause issues with other future uses of WonderGUI plugins
		 implementations.
		 */
//		PluginCalls::object->retain(cFactory);
	}

	//____ destructor ________________________________________________________

	PluginEdgemapFactory::~PluginEdgemapFactory()
	{
		//TODO: Solve this in a more correct way. See commen in constructor.
		
//		PluginCalls::object->release(m_cFactory);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginEdgemapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ createEdgemap() ________________________________________________________

	Edgemap_p PluginEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint)
	{
		bpTranslator cBP(blueprint);

		auto obj = PluginCalls::edgemapFactory->createEdgemap(m_cFactory, cBP.bp());
		auto pEdgemap = PluginEdgemap::create(obj);
		PluginCalls::object->release(obj);
		return pEdgemap;
	}

	Edgemap_p PluginEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		bpTranslator cBP(blueprint);

		auto obj = PluginCalls::edgemapFactory->createEdgemapFromFloats(m_cFactory, cBP.bp(), (wg_sampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
		auto pEdgemap = PluginEdgemap::create(obj);
		PluginCalls::object->release(obj);
		return pEdgemap;
	}

	Edgemap_p PluginEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		bpTranslator cBP(blueprint);

		auto obj = PluginCalls::edgemapFactory->createEdgemapFromSpx(m_cFactory, cBP.bp(), (wg_sampleOrigo)origo, pSamples, edges, edgePitch, samplePitch);
		auto pEdgemap = PluginEdgemap::create(obj);
		PluginCalls::object->release(obj);
		return pEdgemap;
	}

	PluginEdgemapFactory::bpTranslator::bpTranslator( const Edgemap::Blueprint& cppBP )
	{
		m_memReserved = 0;

		m_cBP.colors = (wg_color*) cppBP.colors;
		m_cBP.colorstripsX = (wg_color*) cppBP.colorstripsX;
		m_cBP.colorstripsY = (wg_color*) cppBP.colorstripsY;
		m_cBP.paletteType = (wg_edgemapPalette) cppBP.paletteType;
		m_cBP.tintmaps = nullptr;
		m_cBP.segments = cppBP.segments;
		m_cBP.size.w = cppBP.size.w;
		m_cBP.size.h = cppBP.size.h;

		if( cppBP.tintmaps != nullptr )
		{
			bool bHorr = false;
			bool bVert = false;

			for( int i = 0 ; i < cppBP.segments ; i++ )
			{
				if( cppBP.tintmaps[i]->isHorizontal() )
					bHorr = true;

				if( cppBP.tintmaps[i]->isVertical() )
					bVert = true;
			}

			m_memReserved = (int(bHorr) * cppBP.size.w + int(bVert) * cppBP.size.h) * cppBP.segments * sizeof(HiColor);

			auto pColorstrips = (HiColor*) GfxBase::memStackAlloc(m_memReserved);

			HiColor * pColorstripsX = bHorr ? pColorstrips : nullptr;
			int incX = bHorr ? cppBP.size.w : 0;

			HiColor * pColorstripsY = bVert ? pColorstrips + incX * cppBP.segments : nullptr;
			int incY = bVert ? cppBP.size.h : 0;

			m_cBP.colorstripsX = (wg_color*) pColorstripsX;
			m_cBP.colorstripsY = (wg_color*) pColorstripsY;

			for( int i = 0 ; i < cppBP.segments ; i++ )
			{
				cppBP.tintmaps[i]->exportColors(cppBP.size, pColorstripsX, pColorstripsY);
				pColorstripsX += incX;
				pColorstripsY += incY;
			}
		}
	}

	PluginEdgemapFactory::bpTranslator::~bpTranslator()
	{
		if( m_memReserved > 0 )
			GfxBase::memStackFree(m_memReserved);
	}




} // namespace wg
