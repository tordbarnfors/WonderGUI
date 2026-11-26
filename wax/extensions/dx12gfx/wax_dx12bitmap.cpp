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


#define NOMINMAX

#include <wax_dx12bitmap.h>
#include <wax_dx12backend.h>

#include <wax_base.h>
#include <wax_util.h>
#include <wax_pixeltools.h>

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cwchar>

namespace wax
{

	using namespace std;

	const TypeInfo DX12Bitmap::TYPEINFO = { "DX12Bitmap", &GpuBitmap::TYPEINFO };

	ID3D12Device *										DX12Bitmap::s_pDevice = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			DX12Bitmap::s_copyQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		DX12Bitmap::s_copyAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	DX12Bitmap::s_copyList;
	Microsoft::WRL::ComPtr<ID3D12Fence>					DX12Bitmap::s_copyFence;
	HANDLE												DX12Bitmap::s_copyFenceEvent = nullptr;
	UINT64												DX12Bitmap::s_copyFenceValue = 0;

	DX12Bitmap *										DX12Bitmap::s_pFirstLive = nullptr;

	DX12Bitmap *										DX12Bitmap::s_pFirstPending = nullptr;
	DX12Bitmap *										DX12Bitmap::s_pLastPending = nullptr;
	int64_t												DX12Bitmap::s_pendingBytes = 0;

	//____ setObjectName() _________________________________________________________
	//
	// Names a D3D12 object after the bitmap owning it, so the debug layer's live
	// object report (ID3D12DebugDevice::ReportLiveDeviceObjects) and tools like
	// PIX show which bitmap it belongs to.

	static void setObjectName( ID3D12Object * pObject, const wchar_t * pWhat, const void * pBitmap, SizeI size )
	{
		if( !pObject )
			return;

		wchar_t name[128];
		swprintf( name, 128, L"Wax DX12Bitmap %p %dx%d: %ls", pBitmap, size.w, size.h, pWhat );
		pObject->SetName( name );
	}

	//____ maxSize() _______________________________________________________________

	SizeI DX12Bitmap::maxSize()
	{
		return SizeI(16384, 16384);			// D3D12 guarantees this at feature level 11_0 and up.
	}

	//____ setDevice() _____________________________________________________________

	bool DX12Bitmap::setDevice( ID3D12Device * pDevice )
	{
		exitDevice();
		s_pDevice = pDevice;
		return true;
	}

	//____ exitDevice() ____________________________________________________________

	void DX12Bitmap::exitDevice()
	{
		s_copyList = nullptr;
		s_copyAllocator = nullptr;
		s_copyQueue = nullptr;
		s_copyFence = nullptr;

		if( s_copyFenceEvent )
		{
			CloseHandle(s_copyFenceEvent);
			s_copyFenceEvent = nullptr;
		}

		s_copyFenceValue = 0;
		s_pDevice = nullptr;
	}

	//____ evacuateAll() ___________________________________________________________

	void DX12Bitmap::evacuateAll()
	{
		for( DX12Bitmap * p = s_pFirstLive; p; p = p->m_pNextLive )
			p->_evacuate();
	}

	//____ migrateAll() ____________________________________________________________

	void DX12Bitmap::migrateAll()
	{
		for( DX12Bitmap * p = s_pFirstLive; p; p = p->m_pNextLive )
			p->_migrate();

		// Restorers are called once every bitmap is back, as one may use others. They are
		// code of the application, which can create and let go of bitmaps, so we hold on to
		// those we are to call instead of walking the list.

		std::vector<DX12Bitmap_p> lost;

		for( DX12Bitmap * p = s_pFirstLive; p; p = p->m_pNextLive )
		{
			if( p->_isRestorePending() )
				lost.push_back(p);
		}

		for( auto& pBitmap : lost )
			pBitmap->_restore();
	}

	//____ _link() _________________________________________________________________

	void DX12Bitmap::_link()
	{
		m_pNextLive = s_pFirstLive;

		if( s_pFirstLive )
			s_pFirstLive->m_pPrevLive = this;

		s_pFirstLive = this;
	}

	//____ _unlink() _______________________________________________________________

	void DX12Bitmap::_unlink()
	{
		if( m_pPrevLive )
			m_pPrevLive->m_pNextLive = m_pNextLive;
		else
			s_pFirstLive = m_pNextLive;

		if( m_pNextLive )
			m_pNextLive->m_pPrevLive = m_pPrevLive;

		m_pPrevLive = nullptr;
		m_pNextLive = nullptr;
	}

	//____ _updatePending() _______________________________________________________
	//
	// A bitmap is on the list of pending uploads while it has something its texture
	// lacks: pixels of its own that have changed, or what has been pulled from pixel
	// buffers handed out. Call it whenever any of that or the texture changes.

	void DX12Bitmap::_updatePending()
	{
		bool bDirty = !m_dirtyRect.isEmpty() && m_pPixels;
		bool bPending = m_texture && (bDirty || !m_pulls.empty());

		if( bPending && !m_bUploadPending )
		{
			// Last in the list, so uploads are recorded in the order they came in.

			m_pPrevPending = s_pLastPending;
			m_pNextPending = nullptr;

			if( s_pLastPending )
				s_pLastPending->m_pNextPending = this;
			else
				s_pFirstPending = this;

			s_pLastPending = this;
			m_bUploadPending = true;
		}
		else if( !bPending && m_bUploadPending )
		{
			if( m_pPrevPending )
				m_pPrevPending->m_pNextPending = m_pNextPending;
			else
				s_pFirstPending = m_pNextPending;

			if( m_pNextPending )
				m_pNextPending->m_pPrevPending = m_pPrevPending;
			else
				s_pLastPending = m_pPrevPending;

			m_pPrevPending = nullptr;
			m_pNextPending = nullptr;
			m_bUploadPending = false;
		}

		int64_t bytes = 0;

		if( bPending )
		{
			bytes = int64_t(_dirtyStagingBytes());

			for( auto& pull : m_pulls )
				bytes += int64_t(pull.rect.w) * pull.rect.h * m_pixelBytes;
		}

		s_pendingBytes += bytes - m_pendingBytes;
		m_pendingBytes = bytes;
	}

	//____ _forgetUpload() ________________________________________________________
	//
	// For when we go away or let go of the device. A backend that has recorded an
	// upload of ours and not yet submitted it holds the texture and what it copies
	// from itself, but must not tell us should it never get to submit.

	void DX12Bitmap::_forgetUpload()
	{
		if( m_pUploadRecordedBy )
		{
			m_pUploadRecordedBy->_forgetRecordedUpload(this);
			m_pUploadRecordedBy = nullptr;
		}
	}

	//____ _uploadAbandoned() _____________________________________________________
	//
	// The list a backend recorded our upload into was never submitted, so the
	// texture didn't get it. Our pixels go again, whole. What was pulled from pixel
	// buffers handed out is gone, which takes a lost device and a bitmap without
	// pixels, that has lost its content anyway.

	void DX12Bitmap::_uploadAbandoned()
	{
		m_pUploadRecordedBy = nullptr;

		if( m_pPixels )
			m_dirtyRect = RectI(0, 0, m_size.w, m_size.h);

		_updatePending();
	}

	//____ _waitForCopyFence() _____________________________________________________

	void DX12Bitmap::_waitForCopyFence()
	{
		// Nothing more runs on a lost device. See DX12Backend::_waitForFence().

		if( DX12Backend::isDeviceLost() )
			return;

		if( s_copyFence && s_copyFence->GetCompletedValue() < s_copyFenceValue && s_copyFenceEvent )
		{
			s_copyFence->SetEventOnCompletion(s_copyFenceValue, s_copyFenceEvent);
			WaitForSingleObject(s_copyFenceEvent, INFINITE);
		}
	}

	//____ _initCopyResources() ____________________________________________________
	//
	// One copy queue shared by all bitmaps, created on first use. Only reading a
	// canvas back uses it, which is synchronous: we submit and wait. Uploads go
	// through DX12Backend, see _updatePending().

	bool DX12Bitmap::_initCopyResources()
	{
		if( s_copyList )
			return true;

		if( !s_pDevice )
		{
			// Failing below on a lost device lets go of it, see exitDevice(). That
			// was reported then, and is no news to every bitmap created after.

			if( !DX12Backend::isDeviceLost() )
				Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
					"No D3D12 device set. Call DX12Backend::setDevice() before creating any bitmap.",
					nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		if( FAILED(s_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(s_copyQueue.GetAddressOf()))) ||
			FAILED(s_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(s_copyAllocator.GetAddressOf()))) ||
			FAILED(s_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, s_copyAllocator.Get(), nullptr, IID_PPV_ARGS(s_copyList.GetAddressOf()))) ||
			FAILED(s_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(s_copyFence.GetAddressOf()))) )
		{
			exitDevice();
			// A lost device is reported once by DX12Backend, not by every bitmap.
			// The same goes for the failures further down.

			if( !DX12Backend::checkDeviceLost() )
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create resources for bitmap uploads.",
					nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		s_copyList->Close();

		s_copyQueue->SetName(L"Wax DX12Bitmap: copy queue");
		s_copyAllocator->SetName(L"Wax DX12Bitmap: copy allocator");
		s_copyList->SetName(L"Wax DX12Bitmap: copy list");
		s_copyFence->SetName(L"Wax DX12Bitmap: copy fence");

		s_copyFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		return true;
	}

	//____ _setTextureFormat() _____________________________________________________
	//
	// D3D12 has no texture for the 565 formats, so they are widened to XRGB_8.
	// PixelTools does the conversion when the pixels are copied in. The color
	// space stays.
	//
	// An sRGB bitmap gets an _SRGB texture format. Our shaders work in linear, so
	// the hardware has to convert on the way in and on the way out: sRGB to linear
	// when a texel is sampled, linear to sRGB when a pixel is written.
	//
	// Alpha is linear in every format Wax has, so Alpha_8 stays plain.

	PixelFormat DX12Bitmap::_setTextureFormat( PixelFormat format )
	{
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

		switch( format )
		{
			// XRGB gets a format with no alpha channel rather than being put in a
			// BGRA texture the way MetalBitmap has to. Sampling one reads alpha as
			// 1.0 whatever the unused byte holds, which keeps such a bitmap opaque
			// without anyone having to fill that byte, and it lets DX12Backend tell
			// an XRGB canvas from an ARGB one, since the pipelines are keyed on this.

			case PixelFormat::RGB_565:
			case PixelFormat::BGR_565:
				format = PixelFormat::XRGB_8;
				[[fallthrough]];

			case PixelFormat::XRGB_8:
				m_dxgiFormat = bSRGB ? DXGI_FORMAT_B8G8R8X8_UNORM_SRGB : DXGI_FORMAT_B8G8R8X8_UNORM;
				break;

			case PixelFormat::ARGB_8:
				m_dxgiFormat = bSRGB ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
				break;

			case PixelFormat::Alpha_8:
				m_dxgiFormat = DXGI_FORMAT_A8_UNORM;
				break;

			// The texture holds the indexes, as integers the pixel shader looks up
			// in the palette. The color space is the palette's, and is dealt with
			// when the palette is converted, see GpuTools::paletteToFloats().

			case PixelFormat::Index_8:
				m_dxgiFormat = DXGI_FORMAT_R8_UINT;
				break;

			case PixelFormat::Index_16:
				m_dxgiFormat = DXGI_FORMAT_R16_UINT;
				break;

			default:
				m_dxgiFormat = DXGI_FORMAT_UNKNOWN;			// create() turns other formats away.
				break;
		}

		m_bAlphaOnly = (m_dxgiFormat == DXGI_FORMAT_A8_UNORM);

		return format;
	}

	//____ Create ______________________________________________________________

	DX12Bitmap_p DX12Bitmap::create(const Blueprint& blueprint)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return DX12Bitmap_p();

		return DX12Bitmap_p(new DX12Bitmap(blueprint, PixelSource()));
	}

	DX12Bitmap_p DX12Bitmap::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return DX12Bitmap_p();

		return DX12Bitmap_p(new DX12Bitmap(blueprint, _source(blueprint, pBlob, pitch)));
	}

	DX12Bitmap_p DX12Bitmap::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return DX12Bitmap_p();

		return DX12Bitmap_p(new DX12Bitmap(blueprint, _source(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize)));
	}

	DX12Bitmap_p DX12Bitmap::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		if( !_isBlueprintValid(blueprint, maxSize()) )
			return DX12Bitmap_p();

		return DX12Bitmap_p(new DX12Bitmap(blueprint, _source(blueprint, pPixels, format, pitch, pPalette, paletteSize)));
	}

	//____ constructor ________________________________________________________________

	DX12Bitmap::DX12Bitmap(const Blueprint& bp, const PixelSource& source) : GpuBitmap(bp)
	{
		_link();
		_setup(bp, source);
	}

	//____ _createDeviceObjects() ______________________________________________
	//
	// Everything the bitmap has on the device: the texture with its mip levels,
	// the views and, if palette based, the palette buffer. Either all of it is
	// created or none of it is kept, so a bitmap is never left halfway onto a
	// device.

	bool DX12Bitmap::_createDeviceObjects()
	{
		const char * pFunc = __func__;

		auto fail = [this, pFunc]( const char * pMessage )
		{
			_dropDeviceObjects();

			// A lost device is reported once by DX12Backend, not by every bitmap.

			if( pMessage && !DX12Backend::checkDeviceLost() )
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, pMessage, this, &TYPEINFO, pFunc, __FILE__, __LINE__);

			return false;
		};

		// On a lost device every call below would fail, and each failure costs an
		// exception inside the D3D12 runtime. Should a new device be set,
		// migrateAll() moves us onto it.

		if( DX12Backend::isDeviceLost() || !_initCopyResources() )
			return false;

		DXGI_FORMAT dxgiFormat = m_dxgiFormat;

		// The mip levels are drawn by DX12Backend, so the texture has to be a render
		// target even when the bitmap is no canvas.

		bool bRenderTarget = (m_bCanvas && !m_bIndexed) || m_mipLevels > 1;

		// Create the texture

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_size.w;
		texDesc.Height = m_size.h;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = UINT16(m_mipLevels);
		texDesc.Format = dxgiFormat;
		texDesc.SampleDesc = { 1, 0 };
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = bRenderTarget ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;

		// Kept in COMMON state: D3D12 promotes it to PIXEL_SHADER_RESOURCE when the
		// render queue reads it, and to COPY_SOURCE when the copy queue reads it
		// back, then decays it back again. Uploads are barriered into COPY_DEST and
		// out again by DX12Backend, since that state doesn't decay.

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
													  D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_texture.GetAddressOf()))) )
			return fail( "Failed to create texture for bitmap." );

		setObjectName( m_texture.Get(), m_bCanvas ? L"texture (canvas)" : L"texture", this, m_size );

		m_resourceState = D3D12_RESOURCE_STATE_COMMON;

		// Create the descriptor for the texture. It lives in a heap of its own and
		// is copied into the shader visible heap by DX12Backend when used. Each mip
		// level but the last also gets one of its own, for drawing the next level
		// from it, see mipSourceSRV().

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = m_mipLevels;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if( FAILED(s_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_srvHeap.GetAddressOf()))) )
			return fail( "Failed to create descriptor heap for bitmap." );

		setObjectName( m_srvHeap.Get(), L"SRV heap", this, m_size );

		m_srvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dxgiFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = m_mipLevels;

		s_pDevice->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHandle);

		for( int level = 1; level < m_mipLevels; level++ )
		{
			srvDesc.Texture2D.MostDetailedMip = level - 1;
			srvDesc.Texture2D.MipLevels = 1;

			s_pDevice->CreateShaderResourceView(m_texture.Get(), &srvDesc, mipSourceSRV(level));
		}

		// A canvas bitmap also needs a render target view. DX12Backend hands it
		// straight to OMSetRenderTargets(), so this heap is not shader visible
		// either. A mipmapped bitmap gets one per level, for drawing them.

		if( bRenderTarget )			// Never palette based, we can't render palette lookups in reverse.
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.NumDescriptors = m_mipLevels;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			if( FAILED(s_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf()))) )
				return fail( "Failed to create render target heap for bitmap." );

			setObjectName( m_rtvHeap.Get(), L"RTV heap", this, m_size );

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = dxgiFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			for( int level = 0; level < m_mipLevels; level++ )
			{
				rtvDesc.Texture2D.MipSlice = level;
				s_pDevice->CreateRenderTargetView(m_texture.Get(), &rtvDesc, mipRTV(level));
			}

			// Only a canvas hands out level 0. DX12Backend takes a bitmap without a
			// render target view as one that can't be a canvas.

			if( m_bCanvas )
				m_rtvHandle = mipRTV(0);
		}

		// _createPaletteBuffer() reports its own failure.

		if( m_bIndexed && (!m_pPalette || !_createPaletteBuffer()) )
			return fail( nullptr );

		return true;
	}

	//____ _stagingPitch() _____________________________________________________
	//
	// D3D12 wants each line of what a texture is copied from or to to start on a
	// 256 byte boundary.

	int DX12Bitmap::_stagingPitch( int width ) const
	{
		return (width * m_pixelBytes + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
	}

	//____ _allocStaging() _____________________________________________________
	//
	// A buffer the GPU copies to our texture from, mapped. It holds whatever was
	// in that memory. Reports its own failure.
	//
	// What is handed out as a PixelBuffer may be read, so it isn't in an UPLOAD
	// heap, although the GPU copies from it like from one. That memory is
	// write-combined and uncached, often in VRAM behind the PCIe bus on a discrete
	// GPU, and reading it from the CPU crawls: reading a 2100 x 440 canvas back out
	// of it took half a second on an AMD card. It gets the CPU cached memory the
	// driver uses for READBACK heaps instead, as a custom heap the GPU can copy
	// from. What we write once and never read is in an UPLOAD heap.

	DX12Bitmap::Staging * DX12Bitmap::_allocStaging( SizeI size, bool bReadable )
	{
		if( !m_texture || !s_pDevice || DX12Backend::isDeviceLost() )
			return nullptr;

		int pitch = _stagingPitch(size.w);

		D3D12_HEAP_PROPERTIES heapProps = {};

		if( bReadable )
			heapProps = s_pDevice->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_READBACK);
		else
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(pitch) * size.h;
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// A buffer outside an UPLOAD heap starts out COMMON, and is promoted to a
		// copy source by itself whenever it is copied from.

		D3D12_RESOURCE_STATES state = bReadable ? D3D12_RESOURCE_STATE_COMMON : D3D12_RESOURCE_STATE_GENERIC_READ;

		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		uint8_t * pMapped = nullptr;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, state, nullptr, IID_PPV_ARGS(buffer.GetAddressOf()))) ||
			FAILED(buffer->Map(0, nullptr, (void**) &pMapped)) )
		{
			// A lost device is reported once by DX12Backend, not by every bitmap.

			if( !DX12Backend::checkDeviceLost() )
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for pixels on their way to the texture of a bitmap.", this, &TYPEINFO, __func__, __FILE__, __LINE__);

			return nullptr;
		}

		setObjectName( buffer.Get(), bReadable ? L"pixel buffer" : L"staging buffer", this, size );

		auto pStaging = new DX12Staging();

		pStaging->buffer = buffer;
		pStaging->size = size;
		pStaging->pPixels = pMapped;
		pStaging->pitch = pitch;

		return pStaging;
	}

	//____ _pull() _____________________________________________________________
	//
	// The copy is recorded by a DX12Backend, see _updatePending(). We hold on to
	// the buffer until then and the backend from there on, so whoever has the
	// staging memory can let go of it.

	void DX12Bitmap::_pull( Staging * pStaging, const RectI& rect, CoordI dest )
	{
		if( !m_texture )
			return;

		auto p = static_cast<DX12Staging*>(pStaging);

		Pull pull;

		pull.buffer = p->buffer;
		pull.offset = 0;
		pull.pitch = p->pitch;
		pull.size = p->size;
		pull.rect = rect;
		pull.dest = dest;

		m_pulls.push_back(pull);
		_updatePending();
	}

	//____ _dirtyStagingBytes() ________________________________________________
	//
	// What m_dirtyRect takes of a buffer shared with other bitmaps, where each
	// starts on a 512 byte boundary (D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT).

	UINT64 DX12Bitmap::_dirtyStagingBytes() const
	{
		if( m_dirtyRect.isEmpty() || !m_pPixels )
			return 0;

		UINT64 bytes = UINT64(_stagingPitch(m_dirtyRect.w)) * m_dirtyRect.h;

		return (bytes + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1) & ~UINT64(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
	}

	//____ _stageDirtyInto() ___________________________________________________
	//
	// For DX12Backend, which has all bitmaps of a batch staged in one buffer, as
	// making a buffer takes far longer than filling a small one. Copies what
	// m_dirtyRect covers of our pixels to the given offset of the buffer, which
	// is only written to, and has it pulled from there.

	void DX12Bitmap::_stageDirtyInto( ID3D12Resource * pBuffer, uint8_t * pMapped, UINT64 offset )
	{
		Pull pull;

		pull.buffer = pBuffer;
		pull.offset = offset;
		pull.pitch = _stagingPitch(m_dirtyRect.w);
		pull.size = m_dirtyRect.size();
		pull.rect = RectI(0, 0, m_dirtyRect.w, m_dirtyRect.h);
		pull.dest = m_dirtyRect.pos();

		_fillStaging( m_dirtyRect, pMapped + offset, pull.pitch );

		m_pulls.push_back(pull);
		_dirtyStaged();
	}

	//____ _recordPulls() ______________________________________________________
	//
	// Records the copies of what we have had pulled into the given command list.
	// The texture has to be one that can be copied to there, and whoever calls
	// keeps the buffers until the GPU has copied.

	void DX12Bitmap::_recordPulls( ID3D12GraphicsCommandList * pList )
	{
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_texture.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		for( auto& pull : m_pulls )
		{
			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = pull.buffer.Get();
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint.Offset = pull.offset;
			src.PlacedFootprint.Footprint.Format = m_dxgiFormat;
			src.PlacedFootprint.Footprint.Width = pull.size.w;
			src.PlacedFootprint.Footprint.Height = pull.size.h;
			src.PlacedFootprint.Footprint.Depth = 1;
			src.PlacedFootprint.Footprint.RowPitch = pull.pitch;

			D3D12_BOX box = { UINT(pull.rect.x), UINT(pull.rect.y), 0, UINT(pull.rect.x + pull.rect.w), UINT(pull.rect.y + pull.rect.h), 1 };

			pList->CopyTextureRegion(&dst, pull.dest.x, pull.dest.y, 0, &src, &box);
		}
	}

	//____ _dropDeviceObjects() ________________________________________________
	//
	// Lets go of everything _createDeviceObjects() made, and of what was on its way
	// to the texture.

	void DX12Bitmap::_dropDeviceObjects()
	{
		_forgetUpload();

		m_pulls.clear();
		m_texture = nullptr;
		DX12BufferPool::free(m_paletteBuffer);
		m_srvHeap = nullptr;
		m_rtvHeap = nullptr;

		m_srvHandle = {};
		m_rtvHandle = {};
		m_resourceState = D3D12_RESOURCE_STATE_COMMON;
		m_bMipmapStale = false;

		m_pPaletteColors = nullptr;

		_updatePending();				// Nothing to upload to any more.
	}

	//____ _evacuate() _________________________________________________________
	//
	// Lets go of the device. Our pixels are in plain memory and stay as they are.
	// A canvas or a bitmap with restorer has none, see GpuBitmap::_leaveDevice():
	// what was rendered into a canvas without restorer is read back if the device
	// still works. Otherwise the content went with the texture, and is put back by
	// the restorer when we are on a device again, see migrateAll(). Without one the
	// canvas is marked as having lost its content, see Bitmap::isContentLost(), so
	// whoever rendered into it knows to do it again.
	//
	// A PixelBuffer of such a bitmap handed out before this is of the device we leave.
	// Its memory stays, but what is written to it takes a copy more to get to the
	// texture. That is why this only happens between frames, from
	// DX12Backend::setDevice().

	void DX12Bitmap::_evacuate()
	{
		if( !m_texture )
			return;						// Nothing on the device.

		_leaveDevice( !DX12Backend::isDeviceLost() );
		_dropDeviceObjects();
	}

	//____ _migrate() __________________________________________________________
	//
	// Puts us back on the device set since we were evacuated, or onto the first
	// one we get if we were created while there was none. Our pixels are sent to
	// the new texture, all of them. Bitmaps D3D12 still won't take stay without.

	void DX12Bitmap::_migrate()
	{
		if( m_texture )
			return;

		if( !_createDeviceObjects() )
			return;

		_enterDevice();
	}

	//____ _createPaletteBuffer() ______________________________________________
	//
	// One float4 per palette entry, plus one in front holding the capacity. A root
	// SRV has no size the shader could ask for, and an index past the end of the
	// palette - there is nothing stopping one - must not read past the buffer.

	bool DX12Bitmap::_createPaletteBuffer()
	{
		DX12BufferPool::free(m_paletteBuffer);
		m_pPaletteColors = nullptr;

		// From the pool, which reports if it fails, unless the device is lost.

		if( !DX12BufferPool::alloc( (m_paletteCapacity + 1) * 4 * int(sizeof(float)), m_paletteBuffer ) )
			return false;

		m_pPaletteColors = (float*) m_paletteBuffer.pData;

		_updatePaletteColors();
		return true;
	}

	//____ Destructor ______________________________________________________________

	DX12Bitmap::~DX12Bitmap()
	{
		_unlink();

		// Off the list of pending uploads, and out of reach of a backend that has
		// recorded one of ours. That backend holds the texture and what it copies
		// from until the GPU is done with them.

		_forgetUpload();
		m_pulls.clear();
		m_dirtyRect = RectI();
		_updatePending();

		DX12BufferPool::free(m_paletteBuffer);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12Bitmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _uploadNowAndWait() _________________________________________________
	//
	// Sends what we have for the texture on the copy queue and waits for it. Only
	// for _readBack(), which must have it in the texture before it reads the
	// texture back and is about to wait anyway. Everything else goes through
	// DX12Backend, see _updatePending().

	void DX12Bitmap::_uploadNowAndWait()
	{
		if( !m_texture || !s_copyList )
			return;

		// The texture is gone with the device.

		if( DX12Backend::isDeviceLost() )
			return;

		_stageDirty();				// In a buffer of its own.

		if( m_pulls.empty() )
			return;

		// The copy queue can only touch a texture that rests in COMMON state.
		// DX12Backend puts a bitmap back there when it stops rendering into it or
		// reading from it, so this should never happen.

		if( m_resourceState != D3D12_RESOURCE_STATE_COMMON )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Internal,
				"Can't upload to a bitmap that is in use by the backend.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		s_copyAllocator->Reset();
		s_copyList->Reset(s_copyAllocator.Get(), nullptr);

		_recordPulls( s_copyList.Get() );

		s_copyList->Close();

		// A DX12Backend may still have work for this texture recorded or in flight,
		// and our copy must not land in the middle of it. Any of them might, since
		// bitmaps are shared between backends.

		DX12Backend::waitForCompletionOfAll();

		ID3D12CommandList* pLists[] = { s_copyList.Get() };
		s_copyQueue->ExecuteCommandLists(1, pLists);

		s_copyFenceValue++;
		s_copyQueue->Signal(s_copyFence.Get(), s_copyFenceValue);
		_waitForCopyFence();

		m_pulls.clear();			// The GPU has copied from them.
		_uploaded();
	}

	//____ mipSourceSRV() ______________________________________________________

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Bitmap::mipSourceSRV( int level ) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += SIZE_T(level) * s_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return handle;
	}

	//____ mipRTV() ____________________________________________________________

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Bitmap::mipRTV( int level ) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += SIZE_T(level) * s_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		return handle;
	}

	//____ _readBack() _________________________________________________________
	//
	// Copies a rectangle of our texture to pDest, by a buffer made for it. This
	// stalls until the GPU is done, but only happens when someone asks for the
	// pixels of a bitmap that has none of its own.

	bool DX12Bitmap::_readBack( const RectI& rect, uint8_t * pDest, int destPitch )
	{
		// What was rendered went with the device.

		if( DX12Backend::isDeviceLost() )
			return false;

		if( !m_texture || !s_pDevice || !s_copyList )
			return false;

		// Same rule as for uploads: the copy queue needs the texture at rest.

		if( m_resourceState != D3D12_RESOURCE_STATE_COMMON )
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::Internal,
				"Can't read back a bitmap that is in use by the backend.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		// Rendering into us may still be sitting in a DX12Backend's command list,
		// unsubmitted. It has to be on its way and finished before we look.
		//
		// _uploadNowAndWait() does that itself when it has something to upload,
		// which is newer than what was rendered, so it goes in on top.

		if( m_pulls.empty() && _dirtyStagingBytes() == 0 )
			DX12Backend::waitForCompletionOfAll();
		else
			_uploadNowAndWait();

		int pitch = _stagingPitch(rect.w);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_READBACK;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(pitch) * rect.h;
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(buffer.GetAddressOf()))) )
		{
			if( !DX12Backend::checkDeviceLost() )
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for reading back a bitmap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		setObjectName( buffer.Get(), L"readback buffer", this, rect.size() );

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_texture.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = buffer.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dst.PlacedFootprint.Offset = 0;
		dst.PlacedFootprint.Footprint.Format = m_dxgiFormat;
		dst.PlacedFootprint.Footprint.Width = rect.w;
		dst.PlacedFootprint.Footprint.Height = rect.h;
		dst.PlacedFootprint.Footprint.Depth = 1;
		dst.PlacedFootprint.Footprint.RowPitch = pitch;

		D3D12_BOX box = { UINT(rect.x), UINT(rect.y), 0, UINT(rect.x + rect.w), UINT(rect.y + rect.h), 1 };

		s_copyAllocator->Reset();
		s_copyList->Reset(s_copyAllocator.Get(), nullptr);

		s_copyList->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);

		s_copyList->Close();

		ID3D12CommandList* pLists[] = { s_copyList.Get() };
		s_copyQueue->ExecuteCommandLists(1, pLists);

		s_copyFenceValue++;
		s_copyQueue->Signal(s_copyFence.Get(), s_copyFenceValue);
		_waitForCopyFence();

		if( DX12Backend::isDeviceLost() )
			return false;

		uint8_t * pSrc = nullptr;
		D3D12_RANGE readRange = { 0, SIZE_T(pitch) * rect.h };

		if( FAILED(buffer->Map(0, &readRange, (void**) &pSrc)) )
			return false;

		int lineBytes = rect.w * m_pixelBytes;

		for( int y = 0; y < rect.h; y++ )
			memcpy( pDest + y * destPitch, pSrc + y * pitch, lineBytes );

		D3D12_RANGE writtenRange = { 0, 0 };		// We only read.
		buffer->Unmap(0, &writtenRange);

		return true;
	}

} // namespace wax
