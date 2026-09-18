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

#include <wg_dx12surface.h>

#include <wg_gfxbase.h>
#include <wg_gfxutil.h>
#include <wg_pixeltools.h>

#include <cstring>

namespace wg
{

	using namespace std;

	const TypeInfo DX12Surface::TYPEINFO = { "DX12Surface", &Surface::TYPEINFO };

	ID3D12Device *										DX12Surface::s_pDevice = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			DX12Surface::s_copyQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		DX12Surface::s_copyAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	DX12Surface::s_copyList;
	Microsoft::WRL::ComPtr<ID3D12Fence>					DX12Surface::s_copyFence;
	HANDLE												DX12Surface::s_copyFenceEvent = nullptr;
	UINT64												DX12Surface::s_copyFenceValue = 0;

	//____ maxSize() _______________________________________________________________

	SizeI DX12Surface::maxSize()
	{
		return SizeI(16384, 16384);			// D3D12 guarantees this at feature level 11_0 and up.
	}

	//____ setDevice() _____________________________________________________________

	bool DX12Surface::setDevice( ID3D12Device * pDevice )
	{
		exitDevice();
		s_pDevice = pDevice;
		return true;
	}

	//____ exitDevice() ____________________________________________________________

	void DX12Surface::exitDevice()
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

	//____ _initCopyResources() ____________________________________________________
	//
	// One copy queue shared by all surfaces, created on first use. Uploads are
	// synchronous: we submit and wait. Surfaces are normally filled when loaded,
	// so the stalls are not on the rendering path.

	bool DX12Surface::_initCopyResources()
	{
		if( s_copyList )
			return true;

		if( !s_pDevice )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend must be created before any surface.",
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
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create resources for surface uploads.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		s_copyList->Close();

		s_copyFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		return true;
	}

	//____ _dxgiFormat() ___________________________________________________________
	//
	// sRGB formats are mapped to their linear counterparts for now, since nothing
	// else in the DX12 backend does sRGB conversion yet.

	DXGI_FORMAT DX12Surface::_dxgiFormat( PixelFormat format )
	{
		switch( format )
		{
			case PixelFormat::BGRA_8:
			case PixelFormat::BGRA_8_sRGB:
			case PixelFormat::BGRA_8_linear:
				return DXGI_FORMAT_B8G8R8A8_UNORM;

			case PixelFormat::BGRX_8:
			case PixelFormat::BGRX_8_sRGB:
			case PixelFormat::BGRX_8_linear:
				return DXGI_FORMAT_B8G8R8X8_UNORM;

			case PixelFormat::Alpha_8:
				return DXGI_FORMAT_A8_UNORM;

			default:
				return DXGI_FORMAT_UNKNOWN;
		}
	}

	//____ Create ______________________________________________________________

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint)
	{
		return DX12Surface_p(new DX12Surface(blueprint));
	}

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return DX12Surface_p(new DX12Surface(blueprint, pBlob, pitch));
	}

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		return DX12Surface_p(new DX12Surface(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		return DX12Surface_p(new DX12Surface(blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}

	//____ constructor ________________________________________________________________

	DX12Surface::DX12Surface(const Blueprint& bp) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		_setupTexture( nullptr, 0, PixelFormat::Undefined, nullptr, nullptr, bp.palette, 0 );
	}

	DX12Surface::DX12Surface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		if( pitch == 0 )
			pitch = bp.size.w * m_pPixelDescription->bits/8;

		_setupTexture( pBlob ? pBlob->data() : nullptr, pitch, m_pixelFormat, nullptr, bp.palette, bp.palette, 0 );
	}

	DX12Surface::DX12Surface(const Blueprint& bp, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		auto& srcDesc = Util::pixelFormatToDescription(format);

		if( pitch == 0 )
			pitch = bp.size.w * srcDesc.bits/8;

		_fixSrcParam(format, pPalette, paletteSize);
		_setupTexture( pPixels, pitch, format, nullptr, pPalette, bp.palette, paletteSize );
	}


	DX12Surface::DX12Surface(const Blueprint& bp, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		if( pitch == 0 )
			pitch = bp.size.w * pixelDescription.bits/8;

		_fixSrcParam(pixelDescription, pPalette, paletteSize);
		_setupTexture( pPixels, pitch, PixelFormat::Undefined, &pixelDescription, pPalette, bp.palette, paletteSize );
	}

	//____ _setupTexture() _____________________________________________________

	void DX12Surface::_setupTexture( const void * pPixels, int pitch, PixelFormat srcFormat, const PixelDescription * pSrcPixelDesc,
									 const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize )
	{
		if( !_initCopyResources() )
			return;

		if( m_bCanvas )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"Canvas surfaces are not supported by DX12Backend yet.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		DXGI_FORMAT dxgiFormat = _dxgiFormat(m_pixelFormat);

		if( dxgiFormat == DXGI_FORMAT_UNKNOWN )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam,
				"Pixel format not supported by DX12Backend yet. Use BGRA_8, BGRX_8 or Alpha_8.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		m_pixelSize = m_pPixelDescription->bits/8;
		m_bAlphaOnly = (dxgiFormat == DXGI_FORMAT_A8_UNORM);

		// D3D12 wants each line of a texture copy to start on a 256 byte boundary,
		// so our pixels are stored with a padded pitch. PixelBuffer carries the
		// pitch, so nothing outside this class needs to care.

		m_uploadPitch = (m_size.w * m_pixelSize + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

		// Create the texture

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_size.w;
		texDesc.Height = m_size.h;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = dxgiFormat;
		texDesc.SampleDesc = { 1, 0 };
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Kept in COMMON state: D3D12 promotes it to PIXEL_SHADER_RESOURCE when the
		// render queue uses it and to COPY_DEST when we upload, then decays it back
		// again, so no barriers are needed.

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
													  D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_texture.GetAddressOf()))) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create texture for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Create the buffer holding our pixels

		D3D12_HEAP_PROPERTIES uploadProps = {};
		uploadProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(m_uploadPitch) * m_size.h;
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if( FAILED(s_pDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_uploadBuffer.GetAddressOf()))) )
		{
			m_texture = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create pixel buffer for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		if( FAILED(m_uploadBuffer->Map(0, nullptr, (void**) &m_pUploadData)) )
		{
			m_texture = nullptr;
			m_uploadBuffer = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map pixel buffer for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		memset( m_pUploadData, 0, size_t(m_uploadPitch) * m_size.h );

		// Create the descriptor for the texture. It lives in a heap of its own and
		// is copied into the shader visible heap by DX12Backend when used.

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if( FAILED(s_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_srvHeap.GetAddressOf()))) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create descriptor heap for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		m_srvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dxgiFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;

		s_pDevice->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHandle);

		// Copy the pixels we were given, converting them if needed.

		if( pPixels )
		{
			if( srcPaletteSize == 0 )
				srcPaletteSize = m_paletteSize;

			int dstLineMargin = m_uploadPitch - m_size.w * m_pixelSize;

			if( srcFormat != PixelFormat::Undefined )
			{
				auto& srcDesc = Util::pixelFormatToDescription(srcFormat);

				PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, srcFormat, pitch - m_size.w * srcDesc.bits/8,
									   m_pUploadData, m_pixelFormat, dstLineMargin, pSrcPalette,
									   m_pPalette, srcPaletteSize, m_paletteSize, m_paletteCapacity);
			}
			else
			{
				PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, * pSrcPixelDesc, pitch - m_size.w * pSrcPixelDesc->bits/8,
									   m_pUploadData, m_pixelFormat, dstLineMargin, pSrcPalette,
									   m_pPalette, srcPaletteSize, m_paletteSize, m_paletteCapacity);
			}
		}

		// Everything we have goes to the texture on first use.

		m_dirtyRect = RectI(0, 0, m_size.w, m_size.h);
	}

	//____ Destructor ______________________________________________________________

	DX12Surface::~DX12Surface()
	{
		if( m_uploadBuffer && m_pUploadData )
			m_uploadBuffer->Unmap(0, nullptr);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12Surface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _addDirtyRect() _____________________________________________________

	void DX12Surface::_addDirtyRect( const RectI& rect )
	{
		if( rect.isEmpty() )
			return;

		//TODO: Keep a list of rectangles instead of growing one to cover them all.

		m_dirtyRect = m_dirtyRect.isEmpty() ? rect : RectI::bounds(m_dirtyRect, rect);
	}

	//____ syncTexture() _______________________________________________________
	//
	// Uploads whatever has changed since the last call and waits for it. Called by
	// DX12Backend before the surface is used as blit source.

	void DX12Surface::syncTexture()
	{
		if( m_dirtyRect.isEmpty() || !m_texture || !s_copyList )
			return;

		// Copy full lines, so the source offset stays aligned.

		int y1 = m_dirtyRect.y;
		int y2 = m_dirtyRect.y + m_dirtyRect.h;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_texture.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_uploadBuffer.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Offset = UINT64(y1) * m_uploadPitch;
		src.PlacedFootprint.Footprint.Format = _dxgiFormat(m_pixelFormat);
		src.PlacedFootprint.Footprint.Width = m_size.w;
		src.PlacedFootprint.Footprint.Height = y2 - y1;
		src.PlacedFootprint.Footprint.Depth = 1;
		src.PlacedFootprint.Footprint.RowPitch = m_uploadPitch;

		s_copyAllocator->Reset();
		s_copyList->Reset(s_copyAllocator.Get(), nullptr);

		s_copyList->CopyTextureRegion(&dst, 0, y1, 0, &src, nullptr);

		s_copyList->Close();

		ID3D12CommandList* pLists[] = { s_copyList.Get() };
		s_copyQueue->ExecuteCommandLists(1, pLists);

		// Wait for it. Surfaces are normally filled while loading, not while rendering.

		s_copyFenceValue++;
		s_copyQueue->Signal(s_copyFence.Get(), s_copyFenceValue);

		if( s_copyFence->GetCompletedValue() < s_copyFenceValue && s_copyFenceEvent )
		{
			s_copyFence->SetEventOnCompletion(s_copyFenceValue, s_copyFenceEvent);
			WaitForSingleObject(s_copyFenceEvent, INFINITE);
		}

		m_dirtyRect = RectI();
	}

	//____ alpha() _______________________________________________________________

	int DX12Surface::alpha(CoordSPX coord)
	{
		if( !m_pUploadData )
			return 0;

		// No need to free the PixelBuffer, we know how our pixel buffer works.

		return _alpha(coord, allocPixelBuffer());
	}

	//____ allocPixelBuffer() _________________________________________________
	//
	// Our pixels always live in a CPU accessible buffer, so this just points into
	// it. Note that the pitch is padded, see _setupTexture().

	const PixelBuffer DX12Surface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;

		buf.format = m_pixelFormat;
		buf.palette = m_pPalette;
		buf.pitch = m_uploadPitch;
		buf.pixels = m_pUploadData ? m_pUploadData + rect.y * m_uploadPitch + rect.x * m_pixelSize : nullptr;
		buf.rect = rect;

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool DX12Surface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		// The buffer is our pixels, and nothing renders into our texture yet, so
		// it is always up to date.

		return m_pUploadData != nullptr;
	}

	//____ pullPixels() _______________________________________________________

	void DX12Surface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		_addDirtyRect( bufferRect + buffer.rect.pos() );

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() __________________________________________________

	void DX12Surface::freePixelBuffer(const PixelBuffer& buffer)
	{
		// Nothing to do here.
	}

} // namespace wg
