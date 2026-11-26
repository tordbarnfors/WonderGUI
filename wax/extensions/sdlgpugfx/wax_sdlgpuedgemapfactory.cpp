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

#include <wax_sdlgpuedgemapfactory.h>
#include <wax_sdlgpuedgemap.h>

namespace wax
{



	const TypeInfo SdlGpuEdgemapFactory::TYPEINFO = { "SdlGpuEdgemapFactory", &EdgemapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SdlGpuEdgemapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ createEdgemap() __________________________________________________________


	Edgemap_p SdlGpuEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint)
	{
		return SdlGpuEdgemap::create(blueprint);
	}

	Edgemap_p SdlGpuEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigin origin, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		return SdlGpuEdgemap::create(blueprint, origin, pSamples, edges, edgePitch, samplePitch);
	}

	Edgemap_p SdlGpuEdgemapFactory::createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigin origin, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		return SdlGpuEdgemap::create(blueprint, origin, pSamples, edges, edgePitch, samplePitch);
	}


} // namespace wax
