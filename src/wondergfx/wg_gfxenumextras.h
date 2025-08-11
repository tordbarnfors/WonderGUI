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

#ifndef	WG_GFXENUMEXTRAS_DOT_H
#define WG_GFXENUMEXTRAS_DOT_H
#pragma once

#include <wg_gfxtypes.h>

namespace wg
{

//. startAutoSection
/*=========================================================================

				>>> START OF AUTO GENERATED SECTION <<<

				Any modifications here will be overwritten!

=========================================================================*/

	const static TintMode        TintMode_min        = TintMode::None;
	const static EdgemapPalette  EdgemapPalette_min  = EdgemapPalette::Undefined;
	const static BlendMode       BlendMode_min       = BlendMode::Undefined;
	const static Alignment       Alignment_min       = Alignment::Begin;
	const static Placement       Placement_min       = Placement::Undefined;
	const static Direction       Direction_min       = Direction::Up;
	const static Axis            Axis_min            = Axis::Undefined;
	const static SampleMethod    SampleMethod_min    = SampleMethod::Nearest;
	const static CanvasInit      CanvasInit_min      = CanvasInit::Keep;
	const static PixelFormat     PixelFormat_min     = PixelFormat::Undefined;
	const static PixelType       PixelType_min       = PixelType::Chunky;
	const static ColorSpace      ColorSpace_min      = ColorSpace::Undefined;
	const static GfxFlip         GfxFlip_min         = GfxFlip::None;
	const static TransitionCurve TransitionCurve_min = TransitionCurve::Linear;
	const static CanvasRef       CanvasRef_min       = CanvasRef::None;
	const static SampleOrigo     SampleOrigo_min     = SampleOrigo::Top;
	const static Compression     Compression_min     = Compression::None;

	const static TintMode        TintMode_max        = TintMode::GradientXY;
	const static EdgemapPalette  EdgemapPalette_max  = EdgemapPalette::ColorstripXY;
	const static BlendMode       BlendMode_max       = BlendMode::BlendFixedColor;
	const static Alignment       Alignment_max       = Alignment::Justify;
	const static Placement       Placement_max       = Placement::Center;
	const static Direction       Direction_max       = Direction::Left;
	const static Axis            Axis_max            = Axis::Y;
	const static SampleMethod    SampleMethod_max    = SampleMethod::Undefined;
	const static CanvasInit      CanvasInit_max      = CanvasInit::Clear;
	const static PixelFormat     PixelFormat_max     = PixelFormat::BGRA_16_linear;
	const static PixelType       PixelType_max       = PixelType::Bitplanes;
	const static ColorSpace      ColorSpace_max      = ColorSpace::sRGB;
	const static GfxFlip         GfxFlip_max         = GfxFlip::Rot270FlipY;
	const static TransitionCurve TransitionCurve_max = TransitionCurve::Parametric;
	const static CanvasRef       CanvasRef_max       = CanvasRef::Canvas_32;
	const static SampleOrigo     SampleOrigo_max     = SampleOrigo::Bottom;
	const static Compression     Compression_max     = Compression::Spx16I;

	const static int             TintMode_size       = (int)TintMode::GradientXY + 1;
	const static int             EdgemapPalette_size = (int)EdgemapPalette::ColorstripXY + 1;
	const static int             BlendMode_size      = (int)BlendMode::BlendFixedColor + 1;
	const static int             Alignment_size      = (int)Alignment::Justify + 1;
	const static int             Placement_size      = (int)Placement::Center + 1;
	const static int             Direction_size      = (int)Direction::Left + 1;
	const static int             Axis_size           = (int)Axis::Y + 1;
	const static int             SampleMethod_size   = (int)SampleMethod::Undefined + 1;
	const static int             CanvasInit_size     = (int)CanvasInit::Clear + 1;
	const static int             PixelFormat_size    = (int)PixelFormat::BGRA_16_linear + 1;
	const static int             PixelType_size      = (int)PixelType::Bitplanes + 1;
	const static int             ColorSpace_size     = (int)ColorSpace::sRGB + 1;
	const static int             GfxFlip_size        = (int)GfxFlip::Rot270FlipY + 1;
	const static int             TransitionCurve_size= (int)TransitionCurve::Parametric + 1;
	const static int             CanvasRef_size      = (int)CanvasRef::Canvas_32 + 1;
	const static int             SampleOrigo_size    = (int)SampleOrigo::Bottom + 1;
	const static int             Compression_size    = (int)Compression::Spx16I + 1;

	const char * toString(TintMode);
	const char * toString(EdgemapPalette);
	const char * toString(BlendMode);
	const char * toString(Alignment);
	const char * toString(Placement);
	const char * toString(Direction);
	const char * toString(Axis);
	const char * toString(SampleMethod);
	const char * toString(CanvasInit);
	const char * toString(PixelFormat);
	const char * toString(PixelType);
	const char * toString(ColorSpace);
	const char * toString(GfxFlip);
	const char * toString(TransitionCurve);
	const char * toString(CanvasRef);
	const char * toString(SampleOrigo);
	const char * toString(Compression);

//=========================================================================
//. endAutoSection



} // namespace wg

#endif //WG_GFXENUMEXTRAS_DOT_H
