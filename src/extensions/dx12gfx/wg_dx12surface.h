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

#ifndef WG_DX12SURFACE_DOT_H
#define WG_DX12SURFACE_DOT_H
#pragma once

#include <wg_surface.h>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <vector>

namespace wg
{

	class DX12Backend;

	class DX12Surface;
	typedef	StrongPtr<DX12Surface>	DX12Surface_p;
	typedef	WeakPtr<DX12Surface>	DX12Surface_wp;

	//____ Class DX12Surface _____________________________________________________________________

	class DX12Surface : public Surface
	{

	public:

		//.____ Creation __________________________________________

		static DX12Surface_p	create( const Blueprint& blueprint);
		static DX12Surface_p	create( const Blueprint& blueprint, Blob* pBlob, int pitch = 0);

		static DX12Surface_p	create( const Blueprint& blueprint, const uint8_t* pPixels,
										PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		static DX12Surface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
										const PixelDescription& pixelDescription, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI			maxSize();

		//.____ Content _______________________________________________________

		int						alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		using Surface::allocPixelBuffer;
		using Surface::pullPixels;
		using Surface::pushPixels;

		const PixelBuffer		allocPixelBuffer(const RectI& rect) override;
		bool					pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void					pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true) override;
		void					freePixelBuffer(const PixelBuffer& buffer) override;

		//.____ Misc __________________________________________________________

		// Set by DX12Backend before any surface is created. All surfaces share one
		// copy queue for their uploads, and need the backend to tell them when its
		// rendering has finished.

		static bool				setDevice( ID3D12Device * pDevice, DX12Backend * pBackend );
		static void				exitDevice();

		// Used by DX12Backend when the surface is set as blit source or canvas.

		ID3D12Resource*				texture() const { return m_texture.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE	textureSRV() const { return m_srvHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE	renderTargetView() const { return m_rtvHandle; }
		DXGI_FORMAT					dxgiFormat() const { return m_dxgiFormat; }
		bool						isAlphaOnly() const { return m_bAlphaOnly; }

		// Palette based surfaces hold 8 or 16 bit indexes in the texture and their
		// palette, converted to linear, in a buffer of float4 the pixel shader reads.
		// The first entry holds the palette's capacity, the colors follow.
		bool						isIndexed() const { return m_bIndexed; }
		D3D12_GPU_VIRTUAL_ADDRESS	paletteGPUAddress() const { return m_paletteBuffer ? m_paletteBuffer->GetGPUVirtualAddress() : 0; }

		// Resource state of the texture at the end of the command list DX12Backend
		// is recording. Only canvas surfaces ever leave D3D12_RESOURCE_STATE_COMMON.

		D3D12_RESOURCE_STATES	resourceState() const { return m_resourceState; }
		void					setResourceState( D3D12_RESOURCE_STATES state ) { m_resourceState = state; }

		void					syncTexture();			// Uploads pending pixel changes, if any.
		void					notifyRendered();		// Called by DX12Backend before it renders into us.


	protected:
		DX12Surface(const Blueprint& blueprint);
		DX12Surface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize);

		~DX12Surface();

		void			_setupTexture( const void * pPixels, int pitch, PixelFormat srcFormat, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize );

		void			_addDirtyRect( const RectI& rect );

		void			_copyInPixels( const void * pPixels, int pitch, PixelFormat srcFormat, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, int srcPaletteSize );

		void			_syncBufferAndWait();			// Brings anything rendered into us back to our pixels.
		bool			_initReadbackBuffer();

		bool			_setPixelDetails( PixelFormat format );		// Settles on a format D3D12 can hold.
		bool			_allocFallbackPixels();						// Plain memory, for when D3D12 wouldn't play along.
		bool			_createPaletteBuffer();
		void			_updatePaletteBuffer();						// Converts m_pPalette into the palette buffer.

		static bool			_initCopyResources();
		static void			_waitForCopyFence();


		Microsoft::WRL::ComPtr<ID3D12Resource>			m_texture;			// Rests in COMMON state, promoted on use.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_uploadBuffer;		// Holds our pixels, readable and writable by the CPU.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_readbackBuffer;	// Created on first read back from a canvas surface.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_paletteBuffer;	// Palette based surfaces only. Upload heap, permanently mapped.
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_srvHeap;			// Holds this surface's SRV, not shader visible.
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;			// Holds this surface's RTV. Canvas surfaces only.

		D3D12_CPU_DESCRIPTOR_HANDLE	m_srvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE	m_rtvHandle = {};

		DXGI_FORMAT		m_dxgiFormat = DXGI_FORMAT_UNKNOWN;
		D3D12_RESOURCE_STATES m_resourceState = D3D12_RESOURCE_STATE_COMMON;

		uint8_t *		m_pUploadData = nullptr;		// Our pixels: the mapped upload buffer, or m_pFallbackData.
		uint8_t *		m_pFallbackData = nullptr;		// Plain memory, only used when we have no upload buffer.
		int				m_uploadPitch = 0;				// Bytes per line, aligned as D3D12 requires for texture copies.
		int				m_pixelSize = 0;				// Bytes per pixel.
		bool			m_bAlphaOnly = false;			// Alpha_8, no color channels.
		bool			m_bIndexed = false;				// Index_8 or Index_16, colors come from the palette.
		float *			m_pPaletteData = nullptr;		// Mapped palette buffer, see paletteGPUAddress().

		RectI			m_dirtyRect;					// Area not yet uploaded to the texture. Empty when in sync.
		bool			m_bBufferNeedsSync = false;		// Texture has changes our pixels don't have yet.

		//

		static ID3D12Device *									s_pDevice;
		static DX12Backend *									s_pBackend;			// Not a strong pointer, it owns us indirectly.

		static Microsoft::WRL::ComPtr<ID3D12CommandQueue>		s_copyQueue;
		static Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	s_copyAllocator;
		static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>s_copyList;
		static Microsoft::WRL::ComPtr<ID3D12Fence>				s_copyFence;
		static HANDLE											s_copyFenceEvent;
		static UINT64											s_copyFenceValue;
	};


	//========================================================================================

} // namespace wg
#endif // WG_DX12SURFACE_DOT_H
