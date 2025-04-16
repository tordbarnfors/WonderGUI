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

#include <wg_metalbackend.h>
#include <wg_metalsurfacefactory.h>
#include <wg_metaledgemapfactory.h>
#include <wg_metaledgemap.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>


namespace wg
{
using namespace Util;

const TypeInfo MetalBackend::TYPEINFO = { "MetalBackend", &GfxBackend::TYPEINFO };

id<MTLDevice>       MetalBackend::s_metalDevice = nil;
id<MTLCommandQueue> MetalBackend::s_metalCommandQueue = nil;

const int MetalBackend::s_flipCornerOrder[GfxFlip_size][4] = {
	{ 0,1,2,3 },			// Normal
	{ 1,0,3,2 },			// FlipX
	{ 3,2,1,0 },			// FlipY
	{ 3,0,1,2 },			// Rot90
	{ 0,3,2,1 },			// Rot90FlipX
	{ 2,1,0,3 },			// Rot90FlipY
	{ 2,3,0,1 },			// Rot180
	{ 3,2,1,0 },			// Rot180FlipX
	{ 1,0,3,2 },			// Rot180FlipY
	{ 1,2,3,0 },			// Rot270
	{ 2,1,0,3 },			// Rot270FlipX
	{ 0,3,2,1 }				// Rot270FlipY
};


//____ setMetalDevice() ______________________________________________________

void MetalBackend::setMetalDevice( id<MTLDevice> device )
{
	s_metalDevice = device;

	[s_metalCommandQueue release];

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
	assert(sizeof(Gradient) % 4 == 0);		// Buffering will fail if not!

	m_defaultCanvas.ref = CanvasRef::Default;

	m_bFullyInitialized = true;

	m_flushesInProgress = 0;

	_initTables();

	// Modify shader source

	std::string shaderSource = shaders;
	auto maxSegPos = shaderSource.find("$MAXSEG");

	while(maxSegPos != std::string::npos )
	{
		shaderSource.replace(maxSegPos, 7, std::to_string(c_maxSegments));
		maxSegPos = shaderSource.find("$MAXSEG");
	}

	// Create shader library

	NSError *error = nil;
	NSString *shaderString = [[NSString alloc] initWithUTF8String:shaderSource.c_str()];
	m_library = [s_metalDevice newLibraryWithSource:shaderString options:nil error:&error];

	[error release];
	[shaderString release];

	// Create and init Line pipelines

	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		//            if( blendMode != int(BlendMode::Ignore) && blendMode != int(BlendMode::Undefined) )
		{
			m_linePipelines[blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Line BGRA_8_linear Pipeline", @"lineVertexShader", @"lineFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_linePipelines[blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Line BGRX_8_linear Pipeline", @"lineVertexShader", @"lineFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_linePipelines[blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"Line BGRA_8_sRGB Pipeline", @"lineVertexShader", @"lineFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_linePipelines[blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"Line BGRX_8_sRGB Pipeline", @"lineVertexShader", @"lineFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_linePipelines[blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"Line A_8 Pipeline", @"lineVertexShader", @"lineFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );
		}
	}

	// Create and init Fill pipelines

	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		//            if( blendMode != int(BlendMode::Ignore) && blendMode != int(BlendMode::Undefined) )
		{
			m_fillPipelines[0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Fill BGRA_8_linear Pipeline", @"fillVertexShader", @"fillFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_fillPipelines[0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Fill BGRX_8_linear Pipeline", @"fillVertexShader", @"fillFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_fillPipelines[0][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"Fill BGRA_8_sRGB Pipeline", @"fillVertexShader", @"fillFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_fillPipelines[0][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"Fill BGRX_8_sRGB Pipeline", @"fillVertexShader", @"fillFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_fillPipelines[0][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"Fill A_8 Pipeline", @"fillVertexShader", @"fillFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_fillPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"TintmapFill BGRA_8_linear Pipeline", @"fillTintmapVertexShader", @"fillTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_fillPipelines[1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"TintmapFill BGRX_8_linear Pipeline", @"fillTintmapVertexShader", @"fillTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_fillPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"TintmapFill BGRA_8_sRGB Pipeline", @"fillTintmapVertexShader", @"fillTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_fillPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"TintmapFill BGRX_8_sRGB Pipeline", @"fillTintmapVertexShader", @"fillTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_fillPipelines[1][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"TintmapFill A_8 Pipeline", @"fillTintmapVertexShader", @"fillTintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"AAFill BGRA_8_linear Pipeline", @"fillAAVertexShader", @"fillAAFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"AAFill BGRX_8_linear Pipeline", @"fillAAVertexShader", @"fillAAFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"AAFill BGRA_8_sRGB Pipeline", @"fillAAVertexShader", @"fillAAFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"AAFill BGRX_8_sRGB Pipeline", @"fillAAVertexShader", @"fillAAFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_fillAAPipelines[0][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"AAFill A_8 Pipeline", @"fillAAVertexShader", @"fillAAFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"AATintmapFill BGRA_8_linear Pipeline", @"fillAATintmapVertexShader", @"fillAATintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"AATintmapFill BGRX_8_linear Pipeline", @"fillAATintmapVertexShader", @"fillAATintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"AATintmapFill BGRA_8_sRGB Pipeline", @"fillAATintmapVertexShader", @"fillAATintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"AATintmapFill BGRX_8_sRGB Pipeline", @"fillAATintmapVertexShader", @"fillAATintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_fillAAPipelines[1][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"AATintmapFill A_8 Pipeline", @"fillAATintmapVertexShader", @"fillAATintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );
		}
	}

	// Create and init Blit pipelines

	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		//            if( blendMode != int(BlendMode::Ignore) && blendMode != int(BlendMode::Undefined) )
		{

			// [BlitFragShader][bGradient][BlendMode][DestFormat]

			m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Blit BGRA_8_linear Pipeline", @"blitVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Blit BGRX_8_linear Pipeline", @"blitVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"Blit BGRA_8_sRGB Pipeline", @"blitVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"Blit BGRX_8_sRGB Pipeline", @"blitVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"Blit A_8 Pipeline", @"blitVertexShader", @"blitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Blit BGRA_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"blitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Blit BGRX_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"blitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"Blit BGRA_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"blitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"Blit BGRX_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"blitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"Blit A_8 Tintmap Pipeline", @"blitTintmapVertexShader", @"blitTintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_linear Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_linear Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_sRGB Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_sRGB Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitNearest A_8 Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"paletteBlitNearestTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"paletteBlitNearestTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"paletteBlitNearestTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"paletteBlitNearestTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitNearest A_8 Tintmap Pipeline", @"blitTintmapVertexShader", @"paletteBlitNearestTintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_linear Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_linear Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_sRGB Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_sRGB Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitInterpolated A_8 Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_linear Tintmap Pipeline", @"paletteBlitInterpolateTintmapVertexShader", @"paletteBlitInterpolateTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_linear Tintmap Pipeline", @"paletteBlitInterpolateTintmapVertexShader", @"paletteBlitInterpolateTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_sRGB Tintmap Pipeline", @"paletteBlitInterpolateTintmapVertexShader", @"paletteBlitInterpolateTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_sRGB Tintmap Pipeline", @"paletteBlitInterpolateTintmapVertexShader", @"paletteBlitInterpolateTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitInterpolated A_8 Tintmap Pipeline", @"paletteBlitInterpolateTintmapVertexShader", @"paletteBlitInterpolateTintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRA_8_linear Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRX_8_linear Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRA_8_sRGB Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRX_8_sRGB Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"A8SourceBlit A_8 Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRA_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"alphaBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRX_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"alphaBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRA_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"alphaBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRX_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"alphaBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"A8SourceBlit A_8 Tintmap Pipeline", @"blitTintmapVertexShader", @"alphaBlitTintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_linear Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_linear Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_sRGB Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_sRGB Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"RGBXSourceBlit A_8 Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"rgbxBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"rgbxBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"rgbxBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"rgbxBlitTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"RGBXSourceBlit A_8 Tintmap Pipeline", @"blitTintmapVertexShader", @"rgbxBlitTintmapFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );
		}
	}

	// Create and init Blur pipelines

	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		//            if( blendMode != int(BlendMode::Ignore) && blendMode != int(BlendMode::Undefined) )
		{

			// [BlitFragShader][bGradient][BlendMode][DestFormat]

			m_blurPipelines[0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Blur BGRA_8_linear Pipeline", @"blitVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blurPipelines[0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Blur BGRX_8_linear Pipeline", @"blitVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blurPipelines[0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"Blur BGRA_8_sRGB Pipeline", @"blitVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blurPipelines[0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"Blur BGRX_8_sRGB Pipeline", @"blitVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			//			m_blurPipelines[0][blendMode][(int)DestFormat::Alpha_8]      = _compileRenderPipeline( @"Blit A_8 Pipeline", @"blitVertexShader", @"blurFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_blurPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Blur BGRA_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"blurTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blurPipelines[1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Blur BGRX_8_linear Tintmap Pipeline", @"blitTintmapVertexShader", @"blurTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blurPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"Blur BGRA_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"blurTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blurPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"Blur BGRX_8_sRGB Tintmap Pipeline", @"blitTintmapVertexShader", @"blurTintmapFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
			//			m_blitPipelines[1][blendMode][(int)DestFormat::Alpha_8]      = _compileRenderPipeline( @"Blit A_8 Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );
		}
	}



	// Create and init Segments pipelines


	NSString * segFragShaders[16] = { nil, @"segmentsFragmentShader1", @"segmentsFragmentShader2", @"segmentsFragmentShader3",
		@"segmentsFragmentShader4", @"segmentsFragmentShader5", @"segmentsFragmentShader6", @"segmentsFragmentShader7",
		@"segmentsFragmentShader8", @"segmentsFragmentShader9", @"segmentsFragmentShader10", @"segmentsFragmentShader11",
		@"segmentsFragmentShader12", @"segmentsFragmentShader13", @"segmentsFragmentShader14", @"segmentsFragmentShader15" };

	NSString * segTintmapFragShaders[16] = { nil, @"segmentsTintmapFragmentShader1", @"segmentsTintmapFragmentShader2", @"segmentsTintmapFragmentShader3",
		@"segmentsTintmapFragmentShader4", @"segmentsTintmapFragmentShader5", @"segmentsTintmapFragmentShader6", @"segmentsTintmapFragmentShader7",
		@"segmentsFragmentShader8", @"segmentsFragmentShader9", @"segmentsFragmentShader10", @"segmentsFragmentShader11",
		@"segmentsTintmapFragmentShader12", @"segmentsTintmapFragmentShader13", @"segmentsTintmapFragmentShader14", @"segmentsTintmapFragmentShader15" };


	NSString * segFragShaders_A8[16] = { nil, @"segmentsFragmentShader1_A8", @"segmentsFragmentShader2_A8", @"segmentsFragmentShader3_A8",
		@"segmentsFragmentShader4_A8", @"segmentsFragmentShader5_A8", @"segmentsFragmentShader6_A8", @"segmentsFragmentShader7_A8",
		@"segmentsFragmentShader8_A8", @"segmentsFragmentShader9_A8", @"segmentsFragmentShader10_A8", @"segmentsFragmentShader11_A8",
		@"segmentsFragmentShader12_A8", @"segmentsFragmentShader13_A8", @"segmentsFragmentShader14_A8", @"segmentsFragmentShader15_A8" };

	NSString * segTintmapFragShaders_A8[16] = { nil, @"segmentsTintmapFragmentShader1_A8", @"segmentsTintmapFragmentShader2_A8", @"segmentsTintmapFragmentShader3_A8",
		@"segmentsTintmapFragmentShader4_A8", @"segmentsTintmapFragmentShader5_A8", @"segmentsTintmapFragmentShader6_A8", @"segmentsTintmapFragmentShader7_A8",
		@"segmentsTintmapFragmentShader8_A8", @"segmentsTintmapFragmentShader9_A8", @"segmentsTintmapFragmentShader10_A8", @"segmentsTintmapFragmentShader11_A8",
		@"segmentsTintmapFragmentShader12_A8", @"segmentsTintmapFragmentShader13_A8", @"segmentsTintmapFragmentShader14_A8", @"segmentsTintmapFragmentShader15_A8" };


	int maxSegments = c_maxSegments;				// std::min can't operate on static const only present in header. Does some introspection that fails then.
	int nbShaders = std::min(maxSegments, 16);

	for( int shader = 1 ; shader < nbShaders ; shader++ )
	{
		for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
		{
			m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Segments BGRA_8_linear pipeline", @"segmentsVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Segments BGRX_8_linear pipeline", @"segmentsVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRX_8_linear );

			m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"Segments BGRA_8_sRGB pipeline", @"segmentsVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"Segments BGRX_8_sRGB pipeline", @"segmentsVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );

			m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"Segments A_8 pipeline", @"segmentsVertexShader", segFragShaders_A8[shader], (BlendMode) blendMode, PixelFormat::Alpha_8 );

			m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Segments BGRA_8_linear gradient pipeline", @"segmentsVertexShader", segTintmapFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Segments BGRX_8_linear gradient pipeline", @"segmentsVertexShader", segTintmapFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRX_8_linear );

			m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"Segments BGRA_8_sRGB gradient pipeline", @"segmentsVertexShader", segTintmapFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"Segments BGRX_8_sRGB gradient pipeline", @"segmentsVertexShader", segTintmapFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );

			m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"Segments A_8 gradient pipeline", @"segmentsVertexShader", segTintmapFragShaders_A8[shader], (BlendMode) blendMode, PixelFormat::Alpha_8 );
		}
	}

	// Create samplers

	MTLSamplerDescriptor *desc = [MTLSamplerDescriptor new];

	for( int mipmapped = 0 ; mipmapped < 2 ; mipmapped++ )
	{
		desc.mipFilter = (mipmapped == 0) ? MTLSamplerMipFilterNotMipmapped : MTLSamplerMipFilterLinear;

		for( int interpolated = 0 ; interpolated < 2 ; interpolated++ )
		{
			MTLSamplerMinMagFilter filter = (interpolated == 0) ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
			desc.minFilter = filter;
			desc.magFilter = filter;

			for( int tiled = 0 ; tiled < 2 ; tiled++ )
			{
				MTLSamplerAddressMode addressMode = (tiled == 0) ? MTLSamplerAddressModeClampToZero : MTLSamplerAddressModeRepeat;
				desc.rAddressMode = addressMode;
				desc.sAddressMode = addressMode;
				desc.tAddressMode = addressMode;

				m_samplers[mipmapped][interpolated][tiled] = [s_metalDevice newSamplerStateWithDescriptor:desc];
			}
		}
	}

	[desc release];

	// Initialize our shader environment

	m_uniform.flatTint = { 1.f, 1.f, 1.f, 1.f };
}

//____ destructor() _______________________________________________________

MetalBackend::~MetalBackend()
{
	[m_defaultCanvasRenderPassDesc release];
	m_drawableToAutoPresent = nil;

	[m_vertexBufferId release];
	m_vertexBufferId = nil;

	[m_extrasBufferId release];
	m_extrasBufferId = nil;


	for( int mipmapped = 0 ; mipmapped < 2 ; mipmapped++ )
	{
		for( int interpolated = 0 ; interpolated < 2 ; interpolated++ )
		{
			for( int tiled = 0 ; tiled < 2 ; tiled++ )
			{
				[m_samplers[mipmapped][interpolated][tiled] release];
			}
		}
	}


	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		[m_linePipelines[blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_linePipelines[blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_linePipelines[blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_linePipelines[blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_linePipelines[blendMode][(int)DestFormat::Alpha_8] release];
	}

	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		[m_fillPipelines[0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_fillPipelines[0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_fillPipelines[0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_fillPipelines[0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_fillPipelines[0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_fillPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_fillPipelines[1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_fillPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_fillPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_fillPipelines[1][blendMode][(int)DestFormat::Alpha_8] release];

		[m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_fillAAPipelines[0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_fillAAPipelines[0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_fillAAPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_fillAAPipelines[1][blendMode][(int)DestFormat::Alpha_8] release];
	}

	// Create and init Blit pipelines

	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		[m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::Alpha_8] release];

		[m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
		[m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::Alpha_8] release];
	}


	for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
	{
		[m_blurPipelines[0][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blurPipelines[0][blendMode][(int)DestFormat::BGRX8_linear] release];
		[m_blurPipelines[0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blurPipelines[0][blendMode][(int)DestFormat::BGRX8_sRGB]  release];

		[m_blurPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] release];
		[m_blurPipelines[1][blendMode][(int)DestFormat::BGRX8_linear]  release];
		[m_blurPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
		[m_blurPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB] release];
	}

	int maxSegments = c_maxSegments;                // std::min can't operate on static const only present in header. Does some introspection that fails then.
	int nbShaders = std::min(maxSegments, 16);
	for( int shader = 1 ; shader < nbShaders ; shader++ )
	{
		for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
		{
			[m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRA8_linear] release];
			[m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRX8_linear] release];

			[m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRA8_sRGB] release];
			[m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::BGRX8_sRGB] release];

			[m_segmentsPipelines[shader][0][blendMode][(int)DestFormat::Alpha_8] release];

			[m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRA8_linear] release];
			[m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRX8_linear] release];

			[m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRA8_sRGB] release];
			[m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRX8_sRGB] release];

			[m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::Alpha_8] release];
		}
	}

	[m_library release];
}

//____ typeInfo() _________________________________________________________

const TypeInfo& MetalBackend::typeInfo(void) const
{
	return TYPEINFO;
}

//____ maxEdges() _____________________________________________

int MetalBackend::maxEdges() const
{
	return c_maxSegments - 1;
}

//____ surfaceType() _______________________________________________________

const TypeInfo& MetalBackend::surfaceType( void ) const
{
	return MetalSurface::TYPEINFO;
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

//____ canvasInfo() ____________________________________________________________

const CanvasInfo * MetalBackend::canvasInfo(CanvasRef ref) const
{
	if( ref == CanvasRef::Default )
		return &m_defaultCanvas;
	else
		return nullptr;
}

//____ setDefaultCanvas() ___________________________________________________

bool MetalBackend::setDefaultCanvas( MTLRenderPassDescriptor* renderPassDesc, SizeI pixelSize, PixelFormat pixelFormat, int scale )
{
	if( pixelFormat != PixelFormat::BGRA_8 && pixelFormat != PixelFormat::BGRA_8_linear && pixelFormat != PixelFormat::BGRA_8_sRGB && pixelFormat != PixelFormat::Alpha_8 )
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "pixelFormat must be BGRA_8, BGRA_8_linear, BGRA_8_sRGB, BGRX_8, BGRX_8_linear, BGRX_8_sRGB or A_8", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}

	if( pixelFormat == PixelFormat::BGRA_8 )
		pixelFormat = GfxBase::defaultToSRGB() ? PixelFormat::BGRA_8_sRGB : PixelFormat::BGRA_8_linear;

	if( pixelFormat == PixelFormat::BGRX_8 )
		pixelFormat = GfxBase::defaultToSRGB() ? PixelFormat::BGRX_8_sRGB : PixelFormat::BGRX_8_linear;


	if( m_defaultCanvasRenderPassDesc )
		[m_defaultCanvasRenderPassDesc release];
	m_defaultCanvasRenderPassDesc = renderPassDesc;
	m_defaultCanvasPixelFormat = pixelFormat;
	[m_defaultCanvasRenderPassDesc retain];
	m_defaultCanvas.size = pixelSize*64;
	m_defaultCanvas.scale = scale;

	return true;
}

//____ beginRender() _________________________________________________________

void MetalBackend::beginRender()
{

}

//____ endRender() ___________________________________________________________

void MetalBackend::endRender()
{

}

//____ beginSession() ________________________________________________________

void MetalBackend::beginSession( CanvasRef canvasRef, Surface * pCanvasSurface, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo )
{
	if( !pCanvasSurface && canvasRef != CanvasRef::Default )
		return;

//	m_bFullCanvasSession = (nUpdateRects == 1 && pUpdateRects[0] == RectSPX(0,0,pCanvasSurface->pixelSize()*64) );

	// Reserve buffer for coordinates

	int nVertices = pInfo->nRects * 6 + pInfo->nLineCoords/2 * 6;

	m_vertexBufferId = [s_metalDevice newBufferWithLength:nVertices*sizeof(VertexMTL) options:MTLResourceStorageModeShared];
	m_pVertexBuffer = (VertexMTL *)[m_vertexBufferId contents];
	m_nVertices = 0;

	// Reserve buffer for colors

	int nColors = pInfo->nColors+1;

	m_colorBufferId = [s_metalDevice newBufferWithLength:nColors*sizeof(ColorMTL) options:MTLResourceStorageModeShared];
	m_pColorBuffer = (ColorMTL *)[m_colorBufferId contents];

	// Always present white color used as default tint for blits.

	m_pColorBuffer[0].r = 1.f;
	m_pColorBuffer[0].g = 1.f;
	m_pColorBuffer[0].b = 1.f;
	m_pColorBuffer[0].a = 1.f;

	m_nColors = 1;

	// Reserve buffer for extras

	int nExtrasFloats =
	pInfo->nBlit * 8
	+ pInfo->nBlur * 8
	+ pInfo->nLineCoords/2 * 4
	+ pInfo->nRects * 4;			// This is for possible subpixel fills. We have no way of knowing exactly how much is needed.

	m_extrasBufferId = [s_metalDevice newBufferWithLength:nExtrasFloats*sizeof(float) options:MTLResourceStorageModeShared];
	m_pExtrasBuffer = (float *)[m_extrasBufferId contents];
	m_extrasBufferSize = 0;

	// Setup a metal command queue for this session

	m_metalCommandBuffer = [s_metalCommandQueue commandBuffer];
	m_metalCommandBuffer.label = @"MetalBackend";

	//

	if( pCanvasSurface )
		setCanvas(pCanvasSurface);
	else
		setCanvas( canvasRef );


	// Reset states to start values

	m_tintColorOfs 			= -1;
	m_bTintmap 				= false;

	m_pActiveBlitSource		= nullptr;
	m_activeBlendMode		= BlendMode::Blend;
	m_activeMorphFactor		= 0.5f;
	m_activeFixedBlendColor = HiColor::White;
	m_activeBlurRadius		= 64;

	m_morphFactorInUse		= -1;
	m_fixedBlendColorInUse	= HiColor::Undefined;

	// Reset uniform to start state

	m_uniform.flatTint[0] = 1.f;
	m_uniform.flatTint[1] = 1.f;
	m_uniform.flatTint[2] = 1.f;
	m_uniform.flatTint[3] = 1.f;

	[m_renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex:(unsigned) VertexInputIndex::Uniform];

}

//____ endSession() __________________________________________________________

void MetalBackend::endSession()
{
	//

	[m_renderEncoder endEncoding];
	m_renderEncoder = nil;

	// Finalize and execute our command buffer

	// Schedule a present once the framebuffer is complete using the current drawable.

	m_flushesInProgress++;

	if( m_drawableToAutoPresent != nil )
	{
		[m_metalCommandBuffer presentDrawable:m_drawableToAutoPresent];
		m_drawableToAutoPresent = nil;
	}

	// Add a completion handler.
	[m_metalCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
		// Shared buffer is populated.
		m_flushesInProgress--;
	}];

	// Finalize rendering here & push the command buffer to the GPU.
	[m_metalCommandBuffer commit];

	[m_metalCommandBuffer waitUntilScheduled];

	m_metalCommandBuffer = nil;

	//

	//		[m_metalCommandBuffer release];		We don't release these. Metal handles that itself.
	m_metalCommandBuffer = nil;

	[m_vertexBufferId release];
	m_vertexBufferId = nil;

	[m_colorBufferId release];
	m_colorBufferId = nil;

	[m_extrasBufferId release];
	m_extrasBufferId = nil;

	if( m_pActiveCanvas )
		m_pActiveCanvas->m_bBufferNeedsSync = true;

}

//____ setCanvas() ___________________________________________________________

void MetalBackend::setCanvas(Surface* _pSurface)
{
	MetalSurface * pSurface = dynamic_cast<MetalSurface*>(_pSurface);
	if( !pSurface )
	{
		//TODO: Error handling.

		return;
	}

	if( m_renderEncoder != nil )
		[m_renderEncoder endEncoding];

	m_renderEncoder = _setCanvas(pSurface, pSurface->pixelWidth(), pSurface->pixelHeight() );
}

void MetalBackend::setCanvas(CanvasRef ref)
{
	if( ref != CanvasRef::Default )
		return;								// ERROR: Only CanvasRef::Default supported yet.

	if( m_renderEncoder != nil )
		[m_renderEncoder endEncoding];

	m_renderEncoder = _setCanvas(nullptr, m_defaultCanvas.size.w/64, m_defaultCanvas.size.h/64 );
}

//____ setObjects() ________________________________________________________

void MetalBackend::setObjects(Object* const * pBeg, Object* const * pEnd)
{
	m_pObjectsBeg = pBeg;
	m_pObjectsEnd = pEnd;
	m_pObjectsPtr = pBeg;
}

//____ setRects() _________________________________________________________

void MetalBackend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
{
	m_pRectsBeg = pBeg;
	m_pRectsEnd = pEnd;
	m_pRectsPtr = pBeg;
}

//____ setColors() _________________________________________________________

void MetalBackend::setColors(const HiColor* pBeg, const HiColor* pEnd)
{
	m_pColorsBeg = pBeg;
	m_pColorsEnd = pEnd;
	m_pColorsPtr = pBeg;
}

//____ setTransforms() _____________________________________________________

void MetalBackend::setTransforms(const Transform* pBeg, const Transform* pEnd)
{
	m_pTransformsBeg = pBeg;
	m_pTransformsEnd = pEnd;

}

//____ processCommands() _____________________________________________________

void MetalBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd)
{
	const RectSPX* 	pRects = m_pRectsPtr;
	const HiColor* 	pColors = m_pColorsPtr;
	Object* const *	pObjects = m_pObjectsPtr;

	VertexMTL *	pVertexMTL	= m_pVertexBuffer + m_nVertices;
	ColorMTL*	pColorMTL	= m_pColorBuffer + m_nColors;
	float*		pExtrasMTL	= m_pExtrasBuffer + m_extrasBufferSize;


	auto p = pBeg;
	while (p < pEnd)
	{
		auto cmd = Command(*p++);
		switch (cmd)
		{
			case Command::None:
				break;

			case Command::StateChange:
			{
				bool	bUniformChanged = false;

				int32_t statesChanged = *p++;

				if (statesChanged & uint8_t(StateChange::BlitSource))
				{
					auto pSurf = static_cast<MetalSurface*>(* pObjects++);

					if( pSurf )
					{
						[m_renderEncoder setFragmentTexture:pSurf->getTexture() atIndex: (unsigned) TextureIndex::Texture];

						if(pSurf->pixelDescription()->type == PixelType::Index)
							[m_renderEncoder setFragmentSamplerState: m_samplers[0][0][pSurf->isTiling()] atIndex:0];
						else
							[m_renderEncoder setFragmentSamplerState: m_samplers[pSurf->isMipmapped()][pSurf->sampleMethod() == SampleMethod::Bilinear][pSurf->isTiling()] atIndex:0];

						m_pActiveBlitSource = pSurf;
						pSurf->m_bPendingReads = false;            // Clear this as we pass it by. All pending reads will have encoded before _executeBuffer() ends.

						m_uniform.textureSize = pSurf->pixelSize();
						bUniformChanged = true;

						if (pSurf->m_pPalette)
						{
							[m_renderEncoder setFragmentTexture:pSurf->getPaletteTexture() atIndex:(unsigned) TextureIndex::Palette];
						}
					}
					else
					{
						[m_renderEncoder setFragmentTexture:nil atIndex:(unsigned) TextureIndex::Texture];
						[m_renderEncoder setFragmentSamplerState: m_samplers[0][0][0] atIndex:0];

						m_pActiveBlitSource = nullptr;
					}

				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					HiColor color = *pColors++;

					m_uniform.flatTint[0] = color.r / 4096.f;
					m_uniform.flatTint[1] = color.g / 4096.f;
					m_uniform.flatTint[2] = color.b / 4096.f;
					m_uniform.flatTint[3] = color.a / 4096.f;

					m_bTintmap = false;

					bUniformChanged = true;
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{

					auto p32 = (const spx *) p;

					int32_t	x = *p32++;
					int32_t	y = *p32++;
					int32_t	w = *p32++;
					int32_t	h = *p32++;

					int32_t	nHorrColors = *p32++;
					int32_t	nVertColors = *p32++;

					p = (const uint16_t*) p32;

					//					pColors += nHorrColors + nVertColors;

					m_bTintmap = true;
					m_tintmapRect = RectI(x, y, w, h) / 64;

					if (nHorrColors > 0)
					{
						m_tintmapBeginX = int(pColorMTL - m_pColorBuffer);
						m_tintmapEndX = int(pColorMTL - m_pColorBuffer + nHorrColors);

						for (int i = 0; i < nHorrColors; i++)
						{
							pColorMTL->r = pColors->r / 4096.f;
							pColorMTL->g = pColors->g / 4096.f;
							pColorMTL->b = pColors->b / 4096.f;
							pColorMTL->a = pColors->a / 4096.f;
							pColorMTL++;
							pColors++;
						}
					}
					else
					{
						// Default to use white

						m_tintmapBeginX = 0;
						m_tintmapEndX = 0;
					}

					if (nVertColors > 0)
					{

						m_tintmapBeginY = int(pColorMTL - m_pColorBuffer);
						m_tintmapEndY = int(pColorMTL - m_pColorBuffer + nVertColors);

						for (int i = 0; i < nVertColors; i++)
						{
							pColorMTL->r = pColors->r / 4096.f;
							pColorMTL->g = pColors->g / 4096.f;
							pColorMTL->b = pColors->b / 4096.f;
							pColorMTL->a = pColors->a / 4096.f;
							pColorMTL++;
							pColors++;
						}
					}
					else
					{
						// Default to use white

						m_tintmapBeginY = 0;
						m_tintmapEndY = 0;
					}

					m_tintColorOfs = -1;
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					BlendMode mode = (BlendMode) * p++;

					m_activeBlendMode = mode;

					// If BlendMode is Morph or BlendFixedColor we must make sure Metal's blendColor is set accordingly

					if( mode == BlendMode::Morph )
					{
						if( m_morphFactorInUse != m_activeMorphFactor )
						{
							[m_renderEncoder setBlendColorRed:1.f green:1.f blue:1.f alpha:m_activeMorphFactor];

							m_morphFactorInUse = m_activeMorphFactor;
							m_fixedBlendColorInUse = HiColor::Undefined;
						}
					}
					else if( mode == BlendMode::BlendFixedColor )
					{
						if( m_fixedBlendColorInUse != m_activeFixedBlendColor )
						{
							[m_renderEncoder setBlendColorRed:(m_activeFixedBlendColor.r/4096.f)
														green:(m_activeFixedBlendColor.g/4096.f)
														 blue:(m_activeFixedBlendColor.b/4096.f)
														alpha:(m_activeFixedBlendColor.a/4096.f)];

							m_morphFactorInUse = -1;
							m_fixedBlendColorInUse = m_activeFixedBlendColor;
						}
					}
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					float morphFactor = (* p++) / 4096.f;

					m_activeMorphFactor = morphFactor;

					if( m_activeBlendMode == BlendMode::Morph && m_morphFactorInUse != morphFactor )
					{
						[m_renderEncoder setBlendColorRed:1.f green:1.f blue:1.f alpha:morphFactor];

						m_morphFactorInUse = morphFactor;
					}
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					// Just retrieve the value but do nothing. We use BlendMode::Blend
					// in place of BlendMode::BlendFixedColor.

					HiColor color = *pColors++;

					m_activeFixedBlendColor = color;

					if( m_activeBlendMode == BlendMode::BlendFixedColor && m_fixedBlendColorInUse != color )
					{
						[m_renderEncoder setBlendColorRed:(color.r/4096.f)
													green:(color.g/4096.f)
													 blue:(color.b/4096.f)
													alpha:(color.a/4096.f)];

						m_fixedBlendColorInUse = color;
					}
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					uint16_t	radius = * p++;

					auto size = m_activeCanvasSize;

					float radiusX = radius / float(size.w*64);
					float radiusY = radius / float(size.h*64);

					m_blurUniform.offset[0] = { -radiusX * 0.7f, -radiusY * 0.7f };
					m_blurUniform.offset[1] = { 0, -radiusY };
					m_blurUniform.offset[2] = { radiusX * 0.7f, -radiusY * 0.7f };

					m_blurUniform.offset[3] = { -radiusX, 0 };
					m_blurUniform.offset[4] = { 0, 0 };
					m_blurUniform.offset[5] = { radiusX, 0 };

					m_blurUniform.offset[6] = { -radiusX * 0.7f, radiusY * 0.7f };
					m_blurUniform.offset[7] = { 0, radiusY };
					m_blurUniform.offset[8] = { radiusX * 0.7f, radiusY * 0.7f };

					for( int i = 0 ; i < 9 ; i++ )
					{
						m_blurUniform.colorMtx[i][0] = p[i]/32768.f;
						m_blurUniform.colorMtx[i][1] = p[9+i]/32768.f;
						m_blurUniform.colorMtx[i][2] = p[18+i]/32768.f;
						m_blurUniform.colorMtx[i][3] = 0;
					}

					m_blurUniform.colorMtx[4][3] = 1.f;

					p += 27;

					[m_renderEncoder setFragmentBytes:&m_blurUniform length:sizeof(BlurUniform) atIndex: (unsigned) FragmentInputIndex::BlurUniform];
				}

				// Update uniform if changed

				if( bUniformChanged )
					[m_renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex:(unsigned) VertexInputIndex::Uniform];

				// Take care of alignment

				if( (uintptr_t(p) & 0x2) == 2 )
					p++;

				break;
			}
			case Command::Fill:
			{
				int32_t nRects = *p++;

				const HiColor& col = *pColors++;

				bool	bStraightFill = true;
				int		nRectsWritten = 0;
				int		extrasOfs = 0;

				// Setup Tintmap


				//

				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				// Add rects to vertex buffer

				for (int i = 0; i < nRects; i++)
				{

					int	dx1 = pRects->x;
					int	dy1 = pRects->y;
					int dx2 = dx1 + pRects->w;
					int dy2 = dy1 + pRects->h;

					if (((dx1 | dy1 | dx2 | dy2) & 63) == 0)
					{
						// Straight fill

						if (!bStraightFill)
						{
							if( nRectsWritten > 0 )
							{
								[m_renderEncoder setRenderPipelineState:m_fillPipelines[m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
								[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRectsWritten*6];

								vertexOfs = int(pVertexMTL - m_pVertexBuffer);
								nRectsWritten = 0;
								extrasOfs = 0;
							}
							bStraightFill = true;
						}
					}
					else
					{
						// Subpixel fill

						if(bStraightFill)
						{
							if (nRectsWritten > 0)
							{
								[m_renderEncoder setRenderPipelineState:m_fillAAPipelines[m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
								[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRectsWritten*6];

								vertexOfs = int(pVertexMTL - m_pVertexBuffer);
								nRectsWritten = 0;
							}
							bStraightFill = false;
						}

						extrasOfs = int(pExtrasMTL - m_pExtrasBuffer)/4;

						// Provide rectangle center and radius as extras

						RectSPX rect = * pRects;

						SizeF    radius(rect.w / (2.f * 64), rect.h / (2.f * 64));
						CoordF    center((rect.x / 64.f) + radius.w, (rect.y / 64.f) + radius.h);

						* pExtrasMTL++ = center.x;
						* pExtrasMTL++ = center.y;
						* pExtrasMTL++ = radius.w;
						* pExtrasMTL++ = radius.h;

						dx2 += 63;
						dy2 += 63;
					}

					dx1 >>= 6;
					dy1 >>= 6;
					dx2 >>= 6;
					dy2 >>= 6;


					float tintmapBeginX, tintmapBeginY, tintmapEndX, tintmapEndY;

					if (m_bTintmap)
					{
						if (m_tintmapBeginX == 0)
						{
							tintmapBeginX = 0.f;
							tintmapEndX = 0.f;
						}
						else
						{
							tintmapBeginX = m_tintmapBeginX + (dx1 - m_tintmapRect.x) + 0.f;
							tintmapEndX = tintmapBeginX + (dx2 - dx1) + 0.f;
						}

						if (m_tintmapBeginY == 0)
						{
							tintmapBeginY = 0.f;
							tintmapEndY = 0.f;
						}
						else
						{
							tintmapBeginY = m_tintmapBeginY + (dy1 - m_tintmapRect.y) + 0.f;
							tintmapEndY = tintmapBeginY + (dy2 - dy1) + 0.f;
						}

					}
					else
					{
						tintmapBeginX = 0.f;
						tintmapBeginY = 0.f;
						tintmapEndX = 0.f;
						tintmapEndY = 0.f;
					}

					int colOfs = int(pColorMTL - m_pColorBuffer);

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapEndX,tintmapBeginY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapEndX,tintmapEndY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapEndX,tintmapEndY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapBeginX,tintmapEndY };
					pVertexMTL++;

					pRects++;
					nRectsWritten++;
				}

				// Add colors to buffer

				pColorMTL->r = col.r / 4096.f;
				pColorMTL->g = col.g / 4096.f;
				pColorMTL->b = col.b / 4096.f;
				pColorMTL->a = col.a / 4096.f;
				pColorMTL++;

				// Draw

				if( bStraightFill )
					[m_renderEncoder setRenderPipelineState:m_fillPipelines[m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
				else
					[m_renderEncoder setRenderPipelineState:m_fillAAPipelines[m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];

				[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRectsWritten*6];
				break;
			}

			case Command::Line:
			{
				int32_t nClipRects = *p++;
				float thickness = *p++ / 64.f;
				int32_t nLines = *p++;

				HiColor col = *pColors++;

				// Calculate and store vertices

				int nLinesWritten = 0;
				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				for (int i = 0; i < nLines; i++)
				{
					auto p32 = (const spx *) p;
					CoordSPX begin = { *p32++, *p32++ };
					CoordSPX end = { *p32++, *p32++ };
					p = (const uint16_t*) p32;

					begin = roundToPixels(begin);
					end = roundToPixels(end);

					int 	length;
					float   width;

					float	slope;
					float	s, w;
					bool	bSteep;

					CoordI	c1, c2, c3, c4;

					if (std::abs(begin.x - end.x) > std::abs(begin.y - end.y))
					{
						// Prepare mainly horizontal line segment

						if (begin.x > end.x)
							std::swap(begin, end);

						length = end.x - begin.x;
						if (length == 0)
							continue;											// TODO: Should stil draw the caps!

						slope = ((float)(end.y - begin.y)) / length;
						width = _scaleThickness(thickness, slope);
						bSteep = false;

						if( m_pActiveCanvas )
							s = ((begin.y + 0.5f) - (begin.x + 0.5f) * slope);
						else
							s = m_defaultCanvas.size.h / 64 - ((begin.y + 0.5f) - (begin.x + 0.5f) * slope);

						w = width / 2 + 0.5f;

						float   y1 = begin.y - width / 2;
						float   y2 = end.y - width / 2;

						c1.x = begin.x;
						c1.y = int(y1) - 1;
						c2.x = end.x;
						c2.y = int(y2) - 1;
						c3.x = end.x;
						c3.y = int(y2 + width) + 2;
						c4.x = begin.x;
						c4.y = int(y1 + width) + 2;
					}
					else
					{
						// Prepare mainly vertical line segment

						if (begin.y > end.y)
							std::swap(begin, end);

						length = end.y - begin.y;
						if (length == 0)
							continue;											// TODO: Should stil draw the caps!

						slope = ((float)(end.x - begin.x)) / length;
						width = _scaleThickness(thickness, slope);
						bSteep = true;

						s = (begin.x + 0.5f) - (begin.y + 0.5f) * slope;
						w = width / 2 + 0.5f;

						float   x1 = begin.x - width / 2;
						float   x2 = end.x - width / 2;

						c1.x = int(x1) - 1;
						c1.y = begin.y;
						c2.x = int(x1 + width) + 2;
						c2.y = begin.y;
						c3.x = int(x2 + width) + 2;
						c3.y = end.y;
						c4.x = int(x2) - 1;
						c4.y = end.y;
					}

					int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;
					int colorsOfs = int(pColorMTL - m_pColorBuffer);

					pVertexMTL->coord = c1;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL++;

					pVertexMTL->coord = c2;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL++;

					pVertexMTL->coord = c3;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL++;

					pVertexMTL->coord = c1;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL++;

					pVertexMTL->coord = c3;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL++;

					pVertexMTL->coord = c4;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL++;

					*pExtrasMTL++ = s;
					*pExtrasMTL++ = w;
					*pExtrasMTL++ = slope;
					*pExtrasMTL++ = bSteep;

					nLinesWritten++;
				}

				// Store command, with clip rects

				if (nLinesWritten > 0)
				{
					pColorMTL->r = col.r / 4096.f;
					pColorMTL->g = col.g / 4096.f;
					pColorMTL->b = col.b / 4096.f;
					pColorMTL->a = col.a / 4096.f;
					pColorMTL++;

					[m_renderEncoder setRenderPipelineState:m_linePipelines[(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];

					for (int i = 0; i < nClipRects; i++)
					{
						RectI clip = (*pRects++)/64;
						MTLScissorRect metalClip = {(unsigned) clip.x, (unsigned) clip.y, (unsigned) clip.w, (unsigned) clip.h};
						[m_renderEncoder setScissorRect:metalClip];
						[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nLinesWritten*6];
					}

					MTLScissorRect orgClip = {0, 0, (unsigned) m_activeCanvasSize.w, (unsigned) m_activeCanvasSize.h};
					[m_renderEncoder setScissorRect:orgClip];

				}
				else
					pRects += nClipRects;

				break;
			}

			case Command::DrawEdgemap:
			{
				auto pEdgemap = static_cast<MetalEdgemap*>(*pObjects++);

				int32_t nRects = *p++;
				int32_t	flip = *p++;

				p++;					// padding

				auto p32 = (const spx *) p;
				int32_t	destX = *p32++;
				int32_t	destY = *p32++;
				p = (const uint16_t*) p32;

				//

				auto& mtx = s_blitFlipTransforms[flip];

				RectSPX _destIn = {
					destX,
					destY,
					pEdgemap->m_size.w * 64 * int(abs(mtx.xx)) + pEdgemap->m_size.h * 64 * int(abs(mtx.yx)),
					pEdgemap->m_size.w * 64 * int(abs(mtx.xy)) + pEdgemap->m_size.h * 64 * int(abs(mtx.yy)),
				};


				int nSegments = pEdgemap->m_nbRenderSegments;

				// Do transformations

				RectI dest = roundToPixels(_destIn);

				int uIncX = mtx.xx;
				int vIncX = mtx.xy;
				int uIncY = mtx.yx;
				int vIncY = mtx.yy;

				// Possibly clip the destination rectangle if we have space for more columns than we have

				int maxCol = pEdgemap->m_size.w;
				if (uIncX != 0)								// Columns are aligned horizontally
				{
					if (dest.w > maxCol)
					{
						if (uIncX < 0)
							dest.x += dest.w - maxCol;
						dest.w = maxCol;
					}
				}
				else										// Columns are aligned vertically
				{
					if (dest.h > maxCol)
					{
						if (uIncY < 0)
							dest.y += dest.h - maxCol;
						dest.h = maxCol;
					}
				}

				// Calc topLeft UV values

				int uTopLeft = 0;
				int vTopLeft = 0;

				if (uIncX + uIncY < 0)
					uTopLeft = maxCol;

				if (vIncX < 0)
					vTopLeft = dest.w;
				else if (vIncY < 0)
					vTopLeft = dest.h;

				//

				int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;

				float colorstripPitchX;
				float colorstripPitchY;

				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				// Setup vertices

				for (int i = 0; i < nRects; i++)
				{
					RectI patch = (*pRects++) / 64;

					int		dx1 = patch.x;
					int		dx2 = patch.x + patch.w;
					int		dy1 = patch.y;
					int		dy2 = patch.y + patch.h;

					//

					float tintmapBeginX, tintmapBeginY, tintmapEndX, tintmapEndY;


					if (m_bTintmap)
					{
						if (m_tintmapBeginX == 0)
						{
							tintmapBeginX = 0.f;
							tintmapEndX = 0.f;
						}
						else
						{
							tintmapBeginX = m_tintmapBeginX + (dx1 - m_tintmapRect.x) + 0.f;
							tintmapEndX = tintmapBeginX + (dx2 - dx1) + 0.f;
						}

						if (m_tintmapBeginY == 0)
						{
							tintmapBeginY = 0.f;
							tintmapEndY = 0.f;
						}
						else
						{
							tintmapBeginY = m_tintmapBeginY + (dy1 - m_tintmapRect.y) + 0.f;
							tintmapEndY = tintmapBeginY + (dy2 - dy1) + 0.f;
						}
					}
					else
					{
						tintmapBeginX = 0.f;
						tintmapBeginY = 0.f;
						tintmapEndX = 0.f;
						tintmapEndY = 0.f;
					}

					//

					int ofsX = patch.x - dest.x;
					int ofsY = patch.y - dest.y;

					float colorstripBeginX;
					float colorstripEndX;

					float colorstripBeginY;
					float colorstripEndY;

					if (pEdgemap->m_pFlatColors)
					{
						colorstripBeginX = pEdgemap->_flatColorsOfs();
						colorstripEndX = colorstripBeginX;

						colorstripBeginY = pEdgemap->_whiteColorOfs();
						colorstripEndY = colorstripBeginY;

						colorstripPitchX = 1;
						colorstripPitchY = 0;

					}
					else
					{
						if (pEdgemap->m_pColorstripsX)
						{
							colorstripBeginX = pEdgemap->_colorstripXOfs() + ofsX;
							colorstripEndX = colorstripBeginX + patch.w;

							colorstripPitchX = pEdgemap->m_size.w;
						}
						else
						{
							colorstripBeginX = pEdgemap->_whiteColorOfs();
							colorstripEndX = colorstripBeginX;

							colorstripPitchX = 0;
						}

						if (pEdgemap->m_pColorstripsY)
						{
							colorstripBeginY = pEdgemap->_colorstripYOfs() + ofsY;
							colorstripEndY = colorstripBeginY + patch.h;

							colorstripPitchY = pEdgemap->m_size.h;
						}
						else
						{
							colorstripBeginY = pEdgemap->_whiteColorOfs();
							colorstripEndY = colorstripBeginY;

							colorstripPitchY = 0;
						}
					}


					// Calc UV-coordinates. U is edge offset, V is pixel offset from begin in column.

					float	u1 = (float)(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
					float	v1 = (float)(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

					float	u2 = (float)(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
					float	v2 = (float)(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

					float	u3 = (float)(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
					float	v3 = (float)(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

					float	u4 = (float)(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
					float	v4 = (float)(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

					simd_float2	uv1 = { u1, v1 - 0.5f };
					simd_float2	uv2 = { u2, v2 - 0.5f };
					simd_float2	uv3 = { u3, v3 - 0.5f };
					simd_float2	uv4 = { u4, v4 - 0.5f };

					//

					CoordF	colorstripUV[4];

					CoordF	colorstripUVin[4] = { { colorstripBeginX, colorstripBeginY }, { colorstripEndX, colorstripBeginY }, { colorstripEndX, colorstripEndY }, { colorstripBeginX, colorstripEndY } };

					colorstripUV[0] = colorstripUVin[s_flipCornerOrder[flip][0]];
					colorstripUV[1] = colorstripUVin[s_flipCornerOrder[flip][1]];
					colorstripUV[2] = colorstripUVin[s_flipCornerOrder[flip][2]];
					colorstripUV[3] = colorstripUVin[s_flipCornerOrder[flip][3]];

					//

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv1;
					pVertexMTL->tintmapOfs = { tintmapBeginX, tintmapBeginY };
					pVertexMTL->colorstripOfs = colorstripUV[0];
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv2;
					pVertexMTL->tintmapOfs = { tintmapEndX, tintmapBeginY };
					pVertexMTL->colorstripOfs = colorstripUV[1];
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv3;
					pVertexMTL->tintmapOfs = { tintmapEndX, tintmapEndY };
					pVertexMTL->colorstripOfs = colorstripUV[2];
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv1;
					pVertexMTL->tintmapOfs = { tintmapBeginX, tintmapBeginY };
					pVertexMTL->colorstripOfs = colorstripUV[0];
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv3;
					pVertexMTL->tintmapOfs = { tintmapEndX, tintmapEndY };
					pVertexMTL->colorstripOfs = colorstripUV[2];
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv4;
					pVertexMTL->tintmapOfs = { tintmapBeginX, tintmapEndY };
					pVertexMTL->colorstripOfs = colorstripUV[3];
					pVertexMTL++;
				}

				// Setup extras data

				*pExtrasMTL++ = colorstripPitchX;
				*pExtrasMTL++ = colorstripPitchY;
				*pExtrasMTL++ = 0;			// Dummy
				*pExtrasMTL++ = 0;			// Dummy



				//

				[m_renderEncoder setFragmentBuffer:pEdgemap->m_bufferId offset:0 atIndex:(unsigned) FragmentInputIndex::Edgemap];

				[m_renderEncoder setRenderPipelineState:m_segmentsPipelines[nSegments-1][m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
				[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRects*6];

				break;
			}

			case Command::Blit:
			case Command::ClipBlit:
			case Command::Tile:
			case Command::Blur:
			{
				int32_t nRects = *p++;

				//

				int tintColorOfs = m_tintColorOfs >= 0 ? m_tintColorOfs : 0;
				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				float tintmapBeginX = 0.f;
				float tintmapBeginY = 0.f;
				float tintmapEndX = 0.f;
				float tintmapEndY = 0.f;

				for (int i = 0; i < nRects; i++)
				{
					int	dx1 = (pRects->x) >> 6;
					int	dy1 = (pRects->y) >> 6;
					int dx2 = dx1 + ((pRects->w) >> 6);
					int dy2 = dy1 + ((pRects->h) >> 6);
					pRects++;

					if (m_bTintmap)
					{
						if (m_tintmapBeginX == 0)
						{
							tintmapBeginX = 0.f;
							tintmapEndX = 0.f;
						}
						else
						{
							tintmapBeginX = m_tintmapBeginX + (dx1 - m_tintmapRect.x) + 0.f;
							tintmapEndX = tintmapBeginX + (dx2 - dx1) + 0.f;
						}

						if (m_tintmapBeginY == 0)
						{
							tintmapBeginY = 0.f;
							tintmapEndY = 0.f;
						}
						else
						{
							tintmapBeginY = m_tintmapBeginY + (dy1 - m_tintmapRect.y) + 0.f;
							tintmapEndY = tintmapBeginY + (dy2 - dy1) + 0.f;
						}
					}

					int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapEndX,tintmapBeginY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapEndX,tintmapEndY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapEndX,tintmapEndY };
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintmapOfs = { tintmapBeginX,tintmapEndY };
					pVertexMTL++;

					//

					auto p32 = (const spx *) p;
					int srcX = *p32++;
					int srcY = *p32++;
					spx dstX = *p32++;
					spx dstY = *p32++;
					p = (const uint16_t*) p32;


					if (m_pActiveBlitSource->sampleMethod() == SampleMethod::Bilinear)
					{
						* pExtrasMTL++ = srcX / 1024.f + 0.5f;
						* pExtrasMTL++ = srcY / 1024.f + 0.5f;
						* pExtrasMTL++ = float(dstX >> 6) + 0.5f;
						* pExtrasMTL++ = float(dstY >> 6) + 0.5f;
					}
					else
					{
						*pExtrasMTL++ = srcX / 1024.f;
						*pExtrasMTL++ = srcY / 1024.f;
						*pExtrasMTL++ = float(dstX >> 6) + 0.5f;
						*pExtrasMTL++ = float(dstY >> 6) + 0.5f;
					}

					int32_t transform = *p++;
					p++;						// padding

					auto& mtx = transform < GfxFlip_size ? s_blitFlipTransforms[transform] : m_pTransformsBeg[transform - GfxFlip_size];

					*pExtrasMTL++ = mtx.xx;
					*pExtrasMTL++ = mtx.xy;
					*pExtrasMTL++ = mtx.yx;
					*pExtrasMTL++ = mtx.yy;
				}



				// Draw

				{
					MetalSurface* pSurf = m_pActiveBlitSource;

					BlitFragShader shader = BlitFragShader::Normal;

					if(pSurf->m_pPixelDescription->type == PixelType::Index)
					{
						if( pSurf->sampleMethod() == SampleMethod::Bilinear )
							shader = BlitFragShader::PaletteInterpolated;
						else
							shader = BlitFragShader::PaletteNearest;
					}
					else if(pSurf->m_pixelFormat == PixelFormat::Alpha_8)
						shader = BlitFragShader::A8Source;
					else if(pSurf->m_pPixelDescription->A_mask == 0)
						shader = BlitFragShader::RGBXSource;

					if( cmd == Command::Blur )
						[m_renderEncoder setRenderPipelineState:m_blurPipelines[m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
					else
						[m_renderEncoder setRenderPipelineState:m_blitPipelines[(int)shader][m_bTintmap][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];

					[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRects*6];
					vertexOfs += nRects*6;

					if( m_pActiveCanvas )
						m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
				}

				break;
			}


			default:
				break;
		}
	}

	// Save progress.

	m_pRectsPtr = pRects;
	m_pColorsPtr = pColors;
	m_pObjectsPtr = pObjects;

	m_nVertices			= int(pVertexMTL - m_pVertexBuffer);
	m_nColors			= int(pColorMTL	- m_pColorBuffer);
	m_extrasBufferSize	= int(pExtrasMTL - m_pExtrasBuffer);

	//		_executeBuffer();

}


//____ _setCanvas() ___________________________________________________________

id<MTLRenderCommandEncoder> MetalBackend::_setCanvas( MetalSurface * pCanvas, int width, int height )
{
	id<MTLRenderCommandEncoder> renderEncoder;

	PixelFormat pixelFormat;

	if (pCanvas)
	{
		MTLRenderPassDescriptor* pDescriptor = [MTLRenderPassDescriptor new];

		pDescriptor.colorAttachments[0].texture = pCanvas->getTexture();
		pDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		//		pDescriptor.colorAttachments[0].loadAction = m_bFullCanvasSession ? MTLLoadActionDontCare : MTLLoadActionLoade;
		pDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;

		renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:pDescriptor];
		renderEncoder.label = @"MetalBackend Render to Surface Pass";
		[pDescriptor release];

		pixelFormat = pCanvas->pixelFormat();
	}
	else
	{

		renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:m_defaultCanvasRenderPassDesc];
		renderEncoder.label = @"MetalBackend Render Pass";

		pixelFormat = m_defaultCanvasPixelFormat;
	}

	[renderEncoder setViewport:(MTLViewport){0.0, 0.0, (double) width, (double) height, 0.0, 1.0 }];
	[renderEncoder setBlendColorRed:1.f green:1.f blue:1.f alpha:m_activeMorphFactor];


	// Updating canvas info for our shaders

	m_uniform.canvasDimX = (float) width;
	m_uniform.canvasDimY = (float) height;
	m_uniform.canvasYOfs = height;
	m_uniform.canvasYMul = -1;

	[renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex: (unsigned) VertexInputIndex::Uniform];

	// Set vertex and extras buffer

	[renderEncoder setVertexBuffer:m_vertexBufferId offset:0 atIndex:(unsigned) VertexInputIndex::VertexBuffer];
	[renderEncoder setVertexBuffer:m_colorBufferId offset:0 atIndex:(unsigned) VertexInputIndex::ColorBuffer];
	[renderEncoder setVertexBuffer:m_extrasBufferId offset:0 atIndex:(unsigned) VertexInputIndex::ExtrasBuffer];

	//

	[renderEncoder setFragmentBuffer:m_colorBufferId offset:0 atIndex:(unsigned) FragmentInputIndex::ColorBuffer];
	[renderEncoder setFragmentBuffer:m_extrasBufferId offset:0 atIndex:(unsigned) FragmentInputIndex::ExtrasBuffer];

	//

	m_pActiveCanvas = pCanvas;
	m_activeCanvasSize = {width,height};

	switch(pixelFormat)
	{
		case PixelFormat::BGRX_8_linear:
			m_activeCanvasFormat = DestFormat::BGRX8_linear;
			break;

		case PixelFormat::BGRA_8_linear:
			m_activeCanvasFormat = DestFormat::BGRA8_linear;
			break;

		case PixelFormat::BGRA_8_sRGB:
			m_activeCanvasFormat = DestFormat::BGRA8_sRGB;
			break;

		case PixelFormat::BGRX_8_sRGB:
			m_activeCanvasFormat = DestFormat::BGRX8_sRGB;
			break;

		case PixelFormat::Alpha_8:
			m_activeCanvasFormat = DestFormat::Alpha_8;
			break;

		default:
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::Internal, "Canvas format is neither BGRA_8_linear, BGRA_8_sRGB or A_8", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
			break;
	}


	return renderEncoder;
}


//____ _initTables() ___________________________________________________________

void MetalBackend::_initTables()
{
	// Init lineThicknessTable

	for (int i = 0; i < 17; i++)
	{
		double b = i / 16.0;
		m_lineThicknessTable[i] = (float)Util::squareRoot(1.0 + b * b);
	}

	// Init sRGBtoLinearTable

	float max = powf(255, 2.2f);

	for (int i = 0; i < 256; i++)
	{
		m_sRGBtoLinearTable[i] = powf(float(i), 2.2f)/max;
		m_linearToLinearTable[i] = i / 255.f;
	}
}

//____ _scaleThickness() ___________________________________________________

float MetalBackend::_scaleThickness(float thickness, float slope)
{
	slope = std::abs(slope);

	float scale = m_lineThicknessTable[(int)(slope * 16)];

	if (slope < 1.f)
	{
		float scale2 = m_lineThicknessTable[(int)(slope * 16) + 1];
		scale += (scale2 - scale)*((slope * 16) - ((int)(slope * 16)));
	}

	return thickness * scale;
}

//____ _compileRenderPipeline() _______________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileRenderPipeline( NSString* label, NSString* vertexShader,
																NSString* fragmentShader, BlendMode blendMode, PixelFormat destFormat )
{
	NSError *error = nil;
	MTLRenderPipelineDescriptor *descriptor = [[MTLRenderPipelineDescriptor alloc] init];

	descriptor.label = label;
	descriptor.vertexFunction = [m_library newFunctionWithName:vertexShader];
	descriptor.fragmentFunction = [m_library newFunctionWithName:fragmentShader];

	// Set pixelFormat

	switch(destFormat)
	{
		case PixelFormat::BGRA_8_linear:
		case PixelFormat::BGRX_8_linear:
			descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
			break;

		case PixelFormat::BGRA_8_sRGB:
		case PixelFormat::BGRX_8_sRGB:
			descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
			break;

		case PixelFormat::Alpha_8:
			descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatR8Unorm;
			break;

		default:
			assert(false);
	}

	bool bNoAlpha = ( destFormat == PixelFormat::BGRX_8_linear || destFormat == PixelFormat::BGRX_8_sRGB );


	bool bAlphaOnly = (destFormat == PixelFormat::Alpha_8);

	switch( blendMode )
	{
		case BlendMode::Replace:

			if( bNoAlpha )
			{
				descriptor.colorAttachments[0].blendingEnabled = YES;
				descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorZero;
			}
			else
				descriptor.colorAttachments[0].blendingEnabled = NO;
			break;

		case BlendMode::Undefined:
		case BlendMode::Blend:
			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

			if(bAlphaOnly)
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceColor;
			}
			else
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
			}

			break;

		case BlendMode::Morph:
			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorBlendAlpha;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusBlendAlpha;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorBlendAlpha;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusBlendAlpha;
			break;

		case BlendMode::Add:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

			if( bAlphaOnly )
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			}
			else
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			}
			break;

		case BlendMode::Subtract:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationReverseSubtract;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationReverseSubtract;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

			if( bAlphaOnly )
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			}
			else
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			}
			break;

		case BlendMode::Multiply:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

			break;

		case BlendMode::Invert:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOneMinusDestinationColor;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceColor;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

			break;

		case BlendMode::Min:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationMin;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
			break;

		case BlendMode::Max:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationMax;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
			break;

		case BlendMode::Ignore:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
			break;

		case BlendMode::BlendFixedColor:

			descriptor.colorAttachments[0].blendingEnabled = YES;
			descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
			descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
			descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusBlendAlpha;

			if(bAlphaOnly)
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusBlendColor;
			}
			else
			{
				descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorBlendAlpha;
				descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusBlendAlpha;
			}

			break;

		default:
			assert(false);
			break;
	}

	if(bNoAlpha)
	{
		descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
		descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
		descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
	}

	id<MTLRenderPipelineState> pipelineState = [s_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
	[error release];

	[descriptor.vertexFunction release];
	[descriptor.fragmentFunction release];
	[descriptor release];

	return pipelineState;
}
} //namespace wg
