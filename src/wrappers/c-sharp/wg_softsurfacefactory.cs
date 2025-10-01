
using System.Runtime.InteropServices;

namespace WG;

public class SoftSurfaceFactory : SurfaceFactory
{

    //____ Constructor ________________________________________________________
    public SoftSurfaceFactory()
    {
        _obj = wg_createSoftSurfaceFactory();
    }

    //____ CreateSurface() ____________________________________________________

    public Surface CreateSurface(Surface.Blueprint blueprint)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurface(_obj, ref bp);

        return new SoftSurface(surf);
    }

    public Surface CreateSurface(Surface.Blueprint blueprint, Blob blob, int pitch)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurfaceFromBlob(_obj, ref bp, blob.CHandle(), pitch);

        return new SoftSurface(surf);        
    }

    public Surface CreateSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurfaceFromBitmap(_obj, ref bp, pixels, pixelFormat, pitch, palette, palette.Length);

        return new SoftSurface(surf);
    }

    public Surface CreateSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurfaceFromRawData(_obj, ref bp, pixels, pixelDescription, pitch, palette, palette.Length);

        return new SoftSurface(surf);
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr	wg_createSoftSurfaceFactory();

}