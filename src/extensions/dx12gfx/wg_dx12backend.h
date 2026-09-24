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

#ifndef	WG_DX12BACKEND_DOT_H
#define WG_DX12BACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <wg_tinttools.h>

#include <wrl.h>

#include <d3d12.h>

#include <map>
#include <vector>

namespace wg
{


	class DX12Surface;

	class DX12Backend;
	typedef	StrongPtr<DX12Backend>	DX12Backend_p;
	typedef	WeakPtr<DX12Backend>	DX12Backend_wp;


	//____ DX12Backend __________________________________________________________

	class DX12Backend : public GfxBackend
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
		void	endSession() override;

		void	setCanvas(Surface* pSurface) override;
		void	setCanvas(CanvasRef ref) override;

		void	setObjects(Object* const* pBeg, Object* const* pEnd) override;

		void	setRects(const RectSPX* pBeg, const RectSPX* pEnd) override;
		void	setColors(const HiColor* pBeg, const HiColor* pEnd) override;
		void	setTransforms(const Transform* pBeg, const Transform* pEnd) override;

		void	processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version = 2) override;


		//.____ Device _______________________________________________________

		// Sets the device and render queue shared by all backends, surfaces and
		// edgemaps, holding a reference to both. Can't be changed while any backend
		// exists. setDevice(nullptr, nullptr) releases them, which should be done
		// once all backends, surfaces and edgemaps are gone.

		static bool					setDevice(ID3D12Device * pDX12Device, ID3D12CommandQueue * pDX12CommandQueue);
		static ID3D12Device *		device() { return s_pDevice.Get(); }
		static ID3D12CommandQueue *	commandQueue() { return s_pCommandQueue.Get(); }

		// Submits what every backend has recorded and waits for the GPU to finish
		// all of it. Used by surfaces before they touch a texture any backend might
		// be using.

		static void				waitForCompletionOfAll();

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

		int		maxEdges() const override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

	protected:

		struct Vertex {
			float		x, y;		// Canvas pixels, origin top left. Subpixel positions allowed.
			uint32_t	colorOfs;	// Offset into the color buffer.
			uint32_t	extrasOfs;	// Offset into the extras buffer. Not used by plain fills.

			// Only edgemaps use these. They cost every other primitive eight bytes
			// a vertex, which is what MetalBackend pays too - the alternative is a
			// second vertex format and a second buffer to keep in step with it.

			float		u, v;					// Column, and distance down the column.
		};

		struct ColorDX12 {
			float	r, g, b, a;
		};

		struct ExtrasDX12 {
			float	x, y, z, w;
		};

		// What a pipeline draws. Together with blend mode and canvas format this
		// is what tells one pipeline from another.

		enum class PipelineKind
		{
			Fill,			// Rectangles on whole pixels.
			FillAA,			// Rectangles that aren't, with coverage worked out per pixel.
			Blit,
			Blur,			// Same geometry as a blit, nine taps instead of one.
			PaletteBlit,	// Blit and blur from a palette based source,
			PaletteBlur,	// which has to be looked up before it is filtered.
			Line,
			Segments,		// Edgemaps.

			Size
		};

		DX12Backend(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue);
		~DX12Backend();

		void _waitForFence(UINT64 fenceValue);
		void _flushCommandList();				// Submits what we have recorded so far and reopens the list.
		void _restoreBlitSourceCanvases();		// Returns canvas surfaces we read from to COMMON state.

		void _bindSessionState();				// Everything a session needs on a freshly reset command list.
		void _bindCanvasState();				// Render target, viewport and canvas scale for the active canvas.

		void _createBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& pointer, int nbBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState, LPCWSTR name);
		void _reserveBuffers(const SessionInfo* pInfo);	// Makes sure the frame's buffers have room for the session.

		template<typename T>
		bool _reserveBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, T*& pData, int& capacity,
							T*& pBeg, T*& pPtr, T*& pEnd, int needed, LPCWSTR name);

		bool _createPipelineResources();
		bool _createRootSignature();
		bool _createSamplers();
		bool _createPipeline(BlendMode blendMode, PipelineKind kind, DXGI_FORMAT rtvFormat, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline);

		BlendMode _normalizeBlendMode(BlendMode blendMode);
		ID3D12PipelineState* _pipeline(BlendMode blendMode, PipelineKind kind);
		void _setBlendFactor();					// Morph needs a constant blend factor.

		float _scaleThickness(float thickness, float slope);
		bool _setPipeline(ID3D12PipelineState* pPipeline);

		void _setCanvas(DX12Surface* pCanvas);
		void _transitionCanvas(DX12Surface* pCanvas, D3D12_RESOURCE_STATES state);

		void _drawFillRects(const RectSPX* pRects, int nRects, HiColor color);
		void _drawFillRun(PipelineKind kind, int firstVertex, int nRects);
		void _drawBlitRects(const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, int version, PipelineKind kind);
		void _drawLines(const uint16_t*& pCmd, const RectSPX*& pRects, const HiColor*& pColors, int nClipRects, int nLines);
		void _drawEdgemap(const uint16_t*& pCmd, const RectSPX*& pRects, Object* const*& pObjects, int nRects);

		bool _isDX12Surface(const Object* pObject) const;
		static bool _isDX12SurfaceType(const TypeInfo& type);
		static bool _isOfType(const TypeInfo& type, const TypeInfo& base);

		void _setBlitSource(DX12Surface* pSurface);
		bool _bindBlitSource();					// Puts source and sampler in place for the coming draw.

		int _addColor(HiColor color);			// Returns offset into color buffer, -1 if full.
		void _setTint(const TintTools::DecodedTint& tint);
		void _clearTint();
		void _bindTint();						// Records the tint root constants into the command list.
		int _addExtras(const ExtrasDX12& extras);							// Returns offset, -1 if full.
		int _addBlurExtras();												// Returns offset to 18 entries, -1 if full.
		int _addExtras(const ExtrasDX12& first, const ExtrasDX12& second);	// Returns offset, -1 if full.

		bool _compileVertexShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc );
		bool _compilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc);


		SurfaceFactory_p	m_pSurfaceFactory;	
		EdgemapFactory_p	m_pEdgemapFactory;

		D3D12_CPU_DESCRIPTOR_HANDLE	m_defaultCanvasRTV = {};
		ID3D12Resource*		m_defaultCanvasBuffer = nullptr;
		DXGI_FORMAT			m_defaultCanvasFormat = DXGI_FORMAT_UNKNOWN;		// Until setDefaultCanvas() says otherwise.
		CanvasInfo			m_defaultCanvas;
		CanvasInfo			m_dummyCanvas;

		// The canvas we are rendering into right now. Null means the default one.
		// It can change between processCommands() calls, for render layers.

		DX12Surface*				m_pActiveCanvas = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE	m_activeCanvasRTV = {};
		DXGI_FORMAT					m_activeCanvasFormat = DXGI_FORMAT_UNKNOWN;
		SizeI						m_activeCanvasSize;
		bool						m_bSessionOnDefaultCanvas = false;
		bool						m_bInSession = false;			// Between beginSession() and endSession().

		Object* const* m_pObjectsBeg = nullptr;
		Object* const* m_pObjectsEnd = nullptr;
		Object* const* m_pObjectsPtr = nullptr;

		const RectSPX* m_pRectsBeg = nullptr;
		const RectSPX* m_pRectsEnd = nullptr;
		const RectSPX* m_pRectsPtr = nullptr;

		const HiColor* m_pColorsBeg = nullptr;
		const HiColor* m_pColorsEnd = nullptr;
		const HiColor* m_pColorsPtr = nullptr;

		const Transform* m_pTransformsBeg = nullptr;
		const Transform* m_pTransformsEnd = nullptr;

		// Vertex, color and extras buffers are per frame resource, and all sessions
		// of a frame draw from them until beginRender() rewinds. beginSession() works
		// out from the SessionInfo how much the session can use at most and grows
		// the buffers when what is left isn't enough, see _reserveBuffers(). They
		// never shrink.
		//
		// Without a SessionInfo we have nothing to go on, so we make sure there is
		// at least this much room.

		const static int	c_defaultVertices = 32*1024;		// ~5400 rects.
		const static int	c_defaultColors = 8*1024;
		const static int	c_defaultExtras = 16*1024;
		const static int	c_maxSegments = 16;					// As many as MetalBackend and GlBackend handle.
		const static int	c_nbSRVDescriptors = 1024;			// Per frame resource. One per blit source change.

		Vertex*	m_pVertexBeg = nullptr;		// Start of the current frame's vertex buffer.
		Vertex* m_pVertexEnd = nullptr;
		Vertex* m_pVertexPtr = nullptr;

		ColorDX12* m_pColorBeg = nullptr;	// Start of the current frame's color buffer.
		ColorDX12* m_pColorEnd = nullptr;
		ColorDX12* m_pColorPtr = nullptr;

		ExtrasDX12* m_pExtrasBeg = nullptr;	// Start of the current frame's extras buffer.
		ExtrasDX12* m_pExtrasEnd = nullptr;
		ExtrasDX12* m_pExtrasPtr = nullptr;

		// State tracked while processing commands.

		HiColor					m_tintColor = HiColor::White;

		// The tint, as root constants for the pixel shaders: where its block is in
		// the color buffer, -1 for none. See TintTools::writeGpuTintBlock() for
		// what the block holds. The tint color is independent of it and multiplied
		// into every color _addColor() writes, so the shaders only see the tint.
		//
		// Four values since HLSL rounds the constant buffer up to a whole register,
		// and the root constants have to cover all of it.

		struct TintInfo
		{
			int32_t		ofs;
			int32_t		padding[3];
		};

		TintInfo				m_tint = { -1, { 0, 0, 0 } };
		BlendMode				m_activeBlendMode = BlendMode::Blend;
		float					m_morphFactor = 0.5f;			// Only used by BlendMode::Morph.

		// The blurbrush, set through StateChange commands. The radius is in spx,
		// turned into texture coordinates at draw time, when we know what we are
		// reading from.

		int						m_blurRadius = 0;
		float					m_blurColorMtx[9][4] = {};
		ID3D12PipelineState*	m_pActivePipeline = nullptr;

		// Blit source, set through StateChange commands.

		DX12Surface *			m_pBlitSource = nullptr;
		SizeI					m_blitSourceSize;
		bool					m_bBlitSourceAlphaOnly = false;
		bool					m_bBlitSourceIndexed = false;		// Palette based, see PipelineKind::PaletteBlit.
		int						m_blitSourceSampler = 0;			// Index into the sampler heap.
		bool					m_bBlitSourceBound = false;			// Cleared when source or session changes.


		Microsoft::WRL::ComPtr<ID3D12Device>		m_pDX12Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_pDX12CommandQueue;

		struct FrameResources
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	commandAllocator;
			Microsoft::WRL::ComPtr<ID3D12Resource>			vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>			colorBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>			extrasBuffer;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	srvHeap;						// Shader visible, holds blit source descriptors.
			Vertex*											pVertexBufferData = nullptr;	// Permanently mapped.
			ColorDX12*										pColorBufferData = nullptr;		// Permanently mapped.
			ExtrasDX12*										pExtrasBufferData = nullptr;	// Permanently mapped.
			int												vertexCapacity = 0;				// In entries, not bytes.
			int												colorCapacity = 0;
			int												extrasCapacity = 0;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>	retiredBuffers;			// Outgrown this frame, still used by its command list.
			int												nSRVDescriptors = 0;			// Used so far this frame.
			std::vector<Object_p>							objectRefs;						// What this frame's command list mentions but doesn't own.
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

		// Canvas surfaces we have barriered out of COMMON to read as blit source.
		// They go back before the session ends or the command list is closed, so
		// everything rests in COMMON between sessions and the copy queue can touch
		// it. The references then move to the frame, which holds them until the
		// GPU is done with the command list that mentions them.

		std::vector<Surface_p>								m_blitSourceCanvases;


		//

		// Pipelines vary with blend mode, fill or blit, and the format of the canvas
		// we render into, which we only learn as canvases are set. So they are
		// created on demand and kept, keyed on those three.

		std::map<uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>>	m_pipelines;

		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_pRootSignature;				// Shared by all pipelines.

		// Samplers, in the order nearest/bilinear and clamp/tile.

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_pSamplerHeap;
		UINT												m_srvDescriptorSize = 0;
		UINT												m_samplerDescriptorSize = 0;


		//

		// Compiled once, shared by all backends. Released by setDevice(nullptr, nullptr).

		static Microsoft::WRL::ComPtr<ID3DBlob>				s_vertexShaderBlobs[int(PipelineKind::Size)];
		static Microsoft::WRL::ComPtr<ID3DBlob>				s_pixelShaderBlobs[int(PipelineKind::Size)];

		static Microsoft::WRL::ComPtr<ID3D12Device>			s_pDevice;
		static Microsoft::WRL::ComPtr<ID3D12CommandQueue>	s_pCommandQueue;
		static bool											s_bImplicitDevice;		// Set by create(device, queue), released with the last backend.
		static std::vector<DX12Backend*>					s_backends;				// All that exist. Not thread safe, like the rest of the backend.

		// Widths a line of a given slope needs to keep an even thickness. Indexed
		// by slope * 16, interpolated in between. See _scaleThickness().

		float												m_lineThicknessTable[17];

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
		static const char g_lineVS[];
		static const char g_linePS[];
		static const char g_segmentsVS[];
		static const char g_segmentsPS[];

	};

} // namespace wg
#endif	// WG_DX12BACKEND_DOT_H

