using System.Runtime.InteropServices;

namespace WG;


public class FreeTypeFont : Font
{
    private const string NativeLib = "libstreamgendll";

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

		public Font? backupFont;
		public Blob? blob;
		public BitmapCache? cache;
		public int faceIndex = 0;
		public RenderMode renderMode = RenderMode.BestShapes;
		public bool stemDarkening = GfxBase.GetDefaultToSRGB();
		public int xDPI = 72;
		public int yDPI = 72;
	}

	//____ Constructor ________________________________________________________
	public FreeTypeFont(ref Blueprint bp)
	{
		wg_freeTypeFontBP cbp;

		cbp.backupFont = bp.backupFont == null ? 0 : bp.backupFont.CHandle();
		cbp.blob = bp.blob == null ? 0 : bp.blob.CHandle();
		cbp.cache = bp.cache == null ? 0 : bp.cache.CHandle();
		cbp.faceIndex = bp.faceIndex;

		cbp.renderMode = bp.renderMode;
		cbp.stemDarkening = bp.stemDarkening ? 1 : 0;
		cbp.xDPI = bp.xDPI;
		cbp.yDPI = bp.yDPI;

		_obj = wg_createFreeTypeFont(cbp);
	}


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


	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createFreeTypeFont( wg_freeTypeFontBP blueprint );

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_getFreeTypeFontBitmapCache( IntPtr font );

}