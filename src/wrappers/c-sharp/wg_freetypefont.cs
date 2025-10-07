using System.Runtime.InteropServices;

namespace WG;


public class FreeTypeFont : Font
{

	//____ RenderMode _________________________________________________________
	public enum RenderMode
	{
		Monochrome,
		CrispEdges,
		BestShapes
	}

	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public Blueprint() { }

		public Font? BackupFont;
		public Blob? Blob;
		public BitmapCache? Cache;
		public int FaceIndex = 0;
		public RenderMode RenderMode = RenderMode.BestShapes;
		public bool StemDarkening = GfxBase.GetDefaultToSRGB();
		public int XDPI = 72;
		public int YDPI = 72;
	}

	//____ Constructor ________________________________________________________
	public FreeTypeFont(in Blueprint bp)
	{
		wg_freeTypeFontBP cbp;

		cbp.backupFont = bp.BackupFont == null ? 0 : bp.BackupFont.CHandle();
		cbp.blob = bp.Blob == null ? 0 : bp.Blob.CHandle();
		cbp.cache = bp.Cache == null ? 0 : bp.Cache.CHandle();
		cbp.faceIndex = bp.FaceIndex;

		cbp.renderMode = bp.RenderMode;
		cbp.stemDarkening = bp.StemDarkening ? 1 : 0;
		cbp.xDPI = bp.XDPI;
		cbp.yDPI = bp.YDPI;

		_obj = wg_createFreeTypeFont(cbp);
	}


	[StructLayout(LayoutKind.Sequential)]
	struct wg_freeTypeFontBP
	{
		public IntPtr backupFont;
		public IntPtr blob;
		public IntPtr cache;
		public int faceIndex;
		public RenderMode renderMode;
		public int stemDarkening;
		public int xDPI;
		public int yDPI;
	} 

    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createFreeTypeFont( wg_freeTypeFontBP blueprint );

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_getFreeTypeFontBitmapCache( IntPtr font );

}