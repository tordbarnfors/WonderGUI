using System.Runtime.InteropServices;

namespace WG;


public class SurfaceWriter : Objekt
{

	//____ SaveInfo ___________________________________________________________
	public struct SaveInfo
	{
		public SaveInfo() { }

		public bool identity = true;
		public bool sampleMethod = false;
		public bool scale = true;
		public bool buffered = false;
		public bool canvas = false;
		public bool dynamic = false;
		public bool tiling = true;
		public bool mipmap = false;
	}

	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public SaveInfo saveInfo;
	}

	//____ Constructor ________________________________________________________

	public SurfaceWriter(ref Blueprint blueprint)
	{
		C_Blueprint bp;

		int saveInfo = 0;
		if (blueprint.saveInfo.identity)
			saveInfo += 1;
		if (blueprint.saveInfo.sampleMethod)
			saveInfo += 2;
		if (blueprint.saveInfo.scale)
			saveInfo += 4;
		if (blueprint.saveInfo.buffered)
			saveInfo += 8;
		if (blueprint.saveInfo.canvas)
			saveInfo += 16;
		if (blueprint.saveInfo.dynamic)
			saveInfo += 32;
		if (blueprint.saveInfo.tiling)
			saveInfo += 64;
		if (blueprint.saveInfo.mipmap)
			saveInfo += 128;

		bp.saveInfo = saveInfo;

		_obj = wg_createSurfaceWriter(ref bp);
	}

	//____ WriteSurfaceToBlob() _______________________________________________

	Blob WriteSurfaceToBlob(ref Surface surface, byte[] extraData)
	{
		IntPtr p = wg_writeSurfaceToBlob(_obj, surface.CHandle(), extraData.Length, extraData);
		return new Blob(p);
	}


	protected struct C_Blueprint
	{
		public int saveInfo;
	}


    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createSurfaceWriter(ref C_Blueprint blueprint);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_writeSurfaceToBlob(IntPtr surfaceWriter, IntPtr surface, int extraDataSize, byte[] pExtraData);

}
