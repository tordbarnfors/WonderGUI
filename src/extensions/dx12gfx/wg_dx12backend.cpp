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

#include <wg_dx12backend.h>
#include <wg_dx12surface.h>
#include <wg_dx12surfacefactory.h>
#include <wg_dx12edgemap.h>
#include <wg_dx12edgemapfactory.h>
#include <d3dcompiler.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <vector>
#include <utility>
#include <algorithm>

#include <wg_gfxbase.h>
#include <wg_gfxutil.h>



namespace wg
{

	const TypeInfo DX12Backend::TYPEINFO = { "DX12Backend", &GfxBackend::TYPEINFO };

	const int DX12Backend::s_flipCornerOrder[GfxFlip_size][4] = {
		{ 0,1,2,3 },			// Normal
		{ 1,0,3,2 },			// FlipX
		{ 3,2,1,0 },			// FlipY
		{ 3,0,1,2 },			// Rot90
		{ 0,3,2,1 },			// Rot90FlipX
		{ 2,1,0,3 },			// Rot90FlipY
		{ 2,3,0,1 },			// Rot180
		{ 3,2,1,0 },			// Rot180FlipX
		{ 1,0,3,2 },			// Rot180FlipY
		{ 1,2,3,0 },			// Rot270
		{ 2,1,0,3 },			// Rot270FlipX
		{ 0,3,2,1 }				// Rot270FlipY
	};

	// Debug aid: clear each update rect before drawing it, so it is obvious which
	// parts of the canvas are redrawn and whether anything is left unpainted. The
	// window must present full frames while this is on, see beginSession().

	static constexpr bool c_bDebugClearUpdateRects = false;

	// Shaders were compiled unoptimized whatever the build, which the blur shader
	// with its nine taps is the first one to really feel.

#ifdef _DEBUG
	static constexpr UINT c_shaderCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
												| D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;
#else
	static constexpr UINT c_shaderCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3
												| D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;
#endif

	//____ _checkHR() __________________________________________________________
	//
	// Reports failed HRESULTs through the error handler. Use the CHECK_HR macro,
	// which passes on the call site.

	static bool _checkHR(HRESULT hr, const char* what, const Object* pObject, const TypeInfo* pClassType, const char* func, const char* file, int line)
	{
		if (FAILED(hr))
		{
			char buffer[256];
			sprintf_s(buffer, "%s failed, HRESULT = 0x%08lX", what, (unsigned long)hr);
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, pObject, pClassType, func, file, line);
			assert(false);
			return false;
		}
		return true;
	}

	#define CHECK_HR(call, what) _checkHR(call, what, this, &TYPEINFO, __func__, __FILE__, __LINE__)

	Microsoft::WRL::ComPtr<ID3DBlob>			DX12Backend::s_vertexShaderBlobs[int(PipelineKind::Size)];
	Microsoft::WRL::ComPtr<ID3DBlob>			DX12Backend::s_pixelShaderBlobs[int(PipelineKind::Size)];
	Microsoft::WRL::ComPtr<ID3D12Device>		DX12Backend::s_pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	DX12Backend::s_pCommandQueue;
	bool										DX12Backend::s_bImplicitDevice = false;
	std::vector<DX12Backend*>					DX12Backend::s_backends;

	//____ create() ______________________________________________________________

	DX12Backend_p DX12Backend::create()
	{
		if (!s_pDevice || !s_pCommandQueue)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. Call DX12Backend::setDevice() before creating a backend.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return DX12Backend_p(new DX12Backend(s_pDevice.Get(), s_pCommandQueue.Get()));
	}

	DX12Backend_p DX12Backend::create(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		if (!s_pDevice)
		{
			if (!setDevice(pDX12Device, pDX12CommandQueue))
				return nullptr;
			s_bImplicitDevice = true;
		}
		else if (s_pDevice.Get() != pDX12Device || s_pCommandQueue.Get() != pDX12CommandQueue)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam,
				"All backends must share the device and command queue set by DX12Backend::setDevice().",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return create();
	}

	//____ setDevice() ___________________________________________________________

	bool DX12Backend::setDevice(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		if (!s_backends.empty())
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall,
				"Can't change D3D12 device while DX12Backends exist.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if ((pDX12Device == nullptr) != (pDX12CommandQueue == nullptr))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam,
				"Device and command queue must both be set or both be null.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if (s_pDevice)
		{
			DX12Surface::exitDevice();
			DX12Edgemap::exitDevice();

			for (auto& blob : s_vertexShaderBlobs)
				blob.Reset();
			for (auto& blob : s_pixelShaderBlobs)
				blob.Reset();
		}

		s_pDevice = pDX12Device;
		s_pCommandQueue = pDX12CommandQueue;
		s_bImplicitDevice = false;

		if (pDX12Device)
		{
			// Surfaces and edgemaps create their own resources and need the device for it.

			DX12Surface::setDevice(pDX12Device);
			DX12Edgemap::setDevice(pDX12Device);
		}

		return true;
	}

	//____ waitForCompletionOfAll() ______________________________________________

	void DX12Backend::waitForCompletionOfAll()
	{
		for (auto pBackend : s_backends)
			pBackend->waitForCompletion();
	}

	//____ Constructor ___________________________________________________________

	DX12Backend::DX12Backend(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		m_pSurfaceFactory = DX12SurfaceFactory::create();
		m_pEdgemapFactory = DX12EdgemapFactory::create();

		m_pDX12CommandQueue = pDX12CommandQueue;
		m_pDX12Device = pDX12Device;

		s_backends.push_back(this);

		m_srvDescriptorSize = pDX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_samplerDescriptorSize = pDX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		pDX12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_commandFence));
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		// Create a command allocator for each frame resource.
		//
		// Each frame resource also has its own vertex, color and extras buffers,
		// since the GPU may still be reading the other frame's. They are created
		// by beginSession(), sized by what the sessions say they need, see
		// _reserveBuffers().

		for (int i = 0; i < c_nbFrameResources; ++i)
		{
			pDX12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[i].commandAllocator));
			m_frameResources[i].fenceValue = 0;

			// Shader visible descriptors for blit sources, refilled every frame.

			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.NumDescriptors = c_nbSRVDescriptors;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			CHECK_HR(pDX12Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_frameResources[i].srvHeap.GetAddressOf())), "CreateDescriptorHeap");
		}

		// Create one command list for all frames (will be reset for each frame)

		pDX12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

		m_commandList->Close();
		m_bCommandListOpen = false;

		if (!_createPipelineResources())
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create pipeline resources, nothing will render.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);

	}

	//____ Destructor ____________________________________________________________

	DX12Backend::~DX12Backend()
	{
		// Our resources must not be released while the GPU still uses them.

		waitForCompletion();

		s_backends.erase(std::remove(s_backends.begin(), s_backends.end(), this), s_backends.end());

		// Release the device if create(device, queue) set it for us.

		if (s_backends.empty() && s_bImplicitDevice)
			setDevice(nullptr, nullptr);

		if (m_fenceEvent)
			CloseHandle(m_fenceEvent);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12Backend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________________

	void DX12Backend::beginRender()
	{
		m_currentFrameIndex = (m_currentFrameIndex + 1) % c_nbFrameResources;
		int frame = m_currentFrameIndex;

		_waitForFence(m_frameResources[frame].fenceValue);

		m_frameResources[frame].commandAllocator->Reset();
		m_commandList->Reset(m_frameResources[frame].commandAllocator.Get(), nullptr);
		m_bCommandListOpen = true;

		// Buffers this frame resource outgrew last time it was used. The fence
		// says the GPU is done with them.

		m_frameResources[frame].retiredBuffers.clear();

		// Vertices and colors are written for the whole frame, not per session,
		// since the GPU doesn't run any of it until the command list has been
		// executed. The buffers may not exist yet, beginSession() creates or grows
		// them as needed.

		m_pVertexBeg = m_frameResources[frame].pVertexBufferData;
		m_pVertexEnd = m_pVertexBeg ? m_pVertexBeg + m_frameResources[frame].vertexCapacity : nullptr;
		m_pVertexPtr = m_pVertexBeg;

		m_pColorBeg = m_frameResources[frame].pColorBufferData;
		m_pColorEnd = m_pColorBeg ? m_pColorBeg + m_frameResources[frame].colorCapacity : nullptr;
		m_pColorPtr = m_pColorBeg;

		m_pExtrasBeg = m_frameResources[frame].pExtrasBufferData;
		m_pExtrasEnd = m_pExtrasBeg ? m_pExtrasBeg + m_frameResources[frame].extrasCapacity : nullptr;
		m_pExtrasPtr = m_pExtrasBeg;

		m_frameResources[frame].nSRVDescriptors = 0;

		// The fence says the GPU is done with the command list that mentioned
		// these, so we can let go of them. Edgemaps are in here too: a command list
		// keeps no references of its own, and GfxDeviceGen2 lets go of an edgemap
		// at the end of the session that drew it.

		m_frameResources[frame].objectRefs.clear();

		// Sessions un-park what they parked, so this should already be empty.

		m_blitSourceCanvases.clear();

		m_pActivePipeline = nullptr;		// Resetting the command list cleared its state.
		m_bBlitSourceBound = false;
	}

	//____ endRender() _________________________________________________________

	void DX12Backend::endRender()
	{
		if (!m_bCommandListOpen)
			return;

		_restoreBlitSourceCanvases();

		// Close() must not be inside the assert, or it is never called in release builds.

		HRESULT hr = m_commandList->Close();

		m_bCommandListOpen = false;

		if (!CHECK_HR(hr, "ID3D12GraphicsCommandList::Close"))
			return;

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };

		 m_pDX12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	 
		 // Signal and increment the fence value.

		m_fenceValue++;

		m_pDX12CommandQueue->Signal(m_commandFence.Get(), m_fenceValue);
		m_frameResources[m_currentFrameIndex].fenceValue = m_fenceValue;

	}

	//____ _flushCommandList() _________________________________________________
	//
	// Submits what we have recorded so far, so that waiting for it means anything,
	// then reopens the list. Resetting the list throws away all its state, so this
	// must only happen between sessions - beginSession() sets up everything again.

	void DX12Backend::_flushCommandList()
	{
		if (!m_bCommandListOpen)
			return;

		_restoreBlitSourceCanvases();

		HRESULT hr = m_commandList->Close();

		if (!CHECK_HR(hr, "ID3D12GraphicsCommandList::Close"))
		{
			m_bCommandListOpen = false;
			return;
		}

		ID3D12CommandList* pCommandLists[] = { m_commandList.Get() };
		m_pDX12CommandQueue->ExecuteCommandLists(1, pCommandLists);

		m_fenceValue++;

		m_pDX12CommandQueue->Signal(m_commandFence.Get(), m_fenceValue);
		m_frameResources[m_currentFrameIndex].fenceValue = m_fenceValue;

		// The allocator keeps the memory of what we just submitted, it is only
		// reset in beginRender() once the fence says the GPU is done with it.

		m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(), nullptr);

		m_pActivePipeline = nullptr;
		m_bBlitSourceBound = false;

		// A flush can happen in the middle of a session, when a surface needs the
		// GPU to catch up, so put back everything the reset threw away.

		if (m_bInSession)
		{
			_bindSessionState();
			_bindCanvasState();
		}
	}

	//____ _bindSessionState() _________________________________________________
	//
	// Everything a session needs that a freshly reset command list doesn't have.

	void DX12Backend::_bindSessionState()
	{
		if (!m_bCommandListOpen)
			return;

		auto& frame = m_frameResources[m_currentFrameIndex];

		// Descriptor heaps must be set before any descriptor table is bound.

		if (frame.srvHeap && m_pSamplerHeap)
		{
			ID3D12DescriptorHeap* pHeaps[] = { frame.srvHeap.Get(), m_pSamplerHeap.Get() };
			m_commandList->SetDescriptorHeaps(2, pHeaps);
		}

		m_commandList->SetGraphicsRootSignature(m_pRootSignature.Get());

		// Colors and extras live in buffers the vertex shader indexes into, like
		// MetalBackend does. Root SRVs take the address directly.

		if (frame.colorBuffer)
			m_commandList->SetGraphicsRootShaderResourceView(1, frame.colorBuffer->GetGPUVirtualAddress());

		if (frame.extrasBuffer)
			m_commandList->SetGraphicsRootShaderResourceView(2, frame.extrasBuffer->GetGPUVirtualAddress());

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (frame.vertexBuffer)
		{
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
			vertexBufferView.BufferLocation = frame.vertexBuffer->GetGPUVirtualAddress();
			vertexBufferView.StrideInBytes = sizeof(Vertex);
			vertexBufferView.SizeInBytes = UINT(frame.vertexCapacity * sizeof(Vertex));

			m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView );
		}

		_setBlendFactor();
		_bindTintmap();

		// Setting the root signature dropped the pipeline and whatever the blit
		// source had bound.

		m_pActivePipeline = nullptr;
		m_bBlitSourceBound = false;
	}

	//____ _bindCanvasState() __________________________________________________
	//
	// Points the command list at the canvas _setCanvas() picked out.

	void DX12Backend::_bindCanvasState()
	{
		if (!m_bCommandListOpen || m_activeCanvasRTV.ptr == 0)
			return;								// Nothing to bind, or nowhere to bind it.

		m_commandList->OMSetRenderTargets(1, &m_activeCanvasRTV, FALSE, nullptr);

		// Viewport and scissor are in pixels. Canvas size is in spx for the default
		// canvas, so _setCanvas() divided it by 64.

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (FLOAT) m_activeCanvasSize.w;
		viewport.Height = (FLOAT) m_activeCanvasSize.h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_commandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = (LONG) m_activeCanvasSize.w;
		scissorRect.bottom = (LONG) m_activeCanvasSize.h;
		m_commandList->RSSetScissorRects(1, &scissorRect);

		// Vertex positions are in canvas pixels, the vertex shader needs the canvas
		// size to bring them into clip space.

		float canvasScale[2] = { m_activeCanvasSize.w > 0 ? 2.f / m_activeCanvasSize.w : 0.f,
								 m_activeCanvasSize.h > 0 ? 2.f / m_activeCanvasSize.h : 0.f };

		m_commandList->SetGraphicsRoot32BitConstants(0, 2, canvasScale, 0);

		// Pipelines are tied to the render target format, so the one we had is no
		// longer the one to use.

		m_pActivePipeline = nullptr;
	}

	//____ _restoreBlitSourceCanvases() ________________________________________
	//
	// Canvas surfaces we read as blit source were barriered into
	// PIXEL_SHADER_RESOURCE. They go back to COMMON before the list is closed, so
	// everything rests in COMMON between frames, which is what the copy queue and
	// D3D12's own promotion rules expect.

	void DX12Backend::_restoreBlitSourceCanvases()
	{
		if (!m_bCommandListOpen)
			return;

		auto& frame = m_frameResources[m_currentFrameIndex];

		for (auto& pSurface : m_blitSourceCanvases)
		{
			auto pCanvas = static_cast<DX12Surface*>(pSurface.rawPtr());

			// The one we are rendering into belongs in RENDER_TARGET until the
			// session lets go of it, so leave that one alone.

			if (pCanvas != m_pActiveCanvas)
				_transitionCanvas(pCanvas, D3D12_RESOURCE_STATE_COMMON);

			// A command list holds no references of its own, so we keep one until
			// the GPU is done with it.

			frame.objectRefs.push_back(pSurface);
		}

		m_blitSourceCanvases.clear();
	}

	//____ _isDX12Surface() ____________________________________________________
	//
	// TypeInfo compares by address, so a plain == would turn away anything derived
	// from DX12Surface. Walk the chain instead.

	bool DX12Backend::_isOfType(const TypeInfo& type, const TypeInfo& base)
	{
		for (const TypeInfo* pType = &type; pType; pType = pType->pSuperClass)
		{
			if (pType == &base)
				return true;
		}

		return false;
	}

	bool DX12Backend::_isDX12SurfaceType(const TypeInfo& type)
	{
		return _isOfType(type, DX12Surface::TYPEINFO);
	}

	bool DX12Backend::_isDX12Surface(const Object* pObject) const
	{
		return pObject ? _isDX12SurfaceType(pObject->typeInfo()) : false;
	}

	//____ beginSession() _____________________________________________________

	void DX12Backend::beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo)
	{
		DX12Surface * pCanvasSurface = nullptr;

		if (pCanvas)
		{
			if (_isDX12Surface(pCanvas))
				pCanvasSurface = static_cast<DX12Surface*>(pCanvas);
			else
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Canvas is not a DX12Surface, rendering to default canvas instead.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		// The swap chain buffer is only ours while we render into it, and only if
		// this session renders into it at all.

		m_bSessionOnDefaultCanvas = (pCanvasSurface == nullptr);

		if (!m_bCommandListOpen)
			return;					// Nothing can be recorded, beginRender() was never called or the list broke.

		if (m_bSessionOnDefaultCanvas && m_defaultCanvasBuffer)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = m_defaultCanvasBuffer;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_commandList->ResourceBarrier(1, &barrier);
		}

		m_bInSession = true;

		// Sessions start with default state, changes arrive as StateChange commands.
		// This has to happen before the command list is set up, since some of it -
		// the morph factor - is recorded into the list rather than kept in a
		// pipeline. GfxDeviceGen2 encodes a state change only when a value differs
		// from these defaults, so getting them out of step means a whole session
		// draws with the previous one's value.

		m_tintColor = HiColor::White;
		m_tintmap = {};
		m_activeBlendMode = BlendMode::Blend;
		m_morphFactor = 0.5f;

		// No brush until a StateChange says otherwise. Should one ever be missing,
		// this makes a blur come out as a plain blit rather than as something built
		// from whatever the previous session left behind.

		m_blurRadius = 0;

		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 4; j++)
				m_blurColorMtx[i][j] = 0.f;

		m_blurColorMtx[4][0] = 1.f;
		m_blurColorMtx[4][1] = 1.f;
		m_blurColorMtx[4][2] = 1.f;
		m_blurColorMtx[4][3] = 1.f;

		_setBlitSource(nullptr);

		// Room for everything the session may write, in buffers the session state
		// then points at.

		_reserveBuffers(pInfo);

		_bindSessionState();

		// Sets render target, viewport and the canvas scale our shaders need.

		m_pActiveCanvas = nullptr;
		_setCanvas(pCanvasSurface);

		// We must not touch a single pixel outside the update rects: the swap chain
		// is presented with them as dirty rects, and DXGI copies everything else
		// from the previously presented buffer. Hence no clear of the canvas.
		//
		// The rects we get for the draw commands are already clipped against the
		// update rects by GfxDeviceGen2, so no extra clipping is needed here.
		//
		// Set c_bDebugClearUpdateRects to make the updated areas stand out, which
		// also shows which parts of the canvas are being redrawn. Note that this
		// paints the whole update rect, which breaks the rule above, so the window
		// must present full frames while it is on.

		if (c_bDebugClearUpdateRects && nUpdateRects > 0 && m_bCommandListOpen && m_activeCanvasRTV.ptr != 0)
		{
			std::vector<D3D12_RECT> rects;
			rects.reserve(nUpdateRects);

			for (int i = 0; i < nUpdateRects; i++)
			{
				const RectSPX& rect = pUpdateRects[i];

				D3D12_RECT r;
				r.left = rect.x / 64;
				r.top = rect.y / 64;
				r.right = (rect.x + rect.w) / 64;
				r.bottom = (rect.y + rect.h) / 64;
				rects.push_back(r);
			}

			float clearColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
			m_commandList->ClearRenderTargetView(m_activeCanvasRTV, clearColor, (UINT) rects.size(), rects.data());
		}
	}

	//____ endSession() _______________________________________________________

	void DX12Backend::endSession()
	{
		// A canvas surface goes back to COMMON, where D3D12 promotes it on its own
		// when it is read as a blit source or copied from.

		if (m_pActiveCanvas)
		{
			_transitionCanvas(m_pActiveCanvas, D3D12_RESOURCE_STATE_COMMON);
			m_pActiveCanvas = nullptr;
		}

		// Nothing stays parked between sessions, or the next one would find a
		// surface in a state it doesn't expect.

		_restoreBlitSourceCanvases();

		if (m_bSessionOnDefaultCanvas && m_defaultCanvasBuffer && m_bCommandListOpen)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = m_defaultCanvasBuffer;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_commandList->ResourceBarrier(1, &barrier);

			m_bSessionOnDefaultCanvas = false;
		}

		m_bInSession = false;
	}

	//____ setCanvas() ________________________________________________________
	//
	// Called between processCommands() within a session, when a render layer has
	// a canvas of its own.

	void DX12Backend::setCanvas(Surface* pSurface)
	{
		if (pSurface && !_isDX12Surface(pSurface))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Canvas is not a DX12Surface.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		_setCanvas(static_cast<DX12Surface*>(pSurface));
	}

	void DX12Backend::setCanvas(CanvasRef ref)
	{
		if (ref == CanvasRef::Default)
			_setCanvas(nullptr);
		else
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only CanvasRef::Default is supported.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	//____ _transitionCanvas() ________________________________________________

	void DX12Backend::_transitionCanvas(DX12Surface* pCanvas, D3D12_RESOURCE_STATES state)
	{
		if (!m_bCommandListOpen || !pCanvas || !pCanvas->texture() || pCanvas->resourceState() == state)
			return;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pCanvas->texture();
		barrier.Transition.StateBefore = pCanvas->resourceState();
		barrier.Transition.StateAfter = state;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &barrier);

		pCanvas->setResourceState(state);
	}

	//____ _setCanvas() _______________________________________________________
	//
	// Points the command list at the canvas we should render into from here on.
	// A null surface means the default canvas, which the window owns.

	void DX12Backend::_setCanvas(DX12Surface* pCanvas)
	{
		if (pCanvas && (!pCanvas->texture() || !pCanvas->canBeCanvas() || pCanvas->renderTargetView().ptr == 0))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Surface can not be used as canvas.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			pCanvas = nullptr;
		}

		// Getting here can flush the command list, and a flush rebinds the canvas.
		// Make sure that finds nothing to bind rather than the old canvas, which
		// may be on its way out of existence.

		m_activeCanvasRTV.ptr = 0;
		m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;

		// Whatever we rendered into before is done now.

		if (m_pActiveCanvas && m_pActiveCanvas != pCanvas)
			_transitionCanvas(m_pActiveCanvas, D3D12_RESOURCE_STATE_COMMON);

		// We can't read from what we paint into.

		if (pCanvas && pCanvas == m_pBlitSource)
			_setBlitSource(nullptr);

		m_pActiveCanvas = pCanvas;

		if (pCanvas)
		{
			// Gets any pixels the CPU wrote into the texture before we render over
			// it, and marks our copy of them as the older one from here on. That
			// upload runs on the copy queue, which can only reach a texture that
			// rests in COMMON, so the surface goes back there first. It usually
			// already is, in which case the call below records nothing.

			_transitionCanvas(pCanvas, D3D12_RESOURCE_STATE_COMMON);

			pCanvas->notifyRendered();

			_transitionCanvas(pCanvas, D3D12_RESOURCE_STATE_RENDER_TARGET);

			m_activeCanvasRTV = pCanvas->renderTargetView();
			m_activeCanvasSize = pCanvas->pixelSize();
			m_activeCanvasFormat = pCanvas->dxgiFormat();
		}
		else
		{
			// A session that started on a surface canvas has not claimed the swap
			// chain buffer yet. endSession() hands it back.

			if (!m_bSessionOnDefaultCanvas && m_defaultCanvasBuffer && m_bCommandListOpen)
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = m_defaultCanvasBuffer;
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

				m_commandList->ResourceBarrier(1, &barrier);

				m_bSessionOnDefaultCanvas = true;
			}

			m_activeCanvasRTV = m_defaultCanvasRTV;
			m_activeCanvasSize = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };
			m_activeCanvasFormat = m_defaultCanvasFormat;
		}

		_bindCanvasState();
	}

	//____ setObjects() ________________________________________________________

	void DX12Backend::setObjects(Object* const* pBeg, Object* const* pEnd)
	{
		m_pObjectsBeg = pBeg;
		m_pObjectsEnd = pEnd;
		m_pObjectsPtr = pBeg;
	}

	//____ setRects() __________________________________________________________

	void DX12Backend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
		m_pRectsBeg = pBeg;
		m_pRectsEnd = pEnd;
		m_pRectsPtr = pBeg;
	}

	//____ setColors() _________________________________________________________

	void DX12Backend::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
		m_pColorsBeg = pBeg;
		m_pColorsEnd = pEnd;
		m_pColorsPtr = pBeg;
	}

	//____ setTransforms() _____________________________________________________

	void DX12Backend::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
		m_pTransformsBeg = pBeg;
		m_pTransformsEnd = pEnd;
	}

	//____ processCommands() ___________________________________________________

	void DX12Backend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version )
	{
		const RectSPX*	pRects = m_pRectsPtr;
		const HiColor*	pColors = m_pColorsPtr;
		Object* const*	pObjects = m_pObjectsPtr;

		auto p = pBeg;
		while (p < pEnd)
		{
			auto cmd = Command(*p++);
			switch (cmd)
			{
				case Command::None:
					break;

				case Command::StateChange:
				{
					// Every payload has to be stepped past even when we ignore it,
					// or we lose track of where we are in the command stream.

					int32_t statesChanged = *p++;

					if (statesChanged & uint8_t(StateChange::BlitSource))
					{
						Object * pObject = *pObjects++;

						_setBlitSource( _isDX12Surface(pObject) ? static_cast<DX12Surface*>(pObject) : nullptr );
					}

					// A tint color replaces any tintmap and a tintmap replaces the tint
					// color. GfxDeviceGen2 has already folded one into the other when
					// both are in use, so we never have to combine them.

					if (statesChanged & uint8_t(StateChange::TintColor))
					{
						m_tintColor = *pColors++;
						_clearTintmap();
					}

					if (statesChanged & uint8_t(StateChange::TintMap))
					{
						auto p32 = (const spx*) p;

						int32_t	x = *p32++;
						int32_t	y = *p32++;
						int32_t	w = *p32++;
						int32_t	h = *p32++;

						int32_t	nHorrColors = *p32++;
						int32_t	nVertColors = *p32++;

						p = (const uint16_t*) p32;

						m_tintColor = HiColor::White;
						_setTintmap(pColors, nHorrColors, nVertColors, RectI(x, y, w, h) / 64);
					}

					if (statesChanged & uint8_t(StateChange::BlendMode))
						m_activeBlendMode = BlendMode(*p++);

					if (statesChanged & uint8_t(StateChange::MorphFactor))
					{
						float morphFactor = (*p++) / 4096.f;

						if (morphFactor != m_morphFactor)
						{
							m_morphFactor = morphFactor;
							_setBlendFactor();
						}
					}

					if (statesChanged & uint8_t(StateChange::FixedBlendColor))
					{
						// Blending against a known background is an optimization for
						// software rendering. We render it as an ordinary Blend, which
						// gives the same result, so the color is of no use to us.

						pColors++;
					}

					if (statesChanged & uint8_t(StateChange::Blur))
					{
						m_blurRadius = *p++;

						// Nine taps, each with its own weight per color channel. The
						// values are 15 bit fixed point, so they can go above one.

						for (int i = 0; i < 9; i++)
						{
							m_blurColorMtx[i][0] = p[i] / 32768.f;
							m_blurColorMtx[i][1] = p[9+i] / 32768.f;
							m_blurColorMtx[i][2] = p[18+i] / 32768.f;
							m_blurColorMtx[i][3] = 0.f;
						}

						m_blurColorMtx[4][3] = 1.f;

						p += 27;
					}

					// Take care of alignment

					if( (uintptr_t(p) & 0x2) == 2 )
						p++;

					break;
				}

				case Command::Fill:
				{
					int32_t nRects = *p++;
					HiColor col = *pColors++;

					_drawFillRects(pRects, nRects, col);
					pRects += nRects;
					break;
				}

				case Command::Blit:
				case Command::ClipBlit:
				case Command::Tile:
				case Command::Blur:
				{
					// All four carry the same payload and cover the same geometry.
					// Whether the source is clamped or tiled is a property of the
					// surface, and the rects have already been clipped by
					// GfxDeviceGen2, so only a blur draws differently - it reads nine
					// texels per pixel instead of one.

					int32_t nRects = *p++;

					_drawBlitRects(p, pRects, nRects, version,
								   cmd == Command::Blur ? PipelineKind::Blur : PipelineKind::Blit);
					break;
				}

				case Command::Line:
				{
					int32_t nClipRects = *p++;
					int32_t nLines = *p++;
					p++;								// padding

					_drawLines(p, pRects, pColors, nClipRects, nLines);
					break;
				}

				case Command::DrawEdgemap:
				{
					int32_t nRects = *p++;

					_drawEdgemap(p, pRects, pObjects, nRects);
					break;
				}

				default:
				{
					// Every command we know of is either drawn or stepped over above,
					// so this is a command that didn't exist when this was written. We
					// don't know how big its payload is, so there is no way to find the
					// next one and the rest of the session has to go.

					static bool bReported = false;
					if (!bReported)
					{
						char buffer[128];
						sprintf_s(buffer, "Command %d is unknown, rest of session dropped.", (int)cmd);
						GfxBase::throwError(ErrorLevel::Error, ErrorCode::Other, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
						bReported = true;
					}

					p = pEnd;
					break;
				}
			}
		}

		m_pRectsPtr = pRects;
		m_pColorsPtr = pColors;
		m_pObjectsPtr = pObjects;
	}

	//____ _drawFillRects() ____________________________________________________
	//
	// A rectangle that lands on whole pixels is just a quad. One that doesn't needs
	// the coverage of the pixels along its edges worked out, which is a different
	// pipeline and an entry in the extras buffer. A single command can hold both
	// kinds, so runs are drawn as the kind changes, the way MetalBackend does it.

	void DX12Backend::_drawFillRects(const RectSPX* pRects, int nRects, HiColor color)
	{
		if (nRects <= 0 || !m_pVertexPtr)
			return;

		// One color for the whole command, the vertices only carry its offset.

		int colorOfs = _addColor(color);
		if (colorOfs < 0)
			return;

		PipelineKind kind = PipelineKind::Fill;

		int firstVertex = int(m_pVertexPtr - m_pVertexBeg);
		int nRectsInRun = 0;

		for (int i = 0; i < nRects; i++)
		{
			const RectSPX& rect = pRects[i];

			spx x2spx = rect.x + rect.w;
			spx y2spx = rect.y + rect.h;

			PipelineKind rectKind = (((rect.x | rect.y | x2spx | y2spx) & 63) == 0) ? PipelineKind::Fill : PipelineKind::FillAA;

			if (rectKind != kind)
			{
				_drawFillRun(kind, firstVertex, nRectsInRun);

				kind = rectKind;
				firstVertex = int(m_pVertexPtr - m_pVertexBeg);
				nRectsInRun = 0;
			}

			if (m_pVertexEnd - m_pVertexPtr < 6)
			{
				static bool bReported = false;
				if (!bReported)
				{
					GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Vertex buffer full, fills dropped. SessionInfo underestimated what the session needs.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
					bReported = true;
				}

				break;
			}

			int extrasOfs = 0;

			float x1, y1, x2, y2;

			if (kind == PipelineKind::Fill)
			{
				x1 = float(rect.x >> 6);
				y1 = float(rect.y >> 6);
				x2 = float(x2spx >> 6);
				y2 = float(y2spx >> 6);
			}
			else
			{
				// Center and radius of the rectangle, in pixels, for the shader.

				float radiusX = rect.w / 128.f;
				float radiusY = rect.h / 128.f;

				ExtrasDX12 centerAndRadius = { rect.x / 64.f + radiusX, rect.y / 64.f + radiusY, radiusX, radiusY };

				extrasOfs = _addExtras(centerAndRadius);
				if (extrasOfs < 0)
					break;

				// The quad has to cover every pixel the rectangle touches, so it
				// reaches out to whole pixels in both directions.

				x1 = float(rect.x >> 6);
				y1 = float(rect.y >> 6);
				x2 = float((x2spx + 63) >> 6);
				y2 = float((y2spx + 63) >> 6);
			}

			const float coords[6][2] = { {x1,y1}, {x2,y1}, {x2,y2},
										 {x1,y1}, {x2,y2}, {x1,y2} };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				m_pVertexPtr->x = coords[vertex][0];
				m_pVertexPtr->y = coords[vertex][1];
				m_pVertexPtr->colorOfs = (uint32_t) colorOfs;
				m_pVertexPtr->extrasOfs = (uint32_t) extrasOfs;

				m_pVertexPtr++;
			}

			nRectsInRun++;
		}

		_drawFillRun(kind, firstVertex, nRectsInRun);
	}

	//____ _drawFillRun() ______________________________________________________

	void DX12Backend::_drawFillRun(PipelineKind kind, int firstVertex, int nRects)
	{
		if (nRects <= 0)
			return;

		if (!_setPipeline(_pipeline(m_activeBlendMode, kind)))
			return;

		m_commandList->DrawInstanced(nRects * 6, 1, firstVertex, 0);
	}

	//____ _drawLines() ________________________________________________________
	//
	// Lines are quads that cover everything the line might touch, with the line
	// itself handed to the pixel shader as an offset, a half width and a slope. It
	// works out the coverage of each pixel from those, which is what antialiases
	// the edges. The geometry follows MetalBackend, except that we never flip Y.

	void DX12Backend::_drawLines(const uint16_t*& pCmd, const RectSPX*& pRects, const HiColor*& pColors, int nClipRects, int nLines)
	{
		bool bDraw = _setPipeline(_pipeline(m_activeBlendMode, PipelineKind::Line)) && m_pVertexPtr != nullptr;

		int firstVertex = bDraw ? int(m_pVertexPtr - m_pVertexBeg) : 0;
		int nLinesDrawn = 0;

		for (int i = 0; i < nLines; i++)
		{
			// Even when we can't draw we have to step through the command's data,
			// or we lose track of where we are in the command stream.

			HiColor col = *pColors++;

			auto p32 = (const spx*) pCmd;

			CoordSPX beginSpx, endSpx;

			beginSpx.x = *p32++;
			beginSpx.y = *p32++;
			endSpx.x = *p32++;
			endSpx.y = *p32++;

			pCmd = (const uint16_t*) p32;

			float thickness = *pCmd++ / 64.f;
			pCmd++;								// padding

			if (!bDraw)
				continue;

			if (m_pVertexEnd - m_pVertexPtr < 6)
			{
				static bool bReported = false;
				if (!bReported)
				{
					GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Vertex buffer full, lines dropped. SessionInfo underestimated what the session needs.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
					bReported = true;
				}

				continue;
			}

			CoordI begin = Util::roundToPixels(beginSpx);
			CoordI end = Util::roundToPixels(endSpx);

			float	width;
			float	slope;
			float	s, w;
			bool	bSteep;

			CoordI	c1, c2, c3, c4;

			if (std::abs(begin.x - end.x) > std::abs(begin.y - end.y))
			{
				// Mainly horizontal, so it is measured along X.

				if (begin.x > end.x)
					std::swap(begin, end);

				int length = end.x - begin.x;
				if (length == 0)
					continue;						//TODO: Should still draw the caps!

				slope = ((float)(end.y - begin.y)) / length;
				width = _scaleThickness(thickness, slope);
				bSteep = false;

				s = (begin.y + 0.5f) - (begin.x + 0.5f) * slope;
				w = width / 2 + 0.5f;

				float y1 = begin.y - width / 2;
				float y2 = end.y - width / 2;

				c1.x = begin.x;
				c1.y = int(y1) - 1;
				c2.x = end.x;
				c2.y = int(y2) - 1;
				c3.x = end.x;
				c3.y = int(y2 + width) + 2;
				c4.x = begin.x;
				c4.y = int(y1 + width) + 2;
			}
			else
			{
				// Mainly vertical, so X and Y swap roles.

				if (begin.y > end.y)
					std::swap(begin, end);

				int length = end.y - begin.y;
				if (length == 0)
					continue;						//TODO: Should still draw the caps!

				slope = ((float)(end.x - begin.x)) / length;
				width = _scaleThickness(thickness, slope);
				bSteep = true;

				s = (begin.x + 0.5f) - (begin.y + 0.5f) * slope;
				w = width / 2 + 0.5f;

				float x1 = begin.x - width / 2;
				float x2 = end.x - width / 2;

				c1.x = int(x1) - 1;
				c1.y = begin.y;
				c2.x = int(x1 + width) + 2;
				c2.y = begin.y;
				c3.x = int(x2 + width) + 2;
				c3.y = end.y;
				c4.x = int(x2) - 1;
				c4.y = end.y;
			}

			ExtrasDX12 lineInfo = { s, w, slope, bSteep ? 1.f : 0.f };

			int extrasOfs = _addExtras(lineInfo);
			if (extrasOfs < 0)
				continue;

			int colorOfs = _addColor(col);
			if (colorOfs < 0)
				continue;

			const CoordI coords[6] = { c1, c2, c3, c1, c3, c4 };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				m_pVertexPtr->x = float(coords[vertex].x);
				m_pVertexPtr->y = float(coords[vertex].y);
				m_pVertexPtr->colorOfs = (uint32_t) colorOfs;
				m_pVertexPtr->extrasOfs = (uint32_t) extrasOfs;

				m_pVertexPtr++;
			}

			nLinesDrawn++;
		}

		// The clip rects are ours to apply. Unlike fills and blits, the rects of a
		// line command are not the geometry, they are what it may be drawn inside,
		// so the same lines are drawn once per rect with the scissor set to it.
		//
		// Nothing in this loop may flush the command list: a flush rebinds the
		// canvas, which puts the scissor back to the whole canvas and would quietly
		// unclip every rect after it.

		if (nLinesDrawn > 0 && m_bCommandListOpen)
		{
			for (int i = 0; i < nClipRects; i++)
			{
				const RectSPX& clip = *pRects++;

				D3D12_RECT scissorRect;
				scissorRect.left = clip.x >> 6;
				scissorRect.top = clip.y >> 6;
				scissorRect.right = (clip.x + clip.w) >> 6;
				scissorRect.bottom = (clip.y + clip.h) >> 6;

				m_commandList->RSSetScissorRects(1, &scissorRect);
				m_commandList->DrawInstanced(nLinesDrawn * 6, 1, firstVertex, 0);
			}

			// Back to the whole canvas, which is what everything else expects.

			D3D12_RECT fullRect = {};
			fullRect.right = (LONG) m_activeCanvasSize.w;
			fullRect.bottom = (LONG) m_activeCanvasSize.h;

			m_commandList->RSSetScissorRects(1, &fullRect);
		}
		else
			pRects += nClipRects;
	}

	//____ _addColor() _________________________________________________________

	int DX12Backend::_addColor(HiColor color)
	{
		if (!m_pColorPtr)
			return -1;

		if (m_pColorPtr == m_pColorEnd)
		{
			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Color buffer full, draws dropped. SessionInfo underestimated what the session needs.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			return -1;
		}

		// HiColor components are 13 bit fixed point, tint is a plain multiplication.

		m_pColorPtr->r = (color.r / 4096.f) * (m_tintColor.r / 4096.f);
		m_pColorPtr->g = (color.g / 4096.f) * (m_tintColor.g / 4096.f);
		m_pColorPtr->b = (color.b / 4096.f) * (m_tintColor.b / 4096.f);
		m_pColorPtr->a = (color.a / 4096.f) * (m_tintColor.a / 4096.f);

		return int(m_pColorPtr++ - m_pColorBeg);
	}

	//____ _setTintmap() _______________________________________________________
	//
	// Copies the tintmap's colors, horizontal ones first, into the color buffer
	// and points the pixel shaders at them. The colors are consumed from the
	// color stream whether or not there is room for them.

	void DX12Backend::_setTintmap(const HiColor*& pColors, int nHorrColors, int nVertColors, const RectI& rect)
	{
		const HiColor* pSource = pColors;
		pColors += nHorrColors + nVertColors;

		int nColors = nHorrColors + nVertColors;

		if (!m_pColorPtr || m_pColorEnd - m_pColorPtr < nColors)
		{
			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Color buffer full, tintmap ignored. SessionInfo underestimated what the session needs.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			_clearTintmap();
			return;
		}

		TintmapInfo info = {};

		int beginOfs = int(m_pColorPtr - m_pColorBeg);

		// Unlike _addColor() these are not tinted. With a tintmap in use the tint
		// color is white.

		for (int i = 0; i < nColors; i++)
		{
			const HiColor& color = pSource[i];

			m_pColorPtr->r = color.r / 4096.f;
			m_pColorPtr->g = color.g / 4096.f;
			m_pColorPtr->b = color.b / 4096.f;
			m_pColorPtr->a = color.a / 4096.f;
			m_pColorPtr++;
		}

		if (nHorrColors > 0)
		{
			info.beginX = beginOfs;
			info.originX = rect.x;
			info.countX = nHorrColors;
		}

		if (nVertColors > 0)
		{
			info.beginY = beginOfs + nHorrColors;
			info.originY = rect.y;
			info.countY = nVertColors;
		}

		m_tintmap = info;
		_bindTintmap();
	}

	//____ _clearTintmap() _____________________________________________________

	void DX12Backend::_clearTintmap()
	{
		if (m_tintmap.countX == 0 && m_tintmap.countY == 0)
			return;

		m_tintmap = {};
		_bindTintmap();
	}

	//____ _bindTintmap() ______________________________________________________
	//
	// Root constants are recorded into the command list, so every draw after this
	// sees the new tintmap and none before it does. A reset list has lost them,
	// which is why _bindSessionState() calls this too.

	void DX12Backend::_bindTintmap()
	{
		if (!m_bCommandListOpen)
			return;

		m_commandList->SetGraphicsRoot32BitConstants(6, 8, &m_tintmap, 0);
	}

	//____ _addExtras() ________________________________________________________

	int DX12Backend::_addExtras(const ExtrasDX12& extras)
	{
		if (!m_pExtrasPtr || m_pExtrasPtr == m_pExtrasEnd)
		{
			static bool bReported = false;
			if (!bReported && m_pExtrasPtr)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Extras buffer full, draws dropped. SessionInfo underestimated what the session needs.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			return -1;
		}

		int ofs = int(m_pExtrasPtr - m_pExtrasBeg);

		*m_pExtrasPtr++ = extras;

		return ofs;
	}

	int DX12Backend::_addBlurExtras()
	{
		const int nEntries = 18;			// Nine color matrices, then nine offsets.

		if (!m_pExtrasPtr || m_pExtrasEnd - m_pExtrasPtr < nEntries)
		{
			static bool bReported = false;
			if (!bReported && m_pExtrasPtr)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Extras buffer full, blurs dropped. SessionInfo underestimated what the session needs.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			return -1;
		}

		int ofs = int(m_pExtrasPtr - m_pExtrasBeg);

		for (int i = 0; i < 9; i++)
		{
			m_pExtrasPtr->x = m_blurColorMtx[i][0];
			m_pExtrasPtr->y = m_blurColorMtx[i][1];
			m_pExtrasPtr->z = m_blurColorMtx[i][2];
			m_pExtrasPtr->w = m_blurColorMtx[i][3];

			m_pExtrasPtr++;
		}

		// The offsets are in texture coordinates, so the radius is measured against
		// the source we are reading from. GlBackend does the same; MetalBackend
		// measures against the canvas, which only comes to the same thing when the
		// two are the same size.

		float radiusX = m_blitSourceSize.w > 0 ? m_blurRadius / float(m_blitSourceSize.w * 64) : 0.f;
		float radiusY = m_blitSourceSize.h > 0 ? m_blurRadius / float(m_blitSourceSize.h * 64) : 0.f;

		const float offsets[9][2] = {
			{ -radiusX * 0.7f,	-radiusY * 0.7f },	{ 0.f, -radiusY },	{ radiusX * 0.7f,	-radiusY * 0.7f },
			{ -radiusX,			0.f },				{ 0.f, 0.f },		{ radiusX,			0.f },
			{ -radiusX * 0.7f,	radiusY * 0.7f },	{ 0.f, radiusY },	{ radiusX * 0.7f,	radiusY * 0.7f } };

		for (int i = 0; i < 9; i++)
		{
			m_pExtrasPtr->x = offsets[i][0];
			m_pExtrasPtr->y = offsets[i][1];
			m_pExtrasPtr->z = 0.f;
			m_pExtrasPtr->w = 0.f;

			m_pExtrasPtr++;
		}

		return ofs;
	}

	int DX12Backend::_addExtras(const ExtrasDX12& first, const ExtrasDX12& second)
	{
		if (!m_pExtrasPtr)
			return -1;

		if (m_pExtrasEnd - m_pExtrasPtr < 2)
		{
			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Extras buffer full, blits dropped. SessionInfo underestimated what the session needs.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			return -1;
		}

		int ofs = int(m_pExtrasPtr - m_pExtrasBeg);

		*m_pExtrasPtr++ = first;
		*m_pExtrasPtr++ = second;

		return ofs;
	}

	//____ _setBlitSource() ____________________________________________________

	void DX12Backend::_setBlitSource(DX12Surface* pSurface)
	{
		m_pBlitSource = pSurface;
		m_bBlitSourceBound = false;
		m_bBlitSourceIndexed = false;

		if (!pSurface)
			return;

		// Reading from the surface we are painting into can't work, the texture
		// can't be render target and shader resource at the same time.

		if (pSurface == m_pActiveCanvas)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"Can't blit from the surface we are rendering into.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			m_pBlitSource = nullptr;
			return;
		}

		// Make sure the texture holds everything written to the surface.

		pSurface->syncTexture();

		m_blitSourceSize = pSurface->pixelSize();
		m_bBlitSourceAlphaOnly = pSurface->isAlphaOnly();
		m_bBlitSourceIndexed = pSurface->isIndexed();

		// Samplers are ordered nearest/bilinear, then clamp/tile.

		m_blitSourceSampler = (pSurface->isTiling() ? 2 : 0) + (pSurface->sampleMethod() == SampleMethod::Bilinear ? 1 : 0);
	}

	//____ _bindBlitSource() ___________________________________________________
	//
	// Copies the source's descriptor into this frame's shader visible heap and
	// points the root descriptor tables at it. Only needed when the source has
	// changed or our bindings have been invalidated.

	bool DX12Backend::_bindBlitSource()
	{
		if (!m_bCommandListOpen || !m_pBlitSource || !m_pBlitSource->texture())
			return false;

		if (m_bBlitSourceBound)
			return true;

		// D3D12 would promote a texture resting in COMMON to a shader resource on
		// its own, but a canvas surface's state is one we track, so we do it with
		// a barrier and put it back before the command list closes.

		if (m_pBlitSource->canBeCanvas() && m_pBlitSource->resourceState() == D3D12_RESOURCE_STATE_COMMON)
		{
			_transitionCanvas(m_pBlitSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_blitSourceCanvases.push_back(m_pBlitSource);
		}

		auto& frame = m_frameResources[m_currentFrameIndex];

		if (!frame.srvHeap || !m_pSamplerHeap)
			return false;

		if (frame.nSRVDescriptors >= c_nbSRVDescriptors)
		{
			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Out of blit source descriptors for this frame. Increase c_nbSRVDescriptors.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			return false;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE dest = frame.srvHeap->GetCPUDescriptorHandleForHeapStart();
		dest.ptr += SIZE_T(frame.nSRVDescriptors) * m_srvDescriptorSize;

		m_pDX12Device->CopyDescriptorsSimple(1, dest, m_pBlitSource->textureSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = frame.srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += UINT64(frame.nSRVDescriptors) * m_srvDescriptorSize;

		frame.nSRVDescriptors++;

		m_commandList->SetGraphicsRootDescriptorTable(3, srvHandle);

		D3D12_GPU_DESCRIPTOR_HANDLE samplerHandle = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
		samplerHandle.ptr += UINT64(m_blitSourceSampler) * m_samplerDescriptorSize;

		m_commandList->SetGraphicsRootDescriptorTable(4, samplerHandle);

		// The shaders need the source size to get from pixels to texture coordinates.

		float textureSize[2] = { (float) m_blitSourceSize.w, (float) m_blitSourceSize.h };
		m_commandList->SetGraphicsRoot32BitConstants(0, 2, textureSize, 2);

		// Bit 0 is for the ordinary shaders. The palette shaders fetch texels
		// themselves, so they need to know what the sampler would have done.

		uint32_t flags = (m_bBlitSourceAlphaOnly ? 1 : 0) |
						 (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear ? 2 : 0) |
						 (m_pBlitSource->isTiling() ? 4 : 0);

		m_commandList->SetGraphicsRoot32BitConstants(0, 1, &flags, 4);

		// A palette based source has its palette in the slot edgemaps use, which is
		// free while we blit. _drawEdgemap() clears m_bBlitSourceBound, so we come
		// back here and put it back afterwards.

		if (m_bBlitSourceIndexed)
		{
			D3D12_GPU_VIRTUAL_ADDRESS palette = m_pBlitSource->paletteGPUAddress();

			if (palette == 0)
				return false;

			m_commandList->SetGraphicsRootShaderResourceView(5, palette);
		}

		m_bBlitSourceBound = true;
		return true;
	}

	//____ _drawBlitRects() ____________________________________________________

	void DX12Backend::_drawBlitRects(const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, int version, PipelineKind kind)
	{
		// Transforms below this index are the standard ones, the rest were handed
		// to us through setTransforms().

		int customTransformStart = (version == 1) ? GfxFlip_size : NbStandardTransforms;

		bool bBilinear = (m_pBlitSource && m_pBlitSource->sampleMethod() == SampleMethod::Bilinear);

		// Even when we can't draw we have to step through the command's data, or
		// we lose track of where we are in the stream.

		bool bDraw = _bindBlitSource() && m_pVertexPtr != nullptr;

		// A palette based source has shaders of its own.

		if (m_bBlitSourceIndexed)
			kind = (kind == PipelineKind::Blur) ? PipelineKind::PaletteBlur : PipelineKind::PaletteBlit;

		// A blur needs its brush where the pixel shader can reach it. It goes in the
		// extras buffer, which the shader indexes into from a root constant.
		//
		// That constant is written again for every blur draw, which is what keeps
		// it right: a mid-session flush sets the root signature again and drops
		// every root argument with it. Skipping this when the brush hasn't changed
		// would need a flag cleared alongside m_bBlitSourceBound.

		if (bDraw && (kind == PipelineKind::Blur || kind == PipelineKind::PaletteBlur))
		{
			int blurOfs = _addBlurExtras();

			if (blurOfs < 0)
				bDraw = false;
			else
			{
				uint32_t ofs = (uint32_t) blurOfs;
				m_commandList->SetGraphicsRoot32BitConstants(0, 1, &ofs, 5);
			}
		}

		if (bDraw)
			bDraw = _setPipeline(_pipeline(m_activeBlendMode, kind));

		// _addColor() multiplies by the tint, so what a blit wants here is white.
		// Passing the tint would apply it twice.

		int colorOfs = bDraw ? _addColor(HiColor::White) : -1;

		if (colorOfs < 0)
			bDraw = false;

		int firstVertex = bDraw ? int(m_pVertexPtr - m_pVertexBeg) : 0;
		int nRectsDrawn = 0;

		for (int i = 0; i < nRects; i++)
		{
			auto p32 = (const spx*) pCmd;

			int		srcX = *p32++;			// Source coordinates are in 1/1024 pixels.
			int		srcY = *p32++;
			spx		dstX = *p32++;
			spx		dstY = *p32++;

			pCmd = (const uint16_t*) p32;

			int32_t transform = *pCmd++;
			pCmd++;							// padding

			const RectSPX& dest = *pRects++;

			if (!bDraw)
				continue;

			if (m_pVertexEnd - m_pVertexPtr < 6)
			{
				static bool bReported = false;
				if (!bReported)
				{
					GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Vertex buffer full, blits dropped. SessionInfo underestimated what the session needs.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
					bReported = true;
				}

				continue;
			}

			// Source and destination origin plus the transform is all the vertex
			// shader needs to work out texture coordinates for each corner.

			ExtrasDX12 srcDst;

			srcDst.x = srcX / 1024.f + (bBilinear ? 0.5f : 0.f);
			srcDst.y = srcY / 1024.f + (bBilinear ? 0.5f : 0.f);
			srcDst.z = float(dstX >> 6) + 0.5f;
			srcDst.w = float(dstY >> 6) + 0.5f;

			auto& mtx = (transform < customTransformStart) ? s_standardTransforms[transform] : m_pTransformsBeg[transform - customTransformStart];

			ExtrasDX12 matrix = { mtx.xx, mtx.xy, mtx.yx, mtx.yy };

			int extrasOfs = _addExtras(srcDst, matrix);
			if (extrasOfs < 0)
				continue;

			float x1 = float(dest.x >> 6);
			float y1 = float(dest.y >> 6);
			float x2 = x1 + float(dest.w >> 6);
			float y2 = y1 + float(dest.h >> 6);

			const float coords[6][2] = { {x1,y1}, {x2,y1}, {x2,y2},
										 {x1,y1}, {x2,y2}, {x1,y2} };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				m_pVertexPtr->x = coords[vertex][0];
				m_pVertexPtr->y = coords[vertex][1];
				m_pVertexPtr->colorOfs = (uint32_t) colorOfs;
				m_pVertexPtr->extrasOfs = (uint32_t) extrasOfs;

				m_pVertexPtr++;
			}

			nRectsDrawn++;
		}

		if (nRectsDrawn > 0)
			m_commandList->DrawInstanced(nRectsDrawn * 6, 1, firstVertex, 0);
	}

	//____ _drawEdgemap() ______________________________________________________
	//
	// An edgemap is drawn as one quad per patch, with the work done in the pixel
	// shader: it walks down a column of edges and works out how much of the pixel
	// each segment covers. The vertices carry where in the edgemap each corner
	// sits, which is what lets the same quad be flipped or rotated.

	void DX12Backend::_drawEdgemap(const uint16_t*& pCmd, const RectSPX*& pRects, Object* const*& pObjects, int nRects)
	{
		Object * pObject = *pObjects++;

		int32_t flip = *pCmd++;
		pCmd++;								// padding

		auto p32 = (const spx*) pCmd;

		spx destX = *p32++;
		spx destY = *p32++;

		pCmd = (const uint16_t*) p32;

		DX12Edgemap * pEdgemap = (pObject && _isOfType(pObject->typeInfo(), DX12Edgemap::TYPEINFO)) ?
								 static_cast<DX12Edgemap*>(pObject) : nullptr;

		if (!pEdgemap || pEdgemap->_gpuAddress() == 0 || !m_pVertexPtr || flip < 0 || flip >= GfxFlip_size)
		{
			static bool bReported = false;
			if (!bReported)
			{
				const char * pReason = !pEdgemap ? "it is not a DX12Edgemap" :
									   pEdgemap->_gpuAddress() == 0 ? "it has no buffer" :
									   !m_pVertexPtr ? "there is no vertex buffer" : "the flip value is out of range";

				char buffer[256];
				sprintf_s(buffer, "Can't draw edgemap, %s.", pReason);
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			pRects += nRects;
			return;
		}

		int nSegments = pEdgemap->m_nbRenderSegments;

		if (nSegments < 1 || nRects <= 0)
		{
			pRects += nRects;
			return;
		}

		auto& mtx = s_standardTransforms[flip];

		// Where the edgemap lands, before any patch is taken out of it.

		RectSPX destIn = {
			destX,
			destY,
			pEdgemap->m_size.w * 64 * int(std::abs(mtx.xx)) + pEdgemap->m_size.h * 64 * int(std::abs(mtx.yx)),
			pEdgemap->m_size.w * 64 * int(std::abs(mtx.xy)) + pEdgemap->m_size.h * 64 * int(std::abs(mtx.yy))
		};

		RectI dest = Util::roundToPixels(destIn);

		int uIncX = int(mtx.xx);
		int vIncX = int(mtx.xy);
		int uIncY = int(mtx.yx);
		int vIncY = int(mtx.yy);

		// We may have been given room for more columns than the edgemap has.

		int maxCol = pEdgemap->m_size.w;

		if (uIncX != 0)								// Columns run horizontally.
		{
			if (dest.w > maxCol)
			{
				if (uIncX < 0)
					dest.x += dest.w - maxCol;

				dest.w = maxCol;
			}
		}
		else										// Columns run vertically.
		{
			if (dest.h > maxCol)
			{
				if (uIncY < 0)
					dest.y += dest.h - maxCol;

				dest.h = maxCol;
			}
		}

		// Where the top left corner sits in the edgemap, once it has been flipped.

		int uTopLeft = 0;
		int vTopLeft = 0;

		if (uIncX + uIncY < 0)
			uTopLeft = maxCol;

		if (vIncX < 0)
			vTopLeft = dest.w;
		else if (vIncY < 0)
			vTopLeft = dest.h;

		// The colorstrips. A segment's color is its horizontal strip times its
		// vertical one, and the axis without a strip reads the edgemap's white.

		float colorstripPitchX;
		float colorstripPitchY;

		float colorstripBeginX, colorstripEndX;
		float colorstripBeginY, colorstripEndY;

		// _addColor() multiplies by the tint, so white is what gets the tint applied
		// exactly once. Passing the tint itself would square it.

		int colorOfs = _addColor(HiColor::White);
		int extrasOfs = _addExtras({ 0.f, 0.f, 0.f, 0.f });		// Filled in below, once we know the pitches.

		if (colorOfs < 0 || extrasOfs < 0)
		{
			pRects += nRects;
			return;
		}

		if (m_pVertexEnd - m_pVertexPtr < nRects * 6)
		{
			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Vertex buffer full, edgemaps dropped. SessionInfo underestimated what the session needs.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			pRects += nRects;
			return;
		}

		int firstVertex = int(m_pVertexPtr - m_pVertexBeg);

		for (int i = 0; i < nRects; i++)
		{
			const RectSPX& patchSpx = *pRects++;

			RectI patch;
			patch.x = patchSpx.x / 64;
			patch.y = patchSpx.y / 64;
			patch.w = patchSpx.w / 64;
			patch.h = patchSpx.h / 64;

			int dx1 = patch.x;
			int dy1 = patch.y;
			int dx2 = patch.x + patch.w;
			int dy2 = patch.y + patch.h;

			int ofsX = patch.x - dest.x;
			int ofsY = patch.y - dest.y;

			if (pEdgemap->m_pFlatColors)
			{
				colorstripBeginX = float(pEdgemap->_flatColorsOfs());
				colorstripEndX = colorstripBeginX;

				colorstripBeginY = float(pEdgemap->_whiteColorOfs());
				colorstripEndY = colorstripBeginY;

				colorstripPitchX = 1.f;
				colorstripPitchY = 0.f;
			}
			else
			{
				if (pEdgemap->m_pColorstripsX)
				{
					colorstripBeginX = float(pEdgemap->_colorstripXOfs() + ofsX);
					colorstripEndX = colorstripBeginX + patch.w;

					colorstripPitchX = float(pEdgemap->m_size.w);
				}
				else
				{
					colorstripBeginX = float(pEdgemap->_whiteColorOfs());
					colorstripEndX = colorstripBeginX;

					colorstripPitchX = 0.f;
				}

				if (pEdgemap->m_pColorstripsY)
				{
					colorstripBeginY = float(pEdgemap->_colorstripYOfs() + ofsY);
					colorstripEndY = colorstripBeginY + patch.h;

					colorstripPitchY = float(pEdgemap->m_size.h);
				}
				else
				{
					colorstripBeginY = float(pEdgemap->_whiteColorOfs());
					colorstripEndY = colorstripBeginY;

					colorstripPitchY = 0.f;
				}
			}

			// U is which column of the edgemap, V how far down that column. Half a
			// pixel back, so that the shader works from pixel centers.

			float u1 = float(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
			float v1 = float(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

			float u2 = float(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
			float v2 = float(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

			float u3 = float(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
			float v3 = float(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

			float u4 = float(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
			float v4 = float(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

			const float uv[4][2] = { { u1, v1 - 0.5f }, { u2, v2 - 0.5f }, { u3, v3 - 0.5f }, { u4, v4 - 0.5f } };

			// Which corner of the edgemap each corner of the patch reads from.

			const float colorstripIn[4][2] = {
				{ colorstripBeginX, colorstripBeginY },
				{ colorstripEndX,   colorstripBeginY },
				{ colorstripEndX,   colorstripEndY },
				{ colorstripBeginX, colorstripEndY } };

			const int* pOrder = s_flipCornerOrder[flip];

			const float coords[4][2] = { { float(dx1), float(dy1) }, { float(dx2), float(dy1) },
										 { float(dx2), float(dy2) }, { float(dx1), float(dy2) } };

			const int corners[6] = { 0, 1, 2, 0, 2, 3 };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				int corner = corners[vertex];

				m_pVertexPtr->x = coords[corner][0];
				m_pVertexPtr->y = coords[corner][1];
				m_pVertexPtr->colorOfs = (uint32_t) colorOfs;
				m_pVertexPtr->extrasOfs = (uint32_t) extrasOfs;
				m_pVertexPtr->u = uv[corner][0];
				m_pVertexPtr->v = uv[corner][1];
				m_pVertexPtr->colorstripX = colorstripIn[pOrder[corner]][0];
				m_pVertexPtr->colorstripY = colorstripIn[pOrder[corner]][1];

				m_pVertexPtr++;
			}
		}

		// Now that the pitches are known, fill in the entry we reserved.

		m_pExtrasBeg[extrasOfs].x = colorstripPitchX;
		m_pExtrasBeg[extrasOfs].y = colorstripPitchY;

		if (!_setPipeline(_pipeline(m_activeBlendMode, PipelineKind::Segments)) || !m_bCommandListOpen)
			return;

		// The edgemap's own buffer, and how to walk it. The number of edges we draw
		// and the number stored per column are not always the same, so the shader
		// is told both rather than assuming, as GlBackend and MetalBackend do.

		// The command list mentions the edgemap's buffer but keeps no reference to
		// it, and GfxDeviceGen2 lets go of the edgemap when the session ends. A
		// waveform drawn from a temporary would otherwise be freed while the GPU
		// still had work referring to it.

		m_frameResources[m_currentFrameIndex].objectRefs.push_back(pEdgemap);

		m_commandList->SetGraphicsRootShaderResourceView(5, pEdgemap->_gpuAddress());
		m_bBlitSourceBound = false;			// A palette based blit source keeps its palette here too.

		uint32_t edgeCounts[2] = { uint32_t(nSegments - 1), uint32_t(pEdgemap->m_nbSegments - 1) };
		m_commandList->SetGraphicsRoot32BitConstants(0, 2, edgeCounts, 6);

		m_commandList->DrawInstanced(nRects * 6, 1, firstVertex, 0);
	}

	//____ _setPipeline() ______________________________________________________

	bool DX12Backend::_setPipeline(ID3D12PipelineState* pPipeline)
	{
		if (!pPipeline || !m_bCommandListOpen)
			return false;

		if (pPipeline != m_pActivePipeline)
		{
			m_commandList->SetPipelineState(pPipeline);
			m_pActivePipeline = pPipeline;
		}

		return true;
	}

	//____ _normalizeBlendMode() _______________________________________________

	BlendMode DX12Backend::_normalizeBlendMode(BlendMode blendMode)
	{
		switch (blendMode)
		{
			case BlendMode::Undefined:
				return BlendMode::Blend;

			case BlendMode::BlendFixedColor:
				// Blending against a known background is an optimization for software
				// rendering, and an ordinary Blend is an allowed stand-in. Keeping them
				// as one mode also keeps one pipeline instead of two identical ones.

				return BlendMode::Blend;

			default:
				return blendMode;
		}
	}

	//____ _setBlendFactor() ___________________________________________________
	//
	// The constant BlendMode::Morph blends against. D3D12 keeps this on the command
	// list, not in the pipeline, so it survives a pipeline change but not a reset.

	void DX12Backend::_setBlendFactor()
	{
		if (!m_bCommandListOpen)
			return;

		const float blendFactor[4] = { m_morphFactor, m_morphFactor, m_morphFactor, m_morphFactor };

		m_commandList->OMSetBlendFactor(blendFactor);
	}

	//____ _pipeline() _________________________________________________________
	//
	// The pipeline for the given blend mode and draw kind, on the canvas we are
	// rendering into. Created the first time a combination shows up, since the
	// canvas formats in use are not known until canvases are set.

	ID3D12PipelineState* DX12Backend::_pipeline(BlendMode blendMode, PipelineKind kind)
	{
		if (blendMode == BlendMode::Ignore)
			return nullptr;							// Nothing should be drawn.

		if (m_activeCanvasFormat == DXGI_FORMAT_UNKNOWN)
			return nullptr;							// No canvas set.

		BlendMode mode = _normalizeBlendMode(blendMode);

		uint64_t key = (uint64_t(m_activeCanvasFormat) << 16) |
					   (uint64_t(kind) << 8) |
					   uint64_t(mode);

		auto it = m_pipelines.find(key);
		if (it != m_pipelines.end())
			return it->second.Get();				// May be null, if creation failed before.

		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipeline;

		if (!_createPipeline(mode, kind, m_activeCanvasFormat, pPipeline))
			pPipeline = nullptr;					// Remembered as a failure, so we don't try again every draw.

		m_pipelines[key] = pPipeline;

		return pPipeline.Get();
	}

	//____ setDefaultCanvas() ___________________________________________

	bool DX12Backend::setDefaultCanvas(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, ID3D12Resource* renderTargetBuffer,
									   DXGI_FORMAT renderTargetFormat, SizeSPX size, int scale)
	{
		m_defaultCanvasRTV = renderTargetView;
		m_defaultCanvasBuffer = renderTargetBuffer;
		m_defaultCanvasFormat = renderTargetFormat;
		m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;

		// WonderGUI has no name for R8G8B8A8, the usual swap chain format, so it is
		// reported as its BGRA counterpart. Byte order is the one thing about the
		// default canvas nobody can ask us, and the color space, which they can, is
		// then right.

		switch (renderTargetFormat)
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
				m_defaultCanvas.format = PixelFormat::BGRA_8_linear;
				break;

			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				m_defaultCanvas.format = PixelFormat::BGRA_8_sRGB;
				break;

			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				m_defaultCanvas.format = PixelFormat::BGRA_8_sRGB;
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM:
				m_defaultCanvas.format = PixelFormat::BGRX_8_linear;
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				m_defaultCanvas.format = PixelFormat::BGRX_8_sRGB;
				break;

			default:
				m_defaultCanvas.format = PixelFormat::Undefined;
				break;
		}

		return true;
	}

	//____ canvasInfo() ________________________________________________________

	const CanvasInfo* DX12Backend::canvasInfo(CanvasRef ref) const
	{
		if (ref == CanvasRef::Default)
			return &m_defaultCanvas;
		else
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only Default canvas is supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return &m_dummyCanvas;
		}
	}

	//____ surfaceFactory() ____________________________________________________

	SurfaceFactory_p DX12Backend::surfaceFactory()
	{
		return m_pSurfaceFactory;
	}

	//____ edgemapFactory() ____________________________________________________

	EdgemapFactory_p DX12Backend::edgemapFactory()
	{
		return m_pEdgemapFactory;
	}

	//____ maxEdges() ___________________________________________________________

	int DX12Backend::maxEdges() const
	{
		return c_maxSegments - 1;
	}

	//____ canBeBlitSource() __________________________________________________

	bool DX12Backend::canBeBlitSource(const TypeInfo& type) const
	{
		return _isDX12SurfaceType(type);
	}

	//____ canBeCanvas() ________________________________________________________

	bool DX12Backend::canBeCanvas(const TypeInfo& type) const
	{
		return _isDX12SurfaceType(type);
	}

	//____ waitForCompletion() __________________________________________________

	void DX12Backend::waitForCompletion()
	{
		// Waiting for work that is still being recorded would return at once and
		// mean nothing, so what we have goes to the GPU first.

		_flushCommandList();

		// Wait for the last value signaled, not just the current frame slot's,
		// so work from both in-flight frames is done.

		_waitForFence(m_fenceValue);
	}

	//____ _waitForFence() _____________________________________________________

	void DX12Backend::_waitForFence(UINT64 fenceValue)
	{
		if (m_commandFence && m_commandFence->GetCompletedValue() < fenceValue)
		{
			m_commandFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	//____ _createBuffer() _________________________________________________

	void DX12Backend::_createBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& pointer, int nbBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState, LPCWSTR name)
	{
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type = heapType;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = nbBytes;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1,0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


		if (!CHECK_HR(m_pDX12Device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, 0, IID_PPV_ARGS(pointer.GetAddressOf())), "CreateCommittedResource"))
			return;

		pointer->SetName(name);
	}


	//____ _reserveBuffers() __________________________________________________
	//
	// Makes sure what is left of this frame's vertex, color and extras buffers
	// can hold everything the session may write, going by its SessionInfo. The
	// counts are upper bounds: we can't tell ahead how many fills are subpixel
	// or which rects are blits, so we assume the worst.
	//
	// Per session it can use at most:
	//
	//   Vertices: six per rect of any kind, six per line.
	//   Colors:   those in the color stream - fills, lines and tintmaps copy
	//             theirs in - plus a white one per blit, blur and edgemap draw.
	//   Extras:   two per rect (blits need two, subpixel fills and patches
	//             fewer), one per line, 18 per blur for its brush and one per
	//             edgemap draw.

	void DX12Backend::_reserveBuffers(const SessionInfo* pInfo)
	{
		int nVertices, nColors, nExtras;

		if (pInfo)
		{
			int nLines = pInfo->nLineCoords / 2;

			nVertices = pInfo->nRects * 6 + nLines * 6;
			nColors = pInfo->nColors + pInfo->nBlit + pInfo->nBlur + pInfo->nEdgemapDraws;
			nExtras = pInfo->nRects * 2 + nLines + pInfo->nBlur * 18 + pInfo->nEdgemapDraws;
		}
		else
		{
			nVertices = c_defaultVertices;
			nColors = c_defaultColors;
			nExtras = c_defaultExtras;
		}

		auto& frame = m_frameResources[m_currentFrameIndex];

		_reserveBuffer(frame.vertexBuffer, frame.pVertexBufferData, frame.vertexCapacity,
					   m_pVertexBeg, m_pVertexPtr, m_pVertexEnd, nVertices, L"WonderGUI Vertex Buffer");

		_reserveBuffer(frame.colorBuffer, frame.pColorBufferData, frame.colorCapacity,
					   m_pColorBeg, m_pColorPtr, m_pColorEnd, nColors, L"WonderGUI Color Buffer");

		_reserveBuffer(frame.extrasBuffer, frame.pExtrasBufferData, frame.extrasCapacity,
					   m_pExtrasBeg, m_pExtrasPtr, m_pExtrasEnd, nExtras, L"WonderGUI Extras Buffer");
	}

	//____ _reserveBuffer() ____________________________________________________
	//
	// If what is left of the buffer is too little, it is replaced by a bigger one
	// and writing starts over at its beginning. The old buffer may already be used
	// by draws recorded this frame, so it is kept in retiredBuffers until the
	// fence says the GPU is done with it. Offsets into it recorded so far stay
	// valid, since they go with the root arguments and vertex buffer view that
	// were set when they were recorded, and _bindSessionState() points the coming
	// ones at the new buffer.
	//
	// Buffers at least double when they grow, so a frame settles on a size after
	// a few frames rather than growing a little every session.

	template<typename T>
	bool DX12Backend::_reserveBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, T*& pData, int& capacity,
									 T*& pBeg, T*& pPtr, T*& pEnd, int needed, LPCWSTR name)
	{
		if (needed < 1)
			needed = 1;

		if (pPtr && pEnd - pPtr >= needed)
			return true;

		int newCapacity = capacity * 2;
		if (newCapacity < needed)
			newCapacity = needed;

		Microsoft::WRL::ComPtr<ID3D12Resource> newBuffer;
		T* pNewData = nullptr;

		_createBuffer(newBuffer, int(newCapacity * sizeof(T)), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, name);

		D3D12_RANGE readRange = { 0, 0 };		// We only write. Upload heaps can stay mapped for their entire lifetime.

		if (!newBuffer || !CHECK_HR(newBuffer->Map(0, &readRange, (void**)&pNewData), "ID3D12Resource::Map"))
			return false;						// Keep what we have. Draws that don't fit are dropped and reported.

		if (buffer)
			m_frameResources[m_currentFrameIndex].retiredBuffers.push_back(buffer);

		buffer = newBuffer;
		pData = pNewData;
		capacity = newCapacity;

		pBeg = pNewData;
		pPtr = pNewData;
		pEnd = pNewData + newCapacity;

		return true;
	}

	//____ _createPipelineResources() _________________________________________
	//
	// Everything the pipelines are built from. The pipelines themselves are
	// created on demand by _pipeline(), since they depend on the canvas format.

	bool DX12Backend::_createPipelineResources()
	{
		if (!_createRootSignature())
			return false;

		if (!_createSamplers())
			return false;

		// A line of a given thickness has to be drawn wider the steeper it runs, or
		// it comes out thinner than asked for. The table holds that widening for
		// slopes from 0 to 1, which is all we need since a steeper line is measured
		// against the other axis.

		for (int i = 0; i < 17; i++)
		{
			double b = i / 16.0;
			m_lineThicknessTable[i] = (float) Util::squareRoot(1.0 + b * b);
		}

		struct { PipelineKind kind; const char * pVS; const char * pPS; } shaders[] =
		{
			{ PipelineKind::Fill,	g_fillVS,	g_fillPS },
			{ PipelineKind::FillAA,	g_fillAAVS,	g_fillAAPS },
			{ PipelineKind::Blit,	g_blitVS,	g_blitPS },
			{ PipelineKind::Blur,	g_blitVS,	g_blurPS },		// Same geometry, so the same vertex shader.
			{ PipelineKind::PaletteBlit, g_blitVS, g_paletteBlitPS },
			{ PipelineKind::PaletteBlur, g_blitVS, g_paletteBlurPS },
			{ PipelineKind::Line,	g_lineVS,	g_linePS },
			{ PipelineKind::Segments, g_segmentsVS, g_segmentsPS },
		};

		// Compiled by the first backend, reused by the rest. Opening a window
		// shouldn't cost a round of D3DCompile each time.

		for (auto& shader : shaders)
		{
			if (!s_vertexShaderBlobs[int(shader.kind)] && !_compileVertexShader(s_vertexShaderBlobs[int(shader.kind)], shader.pVS))
				return false;

			if (!s_pixelShaderBlobs[int(shader.kind)] && !_compilePixelShader(s_pixelShaderBlobs[int(shader.kind)], shader.pPS))
				return false;
		}

		return true;
	}

	//____ _scaleThickness() ___________________________________________________

	float DX12Backend::_scaleThickness(float thickness, float slope)
	{
		slope = std::abs(slope);

		float scale = m_lineThicknessTable[(int)(slope * 16)];

		if (slope < 1.f)
		{
			float scale2 = m_lineThicknessTable[(int)(slope * 16) + 1];
			scale += (scale2 - scale) * ((slope * 16) - ((int)(slope * 16)));
		}

		return thickness * scale;
	}

	//____ _createRootSignature() _____________________________________________

	bool DX12Backend::_createRootSignature()
	{
		// One root signature shared by all our pipelines. Switching pipelines is
		// cheaper when they agree on the signature, and the differences between
		// fill and blit are small enough that there is no reason to have two.
		//
		// Root parameter 0 holds the handful of values that change with canvas and
		// blit source: canvas scale for the vertex shader, and source size, flags
		// and the blur brush's place in the extras buffer for the pixel shader.
		// Root constants instead of a constant buffer since there are so few of
		// them and they change often.
		//
		// The color (t0) and extras (t1) buffers are root SRVs, which only need the
		// buffer's address. Extras are read by the vertex shaders and, for a blur,
		// by the pixel shader. The blit source texture (t2) and its sampler have to
		// go through descriptor tables, there is no root descriptor for those.

		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 1;
		srvRange.BaseShaderRegister = 2;						// t2, the blit source.
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE samplerRange = {};
		samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		samplerRange.NumDescriptors = 1;
		samplerRange.BaseShaderRegister = 0;					// s0.
		samplerRange.RegisterSpace = 0;
		samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER rootParameter[7] = {};

		rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[0].Constants.ShaderRegister = 0;			// b0.
		rootParameter[0].Constants.RegisterSpace = 0;
		// Eight is also what HLSL rounds the constant buffer up to, whole 16 byte
		// registers, and the root signature has to cover all of it.

		rootParameter[0].Constants.Num32BitValues = 8;			// canvasScale, textureSize, flags, blurOfs, edgemapEdges, edgemapPitch.
		rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[1].Descriptor.ShaderRegister = 0;			// t0, colors.
		rootParameter[1].Descriptor.RegisterSpace = 0;
		rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// Pixel shaders read tintmaps from it.

		rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[2].Descriptor.ShaderRegister = 1;			// t1, extras.
		rootParameter[2].Descriptor.RegisterSpace = 0;
		rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// The blur shader reads it too.

		rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
		rootParameter[3].DescriptorTable.pDescriptorRanges = &srvRange;
		rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter[4].DescriptorTable.NumDescriptorRanges = 1;
		rootParameter[4].DescriptorTable.pDescriptorRanges = &samplerRange;
		rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[5].Descriptor.ShaderRegister = 3;			// t3, the edgemap being drawn, or the palette of a palette based blit source.
		rootParameter[5].Descriptor.RegisterSpace = 0;
		rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// The tintmap, where its colors are in the color buffer and which pixels
		// they belong to. Only the pixel shaders use it. See TintmapInfo.

		rootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[6].Constants.ShaderRegister = 1;			// b1.
		rootParameter[6].Constants.RegisterSpace = 0;
		rootParameter[6].Constants.Num32BitValues = sizeof(TintmapInfo) / 4;
		rootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc = { };
		rsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		rsDesc.Desc_1_0.pParameters = rootParameter;
		rsDesc.Desc_1_0.NumParameters = 7;
		rsDesc.Desc_1_0.NumStaticSamplers = 0;
		rsDesc.Desc_1_0.pStaticSamplers = 0;
		rsDesc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		Microsoft::WRL::ComPtr<ID3DBlob> pSerializedRS;
		Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;

		HRESULT hr = D3D12SerializeVersionedRootSignature(&rsDesc, pSerializedRS.GetAddressOf(), pErrorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			const char* pErrorMsg = pErrorBlob ? (const char*)pErrorBlob->GetBufferPointer() : "Unknown error";
			char buffer[512];
			sprintf_s(buffer, "D3D12SerializeVersionedRootSignature failed, HRESULT = 0x%08lX: %s", (unsigned long)hr, pErrorMsg);
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			assert(false);
			return false;
		}

		if (!CHECK_HR(m_pDX12Device->CreateRootSignature(0, pSerializedRS->GetBufferPointer(), pSerializedRS->GetBufferSize(), IID_PPV_ARGS(m_pRootSignature.GetAddressOf())), "CreateRootSignature"))
			return false;

		return true;
	}

	//____ _createSamplers() __________________________________________________

	bool DX12Backend::_createSamplers()
	{
		// Four samplers, covering the combinations a surface can ask for. The
		// index is (tiling ? 2 : 0) + (bilinear ? 1 : 0), see _setBlitSource().

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.NumDescriptors = 4;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (!CHECK_HR(m_pDX12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pSamplerHeap.GetAddressOf())), "CreateDescriptorHeap"))
			return false;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();

		for (int i = 0; i < 4; i++)
		{
			bool bTiling = (i & 2) != 0;
			bool bBilinear = (i & 1) != 0;

			D3D12_TEXTURE_ADDRESS_MODE addressMode = bTiling ? D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

			D3D12_SAMPLER_DESC desc = {};
			desc.Filter = bBilinear ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = addressMode;
			desc.AddressV = addressMode;
			desc.AddressW = addressMode;
			desc.MipLODBias = 0.f;
			desc.MaxAnisotropy = 1;
			desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			desc.MinLOD = 0.f;
			desc.MaxLOD = D3D12_FLOAT32_MAX;

			m_pDX12Device->CreateSampler(&desc, handle);

			handle.ptr += m_samplerDescriptorSize;
		}

		return true;
	}

	//____ _createPipeline() __________________________________________________

	bool DX12Backend::_createPipeline(BlendMode blendMode, PipelineKind kind, DXGI_FORMAT rtvFormat, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline)
	{
		// Setup the graphics pipeline state.

		auto& vertexShaderBlob = s_vertexShaderBlobs[int(kind)];
		auto& pixelShaderBlob = s_pixelShaderBlobs[int(kind)];

		if (!vertexShaderBlob || !pixelShaderBlob)
			return false;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_pRootSignature.Get();
		desc.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
		desc.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
		desc.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
		desc.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

		// Blend state, following MetalBackend's mapping. An alpha only canvas keeps
		// its value in the alpha channel here, where Metal keeps it in red, so what
		// Metal does to the color channels we do to alpha.
		//
		// A BGRX canvas has no alpha channel to write to, so the mask below just
		// says so. Blits from it read alpha as 1.0 regardless, which is the point
		// of giving it an X format in the first place.

		bool bAlphaOnly = (rtvFormat == DXGI_FORMAT_A8_UNORM);
		bool bNoAlpha = (rtvFormat == DXGI_FORMAT_B8G8R8X8_UNORM || rtvFormat == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB);

		desc.BlendState.AlphaToCoverageEnable = false;
		desc.BlendState.IndependentBlendEnable = false;

		// We only ever have one render target, but zero is not a valid blend value
		// and the rest of the array would keep the zeroes from desc = {}.

		for (int i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			auto& unused = desc.BlendState.RenderTarget[i];

			unused.SrcBlend = D3D12_BLEND_ONE;
			unused.DestBlend = D3D12_BLEND_ZERO;
			unused.BlendOp = D3D12_BLEND_OP_ADD;
			unused.SrcBlendAlpha = D3D12_BLEND_ONE;
			unused.DestBlendAlpha = D3D12_BLEND_ZERO;
			unused.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			unused.LogicOp = D3D12_LOGIC_OP_NOOP;
		}

		auto& rt = desc.BlendState.RenderTarget[0];

		rt.BlendEnable = (blendMode != BlendMode::Replace);
		rt.LogicOpEnable = false;
		rt.LogicOp = D3D12_LOGIC_OP_NOOP;
		rt.RenderTargetWriteMask = bNoAlpha ? (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE)
											: D3D12_COLOR_WRITE_ENABLE_ALL;

		// Zero is not a valid value for any of these, so they are filled in even
		// where blending is off and they are never used.

		rt.SrcBlend = D3D12_BLEND_ONE;
		rt.DestBlend = D3D12_BLEND_ZERO;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_ZERO;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;

		switch (blendMode)
		{
			case BlendMode::Replace:
				break;								// Blending is off, the defaults above are never used.

			case BlendMode::Blend:
				rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				rt.SrcBlendAlpha = D3D12_BLEND_ONE;
				rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
				break;

			case BlendMode::Add:
				rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				rt.DestBlend = D3D12_BLEND_ONE;

				if (bAlphaOnly)
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ONE;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
				}
				else
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ZERO;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
				}
				break;

			case BlendMode::Subtract:
				rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				rt.DestBlend = D3D12_BLEND_ONE;
				rt.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;

				if (bAlphaOnly)
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ONE;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
					rt.BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
				}
				else
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ZERO;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
				}
				break;

			case BlendMode::Multiply:
				rt.SrcBlend = D3D12_BLEND_DEST_COLOR;
				rt.DestBlend = D3D12_BLEND_ZERO;

				if (bAlphaOnly)
				{
					rt.SrcBlendAlpha = D3D12_BLEND_DEST_ALPHA;
					rt.DestBlendAlpha = D3D12_BLEND_ZERO;
				}
				else
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ZERO;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
				}
				break;

			case BlendMode::Invert:
				rt.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
				rt.DestBlend = D3D12_BLEND_INV_SRC_COLOR;

				if (bAlphaOnly)
				{
					rt.SrcBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
					rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
				}
				else
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ZERO;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
				}
				break;

			case BlendMode::Min:
			case BlendMode::Max:
			{
				D3D12_BLEND_OP op = (blendMode == BlendMode::Min) ? D3D12_BLEND_OP_MIN : D3D12_BLEND_OP_MAX;

				rt.SrcBlend = D3D12_BLEND_ONE;
				rt.DestBlend = D3D12_BLEND_ONE;
				rt.BlendOp = op;

				if (bAlphaOnly)
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ONE;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
					rt.BlendOpAlpha = op;
				}
				else
				{
					rt.SrcBlendAlpha = D3D12_BLEND_ZERO;
					rt.DestBlendAlpha = D3D12_BLEND_ONE;
				}
				break;
			}

			case BlendMode::Morph:
				// Metal uses the blend color's alpha for every channel. We set all
				// four components of the blend factor to the morph factor, which
				// comes to the same thing, see _setBlendFactor().

				rt.SrcBlend = D3D12_BLEND_BLEND_FACTOR;
				rt.DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
				rt.SrcBlendAlpha = D3D12_BLEND_BLEND_FACTOR;
				rt.DestBlendAlpha = D3D12_BLEND_INV_BLEND_FACTOR;
				break;

			case BlendMode::Ignore:
				// Nothing should be drawn. _pipeline() returns null for this, so we
				// only get here if that ever changes.

				rt.SrcBlend = D3D12_BLEND_ZERO;
				rt.DestBlend = D3D12_BLEND_ONE;
				rt.SrcBlendAlpha = D3D12_BLEND_ZERO;
				rt.DestBlendAlpha = D3D12_BLEND_ONE;
				break;

			default:
			{
				char buffer[128];
				sprintf_s(buffer, "BlendMode %d is unknown, using Blend.", (int) blendMode);
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::Other, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

				rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				rt.SrcBlendAlpha = D3D12_BLEND_ONE;
				rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
				break;
			}
		}

		desc.SampleMask = 0xFFFFFFFF;
		desc.SampleDesc = { 1,0 };

		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //might do none
		desc.RasterizerState.FrontCounterClockwise = false;
		desc.RasterizerState.DepthClipEnable = true;
		desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		desc.DepthStencilState.DepthEnable = false;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		// One vertex layout for all of them. The plain fill shader ignores
		// EXTRASOFS, which is fine - the input layout is allowed to offer more
		// than a shader reads.

		D3D12_INPUT_ELEMENT_DESC elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
			{ "COLOROFS", 0, DXGI_FORMAT_R32_UINT, 0, 8,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "EXTRASOFS", 0, DXGI_FORMAT_R32_UINT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLORSTRIP", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_INPUT_LAYOUT_DESC inputLayout = {};

		inputLayout.NumElements = 5;
		inputLayout.pInputElementDescs = elements;


		desc.InputLayout = inputLayout;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;


		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = rtvFormat;
		desc.NodeMask = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		if (!CHECK_HR(m_pDX12Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pPipeline.GetAddressOf())), "CreateGraphicsPipelineState"))
			return false;

		return true;
	}

	//____ _compileVertexShader() _____________________________________________

	bool DX12Backend::_compileVertexShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc)
	{ 
		UINT compileFlags = c_shaderCompileFlags;

		Microsoft::WRL::ComPtr<ID3DBlob> errorMsg;

		HRESULT hr = D3DCompile(pSrc, strlen((const char*)pSrc), nullptr, nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, shaderBlob.GetAddressOf(), errorMsg.GetAddressOf());
		if (FAILED(hr))
		{
			const char* pError = errorMsg ? (const char*)errorMsg->GetBufferPointer() : "Unknown error";
			char buffer[1024];
			sprintf_s(buffer, "Vertex shader compile failed, HRESULT = 0x%08lX: %s", (unsigned long)hr, pError);
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			assert(false);
			return false;
		}

		return true;
	}

	//____ _compilePixelShader() _______________________________________________

	bool DX12Backend::_compilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc)
	{
		UINT compileFlags = c_shaderCompileFlags;

		Microsoft::WRL::ComPtr<ID3DBlob> errorMsg;

		HRESULT hr = D3DCompile(pSrc, strlen((const char*)pSrc), nullptr, nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, shaderBlob.GetAddressOf(), errorMsg.GetAddressOf());
		if (FAILED(hr))
		{
			const char* pError = errorMsg ? (const char*)errorMsg->GetBufferPointer() : "Unknown error";

			char buffer[1024];
			sprintf_s(buffer, "Pixel shader compile failed, HRESULT = 0x%08lX: %s", (unsigned long)hr, pError);
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			assert(false);
			return false;
		}

		return true;

	}



}