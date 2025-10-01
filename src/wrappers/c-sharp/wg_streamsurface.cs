using System.Reflection.Metadata.Ecma335;
using System.Runtime.InteropServices;

namespace WG;

public class StreamSurface : Surface
{
	internal StreamSurface(IntPtr c_obj)
	{
		_obj = c_obj;
	}

	public StreamSurface(StreamEncoder encoder, ref Blueprint blueprint)
	{
		C_Blueprint bp;
		Surface.ConvertBlueprint(in blueprint, out bp);

		_obj = wg_createStreamSurface(encoder.CHandle(), ref bp);
	}

	public StreamSurface(StreamEncoder encoder, Surface.Blueprint blueprint, Blob blob, int pitch)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wg_createStreamSurfaceFromBlob(encoder.CHandle(), ref bp, blob.CHandle(), pitch);
    }

    public StreamSurface(StreamEncoder encoder, Surface.Blueprint blueprint, Byte[] pixels, PixelFormat pixelFormat, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wg_createStreamSurfaceFromBitmap(encoder.CHandle(), ref bp, pixels, pixelFormat, pitch, palette, palette.Length);
    }

    public StreamSurface(StreamEncoder encoder, Surface.Blueprint blueprint, Byte[] pixels, PixelDescription pixelDescription, int pitch, Color8[] palette)
    {
        Surface.C_Blueprint bp;
        Surface.ConvertBlueprint(in blueprint, out bp);

        _obj = wg_createStreamSurfaceFromRawData(encoder.CHandle(), ref bp, pixels, pixelDescription, pitch, palette, palette.Length);
    }

    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createStreamSurface(IntPtr streamEncoder, ref C_Blueprint blueprint);
	
 	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
   	private static extern IntPtr wg_createStreamSurfaceFromBlob(IntPtr streamEncoder, ref C_Blueprint blueprint, IntPtr blob, int pitch);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createStreamSurfaceFromBitmap(IntPtr streamEncoder, ref C_Blueprint blueprint, Byte[] pixels,
																PixelFormat pixelFormat, int pitch, Color8[] palette, int paletteSize );

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createStreamSurfaceFromRawData(IntPtr streamEncoder, ref C_Blueprint blueprint, Byte[] pixels,
																PixelDescription pixelDescription, int pitch, Color8[] palette, int paletteSize );

}
