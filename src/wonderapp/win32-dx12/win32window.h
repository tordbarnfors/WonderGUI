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


#include <wonderapp.h>
#include <wondergui.h>

#include <wappwindow.h>
#include <windows.h>

#include <dxgi_swapchain.h>


//____ Win32Window _______________________________________________________________


class Win32Window : public wapp::WindowAPI::SysCalls
{
public:

	Win32Window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~Win32Window();


    //.____ Misc ____________________________________________________

    void				render();
	void				paint();
	wapp::Window*		userWindow() const { return m_pUserWindow; }
	wg::RootPanel_p		rootPanel() const { return m_pRootPanel; }

	void				onResize(int width, int height);

protected:

	// SysCalls interface

	void			destroy() override;
	wg::Rect		setGeo(const wg::Rect& geo) override;
	bool			requestFocus() override;
	bool			releaseFocus() override;
	bool			minimize() override;
	bool			restore() override;

	bool 			setTitle(std::string& title) override;
	std::string 	title() override;


	//

	void				_createSwapChain(DX12Wrapper* pDX12Wrapper, const HWND hwnd, UINT width, UINT height);

	void				_createSwapChainBuffers();
	void				_dropSwapChainBuffers();


	//

    HWND				m_windowHandle;

//	HBITMAP				m_hBitmap;
//	DWORD *				m_pCanvasPixels;

	wapp::Window *		m_pUserWindow;
	wg::RootPanel_p		m_pRootPanel;

	bool				m_bHidden = false;

	//

	static const UINT								c_nbBuffers = 2;

	Microsoft::WRL::ComPtr<ID3D12Device>			m_pDX12Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_renderBuffers[c_nbBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE						m_rtvHandles[c_nbBuffers];

	UINT m_currentBuffer = 0;

	UINT m_width = 0, m_height = 0;
	UINT m_heapIncrement = 0;

};