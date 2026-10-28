using System.Runtime.InteropServices;

namespace Wax;


public class StreamBackend : Backend
{

    //____ Constructor() ______________________________________________________
    public StreamBackend(StreamEncoder encoder, int maxEdges = 15)
    {
        _obj = wax_createStreamBackend(encoder.CHandle(), maxEdges);
    }

    //____ DefineCanvas() _____________________________________________________
    public bool DefineCanvas(CanvasRef reference, StreamSurface surface)
    {
        return (wax_defineStreamBackendCanvasWithSurface(_obj, reference, surface.CHandle()) == 1);
    }

    public bool DefineCanvas(CanvasRef reference, SizeI pixelSize, PixelFormat pixelFormat, int scale = 64,
                             ColorSpace colorSpace = ColorSpace.sRGB, ByteOrder byteOrder = ByteOrder.Native)
    {
        return (wax_defineStreamBackendCanvas(_obj, reference, pixelSize, pixelFormat, scale, colorSpace, byteOrder) == 1);
    }

    //____ EncodeCanvasList() _________________________________________________

    public void EncodeCanvasList() => wax_encodeCanvasList(_obj);

    //____ EncodeTick() _______________________________________________________

    public void EncodeTick(int microsecPassed) => wax_encodeTick(_obj, microsecPassed);

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createStreamBackend(IntPtr streamEncoder, int maxEdges);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_defineStreamBackendCanvasWithSurface(IntPtr streamBackend, CanvasRef canvas, IntPtr surface);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_defineStreamBackendCanvas(IntPtr streamBackend, CanvasRef canvas, SizeI pixelSize, PixelFormat pixelFormat, int scale,
                                                           ColorSpace colorSpace, ByteOrder byteOrder);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_encodeCanvasList(IntPtr streamBackend);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_encodeTick(IntPtr streamBackend, int microsecPassed );

}
