


namespace WG;

public static class GfxBase
{
    //____ Init() _________________________________________________________

    bool Init()
    {
        int ret = wg_initGfxBase();

        return (ret == 1);
    }

    //____ Exit() _________________________________________________________

    bool Exit()
    {
        int ret = wg_exitGfxBase();

        return (ret == 1);
    }

    //____ IsInitialized() ________________________________________________

    bool IsInitialized()
    {
        int ret = wg_initGfxBase();

        return (ret == 1);
    }


    [DllImport("libstreamgendll.so")]
    private static extern int        wg_initGfxBase();

    [DllImport("libstreamgendll.so")]
    private static extern int        wg_exitGfxBase();

    [DllImport("libstreamgendll.so")]
    private static extern int        wg_isGfxBaseInitialized();

}
