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

	ID3D12Device * DX12Edgemap::s_pDevice = nullptr;
	DX12Edgemap *  DX12Edgemap::s_pFirstLive = nullptr;

	//____ setDevice() _________________________________________________________

	bool DX12Edgemap::setDevice( ID3D12Device * pDevice )
	{
		s_pDevice = pDevice;
		return true;
	}

	//____ exitDevice() ________________________________________________________

	void DX12Edgemap::exitDevice()
	{
		s_pDevice = nullptr;
	}


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

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
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

		p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
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

		if( !s_pDevice )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
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

		_releaseBuffer(m_newBuffer);
		_releaseBuffer(m_buffer);
	}

	//____ _evacuateAll() ________________________________________________________

	void DX12Edgemap::_evacuateAll()
	{
		// Everything in the buffer is worked out from what Edgemap keeps in memory,
		// so nothing is lost.

		for( DX12Edgemap * p = s_pFirstLive; p; p = p->m_pNextLive )
		{
			p->_releaseBuffer(p->m_newBuffer);
			p->_releaseBuffer(p->m_buffer);
			p->m_pGpuData = nullptr;
			p->m_bHasTintSlots = false;
		}
	}

	//____ _migrateAll() _________________________________________________________

	void DX12Edgemap::_migrateAll()
	{
		if( !s_pDevice )
			return;

		for( DX12Edgemap * p = s_pFirstLive; p; p = p->m_pNextLive )
		{
			if( !p->m_buffer )
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

	float * DX12Edgemap::_allocBuffer( size_t bytes )
	{
		_releaseBuffer(m_newBuffer);

		// Nothing can be created on a lost device, and trying costs an exception
		// inside the D3D12 runtime. The edgemap is left with the buffer it has,
		// or without one, as when creating one fails, and DX12Backend draws
		// nothing for it.

		if( DX12Backend::isDeviceLost() || !s_pDevice )
			return nullptr;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(bytes);
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
		float * pBuffer = nullptr;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(buffer.GetAddressOf()))) )
		{
			// A lost device is reported once by DX12Backend, not by every edgemap.

			if( !DX12Backend::checkDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for edgemap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		// Named after us, so the debug layer's live object report shows where it comes from.
		{
			wchar_t name[96];
			swprintf( name, 96, L"Wax DX12Edgemap %p: buffer", (const void*) this );
			buffer->SetName( name );
		}

		D3D12_RANGE readRange = { 0, 0 };			// We only write.

		if( FAILED(buffer->Map(0, &readRange, (void**) &pBuffer)) )
		{
			if( !DX12Backend::checkDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map buffer for edgemap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		m_newBuffer = buffer;
		return pBuffer;
	}

	//____ _commitBuffer() _______________________________________________________
	//
	// Replacing a buffer we had happens once per edgemap at most, when it gets its
	// first tint. The GPU may still be reading the old one, so we wait for it
	// before letting go.

	void DX12Edgemap::_commitBuffer()
	{
		if( m_buffer )
		{
			DX12Backend::waitForCompletionOfAll();
			_releaseBuffer(m_buffer);
		}

		m_buffer = m_newBuffer;
		m_newBuffer = nullptr;
	}

	//____ _releaseBuffer() ______________________________________________________

	void DX12Edgemap::_releaseBuffer( Microsoft::WRL::ComPtr<ID3D12Resource>& buffer )
	{
		if( buffer )
		{
			buffer->Unmap(0, nullptr);
			buffer = nullptr;
		}
	}

} // namespace wax

