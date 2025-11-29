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
#ifndef WG_STREAMEDGEMAPFACTORY_DOT_H
#define	WG_STREAMEDGEMAPFACTORY_DOT_H
#pragma once


#include <wg_edgemapfactory.h>
#include <wg_streamencoder.h>

namespace wg
{


	class StreamEdgemapFactory;
	typedef	StrongPtr<StreamEdgemapFactory>	StreamEdgemapFactory_p;
	typedef	WeakPtr<StreamEdgemapFactory>	StreamEdgemapFactory_wp;


	//____ StreamEdgemapFactory _______________________________________________________

	class StreamEdgemapFactory : public EdgemapFactory
	{
	public:

		//.____ Creation __________________________________________

		static StreamEdgemapFactory_p	create( StreamEncoder * pEncoder ) { return StreamEdgemapFactory_p(new StreamEdgemapFactory(pEncoder)); }


		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint) override;	
		Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch = 0, int samplePitch = 0) override;
		Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch = 0, int samplePitch = 0) override;

	protected:
		StreamEdgemapFactory(StreamEncoder * pEncoder) : m_pEncoder(pEncoder) {}
		virtual ~StreamEdgemapFactory() {}
		
		StreamEncoder_p	m_pEncoder;
	};


	//==============================================================================

} // namespace wg
#endif // WG_STREAMEDGEMAPFACTORY_DOT_H






