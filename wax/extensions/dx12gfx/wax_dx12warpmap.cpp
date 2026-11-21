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

#include <wax_dx12warpmap.h>
#include <wax_dx12backend.h>
#include <wax_base.h>

#include <cstdint>
#include <cstring>
#include <cwchar>
#include <algorithm>

namespace wax
{

	const TypeInfo DX12Warpmap::TYPEINFO = { "DX12Warpmap", &Warpmap::TYPEINFO };

	std::vector<DX12Warpmap::SpareBuffer>	DX12Warpmap::s_spareBuffers;
	size_t									DX12Warpmap::s_spareBytes = 0;
	ID3D12Device *							DX12Warpmap::s_pSpareDevice = nullptr;

	// Spare buffers kept for new Warpmaps, at most this many bytes of them. Warpmaps that are
	// made anew for every frame need as many as they take for the frames in flight.

	static const size_t c_maxSpareBytes = 32 * 1024 * 1024;

	// Buffers are at least this large, which is what D3D12 sets aside for a committed resource
	// anyway, and a power of two above it, so that a spare buffer fits Warpmaps of about the
	// same size as the one it was made for.

	static const size_t c_minBufferBytes = 64 * 1024;

	//____ create() ______________________________________________________________

	DX12Warpmap_p DX12Warpmap::create(const Blueprint& bp)
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return DX12Warpmap_p(new DX12Warpmap(bp));
	}

	DX12Warpmap_p DX12Warpmap::create(const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles)
	{
		if (!pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles))
			return nullptr;

		return DX12Warpmap_p(new DX12Warpmap(bp, pVertices, pTriangles));
	}

	//____ constructor ___________________________________________________________

	DX12Warpmap::DX12Warpmap(const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles) : Warpmap(bp, pVertices, pTriangles)
	{
		static_assert(sizeof(Vertex) == 6 * sizeof(int32_t) && sizeof(Triangle) == 3 * sizeof(uint16_t), "Vertices and triangles are uploaded as they are");

		ID3D12Device * pDevice = DX12Backend::device();

		if (!pDevice)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend::setDevice() must be called before any warpmap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Nothing can be created on a lost device. The warpmap is left without a
		// buffer and DX12Backend draws nothing for it.

		if (DX12Backend::isDeviceLost())
			return;

		// Triangles start at an aligned offset after the vertices.

		m_indexOffset = (size_t(m_nVertices) * sizeof(Vertex) + 255) & ~size_t(255);
		size_t needed = m_indexOffset + size_t(m_nTriangles) * sizeof(Triangle);

		size_t bytes = c_minBufferBytes;
		while (bytes < needed)
			bytes *= 2;

		// A committed resource takes D3D12 long to create. Going by the WarpTests of
		// gfxdevicetester on Windows (Arc B580), which make their Warpmaps anew for each frame,
		// about 250 us, which took most of the time of drawing them. So a buffer left by an
		// earlier Warpmap is taken if there is one.

		if (!_takeSpareBuffer(pDevice, bytes))
		{
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
					Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for warpmap.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
				m_buffer = nullptr;
				return;
			}

			// Named after the first Warpmap to have it, so the debug layer's live object report
			// shows where it comes from.
			{
				wchar_t name[96];
				swprintf(name, 96, L"Wax DX12Warpmap %p: buffer", (const void*) this);
				m_buffer->SetName(name);
			}

			D3D12_RANGE readRange = { 0, 0 };			// We only write.

			if (FAILED(m_buffer->Map(0, &readRange, (void**) &m_pBuffer)))
			{
				if (!DX12Backend::checkDeviceLost())
					Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map buffer for warpmap.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
				m_buffer = nullptr;
				m_pBuffer = nullptr;
				return;
			}

			m_bufferBytes = bytes;
		}

		//TODO: As for DX12Edgemap, there is one buffer and two frames in flight, so
		// updating the mesh can change what the GPU is still reading for the previous frame.

		_updated(0, m_nVertices, 0, m_nTriangles);
	}

	//____ destructor ____________________________________________________________

	DX12Warpmap::~DX12Warpmap()
	{
		if (!m_buffer || !m_pBuffer)
			return;

		// Kept for a new Warpmap, unless that would keep too many bytes or the device has
		// changed since. The backends that drew from it hold it until the GPU is done.

		if (DX12Backend::device() == s_pSpareDevice && !DX12Backend::isDeviceLost() && m_bufferBytes <= c_maxSpareBytes)
		{
			while (s_spareBytes + m_bufferBytes > c_maxSpareBytes)
			{
				SpareBuffer& oldest = s_spareBuffers.front();
				oldest.buffer->Unmap(0, nullptr);
				s_spareBytes -= oldest.bytes;
				s_spareBuffers.erase(s_spareBuffers.begin());
			}

			s_spareBuffers.push_back({ m_buffer, m_pBuffer, m_bufferBytes, std::move(m_uses) });
			s_spareBytes += m_bufferBytes;
			return;
		}

		m_buffer->Unmap(0, nullptr);
	}

	//____ _takeSpareBuffer() ____________________________________________________
	//
	// Takes the oldest spare buffer of the given size that the GPU is done with, if any.
	// A fence of a lost device reads as done.

	bool DX12Warpmap::_takeSpareBuffer(ID3D12Device * pDevice, size_t bytes)
	{
		if (pDevice != s_pSpareDevice)
		{
			_exitDevice();
			s_pSpareDevice = pDevice;
			return false;
		}

		for (auto it = s_spareBuffers.begin(); it != s_spareBuffers.end(); ++it)
		{
			if (it->bytes != bytes)
				continue;

			bool bDone = true;

			for (const Use& use : it->uses)
			{
				if (use.fence->GetCompletedValue() < use.value)
				{
					bDone = false;
					break;
				}
			}

			if (!bDone)
				continue;

			m_buffer = std::move(it->buffer);
			m_pBuffer = it->pData;
			m_bufferBytes = it->bytes;
			s_spareBytes -= it->bytes;
			s_spareBuffers.erase(it);
			return true;
		}

		return false;
	}

	//____ _drawnBy() ____________________________________________________________
	//
	// The GPU reads our buffer until pFence has reached value.

	void DX12Warpmap::_drawnBy(ID3D12Fence * pFence, UINT64 value)
	{
		for (Use& use : m_uses)
		{
			if (use.fence.Get() == pFence)
			{
				use.value = std::max(use.value, value);
				return;
			}
		}

		m_uses.push_back({ pFence, value });
	}

	//____ _exitDevice() _________________________________________________________
	//
	// Lets go of the spare buffers, for when the device goes away or changes.

	void DX12Warpmap::_exitDevice()
	{
		for (SpareBuffer& spare : s_spareBuffers)
			spare.buffer->Unmap(0, nullptr);

		s_spareBuffers.clear();
		s_spareBytes = 0;
		s_pSpareDevice = nullptr;
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& DX12Warpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _indexBufferView() ____________________________________________________

	D3D12_INDEX_BUFFER_VIEW DX12Warpmap::_indexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW view = {};
		view.BufferLocation = _gpuAddress() + m_indexOffset;
		view.SizeInBytes = UINT(size_t(m_nTriangles) * sizeof(Triangle));
		view.Format = DXGI_FORMAT_R16_UINT;
		return view;
	}

	//____ _updated() ____________________________________________________________
	//
	// Copies what changed to the buffer. Periods are given when drawing.

	void DX12Warpmap::_updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles)
	{
		if (!m_pBuffer)
			return;

		if (nVertices > 0)
			std::memcpy(m_pBuffer + size_t(firstVertex) * sizeof(Vertex), m_pVertices + firstVertex, size_t(nVertices) * sizeof(Vertex));

		if (nTriangles > 0)
			std::memcpy(m_pBuffer + m_indexOffset + size_t(firstTriangle) * sizeof(Triangle), m_pTriangles + firstTriangle, size_t(nTriangles) * sizeof(Triangle));
	}

} // namespace wax
