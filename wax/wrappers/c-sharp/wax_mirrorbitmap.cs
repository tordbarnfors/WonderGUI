using System.Reflection.Metadata.Ecma335;
using System.Runtime.InteropServices;

namespace Wax;

public class MirrorBitmap : Bitmap
{
    public new struct Blueprint
    {
        public Blueprint() { }

        public CanvasRef   CanvasRef = CanvasRef.None;  // Stream to specified reference, otherwise create bitmap and stream to that.
        public StreamEncoder Encoder = null!;           // Required
        public Bitmap     Bitmap = null!;             // Required
        public bool StreamOnCreate = false;             // If true, bitmap content will be streamed immediately after creation.
    }


    internal MirrorBitmap(IntPtr c_obj)
	{
		_obj = c_obj;
	}

	public MirrorBitmap(StreamEncoder encoder, ref Blueprint blueprint)
	{
		C_Blueprint bp;
        bp.canvasRef = blueprint.CanvasRef;
        bp.encoder = encoder.CHandle();
        bp.bitmap = blueprint.Bitmap.CHandle();
        bp.streamOnCreate = blueprint.StreamOnCreate ? 1 : 0;

		_obj = wax_createMirrorBitmap(encoder.CHandle(), ref bp);
	}

    [StructLayout(LayoutKind.Sequential)]
    public new struct C_Blueprint
    {
        public CanvasRef   canvasRef;          // Stream to specified reference, otherwise create bitmap and stream to that.
        public IntPtr      encoder;            // Required
        public IntPtr      bitmap;            // Required
        public int         streamOnCreate;		// If true, bitmap will be streamed immediately after creation.
    }

    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern IntPtr wax_createMirrorBitmap(IntPtr streamEncoder, ref C_Blueprint blueprint);

}
