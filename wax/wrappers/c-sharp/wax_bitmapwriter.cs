using System.Runtime.InteropServices;

namespace Wax;


public class BitmapWriter : Objekt
{

	//____ SaveInfo ___________________________________________________________
	public struct SaveInfo
	{
		public SaveInfo() { }

		public bool Identity = true;
		public bool SampleMethod = false;
		public bool Scale = true;
		public bool Canvas = false;
		public bool Dynamic = false;
		public bool Tiling = true;
		public bool Mipmap = false;
	}

	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public SaveInfo saveInfo;
	}

	//____ Constructor ________________________________________________________

	public BitmapWriter(ref Blueprint blueprint)
	{
		C_Blueprint bp;

		int saveInfo = 0;
		if (blueprint.saveInfo.Identity)
			saveInfo += 1;
		if (blueprint.saveInfo.SampleMethod)
			saveInfo += 2;
		if (blueprint.saveInfo.Scale)
			saveInfo += 4;
		if (blueprint.saveInfo.Canvas)
			saveInfo += 16;
		if (blueprint.saveInfo.Dynamic)
			saveInfo += 32;
		if (blueprint.saveInfo.Tiling)
			saveInfo += 64;
		if (blueprint.saveInfo.Mipmap)
			saveInfo += 128;

		bp.saveInfo = saveInfo;

		_obj = wax_createBitmapWriter(ref bp);
	}

	//____ WriteBitmapToBlob() _______________________________________________

	public Blob WriteBitmapToBlob(ref Bitmap bitmap, byte[]? extraData = null)
	{
		int extraLength = extraData?.Length ?? 0;

		IntPtr p = wax_writeBitmapToBlob(_obj, bitmap.CHandle(), extraLength, extraData);
		return new Blob(p);
	}


	[StructLayout(LayoutKind.Sequential)]
	protected struct C_Blueprint
	{
		public int saveInfo;
	}


    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_createBitmapWriter(ref C_Blueprint blueprint);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_writeBitmapToBlob(IntPtr bitmapWriter, IntPtr bitmap, int extraDataSize, byte[]? pExtraData);

}
