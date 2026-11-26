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

#define NOMINMAX

#include <wax_dx12backend.h>
#include <wax_dx12surface.h>
#include <wax_dx12surfacefactory.h>
#include <wax_dx12edgemap.h>
#include <wax_dx12edgemapfactory.h>
#include <wax_dx12warpmap.h>
#include <wax_dx12warpmapfactory.h>
#include <wax_dx12bufferpool.h>

#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <vector>
#include <utility>
#include <algorithm>

#include <wax_base.h>
#include <wax_util.h>



namespace wax
{

	//____ setObjectName() _________________________________________________________
	//
	// Names a D3D12 object after the backend owning it, so the debug layer's live
	// object report (ID3D12DebugDevice::ReportLiveDeviceObjects) and tools like
	// PIX show which backend it belongs to.

	static void setObjectName( const void * pBackend, ID3D12Object * pObject, const wchar_t * pWhat )
	{
		if( !pObject )
			return;

		wchar_t name[128];
		swprintf( name, 128, L"Wax DX12Backend %p: %ls", pBackend, pWhat );
		pObject->SetName( name );
	}

	const TypeInfo DX12Backend::TYPEINFO = { "DX12Backend", &GpuBackend::TYPEINFO };


	// Debug aid: clear each update rect before drawing it, so it is obvious which
	// parts of the canvas are redrawn and whether anything is left unpainted. The
	// window must present full frames while this is on, see beginSession().

	static constexpr bool c_bDebugClearUpdateRects = false;

	//____ _checkHR() __________________________________________________________
	//
	// Reports failed HRESULTs through the error handler. Use the CHECK_HR macro,
	// which passes on the call site.
	//
	// A lost device is reported once by DX12Backend instead. Every call fails from
	// then on, and reporting each one, every frame, is what buried the reason.

	static bool _checkHR(HRESULT hr, const char* what, const Object* pObject, const TypeInfo* pClassType, const char* func, const char* file, int line)
	{
		if (FAILED(hr))
		{
			if (DX12Backend::handleDeviceLoss(hr))
				return false;

			char buffer[256];
			sprintf_s(buffer, "%s failed, HRESULT = 0x%08lX", what, (unsigned long)hr);
			Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, pObject, pClassType, func, file, line);
			assert(false);
			return false;
		}
		return true;
	}

	#define CHECK_HR(call, what) _checkHR(call, what, this, &TYPEINFO, __func__, __FILE__, __LINE__)

	Microsoft::WRL::ComPtr<ID3D12Device>		DX12Backend::s_pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	DX12Backend::s_pCommandQueue;
	bool										DX12Backend::s_bImplicitDevice = false;
	bool										DX12Backend::s_bDeviceLost = false;
	HRESULT										DX12Backend::s_deviceRemovedReason = S_OK;
	TP_CALLBACK_ENVIRON							DX12Backend::s_releaserEnv;
	bool										DX12Backend::s_bReleaserReady = false;
	std::atomic<int>							DX12Backend::s_nPendingReleases(0);
	std::mutex									DX12Backend::s_pendingMutex;
	std::vector<DX12Backend::PendingRelease*>	DX12Backend::s_pendingReleases;
	std::vector<DX12Backend*>					DX12Backend::s_backends;
	UINT64										DX12Backend::s_lastListSerial = 0;

	//____ create() ______________________________________________________________

	DX12Backend_p DX12Backend::create()
	{
		if (!s_pDevice || !s_pCommandQueue)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. Call DX12Backend::setDevice() before creating a backend.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return DX12Backend_p(new DX12Backend(s_pDevice.Get(), s_pCommandQueue.Get()));
	}

	DX12Backend_p DX12Backend::create(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		if (!s_pDevice)
		{
			if (!setDevice(pDX12Device, pDX12CommandQueue))
				return nullptr;
			s_bImplicitDevice = true;
		}
		else if (s_pDevice.Get() != pDX12Device || s_pCommandQueue.Get() != pDX12CommandQueue)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam,
				"All backends must share the device and command queue set by DX12Backend::setDevice().",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return create();
	}

	//____ setDevice() ___________________________________________________________

	bool DX12Backend::setDevice(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		if ((pDX12Device == nullptr) != (pDX12CommandQueue == nullptr))
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam,
				"Device and command queue must both be set or both be null.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if (!s_backends.empty())
		{
			// The owner of the device has to let go of it in between, see the header.

			if (pDX12Device && s_pDevice)
			{
				Base::reportError(ErrorLevel::Error, ErrorCode::IllegalCall,
					"Can't change D3D12 device while DX12Backends exist without first releasing it with setDevice(nullptr, nullptr).",
					nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;
			}

			for (auto pBackend : s_backends)
			{
				if (pBackend->m_bCommandListOpen || pBackend->m_bInSession)
				{
					Base::reportError(ErrorLevel::Error, ErrorCode::IllegalCall,
						"Can't change D3D12 device in the middle of a frame.",
						nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
					return false;
				}
			}
		}

		if (pDX12Device && pDX12Device == s_pDevice.Get() && pDX12CommandQueue == s_pCommandQueue.Get())
			return true;

		if (s_pDevice)
		{
			// Surfaces go first. If the device still works they read back what was
			// rendered into them, and the backends have to be there to submit and
			// wait for what is still recorded.

			DX12Surface::evacuateAll();
			DX12Edgemap::_evacuateAll();

			for (auto pBackend : s_backends)
				pBackend->_releaseDeviceObjects();

			// The GPU is done with everything the release callbacks still hold, or
			// gone. Either way it can go now, and with it the last of the device.

			_flushPendingReleases();

			DX12Surface::exitDevice();
			DX12BufferPool::exitDevice();
		}

		s_pDevice = pDX12Device;
		s_pCommandQueue = pDX12CommandQueue;

		// A device replaced under the backends is still theirs to release with the
		// last of them.

		if (s_backends.empty())
			s_bImplicitDevice = false;

		if (pDX12Device)
		{
			// A new device starts out healthy. If it is the one we already had, the
			// next call on it will tell us again. Without one, a lost device stays
			// lost, so nothing tries to use the missing device in between.

			s_bDeviceLost = false;
			s_deviceRemovedReason = S_OK;

			// Surfaces create their own resources and need the device for it, as does
			// the pool edgemaps, Warpmaps and palettes get their buffers from.

			DX12Surface::setDevice(pDX12Device);
			DX12BufferPool::setDevice(pDX12Device, pDX12CommandQueue);

			for (auto pBackend : s_backends)
				pBackend->_createDeviceObjects(pDX12Device, pDX12CommandQueue);

			DX12Surface::migrateAll();
			DX12Edgemap::_migrateAll();
		}

		return true;
	}

	//____ simulateDeviceLoss() __________________________________________________

	bool DX12Backend::simulateDeviceLoss()
	{
#ifdef __ID3D12Device5_INTERFACE_DEFINED__
		Microsoft::WRL::ComPtr<ID3D12Device5> pDevice5;

		if (!s_pDevice || FAILED(s_pDevice.As(&pDevice5)))
			return false;

		pDevice5->RemoveDevice();
		return true;
#else
		return false;
#endif
	}

	//____ _isAnyRecording() _____________________________________________________

	bool DX12Backend::_isAnyRecording()
	{
		for (auto pBackend : s_backends)
		{
			if (pBackend->m_bCommandListOpen)
				return true;
		}

		return false;
	}

	//____ waitForCompletionOfAll() ______________________________________________

	void DX12Backend::waitForCompletionOfAll()
	{
		for (auto pBackend : s_backends)
			pBackend->waitForCompletion();
	}

	//____ checkDeviceLost() _____________________________________________________

	bool DX12Backend::checkDeviceLost()
	{
		if (s_bDeviceLost)
			return true;

		if (!s_pDevice)
			return false;

		HRESULT reason = s_pDevice->GetDeviceRemovedReason();
		if (reason == S_OK)
			return false;

		_reportDeviceLost(reason);
		return true;
	}

	//____ handleDeviceLoss() ____________________________________________________

	bool DX12Backend::handleDeviceLoss(HRESULT hr)
	{
		// Once the device is gone, every call fails, whatever it says.

		if (s_bDeviceLost)
			return true;

		switch (hr)
		{
			case DXGI_ERROR_DEVICE_REMOVED:
			case DXGI_ERROR_DEVICE_RESET:
			case DXGI_ERROR_DEVICE_HUNG:
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				_reportDeviceLost(hr);
				return true;

			default:
				break;
		}

		// Some calls fail with something less telling, so ask the device itself.

		return checkDeviceLost();
	}

	//____ _reportDeviceLost() ___________________________________________________

	void DX12Backend::_reportDeviceLost(HRESULT hr)
	{
		if (s_bDeviceLost)
			return;

		s_bDeviceLost = true;

		// What the failing call returned is usually just DEVICE_REMOVED. The device
		// knows why.

		HRESULT reason = s_pDevice ? s_pDevice->GetDeviceRemovedReason() : S_OK;
		if (reason == S_OK)
			reason = hr;

		s_deviceRemovedReason = reason;

		const char * pMeaning;
		switch (reason)
		{
			case DXGI_ERROR_DEVICE_HUNG:
				pMeaning = "DEVICE_HUNG, the GPU took too long on our commands and was reset (TDR)";
				break;
			case DXGI_ERROR_DEVICE_RESET:
				pMeaning = "DEVICE_RESET, the GPU was reset, most likely by badly formed commands";
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				pMeaning = "DEVICE_REMOVED, the GPU was removed or its driver updated or restarted";
				break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				pMeaning = "DRIVER_INTERNAL_ERROR, the driver failed";
				break;
			case DXGI_ERROR_INVALID_CALL:
				pMeaning = "INVALID_CALL, the device was given something invalid, run with the debug layer";
				break;
			case E_OUTOFMEMORY:
				pMeaning = "E_OUTOFMEMORY";
				break;
			default:
				pMeaning = "unknown reason";
				break;
		}

		// The breadcrumbs go first. The error handler may well break into the
		// debugger, and then they are already in the output window.

		_dumpDRED();

		char buffer[384];
		sprintf_s(buffer, "D3D12 device lost: 0x%08lX, %s. DX12 rendering stops until a new device is set.",
				  (unsigned long)reason, pMeaning);

		Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	//____ _dumpDRED() ___________________________________________________________
	//
	// Device Removed Extended Data: which operation in which command list the GPU
	// had got to when it died, and what a page fault hit. Only there if DRED was
	// enabled through ID3D12DeviceRemovedExtendedDataSettings before the device
	// was created, which is up to whoever creates it.
	//
	// Written to the debugger's output window, since it doesn't fit an error
	// message. Our objects are named after their owners, see setObjectName(),
	// so the command lists and allocations listed say whose they are.

	void DX12Backend::_dumpDRED()
	{
#ifdef __ID3D12DeviceRemovedExtendedData_INTERFACE_DEFINED__

		if (!s_pDevice)
			return;

		Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedData> pDred;

		if (FAILED(s_pDevice->QueryInterface(IID_PPV_ARGS(pDred.GetAddressOf()))))
		{
			OutputDebugStringA("Wax DX12Backend: No DRED data. Enable it with ID3D12DeviceRemovedExtendedDataSettings before the device is created.\n");
			return;
		}

		static const char * s_opNames[] = {
			"SetMarker", "BeginEvent", "EndEvent", "DrawInstanced", "DrawIndexedInstanced", "ExecuteIndirect",
			"Dispatch", "CopyBufferRegion", "CopyTextureRegion", "CopyResource", "CopyTiles", "ResolveSubresource",
			"ClearRenderTargetView", "ClearUnorderedAccessView", "ClearDepthStencilView", "ResourceBarrier",
			"ExecuteBundle", "Present", "ResolveQueryData", "BeginSubmission", "EndSubmission" };

		const int nOpNames = int(sizeof(s_opNames) / sizeof(s_opNames[0]));

		char line[512];

		// Breadcrumbs. A command list the GPU finished is not where it died, so
		// only the unfinished ones are listed, with the operations around the
		// first one that never completed.

		// Both getters fail with DXGI_ERROR_NOT_CURRENTLY_AVAILABLE when DRED was
		// not enabled for this device. The interface itself is always there on
		// Windows 10 1903 and later, so getting it says nothing about that.

		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs = {};

		HRESULT hrBreadcrumbs = pDred->GetAutoBreadcrumbsOutput(&breadcrumbs);
		if (FAILED(hrBreadcrumbs))
		{
			sprintf_s(line, "Wax DX12Backend: No DRED breadcrumbs (0x%08lX). Auto-breadcrumbs were not enabled when the device was created, "
							"which has to happen before the first D3D12CreateDevice() in the process.\n", (unsigned long)hrBreadcrumbs);
			OutputDebugStringA(line);
		}
		else
		{
			OutputDebugStringA("Wax DX12Backend: DRED breadcrumbs, command lists the GPU had not finished:\n");

			int nUnfinished = 0;

			for (const D3D12_AUTO_BREADCRUMB_NODE* pNode = breadcrumbs.pHeadAutoBreadcrumbNode; pNode; pNode = pNode->pNext)
			{
				UINT32 nOps = pNode->BreadcrumbCount;
				UINT32 nDone = pNode->pLastBreadcrumbValue ? *pNode->pLastBreadcrumbValue : 0;

				if (nDone >= nOps)
					continue;

				nUnfinished++;

				sprintf_s(line, "  %ls, on %ls: %u of %u operations done.\n",
						  pNode->pCommandListDebugNameW ? pNode->pCommandListDebugNameW : L"(unnamed command list)",
						  pNode->pCommandQueueDebugNameW ? pNode->pCommandQueueDebugNameW : L"(unnamed queue)",
						  nDone, nOps);
				OutputDebugStringA(line);

				if (!pNode->pCommandHistory)
					continue;

				UINT32 first = nDone > 8 ? nDone - 8 : 0;
				UINT32 last = (std::min)(nOps, nDone + 8);		// Parenthesized against windows.h's min macro.

				for (UINT32 i = first; i < last; i++)
				{
					int op = int(pNode->pCommandHistory[i]);

					if (op >= 0 && op < nOpNames)
						sprintf_s(line, "    %s %u: %s\n", i == nDone ? "->" : "  ", i, s_opNames[op]);
					else
						sprintf_s(line, "    %s %u: operation %d\n", i == nDone ? "->" : "  ", i, op);

					OutputDebugStringA(line);
				}
			}

			if (nUnfinished == 0)
				OutputDebugStringA("  None. Either all had finished or breadcrumbs were not enabled.\n");
		}

		// Page fault. A fault in a recently freed allocation means something was
		// released while the GPU still used it.

		D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};

		HRESULT hrPageFault = pDred->GetPageFaultAllocationOutput(&pageFault);
		if (FAILED(hrPageFault))
		{
			sprintf_s(line, "Wax DX12Backend: No DRED page fault data (0x%08lX), page fault reporting was not enabled.\n", (unsigned long)hrPageFault);
			OutputDebugStringA(line);
		}
		else
		{
			if (pageFault.PageFaultVA == 0)
				OutputDebugStringA("Wax DX12Backend: DRED reports no page fault.\n");
			else
			{
				sprintf_s(line, "Wax DX12Backend: DRED page fault at GPU address 0x%llX.\n", (unsigned long long) pageFault.PageFaultVA);
				OutputDebugStringA(line);

				struct { const D3D12_DRED_ALLOCATION_NODE* pHead; const char* pWhat; } lists[] = {
					{ pageFault.pHeadExistingAllocationNode, "  Live allocations at that address:\n" },
					{ pageFault.pHeadRecentFreedAllocationNode, "  Recently freed allocations at that address:\n" } };

				for (auto& list : lists)
				{
					OutputDebugStringA(list.pWhat);

					if (!list.pHead)
						OutputDebugStringA("    None.\n");

					for (const D3D12_DRED_ALLOCATION_NODE* pNode = list.pHead; pNode; pNode = pNode->pNext)
					{
						sprintf_s(line, "    %ls (allocation type %d)\n",
								  pNode->ObjectNameW ? pNode->ObjectNameW : L"(unnamed)", int(pNode->AllocationType));
						OutputDebugStringA(line);
					}
				}
			}
		}
#else
		OutputDebugStringA("Wax DX12Backend: Built with a Windows SDK too old for DRED (10.0.18362 or later needed).\n");
#endif
	}

	//____ Constructor ___________________________________________________________

	DX12Backend::DX12Backend(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		m_pSurfaceFactory = DX12SurfaceFactory::create();
		m_pEdgemapFactory = DX12EdgemapFactory::create();

		s_backends.push_back(this);

		if (s_backends.size() == 1)
			_initReleaser();

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		_createDeviceObjects(pDX12Device, pDX12CommandQueue);
	}

	//____ _createDeviceObjects() ________________________________________________
	//
	// Everything we have on the device. Called by the constructor, and by
	// setDevice() when we move to a new device, after _releaseDeviceObjects().

	bool DX12Backend::_createDeviceObjects(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue)
	{
		m_pDX12CommandQueue = pDX12CommandQueue;
		m_pDX12Device = pDX12Device;

		m_srvDescriptorSize = pDX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_samplerDescriptorSize = pDX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		// The old fence may have set the event after we last waited on it.

		if (m_fenceEvent)
			ResetEvent(m_fenceEvent);

		// Anything failing here leaves us unable to record, and beginRender() then
		// does nothing. That is what happens on a device that is already lost,
		// which CHECK_HR reports once rather than for each call.

		bool bOK = CHECK_HR(pDX12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_commandFence)), "CreateFence");
		m_fenceValue = 0;

		// Create a command allocator for each frame resource.
		//
		// Each frame resource also has its own vertex, color and extras buffers,
		// since the GPU may still be reading the other frame's. They are created
		// by beginSession(), sized by what the sessions say they need, see
		// _reserveFrameBuffers().

		for (int i = 0; i < c_nbFrameResources; ++i)
		{
			bOK = CHECK_HR(pDX12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[i].commandAllocator)), "CreateCommandAllocator") && bOK;
			m_frameResources[i].fenceValue = 0;

			// Shader visible descriptors for blit sources, refilled every frame.

			_createSRVHeap(i, c_nbSRVDescriptors);
		}

		// Create one command list for all frames (will be reset for each frame)

		if (bOK)
			bOK = CHECK_HR(pDX12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)), "CreateCommandList");

		if (m_commandList)
			m_commandList->Close();

		m_bDeviceObjectsOK = bOK && m_fenceEvent != nullptr;

		// Named after us, so the debug layer's live object report shows where they come from.

		setObjectName(this, m_commandFence.Get(), L"command fence");
		setObjectName(this, m_commandList.Get(), L"command list");

		for (int i = 0; i < c_nbFrameResources; ++i)
			setObjectName(this, m_frameResources[i].commandAllocator.Get(), L"command allocator");

		m_bCommandListOpen = false;

		if (!_createPipelineResources())
		{
			if (!s_bDeviceLost)
				Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create pipeline resources, nothing will render.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		return m_bDeviceObjectsOK;
	}

	//____ _releaseDeviceObjects() _______________________________________________
	//
	// Lets go of everything we have on the device, and everything we know of
	// that lives there: buffers, pipelines, the canvases we were pointed at. If
	// the device still works, we wait for the GPU to finish with it first.

	void DX12Backend::_releaseDeviceObjects()
	{
		if (!s_bDeviceLost)
			waitForCompletion();

		m_bCommandListOpen = false;
		m_bDeviceObjectsOK = false;

		_abandonRecordedUploads();		// Only if the list was never submitted, which takes a lost device.
		m_bInSession = false;
		m_bSessionOnDefaultCanvas = false;

		m_heldResources.clear();
		m_blitSourceCanvases.clear();
		m_objectSRVs.clear();

		m_commandList = nullptr;
		m_commandFence = nullptr;
		m_fenceValue = 0;

		for (auto& frame : m_frameResources)
		{
			frame.commandAllocator = nullptr;
			frame.vertexBuffer = nullptr;			// Releasing a buffer unmaps it.
			frame.colorBuffer = nullptr;
			frame.extrasBuffer = nullptr;
			frame.srvHeap = nullptr;
			frame.srvCapacity = 0;
			frame.pVertexBufferData = nullptr;
			frame.pColorBufferData = nullptr;
			frame.pExtrasBufferData = nullptr;
			frame.vertexCapacity = 0;
			frame.colorCapacity = 0;
			frame.extrasCapacity = 0;
			frame.nSRVDescriptors = 0;
			frame.fenceValue = 0;
		}

		m_buffers = FrameBuffers();

		m_pipelines.clear();
		m_mipmapPipelines.clear();
		m_pActivePipeline = nullptr;
		m_pRootSignature = nullptr;
		m_pSamplerHeap = nullptr;

		m_pActiveCanvas = nullptr;
		m_bCanvasBound = false;
		m_pCanvasSurface = nullptr;
		m_activeCanvasRTV = {};
		m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;

		m_pBlitSource = nullptr;
		m_bBlitSourceBound = false;

		// The default canvas is a swap chain buffer, which goes with the device.
		// Its size, scale and format stay, they are still what the window has.

		m_defaultCanvasRTV = {};
		m_defaultCanvasBuffer = nullptr;

		m_pDX12Device = nullptr;
		m_pDX12CommandQueue = nullptr;
	}

	//____ Destructor ____________________________________________________________

	DX12Backend::~DX12Backend()
	{
		// Our resources must not be released while the GPU still uses them.

		waitForCompletion();

		// Anything recorded but never submitted. The GPU is done with the rest.

		m_heldResources.clear();

		// waitForCompletion() submitted any uploads we had recorded, so this should
		// be empty, but no surface may be left pointing at us.

		_abandonRecordedUploads();

		s_backends.erase(std::remove(s_backends.begin(), s_backends.end(), this), s_backends.end());

		// The last backend waits for the release callbacks still out, which the
		// GPU work just waited for has released or is releasing, before anyone
		// can let go of the device or unload our module.

		if (s_backends.empty())
			_exitReleaser();

		// Release the device if create(device, queue) set it for us.

		if (s_backends.empty() && s_bImplicitDevice)
			setDevice(nullptr, nullptr);

		if (m_fenceEvent)
			CloseHandle(m_fenceEvent);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12Backend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________________

	void DX12Backend::beginRender()
	{
		// With no device to render with, nothing gets recorded this frame. The
		// command list stays closed, which every recording path checks, and the
		// write pointers are cleared so nothing writes into last frame's buffers.
		//
		// Checking the device every frame is what catches a removal that no call
		// of ours has run into yet.

		if (!m_bDeviceObjectsOK || checkDeviceLost())
		{
			m_bCommandListOpen = false;
			m_buffers = FrameBuffers();
			return;
		}

		m_currentFrameIndex = (m_currentFrameIndex + 1) % c_nbFrameResources;
		int frame = m_currentFrameIndex;

		_waitForFence(m_frameResources[frame].fenceValue);

		// Buffers that edgemaps, Warpmaps and palettes have given back, which the GPU
		// is done with by now.

		DX12BufferPool::collect();

		m_frameResources[frame].commandAllocator->Reset();
		m_commandList->Reset(m_frameResources[frame].commandAllocator.Get(), nullptr);
		m_bCommandListOpen = true;
		_openedCommandList();

		// Vertices and colors are written for the whole frame, not per session,
		// since the GPU doesn't run any of it until the command list has been
		// executed. The buffers may not exist yet, beginSession() creates or grows
		// them as needed.

		auto& res = m_frameResources[frame];

		m_buffers.pVertexBeg = res.pVertexBufferData;
		m_buffers.pVertexEnd = res.pVertexBufferData ? res.pVertexBufferData + res.vertexCapacity : nullptr;
		m_buffers.pVertexPtr = res.pVertexBufferData;

		m_buffers.pColorBeg = res.pColorBufferData;
		m_buffers.pColorEnd = res.pColorBufferData ? res.pColorBufferData + res.colorCapacity : nullptr;
		m_buffers.pColorPtr = res.pColorBufferData;

		m_buffers.pExtrasBeg = res.pExtrasBufferData;
		m_buffers.pExtrasEnd = res.pExtrasBufferData ? res.pExtrasBufferData + res.extrasCapacity : nullptr;
		m_buffers.pExtrasPtr = res.pExtrasBufferData;

		m_frameResources[frame].nSRVDescriptors = 0;
		_forgetObjectSRVs();

		// Sessions un-park what they parked, so this should already be empty.

		m_blitSourceCanvases.clear();

		m_pActivePipeline = nullptr;		// Resetting the command list cleared its state.
		m_bBlitSourceBound = false;
	}

	//____ endRender() _________________________________________________________

	void DX12Backend::endRender()
	{
		if (!m_bCommandListOpen)
			return;

		_restoreBlitSourceCanvases();

		// Close() must not be inside the assert, or it is never called in release builds.

		HRESULT hr = m_commandList->Close();

		m_bCommandListOpen = false;

		// Lost during the frame, or the list broke. Executing would do nothing, and
		// signaling the fence would only fail.

		if (!CHECK_HR(hr, "ID3D12GraphicsCommandList::Close") || s_bDeviceLost)
		{
			_abandonRecordedUploads();
			return;
		}

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };

		 m_pDX12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		 // Signal and increment the fence value.

		m_fenceValue++;

		m_pDX12CommandQueue->Signal(m_commandFence.Get(), m_fenceValue);
		m_frameResources[m_currentFrameIndex].fenceValue = m_fenceValue;

		DX12BufferPool::submitted();

		_uploadsSubmitted();
		_releaseWhenDone(m_fenceValue);

	}

	//____ _flushCommandList() _________________________________________________
	//
	// Submits what we have recorded so far, so that waiting for it means anything,
	// then reopens the list. Resetting the list throws away all its state, so this
	// must only happen between sessions - beginSession() sets up everything again.

	void DX12Backend::_flushCommandList()
	{
		if (!m_bCommandListOpen)
			return;

		_restoreBlitSourceCanvases();

		HRESULT hr = m_commandList->Close();

		if (!CHECK_HR(hr, "ID3D12GraphicsCommandList::Close") || s_bDeviceLost)
		{
			m_bCommandListOpen = false;
			_abandonRecordedUploads();
			return;
		}

		ID3D12CommandList* pCommandLists[] = { m_commandList.Get() };
		m_pDX12CommandQueue->ExecuteCommandLists(1, pCommandLists);

		m_fenceValue++;

		m_pDX12CommandQueue->Signal(m_commandFence.Get(), m_fenceValue);
		m_frameResources[m_currentFrameIndex].fenceValue = m_fenceValue;

		DX12BufferPool::submitted();

		_uploadsSubmitted();
		_releaseWhenDone(m_fenceValue);

		// The textures the descriptors in the heap were copied for are no longer
		// held by the submission we are recording. The descriptors stay where they
		// are, since what we just submitted reads them.

		_forgetObjectSRVs();

		// The allocator keeps the memory of what we just submitted, it is only
		// reset in beginRender() once the fence says the GPU is done with it.

		m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(), nullptr);
		_openedCommandList();

		m_pActivePipeline = nullptr;
		m_bBlitSourceBound = false;

		// A flush can happen in the middle of a session, when a surface needs the
		// GPU to catch up, so put back everything the reset threw away.

		if (m_bInSession)
		{
			_bindSessionState();
			_bindCanvasState();
		}
	}

	//____ _bindSessionState() _________________________________________________
	//
	// Everything a session needs that a freshly reset command list doesn't have.

	void DX12Backend::_bindSessionState()
	{
		if (!m_bCommandListOpen)
			return;

		auto& frame = m_frameResources[m_currentFrameIndex];

		// Descriptor heaps must be set before any descriptor table is bound.

		if (frame.srvHeap && m_pSamplerHeap)
		{
			ID3D12DescriptorHeap* pHeaps[] = { frame.srvHeap.Get(), m_pSamplerHeap.Get() };
			m_commandList->SetDescriptorHeaps(2, pHeaps);
		}

		m_commandList->SetGraphicsRootSignature(m_pRootSignature.Get());

		// Colors and extras live in buffers the vertex shader indexes into, like
		// MetalBackend does. Root SRVs take the address directly.

		if (frame.colorBuffer)
			m_commandList->SetGraphicsRootShaderResourceView(1, frame.colorBuffer->GetGPUVirtualAddress());

		if (frame.extrasBuffer)
			m_commandList->SetGraphicsRootShaderResourceView(2, frame.extrasBuffer->GetGPUVirtualAddress());

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (frame.vertexBuffer)
		{
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
			vertexBufferView.BufferLocation = frame.vertexBuffer->GetGPUVirtualAddress();
			vertexBufferView.StrideInBytes = sizeof(Vertex);
			vertexBufferView.SizeInBytes = UINT(frame.vertexCapacity * sizeof(Vertex));

			m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView );
		}

		// Setting the root signature dropped all root arguments, the constants
		// among them.

		_setConstants(ConstantBlock::Canvas);
		_setConstants(ConstantBlock::Pixel);
		_setConstants(ConstantBlock::Warp);

		// And the pipeline and whatever the blit source had bound.

		m_pActivePipeline = nullptr;
		m_bBlitSourceBound = false;
	}

	//____ _bindCanvasState() __________________________________________________
	//
	// Points the command list at the canvas _bindCanvas() picked out.

	void DX12Backend::_bindCanvasState()
	{
		if (!m_bCommandListOpen || m_activeCanvasRTV.ptr == 0)
			return;								// Nothing to bind, or nowhere to bind it.

		m_commandList->OMSetRenderTargets(1, &m_activeCanvasRTV, FALSE, nullptr);

		// Viewport and scissor are in pixels. Canvas size is in spx for the default
		// canvas, so _bindCanvas() divided it by 64.

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (FLOAT) m_targetSize.w;
		viewport.Height = (FLOAT) m_targetSize.h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_commandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = (LONG) m_targetSize.w;
		scissorRect.bottom = (LONG) m_targetSize.h;
		m_commandList->RSSetScissorRects(1, &scissorRect);

		// The canvas scale the vertex shaders need is in the canvas constants, set
		// by GpuBackend after _bindCanvas() and by _bindSessionState() after a flush.

		// Pipelines are tied to the render target format, so the one we had is no
		// longer the one to use.

		m_pActivePipeline = nullptr;
	}

	//____ _restoreBlitSourceCanvases() ________________________________________
	//
	// Canvas surfaces we read as blit source were barriered into
	// PIXEL_SHADER_RESOURCE. They go back to COMMON before the list is closed, so
	// everything rests in COMMON between frames, which is what the copy queue and
	// D3D12's own promotion rules expect.

	void DX12Backend::_restoreBlitSourceCanvases()
	{
		if (!m_bCommandListOpen)
			return;

		// The textures were held by _bindBlitTexture(), so the surfaces can go.

		for (auto& pSurface : m_blitSourceCanvases)
		{
			auto pCanvas = static_cast<DX12Surface*>(pSurface.rawPtr());

			// The one we are rendering into belongs in RENDER_TARGET until the
			// session lets go of it, so leave that one alone.

			if (pCanvas != m_pCanvasSurface)
				_transitionCanvas(pCanvas, D3D12_RESOURCE_STATE_COMMON);
		}

		m_blitSourceCanvases.clear();
	}

	//____ _isDX12SurfaceType() _______________________________________________

	bool DX12Backend::_isDX12SurfaceType(const TypeInfo& type)
	{
		return _isOfType(type, DX12Surface::TYPEINFO);
	}

	//____ _isRecording() ________________________________________________________

	bool DX12Backend::_isRecording() const
	{
		return m_bCommandListOpen && !s_bDeviceLost;
	}

	//____ _isNativeSurface() ____________________________________________________

	bool DX12Backend::_isNativeSurface(const Object* pObject) const
	{
		return pObject && _isOfType(pObject->typeInfo(), DX12Surface::TYPEINFO);
	}

	//____ _isNativeEdgemap() ____________________________________________________

	bool DX12Backend::_isNativeEdgemap(const Object* pObject) const
	{
		return pObject && _isOfType(pObject->typeInfo(), DX12Edgemap::TYPEINFO);
	}

	//____ _isNativeWarpmap() ____________________________________________________

	bool DX12Backend::_isNativeWarpmap(const Object* pObject) const
	{
		return pObject && _isOfType(pObject->typeInfo(), DX12Warpmap::TYPEINFO);
	}

	//____ beginSession() _____________________________________________________
	//
	// GpuBackend does the work. We must not touch a single pixel outside the
	// update rects: the swap chain is presented with them as dirty rects, and
	// DXGI copies everything else from the previously presented buffer. Hence no
	// clear of the canvas.
	//
	// Set c_bDebugClearUpdateRects to make the updated areas stand out, which
	// also shows which parts of the canvas are being redrawn. Note that this
	// paints the whole update rect, which breaks the rule above, so the window
	// must present full frames while it is on.

	void DX12Backend::beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo)
	{
		// Every pending upload, so no draw of the session finds a texture out of
		// date. Whatever submitUploads() hasn't already sent. It may flush the
		// command list, which is fine before anything of the session is recorded.

		_recordUploads(-1);

		GpuBackend::beginSession(canvasRef, pCanvas, nUpdateRects, pUpdateRects, pInfo);

		if (c_bDebugClearUpdateRects && nUpdateRects > 0 && m_bInSession && m_bCommandListOpen && m_activeCanvasRTV.ptr != 0)
		{
			std::vector<D3D12_RECT> rects;
			rects.reserve(nUpdateRects);

			for (int i = 0; i < nUpdateRects; i++)
			{
				const RectSPX& rect = pUpdateRects[i];

				D3D12_RECT r;
				r.left = rect.x / 64;
				r.top = rect.y / 64;
				r.right = (rect.x + rect.w) / 64;
				r.bottom = (rect.y + rect.h) / 64;
				rects.push_back(r);
			}

			float clearColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
			m_commandList->ClearRenderTargetView(m_activeCanvasRTV, clearColor, (UINT) rects.size(), rects.data());
		}
	}

	//____ _beginSessionOnDevice() _______________________________________________
	//
	// The swap chain buffer is only ours while we render into it, and only if
	// this session renders into it at all, so it is claimed by _bindCanvas().

	void DX12Backend::_beginSessionOnDevice(Surface* pCanvas)
	{
		m_bSessionOnDefaultCanvas = false;
		m_pCanvasSurface = nullptr;
		m_activeCanvasRTV.ptr = 0;
		m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;
	}

	//____ _endSessionOnDevice() _________________________________________________

	void DX12Backend::_endSessionOnDevice()
	{
		// A canvas surface goes back to COMMON, where D3D12 promotes it on its own
		// when it is read as a blit source or copied from.

		if (m_pCanvasSurface)
		{
			_transitionCanvas(m_pCanvasSurface, D3D12_RESOURCE_STATE_COMMON);
			m_pCanvasSurface = nullptr;
		}

		// Nothing stays parked between sessions, or the next one would find a
		// surface in a state it doesn't expect.

		_restoreBlitSourceCanvases();

		if (m_bSessionOnDefaultCanvas)
			_transitionDefaultCanvas(D3D12_RESOURCE_STATE_RENDER_TARGET, m_defaultCanvasRestingState);

		m_bSessionOnDefaultCanvas = false;
		m_activeCanvasRTV.ptr = 0;
		m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;
	}

	//____ _transitionDefaultCanvas() ____________________________________________
	//
	// Nothing to do for a default canvas that rests in RENDER_TARGET, see
	// setDefaultCanvas().

	void DX12Backend::_transitionDefaultCanvas(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
	{
		if (!m_bCommandListOpen || !m_defaultCanvasBuffer || before == after)
			return;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_defaultCanvasBuffer;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &barrier);
	}

	//____ _transitionCanvas() ________________________________________________

	void DX12Backend::_transitionCanvas(DX12Surface* pCanvas, D3D12_RESOURCE_STATES state)
	{
		if (!m_bCommandListOpen || !pCanvas || !pCanvas->texture() || pCanvas->resourceState() == state)
			return;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pCanvas->texture();
		barrier.Transition.StateBefore = pCanvas->resourceState();
		barrier.Transition.StateAfter = state;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &barrier);

		pCanvas->setResourceState(state);
	}

	//____ _holdUntilDone() ___________________________________________________
	//
	// A command list keeps no references to the resources it mentions, and D3D12
	// must not have a resource released while work using it is pending. Surfaces
	// and edgemaps are only retained by DeviceGen2 until the session ends, which
	// is long before the GPU has run the commands, so we hold on to their D3D12
	// resources until the GPU is done, see _releaseWhenDone().

	void DX12Backend::_holdUntilDone(ID3D12Pageable* pObject)
	{
		if (!pObject)
			return;

		// The same canvas or blit source is often set many times per frame.

		if (!m_heldResources.empty() && m_heldResources.back().Get() == pObject)
			return;

		m_heldResources.emplace_back(pObject);
	}

	//____ _releaseWhenDone() __________________________________________________
	//
	// Called right after a submission has been followed by Signal(fenceValue).
	// Everything held so far goes to a thread pool wait that fires once, when
	// the fence reaches that value, and releases it on a pool thread. No thread
	// of our own, and nothing runs while the GPU is busy.
	//
	// Should any of that fail, we wait for the GPU here instead. Slower, but
	// never wrong.

	void DX12Backend::_releaseWhenDone(UINT64 fenceValue)
	{
		if (m_heldResources.empty())
			return;

		// A lost device runs nothing more, and if the GPU is already there we
		// don't need to wait at all.

		if (s_bDeviceLost || !m_commandFence || m_commandFence->GetCompletedValue() >= fenceValue)
		{
			m_heldResources.clear();
			return;
		}

		if (s_bReleaserReady)
		{
			auto pEntry = new PendingRelease;
			pEntry->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			PTP_WAIT pWait = nullptr;

			if (pEntry->hEvent && SUCCEEDED(m_commandFence->SetEventOnCompletion(fenceValue, pEntry->hEvent)))
				pWait = CreateThreadpoolWait(_onSubmissionDone, pEntry, &s_releaserEnv);

			if (pWait)
			{
				pEntry->resources.swap(m_heldResources);

				{
					std::lock_guard<std::mutex> lock(s_pendingMutex);
					s_pendingReleases.push_back(pEntry);
				}

				// Counted before the callback can possibly run. If the GPU got there
				// in the meantime the event is already set, and the wait fires at once.

				s_nPendingReleases++;
				SetThreadpoolWait(pWait, pEntry->hEvent, nullptr);
				return;
			}

			if (pEntry->hEvent)
				CloseHandle(pEntry->hEvent);
			delete pEntry;
		}

		_waitForFence(fenceValue);
		m_heldResources.clear();
	}

	//____ _onSubmissionDone() _________________________________________________
	//
	// Runs once on a thread pool thread when the GPU has finished a submission,
	// or when _flushPendingReleases() hurries it along. Only taking the entry off
	// the pending list needs the lock. Releasing D3D12 resources is thread safe.

	void CALLBACK DX12Backend::_onSubmissionDone(PTP_CALLBACK_INSTANCE pInstance, PVOID pContext, PTP_WAIT pWait, TP_WAIT_RESULT waitResult)
	{
		(void) pInstance;
		(void) waitResult;					// Always signaled, we set no timeout.

		auto pEntry = static_cast<PendingRelease*>(pContext);

		{
			std::lock_guard<std::mutex> lock(s_pendingMutex);
			s_pendingReleases.erase(std::remove(s_pendingReleases.begin(), s_pendingReleases.end(), pEntry), s_pendingReleases.end());
		}

		pEntry->resources.clear();
		CloseHandle(pEntry->hEvent);
		delete pEntry;

		CloseThreadpoolWait(pWait);			// Freed once this callback has returned.

		s_nPendingReleases--;				// Last, _exitReleaser() goes by it.
	}

	//____ _initReleaser() _____________________________________________________
	//
	// The callbacks run on the process's default thread pool. The environment
	// names our module as the callbacks' library, which keeps it loaded while one
	// of them runs, so a host unloading us can't pull the code from under it.

	bool DX12Backend::_initReleaser()
	{
		if (s_bReleaserReady)
			return true;

		InitializeThreadpoolEnvironment(&s_releaserEnv);

		HMODULE hModule = nullptr;
		if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
							   reinterpret_cast<LPCWSTR>(&_onSubmissionDone), &hModule))
		{
			SetThreadpoolCallbackLibrary(&s_releaserEnv, hModule);
		}

		s_nPendingReleases = 0;
		s_bReleaserReady = true;
		return true;
	}

	//____ _exitReleaser() _____________________________________________________
	//
	// Called by the last backend's destructor, after it has waited for the GPU.
	// The callback library keeps our module loaded for any callback still
	// running.

	void DX12Backend::_exitReleaser()
	{
		if (!s_bReleaserReady)
			return;

		_flushPendingReleases();

		DestroyThreadpoolEnvironment(&s_releaserEnv);
		s_bReleaserReady = false;
	}

	//____ _flushPendingReleases() ______________________________________________
	//
	// For when the GPU is done with every submission, or the device is lost:
	// sets the events the release callbacks wait for ourselves, then waits for
	// the callbacks to finish. On a lost device the fence may never set them,
	// and what they hold would keep the old device alive, which stops a new one
	// from being created on the same adapter.
	//
	// The wait is bounded all the same: better to leave a few resources
	// unreleased than to hang the host.

	void DX12Backend::_flushPendingReleases()
	{
		{
			std::lock_guard<std::mutex> lock(s_pendingMutex);

			for (auto pEntry : s_pendingReleases)
				SetEvent(pEntry->hEvent);
		}

		ULONGLONG deadline = GetTickCount64() + 2000;

		while (s_nPendingReleases > 0 && GetTickCount64() < deadline)
			Sleep(1);
	}

	//____ _bindCanvas() ______________________________________________________
	//
	// Points the command list at the canvas we should render into from here on.
	// A null surface means the default canvas, which the window owns.

	bool DX12Backend::_bindCanvas(Surface* pCanvas, SizeI& size, CanvasKind& kind)
	{
		DX12Surface* pSurface = static_cast<DX12Surface*>(pCanvas);

		if (pSurface && (!pSurface->texture() || !pSurface->canBeCanvas() || pSurface->renderTargetView().ptr == 0))
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Surface can not be used as canvas, rendering to default canvas instead.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			pSurface = nullptr;
		}

		// Getting here can flush the command list, and a flush rebinds the canvas.
		// Make sure that finds nothing to bind rather than the old canvas, which
		// may be on its way out of existence.

		m_activeCanvasRTV.ptr = 0;
		m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;
		m_targetSize = SizeI();

		// Whatever we rendered into before is done now.

		if (m_pCanvasSurface && m_pCanvasSurface != pSurface)
			_transitionCanvas(m_pCanvasSurface, D3D12_RESOURCE_STATE_COMMON);

		m_pCanvasSurface = nullptr;

		if (!m_bCommandListOpen)
			return false;

		if (pSurface)
		{
			// Gets any pixels the CPU wrote into the texture before we render over
			// it, and marks our copy of them as the older one from here on. The
			// upload barriers the texture out of COMMON and back, so the surface
			// goes there first. It usually already is, in which case the call
			// below records nothing. beginSession() has normally taken care of the
			// upload already.

			_transitionCanvas(pSurface, D3D12_RESOURCE_STATE_COMMON);

			_recordUpload(pSurface);
			pSurface->notifyRendered();

			if (!m_bCommandListOpen)
				return false;

			_transitionCanvas(pSurface, D3D12_RESOURCE_STATE_RENDER_TARGET);

			// Whoever owns the canvas may let go of it as soon as the session has
			// ended - RootPanel does when the window is resized - while the GPU is
			// still rendering into it.

			_holdUntilDone(pSurface->texture());

			m_pCanvasSurface = pSurface;
			m_activeCanvasRTV = pSurface->renderTargetView();
			m_activeCanvasFormat = pSurface->dxgiFormat();
			m_targetSize = pSurface->pixelSize();
		}
		else
		{
			if (m_defaultCanvasRTV.ptr == 0)
				return false;

			// A session that started on a surface canvas has not claimed the swap
			// chain buffer yet. _endSessionOnDevice() hands it back.

			if (!m_bSessionOnDefaultCanvas && m_defaultCanvasBuffer)
			{
				_transitionDefaultCanvas(m_defaultCanvasRestingState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				m_bSessionOnDefaultCanvas = true;
			}

			m_activeCanvasRTV = m_defaultCanvasRTV;
			m_activeCanvasFormat = m_defaultCanvasFormat;
			m_targetSize = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };
		}

		_bindCanvasState();

		size = m_targetSize;
		kind = _canvasKindOfFormat(m_activeCanvasFormat);
		return true;
	}

	//____ setObjects() ________________________________________________________

	void DX12Backend::setObjects(Object* const* pBeg, Object* const* pEnd)
	{
		GpuBackend::setObjects(pBeg, pEnd);

		// A new table, so nothing has been copied for it yet. The descriptors of the
		// previous one stay in the heap, earlier draws read them.

		m_objectSRVs.assign(pEnd - pBeg, SRVSlot());
	}

	//____ _prepareBlitSource() ________________________________________________

	void DX12Backend::_prepareBlitSource(Surface* pSurface)
	{
		// Normally beginSession() has already recorded the upload and this does nothing.

		_recordUpload(static_cast<DX12Surface*>(pSurface));
	}

	//____ _bindBlitTexture() __________________________________________________
	//
	// Copies the source's descriptor into this frame's shader visible heap and
	// points the root descriptor table at it, and binds its palette if it has one.
	// GpuBackend binds the sampler and sets the constants.

	bool DX12Backend::_bindBlitTexture(Surface* pSurface)
	{
		auto pSource = static_cast<DX12Surface*>(pSurface);

		if (!m_bCommandListOpen || !pSource->texture())
			return false;

		// Mip levels that no longer match level 0 are drawn anew before anything
		// reads them.

		if (pSource->isMipmapStale())
			_generateMipmaps(pSource);

		// D3D12 would promote a texture resting in COMMON to a shader resource on
		// its own, but a canvas surface's state is one we track, so we do it with
		// a barrier and put it back before the command list closes.

		if (pSource->canBeCanvas() && pSource->resourceState() == D3D12_RESOURCE_STATE_COMMON)
		{
			_transitionCanvas(pSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_blitSourceCanvases.push_back(pSource);
		}

		// An upload into it, later in this list, has to know it has been promoted.

		pSource->m_lastReadInList = m_listSerial;

		auto& frame = m_frameResources[m_currentFrameIndex];

		if (!frame.srvHeap || !m_pSamplerHeap)
			return false;

		// The source's descriptor is copied into the heap the first time its entry
		// in the object table is used, after that we point at the copy, see
		// m_objectSRVs.

		ID3D12Resource* pTexture = pSource->texture();
		D3D12_CPU_DESCRIPTOR_HANDLE srcSRV = pSource->textureSRV();

		SRVSlot dummy;
		SRVSlot& entry = m_blitSourceObject < m_objectSRVs.size() ? m_objectSRVs[m_blitSourceObject] : dummy;

		int slot;

		if (entry.slot >= 0 && entry.srv == srcSRV.ptr)
			slot = entry.slot;
		else
		{
			if (frame.nSRVDescriptors >= frame.srvCapacity && !_growSRVHeap())
			{
				if (!m_bOutOfDescriptorsReported && !s_bDeviceLost)
				{
					Base::reportError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Out of blit source descriptors and couldn't make room for more, blits dropped.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
					m_bOutOfDescriptorsReported = true;
				}

				return false;
			}

			// Ordinary surfaces need holding as much as canvases do: a temporary
			// surface is released by DeviceGen2 when the session ends. Once is
			// enough, m_objectSRVs is forgotten with every submission.

			_holdUntilDone(pTexture);

			slot = frame.nSRVDescriptors++;

			D3D12_CPU_DESCRIPTOR_HANDLE dest = frame.srvHeap->GetCPUDescriptorHandleForHeapStart();
			dest.ptr += SIZE_T(slot) * m_srvDescriptorSize;

			m_pDX12Device->CopyDescriptorsSimple(1, dest, srcSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			entry.srv = srcSRV.ptr;
			entry.slot = slot;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = frame.srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += UINT64(slot) * m_srvDescriptorSize;

		m_commandList->SetGraphicsRootDescriptorTable(3, srvHandle);

		// A palette based source has its palette in the slot edgemaps use, which is
		// free while we blit. Drawing an edgemap clears m_bBlitSourceBound, so we come
		// back here and put it back afterwards.

		if (pSource->isIndexed())
		{
			D3D12_GPU_VIRTUAL_ADDRESS palette = pSource->paletteGPUAddress();

			if (palette == 0)
				return false;

			m_commandList->SetGraphicsRootShaderResourceView(5, palette);
		}

		return true;
	}

	//____ _generateMipmaps() __________________________________________________
	//
	// Draws each mip level of the surface from the one above it, the way
	// MetalBackend has Metal do it and GlBackend has OpenGL. D3D12 has no such
	// call, so we draw them ourselves, see ps_mipmap.hlsl.
	//
	// We get here from _bindBlitTexture() when level 0 has changed since the
	// levels were drawn: after an upload, which _recordUploadBatch() ends with a
	// barrier back to COMMON, in a fresh command list if this one had read the
	// surface, or after rendering into the surface, which ended with it back in
	// COMMON. Either way every level rests in COMMON and this command list hasn't
	// promoted it, so we can barrier it ourselves. We leave it in COMMON, where
	// _bindBlitTexture() expects it.

	void DX12Backend::_generateMipmaps(DX12Surface* pSurface)
	{
		auto& frame = m_frameResources[m_currentFrameIndex];

		int nLevels = pSurface->mipLevels();
		ID3D12Resource* pTexture = pSurface->texture();

		if (!m_bCommandListOpen || !pTexture || nLevels < 2 || !frame.srvHeap || !m_pSamplerHeap)
			return;

		ID3D12PipelineState* pPipeline = _mipmapPipeline(pSurface->dxgiFormat());

		if (!pPipeline)
		{
			pSurface->clearMipmapStale();		// No use trying again, level 0 alone is what we have.
			return;
		}

		if (frame.nSRVDescriptors + nLevels - 1 > frame.srvCapacity && !_growSRVHeap())
			return;								// Stays stale, so the next frame tries again.

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &barrier);

		m_commandList->SetPipelineState(pPipeline);

		// Bilinear and clamped, see _createSamplers().

		D3D12_GPU_DESCRIPTOR_HANDLE samplerHandle = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
		samplerHandle.ptr += UINT64(1) * m_samplerDescriptorSize;

		m_commandList->SetGraphicsRootDescriptorTable(4, samplerHandle);

		SizeI size = pSurface->pixelSize();

		for (int level = 1; level < nLevels; level++)
		{
			// Subresource index is the mip level, since there is one array slice and one plane.

			barrier.Transition.Subresource = level;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			m_commandList->ResourceBarrier(1, &barrier);

			D3D12_CPU_DESCRIPTOR_HANDLE dest = frame.srvHeap->GetCPUDescriptorHandleForHeapStart();
			dest.ptr += SIZE_T(frame.nSRVDescriptors) * m_srvDescriptorSize;

			m_pDX12Device->CopyDescriptorsSimple(1, dest, pSurface->mipSourceSRV(level), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = frame.srvHeap->GetGPUDescriptorHandleForHeapStart();
			srvHandle.ptr += UINT64(frame.nSRVDescriptors) * m_srvDescriptorSize;

			frame.nSRVDescriptors++;

			m_commandList->SetGraphicsRootDescriptorTable(3, srvHandle);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv = pSurface->mipRTV(level);
			m_commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

			LONG w = (size.w >> level) > 0 ? (size.w >> level) : 1;
			LONG h = (size.h >> level) > 0 ? (size.h >> level) : 1;

			D3D12_VIEWPORT viewport = { 0.f, 0.f, (FLOAT) w, (FLOAT) h, 0.f, 1.f };
			D3D12_RECT scissorRect = { 0, 0, w, h };

			m_commandList->RSSetViewports(1, &viewport);
			m_commandList->RSSetScissorRects(1, &scissorRect);

			m_commandList->DrawInstanced(3, 1, 0, 0);

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			m_commandList->ResourceBarrier(1, &barrier);
		}

		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		m_commandList->ResourceBarrier(1, &barrier);

		pSurface->clearMipmapStale();

		// Put back the canvas, and have pipeline and blit source bound anew.

		m_pActivePipeline = nullptr;
		m_bBlitSourceBound = false;

		if (m_activeCanvasRTV.ptr != 0)
			_bindCanvasState();
		else
			m_commandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
	}

	//____ _mipmapPipeline() ___________________________________________________
	//
	// Created on first use for each format and kept, like the other pipelines.
	// No blending, no vertex input: the vertex shader makes its triangle from
	// SV_VertexID.

	ID3D12PipelineState* DX12Backend::_mipmapPipeline(DXGI_FORMAT format)
	{
		auto it = m_mipmapPipelines.find(format);
		if (it != m_mipmapPipelines.end())
			return it->second.Get();

		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipeline;

		D3D12_SHADER_BYTECODE vertexShader = _shader("mipmapVS");
		D3D12_SHADER_BYTECODE pixelShader = _shader("mipmapPS");

		if (vertexShader.pShaderBytecode && pixelShader.pShaderBytecode)
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
			desc.pRootSignature = m_pRootSignature.Get();
			desc.VS = vertexShader;
			desc.PS = pixelShader;

			// Zero is not a valid blend value, even with blending off.

			for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
			{
				auto& rt = desc.BlendState.RenderTarget[i];

				rt.BlendEnable = false;
				rt.LogicOpEnable = false;
				rt.SrcBlend = D3D12_BLEND_ONE;
				rt.DestBlend = D3D12_BLEND_ZERO;
				rt.BlendOp = D3D12_BLEND_OP_ADD;
				rt.SrcBlendAlpha = D3D12_BLEND_ONE;
				rt.DestBlendAlpha = D3D12_BLEND_ZERO;
				rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				rt.LogicOp = D3D12_LOGIC_OP_NOOP;
				rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			}

			desc.SampleMask = 0xFFFFFFFF;
			desc.SampleDesc = { 1, 0 };

			desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			desc.RasterizerState.FrontCounterClockwise = false;
			desc.RasterizerState.DepthClipEnable = true;
			desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			desc.DepthStencilState.DepthEnable = false;
			desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

			desc.InputLayout = { nullptr, 0 };
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			desc.NumRenderTargets = 1;
			desc.RTVFormats[0] = format;
			desc.NodeMask = 0;
			desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			if (!CHECK_HR(m_pDX12Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pPipeline.GetAddressOf())), "CreateGraphicsPipelineState"))
				pPipeline = nullptr;
		}

		m_mipmapPipelines[format] = pPipeline;			// Kept also when null, so a failure is only reported once.
		return pPipeline.Get();
	}

	//____ _bindBlitSampler() _________________________________________________

	void DX12Backend::_bindBlitSampler(Surface* pSurface, bool bClip)
	{
		if (!m_bCommandListOpen || !m_pSamplerHeap)
			return;

		// Samplers are ordered nearest/bilinear, clamp/tile, then clip, see _createSamplers().

		int index = (bClip ? 4 : 0) + (pSurface->isTiling() ? 2 : 0) + (pSurface->sampleMethod() == SampleMethod::Bilinear ? 1 : 0);

		D3D12_GPU_DESCRIPTOR_HANDLE samplerHandle = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
		samplerHandle.ptr += UINT64(index) * m_samplerDescriptorSize;

		m_commandList->SetGraphicsRootDescriptorTable(4, samplerHandle);
	}

	//____ _createSRVHeap() _____________________________________________________
	//
	// The shader visible heap blit source descriptors are copied into. One per
	// frame resource, since the GPU may still be reading the other frame's.

	bool DX12Backend::_createSRVHeap(int frameIndex, int nDescriptors)
	{
		auto& frame = m_frameResources[frameIndex];

		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = UINT(nDescriptors);
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;

		if (!CHECK_HR(m_pDX12Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(pHeap.GetAddressOf())), "CreateDescriptorHeap"))
			return false;

		setObjectName(this, pHeap.Get(), L"SRV heap");

		frame.srvHeap = pHeap;
		frame.srvCapacity = nDescriptors;
		frame.nSRVDescriptors = 0;
		return true;
	}

	//____ _growSRVHeap() _______________________________________________________
	//
	// Replaces the current frame's heap with an empty one at least twice the
	// size, and with room for nNeeded, like the vertex buffers grow. Draws
	// recorded so far read their descriptors from the old heap, so it is held
	// until the GPU is done with them.
	//
	// Switching heaps leaves the descriptor tables set so far pointing into a
	// heap that is no longer bound. That is fine: _reserveFrameBuffers() grows it
	// before _bindSessionState() binds anything, and _bindBlitTexture() and
	// _generateMipmaps() set both tables right after. Nothing else uses them.

	bool DX12Backend::_growSRVHeap(int nNeeded)
	{
		auto& frame = m_frameResources[m_currentFrameIndex];

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pOldHeap = frame.srvHeap;

		int newCapacity = frame.srvCapacity * 2;
		if (newCapacity < nNeeded)
			newCapacity = nNeeded;

		if (!_createSRVHeap(m_currentFrameIndex, newCapacity))
			return false;

		_holdUntilDone(pOldHeap.Get());

		// The descriptors we had copied are all in the old heap.

		_forgetObjectSRVs();

		if (m_bCommandListOpen)
		{
			ID3D12DescriptorHeap* pHeaps[] = { frame.srvHeap.Get(), m_pSamplerHeap.Get() };
			m_commandList->SetDescriptorHeaps(2, pHeaps);
		}

		return true;
	}

	//____ _forgetObjectSRVs() ____________________________________________________

	void DX12Backend::_forgetObjectSRVs()
	{
		for (auto& entry : m_objectSRVs)
			entry = SRVSlot();
	}

	//____ _bindEdgemap() _____________________________________________________

	bool DX12Backend::_bindEdgemap(GpuEdgemap* pEdgemap)
	{
		auto pDX12Edgemap = static_cast<DX12Edgemap*>(pEdgemap);

		if (!m_bCommandListOpen || pDX12Edgemap->_gpuAddress() == 0)
			return false;

		// The command list mentions the edgemap's buffer but keeps no reference to
		// it, and DeviceGen2 lets go of the edgemap when the session ends. A
		// waveform drawn from a temporary is still safe: DX12BufferPool doesn't hand
		// out the memory of a buffer again until the GPU is done with what we record.

		m_commandList->SetGraphicsRootShaderResourceView(5, pDX12Edgemap->_gpuAddress());
		return true;
	}

	//____ _drawWarpmap() _____________________________________________________
	//
	// The triangles of the Warpmap, from its own buffer, see DX12Warpmap.

	void DX12Backend::_drawWarpmap(Warpmap* pWarpmap, const RectI* pPatches, int nPatches)
	{
		auto pDX12Warpmap = static_cast<DX12Warpmap*>(pWarpmap);

		if (!m_bCommandListOpen || pWarpmap->triangleCount() == 0 || nPatches <= 0 || !pDX12Warpmap->_prepare())
			return;

		m_commandList->SetGraphicsRootShaderResourceView(7, pDX12Warpmap->_gpuAddress());

		D3D12_INDEX_BUFFER_VIEW indexBufferView = pDX12Warpmap->_indexBufferView();
		m_commandList->IASetIndexBuffer(&indexBufferView);

		for (int i = 0; i < nPatches; i++)
		{
			_setScissor(pPatches[i]);
			m_commandList->DrawIndexedInstanced(UINT(pWarpmap->triangleCount() * 3), 1, 0, 0, 0);
		}

		_resetScissor();
	}

	//____ _bindPipeline() _____________________________________________________
	//
	// Pipelines are created the first time a combination of canvas format, kind
	// and blend mode shows up, as the canvas formats in use are not known until
	// canvases are set.

	bool DX12Backend::_bindPipeline(PipelineKind kind, BlendMode blendMode)
	{
		if (!m_bCommandListOpen || m_activeCanvasRTV.ptr == 0 || m_activeCanvasFormat == DXGI_FORMAT_UNKNOWN)
			return false;

		uint64_t key = _pipelineKey(m_activeCanvasFormat, m_activeCanvasKind, kind, blendMode);

		auto it = m_pipelines.find(key);
		if (it == m_pipelines.end())
		{
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipeline;

			if (!_createPipeline(kind, _blendState(kind, blendMode, m_activeCanvasKind), m_activeCanvasFormat, pPipeline))
				pPipeline = nullptr;

			it = m_pipelines.emplace(key, pPipeline).first;		// Also when null, so a failure is only tried once.
		}

		ID3D12PipelineState* pPipeline = it->second.Get();

		if (!pPipeline)
			return false;

		if (pPipeline != m_pActivePipeline)
		{
			m_commandList->SetPipelineState(pPipeline);
			m_pActivePipeline = pPipeline;
		}

		return true;
	}

	//____ _setConstants() _____________________________________________________
	//
	// Root constants are recorded into the command list, so every draw after this
	// sees the new values and none before it does. A reset list has lost them,
	// which is why _bindSessionState() sets them all again.

	void DX12Backend::_setConstants(ConstantBlock block)
	{
		if (!m_bCommandListOpen)
			return;

		static_assert(sizeof(CanvasConstants) == 8 * 4 && sizeof(PixelConstants) == 4 * 4 && sizeof(WarpConstants) == 8 * 4,
					  "Constant blocks must match constants.hlsl and the root signature");

		switch (block)
		{
			case ConstantBlock::Canvas:
				m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(CanvasConstants) / 4, &m_canvasConstants, 0);
				break;

			case ConstantBlock::Pixel:
				m_commandList->SetGraphicsRoot32BitConstants(6, sizeof(PixelConstants) / 4, &m_pixelConstants, 0);
				break;

			case ConstantBlock::Warp:
				m_commandList->SetGraphicsRoot32BitConstants(8, sizeof(WarpConstants) / 4, &m_warpConstants, 0);
				break;
		}
	}

	//____ _draw() ______________________________________________________________

	void DX12Backend::_draw(int firstVertex, int nVertices)
	{
		if (!m_bCommandListOpen || nVertices <= 0)
			return;

		m_commandList->DrawInstanced(UINT(nVertices), 1, UINT(firstVertex), 0);
	}

	//____ _setScissor() ________________________________________________________

	void DX12Backend::_setScissor(const RectI& rect)
	{
		if (!m_bCommandListOpen)
			return;

		D3D12_RECT scissorRect = { LONG(rect.x), LONG(rect.y), LONG(rect.x + rect.w), LONG(rect.y + rect.h) };
		m_commandList->RSSetScissorRects(1, &scissorRect);
	}

	//____ _resetScissor() ______________________________________________________
	//
	// Back to the whole canvas, which is what everything else expects.

	void DX12Backend::_resetScissor()
	{
		if (!m_bCommandListOpen)
			return;

		D3D12_RECT fullRect = { 0, 0, LONG(m_targetSize.w), LONG(m_targetSize.h) };
		m_commandList->RSSetScissorRects(1, &fullRect);
	}

	//____ setDefaultCanvas() ___________________________________________

	bool DX12Backend::setDefaultCanvas(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, ID3D12Resource* renderTargetBuffer,
									   DXGI_FORMAT renderTargetFormat, SizeSPX size, int scale,
									   D3D12_RESOURCE_STATES restingState)
	{
		m_defaultCanvasRTV = renderTargetView;
		m_defaultCanvasBuffer = renderTargetBuffer;
		m_defaultCanvasFormat = renderTargetFormat;
		m_defaultCanvasRestingState = restingState;
		m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;

		// Wax has no name for R8G8B8A8, the usual swap chain format, so it is
		// reported as ARGB_8. Channel order is the one thing about the default canvas
		// nobody can ask us, and the color space, which they can, is then right.

		switch (renderTargetFormat)
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
				m_defaultCanvas.format = PixelFormat::ARGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::Linear;
				break;

			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				m_defaultCanvas.format = PixelFormat::ARGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::sRGB;
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM:
				m_defaultCanvas.format = PixelFormat::XRGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::Linear;
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				m_defaultCanvas.format = PixelFormat::XRGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::sRGB;
				break;

			default:
				m_defaultCanvas.format = PixelFormat::Undefined;
				break;
		}

		return true;
	}

	//____ canvasInfo() ________________________________________________________

	const CanvasInfo* DX12Backend::canvasInfo(CanvasRef ref) const
	{
		if (ref == CanvasRef::Default)
			return &m_defaultCanvas;
		else
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only Default canvas is supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
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

	//____ warpmapFactory() _____________________________________________________

	WarpmapFactory_p DX12Backend::warpmapFactory()
	{
		if (!m_pWarpmapFactory)
			m_pWarpmapFactory = DX12WarpmapFactory::create();

		return m_pWarpmapFactory;
	}

	//____ canBeBlitSource() __________________________________________________

	bool DX12Backend::canBeBlitSource(const TypeInfo& type) const
	{
		return _isDX12SurfaceType(type);
	}

	//____ canBeCanvas() ________________________________________________________

	bool DX12Backend::canBeCanvas(const TypeInfo& type) const
	{
		return _isDX12SurfaceType(type);
	}

	//____ waitForCompletion() __________________________________________________

	void DX12Backend::waitForCompletion()
	{
		// Waiting for work that is still being recorded would return at once and
		// mean nothing, so what we have goes to the GPU first.

		_flushCommandList();

		// Wait for the last value signaled, not just the current frame slot's,
		// so work from both in-flight frames is done.

		_waitForFence(m_fenceValue);
	}

	//____ _waitForFence() _____________________________________________________

	void DX12Backend::_waitForFence(UINT64 fenceValue)
	{
		// A lost device runs nothing more. Its fences are meant to read as
		// complete, but we don't bet an INFINITE wait on every driver doing so.

		if (s_bDeviceLost)
			return;

		if (m_commandFence && m_commandFence->GetCompletedValue() < fenceValue)
		{
			m_commandFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	//____ submitUploads() _____________________________________________________

	int64_t DX12Backend::submitUploads(int64_t maxBytes)
	{
		if (m_bInSession)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::IllegalCall, "submitUploads() can't be called during a session.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return DX12Surface::pendingUploadBytes();
		}

		// Nothing can be recorded without device objects or on a lost device. The
		// uploads stay pending until a new device is set, which redoes them anyway.

		if (!DX12Surface::s_pFirstPending || !m_bDeviceObjectsOK || s_bDeviceLost)
			return DX12Surface::pendingUploadBytes();

		if (m_bCommandListOpen)
		{
			// Within a frame, between sessions. Submitting what we have so far is
			// allowed here, the next session sets everything up again.

			_recordUploads(maxBytes);
			_flushCommandList();
		}
		else
		{
			// A command list of our own. beginRender() waits for the oldest frame in
			// flight to free its allocator, which is what keeps a caller looping
			// over us from piling up work on the GPU.

			beginRender();
			_recordUploads(maxBytes);
			endRender();
		}

		return DX12Surface::pendingUploadBytes();
	}

	//____ pendingUploadBytes() ________________________________________________

	int64_t DX12Backend::pendingUploadBytes() const
	{
		return DX12Surface::pendingUploadBytes();
	}

	//____ _recordUploads() ____________________________________________________
	//
	// Records the uploads of pending surfaces into our open command list, oldest
	// first, until maxBytes is reached. Always at least one, so a surface larger
	// than maxBytes doesn't block the rest. A negative maxBytes records them all.
	//
	// Only between sessions, or at the start of one, since it may have to flush
	// the command list. See _recordUploadBatch().

	int64_t DX12Backend::_recordUploads(int64_t maxBytes)
	{
		if (!m_bCommandListOpen || s_bDeviceLost || !DX12Surface::s_pFirstPending)
			return 0;

		int64_t bytes = 0;

		for (DX12Surface* p = DX12Surface::s_pFirstPending; p; p = p->m_pNextPending)
		{
			if (maxBytes >= 0 && !m_uploadBatch.empty() && bytes + p->m_pendingBytes > maxBytes)
				break;

			m_uploadBatch.push_back(p);
			bytes += p->m_pendingBytes;
		}

		_recordUploadBatch();
		return bytes;
	}

	//____ _recordUpload() _____________________________________________________
	//
	// For a surface about to be read or rendered into that is still pending. That
	// shouldn't happen, beginSession() records all uploads before anything of the
	// session, but it is cheap to make sure.

	void DX12Backend::_recordUpload(DX12Surface* pSurface)
	{
		if (!pSurface->m_bUploadPending || !m_bCommandListOpen || s_bDeviceLost)
			return;

		m_uploadBatch.push_back(pSurface);
		_recordUploadBatch();
	}

	//____ _recordUploadBatch() ________________________________________________
	//
	// Records the copies of everything in m_uploadBatch, with their barriers in
	// one call before and one after. Textures go into COPY_DEST from whatever state
	// we know them in, which is COMMON for anything but a canvas we track, and come
	// out in COMMON, since COPY_DEST doesn't decay.
	//
	// No wait on the CPU: the command queue is shared by all backends and runs in
	// order, so the copies come after whatever was submitted before and in front
	// of what we record after them.

	void DX12Backend::_recordUploadBatch()
	{
		// Recorded but unsubmitted work of other backends might read the textures
		// we are about to change. It goes in front of our copies, as it would have
		// had it been submitted already.

		for (auto pBackend : s_backends)
		{
			if (pBackend != this && pBackend->m_bCommandListOpen)
				pBackend->_flushCommandList();
		}

		// A texture we have already read from in this list was promoted out of
		// COMMON by D3D12, into a state we don't know. It decays when the list has
		// run, so the upload goes into a fresh one. Canvases are tracked, so their
		// state is always known.

		for (auto p : m_uploadBatch)
		{
			if (!p->canBeCanvas() && p->m_lastReadInList == m_listSerial)
			{
				_flushCommandList();
				break;
			}
		}

		if (!m_bCommandListOpen || s_bDeviceLost)
		{
			m_uploadBatch.clear();		// The flush failed. They stay pending.
			return;
		}

		// Pixels of their own that have changed go into buffers the GPU copies from.

		_stageUploadBatch();

		m_uploadBarriers.clear();

		for (auto p : m_uploadBatch)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = p->m_texture.Get();
			barrier.Transition.StateBefore = p->m_resourceState;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_uploadBarriers.push_back(barrier);
		}

		m_commandList->ResourceBarrier((UINT) m_uploadBarriers.size(), m_uploadBarriers.data());

		for (auto p : m_uploadBatch)
		{
			p->_recordPulls(m_commandList.Get());

			// The surface may go before the GPU has copied, taking the texture with
			// it, and lets go of what is copied from here.

			_holdUntilDone(p->m_texture.Get());

			for (auto& pull : p->m_pulls)
				_holdUntilDone(pull.buffer.Get());

			p->m_pulls.clear();
		}

		for (auto& barrier : m_uploadBarriers)
		{
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		}

		m_commandList->ResourceBarrier((UINT) m_uploadBarriers.size(), m_uploadBarriers.data());

		for (auto p : m_uploadBatch)
		{
			p->m_resourceState = D3D12_RESOURCE_STATE_COMMON;

			// Level 0 has changed, so the others are stale. They are drawn from it
			// before the surface is read, see _generateMipmaps().

			p->_uploaded();

			// Told if we never get to submit, see _abandonRecordedUploads().

			if (p->m_pUploadRecordedBy != this)
			{
				p->m_pUploadRecordedBy = this;
				m_recordedUploads.push_back(p);
			}
		}

		m_uploadBatch.clear();

		// A canvas among them may have been our blit source, and is in COMMON now.

		m_bBlitSourceBound = false;
	}

	//____ _stageUploadBatch() _________________________________________________
	//
	// Copies the pixels the surfaces of m_uploadBatch have to send into buffers
	// the GPU copies from, premultiplied if need be. One buffer for as many
	// surfaces as fit: making a buffer takes far longer than filling a small one,
	// and D3D12 makes none smaller than 64 KB. It is held until the GPU has
	// copied from it and released then.
	//
	// A surface whose buffer couldn't be made keeps what it has to send, and
	// stays pending.

	void DX12Backend::_stageUploadBatch()
	{
		size_t first = 0;

		while (first < m_uploadBatch.size())
		{
			// As many surfaces as stay under the limit, at least one.

			UINT64 bytes = 0;
			size_t end = first;

			for (; end < m_uploadBatch.size(); end++)
			{
				UINT64 surfaceBytes = m_uploadBatch[end]->_dirtyStagingBytes();

				if (bytes > 0 && bytes + surfaceBytes > c_maxStagingBufferBytes)
					break;

				bytes += surfaceBytes;
			}

			if (bytes > 0)
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
				uint8_t* pMapped = nullptr;
				D3D12_RANGE noRead = { 0, 0 };			// Write-combined, we only write.

				_createBuffer(buffer, int(bytes), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, L"Wax DX12Backend: staging buffer");

				if (buffer && SUCCEEDED(buffer->Map(0, &noRead, (void**) &pMapped)))
				{
					UINT64 offset = 0;

					for (size_t i = first; i < end; i++)
					{
						UINT64 surfaceBytes = m_uploadBatch[i]->_dirtyStagingBytes();

						if (surfaceBytes > 0)
						{
							m_uploadBatch[i]->_stageDirtyInto(buffer.Get(), pMapped, offset);
							offset += surfaceBytes;
						}
					}

					buffer->Unmap(0, nullptr);
				}
			}

			first = end;
		}
	}

	//____ _uploadsSubmitted() _________________________________________________
	//
	// The list holding the uploads went to the GPU, so they are no longer ours to
	// tell the surfaces of.

	void DX12Backend::_uploadsSubmitted()
	{
		for (auto p : m_recordedUploads)
			p->m_pUploadRecordedBy = nullptr;

		m_recordedUploads.clear();
	}

	//____ _abandonRecordedUploads() ___________________________________________
	//
	// The list holding the uploads was closed without being submitted, so their
	// textures never got the pixels. They go back on the list, whole, which on a
	// lost device does nothing until a new device replaces their textures anyway.

	void DX12Backend::_abandonRecordedUploads()
	{
		for (auto p : m_recordedUploads)
			p->_uploadAbandoned();

		m_recordedUploads.clear();
	}

	//____ _forgetRecordedUpload() _____________________________________________

	void DX12Backend::_forgetRecordedUpload(DX12Surface* pSurface)
	{
		m_recordedUploads.erase(std::remove(m_recordedUploads.begin(), m_recordedUploads.end(), pSurface), m_recordedUploads.end());
	}

	//____ _openedCommandList() ________________________________________________

	void DX12Backend::_openedCommandList()
	{
		m_listSerial = ++s_lastListSerial;
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


		if (!CHECK_HR(m_pDX12Device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, 0, IID_PPV_ARGS(pointer.GetAddressOf())), "CreateCommittedResource"))
			return;

		pointer->SetName(name);
	}


	//____ _reserveFrameBuffers() _______________________________________________
	//
	// Makes sure what is left of this frame's vertex, color and extras buffers
	// can hold what GpuBackend says the session may write, and that the frame's
	// descriptor heap has room for its blit sources.

	bool DX12Backend::_reserveFrameBuffers(int nVertices, int nColors, int nExtras, int nObjects)
	{
		auto& frame = m_frameResources[m_currentFrameIndex];

		bool bOK = _reserveBuffer(frame.vertexBuffer, frame.pVertexBufferData, frame.vertexCapacity,
								  m_buffers.pVertexBeg, m_buffers.pVertexPtr, m_buffers.pVertexEnd, nVertices, L"Wax Vertex Buffer");

		bOK = _reserveBuffer(frame.colorBuffer, frame.pColorBufferData, frame.colorCapacity,
							 m_buffers.pColorBeg, m_buffers.pColorPtr, m_buffers.pColorEnd, nColors, L"Wax Color Buffer") && bOK;

		bOK = _reserveBuffer(frame.extrasBuffer, frame.pExtrasBufferData, frame.extrasCapacity,
							 m_buffers.pExtrasBeg, m_buffers.pExtrasPtr, m_buffers.pExtrasEnd, nExtras, L"Wax Extras Buffer") && bOK;

		// One descriptor per entry in the object tables of the session is enough
		// for every blit source, see m_objectSRVs. Mipmaps that need drawing take
		// a few more, which _generateMipmaps() makes room for itself. Should the
		// heap fail to grow, _bindBlitTexture() tries again and reports it.

		if (frame.srvHeap && frame.srvCapacity - frame.nSRVDescriptors < nObjects)
			_growSRVHeap(nObjects);

		// The buffers may be new, and a session starts with everything bound anew.

		_bindSessionState();

		return bOK;
	}

	//____ _reserveBuffer() ____________________________________________________
	//
	// If what is left of the buffer is too little, it is replaced by a bigger one
	// and writing starts over at its beginning. The old buffer may already be used
	// by draws recorded this frame, so it is held until the GPU is done with the
	// next command list we submit, see _holdUntilDone(). Offsets into it recorded so far stay
	// valid, since they go with the root arguments and vertex buffer view that
	// were set when they were recorded, and _bindSessionState() points the coming
	// ones at the new buffer.
	//
	// Buffers at least double when they grow, so a frame settles on a size after
	// a few frames rather than growing a little every session.

	template<typename T>
	bool DX12Backend::_reserveBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, T*& pData, int& capacity,
									 T*& pBeg, T*& pPtr, T*& pEnd, int needed, LPCWSTR name)
	{
		if (needed < 1)
			needed = 1;

		if (pPtr && pEnd - pPtr >= needed)
			return true;

		int newCapacity = capacity * 2;
		if (newCapacity < needed)
			newCapacity = needed;

		Microsoft::WRL::ComPtr<ID3D12Resource> newBuffer;
		T* pNewData = nullptr;

		_createBuffer(newBuffer, int(newCapacity * sizeof(T)), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, name);

		D3D12_RANGE readRange = { 0, 0 };		// We only write. Upload heaps can stay mapped for their entire lifetime.

		if (!newBuffer || !CHECK_HR(newBuffer->Map(0, &readRange, (void**)&pNewData), "ID3D12Resource::Map"))
			return false;						// Keep what we have. Draws that don't fit are dropped and reported.

		if (buffer)
			_holdUntilDone(buffer.Get());

		buffer = newBuffer;
		pData = pNewData;
		capacity = newCapacity;

		pBeg = pNewData;
		pPtr = pNewData;
		pEnd = pNewData + newCapacity;

		return true;
	}

	//____ _createPipelineResources() _________________________________________
	//
	// What all pipelines share. The pipelines themselves are created on demand
	// by _bindPipeline(), since they depend on the canvas format.

	bool DX12Backend::_createPipelineResources()
	{
		return _createRootSignature() && _createSamplers();
	}

	//____ _shader() ___________________________________________________________
	//
	// The bytecode of a shader, by the name GpuBackend has for it. They come
	// compiled, see wax_dx12shaders.cpp, as compiling them here took seconds for
	// all of them and up to 650 ms for one. No bytecode if there is none by that
	// name, which is reported.

	D3D12_SHADER_BYTECODE DX12Backend::_shader(const char * pName)
	{
		for (int i = 0; pName && i < s_nbCompiledShaders; i++)
		{
			if (strcmp(s_compiledShaders[i].pName, pName) == 0)
				return { s_compiledShaders[i].pCode, s_compiledShaders[i].size };
		}

		char buffer[128];
		sprintf_s(buffer, "Shader %s is missing, run compile_shaders.py.", pName ? pName : "without a name");
		Base::reportError(ErrorLevel::Error, ErrorCode::Internal, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return { nullptr, 0 };
	}

	//____ _createRootSignature() _____________________________________________

	bool DX12Backend::_createRootSignature()
	{
		// One root signature shared by all our pipelines. Switching pipelines is
		// cheaper when they agree on the signature, and the differences between
		// fill and blit are small enough that there is no reason to have two.
		//
		// Root parameter 0 holds the handful of values that change with canvas and
		// blit source: canvas scale for the vertex shader, and source size, flags
		// and the blur brush's place in the extras buffer for the pixel shader.
		// Root constants instead of a constant buffer since there are so few of
		// them and they change often.
		//
		// The color (t0) and extras (t1) buffers are root SRVs, which only need the
		// buffer's address. Extras are read by the vertex shaders and, for a blur,
		// by the pixel shader. The blit source texture (t2) and its sampler have to
		// go through descriptor tables, there is no root descriptor for those.

		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 1;
		srvRange.BaseShaderRegister = 2;						// t2, the blit source.
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE samplerRange = {};
		samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		samplerRange.NumDescriptors = 1;
		samplerRange.BaseShaderRegister = 0;					// s0.
		samplerRange.RegisterSpace = 0;
		samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER rootParameter[9] = {};

		rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[0].Constants.ShaderRegister = 0;			// b0.
		rootParameter[0].Constants.RegisterSpace = 0;
		// Eight is also what HLSL rounds the constant buffer up to, whole 16 byte
		// registers, and the root signature has to cover all of it.

		rootParameter[0].Constants.Num32BitValues = sizeof(CanvasConstants) / 4;	// canvasScale, textureSize, flags, blurOfs, edgemapEdges, edgemapPitch.
		rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[1].Descriptor.ShaderRegister = 0;			// t0, colors.
		rootParameter[1].Descriptor.RegisterSpace = 0;
		rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// Pixel shaders read the tint from it.

		rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[2].Descriptor.ShaderRegister = 1;			// t1, extras.
		rootParameter[2].Descriptor.RegisterSpace = 0;
		rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// The blur shader reads it too.

		rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
		rootParameter[3].DescriptorTable.pDescriptorRanges = &srvRange;
		rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter[4].DescriptorTable.NumDescriptorRanges = 1;
		rootParameter[4].DescriptorTable.pDescriptorRanges = &samplerRange;
		rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[5].Descriptor.ShaderRegister = 3;			// t3, the edgemap being drawn, or the palette of a palette based blit source.
		rootParameter[5].Descriptor.RegisterSpace = 0;
		rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// The tint, where its block is in the color buffer, and how the pixel shaders
		// hand what they draw to the blender. Only the pixel shaders use them. See
		// PixelConstants.

		rootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[6].Constants.ShaderRegister = 1;			// b1.
		rootParameter[6].Constants.RegisterSpace = 0;
		rootParameter[6].Constants.Num32BitValues = sizeof(PixelConstants) / 4;
		rootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// The vertices of the Warpmap of a warp, see DX12Warpmap, and where to draw it,
		// see WarpConstants and WarpInfo in constants.hlsl.

		rootParameter[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParameter[7].Descriptor.ShaderRegister = 4;			// t4.
		rootParameter[7].Descriptor.RegisterSpace = 0;
		rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		rootParameter[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter[8].Constants.ShaderRegister = 2;			// b2.
		rootParameter[8].Constants.RegisterSpace = 0;
		rootParameter[8].Constants.Num32BitValues = sizeof(WarpConstants) / 4;
		rootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc = { };
		rsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		rsDesc.Desc_1_0.pParameters = rootParameter;
		rsDesc.Desc_1_0.NumParameters = 9;
		rsDesc.Desc_1_0.NumStaticSamplers = 0;
		rsDesc.Desc_1_0.pStaticSamplers = 0;
		rsDesc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		Microsoft::WRL::ComPtr<ID3DBlob> pSerializedRS;
		Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;

		HRESULT hr = D3D12SerializeVersionedRootSignature(&rsDesc, pSerializedRS.GetAddressOf(), pErrorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			const char* pErrorMsg = pErrorBlob ? (const char*)pErrorBlob->GetBufferPointer() : "Unknown error";
			char buffer[512];
			sprintf_s(buffer, "D3D12SerializeVersionedRootSignature failed, HRESULT = 0x%08lX: %s", (unsigned long)hr, pErrorMsg);
			Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			assert(false);
			return false;
		}

		if (!CHECK_HR(m_pDX12Device->CreateRootSignature(0, pSerializedRS->GetBufferPointer(), pSerializedRS->GetBufferSize(), IID_PPV_ARGS(m_pRootSignature.GetAddressOf())), "CreateRootSignature"))
			return false;

		setObjectName(this, m_pRootSignature.Get(), L"root signature");

		return true;
	}

	//____ _createSamplers() __________________________________________________

	bool DX12Backend::_createSamplers()
	{
		// Eight samplers, covering the combinations a surface can ask for. The
		// index is (clip ? 4 : 0) + (tiling ? 2 : 0) + (bilinear ? 1 : 0), see
		// _setBlitSource() and _bindBlitSamplerAndFlags(). The clip ones return
		// transparent black outside the texture, a tiling source never uses them.

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.NumDescriptors = 8;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (!CHECK_HR(m_pDX12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pSamplerHeap.GetAddressOf())), "CreateDescriptorHeap"))
			return false;

		setObjectName(this, m_pSamplerHeap.Get(), L"sampler heap");

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();

		for (int i = 0; i < 8; i++)
		{
			bool bClip = (i & 4) != 0;
			bool bTiling = (i & 2) != 0;
			bool bBilinear = (i & 1) != 0;

			D3D12_TEXTURE_ADDRESS_MODE addressMode = bTiling ? D3D12_TEXTURE_ADDRESS_MODE_WRAP
												   : bClip ? D3D12_TEXTURE_ADDRESS_MODE_BORDER : D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

			D3D12_SAMPLER_DESC desc = {};
			desc.Filter = bBilinear ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = addressMode;
			desc.AddressV = addressMode;
			desc.AddressW = addressMode;
			desc.MipLODBias = 0.f;
			desc.MaxAnisotropy = 1;
			desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			desc.MinLOD = 0.f;
			desc.MaxLOD = D3D12_FLOAT32_MAX;

			for (int c = 0; c < 4; c++)
				desc.BorderColor[c] = 0.f;			// Transparent black, only used by the clip samplers.

			m_pDX12Device->CreateSampler(&desc, handle);

			handle.ptr += m_samplerDescriptorSize;
		}

		return true;
	}

	//____ _canvasKindOfFormat() _______________________________________________
	//
	// What the pixel shaders need to know about the canvas, see output.hlsl.

	GpuBackend::CanvasKind DX12Backend::_canvasKindOfFormat(DXGI_FORMAT format)
	{
		if (format == DXGI_FORMAT_A8_UNORM)
			return CanvasKind::AlphaOnly;

		if (format == DXGI_FORMAT_B8G8R8X8_UNORM || format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
			return CanvasKind::WithoutAlpha;

		return CanvasKind::WithAlpha;
	}

	//____ _createPipeline() __________________________________________________
	//
	// Blending is that of GpuBackend::_blendState(), in D3D12 terms. Pixel shaders
	// only output the second color in pipelines that blend with it, which have
	// shaders of their own for that, since the debug layer warns about output
	// nothing reads.

	bool DX12Backend::_createPipeline(PipelineKind kind, const BlendState& blend, DXGI_FORMAT rtvFormat, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline)
	{
		static const D3D12_BLEND factors[] = {								// In the order of BlendFactor.
			D3D12_BLEND_ZERO, D3D12_BLEND_ONE,
			D3D12_BLEND_SRC_COLOR, D3D12_BLEND_INV_SRC_COLOR, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA,
			D3D12_BLEND_DEST_COLOR, D3D12_BLEND_INV_DEST_COLOR, D3D12_BLEND_DEST_ALPHA, D3D12_BLEND_INV_DEST_ALPHA,
			D3D12_BLEND_SRC1_COLOR, D3D12_BLEND_INV_SRC1_COLOR, D3D12_BLEND_SRC1_ALPHA, D3D12_BLEND_INV_SRC1_ALPHA };

		static const D3D12_BLEND_OP ops[] = {								// In the order of BlendOp.
			D3D12_BLEND_OP_ADD, D3D12_BLEND_OP_REV_SUBTRACT, D3D12_BLEND_OP_MIN, D3D12_BLEND_OP_MAX };

		static_assert(sizeof(factors) / sizeof(factors[0]) == size_t(BlendFactor::Size) && sizeof(ops) / sizeof(ops[0]) == size_t(BlendOp::Size),
					  "A D3D12 value for each blend factor and operation");

		D3D12_SHADER_BYTECODE vertexShader = _shader(_vertexShaderName(kind));
		D3D12_SHADER_BYTECODE pixelShader = _shader(_pixelShaderName(kind, blend.bDualSource));

		if (!vertexShader.pShaderBytecode || !pixelShader.pShaderBytecode)
			return false;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_pRootSignature.Get();
		desc.VS = vertexShader;
		desc.PS = pixelShader;

		desc.BlendState.AlphaToCoverageEnable = false;
		desc.BlendState.IndependentBlendEnable = false;

		// We only ever have one render target, but zero is not a valid blend value
		// and the rest of the array would keep the zeroes from desc = {}.

		for (int i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			auto& unused = desc.BlendState.RenderTarget[i];

			unused.SrcBlend = D3D12_BLEND_ONE;
			unused.DestBlend = D3D12_BLEND_ZERO;
			unused.BlendOp = D3D12_BLEND_OP_ADD;
			unused.SrcBlendAlpha = D3D12_BLEND_ONE;
			unused.DestBlendAlpha = D3D12_BLEND_ZERO;
			unused.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			unused.LogicOp = D3D12_LOGIC_OP_NOOP;
		}

		// Factors are filled in also where blending is off, since zero is not a
		// valid value for any of them.

		auto& rt = desc.BlendState.RenderTarget[0];

		rt.BlendEnable = blend.bEnable;
		rt.LogicOpEnable = false;
		rt.LogicOp = D3D12_LOGIC_OP_NOOP;

		rt.BlendOp = ops[int(blend.colorOp)];
		rt.SrcBlend = factors[int(blend.srcColor)];
		rt.DestBlend = factors[int(blend.dstColor)];
		rt.BlendOpAlpha = ops[int(blend.alphaOp)];
		rt.SrcBlendAlpha = factors[int(blend.srcAlpha)];
		rt.DestBlendAlpha = factors[int(blend.dstAlpha)];

		rt.RenderTargetWriteMask = blend.bWriteAlpha ? D3D12_COLOR_WRITE_ENABLE_ALL
													 : (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE);

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

		// One vertex layout for all but warps, whose vertex shader reads the vertices of
		// the Warpmap itself, see DX12Warpmap. All three inputs, as D3D12 wants each one
		// a vertex shader declares, also those it doesn't read, and the layout is
		// allowed to offer more than a shader declares.

		D3D12_INPUT_ELEMENT_DESC elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 },
			{ "COLOROFS", 0, DXGI_FORMAT_R32_UINT, 0, 8,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "EXTRASOFS", 0, DXGI_FORMAT_R32_UINT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_INPUT_LAYOUT_DESC inputLayout = {};

		bool bWarp = _isWarp(kind);

		inputLayout.NumElements = bWarp ? 0 : 3;
		inputLayout.pInputElementDescs = bWarp ? nullptr : elements;


		desc.InputLayout = inputLayout;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;


		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = rtvFormat;
		desc.NodeMask = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		if (!CHECK_HR(m_pDX12Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pPipeline.GetAddressOf())), "CreateGraphicsPipelineState"))
			return false;

		setObjectName(this, pPipeline.Get(), L"pipeline state");

		return true;
	}

} // namespace wax
