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

#ifndef WAX_DX12BITMAP_DOT_H
#define WAX_DX12BITMAP_DOT_H
#pragma once

#include <wax_gpubitmap.h>
#include <wax_dx12bufferpool.h>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <vector>

namespace wax
{

	class DX12Backend;

	class DX12Bitmap;
	typedef	StrongPtr<DX12Bitmap>	DX12Bitmap_p;
	typedef	WeakPtr<DX12Bitmap>	DX12Bitmap_wp;

	//____ Class DX12Bitmap _____________________________________________________________________
	/**
	 * @brief Bitmap for DX12Backend.
	 *
	 * Built on GpuBitmap, which has our pixels, if we keep any, and knows what the texture lacks
	 * of them. Here is what is on the device, how pixels get to the texture and back, and what
	 * happens when the device is replaced.
	 */

	class DX12Bitmap : public GpuBitmap
	{
		friend class DX12Backend;		// Records our uploads, see "Uploads" below.

	public:

		//.____ Creation __________________________________________

		static DX12Bitmap_p	create( const Blueprint& blueprint);
		static DX12Bitmap_p	create( const Blueprint& blueprint, Blob* pBlob, int pitch = 0);

		static DX12Bitmap_p	create( const Blueprint& blueprint, const uint8_t* pPixels,
										PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		static DX12Bitmap_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
										const PixelDescription& pixelDescription, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI			maxSize();

		//.____ Misc __________________________________________________________

		// Set by DX12Backend::setDevice() before any bitmap is created. Uploads
		// are recorded by DX12Backend on the render queue, see "Uploads" below.
		// Reading a canvas back uses a copy queue shared by all bitmaps, which
		// waits for every DX12Backend first, since any of them may use the texture.

		static bool				setDevice( ID3D12Device * pDevice );
		static void				exitDevice();

		// Used by DX12Backend::setDevice() to move every bitmap from one device to
		// another, see there. evacuateAll() lets go of all D3D12 objects, after
		// reading back what was rendered into canvases without restorer if the
		// device still works. migrateAll() gives the bitmaps new ones on the device
		// set since and uploads their pixels again. Bitmaps that had their content
		// in the texture only then get it back by their restorer, or are cleared.

		static void				evacuateAll();
		static void				migrateAll();

		// Used by DX12Backend when the bitmap is set as blit source or canvas.

		ID3D12Resource*				texture() const { return m_texture.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE	textureSRV() const { return m_srvHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE	renderTargetView() const { return m_rtvHandle; }
		DXGI_FORMAT					dxgiFormat() const { return m_dxgiFormat; }
		bool						isAlphaOnly() const { return m_bAlphaOnly; }

		// Palette based bitmaps hold 8 or 16 bit indexes in the texture and their
		// palette, converted to linear, in a buffer of float4 the pixel shader reads.
		// The first entry holds the palette's capacity, the colors follow.

		D3D12_GPU_VIRTUAL_ADDRESS	paletteGPUAddress() const { return DX12BufferPool::isValid(m_paletteBuffer) ? m_paletteBuffer.gpuAddress : 0; }

		// Resource state of the texture at the end of the command list DX12Backend
		// is recording. Only canvas bitmaps ever leave D3D12_RESOURCE_STATE_COMMON.

		D3D12_RESOURCE_STATES	resourceState() const { return m_resourceState; }
		void					setResourceState( D3D12_RESOURCE_STATES state ) { m_resourceState = state; }

		// DX12Backend draws the mip levels from level 0, see DX12Backend::_generateMipmaps().
		// D3D12 has nothing that does it for us. mipSourceSRV(level) views level - 1 alone,
		// mipRTV(level) level alone.

		D3D12_CPU_DESCRIPTOR_HANDLE	mipSourceSRV( int level ) const;
		D3D12_CPU_DESCRIPTOR_HANDLE	mipRTV( int level ) const;

		// Total size of the uploads waiting to be recorded, for all bitmaps.

		static int64_t			pendingUploadBytes() { return s_pendingBytes; }


	protected:
		DX12Bitmap(const Blueprint& blueprint, const PixelSource& source);
		~DX12Bitmap();

		// What GpuBitmap has us do.

		PixelFormat		_setTextureFormat( PixelFormat format ) override;	// D3D12 has no 565, they are widened to XRGB_8.
		bool			_createDeviceObjects() override;					// Texture, views and palette buffer. All or none.
		Staging *		_allocStaging( SizeI size, bool bReadable ) override;
		void			_pull( Staging * pStaging, const RectI& rect, CoordI dest ) override;
		bool			_readBack( const RectI& rect, uint8_t * pDest, int destPitch ) override;
		void			_updatePending() override;							// Puts us on or takes us off the list.

		// Staging memory is a buffer of its own.

		struct DX12Staging : public Staging
		{
			Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
			SizeI									size;
		};

		// Uploads. Pixels the CPU writes are not sent to the texture right away.
		// The bitmap goes on a list of pending uploads instead, in the order they
		// came in, and DX12Backend records the copies into its own command list:
		// all of them before a session, so no draw finds a texture out of date, or
		// earlier through DX12Backend::submitUploads(). The render queue is shared
		// by all backends and runs in order, which keeps the copy after any earlier
		// work reading the texture and before any later, with no wait on the CPU.
		//
		// What is to be copied is a list of rectangles of buffers, see _pull().
		// Pixels of our own that have changed, m_dirtyRect, are copied to a buffer
		// when the upload is recorded, which DX12Backend shares between the
		// bitmaps of a batch, see _stageDirtyInto(). Nothing on the GPU reads our
		// pixels, so they can be written to at any time.

		struct Pull
		{
			Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
			UINT64		offset;				// Of the pixels in the buffer.
			int			pitch;
			SizeI		size;				// Of the pixels in the buffer.
			RectI		rect;				// What of them to copy.
			CoordI		dest;				// Where in the texture.
		};

		int				_stagingPitch( int width ) const;
		UINT64			_dirtyStagingBytes() const;
		void			_stageDirtyInto( ID3D12Resource * pBuffer, uint8_t * pMapped, UINT64 offset );
		void			_recordPulls( ID3D12GraphicsCommandList * pList );
		void			_uploadNowAndWait();			// On the copy queue, for _readBack() only.

		void			_forgetUpload();				// Drops what DX12Backend knows of an upload of ours.
		void			_uploadAbandoned();				// The list it was recorded into was never submitted.

		void			_dropDeviceObjects();
		void			_evacuate();
		void			_migrate();

		void			_link();							// Into the list of live bitmaps.
		void			_unlink();
		bool			_createPaletteBuffer();

		static bool			_initCopyResources();
		static void			_waitForCopyFence();


		Microsoft::WRL::ComPtr<ID3D12Resource>			m_texture;			// Rests in COMMON state, promoted on use.
		DX12BufferPool::Buffer							m_paletteBuffer;	// Palette based bitmaps only, see _createPaletteBuffer().
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_srvHeap;			// Holds this bitmap's SRV, then one per mip level but the last. Not shader visible.
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;			// Holds one RTV per mip level. Canvas and mipmapped bitmaps only.

		D3D12_CPU_DESCRIPTOR_HANDLE	m_srvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE	m_rtvHandle = {};

		DXGI_FORMAT		m_dxgiFormat = DXGI_FORMAT_UNKNOWN;
		D3D12_RESOURCE_STATES m_resourceState = D3D12_RESOURCE_STATE_COMMON;

		bool			m_bAlphaOnly = false;			// Alpha_8, no color channels.

		// On the list of pending uploads, see _updatePending(). Linked through
		// ourselves, oldest first.

		std::vector<Pull>	m_pulls;						// What is to be copied to the texture.

		bool			m_bUploadPending = false;
		int64_t			m_pendingBytes = 0;				// Our share of s_pendingBytes.
		DX12Bitmap *	m_pPrevPending = nullptr;
		DX12Bitmap *	m_pNextPending = nullptr;

		// The DX12Backend that has recorded an upload of ours and not yet submitted
		// it, which tells us if it never gets to, see _uploadAbandoned().

		DX12Backend *							m_pUploadRecordedBy = nullptr;

		// Serial of the last command list that read us as blit source, see
		// DX12Backend::_recordUploads().

		UINT64			m_lastReadInList = 0;

		// All live bitmaps, linked through themselves, so they can be found when
		// the device has to be replaced. Not thread safe, like the rest of the
		// backend.

		DX12Bitmap *	m_pPrevLive = nullptr;
		DX12Bitmap *	m_pNextLive = nullptr;

		static DX12Bitmap *									s_pFirstLive;

		static DX12Bitmap *									s_pFirstPending;
		static DX12Bitmap *									s_pLastPending;
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

} // namespace wax
#endif // WAX_DX12BITMAP_DOT_H
