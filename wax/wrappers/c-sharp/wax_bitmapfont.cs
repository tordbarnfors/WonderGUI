using System.Runtime.InteropServices;

namespace Wax;

public class BitmapFont : Font
{
    //____ Constructor ________________________________________________________
    // IMPORTANT!!! glyphSpec needs to be zero-terminated!!!

    public BitmapFont(Surface surface, in char[] glyphSpec, Font backupFont)
    {
        _obj = wax_createBitmapFont(surface.CHandle(), glyphSpec, backupFont.CHandle());
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr wax_createBitmapFont( IntPtr surface, in char[] pGlyphSpec, IntPtr backupFont );
/*
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr wax_getBitmapFontSurface( IntPtr bitmapFont );
*/
}