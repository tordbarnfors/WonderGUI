using System.Runtime.InteropServices;

using WG;

public struct StreamSink
{
    //____ Constructor ________________________________________________________
    public StreamSink(IntPtr component, Objekt holder)
    {
        c_component.objekt = holder.CHandle();
        c_component.component = component;

        this.holder = holder;
    }

    //____ ProcessChunks() ____________________________________________________
    public unsafe void ProcessChunks(Span<byte> data)
    {
        fixed (byte* ptr = data)
        {
            wg_processStreamChunks(c_component, ptr, ptr + data.Length);
        }
    }

    //____ Data _______________________________________________________________

    internal C_Component c_component;
    private Objekt holder;      // Make sure to keep the object alive as long as we are.

    //____ DLL functions ______________________________________________________

    [DllImport(Objekt.NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern unsafe void	wg_processStreamChunks( C_Component streamSink, byte * pBegin, byte * pEnd );


}