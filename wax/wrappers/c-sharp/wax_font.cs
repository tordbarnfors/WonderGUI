using System.Runtime.InteropServices;

namespace Wax;

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

    public void SetSize(int size)
    {
	    wax_setFontSize(_obj, size);
    }

    [StructLayout(LayoutKind.Sequential)]
    protected struct wax_glyph
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
    private static extern int wax_setFontSize(IntPtr font, int size);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_fontSize(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_getGlyphWithoutBitmap(IntPtr font, UInt32 chr, ref wax_glyph glyph);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_getGlyphWithBitmap(IntPtr font, UInt32 chr, ref wax_glyph pGlyph);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_getKerning(IntPtr font, ref Glyph leftGlyph, ref Glyph rightGlyph);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_lineGap(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_whitespaceAdvance(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_maxAdvance(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_maxAscend(IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_maxDescend(IntPtr font);
}