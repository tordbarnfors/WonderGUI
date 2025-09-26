namespace WG;


//____ Direction ____________________________________________________________

public enum Direction
{
	Up,
	Right,
	Down,
	Left
}

//____ SampleMethod ____________________________________________________________

public enum SampleMethod
{
	Nearest,
	Bilinear,
	Undefined           // Default to Bilinear if it is accelerated, otherwise Nearest.
}

//____ PixelFormat _____________________________________________________________

public enum PixelFormat
{
	Undefined,          ///< Pixelformat is undefined.
	BGR_8,              ///< One byte of blue, green and red in exactly that order in memory.
	BGR_8_sRGB,         ///< One byte of blue, green and red in exactly that order in memory.
	BGR_8_linear,       ///< One byte of blue, green and red in exactly that order in memory.

	BGRX_8,             ///< One byte of blue, green, red and padding in exactly that order in memory.
	BGRX_8_sRGB,        ///< One byte of blue, green, red and padding in exactly that order in memory.
	BGRX_8_linear,      ///< One byte of blue, green, red and padding in exactly that order in memory.

	BGRA_8,             ///< One byte of blue, green, red and alpha in exactly that order in memory.
	BGRA_8_sRGB,        ///< One byte of blue, green, red and alpha in exactly that order in memory.
	BGRA_8_linear,      ///< One byte of blue, green, red and alpha in exactly that order in memory.

	Index_8,            ///< 8 bits of index into the palette.
	Index_8_sRGB,       ///< 8 bits of index into an sRGB palette.
	Index_8_linear,     ///< 8 bits of index into a linear palette.

	Index_16,           ///< 16 bits of index into the palette.
	Index_16_sRGB,      ///< 16 bits of index into an sRGB palette.
	Index_16_linear,    ///< 16 bits of index into a linear palette.

	Alpha_8,            ///< 8 bits of alpha only.

	BGRA_4_linear,      ///< 4 bits each of blue, green, red and alpha in exactly that order in memory.

	BGR_565,
	BGR_565_sRGB,       ///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.
	BGR_565_linear,     ///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.

	RGB_565_bigendian,  ///< 3 high bits of green, 5 bits of blue, 5 bits of red and 3 low bits of green in exactly that order in memory. Linear and big endian.
	RGB_555_bigendian,  /// Same layout as RGB_565_bigendian, but lowest bit of green is always cleared. Avoids accidental tinting towards green of dark colors.

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
}

	//____ CanvasRef ____________________________________________________________

public enum CanvasRef
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
}
