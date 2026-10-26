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
#include <wg_softedgemapfactory.h>
#include <wg_softedgemap.h>

namespace wg
{



	const TypeInfo SoftEdgemapFactory::TYPEINFO = { "SoftEdgemapFactory", &EdgemapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftEdgemapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ createEdgemap() __________________________________________________________


	Edgemap_p SoftEdgemapFactory::createEdgemap( const Edgemap::Blueprint& blueprint )
	{
		return SoftEdgemap::create(blueprint);
	}	

	Edgemap_p SoftEdgemapFactory::createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch )
	{
		return SoftEdgemap::create(blueprint, origo, pSamples, edges, edgePitch, samplePitch);
	}

	Edgemap_p SoftEdgemapFactory::createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch, int samplePitch )
	{
		return SoftEdgemap::create(blueprint, origo, pSamples, edges, edgePitch, samplePitch);
	}


} // namespace wg
