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
#include <wg_dx12surfacefactory.h>
#include <wg_dx12edgemapfactory.h>
#include <d3dcompiler.h>

#include <cstdio>

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
	// Logs failed HRESULTs to the debug output. The logging stays in release
	// builds, where the assert is compiled out.

	static bool _checkHR(HRESULT hr, const char* what)
	{
		if (FAILED(hr))
		{
			char msg[256];
			sprintf_s(msg, "DX12Backend: %s failed, HRESULT = 0x%08lX\n", what, (unsigned long)hr);
			OutputDebugStringA(msg);
			assert(false);
			return false;
		}
		return true;
	}

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

			if (m_frameResources[i].vertexBuffer)
			{
				// Upload heaps can stay mapped for their entire lifetime.

				D3D12_RANGE readRange = { 0, 0 };		// We only write.
				_checkHR(m_frameResources[i].vertexBuffer->Map(0, &readRange, (void**)&m_frameResources[i].pVertexBufferData), "ID3D12Resource::Map");
			}
		}

		// Create one command list for all frames (will be reset for each frame)

		pDX12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

		m_commandList->Close();

		if (!_createFillPipelines())
			OutputDebugStringA("DX12Backend: failed to create fill pipelines, nothing will render.\n");

	}

	//____ Destructor ____________________________________________________________

	DX12Backend::~DX12Backend()
	{
		// Our resources must not be released while the GPU still uses them.

		waitForCompletion();

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

		// Vertices are written for the whole frame, not per session, since the
		// GPU doesn't run any of it until the command list has been executed.

		m_pVertexBeg = m_frameResources[frame].pVertexBufferData;
		m_pVertexEnd = m_pVertexBeg ? m_pVertexBeg + c_vertexBufferSize / sizeof(Vertex) : nullptr;
		m_pVertexPtr = m_pVertexBeg;

		m_pActivePipeline = nullptr;		// Resetting the command list cleared its state.
	}

	//____ endRender() _________________________________________________________

	void DX12Backend::endRender()
	{
		// Close() must not be inside the assert, or it is never called in release builds.

		HRESULT hr = m_commandList->Close();
		if (!_checkHR(hr, "ID3D12GraphicsCommandList::Close"))
			return;

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };

		 m_pDX12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	 
		 // Signal and increment the fence value.

		m_fenceValue++;

		m_pDX12CommandQueue->Signal(m_commandFence.Get(), m_fenceValue);
		m_frameResources[m_currentFrameIndex].fenceValue = m_fenceValue;

	}

	//____ beginSession() _____________________________________________________

	void DX12Backend::beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo)
	{
		// Barrier

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_defaultCanvasBuffer;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &barrier);

		// Set render target

		m_commandList->OMSetRenderTargets(1, &m_defaultCanvasRTV, FALSE, nullptr);

		// Set viewport and scissor. Canvas size is in spx, D3D12 wants pixels.

		int canvasWidth = m_defaultCanvas.size.w / 64;
		int canvasHeight = m_defaultCanvas.size.h / 64;

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (FLOAT)canvasWidth;
		viewport.Height = (FLOAT)canvasHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_commandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = (LONG)canvasWidth;
		scissorRect.bottom = (LONG)canvasHeight;
		m_commandList->RSSetScissorRects(1, &scissorRect);

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

		if (c_bDebugClearUpdateRects && nUpdateRects > 0)
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
			m_commandList->ClearRenderTargetView(m_defaultCanvasRTV, clearColor, (UINT) rects.size(), rects.data());
		}

		// Set pipeline state. Vertex positions are in canvas pixels, the vertex
		// shader needs the canvas size to bring them into clip space.

		m_commandList->SetGraphicsRootSignature(m_pFillRootSignature.Get());

		float canvasScale[2] = { canvasWidth > 0 ? 2.f / canvasWidth : 0.f,
								 canvasHeight > 0 ? 2.f / canvasHeight : 0.f };

		m_commandList->SetGraphicsRoot32BitConstants(0, 2, canvasScale, 0);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (m_frameResources[m_currentFrameIndex].vertexBuffer)
		{
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
			vertexBufferView.BufferLocation = m_frameResources[m_currentFrameIndex].vertexBuffer->GetGPUVirtualAddress();
			vertexBufferView.StrideInBytes = sizeof(Vertex);
			vertexBufferView.SizeInBytes = c_vertexBufferSize;

			m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView );
		}

		// Sessions start with default state, changes arrive as StateChange commands.

		m_tintColor = HiColor::White;
		m_activeBlendMode = BlendMode::Blend;
	}

	//____ endSession() _______________________________________________________

	void DX12Backend::endSession()
	{
		// Barrier

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_defaultCanvasBuffer;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &barrier);
	}

	//____ setCanvas() ________________________________________________________

	void DX12Backend::setCanvas(Surface* pSurface)
	{
	}

	void DX12Backend::setCanvas(CanvasRef ref)
	{
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
						pObjects++;								// Blits not supported yet.

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

				default:
				{
					// We don't know the size of the payload of a command we don't
					// handle, so there is no way to find the next one. Text and
					// images are blits, so expect them to be missing until those
					// are implemented.

					static bool bReported = false;
					if (!bReported)
					{
						char msg[128];
						sprintf_s(msg, "DX12Backend: command %d not implemented, rest of session dropped.\n", (int)cmd);
						OutputDebugStringA(msg);
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

		auto pPipeline = _fillPipeline(m_activeBlendMode);
		if (!pPipeline)
			return;

		if (pPipeline != m_pActivePipeline)
		{
			m_commandList->SetPipelineState(pPipeline);
			m_pActivePipeline = pPipeline;
		}

		// HiColor components are 13 bit fixed point, tint is a plain multiplication.

		float r = (color.r / 4096.f) * (m_tintColor.r / 4096.f);
		float g = (color.g / 4096.f) * (m_tintColor.g / 4096.f);
		float b = (color.b / 4096.f) * (m_tintColor.b / 4096.f);
		float a = (color.a / 4096.f) * (m_tintColor.a / 4096.f);

		int nVerticesLeft = int(m_pVertexEnd - m_pVertexPtr);

		if (nRects * 6 > nVerticesLeft)
		{
			nRects = nVerticesLeft / 6;

			static bool bReported = false;
			if (!bReported)
			{
				OutputDebugStringA("DX12Backend: vertex buffer full, fills dropped. Increase c_vertexBufferSize.\n");
				bReported = true;
			}

			if (nRects == 0)
				return;
		}

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

				m_pVertexPtr->r = r;
				m_pVertexPtr->g = g;
				m_pVertexPtr->b = b;
				m_pVertexPtr->a = a;

				m_pVertexPtr++;
			}
		}

		m_commandList->DrawInstanced(nRects * 6, 1, firstVertex, 0);
	}

	//____ _fillPipeline() _____________________________________________________

	ID3D12PipelineState* DX12Backend::_fillPipeline(BlendMode blendMode)
	{
		switch (blendMode)
		{
			case BlendMode::Ignore:
				return nullptr;							// Nothing should be drawn.

			case BlendMode::Replace:
				return m_pFillPipelines[c_fillPipelineReplace].Get();

			case BlendMode::Undefined:
			case BlendMode::Blend:
			case BlendMode::BlendFixedColor:		// Defaults to Blend, like GlBackend does.
				return m_pFillPipelines[c_fillPipelineBlend].Get();

			default:
			{
				static bool bReported = false;
				if (!bReported)
				{
					char msg[128];
					sprintf_s(msg, "DX12Backend: BlendMode %d not supported, using Blend.\n", (int)blendMode);
					OutputDebugStringA(msg);
					bReported = true;
				}

				return m_pFillPipelines[c_fillPipelineBlend].Get();
			}
		}
	}

	//____ setDefaultCanvas() ___________________________________________

	bool DX12Backend::setDefaultCanvas(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, ID3D12Resource* renderTargetBuffer, SizeSPX size, int scale)
	{
		m_defaultCanvasRTV = renderTargetView;
		m_defaultCanvasBuffer = renderTargetBuffer;
		m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;
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
		return true;
	}

	//____ canBeCanvas() ________________________________________________________

	bool DX12Backend::canBeCanvas(const TypeInfo& type) const
	{
		return true;
	}

	//____ waitForCompletion() __________________________________________________

	void DX12Backend::waitForCompletion()
	{
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


		if (!_checkHR(m_pDX12Device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, 0, IID_PPV_ARGS(pointer.GetAddressOf())), "CreateCommittedResource"))
			return;

		pointer->SetName(name);
	}


	//____ _createFillPipelines() _____________________________________________

	bool DX12Backend::_createFillPipelines()
	{
		if (!_createFillRootSignature())
			return false;

		if (!_compileVertexShader(m_fillVertexShaderBlob, g_fillVS))
			return false;

		if (!_compilePixelShader(m_fillPixelShaderBlob, g_fillPS))
			return false;

		if (!_createFillPipeline(BlendMode::Blend, m_pFillPipelines[c_fillPipelineBlend]))
			return false;

		if (!_createFillPipeline(BlendMode::Replace, m_pFillPipelines[c_fillPipelineReplace]))
			return false;

		return true;
	}

	//____ _createFillRootSignature() _________________________________________

	bool DX12Backend::_createFillRootSignature()
	{
		// Two root constants holding the canvas scale for the vertex shader. Root
		// constants instead of a constant buffer, since it is just two floats that
		// only change when the canvas does.

		D3D12_ROOT_PARAMETER rootParameter[1] = {};
		rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[0].Constants.ShaderRegister = 0;
		rootParameter[0].Constants.RegisterSpace = 0;
		rootParameter[0].Constants.Num32BitValues = 2;
		rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc = { };
		rsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		rsDesc.Desc_1_0.pParameters = rootParameter;
		rsDesc.Desc_1_0.NumParameters = 1;
		rsDesc.Desc_1_0.NumStaticSamplers = 0;
		rsDesc.Desc_1_0.pStaticSamplers = 0;
		rsDesc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		Microsoft::WRL::ComPtr<ID3DBlob> pSerializedRS;
		Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;

		HRESULT hr = D3D12SerializeVersionedRootSignature(&rsDesc, pSerializedRS.GetAddressOf(), pErrorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			const char* pErrorMsg = pErrorBlob ? (const char*)pErrorBlob->GetBufferPointer() : "Unknown error";
			char msg[512];
			sprintf_s(msg, "DX12Backend: D3D12SerializeVersionedRootSignature failed, HRESULT = 0x%08lX: %s\n", (unsigned long)hr, pErrorMsg);
			OutputDebugStringA(msg);
			assert(false);
			return false;
		}

		if (!_checkHR(m_pDX12Device->CreateRootSignature(0, pSerializedRS->GetBufferPointer(), pSerializedRS->GetBufferSize(), IID_PPV_ARGS(m_pFillRootSignature.GetAddressOf())), "CreateRootSignature"))
			return false;

		return true;
	}

	//____ _createFillPipeline() ______________________________________________

	bool DX12Backend::_createFillPipeline(BlendMode blendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline)
	{
		// Setup the graphics pipeline state.

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_pFillRootSignature.Get();
		desc.VS.pShaderBytecode = m_fillVertexShaderBlob->GetBufferPointer();
		desc.VS.BytecodeLength = m_fillVertexShaderBlob->GetBufferSize();
		desc.PS.pShaderBytecode = m_fillPixelShaderBlob->GetBufferPointer();
		desc.PS.BytecodeLength = m_fillPixelShaderBlob->GetBufferSize();

		desc.BlendState.AlphaToCoverageEnable = false;
		desc.BlendState.IndependentBlendEnable = false;
		desc.BlendState.RenderTarget[0].BlendEnable = (blendMode == BlendMode::Blend);
		desc.BlendState.RenderTarget[0].LogicOpEnable = false;
		desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		if (blendMode == BlendMode::Blend)
		{
			// Same as GlBackend uses for a canvas that isn't alpha only.

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

		D3D12_INPUT_ELEMENT_DESC elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_INPUT_LAYOUT_DESC inputLayout = {};

		inputLayout.NumElements = 2;
		inputLayout.pInputElementDescs = elements;


		desc.InputLayout = inputLayout;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;


		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.NodeMask = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		if (!_checkHR(m_pDX12Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pPipeline.GetAddressOf())), "CreateGraphicsPipelineState"))
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
			char msg[1024];
			sprintf_s(msg, "DX12Backend: vertex shader compile failed, HRESULT = 0x%08lX: %s\n", (unsigned long)hr, pError);
			OutputDebugStringA(msg);
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
			char msg[1024];
			sprintf_s(msg, "DX12Backend: pixel shader compile failed, HRESULT = 0x%08lX: %s\n", (unsigned long)hr, pError);
			OutputDebugStringA(msg);
			assert(false);
			return false;
		}

		return true;

	}



}