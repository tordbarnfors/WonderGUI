using System.Drawing;
using System.Runtime.InteropServices;

namespace WG;


public class Surface : Objekt
{
	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public Blueprint() {}

		public bool 			Buffered = false;
		public bool				Canvas = false;
//		public const wg_color8* palette;				HOW DO WE DO THIS?
		public int				PaletteSize = 0;
		public int				PaletteCapacity = 0;
		public bool				Dynamic = false;
		public PixelFormat		format = PixelFormat.Undefined;
		public int				Identity = 0;
		public bool				Mipmap = false;
		public SampleMethod		SampleMethod = SampleMethod.Undefined;
		public int				Scale = 0;
		public SizeI			Size;					// Mandatory, except when creating from other surface.
		public bool				Tiling = false;
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
	public struct C_Blueprint
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

	static internal void ConvertBlueprint(in Blueprint org, out C_Blueprint converted )
	{
		converted.buffered = (byte) (org.Buffered ? 1 : 0);
		converted.canvas = (byte) (org.Canvas ? 1: 0);
		converted.palette = 0;											//TODO: Need to solve this...
		converted.paletteSize = 0;
		converted.paletteCapacity = org.PaletteCapacity;
		converted.dynamic = (byte) (org.Dynamic ? 1: 0);
		converted.format = org.format;
		converted.identity = org.Identity;
		converted.mipmap = (byte) (org.Mipmap ? 1 : 0);
		converted.sampleMethod = org.SampleMethod;
		converted.scale = org.Scale;
		converted.size = new SizeI( org.Size.W, org.Size.H );
		converted.tiling = (byte) (org.Tiling ? 1 : 0);
	}

    //____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_setSurfaceIdentity(IntPtr surface, int id);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_getSurfaceIdentity(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeI wg_surfacePixelSize(IntPtr surface);                        ///< @brief Get the size in pixels of the surface.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_fillSurface(IntPtr surface, Color col);                                ///< @brief Fill surface with specified color.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_fillSurfaceRect(IntPtr surface, ref RectI region, Color col);          ///< @brief Fill section of surface with specified color

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_copySurface(IntPtr destSurface, CoordI dst, IntPtr sourceSurface);     ///< @brief Copy other surface as a block

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_copySurfaceRect(IntPtr destSurface, CoordI dst, IntPtr sourceSurface, ref RectI srcRect);    ///< @brief Copy block of graphics from other surface




}
