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


#include <wg_dx12surface.h>
#include <wg_dx12backend.h>

#include <wg_gfxbase.h>
#include <wg_gfxutil.h>
#include <wg_pixeltools.h>

#include <cstring>
#include <cstdio>

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

	//____ _waitForCopyFence() _____________________________________________________

	void DX12Surface::_waitForCopyFence()
	{
		if( s_copyFence && s_copyFence->GetCompletedValue() < s_copyFenceValue && s_copyFenceEvent )
		{
			s_copyFence->SetEventOnCompletion(s_copyFenceValue, s_copyFenceEvent);
			WaitForSingleObject(s_copyFenceEvent, INFINITE);
		}
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
				"No D3D12 device set. Call DX12Backend::setDevice() before creating any surface.",
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

	//____ _setPixelDetails() ______________________________________________________
	//
	// Settles on a pixel layout D3D12 can actually hold, which is not always the
	// one that was asked for. The 565 formats are widened to XRGB_8, as MetalSurface
	// does, and pixels are kept in native byte order. PixelTools does the conversion
	// when the pixels are copied in. The color space stays.
	//
	// An sRGB surface gets an _SRGB texture format, as MetalSurface gives one a
	// _sRGB Metal format. Our shaders work in linear, so the hardware has to convert
	// on the way in and on the way out: sRGB to linear when a texel is sampled,
	// linear to sRGB when a pixel is written.
	//
	// Alpha is linear in every format WonderGUI has, so Alpha_8 stays plain.

	bool DX12Surface::_setPixelDetails()
	{
		bool bSupported = true;
		bool bSRGB = (m_colorSpace == ColorSpace::sRGB);

		PixelFormat format = m_pixelFormat;

		switch( format )
		{
			// XRGB gets a format with no alpha channel rather than being put in a
			// BGRA texture the way MetalSurface has to. Sampling one reads alpha as
			// 1.0 whatever the unused byte holds, which keeps such a surface opaque
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
			// when the palette is converted, see _updatePaletteBuffer().

			case PixelFormat::Index_8:
				m_dxgiFormat = DXGI_FORMAT_R8_UINT;
				break;

			case PixelFormat::Index_16:
				m_dxgiFormat = DXGI_FORMAT_R16_UINT;
				break;

			default:
			{
				char buffer[256];
				sprintf_s(buffer, "Pixel format %s is not supported by DX12Backend. The surface will have no texture.", toString(format));
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

				m_dxgiFormat = DXGI_FORMAT_UNKNOWN;
				bSupported = false;
				break;
			}
		}

		// We may have picked a different layout than we were given, so the size and
		// description have to follow along. This is filled in even for a format we
		// can't hold, so that the surface can still hand out pixels.

		m_pixelFormat = format;
		m_bBigEndian = (WG_IS_BIG_ENDIAN == 1);
		m_pixelDescription = Util::pixelFormatToDescription(format, m_bBigEndian);
		m_pixelSize = m_pPixelDescription->bits / 8;
		m_bAlphaOnly = (m_dxgiFormat == DXGI_FORMAT_A8_UNORM);
		m_bIndexed = (m_dxgiFormat == DXGI_FORMAT_R8_UINT || m_dxgiFormat == DXGI_FORMAT_R16_UINT);

		return bSupported;
	}

	//____ _allocFallbackPixels() __________________________________________________
	//
	// A surface with no texture is of no use to the GPU, but it still has to behave
	// like a surface: hand out pixel buffers, be readable, be writable. Anything
	// else turns one unsupported format into a crash somewhere else entirely.

	bool DX12Surface::_allocFallbackPixels()
	{
		if( m_pUploadData )
			return true;

		if( m_uploadPitch <= 0 || m_size.h <= 0 )
			return false;

		m_pFallbackData = new uint8_t[size_t(m_uploadPitch) * m_size.h];
		m_pUploadData = m_pFallbackData;

		memset( m_pUploadData, 0, size_t(m_uploadPitch) * m_size.h );

		return true;
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
	//
	// Default sample method is Bilinear, same as GlSurface and MetalSurface. Code
	// that creates surfaces without specifying one expects that, and Nearest
	// samples on texel corners for 1:1 blits, which float precision in tall
	// textures makes land on the texel above every now and then.

	DX12Surface::DX12Surface(const Blueprint& bp) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;
		_setupTexture( nullptr, 0, nullptr, nullptr, bp.palette, 0 );
	}

	DX12Surface::DX12Surface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;

		// The blob holds the layout that was asked for, which is not always the one
		// we end up with, so take note of it before _setupTexture() settles that.

		PixelDescription srcDesc = m_pixelDescription;

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

		_setupTexture( pBlob ? pBlob->data() : nullptr, pitch, &srcDesc, bp.palette, bp.palette, 0 );
	}

	DX12Surface::DX12Surface(const Blueprint& bp, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;

		// Source pixels are in the byte order and color space of the blueprint.

		if( format == PixelFormat::Undefined )
			format = m_pixelFormat;

		auto srcDesc = Util::pixelFormatToDescription(format, bp.bigEndian);

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(srcDesc, m_size.w);

		_fixSrcParam(srcDesc, pPalette, paletteSize);
		_setupTexture( pPixels, pitch, &srcDesc, pPalette, bp.palette, paletteSize );
	}


	DX12Surface::DX12Surface(const Blueprint& bp, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::ARGB_8, SampleMethod::Bilinear)
	{
		m_bMipmapped = bp.mipmap;

		if( pitch == 0 )
			pitch = PixelTools::bytesPerLine(pixelDescription, m_size.w);

		_fixSrcParam(pixelDescription, pPalette, paletteSize);
		_setupTexture( pPixels, pitch, &pixelDescription, pPalette, bp.palette, paletteSize );
	}

	//____ _setupTexture() _____________________________________________________

	void DX12Surface::_setupTexture( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									 const Color8 * pSrcPalette, const Color8 * pDstPalette, int srcPaletteSize )
	{
		bool bFormatSupported = _setPixelDetails();

		// Surface leaves the palette to us. It has to exist before any pixels are
		// copied in, since converting to a palette based format fills it in. It
		// holds as many entries as the blueprint's palette capacity, which Surface
		// has already worked out.

		if( m_pPixelDescription->type == PixelType::Index && m_paletteCapacity > 0 )
		{
			m_pPalette = new Color8[m_paletteCapacity];
			memset( m_pPalette, 0, sizeof(Color8) * m_paletteCapacity );

			if( pDstPalette && m_paletteSize > 0 )
				memcpy( m_pPalette, pDstPalette, sizeof(Color8) * m_paletteSize );
		}

		if( m_pixelSize <= 0 || m_size.w <= 0 || m_size.h <= 0 )
		{
			// Nothing sensible to hand out, not even a pixel buffer.

			char buffer[256];
			sprintf_s(buffer, "Can't create a %dx%d surface with %d bits per pixel.", m_size.w, m_size.h, m_pPixelDescription->bits);
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		DXGI_FORMAT dxgiFormat = m_dxgiFormat;

		// D3D12 wants each line of a texture copy to start on a 256 byte boundary,
		// so our pixels are stored with a padded pitch. PixelBuffer carries the
		// pitch, so nothing outside this class needs to care.
		//
		// We pad to 512 rather than 256, since a copy that starts partway down the
		// texture puts its offset at a whole number of lines, and that offset has
		// to land on a 512 byte boundary (D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT).
		// 512 is a multiple of 256, so both rules are satisfied at once.

		const int uploadAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

		m_uploadPitch = (m_size.w * m_pixelSize + uploadAlignment - 1) & ~(uploadAlignment - 1);

		// From here on the surface can always hand out pixels, whether or not D3D12
		// gives us what we ask for below.

		if( !bFormatSupported || !_initCopyResources() )
		{
			if( !_allocFallbackPixels() )
				return;

			_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );
			return;
		}

		// A mipmapped surface gets levels all the way down to 1x1. They are drawn
		// by DX12Backend, so the texture has to be a render target even when the
		// surface is no canvas. Palette based surfaces can't be mipmapped, Surface
		// turns such a blueprint away, and an average of indexes means nothing.

		m_mipLevels = 1;

		if( m_bMipmapped && !m_bIndexed )
		{
			for( int size = m_size.w > m_size.h ? m_size.w : m_size.h; size > 1; size >>= 1 )
				m_mipLevels++;
		}

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
		// render queue uses it and to COPY_DEST when we upload, then decays it back
		// again, so no barriers are needed.

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
													  D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_texture.GetAddressOf()))) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create texture for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);

			if( _allocFallbackPixels() )
				_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );

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
			m_uploadBuffer = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create pixel buffer for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);

			if( _allocFallbackPixels() )
				_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );

			return;
		}

		if( FAILED(m_uploadBuffer->Map(0, nullptr, (void**) &m_pUploadData)) )
		{
			m_texture = nullptr;
			m_uploadBuffer = nullptr;
			m_pUploadData = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map pixel buffer for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);

			if( _allocFallbackPixels() )
				_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );

			return;
		}

		memset( m_pUploadData, 0, size_t(m_uploadPitch) * m_size.h );

		// Create the descriptor for the texture. It lives in a heap of its own and
		// is copied into the shader visible heap by DX12Backend when used. Each mip
		// level but the last also gets one of its own, for drawing the next level
		// from it, see mipSourceSRV().

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = m_mipLevels;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if( FAILED(s_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_srvHeap.GetAddressOf()))) )
		{
			m_texture = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create descriptor heap for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);

			_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );
			return;
		}

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

		// A canvas surface also needs a render target view. DX12Backend hands it
		// straight to OMSetRenderTargets(), so this heap is not shader visible
		// either. A mipmapped surface gets one per level, for drawing them.

		if( bRenderTarget )			// Never palette based, we can't render palette lookups in reverse.
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.NumDescriptors = m_mipLevels;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			if( FAILED(s_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf()))) )
			{
				m_texture = nullptr;
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create render target heap for surface.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);

				_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );
				return;
			}

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = dxgiFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			for( int level = 0; level < m_mipLevels; level++ )
			{
				rtvDesc.Texture2D.MipSlice = level;
				s_pDevice->CreateRenderTargetView(m_texture.Get(), &rtvDesc, mipRTV(level));
			}

			// Only a canvas hands out level 0. DX12Backend takes a surface without a
			// render target view as one that can't be a canvas.

			if( m_bCanvas )
				m_rtvHandle = mipRTV(0);
		}

		if( m_bIndexed && (!m_pPalette || !_createPaletteBuffer()) )
		{
			m_texture = nullptr;
			_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );
			return;
		}

		_copyInPixels( pPixels, pitch, pSrcPixelDesc, pSrcPalette, srcPaletteSize );
		_updatePaletteBuffer();
	}

	//____ _createPaletteBuffer() ______________________________________________
	//
	// One float4 per palette entry, plus one in front holding the capacity. A root
	// SRV has no size the shader could ask for, and an index past the end of the
	// palette - there is nothing stopping one - must not read past the buffer.

	bool DX12Surface::_createPaletteBuffer()
	{
		D3D12_HEAP_PROPERTIES uploadProps = {};
		uploadProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(m_paletteCapacity + 1) * 4 * sizeof(float);
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if( FAILED(s_pDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_paletteBuffer.GetAddressOf()))) ||
			FAILED(m_paletteBuffer->Map(0, nullptr, (void**) &m_pPaletteData)) )
		{
			m_paletteBuffer = nullptr;
			m_pPaletteData = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create palette buffer for surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		memset( m_pPaletteData, 0, size_t(m_paletteCapacity + 1) * 4 * sizeof(float) );

		m_pPaletteData[0] = float(m_paletteCapacity);		// Exact, far below where floats lose integers.
		return true;
	}

	//____ _updatePaletteBuffer() ______________________________________________
	//
	// The palette goes to the GPU converted to linear, the same way SoftSurface
	// unpacks it, so the shader can use the colors as they are and interpolate
	// between them. An sRGB texture would have had the hardware do it, but only
	// for colors read from the texture, and ours are indexes.
	//
	// The buffer is read by the GPU where it is. Should the palette change while
	// a frame using it is still in flight, that frame may see the new colors.

	void DX12Surface::_updatePaletteBuffer()
	{
		if( !m_pPaletteData || !m_pPalette )
			return;

		// Palette entries are in the color space of the surface.

		bool bLinear = (m_colorSpace == ColorSpace::Linear);

		float * p = m_pPaletteData + 4;			// Past the capacity.

		for( int i = 0; i < m_paletteCapacity; i++ )
		{
			HiColor col = m_pPalette[i];		// Same color space, just more bits.

			if( bLinear )
			{
				*p++ = col.r / 4096.f;
				*p++ = col.g / 4096.f;
				*p++ = col.b / 4096.f;
				*p++ = col.a / 4096.f;
			}
			else
			{
				col.toLinearFloat(p);
				p += 4;
			}
		}
	}

	//____ _copyInPixels() _____________________________________________________
	//
	// Copies the pixels we were handed at creation into our own buffer, converting
	// them to our format on the way. Our format is not always the one that was
	// asked for, see _setPixelDetails().

	void DX12Surface::_copyInPixels( const void * pPixels, int pitch, const PixelDescription * pSrcPixelDesc,
									 const Color8 * pSrcPalette, int srcPaletteSize )
	{
		if( !m_pUploadData )
			return;

		if( pPixels && pSrcPixelDesc )
		{
			if( srcPaletteSize == 0 )
				srcPaletteSize = m_paletteSize;

			int dstLineMargin = m_uploadPitch - m_size.w * m_pixelSize;

			PixelTools::copyPixels(m_size.w, m_size.h, (const uint8_t*) pPixels, * pSrcPixelDesc, m_colorSpace, pitch - PixelTools::bytesPerLine(*pSrcPixelDesc, m_size.w),
								   pSrcPalette, srcPaletteSize,
								   m_pUploadData, m_pixelFormat, m_colorSpace, m_bBigEndian, dstLineMargin,
								   m_pPalette, m_paletteSize, m_paletteCapacity);
		}

		// Everything we have goes to the texture on first use.

		m_dirtyRect = RectI(0, 0, m_size.w, m_size.h);
	}

	//____ Destructor ______________________________________________________________

	DX12Surface::~DX12Surface()
	{
		if( m_uploadBuffer && m_pUploadData && m_pUploadData != m_pFallbackData )
			m_uploadBuffer->Unmap(0, nullptr);

		delete [] m_pFallbackData;

		if( m_paletteBuffer && m_pPaletteData )
			m_paletteBuffer->Unmap(0, nullptr);

		delete [] m_pPalette;
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

		// The copy queue can only touch a texture that rests in COMMON state.
		// DX12Backend puts a surface back there when it stops rendering into it or
		// reading from it, so this should never happen.

		if( m_resourceState != D3D12_RESOURCE_STATE_COMMON )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::Internal,
				"Can't upload to a surface that is in use by the backend.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

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
		src.PlacedFootprint.Footprint.Format = m_dxgiFormat;
		src.PlacedFootprint.Footprint.Width = m_size.w;
		src.PlacedFootprint.Footprint.Height = y2 - y1;
		src.PlacedFootprint.Footprint.Depth = 1;
		src.PlacedFootprint.Footprint.RowPitch = m_uploadPitch;

		s_copyAllocator->Reset();
		s_copyList->Reset(s_copyAllocator.Get(), nullptr);

		s_copyList->CopyTextureRegion(&dst, 0, y1, 0, &src, nullptr);

		s_copyList->Close();

		// A DX12Backend may still have work for this texture recorded or in flight,
		// and our copy must not land in the middle of it. Any of them might, since
		// surfaces are shared between backends.
		//
		//TODO: This stalls the GPU. Fine for surfaces filled while loading, which
		// is the normal case, but a surface updated every frame wants something
		// better - a second texture, or a wait on the copy queue instead.

		DX12Backend::waitForCompletionOfAll();

		ID3D12CommandList* pLists[] = { s_copyList.Get() };
		s_copyQueue->ExecuteCommandLists(1, pLists);

		// Wait for it. Surfaces are normally filled while loading, not while rendering.

		s_copyFenceValue++;
		s_copyQueue->Signal(s_copyFence.Get(), s_copyFenceValue);
		_waitForCopyFence();

		m_dirtyRect = RectI();

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;
	}

	//____ notifyRendered() ____________________________________________________
	//
	// Called by DX12Backend when this surface is about to be rendered into. Our
	// pixels are the older copy from here on, until someone asks for them.

	void DX12Surface::notifyRendered()
	{
		syncTexture();			// Anything the CPU wrote goes in before we render over it.

		m_bBufferNeedsSync = true;

		if( m_mipLevels > 1 )
			m_bMipmapStale = true;
	}

	//____ mipSourceSRV() ______________________________________________________

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Surface::mipSourceSRV( int level ) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += SIZE_T(level) * s_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return handle;
	}

	//____ mipRTV() ____________________________________________________________

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Surface::mipRTV( int level ) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += SIZE_T(level) * s_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		return handle;
	}

	//____ _initReadbackBuffer() _______________________________________________

	bool DX12Surface::_initReadbackBuffer()
	{
		if( m_readbackBuffer )
			return true;

		if( !s_pDevice )
			return false;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_READBACK;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(m_uploadPitch) * m_size.h;
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_readbackBuffer.GetAddressOf()))) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create readback buffer for canvas surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		return true;
	}

	//____ _syncBufferAndWait() ________________________________________________
	//
	// Brings whatever was rendered into our texture back into our pixel buffer.
	// This stalls until the GPU is done, but only happens when someone actually
	// asks to read a canvas surface.

	void DX12Surface::_syncBufferAndWait()
	{
		if( !m_bBufferNeedsSync )
			return;

		if( !m_texture || !m_pUploadData || !s_copyList || !_initReadbackBuffer() )
			return;			// Leaves the flag set, so we try again rather than hand out stale pixels.

		// Same rule as for uploads: the copy queue needs the texture at rest.

		if( m_resourceState != D3D12_RESOURCE_STATE_COMMON )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::Internal,
				"Can't read back a surface that is in use by the backend.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// Rendering into us may still be sitting in a DX12Backend's command list,
		// unsubmitted. It has to be on its way and finished before we look.
		//
		// syncTexture() does that itself when it has something to upload, and its
		// pixels are the newer ones, so they go in on top of what was rendered.

		if( m_dirtyRect.isEmpty() )
		{
			DX12Backend::waitForCompletionOfAll();
		}
		else
			syncTexture();

		m_bBufferNeedsSync = false;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_texture.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_readbackBuffer.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dst.PlacedFootprint.Offset = 0;
		dst.PlacedFootprint.Footprint.Format = m_dxgiFormat;
		dst.PlacedFootprint.Footprint.Width = m_size.w;
		dst.PlacedFootprint.Footprint.Height = m_size.h;
		dst.PlacedFootprint.Footprint.Depth = 1;
		dst.PlacedFootprint.Footprint.RowPitch = m_uploadPitch;

		s_copyAllocator->Reset();
		s_copyList->Reset(s_copyAllocator.Get(), nullptr);

		s_copyList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		s_copyList->Close();

		ID3D12CommandList* pLists[] = { s_copyList.Get() };
		s_copyQueue->ExecuteCommandLists(1, pLists);

		s_copyFenceValue++;
		s_copyQueue->Signal(s_copyFence.Get(), s_copyFenceValue);
		_waitForCopyFence();

		// Move it into the buffer our PixelBuffers point into. Both use the same
		// padded pitch, so this is one straight copy.

		size_t nBytes = size_t(m_uploadPitch) * m_size.h;

		uint8_t * pSrc = nullptr;
		D3D12_RANGE readRange = { 0, nBytes };

		if( SUCCEEDED(m_readbackBuffer->Map(0, &readRange, (void**) &pSrc)) )
		{
			memcpy( m_pUploadData, pSrc, nBytes );

			D3D12_RANGE writtenRange = { 0, 0 };		// We only read.
			m_readbackBuffer->Unmap(0, &writtenRange);
		}

		m_dirtyRect = RectI();			// Our pixels and the texture are identical now.
	}

	//____ alpha() _______________________________________________________________

	int DX12Surface::alpha(CoordSPX coord)
	{
		if( !m_pUploadData )
			return 0;

		if( m_bBufferNeedsSync )
			_syncBufferAndWait();

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
		buf.colorSpace = m_colorSpace;
		buf.bigEndian = m_bBigEndian;
		buf.palette = m_pPalette;
		buf.pitch = m_uploadPitch;
		buf.pixels = m_pUploadData ? m_pUploadData + rect.y * m_uploadPitch + rect.x * m_pixelSize : nullptr;
		buf.rect = rect;

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool DX12Surface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		// The buffer is our pixels, so it is already up to date unless something
		// has been rendered into us since we last looked.

		//TODO: Only read back the lines asked for, not the whole texture.

		if( m_bBufferNeedsSync )
			_syncBufferAndWait();

		return m_pUploadData != nullptr;
	}

	//____ pullPixels() _______________________________________________________

	void DX12Surface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		_addDirtyRect( bufferRect + buffer.rect.pos() );

		// KLUDGE: Surface::copy() into a palette based surface can append colors to
		// the palette through PixelTools::copyPixels(), then calls pullPixels(), so
		// we reconvert the whole palette on every pullPixels() to catch that. The
		// intended solution is Surface methods for updating (parts of) the palette,
		// API not yet designed. Replace this once they exist.

		if( m_bIndexed )
			_updatePaletteBuffer();

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() __________________________________________________

	void DX12Surface::freePixelBuffer(const PixelBuffer& buffer)
	{
		// Nothing to do here.
	}

} // namespace wg
