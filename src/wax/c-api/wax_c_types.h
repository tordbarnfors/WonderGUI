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
#ifndef WAX_C_TYPES_DOT_H
#define WAX_C_TYPES_DOT_H
#pragma once

#include <stdint.h>

#if __has_include(<wax_export.h>)
#include <wax_export.h>
#else
#	define WAX_EXPORT
#	define WAX_NO_EXPORT
#	define WAX_DEPRECATED
#	define	WAX_DEPRECATED_EXPORT
#	define	WAX_DEPRECATED_NO_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* wax_obj;


typedef	void(*wax_finalizer_func)(wax_obj);

//____ wax_component __________________________________________________________

typedef struct wax_component_struct
{
	wax_obj object;
	void * pInterface;
} wax_component;


//____ wax_typeInfo ___________________________________________________________

typedef struct wax_typeInfo_struct
{
	const char* className;
	const struct wax_typeInfo_struct* pSuperClass;
} wax_typeInfo;

//____ wax_errorLevel ________________________________________________

typedef enum
{
	WAX_WARNING = 0,
	WAX_SILENT_ERROR,
	WAX_ERROR,
	WAX_CRITICAL
} wax_errorLevel;

//____ wax_errorCode ____________________________________________________

typedef enum
{
	WAX_ERROR_OUT_OF_RANGE = 0,
	WAX_ERROR_INVALID_ITERATOR,
	WAX_ERROR_INVALID_PARAM,
	WAX_ERROR_FAILED_PREREQUISITE,
	WAX_ERROR_OPENGL,						// OPENGL HAS ISSUED A GLERROR
	WAX_ERROR_INTERNAL,
	WAX_ERROR_SYSTEM_INTEGRITY,
	WAX_ERROR_ILLEGAL_CALL,
	WAX_ERROR_RESOURCE_EXHAUSTED,			// A LIMITED INTERNAL RESOURCE HAS BEEN EXHAUSTED
	WAX_ERROR_RENDER_FAILURE,
	WAX_ERROR_PERFORMANCE,					// ONLY USED IN COMBO WITH WAX_WARNING.
	WAX_ERROR_OTHER
} wax_errorCode;

//____ wax_errorInfo _____________________________________________________

typedef struct wax_errorInfo_struct
{
	wax_errorLevel	severity;
	wax_errorCode		code;
	const char * 		message;
	const void *		object;
	const char *		classname;
	const char *		function;
	const char *		file;
	int					line;
} wax_errorInfo;


//____ wax_placement ___________________________________________________________

typedef enum
{
	// Clockwise from upper left corner, center last. Must be in range 0-8

	WAX_NORTHWEST = 0,
	WAX_NORTH,
	WAX_NORTHEAST,
	WAX_EAST,
	WAX_SOUTHEAST,
	WAX_SOUTH,
	WAX_SOUTHWEST,
	WAX_WEST,
	WAX_CENTER

} wax_placement;

//____ wax_tintMode ___________________________________________________________

// DEPRECATED!!

typedef enum
{
	WAX_TINT_NONE = 0,
	WAX_TINT_FLAT,
	WAX_TINT_GRADIENT_X,
	WAX_TINT_GRADIENT_Y,
	WAX_TINT_GRADIENT_XY
} wax_tintMode;

//____ wax_tintShape ___________________________________________________________

typedef enum
{
	WAX_TINTSHAPE_LINEAR = 0,
	WAX_TINTSHAPE_RADIAL
} wax_tintShape;

//____ wax_tintSpread ___________________________________________________________

typedef enum
{
	WAX_TINTSPREAD_PAD = 0,
	WAX_TINTSPREAD_REPEAT,
	WAX_TINTSPREAD_REFLECT
} wax_tintSpread;

//____ wax_tintRadius ___________________________________________________________

typedef enum
{
	WAX_TINTRADIUS_FIT = 0,
	WAX_TINTRADIUS_CIRCLE
} wax_tintRadius;


//____ wax_blendMode ____________________________________________________________

typedef enum
{
	WAX_BLENDMODE_UNDEFINED,
	WAX_BLENDMODE_IGNORE,
	WAX_BLENDMODE_REPLACE,
	WAX_BLENDMODE_ADD,
	WAX_BLENDMODE_SUBTRACT,
	WAX_BLENDMODE_MULTIPLY,
	WAX_BLENDMODE_INVERT,
	WAX_BLENDMODE_MIN,
	WAX_BLENDMODE_MAX,
	WAX_BLENDMODE_MORPH
} wax_blendMode;

//____ wax_direction ____________________________________________________________

typedef enum
{
	WAX_DIR_UNDEFINED,
	WAX_DIR_UP,
	WAX_DIR_RIGHT,
	WAX_DIR_DOWN,
	WAX_DIR_LEFT
} wax_direction;

//____ wax_lineCapShape ________________________________________________________

typedef enum
{
	WAX_LINECAP_NONE,
	WAX_LINECAP_FLAT,
	WAX_LINECAP_SQUARE,
	WAX_LINECAP_ROUND,
	WAX_LINECAP_VERTICAL,
	WAX_LINECAP_HORIZONTAL,
	WAX_LINECAP_SPEAR
} wax_lineCapShape;

//____ wax_sampleMethod ____________________________________________________________

typedef enum
{
	WAX_SAMPLE_NEAREST,
	WAX_SAMPLE_BILINEAR,
	WAX_SAMPLE_UNDEFINED			// Default to Bilinear if it is accelerated, otherwise Nearest.
} wax_sampleMethod;

//____ wax_pixelFormat _____________________________________________________________

// Channels are named in register order, starting with the most significant bits.
// Byte order and color space are separate properties.

typedef enum
{
	WAX_PIXFMT_UNDEFINED,
	WAX_PIXFMT_XRGB_8,				///< 8 bits each of padding, red, green and blue.
	WAX_PIXFMT_ARGB_8,				///< 8 bits each of alpha, red, green and blue.
	WAX_PIXFMT_INDEX_8,				///< 8 bits of index into the palette.
	WAX_PIXFMT_INDEX_16,				///< 16 bits of index into the palette.
	WAX_PIXFMT_ALPHA_8,				///< 8 bits of alpha only.
	WAX_PIXFMT_RGB_565,				///< 5 bits of red, 6 bits of green and 5 bits of blue.
	WAX_PIXFMT_BGR_565,				///< 5 bits of blue, 6 bits of green and 5 bits of red.

	WAX_PIXFMT_BITPLANES_1,
	WAX_PIXFMT_BITPLANES_2,
	WAX_PIXFMT_BITPLANES_4,
	WAX_PIXFMT_BITPLANES_5,
	WAX_PIXFMT_BITPLANES_8,

	WAX_PIXFMT_BITPLANES_A1_1,
	WAX_PIXFMT_BITPLANES_A1_2,
	WAX_PIXFMT_BITPLANES_A1_4,
	WAX_PIXFMT_BITPLANES_A1_5,
	WAX_PIXFMT_BITPLANES_A1_8,

	WAX_PIXFMT_XRGB_16,				///< 16 bits each of padding, red, green and blue.
	WAX_PIXFMT_ARGB_16				///< 16 bits each of alpha, red, green and blue.
} wax_pixelFormat;


//____ wax_pixelType _________________________________________________________

typedef enum
{
	WAX_PIXTYPE_CHUNKY,			///< Normal pixel. All bits for a pixel are packed into same sequence of bytes.
	WAX_PIXTYPE_INDEX,			///< Pixels are color indexes into a palette.
	WAX_PIXTYPE_BITPLANES		///< Pixels are color indexes into a palette, stored in 16-bit bitplanes. Starting with lowest bitplane.
} wax_pixelType;

//____ wax_colorSpace ________________________________________________________

typedef enum
{
	WAX_COLORSPACE_UNDEFINED,	///< Only for blueprints, gives the default (sRGB).
	WAX_COLORSPACE_LINEAR,
	WAX_COLORSPACE_SRGB
} wax_colorSpace;

//____ wax_byteOrder ________________________________________________________

typedef enum
{
	WAX_BYTEORDER_NATIVE,		///< Byte order of the system.
	WAX_BYTEORDER_LITTLE_ENDIAN,
	WAX_BYTEORDER_BIG_ENDIAN
} wax_byteOrder;

//____ wax_pixelDescription _____________________________________________________

typedef struct wax_pixelDescription_struct
{
	int				bits;			///< Number of bits for the pixel, includes any non-used padding bits.
	wax_pixelType	type;
	uint8_t			bigEndian;		///< Byte order of the pixels (of the 16-bit words for bitplanes).

	uint64_t		R_mask;			///< bitmask for getting the red bits out of chunky pixel
	uint64_t		G_mask;			///< bitmask for getting the green bits out of chunky pixel
	uint64_t		B_mask;			///< bitmask for getting the blue bits out of chunky pixel
	uint64_t		A_mask;			///< bitmask for getting the alpha bits out of chunky pixel
} wax_pixelDescription;

//____ wax_gfxFlip ____________________________________________________________

typedef enum
{
	WAX_GFXFLIP_NORMAL = 0,
	WAX_GFXFLIP_X,
	WAX_GFXFLIP_Y,
	WAX_GFXFLIP_ROT90,
	WAX_GFXFLIP_ROT90_FLIP_X,
	WAX_GFXFLIP_ROT90_FLIP_Y,
	WAX_GFXFLIP_ROT180,
	WAX_GFXFLIP_ROT180_FLIP_X,
	WAX_GFXFLIP_ROT180_FLIP_Y,
	WAX_GFXFLIP_ROT270,
	WAX_GFXFLIP_ROT270_FLIP_X,
	WAX_GFXFLIP_ROT270_FLIP_Y,
} wax_gfxFlip;

//____ wax_transform ___________________________________________________________

typedef struct
{
	float	xx, xy;		// Increment of X and Y position in source when increasing destination along X-axis
	float	yx, yy;		// Increment of X and Y position in source when increasing destination along Y-axis
}  wax_transform;

//____ wax_canvasRef ____________________________________________________________

typedef enum
{
	WAX_CANVAS_NONE,
	WAX_CANVAS_DEFAULT,
	WAX_CANVAS_1,
	WAX_CANVAS_2,
	WAX_CANVAS_3,
	WAX_CANVAS_4,
	WAX_CANVAS_5,
	WAX_CANVAS_6,
	WAX_CANVAS_7,
	WAX_CANVAS_8,
	WAX_CANVAS_9,
	WAX_CANVAS_10,
	WAX_CANVAS_11,
	WAX_CANVAS_12,
	WAX_CANVAS_13,
	WAX_CANVAS_14,
	WAX_CANVAS_15,
	WAX_CANVAS_16,
	WAX_CANVAS_17,
	WAX_CANVAS_18,
	WAX_CANVAS_19,
	WAX_CANVAS_20,
	WAX_CANVAS_21,
	WAX_CANVAS_22,
	WAX_CANVAS_23,
	WAX_CANVAS_24,
	WAX_CANVAS_25,
	WAX_CANVAS_26,
	WAX_CANVAS_27,
	WAX_CANVAS_28,
	WAX_CANVAS_29,
	WAX_CANVAS_30,
	WAX_CANVAS_31,
	WAX_CANVAS_32
} wax_canvasRef;

//____ wax_sampleOrigo _____________________________________________________________

typedef enum
{
	WAX_SAMPLEORIGO_TOP = 0,				// Origo at top, increasing values goes down.
	WAX_SAMPLEORIGO_MIDDLE_UP = 1,		// Origo at middle, increasing values goes up.
	WAX_SAMPLEORIGO_MIDDLE_DOWN = 2,		// Origo at middle, increasing values goes down.
	WAX_SAMPLEORIGO_BOTTOM = 3			// Origo at bottom, increasing values goes up.
} wax_sampleOrigo;


#ifdef __cplusplus
}
#endif

#endif
