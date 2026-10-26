using System.Runtime.InteropServices;

namespace Wax;


public class BitmapCache : Objekt
{
    //____ Constructor ________________________________________________________
    public BitmapCache(int maxBytes, SurfaceFactory factory)
    {
        _obj = wax_createBitmapCache(maxBytes, factory.CHandle());
    }

    internal BitmapCache(IntPtr c_obj)
    {
        _obj = c_obj;
    }


    //____ SetLimit() _________________________________________________________
    public void SetLimit(int maxBytes)
    {
        wax_setCacheLimit(_obj, maxBytes);
    }

    //____ Limit() ____________________________________________________________
    public int Limit()
    {
        return wax_cacheLimit(_obj);
    }

    //____ Size() _____________________________________________________________
    public int Size()
    {
        return wax_cacheSize(_obj);
    }

    //____ Clear() ____________________________________________________________
    public void Clear()
    {
        wax_clearCache(_obj);
    }


    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createBitmapCache( int maxSize, IntPtr surfaceFactory );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setCacheLimit(IntPtr bitmapCache, int maxBytes);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_cacheLimit(IntPtr bitmapCache);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_cacheSize(IntPtr bitmapCache);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_clearCache(IntPtr bitmapCache);    
}