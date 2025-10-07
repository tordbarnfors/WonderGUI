using System.Runtime.InteropServices;

namespace WG;


public class Blob : Objekt
{

   //____ Callback delegate __________________________________________________________

    // Define a delegate that matches the C++ callback signature
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void BlobDestroyedCallback();

	//____ Constructors _______________________________________________________

	public Blob(int size)
	{
		_obj = wg_createBlob(size);
	}

	public Blob(in Span<byte> bytes, BlobDestroyedCallback callback)
	{
		_obj = wg_createBlobFromData(bytes, bytes.Length, callback);
		_destructor = callback;			// Save it so it doesn't get garbage collected.
	}

	internal Blob(IntPtr c_handle)
	{
		_obj = c_handle;
	}

	//____ Size() _____________________________________________________________
	public int Size()
	{
		return wg_blobSize(_obj);
	}

	//____ Data() _____________________________________________________________
	public Span<byte> Data()
	{
		IntPtr ptr = wg_blobData(_obj);
		int ptrLength = wg_blobSize(_obj);

		Span<byte> byteArray;
		unsafe
		{
			byteArray = new Span<byte>(ptr.ToPointer(), ptrLength);
		}

		return byteArray;
	}

	BlobDestroyedCallback? _destructor = null;

	//____ DLL functions ______________________________________________________

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createBlob(int size);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	static public extern IntPtr	wg_createBlobFromData( Span<byte> pData, int size, BlobDestroyedCallback destructor );

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_blobSize(IntPtr blob);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_blobData(IntPtr blob);
}
