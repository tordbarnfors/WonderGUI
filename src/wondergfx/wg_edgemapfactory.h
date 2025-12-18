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
#ifndef WG_EDGEMAPFACTORY_DOT_H
#define	WG_EDGEMAPFACTORY_DOT_H
#pragma once

#include <wg_edgemap.h>

namespace wg
{


	class EdgemapFactory;
	typedef	StrongPtr<EdgemapFactory>	EdgemapFactory_p;
	typedef	WeakPtr<EdgemapFactory>	EdgemapFactory_wp;


	//____ EdgemapFactory _______________________________________________________
	/**
	 * @brief Factory class for creating edgemaps.
	 *
	 * EdgemapFactory is the base class for all edgemap factories. The edgemap factories
	 * are used by WonderGUI objects that needs to dynamically create edgemaps as
	 * part of their operation.
	 *
	 **/
	class EdgemapFactory : public Object
	{
	public:

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		virtual Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint) = 0;
		virtual Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch = 0, int samplePitch = 0) = 0;
		virtual Edgemap_p createEdgemap( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch = 0, int samplePitch = 0) = 0;

	protected:
		EdgemapFactory() {}
		virtual ~EdgemapFactory() {}
	};


	//==============================================================================

} // namespace wg
#endif // WG_EDGEMAPFACTORY_DOT_H






