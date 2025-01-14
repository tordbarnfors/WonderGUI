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

#ifndef	WG_GFXTYPES_DOT_H
#define WG_GFXTYPES_DOT_H
#pragma once

#include <wg_geartypes.h>

namespace wg
{


//-----------------------------------------------------------------------------
// Defining the internal type "binalInt" which is a 32 or 64 bit integer with
// 15 and 24 binals respetively depending on if architecture is 32 or 64 bits.
// This is used internally for high-precision integer operations.

#if WG_IS_64_BITS

	typedef int64_t binalInt;
/*
	const int64_t BINAL_MUL = 32768;
	const int64_t BINAL_SHIFT = 15;
	const int64_t BINAL_MASK = 0x7FFF;
*/
	const int64_t BINAL_MUL = 16777216;
	const int64_t BINAL_SHIFT = 24;
	const int64_t BINAL_MASK = 0xFFFFFF;

#else

	typedef int32_t binalInt;
	const int32_t BINAL_MUL = 32768;
	const int32_t BINAL_SHIFT = 15;
	const int32_t BINAL_MASK = 0x7FFF;

#endif

//-----------------------------------------------------------------------------

	typedef	float	pts;
	typedef int		spx;		// Measurement in 1/64th of a pixel, e.g. pixels with 6 binals.


	static const int spx_max = 0x70FFFFC0;	// On pixel boundary with margin for rounding operation.

	//____ TintMode ___________________________________________________________

	// DEPRECATED!!!

	enum class TintMode : uint8_t //.autoExtras
	{
		None = 0,
		Flat,
		GradientX,
		GradientY,
		GradientXY
	};

	//____ EdgemapPalette ___________________________________________________________

	enum class EdgemapPalette : uint8_t //.autoExtras
	{
		Undefined = 0,
		Flat,
		ColorstripX,
		ColorstripY,
		ColorstripXY
	};


	//____ BlendMode ____________________________________________________________

	// BlendModes control how blits and fills are blended against their backgrounds and how colors are blended against each other.
	// There are two groups of BlendModes.
	// The first one is for combining two surfaces into one and consists of Replace, Morph and Blend.
	// The second one is for manipulating RGB values of destination and therefore does not update the
	// destinations alpha channel (if any), unless the destination lacks RGB values, in which case the
	// alpha is manipulated instead.


	enum class BlendMode : uint8_t	//. autoExtras
	{
		Undefined,			///< Blitting: Defaults to Blend
							///< Color Blending: Defaults to ignore
							///< This value is used internally to distinguish undefined values from an explicitly set ignore,
		Ignore,				///< Blitting: No blitting performed.
							///< Color Blending: DstRGBA = DstRGBA
		Replace,			///< Blitting: Completely opaque blitting, ignoring alpha of source and tint-color.
							///< Color Blending: DstRGBA = SrcRGBA
		Blend,				///< Blitting: Normal mode, alpha of source and tint-color is taken into account.
							///< Color Blending: DstA = SrcA, DstRGB = SrcRGB + ((TintRGB-SrcRGB)*TintA/255)
		Add,				///< Blitting: RGB Additive, alpha of source and tint-color is taken into account.
							///< Color Blending: DstRGBA = SrcRGBA + TintRGBA
		Subtract,			///< Blitting: RGB Subtractive, alpha is ignored.
							///< Color Blending: DstRGBA = SrcRGBA - TintRGBA
		Multiply,			///< Blitting: RGB Multiply, alpha is ignored.
							///< Color Blending: DstRGB = SrcRGBA * TintRGBA/255
		Invert,				///< Blitting: Inverts destination RGB values where alpha of source is non-zero. Ignores RBG components. Uses alpha of tint-color.
							///< Color Blending: DstA = SrcA, DstRGB = ((255 - SrcRGB)*TintA + SrcRGB*(255-TintA))/255
		Min,				///< Blitting: Minimum value of each RGB component, alpha is ignored.
							///< Color Blending: DstRGBA = min(SrcRGBA,DstRGBA)
		Max,				///< Blitting: Maximum value of each RGB component, alpha is ignored.
							///< Color Blending: DstRGBA = max(SrcRGBA,DstRGBA)
		Morph,				///< Blitting: Transition RGBA into source by morph factor.
							///< Color Blending: A 50% mix of the two colors.
		BlendFixedColor		///< Blitting: Blend source against fixed color and replace destination with result.
							///< Color Blending: Same as Blend
	};



	//____ Alignment _____________________________________________________________

	enum class Alignment : uint8_t	//. autoExtras
	{
	   Begin,
	   Center,
	   End,
	   Justify,
	};


	//____ Placement _____________________________________________________________

	enum class Placement : uint8_t	//. autoExtras
	{
		// Must be this specific order. Clockwise from upper left corner, center last. Must be in range 0-9
		Undefined,
		NorthWest,
		North,
		NorthEast,
		East,
		SouthEast,
		South,
		SouthWest,
		West,
		Center
	};

	//____ Direction ____________________________________________________________

	enum class Direction : uint8_t	//. autoExtras
	{
		Up,
		Right,
		Down,
		Left
	};

	//____ Axis __________________________________________________________

	enum class Axis : uint8_t	//. autoExtras
	{
		Undefined,
		X,
		Y
	};

	//____ XSections _______________________________________________________

	enum class XSections : uint8_t
	{
		None	= 0,
		Left	= 1 << 0,
		Center	= 1 << 1,
		Right	= 1 << 2,
		All = Left | Center | Right
	};

	inline XSections operator|(XSections lhs, XSections rhs) {
		return static_cast<XSections>( static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs) );
	}

	inline XSections operator&(XSections lhs, XSections rhs) {
		return static_cast<XSections>( static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs) );
	}

	//____ YSections _______________________________________________________

	enum class YSections : uint8_t
	{
		None = 0,
		Top = 1 << 0,
		Center = 1 << 1,
		Bottom = 1 << 2,
		All = Top | Center | Bottom
	};

	inline YSections operator|(YSections lhs, YSections rhs) {
		return static_cast<YSections>( static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs) );
	}

	inline YSections operator&(YSections lhs, YSections rhs) {
		return static_cast<YSections>( static_cast<uint8_t>(lhs) &	static_cast<uint8_t>(rhs) );
	}

	//____ SampleMethod ____________________________________________________________

	enum class SampleMethod : uint8_t	//. autoExtras
	{
		Nearest,
		Bilinear,
		Undefined			// Needs to be last! Default to Bilinear if it is accelerated, otherwise Nearest.
	};

	//____ CanvasInit ______________________________________________________________

	enum class CanvasInit 		//. autoExtras
	{
		Keep,
		Discard,
		Clear
	};


	//____ PixelFormat _____________________________________________________________

	enum class PixelFormat : uint8_t	//. autoExtras
	{
		Undefined,			///< Pixelformat is undefined.
		BGR_8,				///< One byte of blue, green and red in exactly that order in memory.
		BGR_8_sRGB,			///< One byte of blue, green and red in exactly that order in memory.
		BGR_8_linear,		///< One byte of blue, green and red in exactly that order in memory.

		BGRX_8,				///< One byte of blue, green, red and padding in exactly that order in memory.
		BGRX_8_sRGB,		///< One byte of blue, green, red and padding in exactly that order in memory.
		BGRX_8_linear,		///< One byte of blue, green, red and padding in exactly that order in memory.

		BGRA_8,				///< One byte of blue, green, red and alpha in exactly that order in memory.
		BGRA_8_sRGB,		///< One byte of blue, green, red and alpha in exactly that order in memory.
		BGRA_8_linear,		///< One byte of blue, green, red and alpha in exactly that order in memory.

		Index_8,			///< 8 bits of index into the palette.
		Index_8_sRGB,		///< 8 bits of index into an sRGB palette.
		Index_8_linear,		///< 8 bits of index into a linear palette.

		Index_16,			///< 16 bits of index into the palette.
		Index_16_sRGB,		///< 16 bits of index into an sRGB palette.
		Index_16_linear,	///< 16 bits of index into a linear palette.

		Alpha_8,			///< 8 bits of alpha only.
		
		BGRA_4_linear,		///< 4 bits each of blue, green, red and alpha in exactly that order in memory.

		BGR_565,
		BGR_565_sRGB,		///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.
		BGR_565_linear,		///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.

		RGB_565_bigendian,	///< 3 high bits of green, 5 bits of blue, 5 bits of red and 3 low bits of green in exactly that order in memory. Linear and big endian.
		RGB_555_bigendian,	/// Same layout as RGB_565_bigendian, but lowest bit of green is always cleared. Avoids accidental tinting towards green of dark colors.

		Bitplanes_1,
		Bitplanes_2,
		Bitplanes_4,
		Bitplanes_5,
		Bitplanes_8,

		Bitplanes_A1_1,
		Bitplanes_A1_2,
		Bitplanes_A1_4,
		Bitplanes_A1_5,
		Bitplanes_A1_8,

		BGRX_16_linear,
		BGRA_16_linear
	};



	//____ PixelType _________________________________________________________

	enum class PixelType		//. autoExtras
	{
		Chunky,						///< Normal pixel. All bits for a pixel are packed into same sequence of bytes.
		Chunky_BE,					///< Same as Chunky, but stored in big-endian format.
		Index,						///< Pixels are color indexes into a palette.
		Bitplanes					///< Pixels are color indexes into a palette, stored in 16-bit bitplanes. Starting with lowest bitplane.
	};

	//____ ColorSpace ________________________________________________________

	enum class ColorSpace	//. autoExtras
	{
		Undefined,
		Linear,
		sRGB
	};

	//____ PixelDescription _________________________________________________

	struct PixelDescription
	{
		// These constructors are needed for Clang to compile GfxUtil::pixelDescTab in C++11 setting.
		
		PixelDescription() {}
		constexpr PixelDescription(int _bits, PixelType _type, ColorSpace _colorSpace, uint64_t _R_mask, uint64_t _G_mask, uint64_t _B_mask, uint64_t _A_mask ) :
		bits(_bits), type(_type), colorSpace(_colorSpace), R_mask(_R_mask), G_mask(_G_mask), B_mask(_B_mask), A_mask(_A_mask) {}
		
		bool operator==( const PixelDescription& k ) const
		{
			return ( bits == k.bits && type == k.type && colorSpace == k.colorSpace && R_mask == k.R_mask && G_mask == k.G_mask && B_mask == k.B_mask && A_mask == k.A_mask );
		}
		
		
		int			bits = 0;			///< Number of bits for the pixel, includes any non-used padding bits.
		PixelType	type = PixelType::Chunky;
		ColorSpace	colorSpace = ColorSpace::sRGB;
		
		uint64_t	R_mask = 0;			///< bitmask for getting the red bits out of chunky pixel
		uint64_t	G_mask = 0;			///< bitmask for getting the green bits out of chunky pixel
		uint64_t	B_mask = 0;			///< bitmask for getting the blue bits out of chunky pixel
		uint64_t	A_mask = 0;			///< bitmask for getting the alpha bits out of chunky pixel
	};

	//____ GfxFlip ____________________________________________________________

	enum class GfxFlip : uint8_t	//. autoExtras
	{
		None = 0,
		FlipX,
		FlipY,
		Rot90,
		Rot90FlipX,
		Rot90FlipY,
		Rot180,
		Rot180FlipX,
		Rot180FlipY,
		Rot270,
		Rot270FlipX,
		Rot270FlipY,
	};

	//____ Transform _________________________________________________________

	struct Transform
	{
		float	xx, xy;		// Increment of X and Y position in source when increasing destination along X-axis
		float	yx, yy;		// Increment of X and Y position in source when increasing destination along Y-axis
	};

	//____ TransitionCurve ________________________________________________________

	enum class TransitionCurve	//. autoExtras
	{
		Linear,
		EaseIn,
		EaseOut,
		EaseInOut,
		Bezier,
		Parametric

	};


	//____ CanvasRef ____________________________________________________________

	enum class CanvasRef			//.autoExtras
	{
		None,
		Default,
		Canvas_1,
		Canvas_2,
		Canvas_3,
		Canvas_4,
		Canvas_5,
		Canvas_6,
		Canvas_7,
		Canvas_8,
		Canvas_9,
		Canvas_10,
		Canvas_11,
		Canvas_12,
		Canvas_13,
		Canvas_14,
		Canvas_15,
		Canvas_16,
		Canvas_17,
		Canvas_18,
		Canvas_19,
		Canvas_20,
		Canvas_21,
		Canvas_22,
		Canvas_23,
		Canvas_24,
		Canvas_25,
		Canvas_26,
		Canvas_27,
		Canvas_28,
		Canvas_29,
		Canvas_30,
		Canvas_31,
		Canvas_32
	};

	//____ SampleOrigo _____________________________________________________________

	enum class SampleOrigo			//.autoExtras
	{
		Top,				// Origo at top, increasing values goes down.
		MiddleUp,			// Origo at middle, increasing values goes up.
		MiddleDown,			// Origo at middle, increasing values goes down.
		Bottom				// Origo at bottom, increasing values goes up.
	};

	//____ Compression ___________________________________________________________

	enum class Compression			//.autoExtras
	{
		None,
		SpxU8I,
		Spx16B,
		Spx16I
	};



} // namespace wg

#include <wg_gfxenumextras.h>

#endif // WG_GFXTYPES_DOT_H
