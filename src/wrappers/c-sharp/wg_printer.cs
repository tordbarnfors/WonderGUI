using System.Runtime.InteropServices;

namespace WG;


public class Printer : Objekt
{

    //    GfxDevice_p	gfxDevice() const { return m_pGfxDevice;  }
    //    Font_p		font() const { return m_pFont;  }


    //____ Constructor ________________________________________________________
    public Printer()
    {
        _obj = wg_createPrinter();
    }

    //____ SetGfxDevice() _____________________________________________________

    public void SetGfxDevice(GfxDevice device)
    {
        wg_setPrinterGfxDevice(_obj, device.CHandle());
    }

    //____ SetFont() __________________________________________________________

    public void SetFont(Font font)
    {
        wg_setPrinterFont(_obj, font.CHandle());
    }


    //____ SetCursorOrigo() ___________________________________________________

    public void SetCursorOrigo(CoordSPX origo)
    {
        wg_setPrinterOrigo(_obj, origo);
    }

    //____ CursorOrigo() ______________________________________________________

    public CoordSPX CursorOrigo()
    {
        return wg_printerOrigo(_obj);
    }

    //____ SetLineWidth() _____________________________________________________

    public void SetLineWidth(int width)
    {
        wg_setPrinterLineWidth(_obj, width);
    }

    //____ GetLineWidth() _____________________________________________________

    public int GetLineWidth()
    {
        return wg_printerLineWidth(_obj);
    }

    //____ GetLineHeight() ____________________________________________________

    public int GetLineHeight()
    {
        return wg_printerLineHeight(_obj);
    }

    //____ SetTabSize() _______________________________________________________

    public void SetTabSize(int nbWhiteSpace)
    {
        wg_setPrinterTabSize(_obj, nbWhiteSpace);
    }

    //____ GetTabSize() _______________________________________________________

    public int GetTabSize()
    {
        return wg_printerTabSize(_obj);
    }

    //____ ResetCursor() ______________________________________________________

    public void ResetCursor()
    {
        wg_resetPrinterCursor(_obj);
    }

    //____ SetCursor() ________________________________________________________

    public void SetCursor(CoordSPX pos)
    {
        wg_setPrinterCursor(_obj, pos);
    }

    //____ GetCursor() ________________________________________________________

    public CoordSPX GetCursor()
    {
        return wg_printerCursor(_obj);
    }

    //____ Tab() ______________________________________________________________

    public void Tab()
    {
        wg_printTab(_obj);
    }

    //____ CRLF() _____________________________________________________________

    public void CRLF()
    {
        wg_printCRLF(_obj);
    }

    public void CRLF(int maxFontSizeThisLine, int maxFontSizeNextLine)
    {
        wg_printCRFLWithFontSizes(_obj, maxFontSizeThisLine, maxFontSizeNextLine);
    }

    //____ Print() ____________________________________________________________

    public void Print(string text)
    {
        wg_print(_obj, text);
    }

    //____ PrintAligned() _____________________________________________________

    public void PrintAligned(Placement xAlignment, string text)
    {
        wg_printAligned(_obj, xAlignment, text);
    }

    //____ PrintWrapping() ____________________________________________________

    public void PrintWrapping(string text, int wrappedLinesIndent = 0)
    {
        wg_printWrapping(_obj, text, wrappedLinesIndent);
    }

    //____ PrintAt() __________________________________________________________

    public CoordSPX PrintAt(CoordSPX pos, string text)
    {
        return wg_printAt(_obj, pos, text);
    }

    //____ PrintInBox() _______________________________________________________

    public CoordSPX PrintInBox(ref RectSPX box, Placement alignment, string text)
    {
        return wg_printInBox(_obj, box, alignment, text);
    }

    //____ TextSize() _________________________________________________________

    public SizeSPX TextSize(string text)
    {
        return wg_printerTextSize(_obj, text);
    }

    //____ WrappingTextSize() _________________________________________________

    public SizeSPX WrappingTextSize(string text, int wrappedLinesIndent = 0)
    {
        return wg_printerWrappingTextSize(_obj, text, wrappedLinesIndent);
    }



    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createPrinter();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setPrinterGfxDevice(IntPtr printer, IntPtr gfxDevice);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_printerGfxDevice(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setPrinterFont(IntPtr printer, IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_printerFont(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setPrinterOrigo(IntPtr printer, CoordSPX coord);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern CoordSPX wg_printerOrigo(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setPrinterLineWidth(IntPtr printer, int spxWidth);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_printerLineWidth(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_setPrinterTabSize(IntPtr printer, int nbWhiteSpace);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_printerTabSize(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_resetPrinterCursor(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setPrinterCursor(IntPtr printer, CoordSPX pos);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern CoordSPX wg_printerCursor(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_printTab(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_printCRLF(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_printCRFLWithFontSizes(IntPtr printer, int maxFontSizeThisLine, int maxFontSizeNextLine);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_print(IntPtr printer, string text);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_printAligned(IntPtr printer, Placement xAlign, string text);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_printWrapping(IntPtr printer, string text, int wrappedLinesIndent);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern CoordSPX wg_printAt(IntPtr printer, CoordSPX pos, string text);
 
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern CoordSPX wg_printInBox( IntPtr printer, RectSPX box, Placement alignment, string text );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_printerLineHeight( IntPtr printer );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeSPX wg_printerTextSize( IntPtr printer, string text );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeSPX wg_printerWrappingTextSize( IntPtr printer, string text, int wrappedLinesIndent );

}