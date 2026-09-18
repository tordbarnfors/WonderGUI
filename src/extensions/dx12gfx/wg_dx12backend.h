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

#ifndef	WG_DX12BACKEND_DOT_H
#define WG_DX12BACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>

#include <wrl.h>

#include <d3d12.h>

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


		//.____ Misc _________________________________________________________

		// ID3D12Resource

		bool	setDefaultCanvas(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, ID3D12Resource * renderTargetBuffer, SizeSPX size, int scale);

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
			uint32_t	extrasOfs;	// Offset into the extras buffer. Only used by blits.
		};

		struct ColorDX12 {
			float	r, g, b, a;
		};

		struct ExtrasDX12 {
			float	x, y, z, w;
		};

		DX12Backend(ID3D12Device* pDX12Device, ID3D12CommandQueue* pDX12CommandQueue);
		~DX12Backend();

		void _waitForFence(UINT64 fenceValue);

		void _createBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& pointer, int nbBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState, LPCWSTR name);

		bool _createPipelines();
		bool _createRootSignature();
		bool _createSamplers();
		bool _createPipeline(BlendMode blendMode, bool bBlit, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipeline);

		ID3D12PipelineState* _fillPipeline(BlendMode blendMode);
		ID3D12PipelineState* _blitPipeline(BlendMode blendMode);
		bool _setPipeline(ID3D12PipelineState* pPipeline);

		void _drawFillRects(const RectSPX* pRects, int nRects, HiColor color);
		void _drawBlitRects(const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, int version);

		void _setBlitSource(DX12Surface* pSurface);
		bool _bindBlitSource();					// Puts source and sampler in place for the coming draw.

		int _addColor(HiColor color);			// Returns offset into color buffer, -1 if full.
		int _addExtras(const ExtrasDX12& first, const ExtrasDX12& second);	// Returns offset, -1 if full.

		bool _compileVertexShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc );
		bool _compilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob, LPCVOID pSrc);


		SurfaceFactory_p	m_pSurfaceFactory;	
		EdgemapFactory_p	m_pEdgemapFactory;

		D3D12_CPU_DESCRIPTOR_HANDLE	m_defaultCanvasRTV;
		ID3D12Resource*		m_defaultCanvasBuffer = nullptr;
		CanvasInfo			m_defaultCanvas;
		CanvasInfo			m_dummyCanvas;

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

		const static int	c_vertexBufferSize = 512*1024;		// Per frame resource. 96 bytes per rect.
		const static int	c_colorBufferSize = 64*1024;		// Per frame resource. 16 bytes per color.
		const static int	c_extrasBufferSize = 64*1024;		// Per frame resource. 32 bytes per blit rect.
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
		BlendMode				m_activeBlendMode = BlendMode::Blend;
		ID3D12PipelineState*	m_pActivePipeline = nullptr;

		// Blit source, set through StateChange commands.

		DX12Surface *			m_pBlitSource = nullptr;
		SizeI					m_blitSourceSize;
		bool					m_bBlitSourceAlphaOnly = false;
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


		//

		// One pipeline per supported blend mode.

		const static int									c_fillPipelineBlend = 0;
		const static int									c_fillPipelineReplace = 1;
		const static int									c_nbFillPipelines = 2;

		Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pFillPipelines[c_nbFillPipelines];
		Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pBlitPipelines[c_nbFillPipelines];
		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_pRootSignature;				// Shared by fill and blit pipelines.

		// Samplers, in the order nearest/bilinear and clamp/tile.

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_pSamplerHeap;
		UINT												m_srvDescriptorSize = 0;
		UINT												m_samplerDescriptorSize = 0;


		//

		Microsoft::WRL::ComPtr<ID3DBlob>					m_fillVertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob>					m_fillPixelShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob>					m_blitVertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob>					m_blitPixelShaderBlob;

		// Source code for shaders:

		static const char g_fillVS[];
		static const char g_fillPS[];
		static const char g_blitVS[];
		static const char g_blitPS[];

	};

} // namespace wg
#endif	// WG_DX12BACKEND_DOT_H

