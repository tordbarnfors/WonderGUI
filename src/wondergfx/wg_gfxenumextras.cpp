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

#include <wg_gfxenumextras.h>

namespace wg
{

//. startAutoSection
/*=========================================================================

				>>> START OF AUTO GENERATED SECTION <<<

				Any modifications here will be overwritten!

=========================================================================*/

	const char * toString(TintMode i)
	{
		static const char * names[] = { 
			"None",
			"Flat",
			"GradientX",
			"GradientY",
			"GradientXY" };

		return names[(int)i];
	}

	const char * toString(EdgemapPalette i)
	{
		static const char * names[] = {
			"Undefined",
			"Flat",
			"ColorstripX",
			"ColorstripY",
			"ColorstripXY" };

		return names[(int)i];
	}

	const char * toString(BlendMode i)
	{
		static const char * names[] = { 
			"Undefined",
			"Ignore",
			"Replace",
			"Blend",
			"Add",
			"Subtract",
			"Multiply",
			"Invert",
			"Min",
			"Max",
			"Morph",
			"BlendFixedColor" };

		return names[(int)i];
	}

	const char * toString(Alignment i)
	{
		static const char * names[] = { 
			"Begin",
			"Center",
			"End",
			"Justify" };

		return names[(int)i];
	}

	const char * toString(Placement i)
	{
		static const char * names[] = { 
			"Undefined",
			"NorthWest",
			"North",
			"NorthEast",
			"East",
			"SouthEast",
			"South",
			"SouthWest",
			"West",
			"Center" };

		return names[(int)i];
	}

	const char * toString(Direction i)
	{
		static const char * names[] = { 
			"Up",
			"Right",
			"Down",
			"Left" };

		return names[(int)i];
	}

	const char * toString(Axis i)
	{
		static const char * names[] = { 
			"Undefined",
			"X",
			"Y" };

		return names[(int)i];
	}

	const char * toString(SampleMethod i)
	{
		static const char * names[] = { 
			"Nearest",
			"Bilinear",
			"Undefined" };

		return names[(int)i];
	}

	const char * toString(CanvasInit i)
	{
		static const char * names[] = { 
			"Keep",
			"Discard",
			"Clear" };

		return names[(int)i];
	}

	const char * toString(PixelFormat i)
	{
		static const char * names[] = { 
			"Undefined",
			"BGR_8",
			"BGR_8_sRGB",
			"BGR_8_linear",
			"BGRX_8",
			"BGRX_8_sRGB",
			"BGRX_8_linear",
			"BGRA_8",
			"BGRA_8_sRGB",
			"BGRA_8_linear",
			"Index_8",
			"Index_8_sRGB",
			"Index_8_linear",
			"Index_16",
			"Index_16_sRGB",
			"Index_16_linear",
			"Alpha_8",
			"BGRA_4_linear",
			"BGR_565",
			"BGR_565_sRGB",
			"BGR_565_linear",
			"RGB_565_bigendian",
			"RGB_555_bigendian",
			"Bitplanes_1",
			"Bitplanes_2",
			"Bitplanes_4",
			"Bitplanes_5",
			"Bitplanes_8",
			"Bitplanes_A1_1",
			"Bitplanes_A1_2",
			"Bitplanes_A1_4",
			"Bitplanes_A1_5",
			"Bitplanes_A1_8",
			"BGRX_16_linear",
			"BGRA_16_linear" };

		return names[(int)i];
	}

	const char * toString(PixelType i)
	{
		static const char * names[] = { 
			"Chunky",
			"Chunky_BE",
			"Index",
			"Bitplanes" };

		return names[(int)i];
	}

	const char * toString(ColorSpace i)
	{
		static const char * names[] = { 
			"Undefined",
			"Linear",
			"sRGB" };

		return names[(int)i];
	}

	const char * toString(GfxFlip i)
	{
		static const char * names[] = { 
			"None",
			"FlipX",
			"FlipY",
			"Rot90",
			"Rot90FlipX",
			"Rot90FlipY",
			"Rot180",
			"Rot180FlipX",
			"Rot180FlipY",
			"Rot270",
			"Rot270FlipX",
			"Rot270FlipY" };

		return names[(int)i];
	}

	const char * toString(TransitionCurve i)
	{
		static const char * names[] = { 
			"Linear",
			"EaseIn",
			"EaseOut",
			"EaseInOut",
			"Bezier",
			"Parametric" };

		return names[(int)i];
	}

	const char * toString(CanvasRef i)
	{
		static const char * names[] = { 
			"None",
			"Default",
			"Canvas_1",
			"Canvas_2",
			"Canvas_3",
			"Canvas_4",
			"Canvas_5",
			"Canvas_6",
			"Canvas_7",
			"Canvas_8",
			"Canvas_9",
			"Canvas_10",
			"Canvas_11",
			"Canvas_12",
			"Canvas_13",
			"Canvas_14",
			"Canvas_15",
			"Canvas_16",
			"Canvas_17",
			"Canvas_18",
			"Canvas_19",
			"Canvas_20",
			"Canvas_21",
			"Canvas_22",
			"Canvas_23",
			"Canvas_24",
			"Canvas_25",
			"Canvas_26",
			"Canvas_27",
			"Canvas_28",
			"Canvas_29",
			"Canvas_30",
			"Canvas_31",
			"Canvas_32" };

		return names[(int)i];
	}

	const char * toString(SampleOrigo i)
	{
		static const char * names[] = { 
			"Top",
			"MiddleUp",
			"MiddleDown",
			"Bottom" };

		return names[(int)i];
	}

	const char * toString(GfxChunkId i)
	{
		static const char * names[] = { 
			"OutOfData",
			"BeginRender",
			"EndRender",
			"Flush",
			"BeginCanvasUpdate",
			"EndCanvasUpdate",
			"SetClipList",
			"ResetClipList",
			"PushClipList",
			"PopClipList",
			"SetTintColor",
			"SetTintGradient",
			"ClearTintGradient",
			"SetBlendMode",
			"SetBlitSource",
			"SetMorphFactor",
			"SetRenderLayer",
			"Fill",
			"FillRect",
			"DrawLineStraight",
			"PlotPixels",
			"DrawLineFromTo",
			"DrawLineStraightDeprecated",
			"Blit",
			"BlitRect",
			"FlipBlit",
			"FlipBlitRect",
			"StretchBlit",
			"StretchBlitRect",
			"StretchFlipBlit",
			"StretchFlipBlitRect",
			"PrecisionBlit",
			"TransformBlit",
			"RotScaleBlit",
			"Tile",
			"FlipTile",
			"ScaleTile",
			"ScaleFlipTile",
			"DrawWave",
			"FlipDrawWave",
			"DrawElipse",
			"DrawPieChart",
			"DrawSegments",
			"FlipDrawSegments",
			"BlitNinePatch",
			"EdgeSamples",
			"CreateSurfaceDeprecated",
			"BeginSurfaceUpdate",
			"SurfacePixels",
			"EndSurfaceUpdate",
			"FillSurface",
			"CopySurface",
			"DeleteSurface",
			"ProtocolVersion",
			"TimeStampMS",
			"CanvasList",
			"SetFixedBlendColor",
			"CreateSurface",
			"CreateEdgemap",
			"SetEdgemapRenderSegments",
			"SetEdgemapColors",
			"SetEdgemapGradients",
			"BeginEdgemapUpdate",
			"EdgemapSamples",
			"EndEdgemapUpdate",
			"DeleteEdgemap",
			"DrawEdgemap",
			"FlipDrawEdgemap" };

		return names[(int)i];
	}


//=========================================================================
//. endAutoSection



}	//namespace wg
