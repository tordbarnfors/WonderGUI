using System.Runtime.InteropServices;

namespace WG;


public class SoftBackend : GfxBackend
{
	private const string NativeLib = "libstreamgendll";

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

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createSoftBackend();

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_defineSoftBackendCanvas( IntPtr backend, CanvasRef canvas, IntPtr softSurface );
}
