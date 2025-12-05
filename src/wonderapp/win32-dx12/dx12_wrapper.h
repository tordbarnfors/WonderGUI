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
#pragma once


#include <windows.h>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")

class DX12Wrapper
{
public:
	DX12Wrapper();
	~DX12Wrapper();


	static bool initDebugger();
	static void reportLiveObjects();
	static void exitDebugger();

	IDXGIFactory* dxgiFactory() const { return m_pDXGIFactory.Get(); }
	IDXGIAdapter* dxgiAdapter() const { return m_pAdapter.Get(); }
	ID3D12Device* dx12Device() const { return m_pDX12Device.Get(); }
	ID3D12CommandQueue* renderCommandQueue() const { return m_pRenderCommandQueue.Get(); }


protected:


	bool _findAdapter();
	bool _createRenderCommandQueue();


	Microsoft::WRL::ComPtr<IDXGIFactory2>		m_pDXGIFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter>		m_pAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device>		m_pDX12Device;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_pRenderCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>			m_renderQueueFence;

	static Microsoft::WRL::ComPtr<IDXGIDebug>	g_pDebugger;

};
