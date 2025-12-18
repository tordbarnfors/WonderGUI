using System.Runtime.InteropServices;

namespace WG;


public class StreamBackend : GfxBackend
{

    //____ Constructor() ______________________________________________________
    public StreamBackend(StreamEncoder encoder, int maxEdges = 15)
    {
        _obj = wg_createStreamBackend(encoder.CHandle(), maxEdges);
    }

    //____ DefineCanvas() _____________________________________________________
    public bool DefineCanvas(CanvasRef reference, StreamSurface surface)
    {
        return (wg_defineStreamBackendCanvasWithSurface(_obj, reference, surface.CHandle()) == 1);
    }

    public bool DefineCanvas(CanvasRef reference, SizeI pixelSize, PixelFormat pixelFormat, int scale = 64)
    {
        return (wg_defineStreamBackendCanvas(_obj, reference, pixelSize, pixelFormat, scale) == 1);
    }

    //____ EncodeCanvasList() _________________________________________________

    public void EncodeCanvasList() => wg_encodeCanvasList(_obj);

    //____ EncodeTick() _______________________________________________________

    public void EncodeTick(int microsecPassed) => wg_encodeTick(_obj, microsecPassed);

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createStreamBackend(IntPtr streamEncoder, int maxEdges);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_defineStreamBackendCanvasWithSurface(IntPtr streamBackend, CanvasRef canvas, IntPtr surface);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_defineStreamBackendCanvas(IntPtr streamBackend, CanvasRef canvas, SizeI pixelSize, PixelFormat pixelFormat, int scale);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_encodeCanvasList(IntPtr streamBackend);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_encodeTick(IntPtr streamBackend, int microsecPassed );

}
