using System.Runtime.InteropServices;

namespace WG;

public class StreamEncoder : Objekt
{
    public struct Blueprint
    {
        public Blueprint() {}
        int bufferBytes = 4096*2;
        UInt16 objectIdStart = 0;
        PixelFormat pixelFormat = PixelFormat.BGRA_8;
        SampleMethod sampleMethod = SampleMethod.Nearest;
        StreamSink sink;
    }

    private struct C_Blueprint
    {
        int bufferBytes;
        UInt16 objectIdStart;
        PixelFormat pixelFormat;
        SampleMethod sampleMethod;
        C_Component sink;
    }


}
