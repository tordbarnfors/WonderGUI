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
#ifndef WG_METALEDGEMAPFACTORY_DOT_H
#define	WG_METALEDGEMAPFACTORY_DOT_H
#pragma once


#include <wg_edgemapfactory.h>

namespace wg
{


	class MetalEdgemapFactory;
	typedef	StrongPtr<MetalEdgemapFactory>	MetalEdgemapFactory_p;
	typedef	WeakPtr<MetalEdgemapFactory>	MetalEdgemapFactory_wp;


	//____ MetalEdgemapFactory _______________________________________________________

	class MetalEdgemapFactory : public EdgemapFactory
	{
	public:

		//.____ Creation __________________________________________

		static MetalEdgemapFactory_p	create() { return MetalEdgemapFactory_p(new MetalEdgemapFactory()); }


		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint) override;	
		Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch = 0, int samplePitch = 0) override;
		Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch = 0, int samplePitch = 0) override;

	protected:
		virtual ~MetalEdgemapFactory() {}
	};


	//==============================================================================

} // namespace wg
#endif // WG_GLEDGEMAPFACTORY_DOT_H






