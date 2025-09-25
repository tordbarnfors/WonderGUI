

namespace WG;

public class Objekt
{


	~Objekt()
	{
		wg_release(_obj);
	}


 	//const wg_typeInfo*	wg_getTypeInfo(wg_obj);
	//int				wg_isInstanceOf(wg_obj, const wg_typeInfo* pTypeInfo);

	//WG_EXPORT void				wg_setFinalizer(wg_obj, wg_finalizer_func pFinalizer);
	//WG_EXPORT wg_finalizer_func	wg_finalizer(wg_obj);




	public void		Retain()
	{
			wg_retain(_obj);
	}

	public void		Release()
	{
			wg_release(_obj);
	}

	public int		Refcount()
	{
			return wg_refcount(_obj);
	}


	private IntPtr		_obj;


	//[DllImport("libstreamgendll.so")]
//public static extern const wg_typeInfo*	wg_getTypeInfo(IntPtr);

//[DllImport("libstreamgendll.so")]
//public static extern int				wg_isInstanceOf(IntPtr, const wg_typeInfo* pTypeInfo);
//[DllImport("libstreamgendll.so")]
//public static extern void				wg_setFinalizer(IntPtr, wg_finalizer_func pFinalizer);
//[DllImport("libstreamgendll.so")]
//public static extern wg_finalizer_func	wg_finalizer(IntPtr);

[DllImport("libstreamgendll.so")]
private static extern void				wg_retain(IntPtr obj);

[DllImport("libstreamgendll.so")]
private static extern void				wg_release(IntPtr obj);

[DllImport("libstreamgendll.so")]
private static extern int				wg_refcount(IntPtr obj);


}
