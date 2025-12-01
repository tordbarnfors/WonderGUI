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

#ifndef WG_DX12EDGEMAP_DOT_H
#define WG_DX12EDGEMAP_DOT_H
#pragma once

#include <wg_edgemap.h>

namespace wg
{

	class DX12Edgemap;
	typedef StrongPtr<DX12Edgemap> DX12Edgemap_p;
	typedef WeakPtr<DX12Edgemap> DX12Edgemap_wp;

	class DX12Edgemap : public Edgemap
	{
		friend class SoftBackend;
		friend class LinearBackend;

	public:

		//.____ Creation __________________________________________

		static DX12Edgemap_p	create(const Blueprint& blueprint);
		static DX12Edgemap_p	create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);
		static DX12Edgemap_p	create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);


		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;


	protected:

		DX12Edgemap(const Blueprint& bp);
		~DX12Edgemap();

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginColor, int endColor) override;

	};



} // namespace wg
#endif // WG_DX12EDGEMAP_DOT_H
