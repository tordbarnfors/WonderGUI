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

#import "Foundation/Foundation.h"

#include <wax_metalbackend.h>
#include <wax_metalsurfacefactory.h>
#include <wax_metaledgemapfactory.h>
#include <wax_metaledgemap.h>
#include <wax_metalwarpmapfactory.h>
#include <wax_metalwarpmap.h>
#include <wax_base.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <unistd.h>


namespace wax
{

const TypeInfo MetalBackend::TYPEINFO = { "MetalBackend", &GpuBackend::TYPEINFO };

id<MTLDevice>       MetalBackend::s_metalDevice = nil;
id<MTLCommandQueue> MetalBackend::s_metalCommandQueue = nil;

//____ setMetalDevice() ______________________________________________________

void MetalBackend::setMetalDevice( id<MTLDevice> device )
{
	[s_metalCommandQueue release];

	s_metalDevice = device;

	if( device == nil )
		s_metalCommandQueue = nil;
	else
		s_metalCommandQueue = [s_metalDevice newCommandQueue];
}

//____ create() _______________________________________________________________

MetalBackend_p MetalBackend::create()
{
	return MetalBackend_p(new MetalBackend());
}

//____ constructor() ______________________________________________________

MetalBackend::MetalBackend()
{
	m_defaultCanvas.ref = CanvasRef::Default;
	m_flushesInProgress = 0;
	m_constants = ShaderConstants();

	// Samplers, as those of DX12Backend and VulkanBackend: tiling sources repeat, ClipBlit reads
	// transparent black outside the source and everything else, including Blur, reads the edge
	// pixels. Mip levels are only there for mipmapped surfaces.

	MTLSamplerDescriptor *desc = [MTLSamplerDescriptor new];

	for( int i = 0 ; i < 8 ; i++ )
	{
		bool bClip = (i & 4) != 0;
		bool bTiling = (i & 2) != 0;
		bool bBilinear = (i & 1) != 0;

		MTLSamplerAddressMode addressMode = bTiling ? MTLSamplerAddressModeRepeat
										  : bClip ? MTLSamplerAddressModeClampToZero : MTLSamplerAddressModeClampToEdge;

		desc.minFilter = bBilinear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
		desc.magFilter = desc.minFilter;
		desc.mipFilter = bBilinear ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;
		desc.rAddressMode = addressMode;
		desc.sAddressMode = addressMode;
		desc.tAddressMode = addressMode;

		m_samplers[i] = [s_metalDevice newSamplerStateWithDescriptor:desc];
	}

	[desc release];
}

//____ destructor() _______________________________________________________

MetalBackend::~MetalBackend()
{
	while( m_flushesInProgress > 0 )
		usleep(100);

	[m_defaultCanvasRenderPassDesc release];
	m_defaultCanvasRenderPassDesc = nil;
	m_drawableToAutoPresent = nil;

	for( auto& sampler : m_samplers )
		[sampler release];

	for( auto& entry : m_pipelines )
		[entry.second release];

	for( auto& entry : m_mipmapPipelines )
		[entry.second release];

	for( auto& entry : m_functions )
		[entry.second release];

	for( auto library : m_libraries )
		[library release];
}

//____ typeInfo() _________________________________________________________

const TypeInfo& MetalBackend::typeInfo(void) const
{
	return TYPEINFO;
}

//____ canBeBlitSource() ______________________________________________________

bool MetalBackend::canBeBlitSource(const TypeInfo& type) const
{
	return (type == MetalSurface::TYPEINFO);
}

//____ canBeCanvas() __________________________________________________________

bool MetalBackend::canBeCanvas(const TypeInfo& type) const
{
	return (type == MetalSurface::TYPEINFO);
}

//____ waitForCompletion() ___________________________________________________
//
// The queue runs command buffers in the order they were committed, so once an empty one is
// done, so are all sessions before it.

void MetalBackend::waitForCompletion()
{
	if( m_flushesInProgress == 0 )
		return;

	@autoreleasepool {
		id<MTLCommandBuffer> commandBuffer = [s_metalCommandQueue commandBuffer];
		[commandBuffer commit];
		[commandBuffer waitUntilCompleted];
	}
}

//____ surfaceFactory() _______________________________________________________

SurfaceFactory_p MetalBackend::surfaceFactory()
{
	if( !m_pSurfaceFactory )
		m_pSurfaceFactory = MetalSurfaceFactory::create();

	return m_pSurfaceFactory;
}

//____ edgemapFactory() ______________________________________________________

EdgemapFactory_p MetalBackend::edgemapFactory()
{
	if (!m_pEdgemapFactory)
		m_pEdgemapFactory = MetalEdgemapFactory::create();

	return m_pEdgemapFactory;
}

//____ warpmapFactory() ___________________________________________________

WarpmapFactory_p MetalBackend::warpmapFactory()
{
	if (!m_pWarpmapFactory)
		m_pWarpmapFactory = MetalWarpmapFactory::create();

	return m_pWarpmapFactory;
}

//____ canvasInfo() ____________________________________________________________

const CanvasInfo * MetalBackend::canvasInfo(CanvasRef ref) const
{
	if( ref == CanvasRef::Default )
		return &m_defaultCanvas;
	else
		return nullptr;
}

//____ setDefaultCanvas() ___________________________________________________

bool MetalBackend::setDefaultCanvas( MTLRenderPassDescriptor* renderPassDesc, SizeI pixelSize, PixelFormat pixelFormat, int scale, ColorSpace colorSpace )
{
	if( pixelFormat == PixelFormat::Undefined )
		pixelFormat = PixelFormat::ARGB_8;

	if( colorSpace == ColorSpace::Undefined )
		colorSpace = ColorSpace::sRGB;

	MTLPixelFormat format;
	if( !_toMetalFormat(pixelFormat, colorSpace, format) )
	{
		Base::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "pixelFormat must be ARGB_8, XRGB_8 or Alpha_8", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}

	[renderPassDesc retain];
	[m_defaultCanvasRenderPassDesc release];
	m_defaultCanvasRenderPassDesc = renderPassDesc;
	m_defaultCanvasFormat = format;
	m_defaultCanvas.size = pixelSize*64;
	m_defaultCanvas.scale = scale;
	m_defaultCanvas.format = pixelFormat;
	m_defaultCanvas.colorSpace = colorSpace;

	return true;
}

//____ _toMetalFormat() _______________________________________________________
//
// The texture formats of canvases, as MetalSurface has them.

bool MetalBackend::_toMetalFormat( PixelFormat format, ColorSpace colorSpace, MTLPixelFormat& metalFormat )
{
	bool bLinear = (colorSpace == ColorSpace::Linear);

	switch( format )
	{
		case PixelFormat::XRGB_8:
		case PixelFormat::ARGB_8:
			metalFormat = bLinear ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatBGRA8Unorm_sRGB;
			return true;
		case PixelFormat::Alpha_8:
			metalFormat = MTLPixelFormatR8Unorm;
			return true;
		default:
			return false;
	}
}

//____ beginRender() _________________________________________________________

void MetalBackend::beginRender()
{
}

//____ endRender() ___________________________________________________________

void MetalBackend::endRender()
{
}

//____ autopresent() ________________________________________________

void MetalBackend::autopresent( id<MTLDrawable> drawable )
{
	m_drawableToAutoPresent = drawable;
}

//____ _isRecording() ________________________________________________________
//
// Each session records into a command buffer of its own, so all we need is a device.

bool MetalBackend::_isRecording() const
{
	return s_metalDevice != nil && s_metalCommandQueue != nil;
}

//____ _isNativeSurface() ____________________________________________________

bool MetalBackend::_isNativeSurface( const Object * pObject ) const
{
	return pObject && _isOfType(pObject->typeInfo(), MetalSurface::TYPEINFO);
}

//____ _isNativeEdgemap() ____________________________________________________

bool MetalBackend::_isNativeEdgemap( const Object * pObject ) const
{
	return pObject && _isOfType(pObject->typeInfo(), MetalEdgemap::TYPEINFO);
}

//____ _isNativeWarpmap() ____________________________________________________

bool MetalBackend::_isNativeWarpmap( const Object * pObject ) const
{
	return pObject && _isOfType(pObject->typeInfo(), MetalWarpmap::TYPEINFO);
}

//____ _beginSessionOnDevice() _______________________________________________

void MetalBackend::_beginSessionOnDevice( Surface * pCanvas )
{
	// The command buffer and encoders we get from Metal are autoreleased and the
	// command buffer keeps our buffers alive. If no autorelease pool is drained
	// between sessions, as when rendering from a loop of our own, they would pile
	// up until Metal runs out of memory, so we give each session a pool of its own.

	m_pSessionPool = [[NSAutoreleasePool alloc] init];

	m_commandBuffer = [s_metalCommandQueue commandBuffer];
	m_commandBuffer.label = @"MetalBackend";

	m_pCanvasSurface = nullptr;
	m_bDefaultCanvasLoaded = false;
	m_targetFormat = MTLPixelFormatInvalid;
	m_targetSize = SizeI();
}

//____ _endSessionOnDevice() _________________________________________________

void MetalBackend::_endSessionOnDevice()
{
	_endEncoding();

	if( m_commandBuffer )
	{
		m_flushesInProgress++;

		// Schedule a present once the framebuffer is complete using the current drawable.

		if( m_drawableToAutoPresent != nil )
		{
			[m_commandBuffer presentDrawable:m_drawableToAutoPresent];
			m_drawableToAutoPresent = nil;
		}

		[m_commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
			m_flushesInProgress--;
		}];

		[m_commandBuffer commit];
		[m_commandBuffer waitUntilScheduled];
	}

	m_commandBuffer = nil;			// Autoreleased, Metal keeps it until it is done.

	// The command buffer keeps the buffers it reads until it is done with them.

	[m_vertexBuffer release];
	[m_colorBuffer release];
	[m_extrasBuffer release];
	m_vertexBuffer = nil;
	m_colorBuffer = nil;
	m_extrasBuffer = nil;
	m_buffers = FrameBuffers();

	// What we rendered isn't in the CPU side buffers of the canvases. Set once the session is
	// committed, so that syncing them comes after it in the queue.

	for( auto& pCanvas : m_sessionCanvases )
		pCanvas->m_bBufferNeedsSync = true;

	m_sessionCanvases.clear();

	m_pCanvasSurface = nullptr;
	m_targetSize = SizeI();
	m_targetFormat = MTLPixelFormatInvalid;
	m_activePipeline = nil;
	m_boundTexture = nil;
	m_boundSampler = nil;
	m_boundEdgemapOrPalette = nil;

	[m_pSessionPool drain];
	m_pSessionPool = nil;
}

//____ _reserveFrameBuffers() ________________________________________________
//
// Each session gets buffers of its own, as big as it needs. Its command buffer keeps them until
// the GPU is done with them, so nothing in flight is ever written to.

bool MetalBackend::_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects )
{
	auto create = [](id<MTLBuffer>& buffer, auto*& pBeg, auto*& pPtr, auto*& pEnd, int needed) -> bool
	{
		typedef typename std::remove_reference<decltype(*pBeg)>::type Entry;

		if( needed < 1 )
			needed = 1;

		[buffer release];
		buffer = [s_metalDevice newBufferWithLength:size_t(needed) * sizeof(Entry) options:MTLResourceStorageModeShared];

		if( buffer == nil )
		{
			pBeg = pPtr = pEnd = nullptr;
			return false;
		}

		pBeg = (Entry*) [buffer contents];
		pPtr = pBeg;
		pEnd = pBeg + needed;
		return true;
	};

	bool bOK = create(m_vertexBuffer, m_buffers.pVertexBeg, m_buffers.pVertexPtr, m_buffers.pVertexEnd, nVertices);
	bOK = create(m_colorBuffer, m_buffers.pColorBeg, m_buffers.pColorPtr, m_buffers.pColorEnd, nColors) && bOK;
	bOK = create(m_extrasBuffer, m_buffers.pExtrasBeg, m_buffers.pExtrasPtr, m_buffers.pExtrasEnd, nExtras) && bOK;

	if( m_renderEncoder )
		_bindEncoderState();

	return bOK;
}

//____ _bindCanvas() _________________________________________________________

bool MetalBackend::_bindCanvas( Surface * pCanvas, SizeI& size, CanvasKind& kind )
{
	if( !m_commandBuffer )
		return false;

	auto pSurface = static_cast<MetalSurface*>(pCanvas);

	if( pSurface && (pSurface->m_texture == nil || (pSurface->m_texture.usage & MTLTextureUsageRenderTarget) == 0) )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Surface can not be used as canvas, rendering to default canvas instead.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		pSurface = nullptr;
	}

	// A render pass per canvas. What was bound belongs to the one that ends.

	_endEncoding();

	m_pCanvasSurface = pSurface;
	m_activePipeline = nil;
	m_boundTexture = nil;
	m_boundSampler = nil;
	m_boundEdgemapOrPalette = nil;

	if( pSurface )
	{
		m_targetFormat = pSurface->m_texture.pixelFormat;
		m_targetSize = pSurface->pixelSize();
		kind = _canvasKind(pSurface);

		// Its mip levels no longer match what we draw into level 0.

		pSurface->m_bMipmapStale = pSurface->m_bMipmapped;

		auto isCanvas = [pSurface](const MetalSurface_p& p) { return p.rawPtr() == pSurface; };

		if( std::none_of(m_sessionCanvases.begin(), m_sessionCanvases.end(), isCanvas) )
			m_sessionCanvases.push_back(pSurface);
	}
	else
	{
		if( m_defaultCanvasRenderPassDesc == nil )
		{
			m_targetSize = SizeI();
			return false;
		}

		// The pipelines must match the texture the render pass draws into.

		id<MTLTexture> texture = m_defaultCanvasRenderPassDesc.colorAttachments[0].texture;

		m_targetFormat = texture ? texture.pixelFormat : m_defaultCanvasFormat;
		m_targetSize = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };

		if( m_defaultCanvas.format == PixelFormat::Alpha_8 )
			kind = CanvasKind::AlphaOnly;
		else if( m_defaultCanvas.format == PixelFormat::XRGB_8 )
			kind = CanvasKind::WithoutAlpha;
		else
			kind = CanvasKind::WithAlpha;
	}

	m_scissor = { 0, 0, NSUInteger(m_targetSize.w), NSUInteger(m_targetSize.h) };
	m_bScissorEmpty = (m_targetSize.w <= 0 || m_targetSize.h <= 0);

	if( !_beginEncoding() )
		return false;

	size = m_targetSize;
	return true;
}

//____ _beginEncoding() ______________________________________________________
//
// Begins a render pass on the canvas, keeping what is in it already. The descriptor of the
// default canvas is the one we were given for the first pass of a session, which may clear it.

bool MetalBackend::_beginEncoding()
{
	if( !m_commandBuffer )
		return false;

	MTLRenderPassDescriptor * pDescriptor;

	if( m_pCanvasSurface )
	{
		pDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];		// Autoreleased.
		pDescriptor.colorAttachments[0].texture = m_pCanvasSurface->m_texture;
		pDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		pDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	}
	else
	{
		if( m_defaultCanvasRenderPassDesc == nil )
			return false;

		if( m_bDefaultCanvasLoaded )
		{
			pDescriptor = [[m_defaultCanvasRenderPassDesc copy] autorelease];
			pDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		}
		else
			pDescriptor = m_defaultCanvasRenderPassDesc;

		m_bDefaultCanvasLoaded = true;
	}

	m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:pDescriptor];		// Autoreleased.

	if( m_renderEncoder == nil )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to begin a render pass.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}

	m_renderEncoder.label = m_pCanvasSurface ? @"MetalBackend Render to Surface Pass" : @"MetalBackend Render Pass";

	_bindEncoderState();
	return true;
}

//____ _endEncoding() ________________________________________________________

void MetalBackend::_endEncoding()
{
	if( m_renderEncoder )
	{
		[m_renderEncoder endEncoding];
		m_renderEncoder = nil;
	}
}

//____ _bindEncoderState() ___________________________________________________
//
// Binds everything to a new render encoder: the buffers of the session and whatever was bound
// to the one before it on the same canvas.

void MetalBackend::_bindEncoderState()
{
	[m_renderEncoder setViewport:(MTLViewport){ 0.0, 0.0, double(m_targetSize.w), double(m_targetSize.h), 0.0, 1.0 }];

	if( !m_bScissorEmpty )
		[m_renderEncoder setScissorRect:m_scissor];

	[m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:BufferIndex::Vertices];
	[m_renderEncoder setVertexBuffer:m_colorBuffer offset:0 atIndex:BufferIndex::Colors];
	[m_renderEncoder setVertexBuffer:m_extrasBuffer offset:0 atIndex:BufferIndex::Extras];
	[m_renderEncoder setFragmentBuffer:m_colorBuffer offset:0 atIndex:BufferIndex::Colors];
	[m_renderEncoder setFragmentBuffer:m_extrasBuffer offset:0 atIndex:BufferIndex::Extras];

	if( m_activePipeline )
		[m_renderEncoder setRenderPipelineState:m_activePipeline];

	if( m_boundTexture )
		[m_renderEncoder setFragmentTexture:m_boundTexture atIndex:c_blitSourceTexture];

	if( m_boundSampler )
		[m_renderEncoder setFragmentSamplerState:m_boundSampler atIndex:c_blitSampler];

	if( m_boundEdgemapOrPalette )
		[m_renderEncoder setFragmentBuffer:m_boundEdgemapOrPalette offset:0 atIndex:BufferIndex::EdgemapOrPalette];

	m_bConstantsDirty = true;
}

//____ _setPipeline() ________________________________________________________

bool MetalBackend::_setPipeline( PipelineKind kind )
{
	if( !m_renderEncoder )
		return false;

	id<MTLRenderPipelineState> pipeline = _pipeline(kind, m_activeBlendMode);
	if( pipeline == nil )
		return false;

	if( pipeline != m_activePipeline )
	{
		[m_renderEncoder setRenderPipelineState:pipeline];
		m_activePipeline = pipeline;
	}

	return true;
}

//____ _pipeline() ___________________________________________________________
//
// The pipeline for the given kind and blend mode on the canvas we are rendering into, created
// the first time a combination shows up.

id<MTLRenderPipelineState> MetalBackend::_pipeline( PipelineKind kind, BlendMode blendMode )
{
	if( blendMode == BlendMode::Ignore )
		return nil;							// Nothing should be drawn.

	BlendMode mode = _normalizeBlendMode(blendMode);

	uint64_t key = (uint64_t(m_targetFormat) << 24) | (uint64_t(m_activeCanvasKind) << 16) | (uint64_t(kind) << 8) | uint64_t(mode);

	auto it = m_pipelines.find(key);
	if( it != m_pipelines.end() )
		return it->second;					// May be nil, if creation failed before.

	id<MTLRenderPipelineState> pipeline = _createPipeline(kind, mode, m_targetFormat, m_activeCanvasKind);
	m_pipelines[key] = pipeline;
	return pipeline;
}

//____ _createPipeline() _____________________________________________________
/*
	Blending as DX12Backend::_createPipeline() and VulkanBackend::_createPipeline() have it, see
	there for why. Canvases with alpha hold premultiplied colors. The pixel shaders pass what they
	draw to the blender as the mode needs it, see output.hlsl, Replace, Morph and Multiply keeping
	the uncovered part of the pixel by the second color of dual-source blending.

	An alpha only canvas is R8 here, as in VulkanBackend. The shaders write the value to all
	channels for it, so the factors of alpha are those of color. Canvases without alpha are
	opaque, their alpha is left alone.
*/

id<MTLRenderPipelineState> MetalBackend::_createPipeline( PipelineKind kind, BlendMode blendMode, MTLPixelFormat format, CanvasKind canvasKind )
{
	bool bAlphaOnly = (canvasKind == CanvasKind::AlphaOnly);
	bool bNoAlpha = (canvasKind == CanvasKind::WithoutAlpha);
	bool bPartialCoverage = _hasPartialCoverage(kind);

	BOOL				bBlend = YES;
	MTLBlendOperation	op = MTLBlendOperationAdd;
	MTLBlendFactor		srcBlend = MTLBlendFactorOne;
	MTLBlendFactor		destBlend = MTLBlendFactorZero;
	bool				bKeepAlpha = false;
	bool				bInvertAlpha = false;

	switch( blendMode )
	{
		case BlendMode::Replace:
			srcBlend = MTLBlendFactorOne;
			destBlend = MTLBlendFactorOneMinusSource1Alpha;
			bBlend = bPartialCoverage ? YES : NO;
			break;

		case BlendMode::Blend:
			srcBlend = MTLBlendFactorOne;
			destBlend = MTLBlendFactorOneMinusSourceAlpha;
			break;

		case BlendMode::Morph:
			srcBlend = MTLBlendFactorOne;
			destBlend = MTLBlendFactorOneMinusSource1Alpha;
			break;

		case BlendMode::Add:
		case BlendMode::Subtract:
			op = (blendMode == BlendMode::Add) ? MTLBlendOperationAdd : MTLBlendOperationReverseSubtract;
			srcBlend = MTLBlendFactorOne;
			destBlend = MTLBlendFactorOne;
			bKeepAlpha = (blendMode == BlendMode::Subtract);
			break;

		case BlendMode::Multiply:
			srcBlend = bAlphaOnly ? MTLBlendFactorDestinationAlpha : MTLBlendFactorDestinationColor;
			destBlend = MTLBlendFactorOneMinusSource1Alpha;
			bKeepAlpha = true;
			break;

		case BlendMode::Invert:
			srcBlend = bAlphaOnly ? MTLBlendFactorOneMinusDestinationAlpha : MTLBlendFactorOneMinusDestinationColor;
			destBlend = bAlphaOnly ? MTLBlendFactorOneMinusSourceAlpha : MTLBlendFactorOneMinusSourceColor;
			bInvertAlpha = !bAlphaOnly;
			break;

		case BlendMode::Min:
		case BlendMode::Max:
			op = (blendMode == BlendMode::Min) ? MTLBlendOperationMin : MTLBlendOperationMax;
			srcBlend = MTLBlendFactorOne;
			destBlend = MTLBlendFactorOne;
			bKeepAlpha = (blendMode == BlendMode::Min);
			break;

		default:
		{
			char buffer[128];
			snprintf(buffer, sizeof(buffer), "BlendMode %d is unknown, using Blend.", (int) blendMode);
			Base::throwError(ErrorLevel::Warning, ErrorCode::Other, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

			srcBlend = MTLBlendFactorOne;
			destBlend = MTLBlendFactorOneMinusSourceAlpha;
			break;
		}
	}

	// The value of an alpha only canvas is in R, as are the alphas the shaders output for it.

	if( bAlphaOnly )
	{
		auto toColor = [](MTLBlendFactor factor)
		{
			switch( factor )
			{
				case MTLBlendFactorSourceAlpha:					return MTLBlendFactorSourceColor;
				case MTLBlendFactorOneMinusSourceAlpha:			return MTLBlendFactorOneMinusSourceColor;
				case MTLBlendFactorDestinationAlpha:			return MTLBlendFactorDestinationColor;
				case MTLBlendFactorOneMinusDestinationAlpha:	return MTLBlendFactorOneMinusDestinationColor;
				case MTLBlendFactorSource1Alpha:				return MTLBlendFactorSource1Color;
				case MTLBlendFactorOneMinusSource1Alpha:		return MTLBlendFactorOneMinusSource1Color;
				default:										return factor;
			}
		};

		srcBlend = toColor(srcBlend);
		destBlend = toColor(destBlend);
		bKeepAlpha = false;
	}

	bool bDualSource = bBlend && (destBlend == MTLBlendFactorOneMinusSource1Alpha || destBlend == MTLBlendFactorOneMinusSource1Color);

	MTLRenderPipelineDescriptor * pDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	MTLRenderPipelineColorAttachmentDescriptor * pAttachment = pDescriptor.colorAttachments[0];

	pAttachment.pixelFormat = format;
	pAttachment.blendingEnabled = bBlend;
	pAttachment.rgbBlendOperation = op;
	pAttachment.sourceRGBBlendFactor = srcBlend;
	pAttachment.destinationRGBBlendFactor = destBlend;

	if( bKeepAlpha )
	{
		pAttachment.alphaBlendOperation = MTLBlendOperationAdd;
		pAttachment.sourceAlphaBlendFactor = MTLBlendFactorZero;
		pAttachment.destinationAlphaBlendFactor = MTLBlendFactorOne;
	}
	else if( bInvertAlpha )
	{
		pAttachment.alphaBlendOperation = MTLBlendOperationAdd;
		pAttachment.sourceAlphaBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
		pAttachment.destinationAlphaBlendFactor = MTLBlendFactorOne;
	}
	else
	{
		// Alpha can't take color factors, but none of these is one, except on an alpha only
		// canvas, which has no alpha.

		auto toAlpha = [](MTLBlendFactor factor)
		{
			switch( factor )
			{
				case MTLBlendFactorSourceColor:					return MTLBlendFactorSourceAlpha;
				case MTLBlendFactorOneMinusSourceColor:			return MTLBlendFactorOneMinusSourceAlpha;
				case MTLBlendFactorDestinationColor:			return MTLBlendFactorDestinationAlpha;
				case MTLBlendFactorOneMinusDestinationColor:	return MTLBlendFactorOneMinusDestinationAlpha;
				case MTLBlendFactorSource1Color:				return MTLBlendFactorSource1Alpha;
				case MTLBlendFactorOneMinusSource1Color:		return MTLBlendFactorOneMinusSource1Alpha;
				default:										return factor;
			}
		};

		pAttachment.alphaBlendOperation = op;
		pAttachment.sourceAlphaBlendFactor = toAlpha(srcBlend);
		pAttachment.destinationAlphaBlendFactor = toAlpha(destBlend);
	}

	// A canvas without alpha has an alpha channel here, which is left alone. Reading it as a blit
	// source gives alpha 1 regardless, see MetalSurface::sampleTexture().

	if( bNoAlpha )
		pAttachment.writeMask = MTLColorWriteMaskRed | MTLColorWriteMaskGreen | MTLColorWriteMaskBlue;

	// Shaders, see compile_shaders.py. Kinds with the same geometry share a vertex shader.

	static const char * const vertexShaders[int(PipelineKind::Size)] = {
		"fillVS", "fillAAVS", "blitVS", "blitVS", "blitVS", "blitVS",
		"lineVS", "segmentsVS", "shapeVS", "shapeVS", "shapeVS", "warpVS", "warpVS", "warpVS" };

	static const char * const pixelShaders[int(PipelineKind::Size)][2] = {
		{ "fillPS", "fillPSDual" },
		{ "fillAAPS", "fillAAPSDual" },
		{ "blitPS", "blitPSDual" },
		{ "blurPS", "blurPSDual" },
		{ "paletteBlitPS", "paletteBlitPSDual" },
		{ "paletteBlurPS", "paletteBlurPSDual" },
		{ "linePS", "linePSDual" },
		{ "segmentsPS", "segmentsPSDual" },
		{ "shapeBoxPS", "shapeBoxPSDual" },
		{ "shapeRingPS", "shapeRingPSDual" },
		{ "shapeLinePS", "shapeLinePSDual" },
		{ "warpEdgemapPS", "warpEdgemapPSDual" },
		{ "warpBlitPS", "warpBlitPSDual" },
		{ "warpPaletteBlitPS", "warpPaletteBlitPSDual" } };

	pDescriptor.vertexFunction = _function(vertexShaders[int(kind)]);
	pDescriptor.fragmentFunction = _function(pixelShaders[int(kind)][bDualSource ? 1 : 0]);

	// One vertex layout for all but warps, whose vertex shader reads the vertices of the Warpmap
	// itself. Only the attributes the vertex shader reads, see ../hlsl/README.md.

	MTLVertexDescriptor * pVertexDescriptor = nil;

	if( !_isWarp(kind) )
	{
		pVertexDescriptor = [[MTLVertexDescriptor alloc] init];

		int nAttributes;

		switch( kind )
		{
			case PipelineKind::Fill:		nAttributes = 2; break;
			case PipelineKind::Segments:	nAttributes = 4; break;
			default:						nAttributes = 3; break;
		}

		const MTLVertexFormat formats[4] = { MTLVertexFormatFloat2, MTLVertexFormatUInt, MTLVertexFormatUInt, MTLVertexFormatFloat2 };
		const NSUInteger offsets[4] = { offsetof(Vertex, x), offsetof(Vertex, colorOfs), offsetof(Vertex, extrasOfs), offsetof(Vertex, u) };

		for( int i = 0 ; i < nAttributes ; i++ )
		{
			pVertexDescriptor.attributes[i].format = formats[i];
			pVertexDescriptor.attributes[i].offset = offsets[i];
			pVertexDescriptor.attributes[i].bufferIndex = BufferIndex::Vertices;
		}

		pVertexDescriptor.layouts[BufferIndex::Vertices].stride = sizeof(Vertex);
		pVertexDescriptor.layouts[BufferIndex::Vertices].stepFunction = MTLVertexStepFunctionPerVertex;

		pDescriptor.vertexDescriptor = pVertexDescriptor;
	}

	NSError * pError = nil;			// Autoreleased.
	id<MTLRenderPipelineState> pipeline = nil;

	if( pDescriptor.vertexFunction != nil && pDescriptor.fragmentFunction != nil )
		pipeline = [s_metalDevice newRenderPipelineStateWithDescriptor:pDescriptor error:&pError];

	if( pipeline == nil )
	{
		NSString * pMessage = [NSString stringWithFormat:@"Failed to create pipeline %s/%s: %@", vertexShaders[int(kind)], pixelShaders[int(kind)][bDualSource ? 1 : 0],
							   pError ? pError.localizedDescription : @"shader function missing"];
		Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, pMessage.UTF8String, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	[pVertexDescriptor release];
	[pDescriptor release];

	return pipeline;
}

//____ _function() ___________________________________________________________
//
// A shader of wax_metalshaders.mm, compiled the first time it is asked for. Each is a library of
// its own, as the shaders declare the same types. Math is kept as precise as in DX12 and Vulkan,
// not Metal's default fast math, which the shaders aren't written for.

id<MTLFunction> MetalBackend::_function( const char * pName )
{
	auto it = m_functions.find(pName);
	if( it != m_functions.end() )
		return it->second;

	const char * pSource = nullptr;

	for( int i = 0 ; i < s_nbShaderSources ; i++ )
	{
		if( std::strcmp(s_shaderSources[i].pName, pName) == 0 )
		{
			pSource = s_shaderSources[i].pSource;
			break;
		}
	}

	id<MTLFunction> function = nil;

	if( pSource )
	{
		MTLCompileOptions * pOptions = [[MTLCompileOptions alloc] init];
		pOptions.languageVersion = MTLLanguageVersion2_3;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		pOptions.fastMathEnabled = NO;
#pragma clang diagnostic pop

		NSString * pString = [[NSString alloc] initWithUTF8String:pSource];
		NSError * pError = nil;		// Autoreleased.

		id<MTLLibrary> library = [s_metalDevice newLibraryWithSource:pString options:pOptions error:&pError];

		[pString release];
		[pOptions release];

		if( library != nil )
		{
			m_libraries.push_back(library);

			NSString * pFunctionName = [[NSString alloc] initWithUTF8String:pName];
			function = [library newFunctionWithName:pFunctionName];
			[pFunctionName release];
		}

		if( function == nil )
		{
			NSString * pMessage = [NSString stringWithFormat:@"Failed to compile shader %s: %@", pName, pError ? pError.localizedDescription : @"function missing"];
			Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, pMessage.UTF8String, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}
	}
	else
	{
		char buffer[128];
		snprintf(buffer, sizeof(buffer), "Shader %s is missing, run compile_shaders.py.", pName);
		Base::throwError(ErrorLevel::Error, ErrorCode::Internal, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	m_functions[pName] = function;			// Kept also when nil, so we don't try again and again.
	return function;
}

//____ _setConstants() _______________________________________________________
//
// The shaders have the three blocks as one, see ShaderConstants, which is sent with the next draw.

void MetalBackend::_setConstants( ConstantBlock block )
{
	static_assert(sizeof(CanvasConstants) == 32 && sizeof(PixelConstants) == 16 && sizeof(WarpConstants) == 32, "Constant blocks must match constants.hlsl");
	static_assert(offsetof(ShaderConstants, pixel) == 32 && offsetof(ShaderConstants, warp) == 48, "Constant blocks must be where constants.hlsl has them");

	switch( block )
	{
		case ConstantBlock::Canvas:
			m_constants.canvas = m_canvasConstants;
			break;

		case ConstantBlock::Pixel:
			m_constants.pixel = m_pixelConstants;
			break;

		case ConstantBlock::Warp:
			m_constants.warp = m_warpConstants;
			break;
	}

	m_bConstantsDirty = true;
}

//____ _flushConstants() _____________________________________________________

void MetalBackend::_flushConstants()
{
	if( !m_bConstantsDirty || !m_renderEncoder )
		return;

	[m_renderEncoder setVertexBytes:&m_constants length:sizeof(ShaderConstants) atIndex:BufferIndex::Constants];
	[m_renderEncoder setFragmentBytes:&m_constants length:sizeof(ShaderConstants) atIndex:BufferIndex::Constants];

	m_bConstantsDirty = false;
}

//____ _draw() _______________________________________________________________

void MetalBackend::_draw( int firstVertex, int nVertices )
{
	if( !m_renderEncoder || nVertices <= 0 || m_bScissorEmpty )
		return;

	_flushConstants();
	[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:NSUInteger(firstVertex) vertexCount:NSUInteger(nVertices)];
}

//____ _setScissor() _________________________________________________________
//
// Metal takes no scissor rect reaching outside the canvas, nor an empty one, so for one that is
// empty nothing is drawn instead.

void MetalBackend::_setScissor( const RectI& rect )
{
	int x1 = std::max(rect.x, 0);
	int y1 = std::max(rect.y, 0);
	int x2 = std::min(rect.x + rect.w, m_targetSize.w);
	int y2 = std::min(rect.y + rect.h, m_targetSize.h);

	m_bScissorEmpty = (x2 <= x1 || y2 <= y1);

	if( m_bScissorEmpty )
		return;

	m_scissor = { NSUInteger(x1), NSUInteger(y1), NSUInteger(x2 - x1), NSUInteger(y2 - y1) };

	if( m_renderEncoder )
		[m_renderEncoder setScissorRect:m_scissor];
}

//____ _resetScissor() _______________________________________________________

void MetalBackend::_resetScissor()
{
	_setScissor( RectI(0, 0, m_targetSize.w, m_targetSize.h) );
}

//____ _prepareBlitSource() __________________________________________________
//
// MetalSurface copies what is written to its pixels to the texture right away.

void MetalBackend::_prepareBlitSource( Surface * pSurface )
{
}

//____ _bindBlitTexture() ____________________________________________________

bool MetalBackend::_bindBlitTexture( Surface * pSurface )
{
	auto pSource = static_cast<MetalSurface*>(pSurface);

	if( !m_renderEncoder || pSource->m_texture == nil )
		return false;

	// Mip levels that no longer match level 0 are drawn before anything reads them, between
	// render passes on the canvas.

	if( pSource->m_bMipmapStale )
	{
		_endEncoding();
		_generateMipmaps(pSource);

		if( !_beginEncoding() )
			return false;
	}

	id<MTLTexture> texture = pSource->sampleTexture();

	[m_renderEncoder setFragmentTexture:texture atIndex:c_blitSourceTexture];
	m_boundTexture = texture;

	// A palette based source has its palette where edgemaps go, which is free while we blit.
	// Drawing an edgemap clears m_bBlitSourceBound, so it is put back afterwards.

	if( pSource->pixelDescription()->type == PixelType::Index )
	{
		id<MTLBuffer> palette = pSource->paletteColors();

		if( palette == nil )
			return false;

		[m_renderEncoder setFragmentBuffer:palette offset:0 atIndex:BufferIndex::EdgemapOrPalette];
		m_boundEdgemapOrPalette = palette;
	}

	return true;
}

//____ _bindBlitSampler() ____________________________________________________

void MetalBackend::_bindBlitSampler( Surface * pSurface, bool bClip )
{
	if( !m_renderEncoder )
		return;

	int index = (bClip ? 4 : 0) + (pSurface->isTiling() ? 2 : 0) + (pSurface->sampleMethod() == SampleMethod::Bilinear ? 1 : 0);

	[m_renderEncoder setFragmentSamplerState:m_samplers[index] atIndex:c_blitSampler];
	m_boundSampler = m_samplers[index];
}

//____ _bindEdgemap() ________________________________________________________

bool MetalBackend::_bindEdgemap( GpuEdgemap * pEdgemap )
{
	auto pMetalEdgemap = static_cast<MetalEdgemap*>(pEdgemap);

	if( !m_renderEncoder || pMetalEdgemap->m_buffer == nil )
		return false;

	[m_renderEncoder setFragmentBuffer:pMetalEdgemap->m_buffer offset:0 atIndex:BufferIndex::EdgemapOrPalette];
	m_boundEdgemapOrPalette = pMetalEdgemap->m_buffer;
	return true;
}

//____ _drawWarpmap() ________________________________________________________

void MetalBackend::_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches )
{
	auto pMetalWarpmap = static_cast<MetalWarpmap*>(pWarpmap);

	if( !m_renderEncoder || pMetalWarpmap->m_bufferId == nil || pWarpmap->triangleCount() == 0 )
		return;

	[m_renderEncoder setVertexBuffer:pMetalWarpmap->m_bufferId offset:0 atIndex:BufferIndex::WarpVertices];
	_flushConstants();

	for( int i = 0 ; i < nPatches ; i++ )
	{
		_setScissor(pPatches[i]);

		if( !m_bScissorEmpty )
			[m_renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:NSUInteger(pWarpmap->triangleCount()) * 3
										 indexType:MTLIndexTypeUInt16 indexBuffer:pMetalWarpmap->m_bufferId indexBufferOffset:pMetalWarpmap->m_indexOffset];
	}

	_resetScissor();
}

//____ _generateMipmaps() _____________________________________________________
//
// Draws each mip level of the surface from the one above it, in render passes
// of their own on our command buffer. Metal's generateMipmapsForTexture would
// do it for us, but makes no promise to filter an _sRGB texture in linear
// space, and on some GPUs doesn't: a fine black and white pattern came out a
// darker grey than on GL and DX12. Called between render passes.

void MetalBackend::_generateMipmaps( MetalSurface * pSurface )
{
	pSurface->m_bMipmapStale = false;

	id<MTLTexture> texture = pSurface->m_texture;
	int nLevels = (int) texture.mipmapLevelCount;

	if( nLevels < 2 || !m_commandBuffer )
		return;

	id<MTLRenderPipelineState> pipeline = _mipmapPipeline(texture.pixelFormat);

	if( pipeline == nil )
		return;

	for( int level = 1 ; level < nLevels ; level++ )
	{
		id<MTLTexture> source = [texture newTextureViewWithPixelFormat:texture.pixelFormat
														   textureType:MTLTextureType2D
																levels:NSMakeRange(level - 1, 1)
																slices:NSMakeRange(0, 1)];

		MTLRenderPassDescriptor* pDescriptor = [MTLRenderPassDescriptor new];

		pDescriptor.colorAttachments[0].texture = texture;
		pDescriptor.colorAttachments[0].level = level;
		pDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
		pDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		id<MTLRenderCommandEncoder> renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:pDescriptor];
		renderEncoder.label = @"MetalBackend Mipmap Pass";
		[pDescriptor release];

		[renderEncoder setRenderPipelineState:pipeline];
		[renderEncoder setFragmentTexture:source atIndex:c_blitSourceTexture];
		[renderEncoder setFragmentSamplerState:m_samplers[1] atIndex:c_blitSampler];		// Bilinear, clamped.
		[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
		[renderEncoder endEncoding];

		[source release];			// The command buffer keeps its own reference until it is done.
	}
}

//____ _mipmapPipeline() ______________________________________________________
//
// One per texture format, created on first use and kept. No blending, no
// vertex buffer, the vertex shader makes its triangle from the vertex ID.

id<MTLRenderPipelineState> MetalBackend::_mipmapPipeline( MTLPixelFormat format )
{
	auto it = m_mipmapPipelines.find(format);
	if( it != m_mipmapPipelines.end() )
		return it->second;

	NSError *error = nil;			// Autoreleased.
	MTLRenderPipelineDescriptor *descriptor = [[MTLRenderPipelineDescriptor alloc] init];

	descriptor.label = @"Mipmap Pipeline";
	descriptor.vertexFunction = _function("mipmapVS");
	descriptor.fragmentFunction = _function("mipmapPS");
	descriptor.colorAttachments[0].pixelFormat = format;
	descriptor.colorAttachments[0].blendingEnabled = NO;

	id<MTLRenderPipelineState> pipelineState = nil;

	if( descriptor.vertexFunction != nil && descriptor.fragmentFunction != nil )
		pipelineState = [s_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];

	if( pipelineState == nil )
	{
		NSString * pMessage = [NSString stringWithFormat:@"Failed to create Mipmap Pipeline: %@", error ? error.localizedDescription : @"shader function missing"];
		Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, pMessage.UTF8String, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	[descriptor release];

	m_mipmapPipelines[format] = pipelineState;		// Kept also when nil, so we don't try again and again.
	return pipelineState;
}

} //namespace wax
