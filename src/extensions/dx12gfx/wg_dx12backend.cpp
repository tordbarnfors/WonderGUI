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


#include <wg_gfxbase.h>



namespace wg
{

	const TypeInfo DX12Backend::TYPEINFO = { "DX12Backend", &GfxBackend::TYPEINFO };

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

		// Create command allocators for each frame resource

		for (int i = 0; i < c_nbFrameResources; ++i)
		{
			pDX12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[i].commandAllocator));
			m_frameResources[i].fenceValue = 0;
		}

		// Create one command list for all frames (will be reset for each frame)

		pDX12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

		m_commandList->Close();

		// Create vertex buffer

		_createBuffer(m_pVertexBuffer, c_vertexBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, L"WonderGUI Vertex Buffer");

//		m_pVertexBuffer->SetName(L"WonderGUI Vertex Buffer");

		/*
			void* destination = nullptr;
			
			m_pVertexBuffer->Map(0, 0, &destination);

			memcpy(destination, &vertexData, sizeof(Vertex));

			m_pVertexBuffer->Unmap(0, 0);
		*/

		_createFillPipeline();

	}

	//____ Destructor ____________________________________________________________

	DX12Backend::~DX12Backend()
	{
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
	}

	//____ endRender() _________________________________________________________

	void DX12Backend::endRender()
	{
		assert( S_OK == m_commandList->Close() );

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

		// Prepare vertex buffer

		m_pVertexBuffer->Map(0, 0, (void**)&m_pVertexBeg);
		m_pVertexEnd = m_pVertexBeg + c_vertexBufferSize / sizeof(Vertex);
		m_pVertexPtr = m_pVertexBeg;

		// Clear render target

		float clearColor[4] = { 0.2f, 0.3f, 0.4f, 1.0f };
		m_commandList->ClearRenderTargetView(m_defaultCanvasRTV, clearColor, 0, nullptr);

		// Set render target

		m_commandList->OMSetRenderTargets(1, &m_defaultCanvasRTV, FALSE, nullptr);

		// Set viewport and scissor

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (FLOAT)m_defaultCanvas.size.w;
		viewport.Height = (FLOAT)m_defaultCanvas.size.h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_commandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = (FLOAT)m_defaultCanvas.size.w;
		scissorRect.bottom = (FLOAT)m_defaultCanvas.size.h;
		m_commandList->RSSetScissorRects(1, &scissorRect);

		// Set pipeline

		m_commandList->SetGraphicsRootSignature(m_pFillRootSignature.Get());
		m_commandList->SetPipelineState(m_pFillPipeline.Get());
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		vertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = c_vertexBufferSize;

		m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView );
	}

	//____ endSession() _______________________________________________________

	void DX12Backend::endSession()
	{
		// Unmap vertex buffer

		m_pVertexBuffer->Unmap(0, 0);



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

	void DX12Backend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd)
	{
		m_pVertexPtr->x = 0.0f;
		m_pVertexPtr->y = 0.0f;
		m_pVertexPtr->z = 0.0f;

		m_pVertexPtr->a = 1.0f;
		m_pVertexPtr->r = 1.0f;
		m_pVertexPtr->g = 1.0f;
		m_pVertexPtr->b = 1.0f;

		m_pVertexPtr++;

		m_pVertexPtr->x = 20.f;
		m_pVertexPtr->y = 0.0f;
		m_pVertexPtr->z = 0.0f;

		m_pVertexPtr->a = 1.0f;
		m_pVertexPtr->r = 1.0f;
		m_pVertexPtr->g = 1.0f;
		m_pVertexPtr->b = 1.0f;

		m_pVertexPtr++;

		m_pVertexPtr->x = 20.f;
		m_pVertexPtr->y = 20.f;
		m_pVertexPtr->z = 0.0f;

		m_pVertexPtr->a = 1.0f;
		m_pVertexPtr->r = 1.0f;
		m_pVertexPtr->g = 1.0f;
		m_pVertexPtr->b = 1.0f;

		m_pVertexPtr++;
	
		m_commandList->DrawInstanced(3, 1, 0, 0);
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
		_waitForFence(m_frameResources[m_currentFrameIndex].fenceValue);
	}

	//____ _waitForFence() _____________________________________________________

	void DX12Backend::_waitForFence(UINT64 fenceValue)
	{
		if (m_commandFence->GetCompletedValue() < fenceValue)
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


		if (S_OK != m_pDX12Device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, 0, IID_PPV_ARGS(pointer.GetAddressOf())))
		{
			assert(false);
		}

		pointer->SetName(name);
	}


	//____ _createFillPipeline() ______________________________________________

	void DX12Backend::_createFillPipeline()
	{
		// First we create the root signature.

		D3D12_ROOT_PARAMETER rootParameter[1];
		rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameter[0].Descriptor.ShaderRegister = 0;
		rootParameter[0].Descriptor.RegisterSpace = 0;
		rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc = { };
		rsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		rsDesc.Desc_1_0.pParameters = rootParameter;
		rsDesc.Desc_1_0.NumParameters = 1;
		rsDesc.Desc_1_0.NumStaticSamplers = 0;
		rsDesc.Desc_1_0.pStaticSamplers = 0;
		rsDesc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* pSerializedRS = nullptr;
		ID3DBlob* pErrorBlob = nullptr;

		if( S_OK != D3D12SerializeVersionedRootSignature(&rsDesc, &pSerializedRS, &pErrorBlob))
		{
			const char* pErrorMsg = pErrorBlob->GetBufferPointer() ? (const char*)pErrorBlob->GetBufferPointer() : "Unknown error";
			assert(false);
		}

		if( S_OK != m_pDX12Device->CreateRootSignature(0, pSerializedRS->GetBufferPointer(), pSerializedRS->GetBufferSize(), IID_PPV_ARGS(m_pFillRootSignature.GetAddressOf())))
		{
			assert(false);
		}

		// Next we compile the shaders.

		if( false == _compileVertexShader(m_fillVertexShaderBlob, g_fillVS) )
		{
			assert(false);
		}

		if( false == _compilePixelShader(m_fillPixelShaderBlob, g_fillPS) )
		{
			assert(false);
		}

		// Setup the graphics pipeline state.

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_pFillRootSignature.Get();
		desc.VS.pShaderBytecode = m_fillVertexShaderBlob->GetBufferPointer();
		desc.VS.BytecodeLength = m_fillVertexShaderBlob->GetBufferSize();
		desc.PS.pShaderBytecode = m_fillPixelShaderBlob->GetBufferPointer();
		desc.PS.BytecodeLength = m_fillPixelShaderBlob->GetBufferSize();

		desc.BlendState.AlphaToCoverageEnable = false;
		desc.BlendState.IndependentBlendEnable = false;
		desc.BlendState.RenderTarget[0].BlendEnable = false;
		desc.BlendState.RenderTarget[0].LogicOpEnable = false;
		desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

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
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
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

		if (S_OK != m_pDX12Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pFillPipeline.GetAddressOf())) )
		{
			assert(false);
		}

	}

	//____ _compileVertexShader() _____________________________________________

	bool DX12Backend::_compileVertexShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc)
	{ 
		UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;

		ID3DBlob* errorMsg = nullptr;

		if (S_OK != D3DCompile(pSrc, strlen((const char*)pSrc), nullptr, nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, shaderBlob.GetAddressOf(), &errorMsg) )
		{
			const char* pError = errorMsg ? (const char*)errorMsg->GetBufferPointer() : nullptr;
			assert(false);
			return false;
		}

		return true;
	}

	//____ _compilePixeShader() _______________________________________________

	bool DX12Backend::_compilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc)
	{
		UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;

		ID3DBlob* errorMsg = nullptr;

		if (S_OK != D3DCompile(pSrc, strlen((const char*)pSrc), nullptr, nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, shaderBlob.GetAddressOf(), &errorMsg))
		{
			const char* pError = errorMsg ? (const char*)errorMsg->GetBufferPointer() : nullptr;
			assert(false);
			return false;
		}

		return true;

	}



}