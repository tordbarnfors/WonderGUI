using System.Runtime.InteropServices;

namespace WG;

public class SurfaceFactory : Objekt
{
    protected SurfaceFactory() { }

    //____ MaxSize() __________________________________________________________

    public SizeI MaxSize()
    {
        return wg_maxSurfaceSize(_obj);
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern SizeI wg_maxSurfaceSize(IntPtr surfaceFactory);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wg_createSurface(IntPtr factory, ref Surface.C_Blueprint blueprint);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wg_createSurfaceFromBlob(IntPtr factory, ref Surface.C_Blueprint blueprint, IntPtr blob, int pitch);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wg_createSurfaceFromBitmap(IntPtr factory, ref Surface.C_Blueprint blueprint, Byte[] pixels,
									                        PixelFormat pixelFormat, int pitch, Color8[] palette, int paletteSize);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wg_createSurfaceFromRawData(IntPtr factory, ref Surface.C_Blueprint blueprint, Byte[] pixels,
										                    PixelDescription pixelDescription, int pitch, Color8[] palette, int paletteSize);


}
