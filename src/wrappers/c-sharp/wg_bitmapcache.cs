using System.Runtime.InteropServices;

namespace WG;


public class BitmapCache : Objekt
{
    //____ Constructor ________________________________________________________
    public BitmapCache(int maxBytes, SurfaceFactory factory)
    {
        _obj = wg_createBitmapCache(maxBytes, factory.CHandle());
    }

    internal BitmapCache(IntPtr c_obj)
    {
        _obj = c_obj;
    }


    //____ SetLimit() _________________________________________________________
    public void SetLimit(int maxBytes)
    {
        wg_setCacheLimit(_obj, maxBytes);
    }

    //____ Limit() ____________________________________________________________
    public int Limit()
    {
        return wg_cacheLimit(_obj);
    }

    //____ Size() _____________________________________________________________
    public int Size()
    {
        return wg_cacheSize(_obj);
    }

    //____ Clear() ____________________________________________________________
    public void Clear()
    {
        wg_clearCache(_obj);
    }


    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createBitmapCache( int maxSize, IntPtr surfaceFactory );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setCacheLimit(IntPtr bitmapCache, int maxBytes);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_cacheLimit(IntPtr bitmapCache);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_cacheSize(IntPtr bitmapCache);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_clearCache(IntPtr bitmapCache);    
}