/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

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

		// Set by DX12Backend before any surface is created. All surfaces share
		// one copy queue for their uploads.

		static bool				setDevice( ID3D12Device * pDevice );
		static void				exitDevice();

		// Used by DX12Backend when the surface is set as blit source.

		ID3D12Resource*				texture() const { return m_texture.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE	textureSRV() const { return m_srvHandle; }
		bool						isAlphaOnly() const { return m_bAlphaOnly; }

		void					syncTexture();			// Uploads pending pixel changes, if any.


	protected:
		DX12Surface(const Blueprint& blueprint);
		DX12Surface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize);

		~DX12Surface();

		void			_setupTexture( const void * pPixels, int pitch, PixelFormat srcFormat, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize );

		void			_addDirtyRect( const RectI& rect );

		static DXGI_FORMAT	_dxgiFormat( PixelFormat format );
		static bool			_initCopyResources();


		Microsoft::WRL::ComPtr<ID3D12Resource>			m_texture;			// Lives in COMMON state, promoted on use.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_uploadBuffer;		// Holds our pixels, readable and writable by the CPU.
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_srvHeap;			// Holds this surface's SRV, not shader visible.

		D3D12_CPU_DESCRIPTOR_HANDLE	m_srvHandle = {};

		uint8_t *		m_pUploadData = nullptr;		// Permanently mapped content of m_uploadBuffer.
		int				m_uploadPitch = 0;				// Bytes per line, aligned as D3D12 requires for texture copies.
		int				m_pixelSize = 0;				// Bytes per pixel.
		bool			m_bAlphaOnly = false;			// Alpha_8, no color channels.

		RectI			m_dirtyRect;					// Area not yet uploaded to the texture. Empty when in sync.

		//

		static ID3D12Device *									s_pDevice;

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
