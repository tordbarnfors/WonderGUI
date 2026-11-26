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

		// Triangles start at an aligned offset after the vertices.

		m_indexOffset = (size_t(m_nVertices) * sizeof(Vertex) + 255) & ~size_t(255);

		if (!DX12Backend::device())
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend::setDevice() must be called before any warpmap is created.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Nothing can be created on a lost device. The warpmap is left without a
		// buffer and DX12Backend draws nothing for it, until there is a new device,
		// see _prepare().

		if (DX12Backend::isDeviceLost())
			return;

		//TODO: As for DX12Edgemap, there is one buffer and two frames in flight, so
		// updating the mesh can change what the GPU is still reading for the previous frame.

		_prepare();
	}

	//____ destructor ____________________________________________________________

	DX12Warpmap::~DX12Warpmap()
	{
		DX12BufferPool::free(m_buffer);
	}

	//____ _prepare() ____________________________________________________________
	//
	// A buffer can't be empty.

	bool DX12Warpmap::_prepare()
	{
		if (DX12BufferPool::isValid(m_buffer))
			return true;

		size_t bytes = std::max(m_indexOffset + size_t(m_nTriangles) * sizeof(Triangle), size_t(256));

		if (!DX12BufferPool::alloc(int(bytes), m_buffer))
			return false;

		_updated(0, m_nVertices, 0, m_nTriangles);
		return true;
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
		if (!DX12BufferPool::isValid(m_buffer))
			return;

		if (nVertices > 0)
			std::memcpy(m_buffer.pData + size_t(firstVertex) * sizeof(Vertex), m_pVertices + firstVertex, size_t(nVertices) * sizeof(Vertex));

		if (nTriangles > 0)
			std::memcpy(m_buffer.pData + m_indexOffset + size_t(firstTriangle) * sizeof(Triangle), m_pTriangles + firstTriangle, size_t(nTriangles) * sizeof(Triangle));
	}

} // namespace wax
