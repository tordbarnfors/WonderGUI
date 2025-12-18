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
		auto obj = PluginCalls::edgemapFactory->createEdgemap(m_cFactory, (wg_edgemapBP*) &blueprint);
		auto pEdgemap = PluginEdgemap::create(obj);
		PluginCalls::object->release(obj);
		return pEdgemap;
	}

	Edgemap_p PluginEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		auto obj = PluginCalls::edgemapFactory->createEdgemapFromFloats(m_cFactory, (wg_edgemapBP*)&blueprint, (wg_sampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
		auto pEdgemap = PluginEdgemap::create(obj);
		PluginCalls::object->release(obj);
		return pEdgemap;
	}

	Edgemap_p PluginEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		auto obj = PluginCalls::edgemapFactory->createEdgemapFromSpx(m_cFactory, (wg_edgemapBP*)&blueprint, (wg_sampleOrigo)origo, pSamples, edges, edgePitch, samplePitch);
		auto pEdgemap = PluginEdgemap::create(obj);
		PluginCalls::object->release(obj);
		return pEdgemap;
	}


} // namespace wg
