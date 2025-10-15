using System.ComponentModel;
using System.Runtime.InteropServices;

namespace WG;

public static class GfxBase
{
    private const string NativeLib = "streamgendll";

    //____ Init() _________________________________________________________

    static public bool Init()
    {
        int ret = wg_initGfxBase();

        return (ret == 1);
    }

    //____ Exit() _________________________________________________________

    static public bool Exit()
    {
        int ret = wg_exitGfxBase();

        return (ret == 1);
    }

    //____ IsInitialized() ________________________________________________

    static public bool IsInitialized()
    {
        int ret = wg_initGfxBase();

        return (ret == 1);
    }

    //____ SetDefaultToSRGB() _________________________________________________

    static public void SetDefaultToSRGB(bool bSRGB)
    {
        wg_setDefaultToSRGB(bSRGB ? 1 : 0);
    }

    //____ GetDefaultToSRGB() _________________________________________________
    static public bool GetDefaultToSRGB()
    {
        return (wg_defaultToSRGB() == 1);
    }

   //____ GetDefaultBitmapCache() ____________________________________________

    static public BitmapCache GetDefaultBitmapCache()
    {
        return new BitmapCache(wg_defaultBitmapCache());
    }


    //____ ErrorInfo ________________________________________________________

    public struct ErrorInfo
    {
        public ErrorLevel severity;
        public ErrorCode code;
        public string message;
        public IntPtr objekt;
        public string classname;
        public string function;
        public string file;
        public int line;
    }


    //____ C_ErrorInfo ________________________________________________________

    [StructLayout(LayoutKind.Sequential)]
    private struct C_ErrorInfo
    {
        public ErrorLevel severity;
        public ErrorCode code;
        public IntPtr message;
        public IntPtr objekt;
        public IntPtr classname;
        public IntPtr function;
        public IntPtr file;
        public int line;
    }

    //____ ErroHandler delegate _______________________________________________
    public delegate void ErrorHandler(in ErrorInfo errInfo);

    //____ SetErrorHandler() __________________________________________________

    static public void SetErrorHandler(ErrorHandler handler)
    {
        wg_setErrorHandler(InternalErrorHandler);

        _handler = handler;
    }

    //____ SetDefaultSurfaceFactory() _________________________________________
    static public void SetDefaultSurfaceFactory(SurfaceFactory factory)
    {
        wg_setDefaultSurfaceFactory( factory.CHandle() );
    }

    /*
        //____ SetDefaultEdgemapFactory() _________________________________________
        static public void SetDefaultSurfaceFactory(EdgemapFactory factory)
        {
            wg_setDefaultEdgemapFactory(factory.CHandle());
        }
    */

    //____ SetDefaultGfxDevice() _________________________________________
    static public void SetDefaultGfxDevice(GfxDevice device)
    {
        wg_setDefaultGfxDevice(device.CHandle());
    }

    private static void InternalErrorHandler(in C_ErrorInfo c_errInfo)
    {
        if (_handler != null)
        {
            ErrorInfo info = new ErrorInfo();
            info.severity = c_errInfo.severity;
            info.code = c_errInfo.code;

            if (c_errInfo.message != IntPtr.Zero)
                info.message = Marshal.PtrToStringUTF8(c_errInfo.message) ?? string.Empty;

            info.objekt = c_errInfo.objekt;

            if (c_errInfo.classname != IntPtr.Zero)
                info.classname = Marshal.PtrToStringUTF8(c_errInfo.classname) ?? string.Empty;

            if (c_errInfo.function != IntPtr.Zero)
                info.function = Marshal.PtrToStringUTF8(c_errInfo.function) ?? string.Empty;

            if (c_errInfo.file != IntPtr.Zero)
                info.file = Marshal.PtrToStringUTF8(c_errInfo.file) ?? string.Empty;

            info.line = c_errInfo.line;

            _handler(in info);
        }
    }


    static private ErrorHandler? _handler = null;

    //____ Callback delegate __________________________________________________________

    // Define a delegate that matches the C++ callback signature
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void C_ErrorHandler(in C_ErrorInfo errInfo);

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_initGfxBase();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_exitGfxBase();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_isGfxBaseInitialized();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setDefaultToSRGB(int bSRGB);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_defaultToSRGB();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr	wg_defaultBitmapCache();
    
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setErrorHandler(C_ErrorHandler errorHandler);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setDefaultSurfaceFactory( IntPtr factory );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_defaultSurfaceFactory();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setDefaultEdgemapFactory( IntPtr factory );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_defaultEdgemapFactory();

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setDefaultGfxDevice( IntPtr device );

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_defaultGfxDevice();




}

