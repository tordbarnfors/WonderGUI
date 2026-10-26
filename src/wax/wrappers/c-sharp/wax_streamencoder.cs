using System.Runtime.InteropServices;

namespace Wax;

public class StreamEncoder : Objekt
{
    //____ Blueprint __________________________________________________________
    public struct Blueprint
    {
        public Blueprint() { }
        public int BufferBytes = 4096 * 2;
        public UInt16 ObjectIdStart = 0;
        public PixelFormat PixelFormat = PixelFormat.ARGB_8;
        public SampleMethod SampleMethod = SampleMethod.Nearest;
        public StreamSink Sink;
    }

    //____ Constructor ________________________________________________________
    public StreamEncoder(ref Blueprint blueprint)
    {
        C_Blueprint c_bp;

        c_bp.bufferBytes = blueprint.BufferBytes;
        c_bp.objectIdStart = blueprint.ObjectIdStart;
        c_bp.pixelFormat = blueprint.PixelFormat;
        c_bp.sampleMethod = blueprint.SampleMethod;
        c_bp.sink = blueprint.Sink.c_component;

        _obj = wax_createStreamEncoder(ref c_bp);
    }

    //____ SetStream() ________________________________________________________
    public void SetStream(StreamSink stream)
    {
        wax_setEncoderStream(_obj, stream.c_component);
    }

    //____ SetDefaultPixelFormat() ____________________________________________
    public void SetDefaultPixelFormat(PixelFormat pixelFormat)
    {
        wax_setEncoderDefaultPixelFormat(_obj, pixelFormat);
    }

    //____ DefaultPixelFormat() _______________________________________________
    public PixelFormat DefaultPixelFormat()
    {
        return wax_getEncoderDefaultPixelFormat(_obj);
    }

    //____ SetDefaultSampleMethod() ___________________________________________
    public void SetDefaultSampleMethod(SampleMethod sampleMethod)
    {
        wax_setEncoderDefaultSampleMethod(_obj, sampleMethod);
    }

    //____ DefaultSampleMethod() ______________________________________________
    public SampleMethod DefaultSampleMethod()
    {
        return wax_getEncoderDefaultSampleMethod(_obj);
    }

    //____ Flush() ____________________________________________________________
    public void Flush()
    {
        wax_flushEncoder(_obj);
    }
    
    //____ C_Blueprint ________________________________________________________
    [StructLayout(LayoutKind.Sequential)]
    private struct C_Blueprint
    {
        public int bufferBytes;
        public UInt16 objectIdStart;
        public PixelFormat pixelFormat;
        public SampleMethod sampleMethod;
        public C_Component sink;
    }

   //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_createStreamEncoder( ref C_Blueprint bp);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_setEncoderStream(IntPtr encoder, C_Component sinkForStream);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_setEncoderDefaultPixelFormat(IntPtr encoder, PixelFormat pixelFormat);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern PixelFormat wax_getEncoderDefaultPixelFormat(IntPtr encoder);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wax_setEncoderDefaultSampleMethod(IntPtr encoder, SampleMethod sampleMethod);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SampleMethod wax_getEncoderDefaultSampleMethod(IntPtr encoder);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void			wax_flushEncoder(IntPtr encoder);


}
