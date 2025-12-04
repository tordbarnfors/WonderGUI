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

#include <wg_rootpanel.h>
#include <dx12_wrapper.h>

#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>


class DXGI_SwapChain
{
	public:
	DXGI_SwapChain( DX12Wrapper* pDX12Wrapper, HWND hwnd, wg::RootPanel * pRoot, UINT width, UINT heigh);
	~DXGI_SwapChain();

	void	resizeBuffers(UINT width, UINT height);
	void	present();

	ID3D12Resource* renderTargetBuffer() const { return m_renderBuffers[m_currentBuffer].Get(); }


	IDXGISwapChain1* swapChain() const { return m_pSwapChain.Get(); }

private:

	void _createBuffers();
	void _dropBuffers();

	static const UINT								c_nbBuffers = 2;

	Microsoft::WRL::ComPtr<ID3D12Device>			m_pDX12Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_renderBuffers[c_nbBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE						m_rtvHandles[c_nbBuffers];

	HWND m_hwnd = nullptr;
	UINT m_currentBuffer = 0;
	 
	UINT m_width = 0, m_height = 0;
	UINT m_heapIncrement = 0;

	wg::RootPanel_p m_pRootPanel;
};