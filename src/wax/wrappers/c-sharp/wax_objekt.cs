using System.Runtime.InteropServices;

namespace Wax;

public class Objekt
{
	internal protected const string NativeLib = "streamgendll";

    protected Objekt()
    {
    }

    ~Objekt()
	{
		wax_release(_obj);
	}


	//const wax_typeInfo*	wax_getTypeInfo(wax_obj);
	//int				wax_isInstanceOf(wax_obj, const wax_typeInfo* pTypeInfo);

	//WAX_EXPORT void				wax_setFinalizer(wax_obj, wax_finalizer_func pFinalizer);
	//WAX_EXPORT wax_finalizer_func	wax_finalizer(wax_obj);




	public void Retain()
	{
		wax_retain(_obj);
	}

	public void Release()
	{
		wax_release(_obj);
	}

	public int Refcount()
	{
		return wax_refcount(_obj);
	}

	public IntPtr CHandle()
	{
		return _obj;
	}

	protected IntPtr _obj = IntPtr.Zero;

    //____ DLL functions ______________________________________________________

	//[DllImport("libstreamgendll.so")]
	//public static extern const wax_typeInfo*	wax_getTypeInfo(IntPtr);

	//[DllImport("libstreamgendll.so")]
	//public static extern int				wax_isInstanceOf(IntPtr, const wax_typeInfo* pTypeInfo);
	//[DllImport("libstreamgendll.so")]
	//public static extern void				wax_setFinalizer(IntPtr, wax_finalizer_func pFinalizer);
	//[DllImport("libstreamgendll.so")]
	//public static extern wax_finalizer_func	wax_finalizer(IntPtr);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_retain(IntPtr obj);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_release(IntPtr obj);

	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern int wax_refcount(IntPtr obj);


}
