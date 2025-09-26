using System.Runtime.InteropServices;

namespace WG;



[StructLayout(LayoutKind.Sequential)]
public struct CoordI
{
    public int x;
    public int y;
}

[StructLayout(LayoutKind.Sequential)]
public struct SizeI
{
    public int w;
    public int h;
}

[StructLayout(LayoutKind.Sequential)]
public struct BorderI
{
    public int top;
    public int right;
    public int bottom;
    public int left;
}

[StructLayout(LayoutKind.Sequential)]
public struct RectI
{
    public int x;
    public int y;
    public int w;
    public int h;
}

[StructLayout(LayoutKind.Sequential)]
public struct CoordSPX
{
    public int x;
    public int y;
}

[StructLayout(LayoutKind.Sequential)]
public struct SizeSPX
{
    public int w;
    public int h;
}

[StructLayout(LayoutKind.Sequential)]
public struct BorderSPX
{
    public int top;
    public int right;
    public int bottom;
    public int left;
}

[StructLayout(LayoutKind.Sequential)]
public struct RectSPX
{
    public int x;
    public int y;
    public int w;
    public int h;
}
