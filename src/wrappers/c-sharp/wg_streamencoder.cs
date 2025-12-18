using System.Runtime.InteropServices;

namespace WG;

public class StreamEncoder : Objekt
{
    //____ Blueprint __________________________________________________________
    public struct Blueprint
    {
        public Blueprint() { }
        public int BufferBytes = 4096 * 2;
        public UInt16 ObjectIdStart = 0;
        public PixelFormat PixelFormat = PixelFormat.BGRA_8;
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

        _obj = wg_createStreamEncoder(ref c_bp);
    }

    //____ SetStream() ________________________________________________________
    public void SetStream(StreamSink stream)
    {
        wg_setEncoderStream(_obj, stream.c_component);
    }

    //____ SetDefaultPixelFormat() ____________________________________________
    public void SetDefaultPixelFormat(PixelFormat pixelFormat)
    {
        wg_setEncoderDefaultPixelFormat(_obj, pixelFormat);
    }

    //____ DefaultPixelFormat() _______________________________________________
    public PixelFormat DefaultPixelFormat()
    {
        return wg_getEncoderDefaultPixelFormat(_obj);
    }

    //____ SetDefaultSampleMethod() ___________________________________________
    public void SetDefaultSampleMethod(SampleMethod sampleMethod)
    {
        wg_setEncoderDefaultSampleMethod(_obj, sampleMethod);
    }

    //____ DefaultSampleMethod() ______________________________________________
    public SampleMethod DefaultSampleMethod()
    {
        return wg_getEncoderDefaultSampleMethod(_obj);
    }

    //____ Flush() ____________________________________________________________
    public void Flush()
    {
        wg_flushEncoder(_obj);
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
	private static extern IntPtr wg_createStreamEncoder( ref C_Blueprint bp);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_setEncoderStream(IntPtr encoder, C_Component sinkForStream);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_setEncoderDefaultPixelFormat(IntPtr encoder, PixelFormat pixelFormat);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern PixelFormat wg_getEncoderDefaultPixelFormat(IntPtr encoder);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void wg_setEncoderDefaultSampleMethod(IntPtr encoder, SampleMethod sampleMethod);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern SampleMethod wg_getEncoderDefaultSampleMethod(IntPtr encoder);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern void			wg_flushEncoder(IntPtr encoder);


}
