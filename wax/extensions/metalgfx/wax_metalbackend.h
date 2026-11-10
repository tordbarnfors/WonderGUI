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

#ifndef WAX_METALBACKEND_DOT_H
#define WAX_METALBACKEND_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wax_gpubackend.h>
#include <wax_metalsurface.h>

#include <map>
#include <string>
#include <vector>


namespace wax
{
	class MetalBackend;
	typedef	StrongPtr<MetalBackend>	MetalBackend_p;
	typedef	WeakPtr<MetalBackend>	MetalBackend_wp;

	//____ MetalBackend ________________________________________________________
	/**
	 * @brief Backend that renders with Metal.
	 *
	 * Built on GpuBackend, which turns the commands into what the shaders read. The shaders are
	 * the HLSL of wax/extensions/hlsl, turned into MSL by compile_shaders.py, see
	 * wax_metalshaders.mm. What is left here is Metal: a command buffer per session, a render
	 * pass per canvas, pipelines, and binding buffers and textures where the shaders expect them.
	 *
	 * Each session is committed when it ends, the buffers of GpuBackend made for it and kept alive
	 * by its command buffer until the GPU is done with them.
	 */

	class MetalBackend : public GpuBackend
	{
		friend class MetalSurface;
		friend class MetalEdgemap;
	public:
		//.____ Creation ________________________________________________

		static void setMetalDevice( id<MTLDevice> device );
		static id<MTLDevice> metalDevice() { return s_metalDevice; }

		static MetalBackend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void    autopresent( id<MTLDrawable> drawable );           // endRender() will clear this, so needs to be set for each begin/end render cycle.

		//.____ Misc _________________________________________________________

		bool    setDefaultCanvas( MTLRenderPassDescriptor* renderPassDesc, SizeI pixelSize, PixelFormat pixelFormat, int scale = 64, ColorSpace colorSpace = ColorSpace::sRGB );

		const CanvasInfo* canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

	protected:
		MetalBackend();
		~MetalBackend();

		//.____ GpuBackend ____________________________________________________

		bool	_isRecording() const override;

		bool	_isNativeSurface( const Object * pObject ) const override;
		bool	_isNativeEdgemap( const Object * pObject ) const override;
		bool	_isNativeWarpmap( const Object * pObject ) const override;

		void	_beginSessionOnDevice( Surface * pCanvas ) override;
		void	_endSessionOnDevice() override;

		bool	_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects ) override;

		bool	_bindCanvas( Surface * pCanvas, SizeI& size, CanvasKind& kind ) override;
		bool	_setPipeline( PipelineKind kind ) override;
		void	_setConstants( ConstantBlock block ) override;
		void	_draw( int firstVertex, int nVertices ) override;
		void	_setScissor( const RectI& rect ) override;
		void	_resetScissor() override;

		void	_prepareBlitSource( Surface * pSurface ) override;
		bool	_bindBlitTexture( Surface * pSurface ) override;
		void	_bindBlitSampler( Surface * pSurface, bool bClip ) override;
		bool	_bindEdgemap( GpuEdgemap * pEdgemap ) override;
		void	_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches ) override;

		//.____ Internal ______________________________________________________

		// Where the shaders find things, see compile_shaders.py. Buffers 1-5 are registers t0-t4
		// of the HLSL, texture 3 is t2.

		struct BufferIndex
		{
			enum : NSUInteger
			{
				Constants = 0,				// All three blocks of constants.hlsl, as one.
				Colors = 1,
				Extras = 2,
				EdgemapOrPalette = 4,		// The palette of a palette based blit source, or the edgemap drawn.
				WarpVertices = 5,			// The vertices of the Warpmap drawn.
				Vertices = 30				// The vertex buffer, which the vertex descriptors of the pipelines read.
			};
		};

		const static int	c_blitSourceTexture = 3;
		const static int	c_blitSampler = 0;

		// The constants as the shaders have them, the three blocks of GpuBackend one after the
		// other, see constants.hlsl. Sent to both stages before a draw when changed.

		struct ShaderConstants
		{
			CanvasConstants	canvas;
			PixelConstants	pixel;
			WarpConstants	warp;
		};

		struct ShaderSource
		{
			const char *	pName;
			const char *	pSource;
		};

		void	_endEncoding();
		bool	_beginEncoding();
		void	_bindEncoderState();
		void	_flushConstants();

		id<MTLFunction>				_function( const char * pName );
		id<MTLRenderPipelineState>	_pipeline( PipelineKind kind, BlendMode blendMode );
		id<MTLRenderPipelineState>	_createPipeline( PipelineKind kind, BlendMode blendMode, MTLPixelFormat format, CanvasKind canvasKind );

		void						_generateMipmaps( MetalSurface * pSurface );		// Draws every mip level from level 0, between render passes.
		id<MTLRenderPipelineState>	_mipmapPipeline( MTLPixelFormat format );

		static bool				_toMetalFormat( PixelFormat format, ColorSpace colorSpace, MTLPixelFormat& metalFormat );

		SurfaceFactory_p    m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		// Session

		id<MTLCommandBuffer>			m_commandBuffer = nil;			// Autoreleased, kept by the pool of the session.
		NSAutoreleasePool*				m_pSessionPool = nil;			// Drains the autoreleased command buffer, encoders etc. of a session.
		id<MTLRenderCommandEncoder>		m_renderEncoder = nil;			// Render pass on the canvas, nil between passes.

		id<MTLBuffer>					m_vertexBuffer = nil;			// GpuBackend's buffers of the session, see _reserveFrameBuffers().
		id<MTLBuffer>					m_colorBuffer = nil;
		id<MTLBuffer>					m_extrasBuffer = nil;

		std::vector<MetalSurface_p>		m_sessionCanvases;				// Surfaces rendered into this session.

		// Canvas

		MetalSurface *					m_pCanvasSurface = nullptr;		// Null for the default canvas.
		bool							m_bDefaultCanvasLoaded = false;	// A render pass on the default canvas has been begun this session.
		MTLPixelFormat					m_targetFormat = MTLPixelFormatInvalid;
		SizeI							m_targetSize;					// In pixels.

		// What the render encoder has bound, so it can be bound again after a render pass has been
		// interrupted to draw mip levels.

		ShaderConstants					m_constants;
		bool							m_bConstantsDirty = true;
		id<MTLRenderPipelineState>		m_activePipeline = nil;
		id<MTLTexture>					m_boundTexture = nil;
		id<MTLSamplerState>				m_boundSampler = nil;
		id<MTLBuffer>					m_boundEdgemapOrPalette = nil;
		MTLScissorRect					m_scissor = { 0, 0, 0, 0 };
		bool							m_bScissorEmpty = false;		// Nothing is drawn, Metal takes no empty scissor rect.

		// Shaders, pipelines and samplers

		std::map<std::string, id<MTLFunction>>		m_functions;		// Each compiled on first use, nil if that failed.
		std::vector<id<MTLLibrary>>					m_libraries;		// One per shader, holding its function.
		std::map<uint64_t, id<MTLRenderPipelineState>>	m_pipelines;	// See _pipeline(), nil if creation failed.
		std::map<MTLPixelFormat, id<MTLRenderPipelineState>>	m_mipmapPipelines;	// See _mipmapPipeline().

		id<MTLSamplerState>				m_samplers[8] = {};				// [bClip*4 + bTiling*2 + bBilinear]

		// Default canvas

		id<MTLDrawable>             m_drawableToAutoPresent = nil;
		MTLRenderPassDescriptor*    m_defaultCanvasRenderPassDesc = nil;
		MTLPixelFormat				m_defaultCanvasFormat = MTLPixelFormatBGRA8Unorm_sRGB;
		CanvasInfo                  m_defaultCanvas;

		std::atomic<int>            m_flushesInProgress;				// Sessions committed and not yet completed.

		static const ShaderSource	s_shaderSources[];					// See wax_metalshaders.mm.
		static const int			s_nbShaderSources;

		static id<MTLDevice>        s_metalDevice;
		static id<MTLCommandQueue>  s_metalCommandQueue;
	};

} // namespace wax
#endif //WAX_METALBACKEND_DOT_H
