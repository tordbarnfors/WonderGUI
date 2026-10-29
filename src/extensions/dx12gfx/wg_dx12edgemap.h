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

		bool	_createBuffer( bool bConvertSamples );	// Creates and fills m_buffer from our samples and colors.

		// Used by DX12Backend::setDevice() to move every edgemap from one device to
		// another. Our buffer only holds what Edgemap already has in memory, so
		// evacuating just lets go of it and migrating builds it anew.

		static void	_evacuateAll();
		static void	_migrateAll();

		void	_link();						// Into the list of live edgemaps.
		void	_unlink();

		// Where DX12Backend finds us. Everything the segments shader reads lives in
		// one buffer: the edge strips first, then the palette, then a white color
		// for the axis that has no colorstrip. Offsets are in float4 entries, which
		// is how the shader indexes it.

		D3D12_GPU_VIRTUAL_ADDRESS	_gpuAddress() const { return m_buffer ? m_buffer->GetGPUVirtualAddress() : 0; }

		int		_whiteColorOfs() const { return m_whiteColorOfs / 4; }
		int		_flatColorsOfs() const { return m_paletteOfs / 4; }
		int		_colorstripXOfs() const { return m_paletteOfs / 4 + int(m_pColorstripsX - m_pPalette); }
		int		_colorstripYOfs() const { return m_paletteOfs / 4 + int(m_pColorstripsY - m_pPalette); }


		Microsoft::WRL::ComPtr<ID3D12Resource>	m_buffer;
		float *	m_pBuffer = nullptr;			// Permanently mapped content of m_buffer.

		int		m_paletteOfs = 0;				// Offset to the palette, in floats.
		int		m_whiteColorOfs = 0;			// Offset to the white color, in floats.
		int		m_bufferSize = 0;				// In floats.

		// All live edgemaps, linked through themselves, so they can be found when
		// the device has to be replaced.

		DX12Edgemap *	m_pPrevLive = nullptr;
		DX12Edgemap *	m_pNextLive = nullptr;

		static DX12Edgemap *	s_pFirstLive;

		static ID3D12Device *	s_pDevice;
	};



} // namespace wg
#endif // WG_DX12EDGEMAP_DOT_H
