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

#include <wg_dx12warpmap.h>
#include <wg_dx12backend.h>
#include <wg_gfxbase.h>

#include <cstring>
#include <cwchar>
#include <algorithm>

namespace wg
{

	const TypeInfo DX12Warpmap::TYPEINFO = { "DX12Warpmap", &Warpmap::TYPEINFO };

	//____ create() ______________________________________________________________

	DX12Warpmap_p DX12Warpmap::create(const Blueprint& bp)
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return DX12Warpmap_p(new DX12Warpmap(bp));
	}

	DX12Warpmap_p DX12Warpmap::create(const Blueprint& bp, const CoordF * pNodes, int pitch)
	{
		if (!_validateBlueprint(bp) || !pNodes)
			return nullptr;

		auto p = DX12Warpmap_p(new DX12Warpmap(bp));

		SizeI nodes = p->nodes();
		p->importNodes({ 0, 0, nodes.w, nodes.h }, pNodes, pitch);
		return p;
	}

	//____ constructor ___________________________________________________________

	DX12Warpmap::DX12Warpmap(const Blueprint& bp) : Warpmap(bp)
	{
		ID3D12Device * pDevice = DX12Backend::device();

		if (!pDevice)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend::setDevice() must be called before any warpmap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Nothing can be created on a lost device. The warpmap is left without a
		// buffer and DX12Backend draws nothing for it.

		if (DX12Backend::isDeviceLost())
			return;

		SizeI cells = _cells();
		size_t bytes = size_t(cells.w) * cells.h * 2 * 4 * sizeof(float);

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

		if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_buffer.GetAddressOf()))))
		{
			// A lost device is reported once by DX12Backend, not by every warpmap.

			if (!DX12Backend::checkDeviceLost())
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for warpmap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			m_buffer = nullptr;
			return;
		}

		// Named after us, so the debug layer's live object report shows where it comes from.
		{
			wchar_t name[96];
			swprintf(name, 96, L"WonderGUI DX12Warpmap %p: buffer", (const void*) this);
			m_buffer->SetName(name);
		}

		D3D12_RANGE readRange = { 0, 0 };			// We only write.

		if (FAILED(m_buffer->Map(0, &readRange, (void**) &m_pBuffer)))
		{
			if (!DX12Backend::checkDeviceLost())
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map buffer for warpmap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			m_buffer = nullptr;
			m_pBuffer = nullptr;
			return;
		}

		//TODO: As for DX12Edgemap, there is one buffer and two frames in flight, so
		// updating nodes can change what the GPU is still reading for the previous frame.

		_nodesUpdated({ 0, 0, m_nodes.w, m_nodes.h });
	}

	//____ destructor ____________________________________________________________

	DX12Warpmap::~DX12Warpmap()
	{
		if (m_buffer && m_pBuffer)
			m_buffer->Unmap(0, nullptr);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& DX12Warpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _nodesUpdated() _______________________________________________________
	//
	// Rebuilds the rows of cells touching the nodes that changed.

	void DX12Warpmap::_nodesUpdated(const RectI& region)
	{
		if (!m_pBuffer)
			return;

		SizeI cells = _cells();

		int rowBeg = std::max(0, region.y - 1);
		int rowEnd = std::min(cells.h, region.y + region.h);

		float * pOut = m_pBuffer + rowBeg * cells.w * 2 * 4;

		for (int row = rowBeg; row < rowEnd; row++)
		{
			for (int column = 0; column < cells.w; column++)
			{
				CoordF corners[4];
				cellCorners(column, row, corners);

				for (int i = 0; i < 4; i++)
				{
					*pOut++ = corners[i].x;
					*pOut++ = corners[i].y;
				}
			}
		}
	}

} // namespace wg
