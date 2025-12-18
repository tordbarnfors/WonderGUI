using System.Reflection.Metadata.Ecma335;
using System.Runtime.InteropServices;

namespace WG;

public class MirrorSurface : Surface
{
    public new struct Blueprint
    {
        public Blueprint() { }

        public CanvasRef   CanvasRef = CanvasRef.None;  // Stream to specified reference, otherwise create surface and stream to that.
        public StreamEncoder Encoder = null!;           // Required
        public Surface     Surface = null!;             // Required
        public bool StreamOnCreate = false;             // If true, surface content will be streamed immediately after creation.
    }


    internal MirrorSurface(IntPtr c_obj)
	{
		_obj = c_obj;
	}

	public MirrorSurface(StreamEncoder encoder, ref Blueprint blueprint)
	{
		C_Blueprint bp;
        bp.canvasRef = blueprint.CanvasRef;
        bp.encoder = encoder.CHandle();
        bp.surface = blueprint.Surface.CHandle();
        bp.streamOnCreate = blueprint.StreamOnCreate ? 1 : 0;

		_obj = wg_createMirrorSurface(encoder.CHandle(), ref bp);
	}

    [StructLayout(LayoutKind.Sequential)]
    public new struct C_Blueprint
    {
        public CanvasRef   canvasRef;          // Stream to specified reference, otherwise create surface and stream to that.
        public IntPtr      encoder;            // Required
        public IntPtr      surface;            // Required
        public int         streamOnCreate;		// If true, surface will be streamed immediately after creation.
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wg_createMirrorSurface(IntPtr streamEncoder, ref C_Blueprint blueprint);

}
