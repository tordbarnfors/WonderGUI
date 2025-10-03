using System.Drawing;
using System.Runtime.InteropServices;

namespace WG;


	[StructLayout(LayoutKind.Sequential)]
	internal struct C_PixelBuffer
	{
		public PixelFormat format;
		public IntPtr pixels;
		public IntPtr palette;
		public RectI rect;
		public int pitch;
	}

//____ PixelBuffer _____________________________________________________________
public unsafe struct PixelBuffer
{
	internal PixelBuffer(C_PixelBuffer c_buffer, int sizeInBytes )
	{
		this._c_buffer = c_buffer;

		this.Format = c_buffer.format;
		this.Rect = c_buffer.rect;
		this.Pitch = c_buffer.pitch;
		this._sizeInBytes = sizeInBytes;
	}

	public Span<T> Pixels<T>() where T : unmanaged
	{
		int count = _sizeInBytes / sizeof(T);
		return new Span<T>(_c_buffer.pixels.ToPointer(), count);
	}

	public PixelFormat Format;
	public RectI Rect;
	public int Pitch;

	internal C_PixelBuffer _c_buffer;
	internal int _sizeInBytes;
}


//____ Surface ________________________________________________________________
public class Surface : Objekt
{
	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public Blueprint() { }

		public bool Buffered = false;
		public bool Canvas = false;
		//		public const wg_color8* palette;				HOW DO WE DO THIS?
		public int PaletteSize = 0;
		public int PaletteCapacity = 0;
		public bool Dynamic = false;
		public PixelFormat format = PixelFormat.Undefined;
		public int Identity = 0;
		public bool Mipmap = false;
		public SampleMethod SampleMethod = SampleMethod.Undefined;
		public int Scale = 0;
		public SizeI Size;                  // Mandatory, except when creating from other surface.
		public bool Tiling = false;
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

    //____ GetPixelDescription() _________________________________________________
    public ref readonly PixelDescription GetPixelDescription() => ref wg_surfacePixelDescription(_obj);

    //____ GetPixelFormat() ______________________________________________________

    public PixelFormat GetPixelFormat() => wg_surfacePixelFormat(_obj);

    //____ GetPixelBits() ________________________________________________________
    public int GetPixelBits() => wg_surfacePixelBits(_obj);

    //____ IsOpaque() _________________________________________________________
    public bool IsOpaque() => wg_surfaceIsOpaque(_obj) == 1;

    //____ CanBeCanvas() ______________________________________________________
    public bool CanBeCanvas() => wg_surfaceCanBeCanvas(_obj) == 1;

	//____ AllocPixelBuffer() _________________________________________________
	public PixelBuffer AllocPixelBuffer()
	{
		C_PixelBuffer c_buff = wg_allocPixelBuffer(_obj);

		int sizeInBytes = (c_buff.rect.H - 1) * c_buff.pitch + (c_buff.rect.W * wg_surfacePixelBits(_obj)) / 8;

		return new(c_buff,sizeInBytes);
	}

	//____ PushPixels() _______________________________________________________
	public void PushPixels(in PixelBuffer buffer)
	{
		wg_pushPixels(_obj, buffer._c_buffer);
	}

	//____ PullPixels() _______________________________________________________
	public void PullPixels(in PixelBuffer buffer)
	{
		wg_pullPixels(_obj, buffer._c_buffer);
	}

	//____ FreePixelBuffer() __________________________________________________
	public void FreePixelBuffer(in PixelBuffer buffer)
	{
		wg_freePixelBuffer(_obj, buffer._c_buffer);
	}

	//____ C_Blueprint ________________________________________________________

	[StructLayout(LayoutKind.Sequential)]
	public struct C_Blueprint
	{
		public byte buffered;
		public byte canvas;
		public IntPtr palette;
		public int paletteSize;
		public int paletteCapacity;
		public byte dynamic;
		public PixelFormat format;
		public int identity;
		public byte mipmap;
		public SampleMethod sampleMethod;
		public int scale;
		public SizeI size;                   // Mandatory, except when creating from other surface.
		public byte tiling;
	}

	//____ ConvertBlueprint() _________________________________________________
	static internal void ConvertBlueprint(in Blueprint org, out C_Blueprint converted)
	{
		converted.buffered = (byte)(org.Buffered ? 1 : 0);
		converted.canvas = (byte)(org.Canvas ? 1 : 0);
		converted.palette = 0;                                          //TODO: Need to solve this...
		converted.paletteSize = 0;
		converted.paletteCapacity = org.PaletteCapacity;
		converted.dynamic = (byte)(org.Dynamic ? 1 : 0);
		converted.format = org.format;
		converted.identity = org.Identity;
		converted.mipmap = (byte)(org.Mipmap ? 1 : 0);
		converted.sampleMethod = org.SampleMethod;
		converted.scale = org.Scale;
		converted.size = new SizeI(org.Size.W, org.Size.H);
		converted.tiling = (byte)(org.Tiling ? 1 : 0);
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

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern ref readonly PixelDescription wg_surfacePixelDescription(IntPtr surface); ///< @brief Get the pixel description for the surface.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern PixelFormat wg_surfacePixelFormat(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_surfacePixelBits(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_surfaceIsOpaque(IntPtr surface);                ///< @brief Check if surface is guaranteed to be entirely opaque.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_surfaceCanBeCanvas(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern C_PixelBuffer wg_allocPixelBuffer(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern C_PixelBuffer wg_allocPixelBufferFromRect(IntPtr surface, in RectI rect);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_pushPixels(IntPtr surface, in C_PixelBuffer buffer);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_pushPixelsFromRect(IntPtr surface, in C_PixelBuffer buffer, in RectI bufferRect);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_pullPixels(IntPtr surface, in C_PixelBuffer buffer);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_pullPixelsFromRect(IntPtr surface, in C_PixelBuffer buffer, in RectI bufferRect);            // Needs to be overridden!

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_freePixelBuffer(IntPtr surface, in C_PixelBuffer buffer);


}
