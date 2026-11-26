using System.Runtime.InteropServices;

namespace Wax;

public class BitmapFactory : Objekt
{
    protected BitmapFactory() { }

    //____ MaxSize() __________________________________________________________

    public SizeI MaxSize()
    {
        return wax_maxBitmapSize(_obj);
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern SizeI wax_maxBitmapSize(IntPtr bitmapFactory);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wax_createBitmap(IntPtr factory, ref Bitmap.C_Blueprint blueprint);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wax_createBitmapFromBlob(IntPtr factory, ref Bitmap.C_Blueprint blueprint, IntPtr blob, int pitch);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wax_createBitmapFromBitmap(IntPtr factory, ref Bitmap.C_Blueprint blueprint, Byte[] pixels,
									                        PixelFormat pixelFormat, int pitch, Color8[] palette, int paletteSize);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern IntPtr wax_createBitmapFromRawData(IntPtr factory, ref Bitmap.C_Blueprint blueprint, Byte[] pixels,
										                    PixelDescription pixelDescription, int pitch, Color8[] palette, int paletteSize);


}
