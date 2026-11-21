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
#ifndef WAX_DX12WARPMAP_DOT_H
#define WAX_DX12WARPMAP_DOT_H
#pragma once

#define NOMINMAX
#include <wax_warpmap.h>

#include <wrl.h>
#include <d3d12.h>
#include <vector>

namespace wax
{

	class DX12Warpmap;
	typedef StrongPtr<DX12Warpmap> DX12Warpmap_p;
	typedef WeakPtr<DX12Warpmap> DX12Warpmap_wp;

	//____ DX12Warpmap __________________________________________________________
	/**
	 * @brief Warpmap for DX12Backend.
	 *
	 * The mesh is kept on the GPU as it is, in one permanently mapped buffer: the vertices (x, y,
	 * u, v, w and coverage as integers, the layout of Warpmap::Vertex), read by the vertex shader as
	 * a structured buffer, followed by the triangles (the corners as 16-bit unsigned ints, the layout
	 * of Warpmap::Triangle), which are the index buffer. Content that changes is copied there, with
	 * nothing worked out on the CPU. Drawing only gives where to draw, the source offset and the
	 * periods. See vs_warp.hlsl and warp.hlsl for what the shaders do with it.
	 *
	 * Needs the device set by DX12Backend::setDevice().
	 */

	class DX12Warpmap : public Warpmap
	{
		friend class DX12Backend;

	public:

		//.____ Creation __________________________________________

		static DX12Warpmap_p	create(const Blueprint& blueprint);
		static DX12Warpmap_p	create(const Blueprint& blueprint, const Vertex * pVertices, const Triangle * pTriangles);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		DX12Warpmap(const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr);
		~DX12Warpmap();

		void	_updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles) override;

		D3D12_GPU_VIRTUAL_ADDRESS	_gpuAddress() const { return m_buffer ? m_buffer->GetGPUVirtualAddress() : 0; }
		D3D12_INDEX_BUFFER_VIEW		_indexBufferView() const;

		// Buffers are kept for new Warpmaps when we go away, see the constructor. A buffer
		// can only be taken once the GPU is done with every submission that drew from it.
		// Each backend has a fence of its own, so it is one value per fence.

		struct Use
		{
			Microsoft::WRL::ComPtr<ID3D12Fence>	fence;
			UINT64								value;
		};

		struct SpareBuffer
		{
			Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
			uint8_t *								pData;			// Still mapped.
			size_t									bytes;
			std::vector<Use>						uses;
		};

		void		_drawnBy(ID3D12Fence * pFence, UINT64 value);	// By DX12Backend, value is signalled after the draw.
		static void	_exitDevice();									// By DX12Backend::setDevice(), when the GPU is done.

		bool		_takeSpareBuffer(ID3D12Device * pDevice, size_t bytes);

		Microsoft::WRL::ComPtr<ID3D12Resource>	m_buffer;
		uint8_t *			m_pBuffer = nullptr;			// Permanently mapped content of m_buffer.
		size_t				m_bufferBytes = 0;
		size_t				m_indexOffset = 0;				// Where the triangles start in m_buffer, in bytes.
		std::vector<Use>	m_uses;

		static std::vector<SpareBuffer>	s_spareBuffers;		// Oldest first.
		static size_t					s_spareBytes;
		static ID3D12Device *			s_pSpareDevice;		// That the spare buffers are on.
	};

} // namespace wax
#endif // WAX_DX12WARPMAP_DOT_H
