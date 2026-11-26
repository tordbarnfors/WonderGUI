using System.Runtime.InteropServices;

using Wax;

public struct StreamSink
{
    //____ Constructor ________________________________________________________
    public StreamSink(IntPtr part, Objekt holder)
    {
        c_part.objekt = holder.CHandle();
        c_part.part = part;

        this.holder = holder;
    }

    //____ ProcessChunks() ____________________________________________________
    public unsafe void ProcessChunks(Span<byte> data)
    {
        fixed (byte* ptr = data)
        {
            wax_processStreamChunks(c_part, ptr, ptr + data.Length);
        }
    }

    //____ Data _______________________________________________________________

    internal C_Part c_part;
    private Objekt holder;      // Make sure to keep the object alive as long as we are.

    //____ DLL functions ______________________________________________________

    [DllImport(Objekt.NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern unsafe void	wax_processStreamChunks( C_Part streamSink, byte * pBegin, byte * pEnd );


}