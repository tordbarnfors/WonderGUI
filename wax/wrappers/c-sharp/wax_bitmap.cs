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


//____ Bitmap ________________________________________________________________
public class Bitmap : Objekt
{
	//____ Blueprint __________________________________________________________
	public struct Blueprint
	{
		public Blueprint() { }

		public ByteOrder ByteOrder = ByteOrder.Native;
		public bool Canvas = false;
		public ColorSpace ColorSpace = ColorSpace.Undefined;        // Defaults to sRGB.
		public Color? FillColor = null;                             // What a bitmap created without pixels is filled with. Null leaves its content unspecified.
		//		public const wax_color8* palette;				HOW DO WE DO THIS?
		public int PaletteSize = 0;
		public int PaletteCapacity = 0;
		public bool Dynamic = false;
		public PixelFormat format = PixelFormat.Undefined;
		public int Identity = 0;
		public bool Mipmap = false;
		public IntPtr Restorer = IntPtr.Zero;                       // Pointer to a native wax_restorer_func, such as Bitmap.DeclineRestore. Zero has the bitmap keep its pixels.
		public IntPtr RestorerData = IntPtr.Zero;                   // Passed to Restorer.
		public SampleMethod SampleMethod = SampleMethod.Undefined;
		public int Scale = 0;
		public SizeI Size;                  // Mandatory, except when creating from other bitmap.
		public bool Tiling = false;
	}

	//____ SetId() ____________________________________________________________
	public void SetId(int id)
	{
		wax_setBitmapIdentity(_obj, id);
	}

	//____ Id() _______________________________________________________________
	public int Id()
	{
		return wax_getBitmapIdentity(_obj);
	}

	//____ SizeInPixels() _____________________________________________________
	public SizeI SizeInPixels()
	{
		return wax_bitmapPixelSize(_obj);
	}

	//____ Fill() _____________________________________________________________
	public bool Fill(ref Color color)
	{
		return (wax_fillBitmap(_obj, color) == 1);
	}

	//____ FillRect() _________________________________________________________
	public bool FillRect(ref RectI rect, ref Color color)
	{
		return (wax_fillBitmapRect(_obj, ref rect, color) == 1);
	}

	//____ Copy() _____________________________________________________________
	public bool Copy(CoordI dest, Bitmap source)
	{
		return (wax_copyBitmap(_obj, dest, source.CHandle()) == 1);
	}

	//____ CopyRect() _________________________________________________________
	public bool CopyRect(ref CoordI dest, Bitmap sourceBitmap, ref RectI sourceRect)
	{
		return (wax_copyBitmapRect(_obj, dest, sourceBitmap.CHandle(), ref sourceRect) == 1);
	}

    //____ GetPixelDescription() _________________________________________________
    public ref readonly PixelDescription GetPixelDescription() => ref wax_bitmapPixelDescription(_obj);

    //____ GetPixelFormat() ______________________________________________________

    public PixelFormat GetPixelFormat() => wax_bitmapPixelFormat(_obj);

    //____ GetColorSpace() ______________________________________________________

    public ColorSpace GetColorSpace() => wax_bitmapColorSpace(_obj);

    //____ IsBigEndian() ________________________________________________________

    public bool IsBigEndian() => wax_bitmapIsBigEndian(_obj) == 1;

    //____ GetPixelBits() ________________________________________________________
    public int GetPixelBits() => wax_bitmapPixelBits(_obj);

    //____ IsOpaque() _________________________________________________________
    public bool IsOpaque() => wax_bitmapIsOpaque(_obj) == 1;

    //____ CanBeCanvas() ______________________________________________________
    public bool CanBeCanvas() => wax_bitmapCanBeCanvas(_obj) == 1;

    //____ IsContentLost() ______________________________________________________

    public bool IsContentLost() => wax_bitmapIsContentLost(_obj) == 1;

    //____ DeclineRestore _______________________________________________________
    // A restorer that does nothing, for Blueprint.Restorer.

    public static IntPtr DeclineRestore => NativeLibrary.GetExport(NativeLibrary.Load(NativeLib), "wax_declineBitmapRestore");

	//____ DropRestorer() _____________________________________________________
	// Has a bitmap with a restorer decline from now on, for when the restorer or its data is going away.

	public void DropRestorer() => wax_dropBitmapRestorer(_obj);

	//____ AllocPixelBuffer() _________________________________________________
	public PixelBuffer AllocPixelBuffer()
	{
		C_PixelBuffer c_buff = wax_allocPixelBuffer(_obj);

		int sizeInBytes = (c_buff.rect.H - 1) * c_buff.pitch + (c_buff.rect.W * wax_bitmapPixelBits(_obj)) / 8;

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
		public SizeI size;                   // Mandatory, except when creating from other bitmap.
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
	private static extern int wax_bitmapIsContentLost(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_dropBitmapRestorer(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_setBitmapIdentity(IntPtr bitmap, int id);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_getBitmapIdentity(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SizeI wax_bitmapPixelSize(IntPtr bitmap);                        ///< @brief Get the size in pixels of the bitmap.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_fillBitmap(IntPtr bitmap, Color col);                                ///< @brief Fill bitmap with specified color.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_fillBitmapRect(IntPtr bitmap, ref RectI region, Color col);          ///< @brief Fill section of bitmap with specified color

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_copyBitmap(IntPtr destBitmap, CoordI dst, IntPtr sourceBitmap);     ///< @brief Copy other bitmap as a block

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_copyBitmapRect(IntPtr destBitmap, CoordI dst, IntPtr sourceBitmap, ref RectI srcRect);    ///< @brief Copy block of graphics from other bitmap

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern ref readonly PixelDescription wax_bitmapPixelDescription(IntPtr bitmap); ///< @brief Get the pixel description for the bitmap.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern PixelFormat wax_bitmapPixelFormat(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern ColorSpace wax_bitmapColorSpace(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_bitmapIsBigEndian(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_bitmapPixelBits(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_bitmapIsOpaque(IntPtr bitmap);                ///< @brief Check if bitmap is guaranteed to be entirely opaque.

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_bitmapCanBeCanvas(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern C_PixelBuffer wax_allocPixelBuffer(IntPtr bitmap);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern C_PixelBuffer wax_allocPixelBufferFromRect(IntPtr bitmap, in RectI rect);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_pushPixels(IntPtr bitmap, in C_PixelBuffer buffer);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_pushPixelsFromRect(IntPtr bitmap, in C_PixelBuffer buffer, in RectI bufferRect);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_pullPixels(IntPtr bitmap, in C_PixelBuffer buffer);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_pullPixelsFromRect(IntPtr bitmap, in C_PixelBuffer buffer, in RectI bufferRect);            // Needs to be overridden!

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_freePixelBuffer(IntPtr bitmap, in C_PixelBuffer buffer);


}
