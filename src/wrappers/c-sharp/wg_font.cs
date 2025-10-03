using System.Runtime.InteropServices;

namespace WG;

public class Glyph
{
    public int      Advance;
    public uint     KerningIndex;

    public IntPtr   FontRef;

    public Surface? Surface;

	public RectSPX	Rect;
	int     	    BearingX;
	int		        BearingY;
}


public class Font : Objekt
{

    public Font()
    {

    }

    protected struct wg_glyph
	{
		public int      advance;
		public uint     kerningIndex;
		public IntPtr	fontRef;

		// These below are not updated by getGlyphWithoutBitmap().

		public IntPtr	surface;
		public RectSPX  rect;
		public int      bearingX;
		public int      bearingY;
	}


    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_setFontSize(IntPtr font, int size);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_fontSize(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_getGlyphWithoutBitmap(IntPtr font, UInt16 chr, ref wg_glyph glyph);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_getGlyphWithBitmap(IntPtr font, UInt16 chr, ref wg_glyph pGlyph);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_getKerning(IntPtr font, ref Glyph leftGlyph, ref Glyph rightGlyph);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_lineGap(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_whitespaceAdvance(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_maxAdvance(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_maxAscend(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_maxDescend(IntPtr font);
}