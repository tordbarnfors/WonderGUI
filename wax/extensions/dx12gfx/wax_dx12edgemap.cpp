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


#include <wax_dx12edgemap.h>
#include <wax_dx12backend.h>
#include <wax_tinttools.h>
#include <wax_base.h>

#include <cstring>
#include <cwchar>
#include <algorithm>
#include <utility>

namespace wax
{

	const TypeInfo DX12Edgemap::TYPEINFO = { "DX12Edgemap", &GpuEdgemap::TYPEINFO };

	DX12Edgemap *  DX12Edgemap::s_pFirstLive = nullptr;


	//____ create() ______________________________________________________________

	DX12Edgemap_p	DX12Edgemap::create(const Blueprint& blueprint)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		return DX12Edgemap_p(new DX12Edgemap(blueprint));
	}

	DX12Edgemap_p DX12Edgemap::create(const Blueprint& blueprint, SampleOrigin origin, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = DX12Edgemap_p(new DX12Edgemap(blueprint));

		p->_importSamples(origin, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	DX12Edgemap_p DX12Edgemap::create(const Blueprint& blueprint, SampleOrigin origin, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = DX12Edgemap_p(new DX12Edgemap(blueprint));

		p->_importSamples(origin, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	DX12Edgemap::DX12Edgemap(const Blueprint& bp) : GpuEdgemap(bp)
	{
		_link();

		// Nothing can be created on a lost device. The edgemap is left without a
		// buffer, as when creating one fails, and DX12Backend draws nothing for it
		// until a new device is set, which gives it one, see _migrateAll().

		if( DX12Backend::isDeviceLost() )
			return;

		if( !DX12Backend::device() )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend must be created before any edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		_createContent();
	}

	//____ destructor ____________________________________________________________

	DX12Edgemap::~DX12Edgemap()
	{
		_unlink();

		DX12BufferPool::free(m_newBuffer);
		DX12BufferPool::free(m_buffer);
	}

	//____ _evacuateAll() ________________________________________________________

	void DX12Edgemap::_evacuateAll()
	{
		// Everything in the buffer is worked out from what Edgemap keeps in memory,
		// so nothing is lost.

		for( DX12Edgemap * p = s_pFirstLive; p; p = p->m_pNextLive )
		{
			DX12BufferPool::free(p->m_newBuffer);
			DX12BufferPool::free(p->m_buffer);
			p->m_pGpuData = nullptr;
			p->m_bHasTintSlots = false;
		}
	}

	//____ _migrateAll() _________________________________________________________

	void DX12Edgemap::_migrateAll()
	{
		if( !DX12Backend::device() )
			return;

		for( DX12Edgemap * p = s_pFirstLive; p; p = p->m_pNextLive )
		{
			if( !DX12BufferPool::isValid(p->m_buffer) )
				p->_createContent();
		}
	}

	//____ _link() _______________________________________________________________

	void DX12Edgemap::_link()
	{
		m_pNextLive = s_pFirstLive;

		if( s_pFirstLive )
			s_pFirstLive->m_pPrevLive = this;

		s_pFirstLive = this;
	}

	//____ _unlink() _____________________________________________________________

	void DX12Edgemap::_unlink()
	{
		if( m_pPrevLive )
			m_pPrevLive->m_pNextLive = m_pNextLive;
		else
			s_pFirstLive = m_pNextLive;

		if( m_pNextLive )
			m_pNextLive->m_pPrevLive = m_pPrevLive;

		m_pPrevLive = nullptr;
		m_pNextLive = nullptr;
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& DX12Edgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _allocBuffer() ________________________________________________________
	//
	//TODO: There is one buffer and two frames in flight, so updating samples can
	// change what the GPU is still reading for the previous frame.
	//
	// Nothing can be created on a lost device, or without one. The edgemap is then
	// left with the buffer it has, or without one, as when creating one fails, and
	// DX12Backend draws nothing for it.

	float * DX12Edgemap::_allocBuffer( size_t bytes )
	{
		DX12BufferPool::free(m_newBuffer);

		if( !DX12BufferPool::alloc(int(bytes), m_newBuffer) )
			return nullptr;

		return (float*) m_newBuffer.pData;
	}

	//____ _commitBuffer() _______________________________________________________
	//
	// Replacing a buffer we had happens once per edgemap at most, when it gets its
	// first tint. The GPU may still be reading the old one, which the pool waits
	// for before it hands out its memory again.

	void DX12Edgemap::_commitBuffer()
	{
		DX12BufferPool::free(m_buffer);

		m_buffer = m_newBuffer;
		m_newBuffer = DX12BufferPool::Buffer();
	}

} // namespace wax
