using System.Runtime.InteropServices;

namespace WG;


[StructLayout(LayoutKind.Sequential)]
public struct Color8
{
	public Color8()
	{
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}

	public Byte b;
	public Byte g;
	public Byte r;
	public Byte a;
}


[StructLayout(LayoutKind.Sequential)]
public struct Color
{
	public short b;
	public short g;
	public short r;
	public short a;

	public Color()
	{
		this.r = 0;
		this.g = 0;
		this.b = 0;
		this.a = 0;
	}

	public Color(int r, int g, int b, int a = 4096)
	{
		this.r = (short) r;
		this.g = (short) g;
		this.b = (short) b;
		this.a = (short) a;
	}

	public void Clamp()
	{
		r = Math.Clamp(r, (short) 0, (short) 4096);
		g = Math.Clamp(g, (short) 0, (short) 4096);
		b = Math.Clamp(b, (short) 0, (short) 4096);
		a = Math.Clamp(a, (short) 0, (short) 4096);
	}

	public Color WithAlpha(int alpha)
	{
		return new Color(r, g, b, alpha);
	}

}