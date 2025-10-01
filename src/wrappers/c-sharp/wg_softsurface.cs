using System.Runtime.InteropServices;

namespace WG;

public class SoftSurface : Surface
{
	internal SoftSurface(IntPtr c_obj)
	{
		_obj = c_obj;
	}

	public SoftSurface(ref Blueprint blueprint)
	{
		C_Blueprint bp;
		Surface.ConvertBlueprint(in blueprint, out bp);

		_obj = wg_createSoftSurface(ref bp);
	}

	public SoftSurface(Surface.Blueprint blueprint, Blob blob, int pitch)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wg_createSoftSurfaceFromBlob(ref bp, blob.CHandle(), pitch);
    }

    public SoftSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wg_createSoftSurfaceFromBitmap(ref bp, pixels, pixelFormat, pitch, palette, palette.Length);
    }

    public SoftSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wg_createSoftSurfaceFromRawData(ref bp, pixels, pixelDescription, pitch, palette, palette.Length);
    }

    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createSoftSurface(ref C_Blueprint blueprint);
	
 	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
   	private static extern IntPtr wg_createSoftSurfaceFromBlob(ref C_Blueprint blueprint, IntPtr blob, int pitch);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createSoftSurfaceFromBitmap(ref C_Blueprint blueprint, Byte[] pixels,
																PixelFormat pixelFormat, int pitch, Color8[] palette, int paletteSize );


	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createSoftSurfaceFromRawData(ref C_Blueprint blueprint, Byte[] pixels,
																PixelDescription pixelDescription, int pitch, Color8[] palette, int paletteSize );

}
