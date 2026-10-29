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

#include <wax_dx12surfacefactory.h>
#include <wax_dx12backend.h>

#include <dx12_wrapper.h>
#include <win32window.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace wg;

// Owns the D3D12 device/queue/debug-layer wrapper. Win32Window (which creates
// the swap chain for each window) reaches it through this extern.

DX12Wrapper* g_pDX12Wrapper = nullptr;

extern std::vector<Win32Window*>	g_win32Windows;

// A new device can't always be had right after the old one was lost - a reset
// GPU or an updated driver takes a moment to come back - so we try again at
// this interval until it works.

static const ULONGLONG	c_recoveryRetryInterval = 1000;		// Milliseconds.
static ULONGLONG		s_nextRecoveryAttempt = 0;

//____ _recoverDevice() _______________________________________________________
//
// Everything has to let go of the old device before D3D12 gives us a new one:
// the windows their swap chains, DX12Backend everything it and the surfaces
// and edgemaps have on it, and the wrapper the device itself. Then it all
// comes back on the new device.

static bool _recoverDevice()
{
	if (wax::DX12Backend::device())
	{
		for (auto pWin : g_win32Windows)
			pWin->releaseDevice();

		wax::DX12Backend::setDevice(nullptr, nullptr);
		g_pDX12Wrapper->releaseDevice();
	}

	if (!g_pDX12Wrapper->createDevice())
		return false;

	if (!wax::DX12Backend::setDevice(g_pDX12Wrapper->dx12Device(), g_pDX12Wrapper->renderCommandQueue()))
	{
		g_pDX12Wrapper->releaseDevice();
		return false;
	}

	for (auto pWin : g_win32Windows)
		pWin->restoreDevice();

	return true;
}

//____ _simulateDeviceLoss() __________________________________________________
//
// Debug aid: set the environment variable WAPP_DX12_SIMULATE_DEVICE_LOSS to a
// number of seconds, and the device is removed that often, to test recovery.

#ifndef NDEBUG
static void _simulateDeviceLoss()
{
	static ULONGLONG interval = 0;
	static ULONGLONG nextLoss = 0;
	static bool bInitialized = false;

	if (!bInitialized)
	{
		bInitialized = true;

		char value[32];
		if (GetEnvironmentVariableA("WAPP_DX12_SIMULATE_DEVICE_LOSS", value, sizeof(value)) > 0)
			interval = ULONGLONG(atoi(value)) * 1000;

		nextLoss = GetTickCount64() + interval;
	}

	if (interval == 0 || wax::DX12Backend::isDeviceLost() || GetTickCount64() < nextLoss)
		return;

	if (wax::DX12Backend::simulateDeviceLoss())
		OutputDebugStringA("wapp: Simulating device loss\n");

	nextLoss = GetTickCount64() + interval;
}
#endif

//____ updateGfxBackend() _____________________________________________________

void updateGfxBackend()
{
#ifndef NDEBUG
	_simulateDeviceLoss();
#endif

	// DX12Backend notices a lost device on its own, but only once it tries to
	// use it. Asking every frame catches it before we render into it.

	if (!wax::DX12Backend::checkDeviceLost())
		return;

	ULONGLONG now = GetTickCount64();
	if (now < s_nextRecoveryAttempt)
		return;

	if (_recoverDevice())
	{
		OutputDebugStringA("wapp: Recovered from device loss\n");
		s_nextRecoveryAttempt = 0;
	}
	else
	{
		OutputDebugStringA("wapp: Failed to recover from device loss, trying again shortly\n");
		s_nextRecoveryAttempt = now + c_recoveryRetryInterval;
	}
}

//____ initGfxBackend() _______________________________________________________

void initGfxBackend()
{
#ifndef NDEBUG
	DX12Wrapper::initDebugger();		// Validation layer is slow, keep it out of release builds.
#endif

	g_pDX12Wrapper = new DX12Wrapper();

	auto pFactory = wax::DX12SurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pFactory);

	auto pBackend = wax::DX12Backend::create(g_pDX12Wrapper->dx12Device(), g_pDX12Wrapper->renderCommandQueue());

	auto pGfxDevice = wg::GfxDeviceGen2::create(pBackend);
	Base::setDefaultDevice(pGfxDevice);

	Base::setDefaultEdgemapFactory(pBackend->edgemapFactory());
	Base::setDefaultWarpmapFactory(pBackend->warpmapFactory());
}

//____ exitGfxBackend() _______________________________________________________

void exitGfxBackend()
{
	delete g_pDX12Wrapper;
	g_pDX12Wrapper = nullptr;

	DX12Wrapper::reportLiveObjects();
	DX12Wrapper::exitDebugger();
}
