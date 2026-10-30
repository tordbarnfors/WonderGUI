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
#ifndef WG_DX12WARPMAP_DOT_H
#define WG_DX12WARPMAP_DOT_H
#pragma once

#define NOMINMAX
#include <wg_warpmap.h>

#include <wrl.h>
#include <d3d12.h>

namespace wg
{

	class DX12Warpmap;
	typedef StrongPtr<DX12Warpmap> DX12Warpmap_p;
	typedef WeakPtr<DX12Warpmap> DX12Warpmap_wp;

	//____ DX12Warpmap __________________________________________________________
	/**
	 * @brief Warpmap for DX12Backend.
	 *
	 * The cells are kept in a buffer for the shaders, two float4 entries per cell, row by row:
	 * the top-left and top-right corners, then the bottom-left and bottom-right corners, as
	 * given by cellCorners(), so already moved by whole periods to be close to the top-left one,
	 * converted from spx to pixels of the source.
	 *
	 * Needs the device set by DX12Backend::setDevice().
	 */

	class DX12Warpmap : public Warpmap
	{
		friend class DX12Backend;

	public:

		//.____ Creation __________________________________________

		static DX12Warpmap_p	create(const Blueprint& blueprint);
		static DX12Warpmap_p	create(const Blueprint& blueprint, const CoordSPX * pNodes, int pitch = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		DX12Warpmap(const Blueprint& bp);
		~DX12Warpmap();

		void	_nodesUpdated(const RectI& region) override;

		inline SizeI	_cells() const { return { m_nodes.w - 1, m_nodes.h - 1 }; }

		D3D12_GPU_VIRTUAL_ADDRESS	_gpuAddress() const { return m_buffer ? m_buffer->GetGPUVirtualAddress() : 0; }

		Microsoft::WRL::ComPtr<ID3D12Resource>	m_buffer;
		float *	m_pBuffer = nullptr;			// Permanently mapped content of m_buffer.
	};

} // namespace wg
#endif // WG_DX12WARPMAP_DOT_H
