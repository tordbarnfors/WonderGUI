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

#include <dxgi_swapchain.h>

#include <assert.h>

//____ Constructor ________________________________________________________

DXGI_SwapChain::DXGI_SwapChain(DX12Wrapper* pDX12Wrapper, const HWND hwnd, wg::RootPanel* pRoot, UINT width, UINT height)
{
	
	
	ID3D12Device* pDevice = pDX12Wrapper->dx12Device();
		
	IDXGIFactory2* pFactory = static_cast<IDXGIFactory2*>(pDX12Wrapper->dxgiFactory());
		
	ID3D12CommandQueue* pCommandQueue = pDX12Wrapper->renderCommandQueue();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = c_nbBuffers;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	if( S_OK != pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RTVHeap.GetAddressOf())) )
		assert(false);

	m_heapIncrement = pDevice->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);

	DXGI_SWAP_CHAIN_DESC1 description = {};
	description.Width = width;
	description.Height = height;
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.Stereo = false;
	description.SampleDesc = { 1,0 };
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = c_nbBuffers;
	description.Scaling = DXGI_SCALING_NONE;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; /* | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; */

	if (S_OK != pFactory->CreateSwapChainForHwnd(pCommandQueue, hwnd, &description, nullptr, nullptr, &m_pSwapChain))
		assert(false);

	m_pDX12Device = pDevice;

	_createBuffers();

	m_width = width;
	m_height = height;
	m_hwnd = hwnd;
}

//____ Destructor _______________________________________________________

DXGI_SwapChain::~DXGI_SwapChain()
{

}

//____ present() ________________________________________________________

void DXGI_SwapChain::present()
{
	m_pSwapChain->Present(0, 0);
}

//____ createBuffers() _________________________________________________

void DXGI_SwapChain::_createBuffers()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < c_nbBuffers; i++)
	{
		m_renderBuffers[i].Reset();

		if (S_OK != m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderBuffers[i])))
			assert(false);
		m_pDX12Device->CreateRenderTargetView(m_renderBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_heapIncrement;
	}
} 

//____ resizeBuffers() _________________________________________________

void DXGI_SwapChain::resizeBuffers(UINT width, UINT height)
{
	if (width == m_width && height == m_height)
		return;

	m_width = width;
	m_height = height;

	// Note: all outstanding references to the buffers must be released before calling ResizeBuffers.

	_dropBuffers();
	if (S_OK != m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING  /* | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/))
		assert(false);
	_createBuffers();

	// Present a frame to update the window with new buffer size.
	//TODO: This should be rendered before presenting, otherwise we will probably get blinking black on resize.
	m_pSwapChain->Present(0, 0);

}

//____ _dropBuffers() ____________________________________________________

void DXGI_SwapChain::_dropBuffers()
{
	for (UINT i = 0; i < c_nbBuffers; i++)
		m_renderBuffers[i].Reset();
}
