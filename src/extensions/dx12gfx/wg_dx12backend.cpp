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

#include <wg_dx12backend.h>
#include <wg_dx12surface.h>
#include <wg_dx12surfacefactory.h>
#include <wg_dx12edgemapfactory.h>
#include <d3dcompiler.h>

#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>

#include <wg_gfxbase.h>



namespace wg
{

	const TypeInfo DX12Backend::TYPEINFO = { "DX12Backend", &GfxBackend::TYPEINFO };

	// Debug aid: clear each update rect before drawing it, so it is obvious which
	// parts of the canvas are redrawn and whether anything is left unpainted. The
	// window must present full frames while this is on, see beginSession().

	static constexpr bool c_bDebugClearUpdateRects = false;

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

	//____ create() ______________________________________________________________

	DX12Backend_p DX12Backend::create(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		return DX12Backend_p(new DX12Backend(pDX12Device, pDX12CommandQueue));
	}

	//____ Constructor ___________________________________________________________

	DX12Backend::DX12Backend(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		m_pSurfaceFactory = DX12SurfaceFactory::create();
		m_pEdgemapFactory = DX12EdgemapFactory::create();

		m_pDX12CommandQueue = pDX12CommandQueue;
		m_pDX12Device = pDX12Device;

		// Surfaces create their own textures and need the device for it.

		DX12Surface::setDevice(pDX12Device, this);

		m_srvDescriptorSize = pDX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_samplerDescriptorSize = pDX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		pDX12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_commandFence));
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		// Create command allocator and vertex buffer for each frame resource.
		//
		// The vertex buffers can't be shared between frame resources since the GPU
		// may still be reading one of them. beginRender() waits for the fence of
		// the frame resource it picks, so one buffer each is enough.

		for (int i = 0; i < c_nbFrameResources; ++i)
		{
			pDX12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[i].commandAllocator));
			m_frameResources[i].fenceValue = 0;

			_createBuffer(m_frameResources[i].vertexBuffer, c_vertexBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, L"WonderGUI Vertex Buffer");
			_createBuffer(m_frameResources[i].colorBuffer, c_colorBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, L"WonderGUI Color Buffer");
			_createBuffer(m_frameResources[i].extrasBuffer, c_extrasBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, L"WonderGUI Extras Buffer");

			// Upload heaps can stay mapped for their entire lifetime.

			D3D12_RANGE readRange = { 0, 0 };		// We only write.

			if (m_frameResources[i].vertexBuffer)
				CHECK_HR(m_frameResources[i].vertexBuffer->Map(0, &readRange, (void**)&m_frameResources[i].pVertexBufferData), "ID3D12Resource::Map");

			if (m_frameResources[i].colorBuffer)
				CHECK_HR(m_frameResources[i].colorBuffer->Map(0, &readRange, (void**)&m_frameResources[i].pColorBufferData), "ID3D12Resource::Map");

			if (m_frameResources[i].extrasBuffer)
				CHECK_HR(m_frameResources[i].extrasBuffer->Map(0, &readRange, (void**)&m_frameResources[i].pExtrasBufferData), "ID3D12Resource::Map");

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

		DX12Surface::exitDevice();

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

		// Vertices and colors are written for the whole frame, not per session,
		// since the GPU doesn't run any of it until the command list has been
		// executed.

		m_pVertexBeg = m_frameResources[frame].pVertexBufferData;
		m_pVertexEnd = m_pVertexBeg ? m_pVertexBeg + c_vertexBufferSize / sizeof(Vertex) : nullptr;
		m_pVertexPtr = m_pVertexBeg;

		m_pColorBeg = m_frameResources[frame].pColorBufferData;
		m_pColorEnd = m_pColorBeg ? m_pColorBeg + c_colorBufferSize / sizeof(ColorDX12) : nullptr;
		m_pColorPtr = m_pColorBeg;

		m_pExtrasBeg = m_frameResources[frame].pExtrasBufferData;
		m_pExtrasEnd = m_pExtrasBeg ? m_pExtrasBeg + c_extrasBufferSize / sizeof(ExtrasDX12) : nullptr;
		m_pExtrasPtr = m_pExtrasBeg;

		m_frameResources[frame].nSRVDescriptors = 0;

		// The fence says the GPU is done with the command list that mentioned
		// these, so we can let go of them.

		m_frameResources[frame].surfaceRefs.clear();

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
			vertexBufferView.SizeInBytes = c_vertexBufferSize;

			m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView );
		}

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

			frame.surfaceRefs.push_back(pSurface);
		}

		m_blitSourceCanvases.clear();
	}

	//____ _isDX12Surface() ____________________________________________________
	//
	// TypeInfo compares by address, so a plain == would turn away anything derived
	// from DX12Surface. Walk the chain instead.

	bool DX12Backend::_isDX12SurfaceType(const TypeInfo& type)
	{
		for (const TypeInfo* pType = &type; pType; pType = pType->pSuperClass)
		{
			if (pType == &DX12Surface::TYPEINFO)
				return true;
		}

		return false;
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

		_bindSessionState();

		// Sessions start with default state, changes arrive as StateChange commands.

		m_tintColor = HiColor::White;
		m_activeBlendMode = BlendMode::Blend;
		_setBlitSource(nullptr);

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
		if (pCanvas && (!pCanvas->texture() || !pCanvas->canBeCanvas()))
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

					if (statesChanged & uint8_t(StateChange::TintColor))
						m_tintColor = *pColors++;

					if (statesChanged & uint8_t(StateChange::TintMap))
					{
						auto p32 = (const spx*) p;

						p32 += 4;								// Tintmap rectangle.

						int32_t	nHorrColors = *p32++;
						int32_t	nVertColors = *p32++;

						p = (const uint16_t*) p32;

						pColors += nHorrColors + nVertColors;	// Tintmaps not supported yet.
						m_tintColor = HiColor::White;
					}

					if (statesChanged & uint8_t(StateChange::BlendMode))
						m_activeBlendMode = BlendMode(*p++);

					if (statesChanged & uint8_t(StateChange::MorphFactor))
						p++;									// Only used by BlendMode::Morph.

					if (statesChanged & uint8_t(StateChange::FixedBlendColor))
						pColors++;								// Only used by BlendMode::BlendFixedColor.

					if (statesChanged & uint8_t(StateChange::Blur))
						p += 28;								// Blur not supported yet.

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
				{
					// All three are the same draw for us. Whether the source is clamped
					// or tiled is a property of the surface, and the rects have already
					// been clipped by GfxDeviceGen2.

					int32_t nRects = *p++;

					_drawBlitRects(p, pRects, nRects, version);
					break;
				}

				default:
				{
					// We don't know the size of the payload of a command we don't
					// handle, so there is no way to find the next one. Text and
					// images are blits, so expect them to be missing until those
					// are implemented.

					static bool bReported = false;
					if (!bReported)
					{
						char buffer[128];
						sprintf_s(buffer, "Command %d not implemented, rest of session dropped.", (int)cmd);
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

	void DX12Backend::_drawFillRects(const RectSPX* pRects, int nRects, HiColor color)
	{
		if (nRects <= 0 || !m_pVertexPtr)
			return;

		if (!_setPipeline(_pipeline(m_activeBlendMode, false)))
			return;

		int nVerticesLeft = int(m_pVertexEnd - m_pVertexPtr);

		if (nRects * 6 > nVerticesLeft)
		{
			nRects = nVerticesLeft / 6;

			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Vertex buffer full, fills dropped. Increase c_vertexBufferSize.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				bReported = true;
			}

			if (nRects == 0)
				return;
		}

		// One color for the whole command, the vertices only carry its offset.

		int colorOfs = _addColor(color);
		if (colorOfs < 0)
			return;

		int firstVertex = int(m_pVertexPtr - m_pVertexBeg);

		for (int i = 0; i < nRects; i++)
		{
			const RectSPX& rect = pRects[i];

			// spx to pixels, rounded to the nearest pixel edge. Rects that are
			// already pixel aligned (the common case) are unaffected.
			//
			// TODO: subpixel precision needs coverage calculated in the pixel
			// shader, the way GlBackend does it.

			float x1 = float((rect.x + 32) >> 6);
			float y1 = float((rect.y + 32) >> 6);
			float x2 = float((rect.x + rect.w + 32) >> 6);
			float y2 = float((rect.y + rect.h + 32) >> 6);

			const float coords[6][2] = { {x1,y1}, {x2,y1}, {x2,y2},
										 {x1,y1}, {x2,y2}, {x1,y2} };

			for (int vertex = 0; vertex < 6; vertex++)
			{
				m_pVertexPtr->x = coords[vertex][0];
				m_pVertexPtr->y = coords[vertex][1];
				m_pVertexPtr->colorOfs = (uint32_t) colorOfs;
				m_pVertexPtr->extrasOfs = 0;

				m_pVertexPtr++;
			}
		}

		m_commandList->DrawInstanced(nRects * 6, 1, firstVertex, 0);
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
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Color buffer full, draws dropped. Increase c_colorBufferSize.",
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

	//____ _addExtras() ________________________________________________________

	int DX12Backend::_addExtras(const ExtrasDX12& first, const ExtrasDX12& second)
	{
		if (!m_pExtrasPtr)
			return -1;

		if (m_pExtrasEnd - m_pExtrasPtr < 2)
		{
			static bool bReported = false;
			if (!bReported)
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Extras buffer full, blits dropped. Increase c_extrasBufferSize.",
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

		uint32_t flags = m_bBlitSourceAlphaOnly ? 1 : 0;
		m_commandList->SetGraphicsRoot32BitConstants(0, 1, &flags, 4);

		m_bBlitSourceBound = true;
		return true;
	}

	//____ _drawBlitRects() ____________________________________________________

	void DX12Backend::_drawBlitRects(const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, int version)
	{
		// Transforms below this index are the standard ones, the rest were handed
		// to us through setTransforms().

		int customTransformStart = (version == 1) ? GfxFlip_size : NbStandardTransforms;

		bool bBilinear = (m_pBlitSource && m_pBlitSource->sampleMethod() == SampleMethod::Bilinear);

		// Even when we can't draw we have to step through the command's data, or
		// we lose track of where we are in the stream.

		bool bDraw = _bindBlitSource() && _setPipeline(_pipeline(m_activeBlendMode, true)) && m_pVertexPtr != nullptr;

		int colorOfs = bDraw ? _addColor(m_tintColor) : -1;

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
					GfxBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Vertex buffer full, blits dropped. Increase c_vertexBufferSize.",
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

	//____ _supportedBlendMode() _______________________________________________

	BlendMode DX12Backend::_supportedBlendMode(BlendMode blendMode)
	{
		switch (blendMode)
		{
			case BlendMode::Replace:
				return BlendMode::Replace;

			case BlendMode::Undefined:
			case BlendMode::Blend:
			case BlendMode::BlendFixedColor:		// Defaults to Blend, like GlBackend does.
				return BlendMode::Blend;

			default:
			{
				static bool bReported = false;
				if (!bReported)
				{
					char buffer[128];
					sprintf_s(buffer, "BlendMode %d not supported, using Blend.", (int)blendMode);
					GfxBase::throwError(ErrorLevel::Warning, ErrorCode::Other, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					bReported = true;
				}

				return BlendMode::Blend;
			}
		}
	}

	//____ _pipeline() _________________________________________________________
	//
	// The pipeline for the given blend mode and draw kind, on the canvas we are
	// rendering into. Created the first time a combination shows up, since the
	// canvas formats in use are not known until canvases are set.

	ID3D12PipelineState* DX12Backend::_pipeline(BlendMode blendMode, bool bBlit)
	{
		if (blendMode == BlendMode::Ignore)
			return nullptr;							// Nothing should be drawn.

		if (m_activeCanvasFormat == DXGI_FORMAT_UNKNOWN)
			return nullptr;							// No canvas set.

		BlendMode mode = _supportedBlendMode(blendMode);

		uint64_t key = (uint64_t(m_activeCanvasFormat) << 8) |
					   (uint64_t(mode == BlendMode::Replace ? 1 : 0) << 1) |
					   (bBlit ? 1 : 0);

		auto it = m_pipelines.find(key);
		if (it != m_pipelines.end())
			return it->second.Get();				// May be null, if creation failed before.

		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipeline;

		if (!_createPipeline(mode, bBlit, m_activeCanvasFormat, pPipeline))
			pPipeline = nullptr;					// Remembered as a failure, so we don't try again every draw.

		m_pipelines[key] = pPipeline;

		return pPipeline.Get();
	}

	//____ setDefaultCanvas() ___________________________________________

	bool DX12Backend::setDefaultCanvas(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, ID3D12Resource* renderTargetBuffer, SizeSPX size, int scale)
	{
		m_defaultCanvasRTV = renderTargetView;
		m_defaultCanvasBuffer = renderTargetBuffer;
		m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;

		// The window decides the swap chain's format, and our pipelines have to
		// match it, so take it from the buffer rather than assuming.

		if (renderTargetBuffer)
		{
			D3D12_RESOURCE_DESC desc = renderTargetBuffer->GetDesc();
			m_defaultCanvasFormat = desc.Format;

			// WonderGUI has no name for R8G8B8A8, which is the usual swap chain
			// format, so that one is left Undefined.

			switch (m_defaultCanvasFormat)
			{
				case DXGI_FORMAT_B8G8R8A8_UNORM:
					m_defaultCanvas.format = PixelFormat::BGRA_8;
					break;

				case DXGI_FORMAT_B8G8R8X8_UNORM:
					m_defaultCanvas.format = PixelFormat::BGRX_8;
					break;

				default:
					m_defaultCanvas.format = PixelFormat::Undefined;
					break;
			}
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
		return 0;
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

		if (!_compileVertexShader(m_fillVertexShaderBlob, g_fillVS))
			return false;

		if (!_compilePixelShader(m_fillPixelShaderBlob, g_fillPS))
			return false;

		if (!_compileVertexShader(m_blitVertexShaderBlob, g_blitVS))
			return false;

		if (!_compilePixelShader(m_blitPixelShaderBlob, g_blitPS))
			return false;

		return true;
	}

	//____ _createRootSignature() _____________________________________________

	bool DX12Backend::_createRootSignature()
	{
		// One root signature shared by all our pipelines. Switching pipelines is
		// cheaper when they agree on the signature, and the differences between
		// fill and blit are small enough that there is no reason to have two.
		//
		// Root parameter 0 holds the handful of values that change with canvas and
		// blit source: canvas scale for the vertex shader, source size and flags
		// for the pixel shader. Root constants instead of a constant buffer since
		// there are so few of them and they change often.
		//
		// The color (t0) and extras (t1) buffers are root SRVs, which only need
		// the buffer's address. The blit source texture (t2) and its sampler have
		// to go through descriptor tables, there is no root descriptor for those.

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

		D3D12_ROOT_PARAMETER rootParameter[5] = {};

		rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[0].Constants.ShaderRegister = 0;			// b0.
		rootParameter[0].Constants.RegisterSpace = 0;
		// Eight rather than the five we use: HLSL rounds a constant buffer up to
		// whole 16 byte registers, and the root signature has to cover all of it.

		rootParameter[0].Constants.Num32BitValues = 8;			// canvasScale, textureSize, flags + padding.
		rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[1].Descriptor.ShaderRegister = 0;			// t0, colors.
		rootParameter[1].Descriptor.RegisterSpace = 0;
		rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[2].Descriptor.ShaderRegister = 1;			// t1, extras.
		rootParameter[2].Descriptor.RegisterSpace = 0;
		rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
		rootParameter[3].DescriptorTable.pDescriptorRanges = &srvRange;
		rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter[4].DescriptorTable.NumDescriptorRanges = 1;
		rootParameter[4].DescriptorTable.pDescriptorRanges = &samplerRange;
		rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc = { };
		rsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		rsDesc.Desc_1_0.pParameters = rootParameter;
		rsDesc.Desc_1_0.NumParameters = 5;
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

	bool DX12Backend::_createPipeline(BlendMode blendMode, bool bBlit, DXGI_FORMAT rtvFormat, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline)
	{
		// Setup the graphics pipeline state.

		auto& vertexShaderBlob = bBlit ? m_blitVertexShaderBlob : m_fillVertexShaderBlob;
		auto& pixelShaderBlob = bBlit ? m_blitPixelShaderBlob : m_fillPixelShaderBlob;

		if (!vertexShaderBlob || !pixelShaderBlob)
			return false;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_pRootSignature.Get();
		desc.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
		desc.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
		desc.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
		desc.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

		desc.BlendState.AlphaToCoverageEnable = false;
		desc.BlendState.IndependentBlendEnable = false;
		desc.BlendState.RenderTarget[0].BlendEnable = (blendMode == BlendMode::Blend);
		desc.BlendState.RenderTarget[0].LogicOpEnable = false;
		desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// Zero is not a valid value for any of these, so they are filled in even
		// for Replace, where blending is off and they are never used.

		desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;

		if (blendMode == BlendMode::Blend)
		{
			// Same as GlBackend uses for a canvas that isn't alpha only. An
			// alpha only canvas keeps only the alpha channel, and those factors
			// already give it the ordinary over operator.

			desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
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

		// One vertex layout for both. The fill shaders ignore EXTRASOFS, which is
		// fine, the input layout is allowed to offer more than a shader reads.

		D3D12_INPUT_ELEMENT_DESC elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
			{ "COLOROFS", 0, DXGI_FORMAT_R32_UINT, 0, 8,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "EXTRASOFS", 0, DXGI_FORMAT_R32_UINT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_INPUT_LAYOUT_DESC inputLayout = {};

		inputLayout.NumElements = 3;
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
		UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;

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

	//____ _compilePixeShader() _______________________________________________

	bool DX12Backend::_compilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc)
	{
		UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;

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