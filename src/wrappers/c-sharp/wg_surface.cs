



namespace WG;


public class Surface : Objekt
{

	public struct Blueprint
	{
		bool				buffered;
		bool				canvas;
//		const wg_color8* 	palette;				HOW DO WE DO THIS?
		int					paletteSize;
		int					paletteCapacity;
		bool				dynamic;
		PixelFormat			format;
		int					identity;
		bool				mipmap;
		SampleMethod		sampleMethod;
		int					scale;
		SizeI				size;					// Mandatory, except when creating from other surface.
		bool				tiling;
	}


	public void SetId( int id)
	{
		wg_setSuraceIdentity(_obj, id);
	}

	public int Id()
	{
		return wg_getSurfaceIdentity(_obj);
	}

	public int Id()
	{
		return wg_getSurfaceIdentity(_obj);
	}

	public SizeI SizeInPixels()
	{
		return wg_surfacePixelSize(_obj)
	}


	[StructLayout(LayoutKind.Sequential)]
	protected struct wg_surfaceBP
	{
	    byte                buffered;
	    byte                canvas;
	    Color[]             palette;
	    int                 paletteCapacity;
	    byte                dynamic;
	    wg_pixelFormat      format;
	    int                 identity;
	    byte                mipmap;
	    wg_sampleMethod     sampleMethod;
	    int                 scale;
	    wg_sizeI            size;                   // Mandatory, except when creating from other surface.
	    byte                tiling;
	}



   [DllImport("libstreamgendll.so")]
    private static extern void			wg_setSurfaceIdentity(IntPtr surface, int id);

    [DllImport("libstreamgendll.so")]
    private static extern int			wg_getSurfaceIdentity(IntPtr surface);

    [DllImport("libstreamgendll.so")]
    private static extern SizeI			wg_surfacePixelSize(IntPtr surface);						///< @brief Get the size in pixels of the surface.


}
