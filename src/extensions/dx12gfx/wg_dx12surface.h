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
		friend class DX12Backend;		// Records our uploads, see "Uploads" below.

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

		// Set by DX12Backend::setDevice() before any surface is created. Uploads
		// are recorded by DX12Backend on the render queue, see "Uploads" below.
		// Reading a canvas back uses a copy queue shared by all surfaces, which
		// waits for every DX12Backend first, since any of them may use the texture.

		static bool				setDevice( ID3D12Device * pDevice );
		static void				exitDevice();

		// Used by DX12Backend::setDevice() to move every surface from one device to
		// another, see there. evacuateAll() takes the pixels of every surface out
		// into plain memory and lets go of all D3D12 objects, migrateAll() gives
		// them new ones on the device set since and uploads the pixels again.

		static void				evacuateAll();
		static void				migrateAll();

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
		ID3D12Resource*				paletteBuffer() const { return m_paletteBuffer.Get(); }		// So DX12Backend can hold it while the GPU reads it.

		// Resource state of the texture at the end of the command list DX12Backend
		// is recording. Only canvas surfaces ever leave D3D12_RESOURCE_STATE_COMMON.

		D3D12_RESOURCE_STATES	resourceState() const { return m_resourceState; }
		void					setResourceState( D3D12_RESOURCE_STATES state ) { m_resourceState = state; }

		// A mipmapped surface has a full chain of mip levels, which DX12Backend
		// draws from level 0 before the surface is read with stale ones, see
		// DX12Backend::_generateMipmaps(). D3D12 has nothing that does it for us.
		// mipSourceSRV(level) views level - 1 alone, mipRTV(level) level alone.

		int							mipLevels() const { return m_mipLevels; }
		bool						isMipmapStale() const { return m_bMipmapStale; }
		void						clearMipmapStale() { m_bMipmapStale = false; }
		D3D12_CPU_DESCRIPTOR_HANDLE	mipSourceSRV( int level ) const;
		D3D12_CPU_DESCRIPTOR_HANDLE	mipRTV( int level ) const;

		// Called by DX12Backend before it renders into us, once anything the CPU
		// wrote has been uploaded.

		void					notifyRendered();

		// Total size of the uploads waiting to be recorded, for all surfaces.

		static int64_t			pendingUploadBytes() { return s_pendingBytes; }


	protected:
		DX12Surface(const Blueprint& blueprint);
		DX12Surface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize);

		~DX12Surface();

		void			_setupTexture( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize,
									   HiColor fillColor = HiColor::Undefined );

		void			_addDirtyRect( const RectI& rect );

		void			_copyInPixels( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									   const Color8 * pSrcPalette, int srcPaletteSize, HiColor fillColor );

		void			_syncBufferAndWait();			// Brings anything rendered into us back to our pixels.
		bool			_initReadbackBuffer();
		void			_uploadNowAndWait();			// On the copy queue, for _syncBufferAndWait() only.

		ID3D12Resource *	_uploadSource( int y1, int y2, Microsoft::WRL::ComPtr<ID3D12Resource>& temp, UINT64& sourceOffset );	// Premultiplied if need be.

		const PixelBuffer	_pixelBuffer( const RectI& rect );	// Points into our pixels, without _waitForUpload().

		// Uploads. Pixels the CPU writes are not sent to the texture right away.
		// The surface goes on a list of pending uploads instead, in the order they
		// came in, and DX12Backend records the copies into its own command list:
		// all of them before a session, so no draw finds a texture out of date, or
		// earlier through DX12Backend::submitUploads(). The render queue is shared
		// by all backends and runs in order, which keeps the copy after any earlier
		// work reading the texture and before any later, with no wait on the CPU.
		//
		// The copy reads our pixels straight from the upload buffer when the GPU
		// gets to it, unless they had to be premultiplied first, see
		// _uploadSource(). So the CPU must not write to them between submission
		// and completion. allocPixelBuffer() waits for that, see _waitForUpload().

		void			_updatePending();				// Puts us on or takes us off the list, as m_dirtyRect says.
		void			_waitForUpload();				// Until the GPU has read our pixels for an upload already submitted.
		void			_forgetUpload();				// Drops what DX12Backend knows of an upload of ours.

		bool			_setPixelDetails();		// Settles on a layout D3D12 can hold.
		bool			_allocFallbackPixels();						// Plain memory, for when D3D12 wouldn't play along.
		bool			_createDeviceObjects();						// Texture, pixel buffer, views and palette buffer. All or none.
		void			_dropDeviceObjects();
		bool			_evacuate();						// False if our pixels went with the device.
		void			_migrate();

		void			_link();							// Into the list of live surfaces.
		void			_unlink();
		bool			_createPaletteBuffer();
		void			_updatePaletteBuffer();						// Converts m_pPalette into the palette buffer.

		bool			_isPremultiplied() const { return m_pixelFormat == PixelFormat::ARGB_8; }		// Texture holds premultiplied colors.
		void			_convertAlpha( uint8_t * pPixels, int pitch, int width, int height, bool bPremultiply ) const;

		static bool			_initCopyResources();
		static void			_waitForCopyFence();


		Microsoft::WRL::ComPtr<ID3D12Resource>			m_texture;			// Rests in COMMON state, promoted on use.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_uploadBuffer;		// Holds our pixels, readable and writable by the CPU.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_readbackBuffer;	// Created on first read back from a canvas surface.
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_paletteBuffer;	// Palette based surfaces only. Upload heap, permanently mapped.
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_srvHeap;			// Holds this surface's SRV, then one per mip level but the last. Not shader visible.
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;			// Holds one RTV per mip level. Canvas and mipmapped surfaces only.

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

		int				m_mipLevels = 1;
		bool			m_bMipmapStale = false;			// Level 0 has changed since the others were drawn from it.

		// On the list of pending uploads, see _updatePending(). Linked through
		// ourselves, oldest first.

		bool			m_bUploadPending = false;
		int64_t			m_pendingBytes = 0;				// Whole lines of m_dirtyRect, our share of s_pendingBytes.
		DX12Surface *	m_pPrevPending = nullptr;
		DX12Surface *	m_pNextPending = nullptr;

		// An upload recorded by a DX12Backend. Until that backend has submitted it,
		// m_pUploadRecordedBy says who has it. After that, the GPU is done reading
		// our pixels when m_uploadFence reaches m_uploadFenceValue.

		DX12Backend *							m_pUploadRecordedBy = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Fence>		m_uploadFence;
		UINT64									m_uploadFenceValue = 0;

		// Serial of the last command list that read us as blit source, see
		// DX12Backend::_recordUploads().

		UINT64			m_lastReadInList = 0;

		// All live surfaces, linked through themselves, so they can be found when
		// the device has to be replaced. Not thread safe, like the rest of the
		// backend.

		DX12Surface *	m_pPrevLive = nullptr;
		DX12Surface *	m_pNextLive = nullptr;

		static DX12Surface *									s_pFirstLive;

		static DX12Surface *									s_pFirstPending;
		static DX12Surface *									s_pLastPending;
		static int64_t											s_pendingBytes;

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
