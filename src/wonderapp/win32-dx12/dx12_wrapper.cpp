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

using namespace Microsoft::WRL;


Microsoft::WRL::ComPtr<IDXGIDebug>	DX12Wrapper::g_pDebugger;


//____ initDebugger() ___________________________________________________________

bool DX12Wrapper::initDebugger()
{
	if (S_OK != DXGIGetDebugInterface1(0, IID_PPV_ARGS(g_pDebugger.GetAddressOf())))
		return false;

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
}


//____ constructor _____________________________________________________________

DX12Wrapper::DX12Wrapper()
{ 
	if (S_OK != CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_pDXGIFactory)))
	{
		assert(false);
	}

	if (!_findAdapter())
	{
		assert(false);
	}

	if (S_OK != D3D12CreateDevice(m_pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_pDX12Device.GetAddressOf())))
	{
		assert(false);
	}

}

//____ destructor ______________________________________________________________

DX12Wrapper::~DX12Wrapper()
{

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