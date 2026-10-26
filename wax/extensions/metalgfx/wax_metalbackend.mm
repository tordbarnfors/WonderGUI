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
#include <wax_core.h>
#include <wax_gfxutil.h>

#include <cmath>


namespace wax
{
using namespace GfxUtil;

const TypeInfo MetalBackend::TYPEINFO = { "MetalBackend", &GfxBackend::TYPEINFO };

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

	if( m_library == nil )
	{
		NSString * pMessage = [NSString stringWithFormat:@"Failed to compile shaders: %@", error ? error.localizedDescription : @"unknown error"];
		Core::throwError(ErrorLevel::Critical, ErrorCode::Internal, pMessage.UTF8String, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	[shaderString release];

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

			// Like the GL and DX12 backends: tiling sources repeat, ClipBlit reads transparent
			// black outside the source and everything else, including Blur, reads the edge pixels.

			const MTLSamplerAddressMode addressModes[3] = { MTLSamplerAddressModeClampToEdge, MTLSamplerAddressModeRepeat, MTLSamplerAddressModeClampToZero };

			for( int edge = 0 ; edge < 3 ; edge++ )
			{
				desc.rAddressMode = addressModes[edge];
				desc.sAddressMode = addressModes[edge];
				desc.tAddressMode = addressModes[edge];

				m_samplers[mipmapped][interpolated][edge] = [s_metalDevice newSamplerStateWithDescriptor:desc];
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
	while( m_flushesInProgress > 0 )
	{
		usleep(100);
	}

	m_metalCommandBuffer = nil;

	[m_defaultCanvasRenderPassDesc release];
	m_defaultCanvasRenderPassDesc = nil;
	m_drawableToAutoPresent = nil;

	[m_vertexBufferId release];
	m_vertexBufferId = nil;

	[m_extrasBufferId release];
	m_extrasBufferId = nil;


	for( int mipmapped = 0 ; mipmapped < 2 ; mipmapped++ )
	{
		for( int interpolated = 0 ; interpolated < 2 ; interpolated++ )
		{
			for( int edge = 0 ; edge < 3 ; edge++ )
			{
				[m_samplers[mipmapped][interpolated][edge] release];
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

	// Release Blit pipelines

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


	for( int shader = 0 ; shader < 4 ; shader++ )
	{
		for( int tint = 0 ; tint < 2 ; tint++ )
		{
			for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
			{
				for( int format = 0 ; format < 5 ; format++ )
					[m_blurPipelines[shader][tint][blendMode][format] release];
			}
		}
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

	for( int tint = 0 ; tint < 2 ; tint++ )
	{
		for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
		{
			for( int format = 0 ; format < 5 ; format++ )
				[m_shapePipelines[tint][blendMode][format] release];
		}
	}

	for( int shader = 0 ; shader < 3 ; shader++ )
	{
		for( int blendMode = 0 ; blendMode < BlendMode_size ; blendMode++ )
		{
			for( int format = 0 ; format < 5 ; format++ )
				[m_warpPipelines[shader][blendMode][format] release];
		}
	}

	for( auto& entry : m_mipmapPipelines )
		[entry.second release];

	[m_library release];
}

//____ _destFormatName() _____________________________________________________

const char * MetalBackend::_destFormatName( DestFormat format )
{
	switch( format )
	{
		case DestFormat::BGRX8_linear:
			return "XRGB_8 linear";
		case DestFormat::BGRA8_linear:
			return "ARGB_8 linear";
		case DestFormat::BGRX8_sRGB:
			return "XRGB_8 sRGB";
		case DestFormat::BGRA8_sRGB:
			return "ARGB_8 sRGB";
		case DestFormat::Alpha_8:
			return "Alpha_8";
	}
	return "Undefined";
}

//____ _toDestFormat() ________________________________________________________

bool MetalBackend::_toDestFormat( PixelFormat format, ColorSpace colorSpace, DestFormat& destFormat )
{
	bool bLinear = (colorSpace == ColorSpace::Linear);

	switch( format )
	{
		case PixelFormat::XRGB_8:
			destFormat = bLinear ? DestFormat::BGRX8_linear : DestFormat::BGRX8_sRGB;
			return true;
		case PixelFormat::ARGB_8:
			destFormat = bLinear ? DestFormat::BGRA8_linear : DestFormat::BGRA8_sRGB;
			return true;
		case PixelFormat::Alpha_8:
			destFormat = DestFormat::Alpha_8;
			return true;
		default:
			return false;
	}
}


//____ _compileLinePipeline() _________________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileLinePipeline( BlendMode blendMode, DestFormat canvasFormat )
{
	bool        isAlpha8    = (canvasFormat == DestFormat::Alpha_8);

	NSString* vertexShader   = @"lineVertexShader";
	NSString* fragmentShader = isAlpha8 ? @"lineFragmentShader_A8" : @"lineFragmentShader";

	NSString* label = [NSString stringWithFormat:@"Line Pipeline (format=%s, blendMode=%s)",
						_destFormatName(canvasFormat), toString(blendMode)];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, true );
}


//____ _compileFillPipeline() _________________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileFillPipeline( bool bTintmap, BlendMode blendMode, DestFormat canvasFormat )
{
	bool        isAlpha8    = (canvasFormat == DestFormat::Alpha_8);

	NSString* vertexShader   = bTintmap ? @"fillTintmapVertexShader" : @"fillVertexShader";
	NSString* fragmentShader = bTintmap ? @"fillTintmapFragmentShader" : @"fillFragmentShader";
	if( isAlpha8 )
		fragmentShader = [fragmentShader stringByAppendingString:@"_A8"];

	NSString* label = [NSString stringWithFormat:@"%@Fill Pipeline (format=%s, blendMode=%s, tintmap=%s)",
						bTintmap ? @"Tintmap" : @"", _destFormatName(canvasFormat), toString(blendMode), bTintmap ? "true" : "false"];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, false );
}

//____ _compileFillAAPipeline() _______________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileFillAAPipeline( bool bTintmap, BlendMode blendMode, DestFormat canvasFormat )
{
	bool        isAlpha8    = (canvasFormat == DestFormat::Alpha_8);

	NSString* vertexShader   = bTintmap ? @"fillAATintmapVertexShader" : @"fillAAVertexShader";
	NSString* fragmentShader = bTintmap ? @"fillAATintmapFragmentShader" : @"fillAAFragmentShader";
	if( isAlpha8 )
		fragmentShader = [fragmentShader stringByAppendingString:@"_A8"];

	NSString* label = [NSString stringWithFormat:@"AA%@Fill Pipeline (format=%s, blendMode=%s, tintmap=%s)",
						bTintmap ? @"Tintmap" : @"", _destFormatName(canvasFormat), toString(blendMode), bTintmap ? "true" : "false"];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, true );
}


//____ _compileBlurPipeline() _________________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileBlurPipeline( BlurFragShader shader, bool bTintmap, BlendMode blendMode, DestFormat canvasFormat )
{
	NSString* namePrefix         = nil;
	NSString* fragmentShaderBase = nil;

	switch( shader )
	{
		case BlurFragShader::Normal:
			namePrefix         = @"Blur";
			fragmentShaderBase = bTintmap ? @"blurTintmapFragmentShader" : @"blurFragmentShader";
			break;

		case BlurFragShader::Palette:
			namePrefix         = @"PaletteBlur";
			fragmentShaderBase = bTintmap ? @"paletteBlurTintmapFragmentShader" : @"paletteBlurFragmentShader";
			break;

		case BlurFragShader::A8Source:
			namePrefix         = @"A8SourceBlur";
			fragmentShaderBase = bTintmap ? @"alphaBlurTintmapFragmentShader" : @"alphaBlurFragmentShader";
			break;

		case BlurFragShader::RGBXSource:
			namePrefix         = @"RGBXSourceBlur";
			fragmentShaderBase = bTintmap ? @"rgbxBlurTintmapFragmentShader" : @"rgbxBlurFragmentShader";
			break;
	}

	NSString* vertexShader		= bTintmap ? @"blitTintmapVertexShader" : @"blitVertexShader";

	bool        isAlpha8        = (canvasFormat == DestFormat::Alpha_8);
	NSString*   fragmentShader  = isAlpha8 ? [fragmentShaderBase stringByAppendingString:@"_A8"] : fragmentShaderBase;

	NSString* label 			= [namePrefix stringByAppendingFormat:@" Pipeline (format=%s, blendMode=%s, tintmap=%s)", _destFormatName(canvasFormat), toString(blendMode), bTintmap ? "true" : "false"];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, false );
}

//____ _compileBlitPipeline() _________________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileBlitPipeline( BlitFragShader shader, bool bTintmap, BlendMode blendMode, DestFormat canvasFormat )
{
	NSString* namePrefix         = nil;
	NSString* vertexShader       = nil;
	NSString* fragmentShaderBase = nil;

	switch( shader )
	{
		case BlitFragShader::Normal:
			namePrefix         = @"Blit";
			vertexShader       = bTintmap ? @"blitTintmapVertexShader" : @"blitVertexShader";
			fragmentShaderBase = bTintmap ? @"blitTintmapFragmentShader" : @"blitFragmentShader";
			break;

		case BlitFragShader::PaletteNearest:
			namePrefix         = @"PaletteBlitNearest";
			vertexShader       = bTintmap ? @"blitTintmapVertexShader" : @"blitVertexShader";
			fragmentShaderBase = bTintmap ? @"paletteBlitNearestTintmapFragmentShader" : @"paletteBlitNearestFragmentShader";
			break;

		case BlitFragShader::PaletteInterpolated:
			namePrefix         = @"PaletteBlitInterpolated";
			vertexShader       = bTintmap ? @"paletteBlitInterpolateTintmapVertexShader" : @"paletteBlitInterpolateVertexShader";
			fragmentShaderBase = bTintmap ? @"paletteBlitInterpolateTintmapFragmentShader" : @"paletteBlitInterpolateFragmentShader";
			break;

		case BlitFragShader::A8Source:
			namePrefix         = @"A8SourceBlit";
			vertexShader       = bTintmap ? @"blitTintmapVertexShader" : @"blitVertexShader";
			fragmentShaderBase = bTintmap ? @"alphaBlitTintmapFragmentShader" : @"alphaBlitFragmentShader";
			break;

		case BlitFragShader::RGBXSource:
			namePrefix         = @"RGBXSourceBlit";
			vertexShader       = bTintmap ? @"blitTintmapVertexShader" : @"blitVertexShader";
			fragmentShaderBase = bTintmap ? @"rgbxBlitTintmapFragmentShader" : @"rgbxBlitFragmentShader";
			break;
	}

	bool        isAlpha8     = (canvasFormat == DestFormat::Alpha_8);
	NSString*   fragmentShader = isAlpha8 ? [fragmentShaderBase stringByAppendingString:@"_A8"] : fragmentShaderBase;

	NSString* label = [namePrefix stringByAppendingFormat:@" Pipeline (format=%s, blendMode=%s, tintmap=%s)",
						_destFormatName(canvasFormat), toString(blendMode), bTintmap ? "true" : "false"];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, false );
}

//____ _compileSegmentsPipeline() _____________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileSegmentsPipeline( int shaderIndex, bool bTintmap, BlendMode blendMode, DestFormat canvasFormat )
{
	bool        isAlpha8    = (canvasFormat == DestFormat::Alpha_8);

	NSString* vertexShader = @"segmentsVertexShader";

	NSString* fragmentShader = [NSString stringWithFormat:@"segments%@FragmentShader%d%@",
								 bTintmap ? @"Tintmap" : @"", shaderIndex, isAlpha8 ? @"_A8" : @""];

	NSString* label = [NSString stringWithFormat:@"Segments Pipeline (shader=%d, format=%s, blendMode=%s, tintmap=%s)",
						shaderIndex, _destFormatName(canvasFormat), toString(blendMode), bTintmap ? "true" : "false"];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, false );
}

//____ _compileShapePipeline() ________________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileShapePipeline( bool bTintmap, BlendMode blendMode, DestFormat canvasFormat )
{
	bool        isAlpha8    = (canvasFormat == DestFormat::Alpha_8);

	NSString* vertexShader = @"shapeVertexShader";

	NSString* fragmentShader = [NSString stringWithFormat:@"shape%@FragmentShader%@",
								 bTintmap ? @"Tintmap" : @"", isAlpha8 ? @"_A8" : @""];

	NSString* label = [NSString stringWithFormat:@"Shape Pipeline (format=%s, blendMode=%s, tintmap=%s)",
						_destFormatName(canvasFormat), toString(blendMode), bTintmap ? "true" : "false"];

	return _compileRenderPipeline( label, vertexShader, fragmentShader, blendMode, canvasFormat, true );
}

//____ _compileWarpPipeline() _________________________________________________

id<MTLRenderPipelineState> MetalBackend::_compileWarpPipeline( WarpFragShader shader, BlendMode blendMode, DestFormat canvasFormat )
{
	bool        isAlpha8    = (canvasFormat == DestFormat::Alpha_8);

	NSString* fragmentShaderBase = nil;

	switch( shader )
	{
		case WarpFragShader::Edgemap:
			fragmentShaderBase = @"warpEdgemapFragmentShader";
			break;

		case WarpFragShader::Blit:
			fragmentShaderBase = @"warpBlitFragmentShader";
			break;

		case WarpFragShader::PaletteBlit:
			fragmentShaderBase = @"warpPaletteBlitFragmentShader";
			break;
	}

	NSString* fragmentShader = isAlpha8 ? [fragmentShaderBase stringByAppendingString:@"_A8"] : fragmentShaderBase;

	NSString* label = [NSString stringWithFormat:@"Warp Pipeline (shader=%@, format=%s, blendMode=%s)",
						fragmentShaderBase, _destFormatName(canvasFormat), toString(blendMode)];

	return _compileRenderPipeline( label, @"warpVertexShader", fragmentShader, blendMode, canvasFormat, true );
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

void MetalBackend::waitForCompletion()
{
	if( m_flushesInProgress == 0 )
		return;

	@autoreleasepool {
		id<MTLCommandBuffer> commandBuffer = [s_metalCommandQueue commandBuffer];
		[commandBuffer commit];
		[commandBuffer waitUntilCompleted];
	}

	return;
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

	DestFormat destFormat;
	if( !_toDestFormat(pixelFormat, colorSpace, destFormat) )
	{
		Core::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "pixelFormat must be ARGB_8, XRGB_8 or Alpha_8", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}

	if( m_defaultCanvasRenderPassDesc )
		[m_defaultCanvasRenderPassDesc release];
	m_defaultCanvasRenderPassDesc = renderPassDesc;
	m_defaultCanvasDestFormat = destFormat;
	[m_defaultCanvasRenderPassDesc retain];
	m_defaultCanvas.size = pixelSize*64;
	m_defaultCanvas.scale = scale;
	m_defaultCanvas.format = pixelFormat;
	m_defaultCanvas.colorSpace = colorSpace;

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

	// The command buffer and encoders we get from Metal are autoreleased and the
	// command buffer keeps our buffers alive. If no autorelease pool is drained
	// between sessions, as when rendering from a loop of our own, they would pile
	// up until Metal runs out of memory, so we give each session a pool of its own.

	m_pSessionPool = [[NSAutoreleasePool alloc] init];

	// Reserve buffer for coordinates

	int nVertices = pInfo->nRects * 6 + pInfo->nLineCoords/2 * 6;

	m_vertexBufferId = [s_metalDevice newBufferWithLength:nVertices*sizeof(VertexMTL) options:MTLResourceStorageModeShared];
	m_pVertexBuffer = (VertexMTL *)[m_vertexBufferId contents];
	m_nVertices = 0;

	// Reserve buffer for colors

	// Tint blocks need a header, geometry and stop positions on top of their
	// colors, which are counted in nColors. Any state change may carry a tint.

	const int maxTintStops = Tint::c_maxMixComponents * Tint::c_maxStops;
	const int tintBlockOverhead = TintTools::gpuTintBlockMaxSize(Tint::c_maxMixComponents, maxTintStops) - maxTintStops;

	int nColors = pInfo->nColors + 1 + (pInfo->nStateChanges + pInfo->nShapeTints) * tintBlockOverhead;

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
	+ pInfo->nLineCoords/2 * GpuShapes::c_lineEntries * 4		// Lines with caps take the most.
	+ pInfo->nRects * 4			// This is for possible subpixel fills. We have no way of knowing exactly how much is needed.
	+ pInfo->nEdgemapDraws * 8
	+ pInfo->nWarpDraws * 16
	+ GpuShapes::maxSessionEntries(*pInfo) * 4;

	m_extrasBufferId = [s_metalDevice newBufferWithLength:nExtrasFloats*sizeof(float) options:MTLResourceStorageModeShared];
	m_pExtrasBuffer = (float *)[m_extrasBufferId contents];
	m_extrasBufferSize = 0;

	// Setup a metal command buffer for this session

	m_metalCommandBuffer = [s_metalCommandQueue commandBuffer];
	m_metalCommandBuffer.label = @"MetalBackend";

	// Set canvas, which also resets states to start values.

	if( pCanvasSurface )
		setCanvas(pCanvasSurface);
	else
		setCanvas( canvasRef );
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
	{
		m_pActiveCanvas->m_bBufferNeedsSync = true;
		m_pActiveCanvas->m_bMipmapStale = m_pActiveCanvas->m_bMipmapped;
	}

	m_pActiveCanvas = nullptr;

	[m_pSessionPool drain];
	m_pSessionPool = nil;
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

	_resetStates();
	m_renderEncoder = _setCanvas(pSurface, pSurface->pixelWidth(), pSurface->pixelHeight() );
}

void MetalBackend::setCanvas(CanvasRef ref)
{
	if( ref != CanvasRef::Default )
		return;								// ERROR: Only CanvasRef::Default supported yet.

	if( m_renderEncoder != nil )
		[m_renderEncoder endEncoding];

	_resetStates();
	m_renderEncoder = _setCanvas(nullptr, m_defaultCanvas.size.w/64, m_defaultCanvas.size.h/64 );
}

//____ _resetStates() _________________________________________________________
//
// States start over with each canvas, as the device sets them again for it, like
// SoftBackend does. Called before _setCanvas(), which hands them to the new render
// encoder. That has no blit source until one is set.

void MetalBackend::_resetStates()
{
	m_tintColorOfs			= -1;
	m_tintOfs				= -1;

	m_pActiveBlitSource		= nullptr;
	m_activeBlendMode		= BlendMode::Blend;
	m_activeMorphFactor		= 0.5f;
	m_activeBlurRadius		= 64;

	m_uniform.flatTint = { 1.f, 1.f, 1.f, 1.f };
}

//____ setObjects() ________________________________________________________

void MetalBackend::setObjects(Object* const * pBeg, Object* const * pEnd)
{
	m_pObjectsBeg = pBeg;
	m_pObjectsEnd = pEnd;
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

void MetalBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version)
{
	const RectSPX* 	pRects = m_pRectsPtr;
	const HiColor* 	pColors = m_pColorsPtr;

	VertexMTL *	pVertexMTL	= m_pVertexBuffer + m_nVertices;
	ColorMTL*	pColorMTL	= m_pColorBuffer + m_nColors;
	float*		pExtrasMTL	= m_pExtrasBuffer + m_extrasBufferSize;

	int 		customTransformStart = version == 1 ? GfxFlip_size : NbStandardTransforms;


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
					auto pSurf = static_cast<MetalSurface*>(m_pObjectsBeg[readObjectIndex(p)]);

					if( pSurf )
					{
						if( pSurf->m_bMipmapStale )
						{
							[m_renderEncoder endEncoding];

							_generateMipmaps(pSurf);

							m_renderEncoder = _setCanvas( m_pActiveCanvas, m_activeCanvasSize.w, m_activeCanvasSize.h );

							pSurf->m_bMipmapStale = false;
						}

						[m_renderEncoder setFragmentTexture:pSurf->getTexture() atIndex: (unsigned) TextureIndex::Texture];

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

					color.toLinearFloat((float*) &m_uniform.flatTint);		// Colors are sRGB, but we blend in linear.

					bUniformChanged = true;
				}

				if (statesChanged & uint8_t(StateChange::Tint))
				{
					// Tint block is written to color buffer, its offset stored in vertex data.
					// Tint color (flatTint in uniform) is applied on top of it.

					TintTools::DecodedTint tint;
					p = TintTools::decodeTint(p, pColors, tint);

					if (tint.nLayers == 0)
						m_tintOfs = -1;
					else
					{
						m_tintOfs = int(pColorMTL - m_pColorBuffer);
						TintTools::writeGpuTintBlock(tint, (float*) pColorMTL);
						pColorMTL += TintTools::gpuTintBlockSize(tint);
					}
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					m_activeBlendMode = (BlendMode) * p++;
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					m_activeMorphFactor = (* p++) / 4096.f;

					[m_renderEncoder setFragmentBytes:&m_activeMorphFactor length:sizeof(float) atIndex:(unsigned) FragmentInputIndex::MorphFactor];
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					// Just retrieve the value but do nothing. We use BlendMode::Blend
					// in place of BlendMode::BlendFixedColor.

					pColors++;
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					// Only the radius is stored here. The offsets are in texture coordinates of the
					// blit source, so they are calculated at draw time from whatever source is active then.

					m_activeBlurRadius = * p++;

					for( int i = 0 ; i < 9 ; i++ )
					{
						m_blurUniform.colorMtx[i][0] = p[i]/32768.f;
						m_blurUniform.colorMtx[i][1] = p[9+i]/32768.f;
						m_blurUniform.colorMtx[i][2] = p[18+i]/32768.f;
						m_blurUniform.colorMtx[i][3] = 0;
					}

					m_blurUniform.colorMtx[4][3] = 1.f;

					p += 27;
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
								if( m_fillAAPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
									m_fillAAPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileFillAAPipeline(m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

								[m_renderEncoder setRenderPipelineState:m_fillAAPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
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
								if( m_fillPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
									m_fillPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileFillPipeline(m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

								[m_renderEncoder setRenderPipelineState:m_fillPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
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



					int colOfs = int(pColorMTL - m_pColorBuffer);

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = colOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pRects++;
					nRectsWritten++;
				}

				// Add colors to buffer

				col.toLinearFloat(&pColorMTL->r);		// Colors are sRGB, but we blend in linear.
				pColorMTL++;

				// Draw

				if( bStraightFill )
				{
					if( m_fillPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
						m_fillPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileFillPipeline(m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

					[m_renderEncoder setRenderPipelineState:m_fillPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
				}
				else
				{
					if( m_fillAAPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
						m_fillAAPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileFillAAPipeline(m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

					[m_renderEncoder setRenderPipelineState:m_fillAAPipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
				}

				[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRectsWritten*6];
				break;
			}

			case Command::Line:
			{
				int32_t nClipRects = *p++;
				int32_t nLines = *p++;
				p++;							// padding

				// Calculate and store vertices

				int nLinesWritten = 0;
				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				for (int i = 0; i < nLines; i++)
				{
					HiColor col = *pColors++;

					CoordSPX begin, end;

					auto p32 = (const spx *) p;
					begin.x = *p32++;
					begin.y = *p32++;
					end.x = *p32++;
					end.y = *p32++;
					p = (const uint16_t*) p32;

					spx thicknessSPX = *p++;
					uint16_t capsWord = *p++;

					if (capsWord != 0)
					{
						m_cappedLines.push_back({ begin, end, thicknessSPX, capsWord, col });
						continue;
					}

					float thickness = thicknessSPX / 64.f;

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

						// The quad holds every pixel whose center is closer than w to the center of the line,
						// rounded outwards. The line passes through the centers of the pixels at begin and end.

						float   y1 = begin.y + 0.5f - slope / 2;				// Center of the line at the left side of the quad.
						float   y2 = y1 + slope * length;					// ...and at the right side.

						c1.x = begin.x;
						c1.y = int(std::floor(y1 - w));
						c2.x = end.x;
						c2.y = int(std::floor(y2 - w));
						c3.x = end.x;
						c3.y = int(std::ceil(y2 + w));
						c4.x = begin.x;
						c4.y = int(std::ceil(y1 + w));
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

						// The quad holds every pixel whose center is closer than w to the center of the line,
						// rounded outwards. The line passes through the centers of the pixels at begin and end.

						float   x1 = begin.x + 0.5f - slope / 2;				// Center of the line at the top of the quad.
						float   x2 = x1 + slope * length;					// ...and at the bottom.

						c1.x = int(std::floor(x1 - w));
						c1.y = begin.y;
						c2.x = int(std::ceil(x1 + w));
						c2.y = begin.y;
						c3.x = int(std::ceil(x2 + w));
						c3.y = end.y;
						c4.x = int(std::floor(x2 - w));
						c4.y = end.y;
					}

					int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;
					int colorsOfs = int(pColorMTL - m_pColorBuffer);

					pVertexMTL->coord = c1;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord = c2;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord = c3;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord = c1;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord = c3;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord = c4;
					pVertexMTL->colorsOfs = colorsOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					*pExtrasMTL++ = s;
					*pExtrasMTL++ = w;
					*pExtrasMTL++ = slope;
					*pExtrasMTL++ = bSteep;

					col.toLinearFloat(&pColorMTL->r);		// Colors are sRGB, but we blend in linear.
					pColorMTL++;

					nLinesWritten++;
				}

				// Store command, with clip rects

				if (nLinesWritten > 0)
				{
					if( m_linePipelines[(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
						m_linePipelines[(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileLinePipeline(m_activeBlendMode, m_activeCanvasFormat);

					[m_renderEncoder setRenderPipelineState:m_linePipelines[(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];

					for (int i = 0; i < nClipRects; i++)
					{
						RectI clip = pRects[i]/64;
						MTLScissorRect metalClip = {(unsigned) clip.x, (unsigned) clip.y, (unsigned) clip.w, (unsigned) clip.h};
						[m_renderEncoder setScissorRect:metalClip];
						[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nLinesWritten*6];
					}

					MTLScissorRect orgClip = {0, 0, (unsigned) m_activeCanvasSize.w, (unsigned) m_activeCanvasSize.h};
					[m_renderEncoder setScissorRect:orgClip];
				}

				// Lines with caps, as shapes in a quad around each, see GpuShapes::writeLine().
				// The shader applies tint color and tint.

				int nCappedWritten = 0;
				int cappedVertexOfs = int(pVertexMTL - m_pVertexBuffer);
				const simd_float2 noUV = { 0.f, 0.f };

				for (const CappedLine& line : m_cappedLines)
				{
					CoordF quad[4];
					int nEntries = GpuShapes::writeLine(line.begin, line.end, line.thickness, decodeLineCaps(line.caps), pExtrasMTL, quad);
					if (nEntries == 0)
						continue;

					pExtrasMTL[3] = float(pColorMTL - m_pColorBuffer);
					line.color.toLinearFloat(&pColorMTL->r);
					pColorMTL++;

					int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;
					pExtrasMTL += nEntries * 4;

					for (int corner : { 0, 1, 2, 0, 2, 3 })
					{
						pVertexMTL->coord = { int(std::lround(quad[corner].x)), int(std::lround(quad[corner].y)) };
						pVertexMTL->colorsOfs = 0;
						pVertexMTL->extrasOfs = extrasOfs;
						pVertexMTL->uv = noUV;
						pVertexMTL->tintOfs = m_tintOfs;
						pVertexMTL++;
					}

					nCappedWritten++;
				}

				m_cappedLines.clear();

				if (nCappedWritten > 0)
				{
					auto& pipeline = m_shapePipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat];
					if( pipeline == nil )
						pipeline = _compileShapePipeline(m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

					[m_renderEncoder setRenderPipelineState:pipeline];

					for (int i = 0; i < nClipRects; i++)
					{
						RectI clip = pRects[i] / 64;
						MTLScissorRect metalClip = {(unsigned) clip.x, (unsigned) clip.y, (unsigned) clip.w, (unsigned) clip.h};
						[m_renderEncoder setScissorRect:metalClip];
						[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:cappedVertexOfs vertexCount:nCappedWritten*6];
					}

					MTLScissorRect orgClip = {0, 0, (unsigned) m_activeCanvasSize.w, (unsigned) m_activeCanvasSize.h};
					[m_renderEncoder setScissorRect:orgClip];
				}

				pRects += nClipRects;
				break;
			}

			case Command::DrawShape:
			{
				auto kind = ShapeKind(p[0]);
				int nRects = p[1];
				int nColors = p[2];
				int nWords = p[3];
				const uint16_t* pPayload = p + c_drawShapeHeaderWords - 1;
				p = pPayload + nWords;

				const RectSPX* pShapeRects = pRects;
				const HiColor* pShapeColors = pColors;
				pRects += nRects;
				pColors += nColors;

				int maxEntries, nSegments;
				GpuShapes::measure(kind, pPayload, maxEntries, nSegments);

				if (maxEntries == 0)
				{
					Core::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, "Failed DrawShape operation. Unsupported kind of shape.",
						this, &TYPEINFO, __func__, __FILE__, __LINE__);
					break;
				}

				// Shape data in extras buffer, read by the shader.

				if ((int)m_shapeSegments.size() < nSegments)
					m_shapeSegments.resize(nSegments);

				int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;
				int nEntries = GpuShapes::write(kind, pPayload, pShapeColors, pExtrasMTL, m_shapeSegments.data());

				// Colors and tint blocks of drawn segments in color buffer, their offsets in the
				// entries of the segments. Tint color and tint are applied by the shader.

				for (int i = 0; i < nSegments; i++)
				{
					const GpuShapes::Segment& seg = m_shapeSegments[i];

					if (!seg.bDrawn)
						continue;

					float* pEntry = pExtrasMTL + seg.entry * 4;

					pEntry[0] = float(pColorMTL - m_pColorBuffer);
					seg.color.toLinearFloat(&pColorMTL->r);
					pColorMTL++;

					if (seg.pTint)
					{
						TintTools::DecodedTint tint;
						const HiColor* pTintColors = seg.pTintColors;
						TintTools::decodeTint(seg.pTint, pTintColors, tint);

						pEntry[1] = float(pColorMTL - m_pColorBuffer);
						TintTools::writeGpuTintBlock(tint, (float*) pColorMTL);
						pColorMTL += TintTools::gpuTintBlockSize(tint);
					}
				}

				pExtrasMTL += nEntries * 4;

				// Vertices, a quad for each clip rectangle.

				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);
				const simd_float2 noUV = { 0.f, 0.f };

				for (int i = 0; i < nRects; i++)
				{
					RectI patch = pShapeRects[i] / 64;

					CoordI corners[6] = {	{ patch.x, patch.y }, { patch.x + patch.w, patch.y }, { patch.x + patch.w, patch.y + patch.h },
											{ patch.x, patch.y }, { patch.x + patch.w, patch.y + patch.h }, { patch.x, patch.y + patch.h } };

					for (auto& corner : corners)
					{
						pVertexMTL->coord = corner;
						pVertexMTL->colorsOfs = 0;
						pVertexMTL->extrasOfs = extrasOfs;
						pVertexMTL->uv = noUV;
						pVertexMTL->tintOfs = m_tintOfs;
						pVertexMTL++;
					}
				}

				auto& pipeline = m_shapePipelines[m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat];
				if( pipeline == nil )
					pipeline = _compileShapePipeline(m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

				[m_renderEncoder setRenderPipelineState:pipeline];
				[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRects*6];
				break;
			}

			case Command::WarpBlit:
			case Command::WarpDrawEdgemap:
			{
				WarpParams params;
				p = decodeWarp(cmd, p, params);

				auto pWarpmap = static_cast<MetalWarpmap*>(m_pObjectsBeg[params.warpmap]);
				auto pEdgemap = (cmd == Command::WarpDrawEdgemap) ? static_cast<MetalEdgemap*>(m_pObjectsBeg[params.edgemap]) : nullptr;

				const RectSPX* pWarpRects = pRects;
				pRects += params.nRects;

				// Extras, see warpVertexShader() in the shaders.

				CoordI dest = roundToPixels(params.dest);
				SizeI warpSize = pWarpmap->pixelSize();

				float srcW, srcH;

				if( pEdgemap )
				{
					srcW = float(pEdgemap->m_size.w);
					srcH = float(pEdgemap->m_size.h);
				}
				else
				{
					srcW = params.source.w / 64.f;
					srcH = params.source.h / 64.f;
				}

				int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;

				*pExtrasMTL++ = float(dest.x);
				*pExtrasMTL++ = float(dest.y);
				*pExtrasMTL++ = float(pWarpmap->cellSize());
				*pExtrasMTL++ = pEdgemap ? float(pEdgemap->m_nbRenderSegments - 1) : 0.f;

				*pExtrasMTL++ = float(warpSize.w);
				*pExtrasMTL++ = float(warpSize.h);
				*pExtrasMTL++ = srcW;
				*pExtrasMTL++ = srcH;

				*pExtrasMTL++ = params.sourceOffset.x / 64.f;
				*pExtrasMTL++ = params.sourceOffset.y / 64.f;
				*pExtrasMTL++ = pWarpmap->uPeriod() * srcW;
				*pExtrasMTL++ = pWarpmap->vPeriod() * srcH;

				if( pEdgemap )
				{
					*pExtrasMTL++ = float(pEdgemap->m_nbSegments - 1);
					*pExtrasMTL++ = float(pEdgemap->_flatColorsOfs());
					*pExtrasMTL++ = float(pEdgemap->_tintTableOfs());
					*pExtrasMTL++ = float(pEdgemap->_rawSamplesOfs());
				}
				else
				{
					// A surface that isn't tiling is read as if the edge pixels of the source rectangle
					// continued outside it, see warpBlitCoverage() in the shaders.

					MetalSurface* pSurf = m_pActiveBlitSource;
					float clampInset = pSurf->isTiling() ? 0.f : pSurf->sampleMethod() == SampleMethod::Bilinear ? 0.5f : 1.f / 256;

					*pExtrasMTL++ = params.source.x / 64.f;
					*pExtrasMTL++ = params.source.y / 64.f;
					*pExtrasMTL++ = clampInset;
					*pExtrasMTL++ = 0.f;
				}

				// Vertices, a quad for the part of each clip rectangle covered by the Warpmap.

				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);
				int nVertices = 0;
				const simd_float2 noUV = { 0.f, 0.f };

				for( int i = 0 ; i < params.nRects ; i++ )
				{
					RectI patch = RectI::overlap(roundToPixels(pWarpRects[i]), RectI(dest, warpSize));

					if( patch.w <= 0 || patch.h <= 0 )
						continue;

					CoordI corners[6] = {	{ patch.x, patch.y }, { patch.x + patch.w, patch.y }, { patch.x + patch.w, patch.y + patch.h },
											{ patch.x, patch.y }, { patch.x + patch.w, patch.y + patch.h }, { patch.x, patch.y + patch.h } };

					for( auto& corner : corners )
					{
						pVertexMTL->coord = corner;
						pVertexMTL->colorsOfs = 0;
						pVertexMTL->extrasOfs = extrasOfs;
						pVertexMTL->uv = noUV;
						pVertexMTL->tintOfs = m_tintOfs;
						pVertexMTL++;
					}

					nVertices += 6;
				}

				if( nVertices == 0 )
					break;

				// Buffers, sampler and pipeline.

				[m_renderEncoder setFragmentBuffer:pWarpmap->m_bufferId offset:0 atIndex:(unsigned) FragmentInputIndex::Warpmap];

				WarpFragShader shader = WarpFragShader::Edgemap;

				if( pEdgemap )
				{
					[m_renderEncoder setFragmentBuffer:pEdgemap->m_bufferId offset:0 atIndex:(unsigned) FragmentInputIndex::Edgemap];
				}
				else
				{
					// The source is read at the position of each pixel center, so it only needs to
					// be wrapped or clamped there, and the shaders keep it within the source rectangle
					// if not tiling. Coverage takes care of its borders.

					MetalSurface* pSurf = m_pActiveBlitSource;

					_setSampler(pSurf, pSurf->isTiling() ? SamplerEdge::Tile : SamplerEdge::Clamp);

					int sourceKind = 0;
					shader = WarpFragShader::Blit;

					if( pSurf->m_pPixelDescription->type == PixelType::Index )
					{
						shader = WarpFragShader::PaletteBlit;
						sourceKind = pSurf->sampleMethod() == SampleMethod::Bilinear ? 1 : 0;
					}
					else if( pSurf->m_pixelFormat == PixelFormat::Alpha_8 )
						sourceKind = 1;
					else if( pSurf->m_pPixelDescription->A_mask == 0 )
						sourceKind = 2;

					[m_renderEncoder setFragmentBytes:&sourceKind length:sizeof(int) atIndex: (unsigned) FragmentInputIndex::SourceKind];
				}

				auto& pipeline = m_warpPipelines[(int)shader][(int)m_activeBlendMode][(int)m_activeCanvasFormat];
				if( pipeline == nil )
					pipeline = _compileWarpPipeline(shader, m_activeBlendMode, m_activeCanvasFormat);

				[m_renderEncoder setRenderPipelineState:pipeline];
				[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nVertices];
				break;
			}

			case Command::DrawEdgemap:
			case Command::TileEdgemap:
			{
				bool bTile = (cmd == Command::TileEdgemap);		// Origo is for one of the copies, rects give area to fill.

				int32_t nRects = *p++;
				int32_t	flip = *p++;

				p++;					// padding

				auto pEdgemap = static_cast<MetalEdgemap*>(m_pObjectsBeg[readObjectIndex(p)]);

				auto p32 = (const spx *) p;
				int32_t	destX = *p32++;
				int32_t	destY = *p32++;
				p = (const uint16_t*) p32;

				//

				auto& mtx = s_standardTransforms[flip];

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

				int vertexOfs = int(pVertexMTL - m_pVertexBuffer);

				// Setup vertices

				for (int i = 0; i < nRects; i++)
				{
					RectI patch = roundToPixels(*pRects++);		// Rounded like the position of the edgemap.

					int		dx1 = patch.x;
					int		dx2 = patch.x + patch.w;
					int		dy1 = patch.y;
					int		dy2 = patch.y + patch.h;

					//


					//

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

					//

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv1;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv2;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv3;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv1;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv3;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->uv = uv4;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;
				}

				// Setup extras data

				*pExtrasMTL++ = float(pEdgemap->_flatColorsOfs());		// Flat segment colors in edgemap buffer.
				*pExtrasMTL++ = float(pEdgemap->_tintTableOfs());		// Table of segment tint block offsets in edgemap buffer.
				*pExtrasMTL++ = float(pEdgemap->m_nbSegments - 1);	// Edgemap pitch (edges stored per column)
				*pExtrasMTL++ = 0;			// Dummy

				// Size of the edgemap when tiled, in columns and pixels, which the shader wraps
				// positions outside of it by. Zero when not.

				*pExtrasMTL++ = bTile ? float(pEdgemap->m_size.w) : 0.f;
				*pExtrasMTL++ = bTile ? float(pEdgemap->m_size.h) : 0.f;
				*pExtrasMTL++ = 0;
				*pExtrasMTL++ = 0;



				//

				[m_renderEncoder setFragmentBuffer:pEdgemap->m_bufferId offset:0 atIndex:(unsigned) FragmentInputIndex::Edgemap];

				if( m_segmentsPipelines[nSegments-1][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
					m_segmentsPipelines[nSegments-1][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileSegmentsPipeline(nSegments-1, m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat);

				[m_renderEncoder setRenderPipelineState:m_segmentsPipelines[nSegments-1][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
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

				for (int i = 0; i < nRects; i++)
				{
					int	dx1 = (pRects->x) >> 6;
					int	dy1 = (pRects->y) >> 6;
					int dx2 = dx1 + ((pRects->w) >> 6);
					int dy2 = dy1 + ((pRects->h) >> 6);
					pRects++;


					int extrasOfs = int(pExtrasMTL - m_pExtrasBuffer) / 4;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy1;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx2;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					pVertexMTL->coord.x = dx1;
					pVertexMTL->coord.y = dy2;
					pVertexMTL->colorsOfs = tintColorOfs;
					pVertexMTL->extrasOfs = extrasOfs;
					pVertexMTL->tintOfs = m_tintOfs;
					pVertexMTL++;

					//

					auto p32 = (const spx *) p;
					int srcX = *p32++;
					int srcY = *p32++;
					spx dstX = *p32++;
					spx dstY = *p32++;
					p = (const uint16_t*) p32;


					int32_t transform = *p++;
					p++;						// padding

					auto& mtx = transform < customTransformStart ? s_standardTransforms[transform] : m_pTransformsBeg[transform - customTransformStart];

					// The source coordinate is where the corner of the first destination pixel lands.
					// Standard transforms (blits, tiles and blurs, possibly flipped) map pixels 1:1,
					// so they sample the center of each source pixel, half a pixel along the
					// transformed axes, whichever way those point. Other transforms sample bilinear
					// sources at the center of the texel, but nearest ones at the corner, so that
					// stretched pixels are evenly distributed. A corner on a texel edge belongs to
					// the texel after it, but rounding could tip it into the one before. Corners land
					// exactly on texel edges when an axis aligned stretch has a simple ratio, so those
					// are nudged forward by a fraction of a texel. Rotated ones practically never do,
					// and a nudge would only move samples that weren't on an edge. Blurs always sample
					// the center, so their fractional sample offsets round to the nearest pixel either way.

					float srcOfsX, srcOfsY;

					if (transform < customTransformStart)
					{
						srcOfsX = (mtx.xx + mtx.yx) * 0.5f;
						srcOfsY = (mtx.xy + mtx.yy) * 0.5f;
					}
					else if (m_pActiveBlitSource->sampleMethod() == SampleMethod::Bilinear || cmd == Command::Blur)
						srcOfsX = srcOfsY = 0.5f;
					else if ((mtx.xy == 0.f && mtx.yx == 0.f) || (mtx.xx == 0.f && mtx.yy == 0.f))
						srcOfsX = srcOfsY = 1.f / 4096;
					else
						srcOfsX = srcOfsY = 0.f;

					*pExtrasMTL++ = srcX / 1024.f + srcOfsX;
					*pExtrasMTL++ = srcY / 1024.f + srcOfsY;
					*pExtrasMTL++ = float(dstX >> 6) + 0.5f;
					*pExtrasMTL++ = float(dstY >> 6) + 0.5f;

					*pExtrasMTL++ = mtx.xx;
					*pExtrasMTL++ = mtx.xy;
					*pExtrasMTL++ = mtx.yx;
					*pExtrasMTL++ = mtx.yy;
				}



				// Draw

				{
					MetalSurface* pSurf = m_pActiveBlitSource;

					_setSampler(pSurf, pSurf->isTiling() ? SamplerEdge::Tile : cmd == Command::ClipBlit ? SamplerEdge::Clip : SamplerEdge::Clamp);

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
					{
						// Blur taps are whole pixels, rounded the way the software backend does it, so
						// they land on texel centers and read the same texels whatever the sample method.
						// Offsets are added to texture coordinates that are normalized against the blit
						// source, so they are normalized against its size as well.

						SizeI size = pSurf->pixelSize();

						int corner = m_activeBlurRadius * 7 / 10;

						float posX = float((m_activeBlurRadius + 32) >> 6) / size.w, negX = float((-m_activeBlurRadius + 32) >> 6) / size.w;
						float posY = float((m_activeBlurRadius + 32) >> 6) / size.h, negY = float((-m_activeBlurRadius + 32) >> 6) / size.h;
						float cPosX = float((corner + 32) >> 6) / size.w, cNegX = float((-corner + 32) >> 6) / size.w;
						float cPosY = float((corner + 32) >> 6) / size.h, cNegY = float((-corner + 32) >> 6) / size.h;

						m_blurUniform.offset[0] = { cNegX, cNegY };
						m_blurUniform.offset[1] = { 0, negY };
						m_blurUniform.offset[2] = { cPosX, cNegY };
						m_blurUniform.offset[3] = { negX, 0 };
						m_blurUniform.offset[4] = { 0, 0 };
						m_blurUniform.offset[5] = { posX, 0 };
						m_blurUniform.offset[6] = { cNegX, cPosY };
						m_blurUniform.offset[7] = { 0, posY };
						m_blurUniform.offset[8] = { cPosX, cPosY };

						[m_renderEncoder setFragmentBytes:&m_blurUniform length:sizeof(BlurUniform) atIndex: (unsigned) FragmentInputIndex::BlurUniform];

						// Same source kinds as for blits, except that a palette based source always
						// is looked up tap by tap.

						BlurFragShader blurShader = BlurFragShader::Normal;

						if( shader == BlitFragShader::PaletteNearest || shader == BlitFragShader::PaletteInterpolated )
							blurShader = BlurFragShader::Palette;
						else if( shader == BlitFragShader::A8Source )
							blurShader = BlurFragShader::A8Source;
						else if( shader == BlitFragShader::RGBXSource )
							blurShader = BlurFragShader::RGBXSource;

						auto& pipeline = m_blurPipelines[(int)blurShader][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat];

						if( pipeline == nil )
							pipeline = _compileBlurPipeline( blurShader, m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat );

						[m_renderEncoder setRenderPipelineState:pipeline ];
					}
					else
					{
						// The sampler returns zero outside a non-tiling source, which clips a
						// ClipBlit, but for a palette based source zero is palette index 0.
						// The palette shaders clip their taps themselves when told to.

						if( shader == BlitFragShader::PaletteNearest || shader == BlitFragShader::PaletteInterpolated )
						{
							int clipToSource = (cmd == Command::ClipBlit && !pSurf->isTiling()) ? 1 : 0;
							[m_renderEncoder setFragmentBytes:&clipToSource length:sizeof(int) atIndex: (unsigned) FragmentInputIndex::ClipToSource];
						}

						// A source without alpha (RGBX) reads as opaque also where the sampler returns
						// zero, so the RGBX shaders work out the alpha themselves when clipping.

						if( shader == BlitFragShader::RGBXSource )
						{
							int clipFlags = (cmd == Command::ClipBlit && !pSurf->isTiling()) ? 1 : 0;
							if( pSurf->sampleMethod() == SampleMethod::Bilinear )
								clipFlags |= 2;
							[m_renderEncoder setFragmentBytes:&clipFlags length:sizeof(int) atIndex: (unsigned) FragmentInputIndex::ClipToSource];
						}

						if(m_blitPipelines[(int)shader][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] == nil )
							m_blitPipelines[(int)shader][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] = _compileBlitPipeline( shader, m_tintOfs >= 0, m_activeBlendMode, m_activeCanvasFormat );

						[m_renderEncoder setRenderPipelineState:m_blitPipelines[(int)shader][m_tintOfs >= 0][(int)m_activeBlendMode][(int)m_activeCanvasFormat] ];
					}

					[m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vertexOfs vertexCount:nRects*6];
					vertexOfs += nRects*6;
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

	m_nVertices			= int(pVertexMTL - m_pVertexBuffer);
	m_nColors			= int(pColorMTL	- m_pColorBuffer);
	m_extrasBufferSize	= int(pExtrasMTL - m_pExtrasBuffer);

	//		_executeBuffer();

}




//____ autopresent() ________________________________________________

void MetalBackend::autopresent( id<MTLDrawable> drawable )
{
	m_drawableToAutoPresent = drawable;
}


//____ _setCanvas() ___________________________________________________________

id<MTLRenderCommandEncoder> MetalBackend::_setCanvas( MetalSurface * pCanvas, int width, int height )
{
	id<MTLRenderCommandEncoder> renderEncoder;

	DestFormat destFormat = DestFormat::BGRA8_sRGB;
	bool bValidFormat = true;

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

		bValidFormat = _toDestFormat(pCanvas->pixelFormat(), pCanvas->colorSpace(), destFormat);
	}
	else
	{

		renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:m_defaultCanvasRenderPassDesc];
		renderEncoder.label = @"MetalBackend Render Pass";

		destFormat = m_defaultCanvasDestFormat;
	}

	[renderEncoder setViewport:(MTLViewport){0.0, 0.0, (double) width, (double) height, 0.0, 1.0 }];
	[renderEncoder setFragmentBytes:&m_activeMorphFactor length:sizeof(float) atIndex:(unsigned) FragmentInputIndex::MorphFactor];


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

	m_activeCanvasFormat = destFormat;

	if( !bValidFormat )
		Core::throwError(ErrorLevel::Error, ErrorCode::Internal, "Canvas format is neither ARGB_8, XRGB_8 or Alpha_8", this, &TYPEINFO, __func__, __FILE__, __LINE__ );


	return renderEncoder;
}


//____ _initTables() ___________________________________________________________

void MetalBackend::_initTables()
{
	// Init lineThicknessTable

	for (int i = 0; i < 17; i++)
	{
		double b = i / 16.0;
		m_lineThicknessTable[i] = (float)GfxUtil::squareRoot(1.0 + b * b);
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

//____ _setSampler() __________________________________________________________
//
// Palette based sources are read through a nearest sampler, so that the taps don't
// blend indexes.

void MetalBackend::_setSampler( MetalSurface * pSurf, SamplerEdge edge )
{
	id<MTLSamplerState> sampler;

	if( pSurf->pixelDescription()->type == PixelType::Index )
		sampler = m_samplers[0][0][(int)edge];
	else
		sampler = m_samplers[pSurf->isMipmapped()][pSurf->sampleMethod() == SampleMethod::Bilinear][(int)edge];

	[m_renderEncoder setFragmentSamplerState:sampler atIndex:0];
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
	id<MTLTexture> texture = pSurface->m_texture;
	int nLevels = (int) texture.mipmapLevelCount;

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

		id<MTLRenderCommandEncoder> renderEncoder = [m_metalCommandBuffer renderCommandEncoderWithDescriptor:pDescriptor];
		renderEncoder.label = @"MetalBackend Mipmap Pass";
		[pDescriptor release];

		[renderEncoder setRenderPipelineState:pipeline];
		[renderEncoder setFragmentTexture:source atIndex:0];
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
	descriptor.vertexFunction = [m_library newFunctionWithName:@"mipmapVertexShader"];
	descriptor.fragmentFunction = [m_library newFunctionWithName:@"mipmapFragmentShader"];
	descriptor.colorAttachments[0].pixelFormat = format;
	descriptor.colorAttachments[0].blendingEnabled = NO;

	id<MTLRenderPipelineState> pipelineState = [s_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];

	if( pipelineState == nil )
	{
		NSString * pMessage = [NSString stringWithFormat:@"Failed to create Mipmap Pipeline: %@", error ? error.localizedDescription : @"unknown error"];
		Core::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, pMessage.UTF8String, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	[descriptor.vertexFunction release];
	[descriptor.fragmentFunction release];
	[descriptor release];

	m_mipmapPipelines[format] = pipelineState;		// Kept also when nil, so we don't try again and again.
	return pipelineState;
}

//____ _compileRenderPipeline() _______________________________________________
/*
	Canvases with alpha hold premultiplied colors, which lets fixed-function blending follow the
	blend rules exactly: Blend is Porter-Duff over, Morph a premultiplied cross-fade and a partly
	covered pixel gets the average of the blend result and the canvas. The fragment shader is
	specialized to pass what it draws to the blender as the mode needs it, see writeColor() in
	the shaders:

	- Replace and Morph keep the uncovered part of the pixel by the second color of dual-source
	  blending. Replace only blends in pipelines that can cover pixels partly (bPartialCoverage).
	- Effect modes work on the premultiplied colors of the canvas, as the blend rules define them,
	  with the source as its light (pc) and partial coverage fading it. Add, Invert and Max raise
	  the alpha of the canvas by the strongest channel of pc, which the fragment shaders output as
	  alpha for them. Subtract, Multiply and Min keep it.
	- On Alpha_8 canvases, effect modes use the alpha of the source as the value.
	- Canvases without alpha are opaque. Their alpha bytes are undefined and left alone.
*/

id<MTLRenderPipelineState> MetalBackend::_compileRenderPipeline( NSString* label, NSString* vertexShader,
																NSString* fragmentShader, BlendMode blendMode, DestFormat destFormat, bool bPartialCoverage )
{
	bool bNoAlpha = ( destFormat == DestFormat::BGRX8_linear || destFormat == DestFormat::BGRX8_sRGB );
	bool bAlphaOnly = (destFormat == DestFormat::Alpha_8);

	// Function constants of the fragment shader, see writeColor() and writeAlpha() in the shaders.

	int		outputMode = 0;
	bool	bMorph = (blendMode == BlendMode::Morph);

	if( blendMode == BlendMode::Replace || blendMode == BlendMode::Morph || (blendMode == BlendMode::Multiply && bAlphaOnly) )
		outputMode = 1;
	else if( blendMode == BlendMode::Min )
		outputMode = 2;
	else if( (blendMode == BlendMode::Add || blendMode == BlendMode::Invert || blendMode == BlendMode::Max) && !bAlphaOnly )
		outputMode = 3;

	MTLFunctionConstantValues * pConstants = [[MTLFunctionConstantValues alloc] init];
	[pConstants setConstantValue:&outputMode type:MTLDataTypeInt atIndex:0];
	[pConstants setConstantValue:&bNoAlpha type:MTLDataTypeBool atIndex:1];
	[pConstants setConstantValue:&bMorph type:MTLDataTypeBool atIndex:2];

	NSError * pError = nil;			// Autoreleased.
	MTLRenderPipelineDescriptor *descriptor = [[MTLRenderPipelineDescriptor alloc] init];

	descriptor.label = label;
	descriptor.vertexFunction = [m_library newFunctionWithName:vertexShader];
	descriptor.fragmentFunction = [m_library newFunctionWithName:fragmentShader constantValues:pConstants error:&pError];

	[pConstants release];

	MTLRenderPipelineColorAttachmentDescriptor * pAttachment = descriptor.colorAttachments[0];

	// Set pixelFormat

	switch(destFormat)
	{
		case DestFormat::BGRA8_linear:
		case DestFormat::BGRX8_linear:
			pAttachment.pixelFormat = MTLPixelFormatBGRA8Unorm;
			break;

		case DestFormat::BGRA8_sRGB:
		case DestFormat::BGRX8_sRGB:
			pAttachment.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
			break;

		case DestFormat::Alpha_8:
			pAttachment.pixelFormat = MTLPixelFormatR8Unorm;
			break;

		default:
			assert(false);
	}

	// Blend operation and factors, for colors and alpha alike unless the alpha of the canvas is kept
	// or inverted.

	BOOL				bBlending = YES;
	MTLBlendOperation	operation = MTLBlendOperationAdd;
	MTLBlendFactor		srcFactor = MTLBlendFactorOne;
	MTLBlendFactor		dstFactor = MTLBlendFactorZero;
	bool				bKeepAlpha = false;
	bool				bInvertAlpha = false;

	switch( blendMode )
	{
		case BlendMode::Replace:
			srcFactor = MTLBlendFactorOne;
			dstFactor = MTLBlendFactorOneMinusSource1Alpha;
			bBlending = bPartialCoverage;
			break;

		case BlendMode::Undefined:
		case BlendMode::Blend:
		case BlendMode::BlendFixedColor:		// Drawn as Blend, which gives the same result.
			srcFactor = MTLBlendFactorOne;
			dstFactor = MTLBlendFactorOneMinusSourceAlpha;
			break;

		case BlendMode::Morph:
			srcFactor = MTLBlendFactorOne;
			dstFactor = MTLBlendFactorOneMinusSource1Alpha;
			break;

		case BlendMode::Add:
		case BlendMode::Subtract:
			// The source is added to or subtracted from the premultiplied colors of the canvas. Add
			// also adds the alpha the fragment shader outputs to that of the canvas, Subtract keeps it.

			operation = (blendMode == BlendMode::Add) ? MTLBlendOperationAdd : MTLBlendOperationReverseSubtract;
			srcFactor = MTLBlendFactorOne;
			dstFactor = MTLBlendFactorOne;
			bKeepAlpha = (blendMode == BlendMode::Subtract);
			break;

		case BlendMode::Multiply:
			srcFactor = MTLBlendFactorDestinationColor;
			dstFactor = MTLBlendFactorOneMinusSource1Alpha;
			bKeepAlpha = true;
			break;

		case BlendMode::Invert:
			// The alpha the fragment shader outputs is put over that of the canvas.

			srcFactor = MTLBlendFactorOneMinusDestinationColor;
			dstFactor = MTLBlendFactorOneMinusSourceColor;
			bInvertAlpha = !bAlphaOnly;
			break;

		case BlendMode::Min:
		case BlendMode::Max:
			// Max also takes the larger of the alpha the fragment shader outputs and that of the
			// canvas, Min keeps it.

			operation = (blendMode == BlendMode::Min) ? MTLBlendOperationMin : MTLBlendOperationMax;
			srcFactor = MTLBlendFactorOne;
			dstFactor = MTLBlendFactorOne;
			bKeepAlpha = (blendMode == BlendMode::Min);
			break;

		case BlendMode::Ignore:
			srcFactor = MTLBlendFactorZero;
			dstFactor = MTLBlendFactorOne;
			break;

		default:
			assert(false);
			break;
	}

	pAttachment.blendingEnabled = bBlending;
	pAttachment.rgbBlendOperation = operation;
	pAttachment.sourceRGBBlendFactor = srcFactor;
	pAttachment.destinationRGBBlendFactor = dstFactor;

	if( bKeepAlpha && !bAlphaOnly )
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
		pAttachment.alphaBlendOperation = operation;
		pAttachment.sourceAlphaBlendFactor = srcFactor;
		pAttachment.destinationAlphaBlendFactor = dstFactor;
	}

	if( bNoAlpha )
		pAttachment.writeMask = MTLColorWriteMaskRed | MTLColorWriteMaskGreen | MTLColorWriteMaskBlue;

	id<MTLRenderPipelineState> pipelineState = nil;

	if( descriptor.vertexFunction != nil && descriptor.fragmentFunction != nil )
		pipelineState = [s_metalDevice newRenderPipelineStateWithDescriptor:descriptor error:&pError];

	if( pipelineState == nil )
	{
		NSString * pMessage = [NSString stringWithFormat:@"Failed to create %@: %@", label, pError ? pError.localizedDescription : @"shader function missing"];
		Core::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, pMessage.UTF8String, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	[descriptor.vertexFunction release];
	[descriptor.fragmentFunction release];
	[descriptor release];

	return pipelineState;
}
} //namespace wax
