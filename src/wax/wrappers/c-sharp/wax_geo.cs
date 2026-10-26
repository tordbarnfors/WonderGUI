using System.Runtime.InteropServices;

namespace WG;



[StructLayout(LayoutKind.Sequential)]
public record struct CoordI(int X, int Y);

[StructLayout(LayoutKind.Sequential)]
public record struct SizeI(int W, int H);

[StructLayout(LayoutKind.Sequential)]
public record struct BorderI(int Top, int Right, int Bottom, int Left);

[StructLayout(LayoutKind.Sequential)]
public record struct RectI(int X, int Y, int W, int H);

[StructLayout(LayoutKind.Sequential)]
public record struct CoordSPX(int X, int Y);

[StructLayout(LayoutKind.Sequential)]
public record struct SizeSPX(int W, int H);

[StructLayout(LayoutKind.Sequential)]
public record struct BorderSPX( int Top, int Right, int Bottom, int Left);

[StructLayout(LayoutKind.Sequential)]
public record struct RectSPX(int X, int Y, int W, int H);
