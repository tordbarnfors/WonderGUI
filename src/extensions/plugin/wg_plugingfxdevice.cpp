/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_plugincalls.h>
#include <wg_plugingfxdevice.h>
#include <wg_pluginsurface.h>
#include <wg_pluginedgemap.h>
#include <wg_pluginsurfacefactory.h>
#include <wg_plugincanvaslayers.h>

#include <wg_base.h>
#include <assert.h>

using namespace std;

namespace wg
{
	const TypeInfo PluginGfxDevice::TYPEINFO = { "PluginGfxDevice", &GfxDevice::TYPEINFO };



	//____ create() _______________________________________________________________

	PluginGfxDevice_p PluginGfxDevice::create( wg_obj object, PluginSurfaceFactory * pSurfaceFactory, PluginEdgemapFactory * pEdgemapFactory )
	{
		PluginGfxDevice_p p(new PluginGfxDevice( object, pSurfaceFactory, pEdgemapFactory ));
		return p;
	}


	//____ constructor _____________________________________________________________

	PluginGfxDevice::PluginGfxDevice( wg_obj object, PluginSurfaceFactory * pSurfaceFactory, PluginEdgemapFactory * pEdgemapFactory )
	{
        PluginCalls::object->retain(object);
		m_cDevice = object;
        m_pSurfaceFactory = pSurfaceFactory;
        m_pEdgemapFactory = pEdgemapFactory;
        m_bRendering = false;
		m_bIsProxyDevice = true;
		
		// Incomplete initialization of m_canvas.
		
		auto canvasSize = PluginCalls::gfxDevice->canvasSize(object);
		
		m_canvas.size.w = canvasSize.w;
		m_canvas.size.h = canvasSize.h;
	}

	//____ Destructor ______________________________________________________________

	PluginGfxDevice::~PluginGfxDevice()
	{
        PluginCalls::object->release(m_cDevice);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginGfxDevice::typeInfo(void) const
	{
		return TYPEINFO;
	}

    //____ canvas() ___________________________________________________________

    const CanvasInfo PluginGfxDevice::canvas(CanvasRef ref) const
    {
        auto info = PluginCalls::gfxDevice->getCanvasRef(m_cDevice, (wg_canvasRef)ref);

        CanvasInfo info2;
        info2.ref = (CanvasRef) info.ref;
        info2.scale = info.scale;
        info2.size.w = info.size.w;
        info2.size.h = info.size.h;
        info2.pSurface = nullptr;       // Not supported. Can't give access to C++ object on other side of CABI-wall.
        return info2;
    }

	//____ surfaceType() _______________________________________________________

	const TypeInfo& PluginGfxDevice::surfaceType( void ) const
	{
		return PluginSurface::TYPEINFO;
	}

	//____ surfaceFactory() ______________________________________________________

	SurfaceFactory_p PluginGfxDevice::surfaceFactory()
	{
        return m_pSurfaceFactory;
	}

    //____ edgemapFactory() ______________________________________________________

    EdgemapFactory_p PluginGfxDevice::edgemapFactory()
    {
        return m_pEdgemapFactory;
    }

    //____ setClipList() _________________________________________________________

    bool PluginGfxDevice::setClipList(int nRectangles, const RectSPX * pRectangles)
    {
        int retVal = PluginCalls::gfxDevice->setClipList(m_cDevice, nRectangles, (wg_rectSPX*)pRectangles);

        if (retVal)
            GfxDeviceGen1::setClipList(nRectangles, pRectangles);

        return retVal;
    }

    //____ resetClipList() _______________________________________________________

    void PluginGfxDevice::resetClipList()
    {
        PluginCalls::gfxDevice->resetClipList(m_cDevice);
        GfxDeviceGen1::resetClipList();
    }

    //____ pushClipList() ________________________________________________________

    bool PluginGfxDevice::pushClipList(int nRectangles, const RectSPX* pRectangles)
    {
        int retVal = PluginCalls::gfxDevice->pushClipList(m_cDevice, nRectangles, (wg_rectSPX*)pRectangles);

        if( retVal )
            GfxDeviceGen1::pushClipList(nRectangles, pRectangles);

        return retVal;
    }

    //____ popClipList() ______________________________________________________

    bool PluginGfxDevice::popClipList()
    {
        int retVal = PluginCalls::gfxDevice->popClipList(m_cDevice);
        if( retVal )
            GfxDeviceGen1::popClipList();
        
        return retVal;
    }

    //____ clipList() _________________________________________________________

    const RectSPX* PluginGfxDevice::clipList() const
    {
        return (RectSPX*) PluginCalls::gfxDevice->getClipList(m_cDevice);
    }

    //____ clipListSize() _____________________________________________________

    int PluginGfxDevice::clipListSize() const
    {
        return PluginCalls::gfxDevice->clipListSize(m_cDevice);
    }

    //____ clipBounds() _______________________________________________________

    const RectSPX& PluginGfxDevice::clipBounds() const
    {
        return  * (RectSPX*) PluginCalls::gfxDevice->clipBounds(m_cDevice);
    }

    //____ setTintColor() _____________________________________________________

    void PluginGfxDevice::setTintColor( HiColor color )
    {
        GfxDeviceGen1::setTintColor(color);

        PluginCalls::gfxDevice->setTintColor(m_cDevice, *(wg_color*)&color);
    }

    //____ tintColor() ________________________________________________________

    HiColor PluginGfxDevice::tintColor() const
    {
        wg_color col = PluginCalls::gfxDevice->getTintColor(m_cDevice);

        return *(HiColor*)&col;
    }

    //____ setTintGradient() __________________________________________________

    void PluginGfxDevice::setTintGradient(const RectSPX& rect, const Gradient& gradient)
    {
        GfxDeviceGen1:setTintGradient(rect, gradient);
        
        PluginCalls::gfxDevice->setTintGradient(m_cDevice, (const wg_rectSPX*)&rect, (const wg_gradient*)&gradient);
    }

    //____ clearTintGradient() ________________________________________________

    void PluginGfxDevice::clearTintGradient()
    {
        GfxDeviceGen1::clearTintGradient();

        PluginCalls::gfxDevice->clearTintGradient(m_cDevice);
    }

    //____ setBlendMode() __________________________________________________________

    bool PluginGfxDevice::setBlendMode( BlendMode blendMode )
    {
        if( blendMode < BlendMode_min || blendMode > BlendMode_max )
            return false;

        int retVal = PluginCalls::gfxDevice->setBlendMode(m_cDevice, (wg_blendMode)blendMode);

        if( retVal )
            GfxDeviceGen1::setBlendMode(blendMode);

        return retVal;
    }

    //____ blendMode() ________________________________________________________

    BlendMode  PluginGfxDevice::blendMode() const
    {
        return (BlendMode) PluginCalls::gfxDevice->getBlendMode(m_cDevice);
    }


    //____ setBlitSource() _______________________________________________________

    bool PluginGfxDevice::setBlitSource(Surface * pSource)
    {
        if (!pSource || !pSource->isInstanceOf(PluginSurface::TYPEINFO) )
            return false;

        int retVal = PluginCalls::gfxDevice->setBlitSource(m_cDevice, static_cast<PluginSurface*>(pSource)->cObject());

        if( retVal )
            GfxDeviceGen1::setBlitSource(pSource);

        return retVal;
    }

    //____ PluginGfxDevice::blitSource() ________________________________________

    Surface_p PluginGfxDevice::blitSource() const
    {
        // This method can not be correctly implemented without huge performance loss,
		// so we just assume that blitSource also was set through plugin.

        return GfxDeviceGen1::blitSource();
    }

    //____ setMorphFactor() ______________________________________________________

    void PluginGfxDevice::setMorphFactor(float factor)
    {
        GfxDeviceGen1::setMorphFactor(factor);

        PluginCalls::gfxDevice->setMorphFactor(m_cDevice, m_morphFactor);
    }

    //____ morphFactor() ______________________________________________________

    float PluginGfxDevice::morphFactor() const
    {
        return PluginCalls::gfxDevice->getMorphFactor(m_cDevice);
    }

	//____ setBlurbrush() _____________________________________________________

	void PluginGfxDevice::setBlurbrush( Blurbrush * pBrush )
	{
		GfxDeviceGen1::setBlurbrush(pBrush);

		return PluginCalls::gfxDevice->setBlurbrush(m_cDevice, static_cast<PluginBlurbrush*>(pBrush)->cObject());
	}

	//____ setFixedBlendColor() __________________________________________________

	void PluginGfxDevice::setFixedBlendColor( HiColor color )
	{
		GfxDeviceGen1::setFixedBlendColor(color);

		PluginCalls::gfxDevice->setFixedBlendColor(m_cDevice, *(wg_color*)&color);
	}

	//____ fixedBlendColor() _____________________________________________________

	HiColor PluginGfxDevice::fixedBlendColor() const
	{
		wg_color col = PluginCalls::gfxDevice->getFixedBlendColor(m_cDevice);

		return *(HiColor*)&col;
	}

    //____ setRenderLayer() _______________________________________________________

    void PluginGfxDevice::setRenderLayer(int layer)
    {
        GfxDeviceGen1::setRenderLayer(layer);
       
        PluginCalls::gfxDevice->setRenderLayer(m_cDevice, m_renderLayer);
    }

    //____ renderLayer() ______________________________________________________

    int PluginGfxDevice::renderLayer() const
    {
        return PluginCalls::gfxDevice->getRenderLayer(m_cDevice);
    }

	//____ beginRender() ___________________________________________________________

	bool PluginGfxDevice::beginRender()
	{
        int bOk = PluginCalls::gfxDevice->beginRender(m_cDevice);

        if (bOk)
            GfxDeviceGen1::beginRender();
        return bOk;
	}

	//____ endRender() _____________________________________________________________

	bool PluginGfxDevice::endRender()
	{
        int bOk = PluginCalls::gfxDevice->endRender(m_cDevice);

        if (bOk)
            GfxDeviceGen1::endRender();
        return bOk;
    }

    //____ flush() ___________________________________________________________________

    void PluginGfxDevice::flush()
    {
        PluginCalls::gfxDevice->flushDevice(m_cDevice);
        GfxDeviceGen1::flush();
    }

    //____ endCanvasUpdate() _________________________________________________________

    void PluginGfxDevice::endCanvasUpdate()
    {
		GfxDeviceGen1::endCanvasUpdate();
        PluginCalls::gfxDevice->endCanvasUpdate(m_cDevice);
    }

    //____ fill() __________________________________________________________________

    void PluginGfxDevice::fill( HiColor _col )
    {
        PluginCalls::gfxDevice->fill( m_cDevice, *(wg_color*)&_col );
    }

    void PluginGfxDevice::fill( const RectSPX& _rect, HiColor _col )
    {
        PluginCalls::gfxDevice->fillRect(m_cDevice, (wg_rectSPX*) &_rect, *(wg_color*)&_col);
    }

    //____ plotPixels() ________________________________________________________

    void PluginGfxDevice::plotPixels(int nCoords, const CoordSPX * pCoords, const HiColor * pColors)
    {
        PluginCalls::gfxDevice->plotPixels(m_cDevice, nCoords, (wg_coordSPX*)pCoords, (wg_color*)pColors);
    }

    //____ drawLine() __________________________________________________________

    void PluginGfxDevice::drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness)
    {
        PluginCalls::gfxDevice->drawLine(m_cDevice, { begin.x, begin.y }, { end.x, end.y }, *(wg_color*)&color, thickness);
    }

    void PluginGfxDevice::drawLine(CoordSPX begin, Direction dir, int length, HiColor color, spx thickness)
    {
        PluginCalls::gfxDevice->drawStraightLine(m_cDevice, { begin.x, begin.y }, (wg_direction) dir, length, * (wg_color*)&color, thickness);
    }

    //____ blit() __________________________________________________________________

    void PluginGfxDevice::blit(CoordSPX dest)
    {
        PluginCalls::gfxDevice->blit(m_cDevice, { dest.x, dest.y });
    }

    void PluginGfxDevice::blit(CoordSPX dest, const RectSPX& src)
    {
        PluginCalls::gfxDevice->blitRect(m_cDevice, { dest.x, dest.y }, (wg_rectSPX*) &src );
    }

    //____ flipBlit() _________________________________________________________

    void PluginGfxDevice::flipBlit(CoordSPX dest, GfxFlip flip )
    {
        PluginCalls::gfxDevice->flipBlit(m_cDevice, { dest.x, dest.y }, (wg_gfxFlip) flip);
    }

    void PluginGfxDevice::flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip )
    {
        PluginCalls::gfxDevice->flipBlitRect(m_cDevice, { dest.x, dest.y }, (wg_rectSPX*)&src, (wg_gfxFlip)flip);
    }

    //____ stretchBlit() ______________________________________________________

    void PluginGfxDevice::stretchBlit(const RectSPX& dest)
    {
        PluginCalls::gfxDevice->stretchBlit(m_cDevice, (wg_rectSPX*) &dest);
    }

    void PluginGfxDevice::stretchBlit(const RectSPX& dest, const RectSPX& source)
    {
        PluginCalls::gfxDevice->stretchBlitRect(m_cDevice, (wg_rectSPX*)&dest, (wg_rectSPX*)&source);
    }

    //____ stretchFlipBlit() __________________________________________________

    void PluginGfxDevice::stretchFlipBlit(const RectSPX& dest, GfxFlip flip)
    {
        PluginCalls::gfxDevice->stretchFlipBlit(m_cDevice, (wg_rectSPX*)&dest, (wg_gfxFlip) flip);
    }

    void PluginGfxDevice::stretchFlipBlit(const RectSPX& dest, const RectSPX& source, GfxFlip flip)
    {
        PluginCalls::gfxDevice->stretchFlipBlitRect(m_cDevice, (wg_rectSPX*)&dest, (wg_rectSPX*)&source, (wg_gfxFlip)flip );
    }

	//____ precisionBlit() _______________________________________________________

	void PluginGfxDevice::precisionBlit(const RectSPX& dest, const RectF& srcSPX)
	{
        PluginCalls::gfxDevice->precisionBlit(m_cDevice, (wg_rectSPX*)&dest, (wg_rectF*)&srcSPX);
	}

	//____ transformBlit() _______________________________________________________

	void PluginGfxDevice::transformBlit(const RectSPX& dest, CoordF srcSPX, const Transform& transform)
	{
        PluginCalls::gfxDevice->transformBlit(m_cDevice, (wg_rectSPX*)&dest, {srcSPX.x,srcSPX.y}, reinterpret_cast<const wg_transform*>(&transform) );
	}

    //____ rotScaleBlit() _____________________________________________________
    void PluginGfxDevice::rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
    {
        PluginCalls::gfxDevice->rotScaleBlit(m_cDevice, (wg_rectSPX*)&dest, rotationDegrees, scale, { srcCenter.x, srcCenter.y }, { destCenter.x, destCenter.y });
    }

    //____ tile() _____________________________________________________________

    void PluginGfxDevice::tile(const RectSPX& dest, CoordSPX shift)
    {
        PluginCalls::gfxDevice->tile(m_cDevice, (wg_rectSPX*)&dest, { shift.x,shift.y });
    }

    //____ flipTile() _________________________________________________________

    void PluginGfxDevice::flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift)
    {
        PluginCalls::gfxDevice->flipTile(m_cDevice, (wg_rectSPX*)&dest, (wg_gfxFlip) flip, { shift.x, shift.y });
    }

    //____ scaleTile() ________________________________________________________

    void PluginGfxDevice::scaleTile(const RectSPX& dest, float scale, CoordSPX shift)
    {
        PluginCalls::gfxDevice->scaleTile(m_cDevice, (wg_rectSPX*)&dest, scale, { shift.x,shift.y });
    }

    //____ scaleFlipTile() ____________________________________________________

    void PluginGfxDevice::scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift)
    {
        PluginCalls::gfxDevice->scaleFlipTile(m_cDevice, (wg_rectSPX*)&dest, scale, (wg_gfxFlip) flip, { shift.x, shift.y });
    }

	//____ blur() ________________________________________________________________

	void PluginGfxDevice::blur(CoordSPX dest)
	{
		PluginCalls::gfxDevice->blur(m_cDevice, { dest.x, dest.y });
	}

	void PluginGfxDevice::blur(CoordSPX dest, const RectSPX& src)
	{
		PluginCalls::gfxDevice->blurRect(m_cDevice, { dest.x, dest.y }, (wg_rectSPX*) &src );
	}

	//____ stretchBlur() _________________________________________________________

	void PluginGfxDevice::stretchBlur(const RectSPX& dest)
	{
		PluginCalls::gfxDevice->stretchBlur(m_cDevice, (wg_rectSPX*) &dest);
	}

	void PluginGfxDevice::stretchBlur(const RectSPX& dest, const RectSPX& source)
	{
		PluginCalls::gfxDevice->stretchBlurRect(m_cDevice, (wg_rectSPX*)&dest, (wg_rectSPX*)&source);
	}

	//____ transformBlur() _______________________________________________________

	void PluginGfxDevice::transformBlur(const RectSPX& dest, CoordF srcSPX, const Transform& transform)
	{
		PluginCalls::gfxDevice->transformBlur(m_cDevice, (wg_rectSPX*)&dest, {srcSPX.x,srcSPX.y}, reinterpret_cast<const wg_transform*>(&transform) );
	}

	//____ rotScaleBlur() ________________________________________________________

	void PluginGfxDevice::rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
	{
		PluginCalls::gfxDevice->rotScaleBlur(m_cDevice, (wg_rectSPX*)&dest, rotationDegrees, scale, { srcCenter.x, srcCenter.y }, { destCenter.x, destCenter.y });
	}

    //____ drawWave() _________________________________________________________

    void PluginGfxDevice::drawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor frontFill, HiColor backFill)
    {
        PluginCalls::gfxDevice->drawWave(m_cDevice, (wg_rectSPX*)&dest, (wg_waveLine*)pTopBorder, (wg_waveLine*)pBottomBorder, *(wg_color*)&frontFill, *(wg_color*)&backFill);
    }

    //____ flipDrawWave() _____________________________________________________

    void PluginGfxDevice::flipDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor frontFill, HiColor backFill, GfxFlip flip)
    {
        PluginCalls::gfxDevice->flipDrawWave(m_cDevice, (wg_rectSPX*)&dest, (wg_waveLine*)pTopBorder, (wg_waveLine*)pBottomBorder, *(wg_color*)&frontFill, *(wg_color*)&backFill, (wg_gfxFlip) flip );
    }

    //____ _drawElipse() ______________________________________________________

    void PluginGfxDevice::drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness, HiColor outlineColor )
    {
        PluginCalls::gfxDevice->drawElipse(m_cDevice,(wg_rectSPX*)&canvas, thickness, *(wg_color*)&color, outlineThickness, *(wg_color*)&outlineColor);
    }

    //____ drawPieChart() _____________________________________________________

    void PluginGfxDevice::drawPieChart(const RectSPX& canvas, float start, int nSlices, const float * pSliceSizes, const HiColor * pSliceColors, float hubSize, HiColor hubColor, HiColor backColor, bool bRectangular)
    {
        PluginCalls::gfxDevice->drawPieChart(m_cDevice, (wg_rectSPX*)&canvas, start, nSlices, pSliceSizes, (const wg_color*)pSliceColors, hubSize, *(wg_color*)&hubColor, *(wg_color*)&backColor, bRectangular);
    }

    //____ drawSegments() _____________________________________________________

    void PluginGfxDevice::drawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode )
    {
        PluginCalls::gfxDevice->drawSegments(m_cDevice, (wg_rectSPX*)&dest, nSegments, (const wg_color*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (wg_tintMode)tintMode);
    }

    //____ flipDrawSegments() _________________________________________________

    void PluginGfxDevice::flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode)
    {
        PluginCalls::gfxDevice->flipDrawSegments(m_cDevice, (wg_rectSPX*)&dest, nSegments, (const wg_color*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (wg_gfxFlip) flip, (wg_tintMode)tintMode);
    }

    //____ drawEdgemap() _____________________________________________________

    void PluginGfxDevice::drawEdgemap(CoordSPX dest, Edgemap* pEdgemap)
    {
        PluginCalls::gfxDevice->drawEdgemap(m_cDevice, { dest.x,dest.y }, static_cast<PluginEdgemap*>(pEdgemap)->cObject());
    }

    //____ flipDrawEdgemap() _____________________________________________________

    void PluginGfxDevice::flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip)
    {
        PluginCalls::gfxDevice->flipDrawEdgemap(m_cDevice, { dest.x,dest.y }, static_cast<PluginEdgemap*>(pEdgemap)->cObject(), (wg_gfxFlip) flip);
    }

    //.____ blitNinePatch() ___________________________________________________

    void PluginGfxDevice::blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale)
    {
        PluginCalls::gfxDevice->blitNinePatch(m_cDevice, (wg_rectSPX*)&dstRect, (wg_borderSPX*)&dstFrame, (wg_ninePatch*)&patch, scale);
    }

    //.____ _beginCanvasUpdate() ______________________________________________

    bool PluginGfxDevice::_beginCanvasUpdate(CanvasRef canvasRef, Surface * pCanvasSurface, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
    {     
		if( GfxDeviceGen1::_beginCanvasUpdate(canvasRef, pCanvasSurface, nUpdateRects, pUpdateRects, pLayers, startLayer) )
		{
			auto pCABILayers = wg_dynamic_cast<PluginCanvasLayers_p>(pLayers);
			wg_obj cAPILayers = pCABILayers ? pCABILayers->cObject() : 0;

			if (canvasRef != CanvasRef::None)
			{
                PluginCalls::gfxDevice->beginCanvasUpdateWithRef(m_cDevice, (wg_canvasRef)canvasRef, nUpdateRects, (wg_rectSPX*)pUpdateRects, cAPILayers, startLayer);
			}
			else
			{
                PluginCalls::gfxDevice->beginCanvasUpdateWithSurface(m_cDevice, static_cast<PluginSurface*>(pCanvasSurface)->cObject(), nUpdateRects, (wg_rectSPX*)pUpdateRects, cAPILayers, startLayer);
			}
			return true;
		}
		else
			return false;
    }

    //____ _canvasWasChanged() ________________________________________________

    void PluginGfxDevice::_canvasWasChanged()
    {
        // Do nothing
    }

    //____ _renderLayerWasChanged() ___________________________________________

    void PluginGfxDevice::_renderLayerWasChanged()
    {
        // Do nothing.
    }

    //____ _transformBlitSimple() ___________________________________________________

    void PluginGfxDevice::_transformBlitSimple(const RectI& dest, CoordI src, const int simpleTransform[2][2], OpType type)
    {
        //This method should never be called, but is pure virtual in super class.
        
        assert(false);
    }

    //____ _transformBlitComplex() ___________________________________________________

    void PluginGfxDevice::_transformBlitComplex(const RectI& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type)
    {
        //This method should never be called, but is pure virtual in super class.
        
        assert(false);
    }

	//____ _transformDrawSegments() ___________________________________________

	void PluginGfxDevice::_transformDrawSegments(const RectI& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2])
	{
        //This method should never be called, but is pure virtual in super class.
        
        assert(false);
	}


} // namespace wg

