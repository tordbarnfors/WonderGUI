using System.Runtime.InteropServices;

namespace WG;


public class SysFont : Font
{
    //____ Constructor ________________________________________________________
 
    public SysFont(SurfaceFactory factory, Font backupFont)
    {
        _obj = wg_createSysFont(factory.CHandle(), backupFont.CHandle());
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr wg_createSysFont( IntPtr surfaceFactory, IntPtr backupFont );
/*
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr wg_getBitmapFontSurface( IntPtr bitmapFont );
*/
}