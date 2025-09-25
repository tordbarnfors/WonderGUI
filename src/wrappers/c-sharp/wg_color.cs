namespace WG;


[StructLayout(LayoutKind.Sequential)]
public struct Color
{
	public short	b;
	public short	g;
	public short	r;
	public short	a;

	public Color()
	{
		this.r = 0;
		this.g = 0;
		this.b = 0;
		this.a = 0;
	}


	public Color( int r, int g, int b, int a = 4096 )
	{
		this.r = r;
		this.g = g;
		this.b = b;
		this.a = a;
	}

	public void clamp()
	{
		Math.clamp(b,0,4096);
		Math.clamp(g,0,4096);
		Math.clamp(r,0,4096);
		Math.clamp(a,0,4096);
	}

	public Color withAlpha(int alpha)
	{
		return Color(r,g,b,alpha);
	}

}