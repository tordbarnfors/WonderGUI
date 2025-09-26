using System.Runtime.InteropServices;

namespace WG;

public class SoftSurface : Surface
{
	private const string NativeLib = "libstreamgendll";


	public SoftSurface( ref Blueprint blueprint )
	{
		// TODO: Default values!!!!

		wg_surfaceBP	bp;

		bp.buffered = (byte) (blueprint.buffered ? 1 : 0);
		bp.canvas = (byte) (blueprint.canvas ? 1: 0);
		bp.palette = 0;											//TODO: Need to solve this...
		bp.paletteSize = 0;
		bp.paletteCapacity = blueprint.paletteCapacity;
		bp.dynamic = (byte) (blueprint.dynamic ? 1: 0);
		bp.format = blueprint.format;
		bp.identity = blueprint.identity;
		bp.mipmap = (byte) (blueprint.mipmap ? 1 : 0);
		bp.sampleMethod = blueprint.sampleMethod;
		bp.scale = blueprint.scale;
		bp.size.w = blueprint.size.w;
		bp.size.h = blueprint.size.h;
		bp.tiling = (byte) (blueprint.tiling ? 1 : 0);

		_obj = wg_createSoftSurface(ref bp);
	}



	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createSoftSurface(ref wg_surfaceBP blueprint);


}
