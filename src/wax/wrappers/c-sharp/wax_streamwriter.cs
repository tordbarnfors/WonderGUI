using System.Runtime.InteropServices;

namespace Wax;


public class StreamWriter : Objekt
{
    //____ Constructor ________________________________________________________
    public StreamWriter(WriteStreamCallback callback)
    {
        _obj = wax_createStreamWriter(callback);
        _callback = callback;
    }

    //____ GetSink() __________________________________________________________

    public StreamSink GetSink()
    {
        C_Component c_comp = wax_getStreamWriterInput(_obj);
        StreamSink sink = new StreamSink(c_comp.component, this);
        return sink;
    }

    //____ Callback delegate __________________________________________________________

    // Define a delegate that matches the C++ callback signature
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void WriteStreamCallback(int nBytes, IntPtr pData);

    //____ Data _______________________________________________________________

    WriteStreamCallback _callback;

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr wax_createStreamWriter(WriteStreamCallback dispatcher);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern C_Component	wax_getStreamWriterInput(IntPtr streamWriter);


}