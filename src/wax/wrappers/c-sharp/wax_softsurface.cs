using System.Runtime.InteropServices;

namespace Wax;

public class SoftSurface : Surface
{
    //____ Constructor ________________________________________________________

	internal SoftSurface(IntPtr c_obj)
    {
        _obj = c_obj;
    }

	public SoftSurface(ref Blueprint blueprint)
	{
		C_Blueprint bp;
		Surface.ConvertBlueprint(in blueprint, out bp);

		_obj = wax_createSoftSurface(ref bp);
	}

    //____ SoftSurface() ______________________________________________________

    public SoftSurface(Surface.Blueprint blueprint, Blob blob, int pitch)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wax_createSoftSurfaceFromBlob(ref bp, blob.CHandle(), pitch);
    }

    public SoftSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wax_createSoftSurfaceFromBitmap(ref bp, pixels, pixelFormat, pitch, palette, palette.Length);
    }

    public SoftSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wax_createSoftSurfaceFromRawData(ref bp, pixels, pixelDescription, pitch, palette, palette.Length);
    }

    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_createSoftSurface(ref C_Blueprint blueprint);
	
 	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
   	private static extern IntPtr wax_createSoftSurfaceFromBlob(ref C_Blueprint blueprint, IntPtr blob, int pitch);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createSoftSurfaceFromBitmap(ref C_Blueprint blueprint, Byte[] pixels,
																PixelFormat pixelFormat, int pitch, Color8[] palette, int paletteSize );


	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createSoftSurfaceFromRawData(ref C_Blueprint blueprint, Byte[] pixels,
																PixelDescription pixelDescription, int pitch, Color8[] palette, int paletteSize );

}
