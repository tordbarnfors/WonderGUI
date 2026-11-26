using System.Runtime.InteropServices;

namespace Wax;

public class SoftBitmap : Bitmap
{
    //____ Constructor ________________________________________________________

	internal SoftBitmap(IntPtr c_obj)
    {
        _obj = c_obj;
    }

	public SoftBitmap(ref Blueprint blueprint)
	{
		C_Blueprint bp;
		Bitmap.ConvertBlueprint(in blueprint, out bp);

		_obj = wax_createSoftBitmap(ref bp);
	}

    //____ SoftBitmap() ______________________________________________________

    public SoftBitmap(Bitmap.Blueprint blueprint, Blob blob, int pitch)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        _obj = wax_createSoftBitmapFromBlob(ref bp, blob.CHandle(), pitch);
    }

    public SoftBitmap(Bitmap.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        _obj = wax_createSoftBitmapFromBitmap(ref bp, pixels, pixelFormat, pitch, palette, palette.Length);
    }

    public SoftBitmap(Bitmap.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        _obj = wax_createSoftBitmapFromRawData(ref bp, pixels, pixelDescription, pitch, palette, palette.Length);
    }

    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_createSoftBitmap(ref C_Blueprint blueprint);
	
 	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
   	private static extern IntPtr wax_createSoftBitmapFromBlob(ref C_Blueprint blueprint, IntPtr blob, int pitch);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createSoftBitmapFromBitmap(ref C_Blueprint blueprint, Byte[] pixels,
																PixelFormat pixelFormat, int pitch, Color8[] palette, int paletteSize );


	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createSoftBitmapFromRawData(ref C_Blueprint blueprint, Byte[] pixels,
																PixelDescription pixelDescription, int pitch, Color8[] palette, int paletteSize );

}
