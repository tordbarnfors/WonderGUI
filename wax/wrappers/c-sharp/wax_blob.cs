using System.Runtime.InteropServices;

namespace Wax;


public class Blob : Objekt
{

   //____ Callback delegate __________________________________________________________

    // Define a delegate that matches the C++ callback signature
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void BlobDestroyedCallback();

	//____ Constructors _______________________________________________________

	public Blob(int size)
	{
		_obj = wax_createBlob(size);
	}

	public Blob(in Span<byte> bytes, BlobDestroyedCallback callback)
	{
		_obj = wax_createBlobFromData(bytes, bytes.Length, callback);
		_destructor = callback;			// Save it so it doesn't get garbage collected.
	}

	internal Blob(IntPtr c_handle)
	{
		_obj = c_handle;
	}

	//____ Size() _____________________________________________________________
	public int Size()
	{
		return wax_blobSize(_obj);
	}

	//____ Data() _____________________________________________________________
	public Span<byte> Data()
	{
		IntPtr ptr = wax_blobData(_obj);
		int ptrLength = wax_blobSize(_obj);

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
	private static extern IntPtr wax_createBlob(int size);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	static public extern IntPtr	wax_createBlobFromData( Span<byte> pData, int size, BlobDestroyedCallback destructor );

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_blobSize(IntPtr blob);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_blobData(IntPtr blob);
}
