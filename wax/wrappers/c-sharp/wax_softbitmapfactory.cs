
using System.Runtime.InteropServices;

namespace Wax;

public class SoftBitmapFactory : BitmapFactory
{

    //____ Constructor ________________________________________________________
    public SoftBitmapFactory()
    {
        _obj = wax_createSoftBitmapFactory();
    }

    //____ CreateBitmap() ____________________________________________________

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmap(_obj, ref bp);

        return new SoftBitmap(bitmap);
    }

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint, Blob blob, int pitch)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmapFromBlob(_obj, ref bp, blob.CHandle(), pitch);

        return new SoftBitmap(bitmap);        
    }

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmapFromBitmap(_obj, ref bp, pixels, pixelFormat, pitch, palette, palette.Length);

        return new SoftBitmap(bitmap);
    }

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmapFromRawData(_obj, ref bp, pixels, pixelDescription, pitch, palette, palette.Length);

        return new SoftBitmap(bitmap);
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr	wax_createSoftBitmapFactory();

}