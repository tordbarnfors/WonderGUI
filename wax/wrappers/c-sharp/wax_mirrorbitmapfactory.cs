
using System.Runtime.InteropServices;

namespace Wax;

public class MirrorBitmapFactory : BitmapFactory
{

    //____ Constructor ________________________________________________________
    public MirrorBitmapFactory(StreamEncoder encoder, BitmapFactory backingBitmapFactory )
    {
        _obj = wax_createMirrorBitmapFactory(encoder.CHandle(),backingBitmapFactory.CHandle());
    }

    //____ CreateBitmap() ____________________________________________________

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmap(_obj, ref bp);

        return new MirrorBitmap(bitmap);
    }

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint, Blob blob, int pitch)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmapFromBlob(_obj, ref bp, blob.CHandle(), pitch);

        return new MirrorBitmap(bitmap);
    }

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmapFromBitmap(_obj, ref bp, pixels, pixelFormat, pitch, palette, palette.Length);

        return new MirrorBitmap(bitmap);
    }

    public Bitmap CreateBitmap(Bitmap.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Bitmap.C_Blueprint bp;
        Bitmap.ConvertBlueprint(in blueprint, out bp);

        IntPtr bitmap = wax_createBitmapFromRawData(_obj, ref bp, pixels, pixelDescription, pitch, palette, palette.Length);

        return new MirrorBitmap(bitmap);
    }

    //____ SetRememberBitmapsCreated() _______________________________________
    public void SetRememberBitmapsCreated(bool bRemember)
    {
        wax_setRememberMirrorBitmapsCreated(_obj, bRemember ? 1 : 0);
    }

    //____ StreamAllBitmapsAsNew() ___________________________________________
    public void StreamAllBitmapsAsNew()
    {
        wax_streamMirrorBitmapsAsNew(_obj);
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr wax_createMirrorBitmapFactory(IntPtr streamEncoder, IntPtr backingBitmapFactory );
    
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void wax_setRememberMirrorBitmapsCreated(IntPtr mirrorBitmapFactory, int bRemember);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern void wax_streamMirrorBitmapsAsNew(IntPtr mirrorBitmapFactory);

}