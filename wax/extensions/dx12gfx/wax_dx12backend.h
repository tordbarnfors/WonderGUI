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

#ifndef	WAX_DX12BACKEND_DOT_H
#define WAX_DX12BACKEND_DOT_H
#pragma once

#include <wax_gpubackend.h>

#include <wrl.h>

#include <d3d12.h>

#include <map>
#include <vector>
#include <atomic>
#include <mutex>

namespace wax
{


	class DX12Surface;

	class DX12Backend;
	typedef	StrongPtr<DX12Backend>	DX12Backend_p;
	typedef	WeakPtr<DX12Backend>	DX12Backend_wp;


	//____ DX12Backend __________________________________________________________
	/**
	 * @brief Backend rendering with Direct3D 12.
	 *
	 * Built on GpuBackend, which turns the commands into what the shaders in
	 * wax/extensions/hlsl read. This class does what needs D3D12: the root
	 * signature, descriptor heaps, resource states, pipelines, recording draws
	 * and keeping resources alive until the GPU is done with them, and moving
	 * everything to a new device when the old one is lost.
	 */

	class DX12Backend : public GpuBackend
	{
	public:

		//.____ Creation __________________________________________

		// Any number of backends can exist at the same time, e.g. one per window,
		// but they all share one device and one command queue, set with setDevice()
		// before the first backend or surface is created. Sharing the queue is what
		// lets surfaces move freely between backends without cross-queue syncing.

		static DX12Backend_p	create();

		// Convenience for when there is only ever one backend: sets device and queue
		// if none are set (they must match if they are) and releases them again when
		// the last backend is destroyed.

		static DX12Backend_p	create(ID3D12Device * pDX12Device, ID3D12CommandQueue * pDX12CommandQueue );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void	beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo = nullptr) override;

		void	setObjects(Object* const* pBeg, Object* const* pEnd) override;


		//.____ Device _______________________________________________________

		// Sets the device and render queue shared by all backends, surfaces and
		// edgemaps, holding a reference to both. setDevice(nullptr, nullptr)
		// releases them, which is done once all backends, surfaces and edgemaps
		// are gone, or to replace the device, see below.
		//
		// Backends, surfaces and edgemaps outlive the device they were made from.
		// Setting a new one moves them all over to it: they let go of everything
		// they had on the old device and create it anew on the new one. Surfaces
		// keep their pixels in plain memory in between. With backends around this
		// has to be done in two steps, between frames:
		//
		//   1. setDevice(nullptr, nullptr). Everything lets go of the device. If it
		//      still works, what was rendered into canvas surfaces is read back
		//      first, otherwise that is lost, and they keep the pixels they last
		//      had. Backends render nothing until step 2.
		//
		//   2. setDevice(pNewDevice, pNewQueue). Everything moves onto it.
		//
		// D3D12 hands out one device per adapter and process, and gives back the
		// old one for as long as anything holds it. So in between, whoever owns the
		// device has to let go of every reference of its own - swap chains, their
		// buffers and descriptor heaps, queues, fences - before creating the new
		// one. Afterwards each backend needs setDefaultCanvas() again, since the
		// default canvas belonged to a swap chain on the old device, and every
		// window has to be redrawn in full, e.g. with RootPanel::addDirtyPatch().
		//
		// That redraw also restores what was rendered into canvas surfaces and lost
		// with a lost device. Such surfaces say so through Surface::isContentLost(),
		// and the widgets that keep content in canvases render it again when they
		// see it. Surfaces of your own that you render into need the same check.

		static bool					setDevice(ID3D12Device * pDX12Device, ID3D12CommandQueue * pDX12CommandQueue);
		static ID3D12Device *		device() { return s_pDevice.Get(); }
		static ID3D12CommandQueue *	commandQueue() { return s_pCommandQueue.Get(); }

		// Submits what every backend has recorded and waits for the GPU to finish
		// all of it. Used by surfaces before they touch a texture any backend might
		// be using.

		static void				waitForCompletionOfAll();

		//.____ Device loss __________________________________________________

		// A device can be removed under us: the GPU hung and was reset (TDR), the
		// driver was updated or crashed, or we gave it something invalid. Every
		// call on it then fails with DXGI_ERROR_DEVICE_REMOVED, and keeps failing,
		// and nothing brings it back.
		//
		// The first failure that turns out to be device loss is reported once
		// through the error handler, with the removal reason and, if DRED was
		// enabled before the device was created, its breadcrumbs and page fault
		// in the debugger's output window. From then on all backends, surfaces and
		// edgemaps stop touching the device and drop their rendering without
		// complaint, until setDevice() is given a new one, which they all move
		// onto as described above. isDeviceLost() stays true until then.

		static bool				isDeviceLost() { return s_bDeviceLost; }
		static HRESULT			deviceRemovedReason() { return s_deviceRemovedReason; }

		// Asks the device whether it is still there, and reports it if not. Cheap
		// enough to call once a frame. Returns true if the device is lost.

		static bool				checkDeviceLost();

		// For when a D3D12 call has failed: returns true if it failed because the
		// device is lost, which is then reported if it hasn't been already. The
		// caller should skip its own error report in that case.

		static bool				handleDeviceLoss(HRESULT hr);

		// Removes the device for real, for testing recovery. Needs ID3D12Device5,
		// Windows 10 1809 or later. The loss is found and reported the same way as
		// any other, by the next call that fails or checkDeviceLost().

		static bool				simulateDeviceLoss();

		//.____ Misc _________________________________________________________

		// ID3D12Resource

		// renderTargetFormat is the format of the view, not of the buffer. They
		// differ when the window wants an sRGB view over a plain swap chain buffer,
		// and it is the view's format the pipelines have to match.

		bool	setDefaultCanvas(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, ID3D12Resource * renderTargetBuffer,
								 DXGI_FORMAT renderTargetFormat, SizeSPX size, int scale);

		const CanvasInfo* canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

	protected:

		DX12Backend(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue);
		~DX12Backend();

		//.____ GpuBackend ___________________________________________________

		bool		_isRecording() const override;

		bool		_isNativeSurface(const Object* pObject) const override;
		bool		_isNativeEdgemap(const Object* pObject) const override;
		bool		_isNativeWarpmap(const Object* pObject) const override;

		void		_beginSessionOnDevice(Surface* pCanvas) override;
		void		_endSessionOnDevice() override;

		bool		_reserveFrameBuffers(int nVertices, int nColors, int nExtras, int nObjects) override;

		bool		_bindCanvas(Surface* pCanvas, SizeI& size, CanvasKind& kind) override;
		bool		_setPipeline(PipelineKind kind) override;
		void		_setConstants(ConstantBlock block) override;
		void		_draw(int firstVertex, int nVertices) override;
		void		_setScissor(const RectI& rect) override;
		void		_resetScissor() override;

		void		_prepareBlitSource(Surface* pSurface) override;
		bool		_bindBlitTexture(Surface* pSurface) override;
		void		_bindBlitSampler(Surface* pSurface, bool bClip) override;
		bool		_bindEdgemap(GpuEdgemap* pEdgemap) override;
		void		_drawWarpmap(Warpmap* pWarpmap, const RectI* pPatches, int nPatches) override;

		//.____ Internal ______________________________________________________

		void _waitForFence(UINT64 fenceValue);
		void _flushCommandList();				// Submits what we have recorded so far and reopens the list.
		void _restoreBlitSourceCanvases();		// Returns canvas surfaces we read from to COMMON state.

		void _bindSessionState();				// Everything a session needs on a freshly reset command list.
		void _bindCanvasState();				// Render target, viewport and scissor for the bound canvas.

		static void _reportDeviceLost(HRESULT hr);		// Sets s_bDeviceLost and reports it, the first time only.

		bool _createDeviceObjects(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue);
		void _releaseDeviceObjects();					// All of it, waiting for the GPU first if it still works.
		static void _dumpDRED();						// Breadcrumbs and page fault to the debugger, if DRED was on.


		void _createBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& pointer, int nbBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState, LPCWSTR name);

		template<typename T>
		bool _reserveBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, T*& pData, int& capacity,
							T*& pBeg, T*& pPtr, T*& pEnd, int needed, LPCWSTR name);

		bool _createPipelineResources();
		bool _createRootSignature();
		bool _createSamplers();
		bool _createPipeline(BlendMode blendMode, PipelineKind kind, DXGI_FORMAT rtvFormat, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline);

		ID3D12PipelineState* _pipeline(BlendMode blendMode, PipelineKind kind);

		void _transitionCanvas(DX12Surface* pCanvas, D3D12_RESOURCE_STATES state);

		// Resources the command list being recorded uses, held until the GPU is
		// done with it. See _holdUntilDone() and _releaseWhenDone().

		void _holdUntilDone(ID3D12Pageable* pObject);
		void _releaseWhenDone(UINT64 fenceValue);		// Call right after Signal(): hands what is held to a one-shot callback.

		static bool _initReleaser();					// With the first backend.
		static void _exitReleaser();					// With the last one, once the GPU is done.
		static void _flushPendingReleases();			// Once the GPU is done or gone: releases what callbacks still hold.
		static void CALLBACK _onSubmissionDone(PTP_CALLBACK_INSTANCE pInstance, PVOID pContext, PTP_WAIT pWait, TP_WAIT_RESULT waitResult);

		bool _createSRVHeap(int frameIndex, int nDescriptors);
		bool _growSRVHeap(int nNeeded = 0);			// When the current frame's heap is full or can't fit a session.
		void _forgetObjectSRVs();					// Descriptors must be copied anew for the object table.

		static bool _isDX12SurfaceType(const TypeInfo& type);

		void _generateMipmaps(DX12Surface* pSurface);			// Draws every mip level from level 0.
		ID3D12PipelineState* _mipmapPipeline(DXGI_FORMAT format);

		static const char * _pixelShaderSource(PipelineKind kind);
		static CanvasKind _canvasKindOfFormat(DXGI_FORMAT format);

		bool _compileVertexShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc );
		bool _compilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc, bool bDualSource = false);


		SurfaceFactory_p	m_pSurfaceFactory;	
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		D3D12_CPU_DESCRIPTOR_HANDLE	m_defaultCanvasRTV = {};
		ID3D12Resource*		m_defaultCanvasBuffer = nullptr;
		DXGI_FORMAT			m_defaultCanvasFormat = DXGI_FORMAT_UNKNOWN;		// Until setDefaultCanvas() says otherwise.
		CanvasInfo			m_defaultCanvas;
		CanvasInfo			m_dummyCanvas;

		// The canvas the command list renders into, see _bindCanvas(). It can change
		// between processCommands() calls, for render layers.

		DX12Surface*				m_pCanvasSurface = nullptr;		// In RENDER_TARGET state, null for the default canvas.
		D3D12_CPU_DESCRIPTOR_HANDLE	m_activeCanvasRTV = {};			// Zero when there is nothing to render into.
		DXGI_FORMAT					m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;
		SizeI						m_targetSize;					// Of the canvas, in pixels.
		bool						m_bSessionOnDefaultCanvas = false;	// Swap chain buffer is in RENDER_TARGET state.

		const static int	c_nbSRVDescriptors = 1024;			// Per frame resource to begin with, grows when needed. One per distinct blit source.

		ID3D12PipelineState*	m_pActivePipeline = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Device>		m_pDX12Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_pDX12CommandQueue;

		struct FrameResources
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	commandAllocator;
			Microsoft::WRL::ComPtr<ID3D12Resource>			vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>			colorBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>			extrasBuffer;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	srvHeap;						// Shader visible, holds blit source descriptors.
			int												srvCapacity = 0;				// Descriptors in srvHeap.
			Vertex*											pVertexBufferData = nullptr;	// Permanently mapped.
			Color*											pColorBufferData = nullptr;		// Permanently mapped.
			Extras*											pExtrasBufferData = nullptr;	// Permanently mapped.
			int												vertexCapacity = 0;				// In entries, not bytes.
			int												colorCapacity = 0;
			int												extrasCapacity = 0;
			int												nSRVDescriptors = 0;			// Used so far this frame.
			UINT64											fenceValue;
		};

		const static int									c_nbFrameResources = 2;

		FrameResources										m_frameResources[c_nbFrameResources];
		UINT												m_currentFrameIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence>					m_commandFence;
		UINT64												m_fenceValue = 0;
		HANDLE 												m_fenceEvent = nullptr;
		bool												m_bCommandListOpen = false;
		bool												m_bDeviceObjectsOK = false;		// Command list, allocators and fence were all created.

		// Canvas surfaces we have barriered out of COMMON to read as blit source.
		// They go back before the session ends or the command list is closed, so
		// everything rests in COMMON between sessions and the copy queue can touch
		// it.

		std::vector<Surface_p>								m_blitSourceCanvases;

		// A command list holds no references to what it uses, and D3D12 must not
		// have a resource released while the GPU may still use it. So every
		// texture, buffer and descriptor heap a recorded command reads or writes
		// is held here, as
		// the D3D12 resource only - surfaces and edgemaps die when their owners
		// let go, like with any other backend. On submit, _releaseWhenDone() hands
		// the lot to a thread pool callback that fires once, when the GPU has
		// passed the fence value signalled after it, and releases them there.
		// Releasing a D3D12 resource is thread safe.

		std::vector<Microsoft::WRL::ComPtr<ID3D12Pageable>>	m_heldResources;

		struct PendingRelease
		{
			HANDLE												hEvent = nullptr;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Pageable>>	resources;
		};

		// Where in the current frame's shader visible heap the descriptor of each blit
		// source in the object table has been copied, by index in the table. Text
		// switches blit source for almost every glyph, since the glyph cache sorts
		// glyphs into surfaces by height, but between only a handful of surfaces, so
		// the table has each once and a descriptor per entry is all a session needs.
		//
		// Whoever let go of a surface, its texture is held by m_heldResources until
		// the GPU is done with it. That is per submission, so the slots are forgotten
		// with each one and the next draw from the surface holds it again.

		struct SRVSlot
		{
			SIZE_T	srv = 0;		// The surface's own descriptor, which is what was copied.
			int		slot = -1;		// -1 until copied.
		};

		std::vector<SRVSlot>								m_objectSRVs;

		static TP_CALLBACK_ENVIRON							s_releaserEnv;			// Keeps our module loaded while a callback runs.
		static bool											s_bReleaserReady;
		static std::atomic<int>								s_nPendingReleases;		// Callbacks not yet finished.

		// Entries whose callbacks have not yet started, so _flushPendingReleases()
		// can fire them. A lost device may never signal the fence events they wait
		// on, and they hold the old device through what they hold.

		static std::mutex									s_pendingMutex;
		static std::vector<PendingRelease*>					s_pendingReleases;


		//

		// Pipelines vary with blend mode, fill or blit, and the format of the canvas
		// we render into, which we only learn as canvases are set. So they are
		// created on demand and kept, keyed on those three.

		std::map<uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>>	m_pipelines;

		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_pRootSignature;				// Shared by all pipelines.

		// Pipelines drawing a mip level from the one above, one per texture format.
		// See _generateMipmaps(). They take the same root signature as the rest.

		std::map<DXGI_FORMAT, Microsoft::WRL::ComPtr<ID3D12PipelineState>>	m_mipmapPipelines;

		// Samplers, in the order nearest/bilinear and clamp/tile.

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_pSamplerHeap;
		UINT												m_srvDescriptorSize = 0;
		UINT												m_samplerDescriptorSize = 0;


		//

		// Compiled once, shared by all backends. Plain bytecode, so they survive a
		// change of device. Released by setDevice(nullptr, nullptr) once no backend
		// is left.

		static Microsoft::WRL::ComPtr<ID3DBlob>				s_vertexShaderBlobs[int(PipelineKind::Size)];
		static Microsoft::WRL::ComPtr<ID3DBlob>				s_pixelShaderBlobs[int(PipelineKind::Size)];
		static Microsoft::WRL::ComPtr<ID3DBlob>				s_dualSourcePixelShaderBlobs[int(PipelineKind::Size)];		// Compiled when first needed, see _createPipeline().
		static Microsoft::WRL::ComPtr<ID3DBlob>				s_mipmapVertexShaderBlob;
		static Microsoft::WRL::ComPtr<ID3DBlob>				s_mipmapPixelShaderBlob;

		static Microsoft::WRL::ComPtr<ID3D12Device>			s_pDevice;
		static Microsoft::WRL::ComPtr<ID3D12CommandQueue>	s_pCommandQueue;
		static bool											s_bImplicitDevice;		// Set by create(device, queue), released with the last backend.
		static bool											s_bDeviceLost;			// Cleared when setDevice() is given a device.
		static HRESULT										s_deviceRemovedReason;
		static std::vector<DX12Backend*>					s_backends;				// All that exist. Not thread safe, like the rest of the backend.

		// Source code for shaders:

		static const char g_fillVS[];
		static const char g_fillPS[];
		static const char g_fillAAVS[];
		static const char g_fillAAPS[];
		static const char g_blitVS[];
		static const char g_blitPS[];
		static const char g_blurPS[];
		static const char g_paletteBlitPS[];
		static const char g_paletteBlurPS[];
		static const char g_mipmapVS[];
		static const char g_mipmapPS[];
		static const char g_lineVS[];
		static const char g_linePS[];
		static const char g_segmentsVS[];
		static const char g_segmentsPS[];
		static const char g_shapeVS[];
		static const char g_shapeBoxPS[];
		static const char g_shapeRingPS[];
		static const char g_shapeLinePS[];
		static const char g_warpVS[];
		static const char g_warpEdgemapPS[];
		static const char g_warpBlitPS[];
		static const char g_warpPaletteBlitPS[];

	};

} // namespace wax
#endif	// WAX_DX12BACKEND_DOT_H

