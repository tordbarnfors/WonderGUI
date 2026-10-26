using System.Runtime.InteropServices;

namespace WG;

[StructLayout(LayoutKind.Sequential)]
public record struct Color8(byte B, byte G, byte R, byte A);

[StructLayout(LayoutKind.Sequential)]
public record struct Color(short B, short G, short R, short A = 4096)
{
	public void Clamp()
	{
		R = Math.Clamp(R, (short) 0, (short) 4096);
		G = Math.Clamp(G, (short) 0, (short) 4096);
		B = Math.Clamp(B, (short) 0, (short) 4096);
		A = Math.Clamp(A, (short) 0, (short) 4096);
	}

	public Color WithAlpha(int alpha)
	{
		return new Color(R, G, B, (short) alpha);
	}

}