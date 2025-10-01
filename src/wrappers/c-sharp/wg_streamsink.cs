using System.Runtime.InteropServices;

using WG;

public struct StreamSink
{
    public StreamSink(IntPtr component, Objekt holder)
    {
        c_component.objekt = holder.CHandle();
        c_component.component = component;
    }

    public unsafe void ProcessChunks(Span<byte> data)
    {
        fixed (byte* ptr = data)
        {
            wg_processStreamChunks(c_component, ptr, ptr + data.Length);
        }
    }


    internal C_Component c_component;

    //____ DLL functions ______________________________________________________

    [DllImport(Objekt.NativeLib, CallingConvention = CallingConvention.Cdecl)]
	private static extern unsafe void	wg_processStreamChunks( C_Component streamSink, byte * pBegin, byte * pEnd );


}