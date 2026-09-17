/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/

#include <win32gfxbackend.h>

#include <wondergui.h>

#include <wg_dx12surfacefactory.h>
#include <wg_dx12backend.h>

#include <dx12_wrapper.h>

using namespace wg;

// Owns the D3D12 device/queue/debug-layer wrapper. Win32Window (which creates
// the swap chain for each window) reaches it through this extern.

DX12Wrapper* g_pDX12Wrapper = nullptr;

//____ initGfxBackend() _______________________________________________________

void initGfxBackend()
{
#ifndef NDEBUG
	DX12Wrapper::initDebugger();		// Validation layer is slow, keep it out of release builds.
#endif

	g_pDX12Wrapper = new DX12Wrapper();

	auto pFactory = wg::DX12SurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pFactory);

	auto pBackend = wg::DX12Backend::create(g_pDX12Wrapper->dx12Device(), g_pDX12Wrapper->renderCommandQueue());

	auto pGfxDevice = wg::GfxDeviceGen2::create(pBackend);
	Base::setDefaultGfxDevice(pGfxDevice);
}

//____ exitGfxBackend() _______________________________________________________

void exitGfxBackend()
{
	delete g_pDX12Wrapper;
	g_pDX12Wrapper = nullptr;

	DX12Wrapper::reportLiveObjects();
	DX12Wrapper::exitDebugger();
}
