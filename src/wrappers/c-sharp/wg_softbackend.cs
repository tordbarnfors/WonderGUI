using System.Runtime.InteropServices;

namespace WG;


public class SoftBackend : GfxBackend
{

    //____ Constructor() ______________________________________________________
    public SoftBackend()
    {
        _obj = wg_createSoftBackend();
    }

    //____ DefineCanvas() _____________________________________________________
    public bool DefineCanvas(CanvasRef reference, SoftSurface surface)
    {
        return (wg_defineSoftBackendCanvas(_obj, reference, surface.CHandle()) == 1);
    }

    //____ Static Add Kernel Methods _________________________________________________
    
    public static bool AddDefaultSoftKernels(SoftBackend backend) => wg_addDefaultSoftKernels(backend.CHandle()) == 1;
    public static bool AddBaseSoftKernelsForBGR565LinearCanvas(SoftBackend backend) => wg_addBaseSoftKernelsForBGR565LinearCanvas(backend.CHandle()) == 1;
    public static bool AddExtraSoftKernelsForBGR565LinearCanvas(SoftBackend backend) => wg_addExtraSoftKernelsForBGR565LinearCanvas(backend.CHandle()) == 1;
    public static bool AddBaseSoftKernelsForBGR565sRGBCanvas(SoftBackend backend) => wg_addBaseSoftKernelsForBGR565sRGBCanvas(backend.CHandle()) == 1;
    public static bool AddExtraSoftKernelsForBGR565sRGBCanvas(SoftBackend backend) => wg_addExtraSoftKernelsForBGR565sRGBCanvas(backend.CHandle()) == 1;
    public static bool AddBaseSoftKernelsForRGB555BECanvas(SoftBackend backend) => wg_addBaseSoftKernelsForRGB555BECanvas(backend.CHandle()) == 1;

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createSoftBackend();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_defineSoftBackendCanvas(IntPtr backend, CanvasRef canvas, IntPtr softSurface);



    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_addDefaultSoftKernels(IntPtr backend);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_addBaseSoftKernelsForBGR565LinearCanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_addExtraSoftKernelsForBGR565LinearCanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_addExtraSoftKernelsForBGR565sRGBCanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_addBaseSoftKernelsForRGB555BECanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_addBaseSoftKernelsForBGR565sRGBCanvas( IntPtr backend );


}
