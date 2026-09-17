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

#include <dx12_wrapper.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <assert.h>
#include <cstdio>

using namespace Microsoft::WRL;


Microsoft::WRL::ComPtr<IDXGIDebug>	DX12Wrapper::g_pDebugger;
Microsoft::WRL::ComPtr<ID3D12Debug>	DX12Wrapper::g_pD3D12DebugLayer;


//____ _logHR() __________________________________________________________
//
// Logs failed HRESULTs to the debug output. Unlike assert() this is not
// compiled out in release builds.

static bool _logHR(HRESULT hr, const char* what)
{
	if (FAILED(hr))
	{
		char msg[256];
		sprintf_s(msg, "DX12Wrapper: %s failed, HRESULT = 0x%08lX\n", what, (unsigned long)hr);
		OutputDebugStringA(msg);
		return false;
	}
	return true;
}

//____ initDebugger() ___________________________________________________________
//
// Must be called before the DX12Wrapper is created, since the D3D12 validation
// layer only applies to devices created after it has been enabled.

bool DX12Wrapper::initDebugger()
{
	if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(g_pD3D12DebugLayer.GetAddressOf())))
	{
		// GPU-based validation (ID3D12Debug1::SetEnableGPUBasedValidation) is left
		// off. It catches more, but can delay the first frame by minutes.

		g_pD3D12DebugLayer->EnableDebugLayer();
	}
	else
	{
		g_pD3D12DebugLayer.Reset();
		OutputDebugStringA("DX12Wrapper: D3D12 validation layer not available "
			"(on Windows 10/11 install the 'Graphics Tools' optional feature).\n");
	}

	if (S_OK != DXGIGetDebugInterface1(0, IID_PPV_ARGS(g_pDebugger.GetAddressOf())))
		return false;

	// Break into the debugger on DXGI errors. DXGI has its own message queue,
	// separate from the D3D12 one set up in the constructor.
	//
	// Only when a debugger is attached: without one the break is raised as an
	// exception, which kills the app if it happens inside a window procedure.

	if (IsDebuggerPresent())
	{
		ComPtr<IDXGIInfoQueue> pDXGIInfoQueue;
		if (S_OK == DXGIGetDebugInterface1(0, IID_PPV_ARGS(pDXGIInfoQueue.GetAddressOf())))
		{
			pDXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pDXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
		}
	}

	return true;
}

//____ reportLiveObjects() ______________________________________________________

void DX12Wrapper::reportLiveObjects()
{
	if (g_pDebugger)
	{
		g_pDebugger->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}
}

//____ exitDebugger() ___________________________________________________________

void DX12Wrapper::exitDebugger()
{
	g_pDebugger.Reset();
	g_pD3D12DebugLayer.Reset();
}


//____ constructor _____________________________________________________________

DX12Wrapper::DX12Wrapper()
{
	// Create factory. The debug flag requires the debug layer to be installed,
	// so only ask for it if initDebugger() managed to enable it.

	UINT factoryFlags = g_pD3D12DebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;

	if (!_logHR(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_pDXGIFactory)), "CreateDXGIFactory2"))
	{
		assert(false);
		return;
	}

	// Find adapter

	if (!_findAdapter())
	{
		OutputDebugStringA("DX12Wrapper: _findAdapter failed - no suitable DXGI adapter found\n");
		assert(false);
		return;
	}

	// Create device

	if (!_logHR(D3D12CreateDevice(m_pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_pDX12Device.GetAddressOf())), "D3D12CreateDevice"))
	{
		assert(false);
		return;
	}

	// Break into the debugger on D3D12 errors (only succeeds if the validation
	// layer is enabled). Same debugger-attached condition as in initDebugger().

	if (IsDebuggerPresent())
	{
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (S_OK == m_pDX12Device.As(&pInfoQueue))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		}
	}

	// Create render command queue

	if (!_createRenderCommandQueue())
	{
		OutputDebugStringA("DX12Wrapper: _createRenderCommandQueue failed\n");
		assert(false);
		return;
	}

}

//____ destructor ______________________________________________________________

DX12Wrapper::~DX12Wrapper()
{
	flushRenderQueue();

	if (m_renderQueueFenceEvent)
		CloseHandle(m_renderQueueFenceEvent);
}

//____ flushRenderQueue() ______________________________________________________

void DX12Wrapper::flushRenderQueue()
{
	if (!m_pRenderCommandQueue || !m_renderQueueFence || !m_renderQueueFenceEvent)
		return;

	m_renderQueueFenceValue++;

	if (!_logHR(m_pRenderCommandQueue->Signal(m_renderQueueFence.Get(), m_renderQueueFenceValue), "ID3D12CommandQueue::Signal"))
		return;

	if (m_renderQueueFence->GetCompletedValue() < m_renderQueueFenceValue)
	{
		if (_logHR(m_renderQueueFence->SetEventOnCompletion(m_renderQueueFenceValue, m_renderQueueFenceEvent), "ID3D12Fence::SetEventOnCompletion"))
			WaitForSingleObject(m_renderQueueFenceEvent, INFINITE);
	}
}

//____ _findAdapter() ___________________________________________________________

bool DX12Wrapper::_findAdapter()
{
	UINT adapterIndex = 0;

	ComPtr<IDXGIFactory6> fac6;
	ComPtr<IDXGIAdapter> adapter;

	if (m_pDXGIFactory->QueryInterface(IID_PPV_ARGS(&fac6)) == S_OK) {

		if (S_OK != fac6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))
		{
			return false;
		}

	}
	else
	{
		if (S_OK != m_pDXGIFactory->EnumAdapters(adapterIndex, &adapter))
		{
			return false;
		}

	}

	m_pAdapter = adapter;
	return true;
}

//____ _createRenderCommandQueue() ______________________________________________

bool DX12Wrapper::_createRenderCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC description = {};
	description.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	description.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	description.NodeMask = 0;

	if(S_OK != m_pDX12Device->CreateCommandQueue(&description, IID_PPV_ARGS(&m_pRenderCommandQueue)))
	{
		return false;
	}

	if(S_OK != m_pDX12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_renderQueueFence.GetAddressOf())))
	{
		return false;
	}

	m_renderQueueFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!m_renderQueueFenceEvent)
		return false;

	return true;
}
