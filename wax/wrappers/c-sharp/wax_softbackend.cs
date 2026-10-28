using System.Runtime.InteropServices;

namespace Wax;


public class SoftBackend : Backend
{

    //____ Constructor() ______________________________________________________
    public SoftBackend()
    {
        _obj = wax_createSoftBackend();
    }

    //____ DefineCanvas() _____________________________________________________
    public bool DefineCanvas(CanvasRef reference, SoftSurface surface)
    {
        return (wax_defineSoftBackendCanvas(_obj, reference, surface.CHandle()) == 1);
    }

    //____ Static Add Kernel Methods _________________________________________________
    
    public static bool AddDefaultSoftKernels(SoftBackend backend) => wax_addDefaultSoftKernels(backend.CHandle()) == 1;
    public static bool AddBaseSoftKernelsForBGR565LinearCanvas(SoftBackend backend) => wax_addBaseSoftKernelsForBGR565LinearCanvas(backend.CHandle()) == 1;
    public static bool AddExtraSoftKernelsForBGR565LinearCanvas(SoftBackend backend) => wax_addExtraSoftKernelsForBGR565LinearCanvas(backend.CHandle()) == 1;
    public static bool AddBaseSoftKernelsForBGR565sRGBCanvas(SoftBackend backend) => wax_addBaseSoftKernelsForBGR565sRGBCanvas(backend.CHandle()) == 1;
    public static bool AddExtraSoftKernelsForBGR565sRGBCanvas(SoftBackend backend) => wax_addExtraSoftKernelsForBGR565sRGBCanvas(backend.CHandle()) == 1;
    public static bool AddBaseSoftKernelsForRGB555BECanvas(SoftBackend backend) => wax_addBaseSoftKernelsForRGB555BECanvas(backend.CHandle()) == 1;

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createSoftBackend();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_defineSoftBackendCanvas(IntPtr backend, CanvasRef canvas, IntPtr softSurface);



    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_addDefaultSoftKernels(IntPtr backend);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_addBaseSoftKernelsForBGR565LinearCanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_addExtraSoftKernelsForBGR565LinearCanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_addExtraSoftKernelsForBGR565sRGBCanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_addBaseSoftKernelsForRGB555BECanvas( IntPtr backend );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_addBaseSoftKernelsForBGR565sRGBCanvas( IntPtr backend );


}
