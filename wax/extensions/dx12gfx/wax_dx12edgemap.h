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

#ifndef WAX_DX12EDGEMAP_DOT_H
#define WAX_DX12EDGEMAP_DOT_H
#pragma once

#define NOMINMAX
#include <wax_gpuedgemap.h>

#include <wrl.h>
#include <d3d12.h>

namespace wax
{

	class DX12Edgemap;
	typedef StrongPtr<DX12Edgemap> DX12Edgemap_p;
	typedef WeakPtr<DX12Edgemap> DX12Edgemap_wp;

	//____ DX12Edgemap __________________________________________________________
	/**
	 * @brief Edgemap for DX12Backend.
	 *
	 * GpuEdgemap fills in the buffer the shaders read, this class keeps it in an
	 * upload heap buffer, permanently mapped.
	 */

	class DX12Edgemap : public GpuEdgemap
	{
		friend class DX12Backend;

	public:

		//.____ Creation __________________________________________

		static DX12Edgemap_p	create(const Blueprint& blueprint);
		static DX12Edgemap_p	create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);
		static DX12Edgemap_p	create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);


		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

		//.____ Misc __________________________________________________________

		// Set by DX12Backend before any edgemap is created.

		static bool				setDevice( ID3D12Device * pDevice );
		static void				exitDevice();


	protected:

		DX12Edgemap(const Blueprint& bp);
		~DX12Edgemap();

		float *	_allocBuffer( size_t bytes ) override;
		void	_commitBuffer() override;

		// Used by DX12Backend::setDevice() to move every edgemap from one device to
		// another. Our buffer only holds what Edgemap already has in memory, so
		// evacuating just lets go of it and migrating builds it anew.

		static void	_evacuateAll();
		static void	_migrateAll();

		void	_link();						// Into the list of live edgemaps.
		void	_unlink();

		void	_releaseBuffer( Microsoft::WRL::ComPtr<ID3D12Resource>& buffer );

		// Where DX12Backend finds us.

		D3D12_GPU_VIRTUAL_ADDRESS	_gpuAddress() const { return m_buffer ? m_buffer->GetGPUVirtualAddress() : 0; }

		Microsoft::WRL::ComPtr<ID3D12Resource>	m_buffer;		// Permanently mapped at m_pGpuData.
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_newBuffer;	// From _allocBuffer() until _commitBuffer().

		// All live edgemaps, linked through themselves, so they can be found when
		// the device has to be replaced.

		DX12Edgemap *	m_pPrevLive = nullptr;
		DX12Edgemap *	m_pNextLive = nullptr;

		static DX12Edgemap *	s_pFirstLive;

		static ID3D12Device *	s_pDevice;
	};



} // namespace wax
#endif // WAX_DX12EDGEMAP_DOT_H
