using System.Runtime.InteropServices;

namespace WG;

public class BitmapFont : Font
{
    //____ Constructor ________________________________________________________
    // IMPORTANT!!! glyphSpec needs to be zero-terminated!!!

    public BitmapFont(Surface surface, in char[] glyphSpec, Font backupFont)
    {
        _obj = wg_createBitmapFont(surface.CHandle(), glyphSpec, backupFont.CHandle());
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr wg_createBitmapFont( IntPtr surface, in char[] pGlyphSpec, IntPtr backupFont );
/*
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr wg_getBitmapFontSurface( IntPtr bitmapFont );
*/
}