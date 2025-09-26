using System.Runtime.InteropServices;

namespace WG;


public class Surface : Objekt
{
	private const string NativeLib = "libstreamgendll";

	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public bool				buffered;
		public bool				canvas;
//		public const wg_color8* palette;				HOW DO WE DO THIS?
		public int				paletteSize;
		public int				paletteCapacity;
		public bool				dynamic;
		public PixelFormat		format;
		public int				identity;
		public bool				mipmap;
		public SampleMethod		sampleMethod;
		public int				scale;
		public SizeI			size;					// Mandatory, except when creating from other surface.
		public bool				tiling;
	}

	//____ SetId() ____________________________________________________________
	public void SetId(int id)
	{
		wg_setSurfaceIdentity(_obj, id);
	}

	//____ Id() _______________________________________________________________
	public int Id()
	{
		return wg_getSurfaceIdentity(_obj);
	}

	//____ SizeInPixels() _____________________________________________________
	public SizeI SizeInPixels()
	{
		return wg_surfacePixelSize(_obj);
	}

	//____ Fill() _____________________________________________________________
	public bool Fill(ref Color color)
	{
		return (wg_fillSurface(_obj, color) == 1);
	}

	//____ FillRect() _________________________________________________________
	public bool FillRect(ref RectI rect, ref Color color)
	{
		return (wg_fillSurfaceRect(_obj, ref rect, color) == 1);
	}

	//____ Copy() _____________________________________________________________
	public bool Copy(CoordI dest, Surface source)
	{
		return (wg_copySurface(_obj, dest, source.CHandle()) == 1);
	}

	//____ CopyRect() _________________________________________________________
	public bool CopyRect(ref CoordI dest, Surface sourceSurface, ref RectI sourceRect)
	{
		return (wg_copySurfaceRect(_obj, dest, sourceSurface.CHandle(), ref sourceRect) == 1);
	}


	[StructLayout(LayoutKind.Sequential)]
	protected struct wg_surfaceBP
	{
	    public byte					buffered;
	    public byte					canvas;
	    public IntPtr             	palette;
		public int					paletteSize;
	    public int					paletteCapacity;
	    public byte					dynamic;
	    public PixelFormat			format;
	    public int					identity;
	    public byte					mipmap;
	    public SampleMethod			sampleMethod;
	    public int					scale;
	    public SizeI				size;                   // Mandatory, except when creating from other surface.
	    public byte					tiling;
	}



	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_setSurfaceIdentity(IntPtr surface, int id);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_getSurfaceIdentity(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeI wg_surfacePixelSize(IntPtr surface);                        ///< @brief Get the size in pixels of the surface.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_fillSurface(IntPtr surface, Color col);                                ///< @brief Fill surface with specified color.
	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_fillSurfaceRect(IntPtr surface, ref RectI region, Color col);			///< @brief Fill section of surface with specified color
	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_copySurface(IntPtr destSurface, CoordI dst, IntPtr sourceSurface);     ///< @brief Copy other surface as a block
	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_copySurfaceRect(IntPtr destSurface, CoordI dst, IntPtr sourceSurface, ref RectI srcRect);    ///< @brief Copy block of graphics from other surface




}
