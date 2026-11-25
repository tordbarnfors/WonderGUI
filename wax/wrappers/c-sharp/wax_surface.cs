using System.Drawing;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Wax;


	[StructLayout(LayoutKind.Sequential)]
	internal struct C_PixelBuffer
	{
		public PixelFormat format;
		public IntPtr pixels;
		public IntPtr palette;
		public RectI rect;
		public int pitch;
		public ColorSpace colorSpace;
		public byte bigEndian;
	}

//____ PixelBuffer _____________________________________________________________
public unsafe struct PixelBuffer
{
	internal PixelBuffer(C_PixelBuffer c_buffer, int sizeInBytes )
	{
		this._c_buffer = c_buffer;

		this.Format = c_buffer.format;
		this.ColorSpace = c_buffer.colorSpace;
		this.BigEndian = c_buffer.bigEndian != 0;
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
	public ColorSpace ColorSpace;
	public bool BigEndian;
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

		public ByteOrder ByteOrder = ByteOrder.Native;
		public bool Canvas = false;
		public ColorSpace ColorSpace = ColorSpace.Undefined;        // Defaults to sRGB.
		public Color? FillColor = null;                             // What a surface created without pixels is filled with. Null leaves its content unspecified.
		//		public const wax_color8* palette;				HOW DO WE DO THIS?
		public int PaletteSize = 0;
		public int PaletteCapacity = 0;
		public bool Dynamic = false;
		public PixelFormat format = PixelFormat.Undefined;
		public int Identity = 0;
		public bool Mipmap = false;
		public IntPtr Restorer = IntPtr.Zero;                       // Pointer to a native wax_restorer_func, such as Surface.DeclineRestore. Zero has the surface keep its pixels.
		public IntPtr RestorerData = IntPtr.Zero;                   // Passed to Restorer.
		public SampleMethod SampleMethod = SampleMethod.Undefined;
		public int Scale = 0;
		public SizeI Size;                  // Mandatory, except when creating from other surface.
		public bool Tiling = false;
	}

	//____ SetId() ____________________________________________________________
	public void SetId(int id)
	{
		wax_setSurfaceIdentity(_obj, id);
	}

	//____ Id() _______________________________________________________________
	public int Id()
	{
		return wax_getSurfaceIdentity(_obj);
	}

	//____ SizeInPixels() _____________________________________________________
	public SizeI SizeInPixels()
	{
		return wax_surfacePixelSize(_obj);
	}

	//____ Fill() _____________________________________________________________
	public bool Fill(ref Color color)
	{
		return (wax_fillSurface(_obj, color) == 1);
	}

	//____ FillRect() _________________________________________________________
	public bool FillRect(ref RectI rect, ref Color color)
	{
		return (wax_fillSurfaceRect(_obj, ref rect, color) == 1);
	}

	//____ Copy() _____________________________________________________________
	public bool Copy(CoordI dest, Surface source)
	{
		return (wax_copySurface(_obj, dest, source.CHandle()) == 1);
	}

	//____ CopyRect() _________________________________________________________
	public bool CopyRect(ref CoordI dest, Surface sourceSurface, ref RectI sourceRect)
	{
		return (wax_copySurfaceRect(_obj, dest, sourceSurface.CHandle(), ref sourceRect) == 1);
	}

    //____ GetPixelDescription() _________________________________________________
    public ref readonly PixelDescription GetPixelDescription() => ref wax_surfacePixelDescription(_obj);

    //____ GetPixelFormat() ______________________________________________________

    public PixelFormat GetPixelFormat() => wax_surfacePixelFormat(_obj);

    //____ GetColorSpace() ______________________________________________________

    public ColorSpace GetColorSpace() => wax_surfaceColorSpace(_obj);

    //____ IsBigEndian() ________________________________________________________

    public bool IsBigEndian() => wax_surfaceIsBigEndian(_obj) == 1;

    //____ GetPixelBits() ________________________________________________________
    public int GetPixelBits() => wax_surfacePixelBits(_obj);

    //____ IsOpaque() _________________________________________________________
    public bool IsOpaque() => wax_surfaceIsOpaque(_obj) == 1;

    //____ CanBeCanvas() ______________________________________________________
    public bool CanBeCanvas() => wax_surfaceCanBeCanvas(_obj) == 1;

    //____ IsContentLost() ______________________________________________________

    public bool IsContentLost() => wax_surfaceIsContentLost(_obj) == 1;

    //____ DeclineRestore _______________________________________________________
    // A restorer that does nothing, for Blueprint.Restorer.

    public static IntPtr DeclineRestore => NativeLibrary.GetExport(NativeLibrary.Load(NativeLib), "wax_declineSurfaceRestore");

	//____ DropRestorer() _____________________________________________________
	// Has a surface with a restorer decline from now on, for when the restorer or its data is going away.

	public void DropRestorer() => wax_dropSurfaceRestorer(_obj);

	//____ AllocPixelBuffer() _________________________________________________
	public PixelBuffer AllocPixelBuffer()
	{
		C_PixelBuffer c_buff = wax_allocPixelBuffer(_obj);

		int sizeInBytes = (c_buff.rect.H - 1) * c_buff.pitch + (c_buff.rect.W * wax_surfacePixelBits(_obj)) / 8;

		return new(c_buff,sizeInBytes);
	}

	//____ PushPixels() _______________________________________________________
	public void PushPixels(in PixelBuffer buffer)
	{
		wax_pushPixels(_obj, buffer._c_buffer);
	}

	//____ PullPixels() _______________________________________________________
	public void PullPixels(in PixelBuffer buffer)
	{
		wax_pullPixels(_obj, buffer._c_buffer);
	}

	//____ FreePixelBuffer() __________________________________________________
	public void FreePixelBuffer(in PixelBuffer buffer)
	{
		wax_freePixelBuffer(_obj, buffer._c_buffer);
	}

	//____ C_Blueprint ________________________________________________________

	[StructLayout(LayoutKind.Sequential)]
	public struct C_Blueprint
	{
		public ByteOrder byteOrder;
		public byte canvas;
		public ColorSpace colorSpace;
		public byte useFillColor;
		public long fillColor;               // A wax_color, which is aligned as the 64 bit word it also is.
		public IntPtr palette;
		public int paletteSize;
		public int paletteCapacity;
		public byte dynamic;
		public PixelFormat format;
		public int identity;
		public byte mipmap;
		public IntPtr restorer;
		public IntPtr restorerData;
		public SampleMethod sampleMethod;
		public int scale;
		public SizeI size;                   // Mandatory, except when creating from other surface.
		public byte tiling;
	}

	//____ ConvertBlueprint() _________________________________________________
	static internal void ConvertBlueprint(in Blueprint org, out C_Blueprint converted)
	{
		converted.byteOrder = org.ByteOrder;
		converted.canvas = (byte)(org.Canvas ? 1 : 0);
		converted.colorSpace = org.ColorSpace;
		converted.useFillColor = (byte)(org.FillColor.HasValue ? 1 : 0);
		Color fillColor = org.FillColor ?? new Color(0, 0, 0, 0);
		converted.fillColor = Unsafe.As<Color, long>(ref fillColor);
		converted.palette = 0;                                          //TODO: Need to solve this...
		converted.paletteSize = 0;
		converted.paletteCapacity = org.PaletteCapacity;
		converted.dynamic = (byte)(org.Dynamic ? 1 : 0);
		converted.format = org.format;
		converted.identity = org.Identity;
		converted.mipmap = (byte)(org.Mipmap ? 1 : 0);
		converted.restorer = org.Restorer;
		converted.restorerData = org.RestorerData;
		converted.sampleMethod = org.SampleMethod;
		converted.scale = org.Scale;
		converted.size = new SizeI(org.Size.W, org.Size.H);
		converted.tiling = (byte)(org.Tiling ? 1 : 0);
	}

	//____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_surfaceIsContentLost(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_dropSurfaceRestorer(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_setSurfaceIdentity(IntPtr surface, int id);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_getSurfaceIdentity(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeI wax_surfacePixelSize(IntPtr surface);                        ///< @brief Get the size in pixels of the surface.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_fillSurface(IntPtr surface, Color col);                                ///< @brief Fill surface with specified color.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_fillSurfaceRect(IntPtr surface, ref RectI region, Color col);          ///< @brief Fill section of surface with specified color

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_copySurface(IntPtr destSurface, CoordI dst, IntPtr sourceSurface);     ///< @brief Copy other surface as a block

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_copySurfaceRect(IntPtr destSurface, CoordI dst, IntPtr sourceSurface, ref RectI srcRect);    ///< @brief Copy block of graphics from other surface

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern ref readonly PixelDescription wax_surfacePixelDescription(IntPtr surface); ///< @brief Get the pixel description for the surface.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern PixelFormat wax_surfacePixelFormat(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern ColorSpace wax_surfaceColorSpace(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_surfaceIsBigEndian(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_surfacePixelBits(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_surfaceIsOpaque(IntPtr surface);                ///< @brief Check if surface is guaranteed to be entirely opaque.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_surfaceCanBeCanvas(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern C_PixelBuffer wax_allocPixelBuffer(IntPtr surface);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern C_PixelBuffer wax_allocPixelBufferFromRect(IntPtr surface, in RectI rect);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_pushPixels(IntPtr surface, in C_PixelBuffer buffer);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_pushPixelsFromRect(IntPtr surface, in C_PixelBuffer buffer, in RectI bufferRect);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_pullPixels(IntPtr surface, in C_PixelBuffer buffer);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_pullPixelsFromRect(IntPtr surface, in C_PixelBuffer buffer, in RectI bufferRect);            // Needs to be overridden!

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_freePixelBuffer(IntPtr surface, in C_PixelBuffer buffer);


}
