using System.Runtime.InteropServices;

namespace Wax;


public class Printer : Objekt
{

    //    Device_p	device() const { return m_pGfxDevice;  }
    //    Font_p		font() const { return m_pFont;  }


    //____ Constructor ________________________________________________________
    public Printer()
    {
        _obj = wax_createPrinter();
    }

    //____ SetDevice() _____________________________________________________

    public void SetDevice(Device device)
    {
        wax_setPrinterDevice(_obj, device.CHandle());
    }

    //____ SetFont() __________________________________________________________

    public void SetFont(Font font)
    {
        wax_setPrinterFont(_obj, font.CHandle());
    }


    //____ SetCursorOrigo() ___________________________________________________

    public void SetCursorOrigo(CoordSPX origo)
    {
        wax_setPrinterOrigo(_obj, origo);
    }

    //____ CursorOrigo() ______________________________________________________

    public CoordSPX CursorOrigo()
    {
        return wax_printerOrigo(_obj);
    }

    //____ SetLineWidth() _____________________________________________________

    public void SetLineWidth(int width)
    {
        wax_setPrinterLineWidth(_obj, width);
    }

    //____ GetLineWidth() _____________________________________________________

    public int GetLineWidth()
    {
        return wax_printerLineWidth(_obj);
    }

    //____ GetLineHeight() ____________________________________________________

    public int GetLineHeight()
    {
        return wax_printerLineHeight(_obj);
    }

    //____ SetTabSize() _______________________________________________________

    public void SetTabSize(int nbWhiteSpace)
    {
        wax_setPrinterTabSize(_obj, nbWhiteSpace);
    }

    //____ GetTabSize() _______________________________________________________

    public int GetTabSize()
    {
        return wax_printerTabSize(_obj);
    }

    //____ ResetCursor() ______________________________________________________

    public void ResetCursor()
    {
        wax_resetPrinterCursor(_obj);
    }

    //____ SetCursor() ________________________________________________________

    public void SetCursor(CoordSPX pos)
    {
        wax_setPrinterCursor(_obj, pos);
    }

    //____ GetCursor() ________________________________________________________

    public CoordSPX GetCursor()
    {
        return wax_printerCursor(_obj);
    }

    //____ Tab() ______________________________________________________________

    public void Tab()
    {
        wax_printTab(_obj);
    }

    //____ CRLF() _____________________________________________________________

    public void CRLF()
    {
        wax_printCRLF(_obj);
    }

    public void CRLF(int maxFontSizeThisLine, int maxFontSizeNextLine)
    {
        wax_printCRFLWithFontSizes(_obj, maxFontSizeThisLine, maxFontSizeNextLine);
    }

    //____ Print() ____________________________________________________________

    public void Print(string text)
    {
        wax_print(_obj, text);
    }

    //____ PrintAligned() _____________________________________________________

    public void PrintAligned(Placement xAlignment, string text)
    {
        wax_printAligned(_obj, xAlignment, text);
    }

    //____ PrintWrapping() ____________________________________________________

    public void PrintWrapping(string text, int wrappedLinesIndent = 0)
    {
        wax_printWrapping(_obj, text, wrappedLinesIndent);
    }

    //____ PrintAt() __________________________________________________________

    public CoordSPX PrintAt(CoordSPX pos, string text)
    {
        return wax_printAt(_obj, pos, text);
    }

    //____ PrintInBox() _______________________________________________________

    public CoordSPX PrintInBox(ref RectSPX box, Placement alignment, string text)
    {
        return wax_printInBox(_obj, box, alignment, text);
    }

    //____ TextSize() _________________________________________________________

    public SizeSPX TextSize(string text)
    {
        return wax_printerTextSize(_obj, text);
    }

    //____ WrappingTextSize() _________________________________________________

    public SizeSPX WrappingTextSize(string text, int wrappedLinesIndent = 0)
    {
        return wax_printerWrappingTextSize(_obj, text, wrappedLinesIndent);
    }



    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createPrinter();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setPrinterDevice(IntPtr printer, IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_printerDevice(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setPrinterFont(IntPtr printer, IntPtr font);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_printerFont(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setPrinterOrigo(IntPtr printer, CoordSPX coord);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern CoordSPX wax_printerOrigo(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setPrinterLineWidth(IntPtr printer, int spxWidth);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_printerLineWidth(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_setPrinterTabSize(IntPtr printer, int nbWhiteSpace);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_printerTabSize(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_resetPrinterCursor(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setPrinterCursor(IntPtr printer, CoordSPX pos);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern CoordSPX wax_printerCursor(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_printTab(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_printCRLF(IntPtr printer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_printCRFLWithFontSizes(IntPtr printer, int maxFontSizeThisLine, int maxFontSizeNextLine);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_print(IntPtr printer, string text);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_printAligned(IntPtr printer, Placement xAlign, string text);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_printWrapping(IntPtr printer, string text, int wrappedLinesIndent);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern CoordSPX wax_printAt(IntPtr printer, CoordSPX pos, string text);
 
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern CoordSPX wax_printInBox( IntPtr printer, RectSPX box, Placement alignment, string text );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_printerLineHeight( IntPtr printer );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeSPX wax_printerTextSize( IntPtr printer, string text );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeSPX wax_printerWrappingTextSize( IntPtr printer, string text, int wrappedLinesIndent );

}