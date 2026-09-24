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

#ifndef WG_DX12EDGEMAP_DOT_H
#define WG_DX12EDGEMAP_DOT_H
#pragma once

#define NOMINMAX
#include <wg_edgemap.h>

#include <wrl.h>
#include <d3d12.h>

namespace wg
{

	class DX12Edgemap;
	typedef StrongPtr<DX12Edgemap> DX12Edgemap_p;
	typedef WeakPtr<DX12Edgemap> DX12Edgemap_wp;

	class DX12Edgemap : public Edgemap
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

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginColor, int endColor) override;

		// Where DX12Backend finds us. Everything the segments shader reads lives in
		// one buffer, in float4 entries, which is how the shader indexes it:
		//
		//   Edge strips, one per pixel column.
		//   Flat colors, one per segment.
		//   Tint table, one per segment. X is the offset of the segment's tint
		//   block, -1 for a flat colored segment.
		//   Tint blocks, a slot per segment big enough for any tint. Only there
		//   once a segment has had a tint, most edgemaps never do.

		D3D12_GPU_VIRTUAL_ADDRESS	_gpuAddress() const { return m_buffer ? m_buffer->GetGPUVirtualAddress() : 0; }

		int		_flatColorsOfs() const { return m_samplesSize; }
		int		_tintTableOfs() const { return m_samplesSize + m_nbSegments; }
		int		_tintSlotOfs(int segment) const { return m_samplesSize + m_nbSegments * 2 + segment * c_tintSlotSize; }

		bool	_createBuffer(bool bWithTintSlots);
		void	_writeColors(int beginSegment, int endSegment);

		static const int c_tintSlotSize;		// In float4 entries.

		Microsoft::WRL::ComPtr<ID3D12Resource>	m_buffer;
		float *	m_pBuffer = nullptr;			// Permanently mapped content of m_buffer.

		int		m_samplesSize = 0;				// Size of the edge strips, in float4 entries.
		bool	m_bHasTintSlots = false;

		static ID3D12Device *	s_pDevice;
	};



} // namespace wg
#endif // WG_DX12EDGEMAP_DOT_H
