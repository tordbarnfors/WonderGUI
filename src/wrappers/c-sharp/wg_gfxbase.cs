using System.Runtime.InteropServices;

namespace WG;

public static class GfxBase
{
    private const string NativeLib = "libstreamgendll";

    //____ Init() _________________________________________________________

    static public bool Init()
    {
        int ret = wg_initGfxBase();

        return (ret == 1);
    }

    //____ Exit() _________________________________________________________

    static public bool Exit()
    {
        int ret = wg_exitGfxBase();

        return (ret == 1);
    }

    //____ IsInitialized() ________________________________________________

    static public bool IsInitialized()
    {
        int ret = wg_initGfxBase();

        return (ret == 1);
    }

    //____ SetDefaultToSRGB() _________________________________________________

    static public void SetDefaultToSRGB(bool bSRGB)
    {
        wg_setDefaultToSRGB(bSRGB ? 1 : 0);
    }

    //____ GetDefaultToSRGB() _________________________________________________
    static public bool GetDefaultToSRGB()
    {
        return (wg_defaultToSRGB() == 1);
    }


    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_initGfxBase();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_exitGfxBase();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_isGfxBaseInitialized();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setDefaultToSRGB( int bSRGB );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_defaultToSRGB();


}

