using System.Runtime.InteropServices;

namespace WG;

[StructLayout(LayoutKind.Sequential)]
internal struct C_Component
{
    public IntPtr objekt;
    public IntPtr component;
}