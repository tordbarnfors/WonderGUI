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

#ifndef WG_DX12EDGEMAPFACTORY_DOT_H
#define	WG_DX12EDGEMAPFACTORY_DOT_H
#pragma once


#include <wg_edgemapfactory.h>

namespace wg
{


	class DX12EdgemapFactory;
	typedef	StrongPtr<DX12EdgemapFactory>	DX12EdgemapFactory_p;
	typedef	WeakPtr<DX12EdgemapFactory>	DX12EdgemapFactory_wp;


	//____ DX12EdgemapFactory _______________________________________________________

	class DX12EdgemapFactory : public EdgemapFactory
	{
	public:

		//.____ Creation __________________________________________

		static DX12EdgemapFactory_p	create() { return DX12EdgemapFactory_p(new DX12EdgemapFactory()); }


		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Edgemap_p createEdgemap(const Edgemap::Blueprint& blueprint) override;
		Edgemap_p createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch = 0, int samplePitch = 0) override;
		Edgemap_p createEdgemap(const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch = 0, int samplePitch = 0) override;

	protected:
		DX12EdgemapFactory() {}
		virtual ~DX12EdgemapFactory() {}
	};


	//==============================================================================

} // namespace wg
#endif // WG_DX12EDGEMAPFACTORY_DOT_H






