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
#ifndef WAX_SDLGPUBACKEND_DOT_H
#define WAX_SDLGPUBACKEND_DOT_H
#pragma once

#include <wax_gpubackend.h>
#include <wax_sdlgpucontext.h>
#include <wax_sdlgpushaders.h>

#include <map>
#include <vector>

namespace wax
{
	class SdlGpuBitmap;

	class SdlGpuBackend;
	typedef	StrongPtr<SdlGpuBackend>	SdlGpuBackend_p;
	typedef	WeakPtr<SdlGpuBackend>		SdlGpuBackend_wp;

	//____ SdlGpuBackend ________________________________________________________
	/**
	 * @brief Backend rendering with the GPU API of SDL 3, on whatever device it has: Vulkan, D3D12
	 * or Metal.
	 *
	 * Draws with the HLSL shaders of the other GPU backends, compiled to SPIR-V, DXIL and MSL by
	 * compile_shaders.py, and is built on GpuBackend, which turns the commands into what the shaders
	 * read. This class does what needs SDL_GPU.
	 *
	 * SDL_GPU has no buffers the CPU writes to where the GPU reads them, and copies only happen in
	 * copy passes, outside render passes. So the hooks GpuBackend calls only record what to do, as
	 * in GlBackend, and endSession() plays the recording back into a command buffer of its own,
	 * which it submits: a copy pass with the vertices, colors and extras of the session and what the
	 * bitmaps, edgemaps and Warpmaps it draws have changed, then a render pass on the canvas.
	 * The objects drawn are read when the session ends, which is when the Wax design says they are.
	 *
	 * An application that renders its own frame with SDL_GPU can have us draw on top of it, also
	 * straight into the window, see setCommandBuffer().
	 *
	 * SDL_GPU has no dual-source blending either. Replace and Morph, which keep a part of the canvas
	 * that only the shader knows, are drawn twice: once to take that part away, once to add the
	 * color. Multiply needs one draw, with the shader putting the two colors together, see
	 * ../hlsl/sdlgpu_dual.hlsl. Two draws give another result than one where the triangles of a
	 * draw overlap, which they don't but for pixels the edges of two share.
	 *
	 * Any number of backends can exist at the same time, e.g. one per window, but they all share
	 * one device, set with setDevice() before the first backend or bitmap is created.
	 */

	class SdlGpuBackend : public GpuBackend
	{
	public:

		//.____ Creation __________________________________________

		static SdlGpuBackend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		//.____ Device _______________________________________________________

		// Sets the device shared by all backends, bitmaps, edgemaps and Warpmaps. Must be called
		// before any of them is created, with a device created for at least one of the shader
		// formats SDL_GPU_SHADERFORMAT_SPIRV, _DXIL or _MSL, depending on platform, see
		// SdlGpuShaders. setDevice() with null lets go of it, which is only allowed once they are
		// all gone. Replacing a device that is in use isn't supported.

		static bool				setDevice( SDL_GPUDevice * pDevice );
		static SDL_GPUDevice *	device() { return SdlGpuContext::device(); }

		//.____ Misc _________________________________________________________

		// The default canvas is a texture of the owner of the backend, with COLOR_TARGET usage. What
		// is in it stays where we don't draw. Supported formats are B8G8R8A8 and R8G8B8A8, UNORM or
		// UNORM_SRGB, where UNORM is taken as linear, so a window wants an sRGB swap chain
		// (SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR) not to come out too dark.
		//
		// A texture of a swap chain is only valid for the command buffer it was acquired with, so
		// it can only be the default canvas together with that command buffer, see
		// setCommandBuffer(), and has to be set for each frame. It doesn't keep its content between
		// frames either, so everything has to be drawn each time.

		bool	setDefaultCanvas( SDL_GPUTexture * pTexture, SDL_GPUTextureFormat format, SizeSPX size, int scale );

		// For drawing into a frame of the owner's, as on top of what it has rendered into the
		// window itself. The render pass of a session on the default canvas goes into
		// pCommandBuffer instead of a command buffer of ours, and it is the owner who submits it:
		//
		//		SDL_GPUCommandBuffer * pCmd = SDL_AcquireGPUCommandBuffer(pDevice);
		//		SDL_WaitAndAcquireGPUSwapchainTexture(pCmd, pWindow, &pTexture, &w, &h);
		//		...the owner's render passes, all ended...
		//		pBackend->setDefaultCanvas(pTexture, format, {w*64, h*64}, scale);
		//		pBackend->setCommandBuffer(pCmd);
		//		pRootPanel->render();
		//		SDL_SubmitGPUCommandBuffer(pCmd);
		//
		// Set before a render and forgotten by endRender(), so it is given for each one, on the
		// thread that acquired it and with no pass of the owner's open. Sessions on bitmaps still
		// have command buffers of their own, as do the uploads of all sessions, so everything but
		// the draws into the default canvas is submitted when a session ends, as without it.
		//
		// We push uniform data to slots 0-3 of both stages, so an owner that draws more in the
		// command buffer afterwards pushes its own again. And what the owner changes of a bitmap
		// after the render but before it submits is what the default canvas gets.

		void	setCommandBuffer( SDL_GPUCommandBuffer * pCommandBuffer );

		const CanvasInfo*	canvasInfo(CanvasRef ref) const override;

		BitmapFactory_p	bitmapFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

	protected:
		SdlGpuBackend();
		~SdlGpuBackend();

		//.____ GpuBackend ___________________________________________________

		bool		_isRecording() const override;

		bool		_isNativeBitmap( const Object * pObject ) const override;
		bool		_isNativeEdgemap( const Object * pObject ) const override;
		bool		_isNativeWarpmap( const Object * pObject ) const override;

		void		_beginSessionOnDevice( Bitmap * pCanvas ) override;
		void		_endSessionOnDevice() override;

		bool		_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects ) override;

		bool		_bindCanvas( Bitmap * pCanvas, SizeI& size, CanvasKind& kind ) override;
		bool		_bindPipeline( PipelineKind kind, BlendMode blendMode ) override;
		void		_setConstants( ConstantBlock block ) override;
		void		_draw( int firstVertex, int nVertices ) override;
		void		_setScissor( const RectI& rect ) override;
		void		_resetScissor() override;

		void		_prepareBlitSource( Bitmap * pBitmap ) override;
		bool		_bindBlitTexture( Bitmap * pBitmap ) override;
		void		_bindBlitSampler( Bitmap * pBitmap, bool bClip ) override;
		bool		_bindEdgemap( GpuEdgemap * pEdgemap ) override;
		void		_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches ) override;

		//.____ Internal ______________________________________________________

		// A pipeline of GpuBackend is one or two of SDL_GPU, drawn one after the other, see the
		// class description. All draw with the same resources.

		struct Pipeline
		{
			SDL_GPUGraphicsPipeline *	pDraws[2] = {};		// Null if it couldn't be made.
			uint32_t					sdlOutput = 0;		// What the first draws, see sdlgpu_dual.hlsl. 0 if not a Dual shader.

			const SdlGpuShaders::Shader *	pVertexShader = nullptr;
			const SdlGpuShaders::Shader *	pPixelShader = nullptr;
		};

		// What the hooks record, played back by _execute().

		enum class Op : uint8_t
		{
			BindCanvas,				// pObject: the bitmap, null for the default canvas.
			SetPipeline,			// pPipeline.
			SetConstants,			// a: ConstantBlock, b: index in m_constantSnapshots.
			Draw,					// a: first vertex, b: vertices.
			SetScissor,				// rect, in canvas pixels.
			BindBlitTexture,		// pObject: the bitmap.
			BindSampler,			// a: index in m_samplers.
			BindEdgemap,			// pObject: the edgemap.
			DrawWarpmap				// pObject: the Warpmap, a: first patch in m_patches, b: patches.
		};

		struct Op_t
		{
			Op					op;
			int					a;
			int					b;
			Object *			pObject;		// Kept alive by m_sessionObjects.
			const Pipeline *	pPipeline;
			RectI				rect;
		};

		// The constants, as big as their uniform buffers. Of WarpConstants HLSL makes a whole
		// register of its last members.

		union ConstantSnapshot
		{
			CanvasConstants		canvas;
			PixelConstants		pixel;
			WarpConstants		warp;
		};

		void		_record( Op op, int a = 0, int b = 0, Object * pObject = nullptr, const RectI& rect = RectI(), const Pipeline * pPipeline = nullptr );
		void		_holdForSession( Object * pObject );

		bool		_createDeviceObjects();
		void		_destroyDeviceObjects();

		Pipeline *	_pipeline( PipelineKind kind, BlendMode blendMode );
		Pipeline	_createPipeline( PipelineKind kind, const BlendState& blend, SDL_GPUTextureFormat format );
		SDL_GPUGraphicsPipeline *	_createDraw( PipelineKind kind, const char * pPixelShader, SDL_GPUColorTargetBlendState blend, SDL_GPUTextureFormat format );
		SDL_GPUShader *	_shader( const char * pName, const SdlGpuShaders::Shader ** pInfo );

		// Playing back.

		void		_execute();
		void		_recordUploads( SDL_GPUCopyPass * pCopyPass );
		void		_generateMipmaps( SDL_GPUCommandBuffer * pCmd );
		void		_executeBindCanvas( SdlGpuBitmap * pCanvas );
		void		_executeScissor( const RectI& rect );
		bool		_beginPass();					// Into the canvas being played back for, unless already.
		void		_endPass();
		bool		_prepareDraw( int draw );		// Of the pipeline being played back, see Pipeline. False if there is none.
		void		_bindResources();

		BitmapFactory_p	m_pBitmapFactory;
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		bool				m_bDeviceObjectsOK = false;
		bool				m_bRendering = false;			// Between beginRender() and endRender().

		// Recording, see the class description.

		std::vector<Op_t>				m_ops;
		std::vector<RectI>				m_patches;				// Of DrawWarpmap.
		std::vector<Object_p>			m_sessionObjects;		// Objects the recording points at.
		std::vector<ConstantSnapshot>	m_constantSnapshots;

		SDL_GPUTextureFormat	m_targetFormat = SDL_GPU_TEXTUREFORMAT_INVALID;	// Of the canvas being recorded for.
		SizeI					m_targetSize;
		bool					m_bTargetBound = false;

		// What the session writes, for GpuBackend: a transfer buffer, mapped while the session
		// records, with vertices, colors and extras one after the other. Uploaded to the buffers
		// the shaders read by _execute(). They grow when a session needs more, at least doubling.

		SDL_GPUTransferBuffer *	m_pSessionTransfer = nullptr;
		bool					m_bSessionTransferMapped = false;
		uint32_t				m_sessionTransferSize = 0;

		SDL_GPUBuffer *			m_pVertexBuffer = nullptr;
		SDL_GPUBuffer *			m_pColorBuffer = nullptr;
		SDL_GPUBuffer *			m_pExtrasBuffer = nullptr;
		int						m_vertexCapacity = 0;		// In entries.
		int						m_colorCapacity = 0;
		int						m_extrasCapacity = 0;

		// Playing back.

		SDL_GPUCommandBuffer *	m_pCmd = nullptr;			// The one draws go into.
		SDL_GPURenderPass *		m_pPass = nullptr;

		SDL_GPUTexture *		m_pExecTarget = nullptr;	// Null when there is nothing to draw into.
		SizeI					m_execTargetSize;
		RectI					m_execScissor;

		const Pipeline *		m_pExecPipeline = nullptr;
		SDL_GPUGraphicsPipeline *	m_pBoundDraw = nullptr;		// Bound in the render pass.
		uint32_t				m_execSdlOutput = 0;		// Pushed to the pixel shaders, see sdlgpu_dual.hlsl.

		SDL_GPUBuffer *			m_execBuffers[5] = {};		// By SdlGpuShaders::Resource.
		SDL_GPUTexture *		m_pExecBlitTexture = nullptr;
		SDL_GPUSampler *		m_pExecSampler = nullptr;
		bool					m_bBindingsChanged = true;	// Resources or pipeline have changed since bound.

		// Default canvas.

		SDL_GPUTexture *		m_pDefaultTexture = nullptr;
		SDL_GPUTextureFormat	m_defaultFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
		SDL_GPUCommandBuffer *	m_pOwnerCmd = nullptr;		// See setCommandBuffer(). Null between renders.
		bool					m_bSessionOnDefaultCanvas = false;
		CanvasInfo				m_defaultCanvas;
		CanvasInfo				m_dummyCanvas;

		// Pipelines are created as combinations of canvas format, kind and blend mode show up, and
		// shaders as pipelines need them.

		std::map<uint64_t, Pipeline>	m_pipelines;
		std::vector<SDL_GPUShader*>		m_shaders;			// Of SdlGpuShaders::shaders, null until needed.

		// Samplers, indexed (tiling ? 2 : 0) + (bilinear ? 1 : 0). Clipping has no sampler of its
		// own, as SDL_GPU has no border color, see ../hlsl/sdlgpu.hlsl.

		SDL_GPUSampler *		m_samplers[4] = {};

		SDL_GPUBuffer *			m_pDummyBuffer = nullptr;	// For storage buffer slots that have nothing bound yet.
	};

} // namespace wax
#endif // WAX_SDLGPUBACKEND_DOT_H
