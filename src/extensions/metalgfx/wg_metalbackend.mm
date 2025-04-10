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

				m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Blit BGRA_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Blit BGRX_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"Blit BGRA_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"Blit BGRX_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::Normal][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"Blit A_8 Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_linear Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_linear Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_sRGB Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_sRGB Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitNearest A_8 Pipeline", @"blitVertexShader", @"paletteBlitNearestFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRA_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitNearest BGRX_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"paletteBlitNearestFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteNearest][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitNearest A_8 Gradient Pipeline", @"blitGradientVertexShader", @"paletteBlitNearestFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_linear Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_linear Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_sRGB Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_sRGB Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitInterpolated A_8 Pipeline", @"paletteBlitInterpolateVertexShader", @"paletteBlitInterpolateFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_linear Gradient Pipeline", @"paletteBlitInterpolateGradientVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_linear Gradient Pipeline", @"paletteBlitInterpolateGradientVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRA_8_sRGB Gradient Pipeline", @"paletteBlitInterpolateGradientVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"PaletteBlitInterpolated BGRX_8_sRGB Gradient Pipeline", @"paletteBlitInterpolateGradientVertexShader", @"paletteBlitInterpolateFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::PaletteInterpolated][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"PaletteBlitInterpolated A_8 Gradient Pipeline", @"paletteBlitInterpolateGradientVertexShader", @"paletteBlitInterpolateFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRA_8_linear Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRX_8_linear Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRA_8_sRGB Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRX_8_sRGB Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::A8Source][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"A8SourceBlit A_8 Pipeline", @"blitVertexShader", @"alphaBlitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRA_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"A8SourceBlit BGRX_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRA_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"A8SourceBlit BGRX_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"alphaBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::A8Source][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"A8SourceBlit A_8 Gradient Pipeline", @"blitGradientVertexShader", @"alphaBlitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_linear Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_linear Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_sRGB Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_sRGB Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][0][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"RGBXSourceBlit A_8 Pipeline", @"blitVertexShader", @"rgbxBlitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

				m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRA_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"RGBXSourceBlit BGRX_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"rgbxBlitFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
				m_blitPipelines[(int)BlitFragShader::RGBXSource][1][blendMode][(int)DestFormat::Alpha_8]          = _compileRenderPipeline( @"RGBXSourceBlit A_8 Gradient Pipeline", @"blitGradientVertexShader", @"rgbxBlitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );
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

			m_blurPipelines[1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Blit BGRA_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
			m_blurPipelines[1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Blit BGRX_8_linear Gradient Pipeline", @"blitGradientVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_linear );
			m_blurPipelines[1][blendMode][(int)DestFormat::BGRA8_sRGB]   = _compileRenderPipeline( @"Blit BGRA_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
			m_blurPipelines[1][blendMode][(int)DestFormat::BGRX8_sRGB]   = _compileRenderPipeline( @"Blit BGRX_8_sRGB Gradient Pipeline", @"blitGradientVertexShader", @"blurFragmentShader", (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );
//			m_blitPipelines[1][blendMode][(int)DestFormat::Alpha_8]      = _compileRenderPipeline( @"Blit A_8 Gradient Pipeline", @"blitGradientVertexShader", @"blitFragmentShader_A8", (BlendMode) blendMode, PixelFormat::Alpha_8 );

		}
	}



		// Create and init Segments pipelines


		NSString * segFragShaders[16] = { nil, @"segmentsFragmentShader1", @"segmentsFragmentShader2", @"segmentsFragmentShader3",
											@"segmentsFragmentShader4", @"segmentsFragmentShader5", @"segmentsFragmentShader6", @"segmentsFragmentShader7",
											@"segmentsFragmentShader8", @"segmentsFragmentShader9", @"segmentsFragmentShader10", @"segmentsFragmentShader11",
											@"segmentsFragmentShader12", @"segmentsFragmentShader13", @"segmentsFragmentShader14", @"segmentsFragmentShader15" };

		NSString * segFragShaders_A8[16] = { nil, @"segmentsFragmentShader1_A8", @"segmentsFragmentShader2_A8", @"segmentsFragmentShader3_A8",
											@"segmentsFragmentShader4_A8", @"segmentsFragmentShader5_A8", @"segmentsFragmentShader6_A8", @"segmentsFragmentShader7_A8",
											@"segmentsFragmentShader8_A8", @"segmentsFragmentShader9_A8", @"segmentsFragmentShader10_A8", @"segmentsFragmentShader11_A8",
											@"segmentsFragmentShader12_A8", @"segmentsFragmentShader13_A8", @"segmentsFragmentShader14_A8", @"segmentsFragmentShader15_A8" };

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

				m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRA8_linear] = _compileRenderPipeline( @"Segments BGRA_8_linear gradient pipeline", @"segmentsGradientVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRA_8_linear );
				m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRX8_linear] = _compileRenderPipeline( @"Segments BGRX_8_linear gradient pipeline", @"segmentsGradientVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRX_8_linear );

				m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRA8_sRGB] = _compileRenderPipeline( @"Segments BGRA_8_sRGB gradient pipeline", @"segmentsGradientVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRA_8_sRGB );
				m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::BGRX8_sRGB] = _compileRenderPipeline( @"Segments BGRX_8_sRGB gradient pipeline", @"segmentsGradientVertexShader", segFragShaders[shader], (BlendMode) blendMode, PixelFormat::BGRX_8_sRGB );

				m_segmentsPipelines[shader][1][blendMode][(int)DestFormat::Alpha_8] = _compileRenderPipeline( @"Segments A_8 gradient pipeline", @"segmentsGradientVertexShader", segFragShaders_A8[shader], (BlendMode) blendMode, PixelFormat::Alpha_8 );
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

//		[m_metalCommandBuffer release];
		m_metalCommandBuffer = nil;

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

		m_bFullCanvasSession = (nUpdateRects == 1 && pUpdateRects[0] == RectSPX(0,0,pCanvasSurface->pixelSize()*64) );

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
					p += 28;
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

				pRects += nRects;

				break;
			}

			case Command::Blit:
			case Command::ClipBlit:
			case Command::Tile:
			case Command::Blur:
			{
				int32_t nRects = *p++;
				int32_t transform = *p++;

				p++;						// padding

				auto p32 = (const spx *) p;
				int srcX = *p32++;
				int srcY = *p32++;
				spx dstX = *p32++;
				spx dstY = *p32++;
				p = (const uint16_t*) p32;

				//

				int tintColorOfs = m_tintColorOfs >= 0 ? m_tintColorOfs : 0;
				int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;
				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				for (int i = 0; i < nRects; i++)
				{
					int	dx1 = (pRects->x) >> 6;
					int	dy1 = (pRects->y) >> 6;
					int dx2 = dx1 + ((pRects->w) >> 6);
					int dy2 = dy1 + ((pRects->h) >> 6);
					pRects++;

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

				}

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

				auto& mtx = transform < GfxFlip_size ? s_blitFlipTransforms[transform] : m_pTransformsBeg[transform - GfxFlip_size];


				*pExtrasMTL++ = mtx.xx;
				*pExtrasMTL++ = mtx.xy;
				*pExtrasMTL++ = mtx.yx;
				*pExtrasMTL++ = mtx.yy;

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
		pDescriptor.colorAttachments[0].loadAction = m_bFullCanvasSession ? MTLLoadActionDontCare : MTLLoadActionDontCare;

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


	// Set buffers/textures for segments fragment shader

//	[renderEncoder setFragmentTexture:m_segPalTextureId atIndex: (unsigned) TextureIndex::SegPal];
//	[renderEncoder setFragmentBuffer:m_segEdgeBufferId offset:0 atIndex:(unsigned) FragmentInputIndex::ExtrasBuffer];

//	[renderEncoder setFragmentBytes:&m_blurUniform length:sizeof(BlurUniform) atIndex: (unsigned) FragmentInputIndex::BlurUniform];


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



/*
	//____ setTintColor() _______________________________________________________

	void MetalGfxDevice::setTintColor(HiColor color)
	{
		if (color == m_tintColor)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		GfxDeviceGen1::setTintColor(color);

		_endCommand();
		_beginStateCommandWithAlignedData(Command::SetTintColor, 2);
		*(int64_t*)(&m_pCommandBuffer[m_commandOfs]) = color.argb;
		m_commandOfs += 2;
	}

	//____ setTintGradient() ______________________________________________________

	void MetalGfxDevice::setTintGradient(const RectSPX& rect, const Gradient& gradient)
	{
		if (m_bTintGradient && gradient == m_tintGradient && rect == m_tintGradientRect)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		GfxDeviceGen1::setTintGradient(rect, gradient);

		_endCommand();
		_beginStateCommandWithAlignedData(Command::SetTintGradient, 4 + sizeof(Gradient)/4);
		m_pCommandBuffer[m_commandOfs++] = rect.x / 64;
		m_pCommandBuffer[m_commandOfs++] = rect.y / 64;
		m_pCommandBuffer[m_commandOfs++] = rect.w / 64;
		m_pCommandBuffer[m_commandOfs++] = rect.h / 64;
		*(Gradient*)(&m_pCommandBuffer[m_commandOfs]) = gradient;
		m_commandOfs += sizeof(Gradient)/4;
	}

	//____ clearTintGradient() _____________________________________________________

	void MetalGfxDevice::clearTintGradient()
	{
		if (m_bTintGradient == false)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		GfxDeviceGen1::clearTintGradient();

		_endCommand();
		_beginStateCommand(Command::ClearTintGradient, 0);
	}

	//____ setBlendMode() ____________________________________________________________

	bool MetalGfxDevice::setBlendMode(BlendMode blendMode)
	{
		if (blendMode == m_blendMode)
			return true;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return false;
		}

		GfxDeviceGen1::setBlendMode(blendMode);

		_endCommand();
		_beginStateCommand(Command::SetBlendMode, 1);
		m_pCommandBuffer[m_commandOfs++] = (int) blendMode;

		return true;
	}

	//____ setBlitSource() ___________________________________________________________

	bool MetalGfxDevice::setBlitSource(Surface * pSource)
	{
		if (pSource == m_pBlitSource)
			return true;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return false;
		}

		if( !pSource )
			return false;

		if (pSource->typeInfo() != MetalSurface::TYPEINFO)
		{
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Provided surface is NOT a MetalSurface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_pBlitSource = pSource;

		//TODO: Check so that we don't overrun m_pSurfaceBuffer;

		_endCommand();
		_beginStateCommandWithAlignedData(Command::SetBlitSource, sizeof(void*)/sizeof(int));
		* (void**)(m_pCommandBuffer+m_commandOfs) = pSource;
		m_commandOfs += sizeof(void*)/sizeof(int);
		if( pSource )
			pSource->retain();

		return true;
	}

	//____ setMorphFactor() __________________________________________________________

	void MetalGfxDevice::setMorphFactor(float factor)
	{
		if (factor == m_morphFactor)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		limit(factor, 0.f, 1.f);

		m_morphFactor = factor;

		_endCommand();
		_beginStateCommand(Command::SetMorphFactor, 1);
		m_pCommandBuffer[m_commandOfs++] = (int)(factor*1024);
	}

	//____ setFixedBlendColor() __________________________________________________

	void MetalGfxDevice::setFixedBlendColor( HiColor color )
	{
		if( color == m_fixedBlendColor )
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		m_fixedBlendColor = color;

		_endCommand();
		_beginStateCommandWithAlignedData(Command::SetFixedBlendColor, 2);
		*(int64_t*)(&m_pCommandBuffer[m_commandOfs]) = color.argb;
		m_commandOfs += 2;
	}

	//____ setBlurbrush() ________________________________________________________

	void MetalGfxDevice::setBlurbrush(Blurbrush* pBrush)
	{
		GfxDeviceGen1::setBlurbrush(pBrush);

		_endCommand();
		_beginStateCommand(Command::SetBlurMatrices, 28);

		m_pCommandBuffer[m_commandOfs++] = pBrush->size();

		// Copy floats as they are

		memcpy( m_pCommandBuffer + m_commandOfs, pBrush->red(), sizeof(float)*9 );
		m_commandOfs += 9;
		memcpy( m_pCommandBuffer + m_commandOfs, pBrush->green(), sizeof(float)*9 );
		m_commandOfs += 9;
		memcpy( m_pCommandBuffer + m_commandOfs, pBrush->blue(), sizeof(float)*9 );
		m_commandOfs += 9;
	}

	//____ isCanvasReady() ___________________________________________________________

	bool MetalGfxDevice::isCanvasReady() const
	{
		return m_flushesInProgress == 0;
	}

	//____ autopresent() ________________________________________________

	void MetalGfxDevice::autopresent( id<MTLDrawable> drawable )
	{
		m_drawableToAutoPresent = drawable;
	}

	//____ beginRender() _____________________________________________________________

	bool MetalGfxDevice::beginRender()
	{
		if( m_bRendering == true )
			return false;

		m_bRendering = true;

		m_cmd = Command::None;

		//

		_resetBuffers();


		// Create a new command buffer for each render pass to the current drawable.
		m_metalCommandBuffer = [s_metalCommandQueue commandBuffer];
		m_metalCommandBuffer.label = @"MetalGfxDevice";

		// Set intial canvas

		m_pActiveCanvas     = nullptr;
		m_activeCanvasSize  = {0,0};

		// Set intial states in super

		GfxDeviceGen1::setBlitSource(nullptr);
		GfxDeviceGen1::setBlendMode(BlendMode::Blend);
		GfxDeviceGen1::setMorphFactor(0.5f);
		GfxDeviceGen1::setFixedBlendColor(HiColor::Black);
		GfxDeviceGen1::setTintColor(HiColor::White);
		GfxDeviceGen1::clearTintGradient();

		// Set initial active states

		m_pActiveBlitSource = nullptr;
		m_activeBlendMode   = BlendMode::Blend;
		m_activeMorphFactor = 0.5f;
		m_activeFixedBlendColor = HiColor::White;

		m_morphFactorInUse = -1.f;
		m_fixedBlendColorInUse = HiColor::Undefined;

		m_uniform.flatTint[0] = 1.f;
		m_uniform.flatTint[1] = 1.f;
		m_uniform.flatTint[2] = 1.f;
		m_uniform.flatTint[3] = 1.f;

		m_bGradientActive = false;

		// Wait for previous render pass to complete by doing a flushAndWait on what we have in buffer so far.

		if( m_flushesInProgress > 0 )
			flushAndWait();

		return true;
	}

	//____ endRender() ___________________________________________________________

	bool MetalGfxDevice::endRender()
	{
		if( !m_bRendering )
			return false;

		// Finalize any ongoing operation

		_endCommand();
		_executeBuffer();

		m_flushesInProgress++;

		//

		// Schedule a present once the framebuffer is complete using the current drawable.

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

		m_bRendering = false;

		return true;
	}

	//____ isIdle() ______________________________________________________________

	bool MetalGfxDevice::isIdle()
	{
		return !m_bRendering && m_flushesInProgress == 0;
	}

	//____ flush() _______________________________________________________________

	void MetalGfxDevice::flush()
	{
		if( !m_bRendering )
			return;

		// Finalize any commands and execute queue.

		_endCommand();
		_executeBuffer();

		m_flushesInProgress++;

		// Add a completion handler.
		[m_metalCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
			// Shared buffer is populated.
			m_flushesInProgress--;
		}];

		// Push the command buffer to the GPU.

		[m_metalCommandBuffer commit];

		// Create a new command buffer.
		m_metalCommandBuffer = [s_metalCommandQueue commandBuffer];
		m_metalCommandBuffer.label = @"MetalBackend";
	}

	//____ flushAndWait() _______________________________________________________________

	void MetalGfxDevice::flushAndWait()
	{
		if( !m_bRendering )
			return;

		// Finalize any commands and execute queue.

		_endCommand();
		_executeBuffer();

		// Push the command buffer to the GPU.

		[m_metalCommandBuffer commit];
		[m_metalCommandBuffer waitUntilCompleted];

		_resetBuffers();

		// Create a new command buffer.
		m_metalCommandBuffer = [s_metalCommandQueue commandBuffer];
		m_metalCommandBuffer.label = @"MetalBackend";

	}

	//____ fill() ____ [standard] __________________________________________________

	void MetalGfxDevice::fill(const RectSPX& rect, HiColor col)
	{
		// Skip calls that won't affect destination

		if (col.a == 0 && (m_blendMode == BlendMode::Blend))
			return;

		if (!rect.isOverlapping(m_clipBounds))
			return;

		//

		if (m_vertexOfs > m_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > m_extrasBufferSize - 8)
			_resizeBuffers();

		//

		if (((rect.x | rect.y | rect.w | rect.h) & 0x3F) == 0)
		{
			// No subpixel precision, make it quick and easy


			if (m_cmd != Command::Fill)
			{
				_endCommand();
				_beginDrawCommand(Command::Fill);
			}

			for (int i = 0; i < m_nClipRects; i++)
			{
				RectI patch = roundToPixels(RectSPX::overlap(m_pClipRects[i], rect));
				if (patch.w > 0 && patch.h > 0)
				{
					int    dx1 = patch.x;
					int    dy1 = patch.y;
					int dx2 = patch.x + patch.w;
					int dy2 = patch.y + patch.h;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;
				}
			}

			m_pExtrasBuffer[m_extrasOfs++] = col.r / 4096.f;
			m_pExtrasBuffer[m_extrasOfs++] = col.g / 4096.f;
			m_pExtrasBuffer[m_extrasOfs++] = col.b / 4096.f;
			m_pExtrasBuffer[m_extrasOfs++] = col.a / 4096.f;
		}
		else
		{
			// We have subpixel precision

			// Create our outer rectangle

			RectI outerRect( rect.x / 64, rect.y / 64, ((rect.x+rect.w+63)/64) - (rect.x/64), ((rect.y + rect.h + 63)/64) - (rect.y/64) );

			//

			if (m_cmd != Command::FillSubPixel)
			{
				_endCommand();
				_beginDrawCommand(Command::FillSubPixel);
			}

			for (int i = 0; i < m_nClipRects; i++)
			{
				RectI patch = RectI::overlap(m_pClipRects[i]/64, outerRect);
				if (patch.w > 0 && patch.h > 0)
				{
					int    dx1 = patch.x;
					int    dy1 = patch.y;
					int dx2 = patch.x + patch.w;
					int dy2 = patch.y + patch.h;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;

					m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
					m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
					m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexOfs++;
				}
			}

			// Provide color

			m_pExtrasBuffer[m_extrasOfs++] = col.r / 4096.f;
			m_pExtrasBuffer[m_extrasOfs++] = col.g / 4096.f;
			m_pExtrasBuffer[m_extrasOfs++] = col.b / 4096.f;
			m_pExtrasBuffer[m_extrasOfs++] = col.a / 4096.f;

			// Provide rectangle center and radius

			SizeF    radius(rect.w / (2.f*64), rect.h / (2.f*64));
			CoordF    center( (rect.x/64.f) + radius.w, (rect.y/64.f) + radius.h);

			m_pExtrasBuffer[m_extrasOfs++] = center.x;
			m_pExtrasBuffer[m_extrasOfs++] = center.y;
			m_pExtrasBuffer[m_extrasOfs++] = radius.w;
			m_pExtrasBuffer[m_extrasOfs++] = radius.h;

		}
	}

	//____ drawLine() ____ [from/to] __________________________________________________

	void MetalGfxDevice::drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx _thickness)
	{
		//TODO: Proper 26:6 support
		begin = roundToPixels(begin);
		end = roundToPixels(end);

		float thickness = _thickness/64.f;

		// Skip calls that won't affect destination

		if (color.a == 0 && (m_blendMode == BlendMode::Blend))
			return;

		//

		if (m_vertexOfs > m_vertexBufferSize - 6 || m_extrasOfs > m_extrasBufferSize - 8 )
			  _resizeBuffers();

		if (m_cmd != Command::Line || m_clipCurrOfs == -1)
		{
			_endCommand();
			_beginClippedDrawCommand(Command::Line);
		}

		int     length;
		float   width;

		float    slope;
		float    s, w;
		bool    bSteep;

		CoordI    c1, c2, c3, c4;

		if (std::abs(begin.x - end.x) > std::abs(begin.y - end.y))
		{
			// Prepare mainly horizontal line segment

			if (begin.x > end.x)
				std::swap(begin, end);

			length = end.x - begin.x;
			if (length == 0)
				return;                                            // TODO: Should stil draw the caps!

			slope = ((float)(end.y - begin.y)) / length;
			width = _scaleThickness(thickness, slope);
			bSteep = false;

			s = ((begin.y + 0.5f) - (begin.x + 0.5f)*slope);
			w =  width / 2 + 0.5f;

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
				return;                                            // TODO: Should stil draw the caps!

			slope = ((float)(end.x - begin.x)) / length;
			width = _scaleThickness(thickness, slope);
			bSteep = true;

			s = (begin.x + 0.5f) - ((begin.y + 0.5f))*slope;
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

		m_pVertexBuffer[m_vertexOfs].coord = c1;
		m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexOfs++;

		m_pVertexBuffer[m_vertexOfs].coord = c2;
		m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexOfs++;

		m_pVertexBuffer[m_vertexOfs].coord = c3;
		m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexOfs++;

		m_pVertexBuffer[m_vertexOfs].coord = c1;
		m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexOfs++;

		m_pVertexBuffer[m_vertexOfs].coord = c3;
		m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexOfs++;

		m_pVertexBuffer[m_vertexOfs].coord = c4;
		m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexOfs++;

		m_pExtrasBuffer[m_extrasOfs++] = color.r / 4096.f;
		m_pExtrasBuffer[m_extrasOfs++] = color.g / 4096.f;
		m_pExtrasBuffer[m_extrasOfs++] = color.b / 4096.f;
		m_pExtrasBuffer[m_extrasOfs++] = color.a / 4096.f;

		m_pExtrasBuffer[m_extrasOfs++] = s;
		m_pExtrasBuffer[m_extrasOfs++] = w;
		m_pExtrasBuffer[m_extrasOfs++] = slope;
		m_pExtrasBuffer[m_extrasOfs++] = bSteep;

	}

	//____ drawLine() ____ [start/direction] __________________________________________________

	void MetalGfxDevice::drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx _thickness)
	{
		//TODO: Proper 26:6 support
		begin = roundToPixels(begin);
		length = roundToPixels(length);

		float thickness = _thickness/64.f;

		// Skip calls that won't affect destination

		if (color.a == 0 && (m_blendMode == BlendMode::Blend))
			return;

		// Create a rectangle from the line

		RectF rect;

		switch (dir)
		{
			case Direction::Up:
				rect.x = begin.x + 0.5f - thickness/2;
				rect.y = float(begin.y - length);
				rect.w = thickness;
				rect.h = float(length);
				break;

			case Direction::Down:
				rect.x = begin.x + 0.5f - thickness/2;
				rect.y = float(begin.y);
				rect.w = thickness;
				rect.h = float(length);
				break;

			case Direction::Left:
				rect.x = float(begin.x - length);
				rect.y = begin.y + 0.5f - thickness/2;
				rect.w = float(length);
				rect.h = thickness;
				break;

			case Direction::Right:
				rect.x = float(begin.x);
				rect.y = begin.y + 0.5f - thickness/2;
				rect.w = float(length);
				rect.h = thickness;
				break;
		}


		// Create our outer rectangle

		RectI outerRect((int)rect.x, (int)rect.y, ((int)(rect.x + rect.w + 0.999f)) - (int)rect.x, ((int)(rect.y + rect.h + 0.999f)) - (int)rect.y);

		// Clip our rectangle

		if (!outerRect.isOverlapping(m_clipBounds/64))
			return;

		//

		if (m_vertexOfs > m_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > m_extrasBufferSize - 8)
			_resizeBuffers();

		if (m_cmd != Command::FillSubPixel)
		{
			_endCommand();
			_beginDrawCommand(Command::FillSubPixel);
		}


		// Provide the patches

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, outerRect);
			if (patch.w > 0 && patch.h > 0)
			{
				int    dx1 = patch.x;
				int    dy1 = patch.y;
				int dx2 = patch.x + patch.w;
				int dy2 = patch.y + patch.h;

				m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
				m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
				m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexOfs++;

				m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
				m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
				m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexOfs++;

				m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
				m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
				m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexOfs++;

				m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
				m_pVertexBuffer[m_vertexOfs].coord.y = dy1;
				m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexOfs++;

				m_pVertexBuffer[m_vertexOfs].coord.x = dx2;
				m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
				m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexOfs++;

				m_pVertexBuffer[m_vertexOfs].coord.x = dx1;
				m_pVertexBuffer[m_vertexOfs].coord.y = dy2;
				m_pVertexBuffer[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexOfs++;
			}
		}

		// Provide color

		m_pExtrasBuffer[m_extrasOfs++] = color.r / 4096.f;
		m_pExtrasBuffer[m_extrasOfs++] = color.g / 4096.f;
		m_pExtrasBuffer[m_extrasOfs++] = color.b / 4096.f;
		m_pExtrasBuffer[m_extrasOfs++] = color.a / 4096.f;


		// Provide rectangle center and raidus.

		SizeF    radius(rect.w / 2, rect.h / 2);
		CoordF    center(rect.x + radius.w, rect.y + radius.h);

		m_pExtrasBuffer[m_extrasOfs++] = center.x;
		m_pExtrasBuffer[m_extrasOfs++] = center.y;
		m_pExtrasBuffer[m_extrasOfs++] = radius.w;
		m_pExtrasBuffer[m_extrasOfs++] = radius.h;
	}

	//____ drawEdgemap() __________________________________________________________

	void MetalGfxDevice::drawEdgemap(CoordSPX destPos, Edgemap * pEdgemap )
	{
		if( pEdgemap->typeInfo() != MetalEdgemap::TYPEINFO )
		{
			//TODO: Throw an error.
			return;
		}

		auto pWave = static_cast<MetalEdgemap*>(pEdgemap);

		TintMode mode = TintMode::Flat;

		if (pEdgemap->paletteType() == EdgemapPalette::ColorstripXY)
			mode = TintMode::GradientXY;
		else if (pEdgemap->paletteType() == EdgemapPalette::ColorstripX)
			mode = TintMode::GradientX;
		else if (pEdgemap->paletteType() == EdgemapPalette::ColorstripY)
			mode = TintMode::GradientY;

		HiColor		palette[Edgemap::maxSegments * 4];

		pEdgemap->exportLegacyPalette(palette);

		_transformDrawSegments( {destPos,pWave->m_size*64}, pWave->m_nbRenderSegments, palette,
							   pWave->m_size.w+1, pWave->m_pSamples, pWave->m_nbSegments-1, mode,
							   s_blitFlipTransforms[(int)GfxFlip::None] );
	}

	//____ flipDrawEdgemap() ______________________________________________________

	void MetalGfxDevice::flipDrawEdgemap(CoordSPX destPos, Edgemap * pEdgemap, GfxFlip flip)
	{
		if( pEdgemap->typeInfo() != MetalEdgemap::TYPEINFO )
		{
			//TODO: Throw an error.
			return;
		}

		auto pWave = static_cast<MetalEdgemap*>(pEdgemap);

		const int (&transform)[2][2] = s_blitFlipTransforms[(int)flip];

		RectSPX dest;
		dest.x = destPos.x;
		dest.y = destPos.y;
		dest.w = pWave->m_size.w*64 * abs(transform[0][0]) + pWave->m_size.h*64 * abs(transform[1][0]);
		dest.h = pWave->m_size.w*64 * abs(transform[0][1]) + pWave->m_size.h*64 * abs(transform[1][1]);

		TintMode mode = TintMode::Flat;

		if (pEdgemap->paletteType() == EdgemapPalette::ColorstripXY)
			mode = TintMode::GradientXY;
		else if (pEdgemap->paletteType() == EdgemapPalette::ColorstripX)
			mode = TintMode::GradientX;
		else if (pEdgemap->paletteType() == EdgemapPalette::ColorstripY)
			mode = TintMode::GradientY;


		HiColor		palette[Edgemap::maxSegments * 4];

		pEdgemap->exportLegacyPalette(palette);

		_transformDrawSegments( dest, pWave->m_nbRenderSegments, palette,
							   pWave->m_size.w+1, pWave->m_pSamples, pWave->m_nbSegments-1, mode,
							   transform );
	}

	//____ _transformBlitSimple() ______________________________________________________

	void MetalGfxDevice::_transformBlitSimple(const RectSPX& dest, CoordSPX src, const int simpleTransform[2][2],  OpType type)
	{
		if (m_pBlitSource == nullptr)
			return;

		if (!dest.isOverlapping(m_clipBounds))
			return;

		if (m_vertexOfs > m_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > m_extrasBufferSize - 8 )
			_resizeBuffers();


		if( type == OpType::Blur && m_cmd != Command::Blur )
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blur);
		}
		else if (m_cmd != Command::Blit)
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blit);
		}

		//TODO: Proper 26:6 support
//        RectI dest = roundToPixels(_dest);

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = roundToPixels(RectI::overlap(m_pClipRects[i], dest));
			if (patch.w > 0 && patch.h > 0)
			{
				int        dx1 = patch.x;
				int        dx2 = patch.x + patch.w;
				int        dy1 = patch.y;
				int        dy2 = patch.y + patch.h;

				Vertex * pVertex = m_pVertexBuffer + m_vertexOfs;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs/4;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				m_vertexOfs += 6;
			}
		}

		m_pExtrasBuffer[m_extrasOfs++] = (float) roundToPixels(src.x);
		m_pExtrasBuffer[m_extrasOfs++] = (float) roundToPixels(src.y);
		m_pExtrasBuffer[m_extrasOfs++] = (float) roundToPixels(dest.x);
		m_pExtrasBuffer[m_extrasOfs++] = (float) roundToPixels(dest.y);

		m_pExtrasBuffer[m_extrasOfs++] = (float) simpleTransform[0][0];
		m_pExtrasBuffer[m_extrasOfs++] = (float) simpleTransform[0][1];
		m_pExtrasBuffer[m_extrasOfs++] = (float) simpleTransform[1][0];
		m_pExtrasBuffer[m_extrasOfs++] = (float) simpleTransform[1][1];
	}

	//____ _transformBlitComplex() ______________________________________________________

	void MetalGfxDevice::_transformBlitComplex(const RectSPX& _dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type)
	{
		if (m_pBlitSource == nullptr)
			return;

		if (!_dest.isOverlapping(m_clipBounds))
			return;

		if (m_vertexOfs > m_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > m_extrasBufferSize - 8)
			_resizeBuffers();

		if( type == OpType::Blur && m_cmd != Command::Blur )
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blur);
		}
		else if (m_cmd != Command::Blit)
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blit);
		}

		//

		//TODO: Proper 26:6 support
		//TODO: Proper 26:6 support
		RectI dest = roundToPixels(_dest);

		//

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, dest);
			if (patch.w > 0 && patch.h > 0)
			{
				Vertex * pVertex = m_pVertexBuffer + m_vertexOfs;

				int        dx1 = patch.x;
				int        dx2 = patch.x + patch.w;
				int        dy1 = patch.y;
				int        dy2 = patch.y + patch.h;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex++;

				m_vertexOfs += 6;
			}
		}

		if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
		{
			m_pExtrasBuffer[m_extrasOfs++] = (float(src.x) / BINAL_MUL) + 0.5f;
			m_pExtrasBuffer[m_extrasOfs++] = (float(src.y) / BINAL_MUL) + 0.5f;
			m_pExtrasBuffer[m_extrasOfs++] = dest.x + 0.5f;
			m_pExtrasBuffer[m_extrasOfs++] = dest.y + 0.5f;
		}
		else
		{
			m_pExtrasBuffer[m_extrasOfs++] = (float(src.x) / BINAL_MUL);                //TODO: Ugly patch. Figure out what exactly goes wrong and fix it!
			m_pExtrasBuffer[m_extrasOfs++] = (float(src.y) / BINAL_MUL);                //TODO: Ugly patch. Figure out what exactly goes wrong and fix it!
			m_pExtrasBuffer[m_extrasOfs++] = dest.x +0.5f;
			m_pExtrasBuffer[m_extrasOfs++] = dest.y +0.5f;
		}

		m_pExtrasBuffer[m_extrasOfs++] = float(complexTransform[0][0]) / BINAL_MUL;
		m_pExtrasBuffer[m_extrasOfs++] = float(complexTransform[0][1]) / BINAL_MUL;
		m_pExtrasBuffer[m_extrasOfs++] = float(complexTransform[1][0]) / BINAL_MUL;
		m_pExtrasBuffer[m_extrasOfs++] = float(complexTransform[1][1]) / BINAL_MUL;
	}

	//____ _transformDrawSegments() ___________________________________________________

	void MetalGfxDevice::_transformDrawSegments(const RectSPX& _destIn, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2])
	{
		if (!_destIn.isOverlapping(m_clipBounds))
			return;

		//

		//TODO: Proper 26:6 support
		RectI _dest = roundToPixels(_destIn);

		int segEdgeSpaceNeeded = 4 * (nEdgeStrips - 1)*(nSegments - 1);

		if( m_segPalOfs == m_segPalBufferSize )
		{
			flushAndWait();
			m_nSegments = nSegments;
			_beginDrawCommandWithInt(Command::Segments, m_nSegments);
		}

		if (m_vertexOfs > m_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > m_extrasBufferSize - 8 || m_segEdgeOfs > m_segEdgeBufferSize - segEdgeSpaceNeeded )            // various data, transform , colors, edgestrips
		{
			m_neededSegEdge = segEdgeSpaceNeeded;
			_resizeBuffers();
		}

		if (m_cmd != Command::Segments || m_nSegments != nSegments )
		{
			m_nSegments = nSegments;

			_endCommand();
			_beginDrawCommandWithInt(Command::Segments, m_nSegments);
		}

		// Do transformations

		RectI dest = _dest;

		int uIncX = simpleTransform[0][0];
		int vIncX = simpleTransform[0][1];
		int uIncY = simpleTransform[1][0];
		int vIncY = simpleTransform[1][1];

		// Possibly clip the destination rectangle if we have space for more columns than we have

		int maxCol = (nEdgeStrips - 1);
		if (uIncX != 0)                                // Columns are aligned horizontally
		{
			if (dest.w > maxCol)
			{
				if (uIncX < 0)
					dest.x += dest.w - maxCol;
				dest.w = maxCol;
			}
		}
		else                                        // Columns are aligned vertically
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

		// Setup vertices

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, dest);
			if (patch.w > 0 && patch.h > 0)
			{
				Vertex * pVertex = m_pVertexBuffer + m_vertexOfs;

				int        dx1 = patch.x;
				int        dx2 = patch.x + patch.w;
				int        dy1 = patch.y;
				int        dy2 = patch.y + patch.h;

				// Calc UV-coordinates. U is edge offset, V is pixel offset from begin in column.

				float    u1 = (float) (uTopLeft + (patch.x - dest.x) * simpleTransform[0][0] + (patch.y - dest.y) * simpleTransform[1][0]);
				float    v1 = (float) (vTopLeft + (patch.x - dest.x) * simpleTransform[0][1] + (patch.y - dest.y) * simpleTransform[1][1]);

				float    u2 = (float) (uTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][0] + (patch.y - dest.y) * simpleTransform[1][0]);
				float    v2 = (float) (vTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][1] + (patch.y - dest.y) * simpleTransform[1][1]);

				float    u3 = (float) (uTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][0] + (patch.y + patch.h - dest.y) * simpleTransform[1][0]);
				float    v3 = (float) (vTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][1] + (patch.y + patch.h - dest.y) * simpleTransform[1][1]);

				float    u4 = (float) (uTopLeft + (patch.x - dest.x) * simpleTransform[0][0] + (patch.y + patch.h - dest.y) * simpleTransform[1][0]);
				float    v4 = (float) (vTopLeft + (patch.x - dest.x) * simpleTransform[0][1] + (patch.y + patch.h - dest.y) * simpleTransform[1][1]);

				float uMod = 0; //-0.5f;
				float vMod = -0.5f;

				CoordF    uv1 = { u1 + uMod, v1 + vMod };
				CoordF    uv2 = { u2 + uMod, v2 + vMod };
				CoordF    uv3 = { u3 + uMod, v3 + vMod };
				CoordF    uv4 = { u4 + uMod, v4 + vMod };


				//

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->uv = { uv1.x, uv1.y };
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->uv = { uv2.x, uv2.y };
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->uv = { uv3.x, uv3.y };
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->uv = { uv1.x, uv1.y };
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->uv = { uv3.x, uv3.y };
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->uv = { uv4.x, uv4.y };
				pVertex++;

				m_vertexOfs += 6;
			}
		}

		// Setup extras data

		GLfloat * pExtras = m_pExtrasBuffer + m_extrasOfs;

		// Add various data to extras

		pExtras[0] = (float) nSegments;
		pExtras[1] = (float) m_segEdgeOfs/4;
		pExtras[2] = (float)((_dest.w) * abs(simpleTransform[0][0]) + (_dest.h) * abs(simpleTransform[1][0]));
		pExtras[3] = (float)((_dest.w) * abs(simpleTransform[0][1]) + (_dest.h) * abs(simpleTransform[1][1]));
		pExtras[4] = float( 0.25f/c_maxSegments );
		pExtras[5] = float(m_segPalOfs + 0.25f) / m_segPalBufferSize;
		pExtras[6] = float(c_maxSegments*2);
		pExtras[7] = float(m_segPalBufferSize*2);

		m_extrasOfs += 8;

		// Add colors to segmentsTintTexMap

		const HiColor* pSegCol = pSegmentColors;

		uint16_t* pMapRow = &m_pSegPalBuffer[m_segPalOfs*c_segPalEntrySize/2];
		int            mapPitch = c_maxSegments * 4 * 2;

		switch (tintMode)
		{
			case TintMode::None:
			case TintMode::Flat:
			{
				for (int i = 0; i < nSegments; i++)
				{
					uint16_t r = uint16_t(int(pSegCol->r) * 65535 / 4096);
					uint16_t g = uint16_t(int(pSegCol->g) * 65535 / 4096);
					uint16_t b = uint16_t(int(pSegCol->b) * 65535 / 4096);
					uint16_t a = uint16_t(int(pSegCol->a) * 65535 / 4096);

					pMapRow[i * 8 + 0] = r;
					pMapRow[i * 8 + 1] = g;
					pMapRow[i * 8 + 2] = b;
					pMapRow[i * 8 + 3] = a;
					pMapRow[i * 8 + 4] = r;
					pMapRow[i * 8 + 5] = g;
					pMapRow[i * 8 + 6] = b;
					pMapRow[i * 8 + 7] = a;

					pMapRow[mapPitch + i * 8 + 0] = r;
					pMapRow[mapPitch + i * 8 + 1] = g;
					pMapRow[mapPitch + i * 8 + 2] = b;
					pMapRow[mapPitch + i * 8 + 3] = a;
					pMapRow[mapPitch + i * 8 + 4] = r;
					pMapRow[mapPitch + i * 8 + 5] = g;
					pMapRow[mapPitch + i * 8 + 6] = b;
					pMapRow[mapPitch + i * 8 + 7] = a;
					pSegCol++;
				}
				break;
			}

			case TintMode::GradientX:
			{
				for (int i = 0; i < nSegments; i++)
				{
					int r1 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					int g1 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					int b1 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					int a1 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					int r2 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					int g2 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					int b2 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					int a2 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[i * 8 + 0] = r1;
					pMapRow[i * 8 + 1] = g1;
					pMapRow[i * 8 + 2] = b1;
					pMapRow[i * 8 + 3] = a1;
					pMapRow[i * 8 + 4] = r2;
					pMapRow[i * 8 + 5] = g2;
					pMapRow[i * 8 + 6] = b2;
					pMapRow[i * 8 + 7] = a2;

					pMapRow[mapPitch + i * 8 + 0] = r1;
					pMapRow[mapPitch + i * 8 + 1] = g1;
					pMapRow[mapPitch + i * 8 + 2] = b1;
					pMapRow[mapPitch + i * 8 + 3] = a1;
					pMapRow[mapPitch + i * 8 + 4] = r2;
					pMapRow[mapPitch + i * 8 + 5] = g2;
					pMapRow[mapPitch + i * 8 + 6] = b2;
					pMapRow[mapPitch + i * 8 + 7] = a2;
				}
				break;
			}

			case TintMode::GradientY:
			{
				for (int i = 0; i < nSegments; i++)
				{
					int r1 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					int g1 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					int b1 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					int a1 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					int r2 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					int g2 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					int b2 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					int a2 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[i * 8 + 0] = r1;
					pMapRow[i * 8 + 1] = g1;
					pMapRow[i * 8 + 2] = b1;
					pMapRow[i * 8 + 3] = a1;
					pMapRow[i * 8 + 4] = r1;
					pMapRow[i * 8 + 5] = g1;
					pMapRow[i * 8 + 6] = b1;
					pMapRow[i * 8 + 7] = a1;

					pMapRow[mapPitch + i * 8 + 0] = r2;
					pMapRow[mapPitch + i * 8 + 1] = g2;
					pMapRow[mapPitch + i * 8 + 2] = b2;
					pMapRow[mapPitch + i * 8 + 3] = a2;
					pMapRow[mapPitch + i * 8 + 4] = r2;
					pMapRow[mapPitch + i * 8 + 5] = g2;
					pMapRow[mapPitch + i * 8 + 6] = b2;
					pMapRow[mapPitch + i * 8 + 7] = a2;
				}
				break;
			}

			case TintMode::GradientXY:
			{
				for (int i = 0; i < nSegments; i++)
				{
					pMapRow[i * 8 + 0] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[i * 8 + 1] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[i * 8 + 2] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[i * 8 + 3] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[i * 8 + 4] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[i * 8 + 5] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[i * 8 + 6] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[i * 8 + 7] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[mapPitch + i * 8 + 4] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 5] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 6] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 7] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[mapPitch + i * 8 + 0] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 1] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 2] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 3] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;
				}
				break;
			}
		}

		m_segPalOfs++;

		// Add our edge strips to segEdgeBuffer;

		float * pEdgeData = m_pSegEdgeBuffer + m_segEdgeOfs;

		const int * pEdges = pEdgeStrips;

		for (int i = 0; i < nEdgeStrips-1; i++)
		{
			for (int j = 0; j < nSegments - 1; j++)
			{
				int edgeIn = pEdges[j];
				int edgeOut = pEdges[edgeStripPitch+j];

				if (edgeIn > edgeOut)
					std::swap(edgeIn, edgeOut);

				float increment = edgeOut == edgeIn ? 100.f : 64.f / (edgeOut - edgeIn);
				float beginAdder;
				float endAdder;

				if ((edgeOut & 0xFFFFFFC0) <= (unsigned int) edgeIn)
				{
					float firstPixelCoverage = ((64 - (edgeOut & 0x3F)) + (edgeOut - edgeIn) / 2) / 64.f;

					beginAdder = increment * (edgeIn & 0x3F)/64.f + firstPixelCoverage;
					endAdder = beginAdder;
				}
				else
				{
					int height = 64 - (edgeIn & 0x3F);
					int width = (int)(increment * height);
					float firstPixelCoverage = (height * width) / (2 * 4096.f);
					float lastPixelCoverage = 1.f - (edgeOut & 0x3F)*increment*(edgeOut & 0x3F) / (2*4096.f);

					beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
					endAdder = lastPixelCoverage - (1.f - (edgeOut & 0x3F)*increment / 64.f);
//                     endAdder = lastPixelCoverage - ((edgeOut & 0xFFFFFF00)-edgeIn)*increment / 256.f;
				}

				*pEdgeData++ = edgeIn/64.f;                    // Segment begin pixel
				*pEdgeData++ = increment;                        // Segment increment
				*pEdgeData++ = beginAdder;                    // Segment begin adder
				*pEdgeData++ = endAdder;                        // Segment end adder
			}

			pEdges += edgeStripPitch;
		}

		m_segEdgeOfs+= segEdgeSpaceNeeded;
	}

	//____ _resizeBuffers() ____________________________________________________

	void MetalGfxDevice::_resizeBuffers()
	{
		// We are modifying buffers even though a flush might be in progress.
		// We are assuming that original buffer will be kept alive and utilized during the flush.

		if( (m_commandBufferSize - m_commandOfs) < m_commandBufferSize/4 )
		{
			m_commandBufferSize *= 2;
			int * pNewBuffer = new int[m_commandBufferSize];
			memcpy( pNewBuffer, m_pCommandBuffer, m_commandOfs * sizeof(int));
			delete [] m_pCommandBuffer;
			m_pCommandBuffer = pNewBuffer;
		}

		if( (m_vertexBufferSize - m_vertexOfs) < m_vertexBufferSize/4 )
		{
			m_vertexBufferSize *= 2;

			id<MTLBuffer> newId = [s_metalDevice newBufferWithLength:m_vertexBufferSize*sizeof(Vertex) options:MTLResourceStorageModeShared];

			Vertex * pNewBuffer = (Vertex *)[newId contents];
			memcpy( pNewBuffer, m_pVertexBuffer, m_vertexOfs * sizeof(Vertex));
			m_pVertexBuffer = pNewBuffer;

			[m_vertexBufferId release];
			m_vertexBufferId = newId;
		}

		if( (m_extrasBufferSize - m_extrasOfs) < m_extrasBufferSize/4 || (m_extrasBufferSize - m_extrasOfs) < m_neededExtras )
		{
			m_extrasBufferSize *= 2;
			while( (m_extrasBufferSize - m_extrasOfs) < m_neededExtras )
				m_extrasBufferSize *= 2;
			m_neededExtras = 0;

			id<MTLBuffer> newId = [s_metalDevice newBufferWithLength:m_extrasBufferSize*sizeof(float) options:MTLResourceStorageModeShared];

			float * pNewBuffer = (float*)[newId contents];
			memcpy( pNewBuffer, m_pExtrasBuffer, m_extrasOfs * sizeof(float));
			m_pExtrasBuffer = pNewBuffer;

			[m_extrasBufferId release];
			m_extrasBufferId = newId;
		}

		if( (m_clipListBufferSize - m_clipWriteOfs) < m_clipListBufferSize/4 )
		{
			m_clipListBufferSize *= 2;

			id<MTLBuffer> newId = [s_metalDevice newBufferWithLength:m_clipListBufferSize*sizeof(RectI) options:MTLResourceStorageModeShared];

			RectI * pNewBuffer = (RectI *)[newId contents];
			memcpy( pNewBuffer, m_pClipListBuffer, m_clipWriteOfs * sizeof(RectI));
			m_pClipListBuffer = pNewBuffer;

			[m_clipListBufferId release];
			m_clipListBufferId = newId;
		}

		if( (m_segEdgeBufferSize - m_segEdgeOfs ) < m_segEdgeBufferSize/4 || (m_segEdgeBufferSize - m_segEdgeOfs) < m_neededSegEdge )
		{
			m_segEdgeBufferSize *= 2;
			while( (m_segEdgeBufferSize - m_segEdgeOfs) < m_neededSegEdge )
				m_segEdgeBufferSize *= 2;
			m_neededSegEdge = 0;

			id<MTLBuffer> newId = [s_metalDevice newBufferWithLength:m_segEdgeBufferSize*sizeof(float) options:MTLResourceStorageModeManaged];

			float * pNewBuffer = (float*)[newId contents];
			memcpy( pNewBuffer, m_pSegEdgeBuffer, m_segEdgeOfs * sizeof(float));
			m_pSegEdgeBuffer = pNewBuffer;

			[m_segEdgeBufferId release];
			m_segEdgeBufferId = newId;
		}

	}


	//____ _executeBuffer() ___________________________________________________

	void MetalGfxDevice::_executeBuffer()
	{
		if( m_commandOfs == 0 )
			return;

		// Sync segments segEdge buffer to GPU

		if( m_segEdgeOfs > m_segEdgeFlushPoint )
		{
			[m_segEdgeBufferId didModifyRange:NSMakeRange(m_segEdgeFlushPoint*sizeof(float), (m_segEdgeOfs-m_segEdgeFlushPoint)*sizeof(float))];
		}


		// Update segments palette texture

		if( m_segPalOfs > 0 )
		{
			int bytesPerRow = sizeof(uint16_t)*4*2*c_maxSegments;

			MTLSize textureSize = { (unsigned) c_maxSegments*2, (unsigned) m_segPalOfs*2, 1};
			MTLOrigin textureOrigin = {0,0,0};

			id<MTLBlitCommandEncoder> blitCommandEncoder = [m_metalCommandBuffer blitCommandEncoder];
			[blitCommandEncoder copyFromBuffer:     m_segPalBufferId
								sourceOffset:       0
								sourceBytesPerRow:  bytesPerRow
								sourceBytesPerImage:m_segPalOfs*c_segPalEntrySize
								sourceSize:         textureSize
								toTexture:          m_segPalTextureId
								destinationSlice:   0
								destinationLevel:   0
								destinationOrigin:  textureOrigin];

			[blitCommandEncoder endEncoding];
			blitCommandEncoder = nil;

			//TODO: We need to sync this, don't start drawing segments until this has been fully uploaded.
		}

		// Create our render encoder.

		id<MTLRenderCommandEncoder> renderEncoder = nil;

		int * pCmd = m_pCommandBuffer;
		int * pCmdEnd = &m_pCommandBuffer[m_commandOfs];

		int vertexOfs = m_vertexFlushPoint;

		// Set initial state of canvas

		if( m_pActiveCanvas )
		{
			renderEncoder = _setCanvas( m_pActiveCanvas, m_activeCanvasSize.w, m_activeCanvasSize.h, CanvasInit::Keep, Color::Black );
			_setBlitSource( renderEncoder, m_pActiveBlitSource );
		}


		// Clear pending flags of active BlitSource and Canvas.

	//    if (m_pActiveBlitSource)
	//        m_pActiveBlitSource->m_bPendingReads = false;

		while (pCmd < pCmdEnd)
		{
			Command cmd = (Command) * pCmd++;

			switch (cmd)
			{
				case Command::None:
					break;

				case Command::SetCanvas:
				{
					if( renderEncoder != nil )
					{
						[renderEncoder endEncoding];
						renderEncoder = nil;
					}

					if( pCmd[0] == 0 && pCmd[1] == 0)
					{
						m_pActiveCanvas = nullptr;
						m_activeCanvasSize.clear();
					}
					else
					{
						MetalSurface* pSurf = *((MetalSurface**)(pCmd+4));
						renderEncoder = _setCanvas(pSurf, pCmd[0], pCmd[1], (CanvasInit) pCmd[2], pCmd[3]);

						if( pSurf )
							pSurf->release();

					}
					pCmd += 4 + sizeof(MetalSurface*)/sizeof(int);
					break;
				}
				case Command::SetBlendMode:
				{
					_setBlendMode(renderEncoder, (BlendMode)* pCmd++);
					break;
				}
				case Command::SetMorphFactor:
				{
					_setMorphFactor(renderEncoder, (*pCmd++) / 1024.f);
					break;
				}
				case Command::SetTintColor:
				{
					_setTintColor(renderEncoder, *(HiColor*)(pCmd));
					pCmd+=2;
					break;
				}
				case Command::SetFixedBlendColor:
				{
					_setFixedBlendColor(renderEncoder, *(HiColor*)(pCmd));
					pCmd+=2;
					break;
				}

				case Command::SetBlurMatrices:
				{
					spx radius = * pCmd++;

					_setBlurMatrices(renderEncoder, radius, (float *)(pCmd), (float *)(pCmd+9), (float *)(pCmd+18) );
					pCmd+=27;
					break;
				}

				case Command::SetTintGradient:
				{
					RectI& rect = *(RectI*)pCmd;
					pCmd += 4;
					Gradient* pGradient = (Gradient*)pCmd;
					pCmd += sizeof(Gradient)/4;

					_setTintGradient(renderEncoder, rect, * pGradient);
					break;
				}
				case Command::ClearTintGradient:
				{
					 _clearTintGradient(renderEncoder);
				   break;
				}
				case Command::SetBlitSource:
				{
					MetalSurface* pSurf = *((MetalSurface**)(pCmd));

					if( pSurf && pSurf->m_bMipmapStale )
					{
						if( renderEncoder != nil )
							[renderEncoder endEncoding];

						id<MTLBlitCommandEncoder> blitCommandEncoder = [m_metalCommandBuffer blitCommandEncoder];

						[blitCommandEncoder generateMipmapsForTexture:pSurf->m_texture];
						[blitCommandEncoder endEncoding];
						blitCommandEncoder = nil;

						if( renderEncoder != nil )
							renderEncoder = _setCanvas( m_pActiveCanvas, m_activeCanvasSize.w, m_activeCanvasSize.h, CanvasInit::Keep, Color::White );

						pSurf->m_bMipmapStale = false;
					}

					_setBlitSource(renderEncoder, pSurf);
					if( pSurf )
						pSurf->release();
					pCmd += sizeof(MetalSurface*)/sizeof(int);
					break;
				}
				case Command::Blit:
				{
					int nVertices = *pCmd++;
					if (nVertices > 0 && m_pActiveBlitSource)
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

						[renderEncoder setRenderPipelineState:m_blitPipelines[(int)shader][m_bGradientActive][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
						[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
						vertexOfs += nVertices;

						if( m_pActiveCanvas )
							m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					}
					break;
				}

				case Command::Blur:
				{
					int nVertices = *pCmd++;
					if (nVertices > 0 && m_pActiveBlitSource)
					{
						auto size = m_pActiveBlitSource->pixelSize();

						float radiusX = m_activeBlurRadius / float(size.w*64);
						float radiusY = m_activeBlurRadius / float(size.h*64);

						m_blurUniform.offset[0] = { -radiusX * 0.7f, -radiusY * 0.7f };
						m_blurUniform.offset[1] = { 0, -radiusY };
						m_blurUniform.offset[2] = { radiusX * 0.7f, -radiusY * 0.7f };

						m_blurUniform.offset[3] = { -radiusX, 0 };
						m_blurUniform.offset[4] = { 0, 0 };
						m_blurUniform.offset[5] = { radiusX, 0 };

						m_blurUniform.offset[6] = { -radiusX * 0.7f, radiusY * 0.7f };
						m_blurUniform.offset[7] = { 0, radiusY };
						m_blurUniform.offset[8] = { radiusX * 0.7f, radiusY * 0.7f };

						[renderEncoder setFragmentBytes:&m_blurUniform length:sizeof(BlurUniform) atIndex: (unsigned) FragmentInputIndex::BlurUniform];


						[renderEncoder setRenderPipelineState:m_blurPipelines[m_bGradientActive][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
						[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
						vertexOfs += nVertices;

						if( m_pActiveCanvas )
							m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					}
					break;
				}

				case Command::Fill:
				{
					int nVertices = *pCmd++;
					if( nVertices > 0 )
					{
						[renderEncoder setRenderPipelineState:m_fillPipelines[m_bGradientActive][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
						[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
						vertexOfs += nVertices;

						if( m_pActiveCanvas )
							m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					}
					break;
				}

				case Command::FillSubPixel:
				{
					int nVertices = *pCmd++;
					if( nVertices > 0 )
					{
						[renderEncoder setRenderPipelineState:m_fillAAPipelines[m_bGradientActive][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
						[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
						vertexOfs += nVertices;

						if( m_pActiveCanvas )
							m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					}
					break;
				}
				case Command::Line:
				{
					int clipListOfs = *pCmd++;
					int clipListLen = *pCmd++;
					int nVertices = *pCmd++;
					if( nVertices > 0 )
					{
						[renderEncoder setRenderPipelineState:m_linePipelines[(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];

						for (int i = 0; i < clipListLen; i++)
						{
							RectI& clip = m_pClipListBuffer[clipListOfs++];
							MTLScissorRect metalClip = {(unsigned) clip.x/64, (unsigned) clip.y/64, (unsigned) clip.w/64, (unsigned) clip.h/64};
							[renderEncoder setScissorRect:metalClip];
							[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
						}

						MTLScissorRect orgClip = {0, 0, (unsigned) m_activeCanvasSize.w, (unsigned) m_activeCanvasSize.h};
						[renderEncoder setScissorRect:orgClip];

						vertexOfs += nVertices;

						if( m_pActiveCanvas )
							m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					}
					break;
				}
				case Command::Plot:
				{
					 int nVertices = *pCmd++;
					 if( nVertices > 0 )
					 {
						 [renderEncoder setRenderPipelineState:m_plotPipelines[(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
						 [renderEncoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:vertexOfs vertexCount:nVertices];
						 vertexOfs += nVertices;

						 if( m_pActiveCanvas )
							 m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					 }
					 break;

				}
				case Command::Segments:
				{
					int nSegments = (*pCmd++);
					if (nSegments > c_maxSegments)
						nSegments = c_maxSegments;
					int nEdges = nSegments-1;
					int nVertices = *pCmd++;
					if( nVertices > 0 )
					{
						[renderEncoder setRenderPipelineState:m_segmentsPipelines[nEdges][m_bGradientActive][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
						[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
						vertexOfs += nVertices;

						if( m_pActiveCanvas )
							m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
					}
					break;
				}

				default:
	//                assert(false);
					break;
			}
		}

		if( renderEncoder != nil )
		{
			[renderEncoder endEncoding];
			renderEncoder = nil;
		}

		//

		m_vertexFlushPoint = m_vertexOfs;
		m_segEdgeFlushPoint = m_segEdgeOfs;
		m_commandOfs = 0;
		m_segPalOfs = 0;
	}

	//____ _resetBuffers() ____________________________________________________

	void MetalGfxDevice::_resetBuffers()
	{
		// Prepare our buffers

		m_vertexOfs = 0;
		m_extrasOfs = 0;
		m_commandOfs = 0;
		m_segEdgeOfs = 0;
		m_segPalOfs = 0;
		m_clipWriteOfs = 0;
		m_clipCurrOfs = -1;

		m_vertexFlushPoint = 0;
		m_segEdgeFlushPoint = 0;

	}

	//____ _setCanvas() _______________________________________________________

	id<MTLRenderCommandEncoder> MetalGfxDevice::_setCanvas( MetalSurface * pCanvas, int width, int height, CanvasInit initOperation, Color clearColor )
	{
		id<MTLRenderCommandEncoder> renderEncoder;

		PixelFormat pixelFormat;

		if (pCanvas)
		{
			MTLRenderPassDescriptor* pDescriptor = [MTLRenderPassDescriptor new];

			pDescriptor.colorAttachments[0].texture = pCanvas->getTexture();
			pDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

			switch(initOperation)
			{
				case CanvasInit::Keep:
					pDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
					break;

				case CanvasInit::Clear:
				{
					pDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
					float* pConv = GfxBase::defaultToSRGB() ? m_sRGBtoLinearTable : m_linearToLinearTable;
					pDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(pConv[clearColor.r]/4096.f, pConv[clearColor.g]/4096.f, pConv[clearColor.b]/4096.f, clearColor.a/255.f);
					break;
				}
				case CanvasInit::Discard:
					pDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
					break;
			}


			renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:pDescriptor];
			renderEncoder.label = @"GfxDeviceMetal Render to Surface Pass";
			[pDescriptor release];

			pixelFormat = pCanvas->pixelFormat();
		}
		else
		{

			renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:m_defaultCanvasRenderPassDesc];
			renderEncoder.label = @"GfxDeviceMetal Render Pass";

			pixelFormat = m_defaultCanvasPixelFormat;
		}

		[renderEncoder setViewport:(MTLViewport){0.0, 0.0, (double) width, (double) height, 0.0, 1.0 }];
		[renderEncoder setBlendColorRed:1.f green:1.f blue:1.f alpha:m_activeMorphFactor];


		// Updating canvas info for our shaders

		m_uniform.canvasDimX = (GLfloat) width;
		m_uniform.canvasDimY = (GLfloat) height;
		m_uniform.canvasYOfs = height;
		m_uniform.canvasYMul = -1;

		[renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex: (unsigned) VertexInputIndex::Uniform];

		// Set vertex and extras buffer

		[renderEncoder setVertexBuffer:m_vertexBufferId offset:0 atIndex:(unsigned) VertexInputIndex::VertexBuffer];
		[renderEncoder setVertexBuffer:m_extrasBufferId offset:0 atIndex:(unsigned) VertexInputIndex::ExtrasBuffer];

		// Set buffers/textures for segments fragment shader

		[renderEncoder setFragmentTexture:m_segPalTextureId atIndex: (unsigned) TextureIndex::SegPal];
		[renderEncoder setFragmentBuffer:m_segEdgeBufferId offset:0 atIndex:(unsigned) FragmentInputIndex::ExtrasBuffer];

		[renderEncoder setFragmentBytes:&m_blurUniform length:sizeof(BlurUniform) atIndex: (unsigned) FragmentInputIndex::BlurUniform];


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

	//____ _setBlendMode() _______________________________________________________

	void MetalGfxDevice::_setBlendMode( id<MTLRenderCommandEncoder> renderEncoder, BlendMode mode )
	{
		m_activeBlendMode = mode;

		// If BlendMode is Morph or BlendFixedColor we must make sure Metal's blendColor is set accordingly

		if( mode == BlendMode::Morph )
		{
			if( m_morphFactorInUse != m_activeMorphFactor )
			{
				[renderEncoder setBlendColorRed:1.f green:1.f blue:1.f alpha:m_activeMorphFactor];

				m_morphFactorInUse = m_activeMorphFactor;
				m_fixedBlendColorInUse = HiColor::Undefined;
			}
		}
		else if( mode == BlendMode::BlendFixedColor )
		{
			if( m_fixedBlendColorInUse != m_activeFixedBlendColor )
			{
				[renderEncoder setBlendColorRed:(m_activeFixedBlendColor.r/4096.f)
										  green:(m_activeFixedBlendColor.g/4096.f)
										   blue:(m_activeFixedBlendColor.b/4096.f)
										  alpha:(m_activeFixedBlendColor.a/4096.f)];

				m_morphFactorInUse = -1;
				m_fixedBlendColorInUse = m_activeFixedBlendColor;
			}
		}
	}

	//____ _setMorphFactor() __________________________________________________

	void MetalGfxDevice::_setMorphFactor( id<MTLRenderCommandEncoder> renderEncoder, float morphFactor)
	{
		m_activeMorphFactor = morphFactor;

		if( m_activeBlendMode == BlendMode::Morph )
		{
			if( m_morphFactorInUse != m_activeMorphFactor )
			{
				[renderEncoder setBlendColorRed:1.f green:1.f blue:1.f alpha:m_activeMorphFactor];

				m_morphFactorInUse = m_activeMorphFactor;
			}
		}
	}

	//____ _setFixedBlendColor() __________________________________________________

	void MetalGfxDevice::_setFixedBlendColor( id<MTLRenderCommandEncoder> renderEncoder, HiColor color)
	{
		m_activeFixedBlendColor = color;

		if( m_activeBlendMode == BlendMode::BlendFixedColor )
		{
			if( m_fixedBlendColorInUse != m_activeFixedBlendColor )
			{
				[renderEncoder setBlendColorRed:(m_activeFixedBlendColor.r/4096.f)
										  green:(m_activeFixedBlendColor.g/4096.f)
										   blue:(m_activeFixedBlendColor.b/4096.f)
										  alpha:(m_activeFixedBlendColor.a/4096.f)];

				m_fixedBlendColorInUse = m_activeFixedBlendColor;
			}
		}
	}

	//____ _setBlurMatrices() __________________________________________________

	void MetalGfxDevice::_setBlurMatrices( id<MTLRenderCommandEncoder> renderEncoder, spx radius, const float red[9], const float green[9], const float blue[9] )
	{
		m_activeBlurRadius = radius;

		m_blurUniform.colorMtx[0] = { red[0], green[0], blue[0], 0.f };
		m_blurUniform.colorMtx[1] = { red[1], green[1], blue[1], 0.f };
		m_blurUniform.colorMtx[2] = { red[2], green[2], blue[2], 0.f };
		m_blurUniform.colorMtx[3] = { red[3], green[3], blue[3], 0.f };
		m_blurUniform.colorMtx[4] = { red[4], green[4], blue[4], 1.f };
		m_blurUniform.colorMtx[5] = { red[5], green[5], blue[5], 0.f };
		m_blurUniform.colorMtx[6] = { red[6], green[6], blue[6], 0.f };
		m_blurUniform.colorMtx[7] = { red[7], green[7], blue[7], 0.f };
		m_blurUniform.colorMtx[8] = { red[8], green[8], blue[8], 0.f };

	}

	//____ _setBlitSource() _______________________________________________________

	void MetalGfxDevice::_setBlitSource( id<MTLRenderCommandEncoder> renderEncoder, MetalSurface * pSurf )
	{
		if( pSurf )
		{
			[renderEncoder setFragmentTexture:pSurf->getTexture() atIndex: (unsigned) TextureIndex::Texture];

			if(pSurf->pixelDescription()->type == PixelType::Index)
				[renderEncoder setFragmentSamplerState: m_samplers[0][0][pSurf->isTiling()] atIndex:0];
			else
				[renderEncoder setFragmentSamplerState: m_samplers[pSurf->isMipmapped()][pSurf->sampleMethod() == SampleMethod::Bilinear][pSurf->isTiling()] atIndex:0];

			m_pActiveBlitSource = pSurf;
			pSurf->m_bPendingReads = false;            // Clear this as we pass it by. All pending reads will have encoded before _executeBuffer() ends.

			m_uniform.textureSize = pSurf->pixelSize();
			[renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex:(unsigned) VertexInputIndex::Uniform];

			if (pSurf->m_pPalette)
			{
				[renderEncoder setFragmentTexture:pSurf->getPaletteTexture() atIndex:(unsigned) TextureIndex::Palette];
			}
		}
		else
		{
			[renderEncoder setFragmentTexture:nil atIndex:(unsigned) TextureIndex::Texture];
			[renderEncoder setFragmentSamplerState: m_samplers[0][0][0] atIndex:0];

			m_pActiveBlitSource = nullptr;
		}

	}

	//____ _setTintColor() ____________________________________________________

	void MetalGfxDevice::_setTintColor( id<MTLRenderCommandEncoder> renderEncoder, HiColor color)
	{
		m_uniform.flatTint[0] = color.r / 4096.f;
		m_uniform.flatTint[1] = color.g / 4096.f;
		m_uniform.flatTint[2] = color.b / 4096.f;
		m_uniform.flatTint[3] = color.a / 4096.f;

		[renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex:(unsigned) VertexInputIndex::Uniform];
	}

	//____ _setTintGradient() _________________________________________________

	void MetalGfxDevice::_setTintGradient( id<MTLRenderCommandEncoder> renderEncoder, const RectI& rect, const Gradient& gradient)
	{
		m_bGradientActive = true;

		m_uniform.tintRect = rect;

		m_uniform.topLeftTint[0] = gradient.topLeft.r / 4096.f;
		m_uniform.topLeftTint[1] = gradient.topLeft.g / 4096.f;
		m_uniform.topLeftTint[2] = gradient.topLeft.b / 4096.f;
		m_uniform.topLeftTint[3] = gradient.topLeft.a / 4096.f;

		m_uniform.topRightTint[0] = gradient.topRight.r / 4096.f;
		m_uniform.topRightTint[1] = gradient.topRight.g / 4096.f;
		m_uniform.topRightTint[2] = gradient.topRight.b / 4096.f;
		m_uniform.topRightTint[3] = gradient.topRight.a / 4096.f;

		m_uniform.bottomRightTint[0] = gradient.bottomRight.r / 4096.f;
		m_uniform.bottomRightTint[1] = gradient.bottomRight.g / 4096.f;
		m_uniform.bottomRightTint[2] = gradient.bottomRight.b / 4096.f;
		m_uniform.bottomRightTint[3] = gradient.bottomRight.a / 4096.f;

		m_uniform.bottomLeftTint[0] = gradient.bottomLeft.r / 4096.f;
		m_uniform.bottomLeftTint[1] = gradient.bottomLeft.g / 4096.f;
		m_uniform.bottomLeftTint[2] = gradient.bottomLeft.b / 4096.f;
		m_uniform.bottomLeftTint[3] = gradient.bottomLeft.a / 4096.f;

		[renderEncoder setVertexBytes:&m_uniform length:sizeof(Uniform) atIndex:(unsigned) VertexInputIndex::Uniform];
	}

	//____ _clearTintGradient() _________________________________________________

	void MetalGfxDevice::_clearTintGradient( id<MTLRenderCommandEncoder> renderEncoder )
	{
		m_bGradientActive = false;
	}



*/

} // namespace wg

