using System.Runtime.InteropServices;

namespace Wax;


public class SysFont : Font
{
    //____ Constructor ________________________________________________________
 
    public SysFont(BitmapFactory factory, Font backupFont)
    {
        _obj = wax_createSysFont(factory.CHandle(), backupFont.CHandle());
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr wax_createSysFont( IntPtr bitmapFactory, IntPtr backupFont );
/*
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr wax_getBitmapFontBitmap( IntPtr bitmapFont );
*/
}