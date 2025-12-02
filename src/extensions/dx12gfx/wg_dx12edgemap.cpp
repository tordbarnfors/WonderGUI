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

#include <wg_dx12edgemap.h>
#include <wg_gradyent.h>

#include <cstring>

namespace wg
{

	const TypeInfo DX12Edgemap::TYPEINFO = { "DX12Edgemap", &Edgemap::TYPEINFO };


	//____ create() ______________________________________________________________

	DX12Edgemap_p	DX12Edgemap::create(const Blueprint& blueprint)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		return DX12Edgemap_p(new DX12Edgemap(blueprint));
	}

	DX12Edgemap_p DX12Edgemap::create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = DX12Edgemap_p(new DX12Edgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	DX12Edgemap_p DX12Edgemap::create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = DX12Edgemap_p(new DX12Edgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	DX12Edgemap::DX12Edgemap(const Blueprint& bp) : Edgemap(bp)
	{
	}

	//____ destructor ____________________________________________________________

	DX12Edgemap::~DX12Edgemap()
	{
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& DX12Edgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _samplesUpdated() _______________________________________________________

	void DX12Edgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
	}

	//____ _colorsUpdated() ________________________________________________________

	void DX12Edgemap::_colorsUpdated(int beginColor, int endColor)
	{
	}


} // namespace wg

