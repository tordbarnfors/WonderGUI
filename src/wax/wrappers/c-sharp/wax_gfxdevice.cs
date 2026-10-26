using System.Runtime.InteropServices;

namespace Wax;


public class GfxDevice : Objekt
{

    //____ Constructor ________________________________________________________
    public GfxDevice(GfxBackend backend)
    {
        _obj = wax_createGfxDevice(backend.CHandle());
    }

    //____ SetBackend() _______________________________________________________
    public bool SetBackend(GfxBackend backend)
    {
        return (wax_setBackend(_obj, backend.CHandle()) == 1);
    }

    //____ BeginRender() ______________________________________________________
    public bool BeginRender()
    {
        return (wax_beginRender(_obj) == 1);
    }

    //____ EndRender() ________________________________________________________
    public bool EndRender()
    {
        return (wax_endRender(_obj) == 1);
    }

    //____ IsRendering() ______________________________________________________
    public bool IsRendering()
    {
        return (wax_isDeviceRendering(_obj) == 1);
    }

    //____ IsIdle() ___________________________________________________________
    public bool IsIdle()
    {
        return (wax_isDeviceIdle(_obj) == 1);
    }

    //____ Flush() ____________________________________________________________
    public void Flush()
    {
        wax_flushDevice(_obj);
    }

    //____ BeginCanvasUpdate() ________________________________________________

    public bool BeginCanvasUpdate(CanvasRef canvas, RectSPX[]? updateRects = null, CanvasLayers? layers = null, int startLayer = 0)
    {
        int rectCount = updateRects?.Length ?? 0;
        IntPtr layersHandle = layers?.CHandle() ?? 0;
        return (wax_beginCanvasUpdateWithRef(_obj, canvas, rectCount, updateRects, layersHandle, startLayer) == 1);
    }

    public bool BeginCanvasUpdate(Surface canvas, RectSPX[]? updateRects = null, CanvasLayers? layers = null, int startLayer = 0)
    {
        int rectCount = updateRects?.Length ?? 0;
        IntPtr layersHandle = layers?.CHandle() ?? 0;

        return (wax_beginCanvasUpdateWithSurface(_obj, canvas.CHandle(), rectCount, updateRects, layersHandle, startLayer) == 1);
    }

    //____ EndCanvasUpdate() __________________________________________________

    public void EndCanvasUpdate()
    {
        wax_endCanvasUpdate(_obj);
    }

    //____ SetTintColor() _____________________________________________________

    public void SetTintColor(Color color)
    {
        wax_setTintColor(_obj, color);
    }

    //____ GetTintColor() _____________________________________________________

    public Color GetTintColor()
    {
        return wax_getTintColor(_obj);
    }

    //____ ClearTintColor() ___________________________________________________

    public void ClearTintColor()
    {
        wax_clearTintColor(_obj);
    }

    //____ HasTintColor() _____________________________________________________

    public bool HasTintColor()
    {
        return (wax_hasTintColor(_obj) == 1);
    }

    //____ SetBlendMode() _____________________________________________________

    public bool SetBlendMode(BlendMode blendMode)
    {
        return (wax_setBlendMode(_obj, blendMode) == 1);
    }

    //____ BlendMode() ________________________________________________________

    public BlendMode GetBlendMode()
    {
        return wax_getBlendMode(_obj);
    }

    //____ SetBlitSource() ____________________________________________________

    public bool SetBlitSource(Surface blitSource)
    {
        return (wax_setBlitSource(_obj, blitSource.CHandle()) == 1);
    }

    //____ SetMorphFactor() ___________________________________________________

    public void SetMorphFactor(float factor)
    {
        wax_setMorphFactor(_obj, factor);
    }

    //____ GetMorphFactor() ___________________________________________________

    public float GetMorphFactor()
    {
        return wax_getMorphFactor(_obj);
    }

    //____ Fill() _____________________________________________________________

    public void Fill(Color color)
    {
        wax_fill(_obj, color);
    }

    public void Fill(in RectSPX rect, in Color color)
    {
        wax_fillRect(_obj, rect, color);
    }

    //____ DrawLine() _________________________________________________________

    public void DrawLine(CoordSPX begin, CoordSPX end, Color color, int thickness = 64)
    {
        wax_drawLine(_obj, begin, end, color, thickness);
    }

    public void DrawLine(CoordSPX begin, Direction dir, int length, Color color, int thickness = 64)
    {
        wax_drawStraightLine(_obj, begin, dir, length, color, thickness);
    }

    //____ Blit() _____________________________________________________________

    public void Blit(CoordSPX dest)
    {
        wax_blit(_obj, dest);
    }

    public void Blit(CoordSPX dest, in RectSPX src)
    {
        wax_blitRect(_obj, dest, src);
    }

    //____ FlipBlit() _________________________________________________________

    public void FlipBlit(CoordSPX dest, GfxFlip flip)
    {
        wax_flipBlit(_obj, dest, flip);
    }

    public void FlipBlit(CoordSPX dest, in RectSPX src, GfxFlip flip)
    {
        wax_flipBlitRect(_obj, dest, src, flip);
    }

    //____ StretchBlit() ______________________________________________________

    public void StretchBlit(in RectSPX dest)
    {
        wax_stretchBlit(_obj, dest);
    }

    public void StretchBlit(in RectSPX dest, in RectSPX src)
    {
        wax_stretchBlitRect(_obj, dest, src);
    }

    //____ StretchFlipBlit() __________________________________________________

    public void StretchFlipBlit(in RectSPX dest, GfxFlip flip)
    {
        wax_stretchFlipBlit(_obj, dest, flip);
    }

    public void StretchFlipBlit(in RectSPX dest, in RectSPX src, GfxFlip flip)
    {
        wax_stretchFlipBlitRect(_obj, dest, src, flip);
    }


    //____ DLL functions ______________________________________________________

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_createGfxDevice(IntPtr backend);

    //	[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern const wax_typeInfo* wax_deviceSurfaceType(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_setBackend(IntPtr device, IntPtr backend);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_getBackend(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wax_canvasInfo wax_getCanvas(IntPtr device);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wax_canvasInfo wax_getcanvasRef(IntPtr device, wax_canvasRef IntPtr);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_canvasLayers(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_surfaceFactory(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_edgemapFactory(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_maxSegments();

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wax_sizeSPX wax_canvasSize(IntPtr device);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern int wax_setClipList(IntPtr device, int nRectangles, const wax_rectSPX* pRectangles);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_resetClipList(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern int wax_pushClipList(IntPtr device, int nRectangles, const wax_rectSPX* pRectangles);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_popClipList(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern const wax_rectSPX* wax_getClipList(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_clipListSize(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern const wax_rectSPX* wax_clipBounds(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setTintColor(IntPtr device, Color color);
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern Color wax_getTintColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_clearTintColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_hasTintColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_setBlendMode(IntPtr device, BlendMode blendMode);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern BlendMode wax_getBlendMode(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_setBlitSource(IntPtr device, IntPtr surface);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_getBlitSource(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setMorphFactor(IntPtr device, float factor);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern float wax_getMorphFactor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setBlurbrush(IntPtr device, IntPtr brush);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr wax_getBlurbrush(IntPtr device);

    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_setFixedBlendColor(IntPtr device, wax_color color );
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern wax_color wax_getFixedBlendColor(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_setRenderLayer(IntPtr device, int layer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_getRenderLayer(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_beginRender(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_endRender(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_isDeviceRendering(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_isDeviceIdle(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_flushDevice(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_beginCanvasUpdateWithRef(IntPtr device, CanvasRef canvas, int nUpdateRects, RectSPX[]? updateRects, IntPtr canvasLayers, int startLayer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern int wax_beginCanvasUpdateWithSurface(IntPtr device, IntPtr surface, int nUpdateRects, RectSPX[]? updateRects, IntPtr canvasLayers, int startLayer);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_endCanvasUpdate(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_flattenLayers(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_clearLayers(IntPtr device);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_fill(IntPtr device, Color col);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_fillRect(IntPtr device, in RectSPX rect, Color col);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_drawLine(IntPtr device, CoordSPX begin, CoordSPX end, Color color, int thickness);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_drawStraightLine(IntPtr device, CoordSPX begin, Direction dir, int length, Color color, int thickness);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_blit(IntPtr device, CoordSPX dest);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_blitRect(IntPtr device, CoordSPX dest, in RectSPX src);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_flipBlit(IntPtr device, CoordSPX dest, GfxFlip flip);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_flipBlitRect(IntPtr device, CoordSPX dest, in RectSPX src, GfxFlip flip);
 
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_stretchBlit(IntPtr device, in RectSPX dest);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_stretchBlitRect(IntPtr device, in RectSPX dest, in RectSPX src);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_stretchFlipBlit(IntPtr device, in RectSPX dest, GfxFlip flip);
 
    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    private static extern void wax_stretchFlipBlitRect(IntPtr device, in RectSPX dest, in RectSPX src, GfxFlip flip);
 
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_precisionBlit(IntPtr device, const wax_rectSPX* dest, const wax_rectF* srcSPX);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_transformBlit(IntPtr device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform* pTransform);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_rotScaleBlit(IntPtr device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_tile(IntPtr device, const wax_rectSPX* dest, wax_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_flipTile(IntPtr device, const wax_rectSPX* dest, wax_gfxFlip flip, wax_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_scaleTile(IntPtr device, const wax_rectSPX* dest, float scale, wax_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_scaleFlipTile(IntPtr device, const wax_rectSPX* dest, float scale, wax_gfxFlip flip, wax_coordSPX shift);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_blur(IntPtr device, wax_coordSPX dest);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_blurRect(IntPtr device, wax_coordSPX dest, const wax_rectSPX* src);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_stretchBlur(IntPtr device, const wax_rectSPX* dest);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_stretchBlurRect(IntPtr device, const wax_rectSPX* dest, const wax_rectSPX* src);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_transformBlur(IntPtr device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform* pTransform);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_rotScaleBlur(IntPtr device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_drawEdgemap(IntPtr device, wax_coordSPX dest, IntPtr edgemap);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_flipDrawEdgemap(IntPtr device, wax_coordSPX dest, IntPtr edgemap, wax_gfxFlip flip);
    //[DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl)]
    //private static extern void wax_blitNinePatch(IntPtr device, const wax_rectSPX* dstRect, const wax_borderSPX* dstFrame, const wax_ninePatch* patch, int scale);


}
