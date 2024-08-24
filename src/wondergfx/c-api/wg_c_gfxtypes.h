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

#ifndef WG_C_GFXTYPES_DOT_H
#define WG_C_GFXTYPES_DOT_H
#pragma once

#include <wg_c_geartypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//____ wg_placement ___________________________________________________________

typedef enum
{
	// Clockwise from upper left corner, center last. Must be in range 0-8

	WG_NORTHWEST = 0,
	WG_NORTH,
	WG_NORTHEAST,
	WG_EAST,
	WG_SOUTHEAST,
	WG_SOUTH,
	WG_SOUTHWEST,
	WG_WEST,
	WG_CENTER

} wg_placement;

//____ wg_tintMode ___________________________________________________________

typedef enum
{
	WG_TINT_NONE = 0,
	WG_TINT_FLAT,
	WG_TINT_GRADIENT_X,
	WG_TINT_GRADIENT_Y,
	WG_TINT_GRADIENT_XY
} wg_tintMode;


//____ wg_blendMode ____________________________________________________________

typedef enum
{
	WG_BLENDMODE_UNDEFINED,
	WG_BLENDMODE_IGNORE,
	WG_BLENDMODE_REPLACE,
	WG_BLENDMODE_ADD,
	WG_BLENDMODE_SUBTRACT,
	WG_BLENDMODE_MULTIPLY,
	WG_BLENDMODE_INVERT,
	WG_BLENDMODE_MIN,
	WG_BLENDMODE_MAX,
	WG_BLENDMODE_MORPH
} wg_blendMode;

//____ wg_direction ____________________________________________________________

typedef enum
{
	WG_DIR_UP,
	WG_DIR_RIGHT,
	WG_DIR_DOWM,
	WG_DIR_LEFT
} wg_direction;

//____ wg_sampleMethod ____________________________________________________________

typedef enum
{
	WG_SAMPLE_NEAREST,
	WG_SAMPLE_BILINEAR,
	WG_SAMPLE_UNDEFINED			// Default to Bilinear if it is accelerated, otherwise Nearest.
} wg_sampleMethod;

//____ wg_pixelFormat _____________________________________________________________

typedef enum
{
	WG_PIXFMT_UNDEFINED,
	WG_PIXFMT_BGR_8,				///< One byte of blue, green and red in exactly that order in memory.
	WG_PIXFMT_BGR_8_SRGB,			///< One byte of blue, green and red in exactly that order in memory.
	WG_PIXFMT_BGR_8_LINEAR,			///< One byte of blue, green and red in exactly that order in memory.

	WG_PIXFMT_BGRX_8,				///< One byte of blue, green, red and padding in exactly that order in memory.
	WG_PIXFMT_BGRX_8_SRGB,			///< One byte of blue, green, red and padding in exactly that order in memory.
	WG_PIXFMT_BGRX_8_LINEAR,		///< One byte of blue, green, red and padding in exactly that order in memory.

	WG_PIXFMT_BGRA_8,				///< One byte of blue, green, red and alpha in exactly that order in memory.
	WG_PIXFMT_BGRA_8_SRGB,			///< One byte of blue, green, red and alpha in exactly that order in memory.
	WG_PIXFMT_BGRA_8_LINEAR,		///< One byte of blue, green, red and alpha in exactly that order in memory.

	WG_PIXFMT_INDEX_8,				///< 8 bits of index into the palette.
	WG_PIXFMT_INDEX_8_SRGB,			///< 8 bits of index into the palette.
	WG_PIXFMT_INDEX_8_LINEAR,		///< 8 bits of index into the palette.

	WG_PIXFMT_INDEX_16,				///< 8 bits of index into the palette.
	WG_PIXFMT_INDEX_16_SRGB,		///< 8 bits of index into the palette.
	WG_PIXFMT_INDEX_16_LINEAR,		///< 8 bits of index into the palette.

	WG_PIXFMT_ALPHA_8,

	WG_PIXFMT_BGRA_4_LINEAR,		///< 4 bits each of blue, green, red and alpha in exactly that order in memory.

	WG_PIXFMT_BGR_565,				///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.
	WG_PIXFMT_BGR_565_sRGB,			///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.
	WG_PIXFMT_BGR_565_LINEAR,		///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.

	WG_PIXFMT_RGB_565_BIGENDIAN,	///< 3 high bits of green, 5 bits of blue, 5 bits of red and 3 low bits of green in exactly that order in memory.
	WG_PIXFMT_RGB_555_BIGENDIAN,	/// Same layout as RGB_565_bigendian, but lowest bit of green is always cleared. Avoids accidental tinting towards green of dark colors.

	WG_PIXFMT_BITPLANES_1,
	WG_PIXFMT_BITPLANES_2,
	WG_PIXFMT_BITPLANES_4,
	WG_PIXFMT_BITPLANES_5,
	WG_PIXFMT_BITPLANES_8,

	WG_PIXFMT_BITPLANES_1_A1,
	WG_PIXFMT_BITPLANES_2_A1,
	WG_PIXFMT_BITPLANES_4_A1,
	WG_PIXFMT_BITPLANES_5_A1,
	WG_PIXFMT_BITPLANES_8_A1
	
} wg_pixelFormat;


//____ wg_pixelType _________________________________________________________

typedef enum
{
	WG_PIXTYPE_CHUNKY,			///< Normal pixel. All bits for a pixel are packed into same sequence of bytes.
	WG_PIXTYPE_CHUNKY_BE,		///< Same as Chunky, but stored in big-endian format.
	WG_PIXTYPE_INDEX,			///< Pixels are color indexes into a palette.
	WG_PIXTYPE_BITPLANES		///< Pixels are color indexes into a palette, stored in 16-bit bitplanes. Starting with lowest bitplane.
} wg_pixelType;

//____ wg_colorSpace ________________________________________________________

typedef enum
{
	WG_COLORSPACE_UNDEFINED,
	WG_COLORSPACE_LINEAR,
	WG_COLORSPACE_SRGB
} wg_colorSpace;

//____ wg_pixelDescription _____________________________________________________

typedef struct wg_pixelDescription_struct
{
	int				bits;			///< Number of bits for the pixel, includes any non-used padding bits.
	wg_pixelType	type;
	wg_colorSpace	colorSpace;
	
	uint64_t		R_mask;			///< bitmask for getting the red bits out of chunky pixel
	uint64_t		G_mask;			///< bitmask for getting the green bits out of chunky pixel
	uint64_t		B_mask;			///< bitmask for getting the blue bits out of chunky pixel
	uint64_t		A_mask;			///< bitmask for getting the alpha bits out of chunky pixel
} wg_pixelDescription;

//____ wg_gfxFlip ____________________________________________________________

typedef enum
{
	WG_GFXFLIP_NORMAL = 0,
	WG_GFXFLIP_X,
	WG_GFXFLIP_Y,
	WG_GFXFLIP_ROT90,
	WG_GFXFLIP_ROT90_FLIP_X,
	WG_GFXFLIP_ROT90_FLIP_Y,
	WG_GFXFLIP_ROT180,
	WG_GFXFLIP_ROT180_FLIP_X,
	WG_GFXFLIP_ROT180_FLIP_Y,
	WG_GFXFLIP_ROT270,
	WG_GFXFLIP_ROT270_FLIP_X,
	WG_GFXFLIP_ROT270_FLIP_Y,
} wg_gfxFlip;

typedef struct
{
	float	xx, xy;		// Increment of X and Y position in source when increasing destination along X-axis
	float	yx, yy;		// Increment of X and Y position in source when increasing destination along Y-axis
}  wg_transform;

//____ wg_canvasRef ____________________________________________________________

typedef enum
{
	WG_CANVAS_NONE,
	WG_CANVAS_DEFAULT,
	WG_CANVAS_1,
	WG_CANVAS_2,
	WG_CANVAS_3,
	WG_CANVAS_4,
	WG_CANVAS_5,
	WG_CANVAS_6,
	WG_CANVAS_7,
	WG_CANVAS_8,
	WG_CANVAS_9,
	WG_CANVAS_10,
	WG_CANVAS_11,
	WG_CANVAS_12,
	WG_CANVAS_13,
	WG_CANVAS_14,
	WG_CANVAS_15,
	WG_CANVAS_16,
	WG_CANVAS_17,
	WG_CANVAS_18,
	WG_CANVAS_19,
	WG_CANVAS_20,
	WG_CANVAS_21,
	WG_CANVAS_22,
	WG_CANVAS_23,
	WG_CANVAS_24,
	WG_CANVAS_25,
	WG_CANVAS_26,
	WG_CANVAS_27,
	WG_CANVAS_28,
	WG_CANVAS_29,
	WG_CANVAS_30,
	WG_CANVAS_31,
	WG_CANVAS_32
} wg_canvasRef;

//____ wg_sampleOrigo _____________________________________________________________

typedef enum
{
	WG_WAVEORIGO_TOP = 0,				// Origo at top, increasing values goes down.
	WG_WAVEORIGO_MIDDLE_UP = 1,			// Origo at middle, increasing values goes up.
	WG_WAVEORIGO_MIDDLE_DOWN = 2,		// Origo at middle, increasing values goes down.
	WG_WAVEORIGO_BOTTOM = 3				// Origo at bottom, increasing values goes up.
} wg_sampleOrigo;

//____ wg_dataSegment _________________________________________________________

typedef struct wg_dataSegment_struct
{
	uint8_t*	pBegin;
	uint8_t*	pEnd;
} wg_dataSegment;

//____ wg_gfxChunkId ____________________________________________________

typedef enum
{
	WG_GFXCHUNK_OUT_OF_DATA = 0,
	WG_GFXCHUNK_BEGIN_RENDER = 1,
	WG_GFXCHUNK_END_RENDER = 2,
	WG_GFXCHUNK_FLUSH = 3,

	WG_GFXCHUNK_BEGIN_CANVAS_UPDATE = 4,
	WG_GFXCHUNK_END_CANVAS_UPDATE = 5,

	WG_GFXCHUNK_SET_CLIP_LIST = 6,
	WG_GFXCHUNK_RESET_CLIP_LIST = 7,
	WG_GFXCHUNK_PUSH_CLIP_LIST = 8,
	WG_GFXCHUNK_POP_CLIP_LIST = 9,

	WG_GFXCHUNK_SET_TINT_COLOR = 10,
	WG_GFXCHUNK_SET_TINT_GRADIENT = 11,
	WG_GFXCHUNK_CLEAR_TINT_GRADIENT = 12,

	WG_GFXCHUNK_SET_BLEND_MODE = 13,
	WG_GFXCHUNK_SET_BLIT_SOURCE = 14,
	WG_GFXCHUNK_SET_MORPH_FACTOR = 15,
	WG_GFXCHUNK_SET_RENDER_LAYER = 16,

	WG_GFXCHUNK_FILL = 17,
	WG_GFXCHUNK_FILL_RECT = 18,
	WG_GFXCHUNK_DRAW_LINE_STRAIGHT = 19,
	WG_GFXCHUNK_PLOT_PIXELS = 20,
	WG_GFXCHUNK_DRAW_LINE_FROM_TO = 21,
	WG_GFXCHUNK_DRAW_LINE_STRAIGHT_DEPRECATED = 22,

	WG_GFXCHUNK_BLIT = 23,
	WG_GFXCHUNK_BLIT_RECT = 24,
	WG_GFXCHUNK_FLIP_BLIT = 25,
	WG_GFXCHUNK_FLIP_BLIT_RECT = 26,

	WG_GFXCHUNK_STRETCH_BLIT = 27,
	WG_GFXCHUNK_STRETCH_BLIT_RECT = 28,
	WG_GFXCHUNK_PRECISION_BLIT = 29,

	WG_GFXCHUNK_STRETCH_FLIP_BLIT = 30,
	WG_GFXCHUNK_STRETCH_FLIP_BLIT_RECT = 31,
	WG_GFXCHUNK_TRANSFORM_BLIT = 32,

	WG_GFXCHUNK_ROT_SCALE_BLIT = 33,
	WG_GFXCHUNK_TILE = 34,
	WG_GFXCHUNK_FLIP_TILE = 35,

	WG_GFXCHUNK_SCALE_TILE = 36,
	WG_GFXCHUNK_SCALE_FLIP_TILE = 37,

	WG_GFXCHUNK_DRAW_WAVE = 38,
	WG_GFXCHUNK_FLIP_DRAW_WAVE = 39,

	WG_GFXCHUNK_DRAW_ELIPSE = 40,
	WG_GFXCHUNK_DRAW_PIE_CHART = 41,

	WG_GFXCHUNK_DRAW_SEGMENTS = 42,
	WG_GFXCHUNK_FLIP_DRAW_SEGMENTS = 43,

	WG_GFXCHUNK_BLIT_NINE_PATCH = 44,

	WG_GFXCHUNK_EDGE_SAMPLES = 45,

	WG_GFXCHUNK_CREATE_SURFACE_DEPRECATED = 46,
	WG_GFXCHUNK_BEGIN_SURFACE_UPDATE = 47,
	WG_GFXCHUNK_SURFACE_PIXELS = 48,
	WG_GFXCHUNK_END_SURFACE_UPDATE = 49,
	WG_GFXCHUNK_FILL_SURFACE = 50,
	WG_GFXCHUNK_COPY_SURFACE = 51,
	WG_GFXCHUNK_DELETE_SURFACE = 52,

	WG_GFXCHUNK_PROTOCOL_VERSION = 53,
	WG_GFXCHUNK_TIME_STAMP_MS = 54,
	WG_GFXCHUNK_CANVAS_LIST = 55,
	
	WG_GFXCHUNK_SET_FIXED_BLEND_COLOR = 56,
	WG_GFXCHUNK_CREATE_SURFACE = 57,
	
	WG_GFXCHUNK_CREATE_EDGEMAP = 58,
	WG_GFXCHUNK_SET_EDGEMAP_RENDER_SEGMENTS = 59,
	WG_GFXCHUNK_SET_EDGEMAP_COLORS = 60,
	WG_GFXCHUNK_SET_EDGEMAP_GRADIENTS = 61,
	WG_GFXCHUNK_BEGIN_EDGEMAP_UPDATE = 62,
	WG_GFXCHUNK_EDGEMAP_SAMPLES = 63,
	WG_GFXCHUNK_END_EDGEMAP_UPDATE = 64,
	WG_GFXCHUNK_DELETE_EDGEMAP = 65,

	WG_GFXCHUNK_DRAW_EDGEMAP = 66,
	WG_GFXCHUNK_FLIP_DRAW_EDGEMAP = 67
	
} wg_gfxChunkId;


#ifdef __cplusplus
}
#endif

#endif
