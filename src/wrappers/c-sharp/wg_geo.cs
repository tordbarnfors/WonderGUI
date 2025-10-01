using System.Runtime.InteropServices;

namespace WG;



[StructLayout(LayoutKind.Sequential)]
public struct CoordI
{
    public CoordI() { }
    public int x = 0;
    public int y = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct SizeI
{
    public SizeI() {}

    public int w = 0;
    public int h = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct BorderI
{
    public BorderI() {}

    public int top = 0;
    public int right = 0;
    public int bottom = 0;
    public int left = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct RectI
{
    public RectI() { }

    public int x = 0;
    public int y = 0;
    public int w = 0;
    public int h = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct CoordSPX
{
    public CoordSPX() { }

    public int x = 0;
    public int y = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct SizeSPX
{
    public SizeSPX() { }

    public int w = 0;
    public int h = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct BorderSPX
{
    public BorderSPX() { }

    public int top = 0;
    public int right = 0;
    public int bottom = 0;
    public int left = 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct RectSPX
{
    public RectSPX() { }

    public int x = 0;
    public int y = 0;
    public int w = 0;
    public int h = 0;
}
