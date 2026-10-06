using System.Runtime.InteropServices;

namespace WG;


//____ BlendMode ____________________________________________________________


public enum BlendMode
{
	Undefined,          ///< Blitting: Defaults to Blend
						///< Color Blending: Defaults to ignore
						///< This value is used internally to distinguish undefined values from an explicitly set ignore,
	Ignore,             ///< Blitting: No blitting performed.
						///< Color Blending: DstRGBA = DstRGBA
	Replace,            ///< Blitting: Completely opaque blitting, ignoring alpha of source and tint-color.
						///< Color Blending: DstRGBA = SrcRGBA
	Blend,              ///< Blitting: Normal mode, alpha of source and tint-color is taken into account.
						///< Color Blending: DstA = SrcA, DstRGB = SrcRGB + ((TintRGB-SrcRGB)*TintA/255)
	Add,                ///< Blitting: RGB Additive, alpha of source and tint-color is taken into account.
						///< Color Blending: DstRGBA = SrcRGBA + TintRGBA
	Subtract,           ///< Blitting: RGB Subtractive, alpha is ignored.
						///< Color Blending: DstRGBA = SrcRGBA - TintRGBA
	Multiply,           ///< Blitting: RGB Multiply, alpha is ignored.
						///< Color Blending: DstRGB = SrcRGBA * TintRGBA/255
	Invert,             ///< Blitting: Inverts destination RGB values where alpha of source is non-zero. Ignores RBG components. Uses alpha of tint-color.
						///< Color Blending: DstA = SrcA, DstRGB = ((255 - SrcRGB)*TintA + SrcRGB*(255-TintA))/255
	Min,                ///< Blitting: Minimum value of each RGB component, alpha is ignored.
						///< Color Blending: DstRGBA = min(SrcRGBA,DstRGBA)
	Max,                ///< Blitting: Maximum value of each RGB component, alpha is ignored.
						///< Color Blending: DstRGBA = max(SrcRGBA,DstRGBA)
	Morph,              ///< Blitting: Transition RGBA into source by morph factor.
						///< Color Blending: A 50% mix of the two colors.
	BlendFixedColor     ///< Blitting: Blend source against fixed color and replace destination with result.
						///< Color Blending: Same as Blend
}



//____ Alignment _____________________________________________________________

public enum Alignment
{
	Begin,
	Center,
	End,
	Justify
}


//____ Placement _____________________________________________________________

public enum Placement
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
}


//____ Direction ____________________________________________________________

public enum Direction
{
	Up,
	Right,
	Down,
	Left
}

//____ Axis __________________________________________________________

public enum Axis
{
	Undefined,
	X,
	Y
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
	// Channels are named in register order, starting with the most significant bits.
	// Byte order and color space are separate properties.

	Undefined,          ///< Pixelformat is undefined.
	XRGB_8,             ///< 8 bits each of padding, red, green and blue.
	ARGB_8,             ///< 8 bits each of alpha, red, green and blue.
	Index_8,            ///< 8 bits of index into the palette.
	Index_16,           ///< 16 bits of index into the palette.
	Alpha_8,            ///< 8 bits of alpha only.
	RGB_565,            ///< 5 bits of red, 6 bits of green and 5 bits of blue.
	BGR_565,            ///< 5 bits of blue, 6 bits of green and 5 bits of red.

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

	XRGB_16,            ///< 16 bits each of padding, red, green and blue.
	ARGB_16             ///< 16 bits each of alpha, red, green and blue.
}

//____ PixelType _________________________________________________________

public enum PixelType      //. autoExtras
{
	Chunky,                     ///< Normal pixel. All bits for a pixel are packed into same sequence of bytes.
	Index,                      ///< Pixels are color indexes into a palette.
	Bitplanes                   ///< Pixels are color indexes into a palette, stored in 16-bit bitplanes. Starting with lowest bitplane.
}

//____ ColorSpace ________________________________________________________

public enum ColorSpace
{
	Undefined,                  ///< Only for blueprints, gives the default (sRGB).
	Linear,
	sRGB
}

//____ ByteOrder ________________________________________________________

public enum ByteOrder
{
	Native,                     ///< Byte order of the system.
	LittleEndian,
	BigEndian
}

//____ PixelDescription _________________________________________________

[StructLayout(LayoutKind.Sequential)]
public struct PixelDescription
{
	public PixelDescription() { }

	public int bits = 0;           ///< Number of bits for the pixel, includes any non-used padding bits.
	public PixelType type = PixelType.Chunky;
	public byte bigEndian = BitConverter.IsLittleEndian ? (byte)0 : (byte)1;	///< Byte order of the pixels (of the 16-bit words for bitplanes).

	public UInt64 R_mask = 0;          ///< bitmask for getting the red bits out of chunky pixel
	public UInt64 G_mask = 0;          ///< bitmask for getting the green bits out of chunky pixel
	public UInt64 B_mask = 0;          ///< bitmask for getting the blue bits out of chunky pixel
	public UInt64 A_mask = 0;          ///< bitmask for getting the alpha bits out of chunky pixel
}


//____ GfxFlip ____________________________________________________________

public enum GfxFlip
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
