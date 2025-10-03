using System.Runtime.InteropServices;

namespace WG;


public class GfxDevice : Objekt
{

    //____ Constructor ________________________________________________________
    public GfxDevice(GfxBackend backend)
    {
        _obj = wg_createGfxDevice(backend.CHandle());
    }

    //____ SetBackend() _______________________________________________________
    public bool SetBackend(GfxBackend backend)
    {
        return (wg_setBackend(_obj, backend.CHandle()) == 1);
    }

    //____ BeginRender() ______________________________________________________
    public bool BeginRender()
    {
        return (wg_beginRender(_obj) == 1);
    }

    //____ EndRender() ________________________________________________________
    public bool EndRender()
    {
        return (wg_endRender(_obj) == 1);
    }

    //____ IsRendering() ______________________________________________________
    public bool IsRendering()
    {
        return (wg_isDeviceRendering(_obj) == 1);
    }

    //____ IsIdle() ___________________________________________________________
    public bool IsIdle()
    {
        return (wg_isDeviceIdle(_obj) == 1);
    }

    //____ Flush() ____________________________________________________________
    public void Flush()
    {
        wg_flushDevice(_obj);
    }

    //____ BeginCanvasUpdate() ________________________________________________

    public bool BeginCanvasUpdate(CanvasRef canvas, RectSPX[]? updateRects = null, CanvasLayers? layers = null, int startLayer = 0)
    {
        int rectCount = updateRects?.Length ?? 0;
        IntPtr layersHandle = layers?.CHandle() ?? 0;
        return (wg_beginCanvasUpdateWithRef(_obj, canvas, rectCount, updateRects, layersHandle, startLayer) == 1);
    }

    public bool BeginCanvasUpdate(Surface canvas, RectSPX[]? updateRects = null, CanvasLayers? layers = null, int startLayer = 0)
    {
        int rectCount = updateRects?.Length ?? 0;
        IntPtr layersHandle = layers?.CHandle() ?? 0;

        return (wg_beginCanvasUpdateWithSurface(_obj, canvas.CHandle(), rectCount, updateRects, layersHandle, startLayer) == 1);
    }

    //____ EndCanvasUpdate() __________________________________________________

    public void EndCanvasUpdate()
    {
        wg_endCanvasUpdate(_obj);
    }

    //____ SetTintColor() _____________________________________________________

    public void SetTintColor(Color color)
    {
        wg_setTintColor(_obj, color);
    }

    //____ GetTintColor() _____________________________________________________

    public Color GetTintColor()
    {
        return wg_getTintColor(_obj);
    }

    //____ ClearTintColor() ___________________________________________________

    public void ClearTintColor()
    {
        wg_clearTintColor(_obj);
    }

    //____ HasTintColor() _____________________________________________________

    public bool HasTintColor()
    {
        return (wg_hasTintColor(_obj) == 1);
    }

    //____ SetBlendMode() _____________________________________________________

    public bool SetBlendMode(BlendMode blendMode)
    {
        return (wg_setBlendMode(_obj, blendMode) == 1);
    }

    //____ BlendMode() ________________________________________________________

    public BlendMode GetBlendMode()
    {
        return wg_getBlendMode(_obj);
    }

    //____ SetBlitSource() ____________________________________________________

    public bool SetBlitSource(Surface blitSource)
    {
        return (wg_setBlitSource(_obj, blitSource.CHandle()) == 1);
    }

    //____ SetMorphFactor() ___________________________________________________

    public void SetMorphFactor(float factor)
    {
        wg_setMorphFactor(_obj, factor);
    }

    //____ GetMorphFactor() ___________________________________________________

    public float GetMorphFactor()
    {
        return wg_getMorphFactor(_obj);
    }

    //____ Fill() _____________________________________________________________

    public void Fill(Color color)
    {
        wg_fill(_obj, color);
    }

    public void Fill(in RectSPX rect, in Color color)
    {
        wg_fillRect(_obj, rect, color);
    }

    //____ DrawLine() _________________________________________________________

    public void DrawLine(CoordSPX begin, CoordSPX end, Color color, int thickness = 64)
    {
        wg_drawLine(_obj, begin, end, color, thickness);
    }

    public void DrawLine(CoordSPX begin, Direction dir, int length, Color color, int thickness = 64)
    {
        wg_drawStraightLine(_obj, begin, dir, length, color, thickness);
    }

    //____ Blit() _____________________________________________________________

    public void Blit(CoordSPX dest)
    {
        wg_blit(_obj, dest);
    }

    public void Blit(CoordSPX dest, in RectSPX src)
    {
        wg_blitRect(_obj, dest, src);
    }

    //____ FlipBlit() _________________________________________________________

    public void FlipBlit(CoordSPX dest, GfxFlip flip)
    {
        wg_flipBlit(_obj, dest, flip);
    }

    public void FlipBlit(CoordSPX dest, in RectSPX src, GfxFlip flip)
    {
        wg_flipBlitRect(_obj, dest, src, flip);
    }

    //____ StretchBlit() ______________________________________________________

    public void StretchBlit(in RectSPX dest)
    {
        wg_stretchBlit(_obj, dest);
    }

    public void StretchBlit(in RectSPX dest, in RectSPX src)
    {
        wg_stretchBlitRect(_obj, dest, src);
    }

    //____ StretchFlipBlit() __________________________________________________

    public void StretchFlipBlit(in RectSPX dest, GfxFlip flip)
    {
        wg_stretchFlipBlit(_obj, dest, flip);
    }

    public void StretchFlipBlit(in RectSPX dest, in RectSPX src, GfxFlip flip)
    {
        wg_stretchFlipBlitRect(_obj, dest, src, flip);
    }


    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_createGfxDevice(IntPtr backend);

    //	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern const wg_typeInfo* wg_deviceSurfaceType(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_setBackend(IntPtr device, IntPtr backend);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_getBackend(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wg_canvasInfo wg_getCanvas(IntPtr device);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wg_canvasInfo wg_getcanvasRef(IntPtr device, wg_canvasRef IntPtr);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_canvasLayers(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_surfaceFactory(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_edgemapFactory(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_maxSegments();

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wg_sizeSPX wg_canvasSize(IntPtr device);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern int wg_setClipList(IntPtr device, int nRectangles, const wg_rectSPX* pRectangles);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_resetClipList(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern int wg_pushClipList(IntPtr device, int nRectangles, const wg_rectSPX* pRectangles);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_popClipList(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern const wg_rectSPX* wg_getClipList(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_clipListSize(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern const wg_rectSPX* wg_clipBounds(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setTintColor(IntPtr device, Color color);
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern Color wg_getTintColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_clearTintColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_hasTintColor(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_setTintmap(IntPtr device, const wg_rectSPX* rect, const IntPtr tintmap);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_getTintmap(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wg_rectSPX wg_getTintmapRect(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_clearTintmap(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_hasTintmap(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_setBlendMode(IntPtr device, BlendMode blendMode);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern BlendMode wg_getBlendMode(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_setBlitSource(IntPtr device, IntPtr surface);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_getBlitSource(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setMorphFactor(IntPtr device, float factor);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern float wg_getMorphFactor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setBlurbrush(IntPtr device, IntPtr brush);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wg_getBlurbrush(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_setFixedBlendColor(IntPtr device, wg_color color );
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wg_color wg_getFixedBlendColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_setRenderLayer(IntPtr device, int layer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_getRenderLayer(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_beginRender(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_endRender(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_isDeviceRendering(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_isDeviceIdle(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_flushDevice(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_beginCanvasUpdateWithRef(IntPtr device, CanvasRef canvas, int nUpdateRects, RectSPX[]? updateRects, IntPtr canvasLayers, int startLayer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wg_beginCanvasUpdateWithSurface(IntPtr device, IntPtr surface, int nUpdateRects, RectSPX[]? updateRects, IntPtr canvasLayers, int startLayer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_endCanvasUpdate(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_flattenLayers(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_clearLayers(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_fill(IntPtr device, Color col);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_fillRect(IntPtr device, in RectSPX rect, Color col);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_drawLine(IntPtr device, CoordSPX begin, CoordSPX end, Color color, int thickness);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_drawStraightLine(IntPtr device, CoordSPX begin, Direction dir, int length, Color color, int thickness);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_blit(IntPtr device, CoordSPX dest);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_blitRect(IntPtr device, CoordSPX dest, in RectSPX src);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_flipBlit(IntPtr device, CoordSPX dest, GfxFlip flip);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_flipBlitRect(IntPtr device, CoordSPX dest, in RectSPX src, GfxFlip flip);
 
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_stretchBlit(IntPtr device, in RectSPX dest);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_stretchBlitRect(IntPtr device, in RectSPX dest, in RectSPX src);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_stretchFlipBlit(IntPtr device, in RectSPX dest, GfxFlip flip);
 
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wg_stretchFlipBlitRect(IntPtr device, in RectSPX dest, in RectSPX src, GfxFlip flip);
 
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_precisionBlit(IntPtr device, const wg_rectSPX* dest, const wg_rectF* srcSPX);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_transformBlit(IntPtr device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform* pTransform);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_rotScaleBlit(IntPtr device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_tile(IntPtr device, const wg_rectSPX* dest, wg_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_flipTile(IntPtr device, const wg_rectSPX* dest, wg_gfxFlip flip, wg_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_scaleTile(IntPtr device, const wg_rectSPX* dest, float scale, wg_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_scaleFlipTile(IntPtr device, const wg_rectSPX* dest, float scale, wg_gfxFlip flip, wg_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_blur(IntPtr device, wg_coordSPX dest);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_blurRect(IntPtr device, wg_coordSPX dest, const wg_rectSPX* src);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_stretchBlur(IntPtr device, const wg_rectSPX* dest);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_stretchBlurRect(IntPtr device, const wg_rectSPX* dest, const wg_rectSPX* src);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_transformBlur(IntPtr device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform* pTransform);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_rotScaleBlur(IntPtr device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_drawEdgemap(IntPtr device, wg_coordSPX dest, IntPtr edgemap);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_flipDrawEdgemap(IntPtr device, wg_coordSPX dest, IntPtr edgemap, wg_gfxFlip flip);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wg_blitNinePatch(IntPtr device, const wg_rectSPX* dstRect, const wg_borderSPX* dstFrame, const wg_ninePatch* patch, int scale);


}
