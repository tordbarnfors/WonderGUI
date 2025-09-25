namespace WG;

public class SoftSurface : Surface
{


	public SoftSurface( ref Blueprint blueprint )
	{
		// TODO: Default values!!!!

		wg_surfaceBP	bp;

		bp.buffered = blueprint.buffered;
		bp.canvas = blueprint.canvas;
		bp.palette = null;											//TODO: Need to solve this...
		bp.paletteCapacity = blueprint.paletteCapacity;
		bp.dynamic = blueprint.dynamic;
		bp.format = blueprint.format;
		bp.identity = blueprint.identity;
		bp.mipmap = blueprint.mipmap;
		bp.sampleMethod = blueprint.sampleMethod;
		bp.scale = blueprint.scale;
		bp.size.w = blueprint.size.w;
		bp.size.h = blueprint.size.h;
		bp.tiling = blueprint.tiling;

		_obj = wg_createSoftSurface(bp);
	}



    [DllImport("libstreamgendll.so")]
    private static extern IntPtr	wg_createSoftSurface(ref wg_surfaceBP blueprint);


}
