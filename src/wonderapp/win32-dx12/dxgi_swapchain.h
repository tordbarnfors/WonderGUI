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

#include <wrl.h>
#include <dxgi1_6.h>


class DXGI_SwapChain
{
	public:
	DXGI_SwapChain();
	~DXGI_SwapChain();

//	bool		createSwapChain(IDXGIFactory2* pFactory, IUnknown* pDevice, HWND hWnd, UINT width, UINT height, UINT bufferCount, DXGI_FORMAT format, bool bWindowed);
	void		present();

	IDXGISwapChain1* swapChain() const { return m_pSwapChain.Get(); }

private:

	Microsoft::WRL::ComPtr<IDXGISwapChain1>	m_pSwapChain;

};