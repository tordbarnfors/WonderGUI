using System.Runtime.InteropServices;

namespace WG;


public class Blob : Objekt
{
	private const string NativeLib = "libstreamgendll";

	//____ Constructors _______________________________________________________

	public Blob(int size)
	{
		_obj = wg_createBlob(size);
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



	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createBlob(int size);

	//	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	//	static public extern IntPtr	wg_createBlobFromData( Byte[] pData, int size, void(*destructor)() );

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wg_blobSize(IntPtr blob);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_blobData(IntPtr blob);
}
