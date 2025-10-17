
using System.Runtime.InteropServices;

namespace WG;

public class MirrorSurfaceFactory : SurfaceFactory
{

    //____ Constructor ________________________________________________________
    public MirrorSurfaceFactory(StreamEncoder encoder, SurfaceFactory backingSurfaceFactory )
    {
        _obj = wg_createMirrorSurfaceFactory(encoder.CHandle(),backingSurfaceFactory.CHandle());
    }

    //____ CreateSurface() ____________________________________________________

    public Surface CreateSurface(Surface.Blueprint blueprint)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurface(_obj, ref bp);

        return new MirrorSurface(surf);
    }

    public Surface CreateSurface(Surface.Blueprint blueprint, Blob blob, int pitch)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurfaceFromBlob(_obj, ref bp, blob.CHandle(), pitch);

        return new MirrorSurface(surf);
    }

    public Surface CreateSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurfaceFromBitmap(_obj, ref bp, pixels, pixelFormat, pitch, palette, palette.Length);

        return new MirrorSurface(surf);
    }

    public Surface CreateSurface(Surface.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        IntPtr surf = wg_createSurfaceFromRawData(_obj, ref bp, pixels, pixelDescription, pitch, palette, palette.Length);

        return new MirrorSurface(surf);
    }

    //____ SetRememberSurfacesCreated() _______________________________________
    public void SetRememberSurfacesCreated(bool bRemember)
    {
        wg_setRememberMirrorSurfacesCreated(_obj, bRemember ? 1 : 0);
    }

    //____ StreamAllSurfacesAsNew() ___________________________________________
    public void StreamAllSurfacesAsNew()
    {
        wg_streamMirrorSurfacesAsNew(_obj);
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr wg_createMirrorSurfaceFactory(IntPtr streamEncoder, IntPtr backingSurfaceFactory );
    
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void wg_setRememberMirrorSurfacesCreated(IntPtr mirrorSurfaceFactory, int bRemember);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	protected static extern void wg_streamMirrorSurfacesAsNew(IntPtr mirrorSurfaceFactory);

}