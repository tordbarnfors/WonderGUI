/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <wg_plugincalls.h>
#include <wg_plugingfxdevice.h>
#include <wg_pluginwarpmap.h>
#include <algorithm>
#include <wg_pluginsurface.h>
#include <wg_pluginedgemap.h>
#include <wg_pluginsurfacefactory.h>
#include <wg_plugincanvaslayers.h>

#include <wg_base.h>
#include <wax_core.h>
#include <assert.h>

using namespace std;

namespace wg
{
	const TypeInfo PluginGfxDevice::TYPEINFO = { "PluginGfxDevice", &GfxDevice::TYPEINFO };



	//____ create() _______________________________________________________________

	PluginGfxDevice_p PluginGfxDevice::create( wax_obj object, PluginSurfaceFactory * pSurfaceFactory, PluginEdgemapFactory * pEdgemapFactory )
	{
		PluginGfxDevice_p p(new PluginGfxDevice( object, pSurfaceFactory, pEdgemapFactory ));
		return p;
	}


	//____ constructor _____________________________________________________________

	PluginGfxDevice::PluginGfxDevice( wax_obj object, PluginSurfaceFactory * pSurfaceFactory, PluginEdgemapFactory * pEdgemapFactory )
	{
        PluginCalls::object->retain(object);
		m_cDevice = object;
        m_pSurfaceFactory = pSurfaceFactory;
        m_pEdgemapFactory = pEdgemapFactory;
		
		// Incomplete initialization of m_canvas.
		
		auto canvasSize = PluginCalls::gfxDevice->canvasSize(object);
		
		m_canvas.size.w = canvasSize.w;
		m_canvas.size.h = canvasSize.h;
	}

	//____ Destructor ______________________________________________________________

	PluginGfxDevice::~PluginGfxDevice()
	{
		if( m_hostBlurbrush )
			PluginCalls::object->release(m_hostBlurbrush);

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
        auto info = PluginCalls::gfxDevice->getCanvasRef(m_cDevice, (wax_canvasRef)ref);

        CanvasInfo info2;
        info2.ref = (CanvasRef) info.ref;
        info2.scale = info.scale;
        info2.size.w = info.size.w;
        info2.size.h = info.size.h;
        info2.pSurface = nullptr;       // Not supported. Can't give access to C++ object on other side of CABI-wall.
        return info2;
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

    //____ warpmapFactory() ______________________________________________________

    WarpmapFactory_p PluginGfxDevice::warpmapFactory()
    {
        if (!m_pWarpmapFactory)
        {
            wax_obj cFactory = PluginCalls::gfxDevice->warpmapFactory(m_cDevice);
            if (cFactory)
                m_pWarpmapFactory = PluginWarpmapFactory::create(cFactory);
        }

        return m_pWarpmapFactory;
    }

    //____ setClipList() _________________________________________________________

    bool PluginGfxDevice::setClipList(int nRectangles, const RectSPX * pRectangles)
    {
        return PluginCalls::gfxDevice->setClipList(m_cDevice, nRectangles, (wax_rectSPX*)pRectangles);
    }

    //____ resetClipList() _______________________________________________________

    void PluginGfxDevice::resetClipList()
    {
        PluginCalls::gfxDevice->resetClipList(m_cDevice);
    }

    //____ pushClipList() ________________________________________________________

    bool PluginGfxDevice::pushClipList(int nRectangles, const RectSPX* pRectangles)
    {
        return PluginCalls::gfxDevice->pushClipList(m_cDevice, nRectangles, (wax_rectSPX*)pRectangles);
    }

    //____ popClipList() ______________________________________________________

    bool PluginGfxDevice::popClipList()
    {
        return PluginCalls::gfxDevice->popClipList(m_cDevice);
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
        PluginCalls::gfxDevice->setTintColor(m_cDevice, *(wax_color*)&color);
    }

    //____ tintColor() ________________________________________________________

    HiColor PluginGfxDevice::tintColor() const
    {
        wax_color col = PluginCalls::gfxDevice->getTintColor(m_cDevice);

        return *(HiColor*)&col;
    }

    //____ clearTintColor() ___________________________________________________

    void PluginGfxDevice::clearTintColor()
    {
        PluginCalls::gfxDevice->clearTintColor(m_cDevice);
    }

    //____ hasTintColor() _____________________________________________________

    bool PluginGfxDevice::hasTintColor() const
    {
        return PluginCalls::gfxDevice->hasTintColor(m_cDevice) != 0;
    }

	//____ setTint() ___________________________________________________________
	//
	// The host gets its own copy of the Tint.

	void PluginGfxDevice::setTint(const RectSPX& rect, Tint* pTint)
	{
		if( !pTint )
		{
			clearTint();
			return;
		}

		wax_obj hostTint = PluginCalls::_hostTint(pTint);

		PluginCalls::gfxDevice->setTint( m_cDevice, (const wax_rectSPX*)&rect, hostTint );
		PluginCalls::object->release(hostTint);
	}

	//____ clearTint() ___________________________________________________________

	void PluginGfxDevice::clearTint()
	{
		PluginCalls::gfxDevice->clearTint(m_cDevice);
	}

	//____ hasTint() _____________________________________________________________

	bool PluginGfxDevice::hasTint() const
	{
		return (PluginCalls::gfxDevice->hasTint(m_cDevice) == 1);
	}

	//____ tint() ________________________________________________________________

	Tint_p PluginGfxDevice::tint() const
	{
		// A plugin side copy of the host's tint, equal but not the same object as
		// the one given to setTint().

		return PluginCalls::_localTint( PluginCalls::gfxDevice->getTint(m_cDevice) );
	}

	//____ tintRect() ____________________________________________________________

	RectSPX PluginGfxDevice::tintRect() const
	{
		auto rect = PluginCalls::gfxDevice->getTintRect(m_cDevice);
		return { rect.x, rect.y, rect.w, rect.h };
	}

    //____ setBlendMode() __________________________________________________________

    bool PluginGfxDevice::setBlendMode( BlendMode blendMode )
    {
        if( blendMode < BlendMode_min || blendMode > BlendMode_max )
            return false;

        return PluginCalls::gfxDevice->setBlendMode(m_cDevice, (wax_blendMode)blendMode);
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
            m_pBlitSource = pSource;

        return retVal;
    }

    //____ PluginGfxDevice::blitSource() ________________________________________

    Surface_p PluginGfxDevice::blitSource() const
    {
        // This method can not be correctly implemented without huge performance loss,
		// so we just assume that blitSource also was set through plugin.

        return m_pBlitSource;
    }

    //____ setMorphFactor() ______________________________________________________

    void PluginGfxDevice::setMorphFactor(float factor)
    {
        limit(factor, 0.f, 1.f);

        PluginCalls::gfxDevice->setMorphFactor(m_cDevice, factor);
    }

    //____ morphFactor() ______________________________________________________

    float PluginGfxDevice::morphFactor() const
    {
        return PluginCalls::gfxDevice->getMorphFactor(m_cDevice);
    }

	//____ setBlurbrush() _____________________________________________________

	void PluginGfxDevice::setBlurbrush( Blurbrush * pBrush )
	{
		// The host device forgets its brush when a canvas update begins, which we
		// don't, so the brush is always passed on. The host's copy of it is reused.

		if( pBrush == m_pBlurbrush && (m_hostBlurbrush || !pBrush) )
		{
			PluginCalls::gfxDevice->setBlurbrush(m_cDevice, m_hostBlurbrush);
			return;
		}

		m_pBlurbrush = pBrush;

		if( m_hostBlurbrush )
		{
			PluginCalls::object->release(m_hostBlurbrush);
			m_hostBlurbrush = nullptr;
		}
			
		if( pBrush )
		{
			wax_blurbrushBP bp;
			bp.size = pBrush->size();
			bp.red = pBrush->red();
			bp.green = pBrush->green();
			bp.blue = pBrush->blue();
			
			m_hostBlurbrush = PluginCalls::blurbrush->create(bp);
		}

		return PluginCalls::gfxDevice->setBlurbrush(m_cDevice, m_hostBlurbrush);
	}

	//____ blurbrush() ________________________________________________________

	Blurbrush_p PluginGfxDevice::blurbrush() const
	{
		return m_pBlurbrush;
	}

	//____ setFixedBlendColor() __________________________________________________

	void PluginGfxDevice::setFixedBlendColor( HiColor color )
	{
		PluginCalls::gfxDevice->setFixedBlendColor(m_cDevice, *(wax_color*)&color);
	}

	//____ fixedBlendColor() _____________________________________________________

	HiColor PluginGfxDevice::fixedBlendColor() const
	{
		wax_color col = PluginCalls::gfxDevice->getFixedBlendColor(m_cDevice);

		return *(HiColor*)&col;
	}

    //____ setRenderLayer() _______________________________________________________

    void PluginGfxDevice::setRenderLayer(int layer)
    {
        PluginCalls::gfxDevice->setRenderLayer(m_cDevice, layer);
    }

    //____ renderLayer() ______________________________________________________

    int PluginGfxDevice::renderLayer() const
    {
        return PluginCalls::gfxDevice->getRenderLayer(m_cDevice);
    }

	//____ beginRender() ___________________________________________________________

	bool PluginGfxDevice::beginRender()
	{
        return PluginCalls::gfxDevice->beginRender(m_cDevice);
	}

	//____ endRender() _____________________________________________________________

	bool PluginGfxDevice::endRender()
	{
        return PluginCalls::gfxDevice->endRender(m_cDevice);
    }

	//____ isRendering() ___________________________________________________________

	bool PluginGfxDevice::isRendering()
	{
		return PluginCalls::gfxDevice->isDeviceRendering(m_cDevice) != 0;
	}

	//____ isIdle() ________________________________________________________________

	bool PluginGfxDevice::isIdle()
	{
		return PluginCalls::gfxDevice->isDeviceIdle(m_cDevice) != 0;
	}

    //____ flush() ___________________________________________________________________

    void PluginGfxDevice::flush()
    {
        PluginCalls::gfxDevice->flushDevice(m_cDevice);
    }

	//____ beginCanvasUpdate() _______________________________________________________

	bool PluginGfxDevice::beginCanvasUpdate(CanvasRef ref, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
	{
		return _beginCanvasUpdate(ref, nullptr, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}

	bool PluginGfxDevice::beginCanvasUpdate(Surface * pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
	{
		return _beginCanvasUpdate(CanvasRef::None, pCanvas, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}

    //____ endCanvasUpdate() _________________________________________________________

    void PluginGfxDevice::endCanvasUpdate()
    {
		// Unwind the stack, retrieve saved values

		if( !m_canvasStack.empty() )
		{
			auto& back = m_canvasStack.back();

			m_canvas = back.canvas;
			m_pCanvasLayers = back.pCanvasLayers;

			m_canvasStack.pop_back();
		}
		else
		{
			m_canvas.ref = CanvasRef::None;
			m_canvas.pSurface = nullptr;
			m_canvas.size.clear();

			m_pCanvasLayers = nullptr;
		}

        PluginCalls::gfxDevice->endCanvasUpdate(m_cDevice);
    }

    //____ fill() __________________________________________________________________

    void PluginGfxDevice::fill( HiColor _col )
    {
        PluginCalls::gfxDevice->fill( m_cDevice, *(wax_color*)&_col );
    }

    void PluginGfxDevice::fill( const RectSPX& _rect, HiColor _col )
    {
        PluginCalls::gfxDevice->fillRect(m_cDevice, (wax_rectSPX*) &_rect, *(wax_color*)&_col);
    }

    //____ drawLine() __________________________________________________________

    static wax_lineCap toCLineCap(LineCap cap)
    {
        return { (wax_lineCapShape) cap.shape, cap.spearLength };
    }

    void PluginGfxDevice::drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness, LineCaps caps)
    {
        if( caps.isNone() )
            PluginCalls::gfxDevice->drawLine(m_cDevice, { begin.x, begin.y }, { end.x, end.y }, *(wax_color*)&color, thickness);
        else
            PluginCalls::gfxDevice->drawLineWithCaps(m_cDevice, { begin.x, begin.y }, { end.x, end.y }, *(wax_color*)&color, thickness, toCLineCap(caps.begin), toCLineCap(caps.end));
    }

    void PluginGfxDevice::drawLine(CoordSPX begin, Direction dir, int length, HiColor color, spx thickness, LineCaps caps)
    {
        if( caps.isNone() )
            PluginCalls::gfxDevice->drawStraightLine(m_cDevice, { begin.x, begin.y }, (wax_direction) dir, length, * (wax_color*)&color, thickness);
        else
            PluginCalls::gfxDevice->drawStraightLineWithCaps(m_cDevice, { begin.x, begin.y }, (wax_direction) dir, length, * (wax_color*)&color, thickness, toCLineCap(caps.begin), toCLineCap(caps.end));
    }

    //____ blit() __________________________________________________________________

    void PluginGfxDevice::blit(CoordSPX dest)
    {
        PluginCalls::gfxDevice->blit(m_cDevice, { dest.x, dest.y });
    }

    void PluginGfxDevice::blit(CoordSPX dest, const RectSPX& src)
    {
        PluginCalls::gfxDevice->blitRect(m_cDevice, { dest.x, dest.y }, (wax_rectSPX*) &src );
    }

    //____ flipBlit() _________________________________________________________

    void PluginGfxDevice::flipBlit(CoordSPX dest, GfxFlip flip )
    {
        PluginCalls::gfxDevice->flipBlit(m_cDevice, { dest.x, dest.y }, (wax_gfxFlip) flip);
    }

    void PluginGfxDevice::flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip )
    {
        PluginCalls::gfxDevice->flipBlitRect(m_cDevice, { dest.x, dest.y }, (wax_rectSPX*)&src, (wax_gfxFlip)flip);
    }

    //____ stretchBlit() ______________________________________________________

    void PluginGfxDevice::stretchBlit(const RectSPX& dest)
    {
        PluginCalls::gfxDevice->stretchBlit(m_cDevice, (wax_rectSPX*) &dest);
    }

    void PluginGfxDevice::stretchBlit(const RectSPX& dest, const RectSPX& source)
    {
        PluginCalls::gfxDevice->stretchBlitRect(m_cDevice, (wax_rectSPX*)&dest, (wax_rectSPX*)&source);
    }

    //____ stretchFlipBlit() __________________________________________________

    void PluginGfxDevice::stretchFlipBlit(const RectSPX& dest, GfxFlip flip)
    {
        PluginCalls::gfxDevice->stretchFlipBlit(m_cDevice, (wax_rectSPX*)&dest, (wax_gfxFlip) flip);
    }

    void PluginGfxDevice::stretchFlipBlit(const RectSPX& dest, const RectSPX& source, GfxFlip flip)
    {
        PluginCalls::gfxDevice->stretchFlipBlitRect(m_cDevice, (wax_rectSPX*)&dest, (wax_rectSPX*)&source, (wax_gfxFlip)flip );
    }

	//____ precisionBlit() _______________________________________________________

	void PluginGfxDevice::precisionBlit(const RectSPX& dest, const RectF& srcSPX)
	{
        PluginCalls::gfxDevice->precisionBlit(m_cDevice, (wax_rectSPX*)&dest, (wax_rectF*)&srcSPX);
	}

	//____ transformBlit() _______________________________________________________

	void PluginGfxDevice::transformBlit(const RectSPX& dest, CoordF srcSPX, const Transform& transform)
	{
        PluginCalls::gfxDevice->transformBlit(m_cDevice, (wax_rectSPX*)&dest, {srcSPX.x,srcSPX.y}, reinterpret_cast<const wax_transform*>(&transform) );
	}

    //____ rotScaleBlit() _____________________________________________________
    void PluginGfxDevice::rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
    {
        PluginCalls::gfxDevice->rotScaleBlit(m_cDevice, (wax_rectSPX*)&dest, rotationDegrees, scale, { srcCenter.x, srcCenter.y }, { destCenter.x, destCenter.y });
    }

    //____ tile() _____________________________________________________________

    void PluginGfxDevice::tile(const RectSPX& dest, CoordSPX shift)
    {
        PluginCalls::gfxDevice->tile(m_cDevice, (wax_rectSPX*)&dest, { shift.x,shift.y });
    }

    //____ flipTile() _________________________________________________________

    void PluginGfxDevice::flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift)
    {
        PluginCalls::gfxDevice->flipTile(m_cDevice, (wax_rectSPX*)&dest, (wax_gfxFlip) flip, { shift.x, shift.y });
    }

    //____ scaleTile() ________________________________________________________

    void PluginGfxDevice::scaleTile(const RectSPX& dest, float scale, CoordSPX shift)
    {
        PluginCalls::gfxDevice->scaleTile(m_cDevice, (wax_rectSPX*)&dest, scale, { shift.x,shift.y });
    }

    //____ scaleFlipTile() ____________________________________________________

    void PluginGfxDevice::scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift)
    {
        PluginCalls::gfxDevice->scaleFlipTile(m_cDevice, (wax_rectSPX*)&dest, scale, (wax_gfxFlip) flip, { shift.x, shift.y });
    }

	//____ blur() ________________________________________________________________

	void PluginGfxDevice::blur(CoordSPX dest)
	{
		PluginCalls::gfxDevice->blur(m_cDevice, { dest.x, dest.y });
	}

	void PluginGfxDevice::blur(CoordSPX dest, const RectSPX& src)
	{
		PluginCalls::gfxDevice->blurRect(m_cDevice, { dest.x, dest.y }, (wax_rectSPX*) &src );
	}

	//____ stretchBlur() _________________________________________________________

	void PluginGfxDevice::stretchBlur(const RectSPX& dest)
	{
		PluginCalls::gfxDevice->stretchBlur(m_cDevice, (wax_rectSPX*) &dest);
	}

	void PluginGfxDevice::stretchBlur(const RectSPX& dest, const RectSPX& source)
	{
		PluginCalls::gfxDevice->stretchBlurRect(m_cDevice, (wax_rectSPX*)&dest, (wax_rectSPX*)&source);
	}

	//____ transformBlur() _______________________________________________________

	void PluginGfxDevice::transformBlur(const RectSPX& dest, CoordF srcSPX, const Transform& transform)
	{
		PluginCalls::gfxDevice->transformBlur(m_cDevice, (wax_rectSPX*)&dest, {srcSPX.x,srcSPX.y}, reinterpret_cast<const wax_transform*>(&transform) );
	}

	//____ rotScaleBlur() ________________________________________________________

	void PluginGfxDevice::rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
	{
		PluginCalls::gfxDevice->rotScaleBlur(m_cDevice, (wax_rectSPX*)&dest, rotationDegrees, scale, { srcCenter.x, srcCenter.y }, { destCenter.x, destCenter.y });
	}

    //____ drawWave() _________________________________________________________

    void PluginGfxDevice::drawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill)
    {
        PluginCalls::gfxDevice->drawWave(m_cDevice, (wax_rectSPX*)&dest, (wax_waveLine*)pTopBorder, (wax_waveLine*)pBottomBorder, *(wax_color*)&fill);
    }

    //____ flipDrawWave() _____________________________________________________

    void PluginGfxDevice::flipDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill, GfxFlip flip)
    {
        PluginCalls::gfxDevice->flipDrawWave(m_cDevice, (wax_rectSPX*)&dest, (wax_waveLine*)pTopBorder, (wax_waveLine*)pBottomBorder, *(wax_color*)&fill, (wax_gfxFlip) flip );
    }

    //____ _drawElipse() ______________________________________________________

    void PluginGfxDevice::drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness, HiColor outlineColor )
    {
        PluginCalls::gfxDevice->drawElipse(m_cDevice,(wax_rectSPX*)&canvas, thickness, *(wax_color*)&color, outlineThickness, *(wax_color*)&outlineColor);
    }

    //____ drawPieChart() _____________________________________________________

    void PluginGfxDevice::drawPieChart(const RectSPX& canvas, float start, int nSlices, const float * pSliceSizes, const HiColor * pSliceColors, float hubSize, HiColor hubColor, HiColor backColor, bool bRectangular)
    {
        PluginCalls::gfxDevice->drawPieChart(m_cDevice, (wax_rectSPX*)&canvas, start, nSlices, pSliceSizes, (const wax_color*)pSliceColors, hubSize, *(wax_color*)&hubColor, *(wax_color*)&backColor, bRectangular);
    }

    //____ drawSegments() _____________________________________________________

    void PluginGfxDevice::drawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode )
    {
        PluginCalls::gfxDevice->drawSegments(m_cDevice, (wax_rectSPX*)&dest, nSegments, (const wax_color*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (wax_tintMode)tintMode);
    }

    //____ flipDrawSegments() _________________________________________________

    void PluginGfxDevice::flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode)
    {
        PluginCalls::gfxDevice->flipDrawSegments(m_cDevice, (wax_rectSPX*)&dest, nSegments, (const wax_color*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (wax_gfxFlip) flip, (wax_tintMode)tintMode);
    }

    //____ drawEdgemap() _____________________________________________________

    void PluginGfxDevice::drawEdgemap(CoordSPX dest, Edgemap* pEdgemap)
    {
        PluginCalls::gfxDevice->drawEdgemap(m_cDevice, { dest.x,dest.y }, static_cast<PluginEdgemap*>(pEdgemap)->cObject());
    }

    //____ flipDrawEdgemap() _____________________________________________________

    void PluginGfxDevice::flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip)
    {
        PluginCalls::gfxDevice->flipDrawEdgemap(m_cDevice, { dest.x,dest.y }, static_cast<PluginEdgemap*>(pEdgemap)->cObject(), (wax_gfxFlip) flip);
    }

    //____ tileEdgemap() _____________________________________________________

    void PluginGfxDevice::tileEdgemap(const RectSPX& dest, Edgemap* pEdgemap, CoordSPX shift)
    {
        PluginCalls::gfxDevice->tileEdgemap(m_cDevice, (const wax_rectSPX*)&dest, static_cast<PluginEdgemap*>(pEdgemap)->cObject(), { shift.x,shift.y });
    }

    //____ flipTileEdgemap() _________________________________________________

    void PluginGfxDevice::flipTileEdgemap(const RectSPX& dest, Edgemap* pEdgemap, GfxFlip flip, CoordSPX shift)
    {
        PluginCalls::gfxDevice->flipTileEdgemap(m_cDevice, (const wax_rectSPX*)&dest, static_cast<PluginEdgemap*>(pEdgemap)->cObject(), (wax_gfxFlip) flip, { shift.x,shift.y });
    }

    //____ warpBlit() ________________________________________________________

    // Warpmaps need to be the host's, created through our warpmapFactory().

    static wax_obj hostWarpmap(Warpmap* pWarpmap, const Object* pCaller, const char* pFunc)
    {
        if (!pWarpmap || !pWarpmap->isInstanceOf(PluginWarpmap::TYPEINFO))
        {
            wax::Core::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Warpmap not created by the WarpmapFactory of the device.",
                                pCaller, &PluginGfxDevice::TYPEINFO, pFunc, __FILE__, __LINE__);
            return nullptr;
        }

        return static_cast<PluginWarpmap*>(pWarpmap)->cObject();
    }

    void PluginGfxDevice::warpBlit(CoordSPX dest, Warpmap* pWarpmap, CoordSPX sourceOffset)
    {
        if (wax_obj cWarpmap = hostWarpmap(pWarpmap, this, __func__))
            PluginCalls::gfxDevice->warpBlit(m_cDevice, { dest.x, dest.y }, cWarpmap, { sourceOffset.x, sourceOffset.y });
    }

    void PluginGfxDevice::warpBlit(CoordSPX dest, Warpmap* pWarpmap, const RectSPX& source, CoordSPX sourceOffset)
    {
        if (wax_obj cWarpmap = hostWarpmap(pWarpmap, this, __func__))
            PluginCalls::gfxDevice->warpBlitRect(m_cDevice, { dest.x, dest.y }, cWarpmap, (const wax_rectSPX*)&source, { sourceOffset.x, sourceOffset.y });
    }

    //____ warpDrawEdgemap() _________________________________________________

    void PluginGfxDevice::warpDrawEdgemap(CoordSPX dest, Warpmap* pWarpmap, Edgemap* pEdgemap, CoordSPX sourceOffset)
    {
        if (wax_obj cWarpmap = hostWarpmap(pWarpmap, this, __func__))
            PluginCalls::gfxDevice->warpDrawEdgemap(m_cDevice, { dest.x, dest.y }, cWarpmap, static_cast<PluginEdgemap*>(pEdgemap)->cObject(), { sourceOffset.x, sourceOffset.y });
    }

    //____ drawRoundedBox() ___________________________________________________
    //
    // The host gets its own copies of the Tints.

    void PluginGfxDevice::drawRoundedBox(const RoundedBoxSpec& spec)
    {
        wax_roundedBoxSpec box;

        box.rect = *(const wax_rectSPX*)&spec.rect;

        for (int i = 0; i < 4; i++)
            box.radius[i] = spec.radius[i];

        box.outline = *(const wax_borderSPX*)&spec.outline;
        box.fillColor = *(const wax_color*)&spec.fillColor;
        box.outlineColor = *(const wax_color*)&spec.outlineColor;
        box.fillTint = PluginCalls::_hostTint(spec.pFillTint);
        box.outlineTint = PluginCalls::_hostTint(spec.pOutlineTint);

        PluginCalls::gfxDevice->drawRoundedBox(m_cDevice, &box);

        if (box.fillTint)
            PluginCalls::object->release(box.fillTint);
        if (box.outlineTint)
            PluginCalls::object->release(box.outlineTint);
    }

    //____ drawRing() _________________________________________________________
    //
    // The host gets its own copies of the Tints.

    void PluginGfxDevice::drawRing(const RingSpec& spec)
    {
        // Invalid slices are passed on for the host to report.

        int nSlices = std::min(spec.nSlices, maxRingSlices());
        int nCopied = spec.pSlices ? std::max(nSlices, 0) : 0;

        if ((int)m_ringSlices.size() < nCopied)
            m_ringSlices.resize(nCopied);

        wax_ringSlice* slices = m_ringSlices.data();

        for (int i = 0; i < nCopied; i++)
        {
            slices[i].size = spec.pSlices[i].size;
            slices[i].color = *(const wax_color*)&spec.pSlices[i].color;
            slices[i].tint = PluginCalls::_hostTint(spec.pSlices[i].pTint);
        }

        wax_ringSpec ring;

        ring.rect = *(const wax_rectSPX*)&spec.rect;
        ring.thickness = spec.thickness;
        ring.outline = spec.outline;
        ring.start = spec.start;
        ring.nSlices = nSlices;
        ring.pSlices = spec.pSlices ? slices : nullptr;
        ring.bSquare = spec.bSquare;
        ring.outlineColor = *(const wax_color*)&spec.outlineColor;
        ring.outlineTint = PluginCalls::_hostTint(spec.pOutlineTint);
        ring.hubColor = *(const wax_color*)&spec.hubColor;
        ring.hubTint = PluginCalls::_hostTint(spec.pHubTint);
        ring.backColor = *(const wax_color*)&spec.backColor;
        ring.backTint = PluginCalls::_hostTint(spec.pBackTint);

        PluginCalls::gfxDevice->drawRing(m_cDevice, &ring);

        auto release = [](wax_obj obj) { if (obj) PluginCalls::object->release(obj); };

        for (int i = 0; i < nCopied; i++)
            release(slices[i].tint);

        release(ring.outlineTint);
        release(ring.hubTint);
        release(ring.backTint);
    }

    //.____ blitNinePatch() ___________________________________________________

    void PluginGfxDevice::blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale)
    {
        PluginCalls::gfxDevice->blitNinePatch(m_cDevice, (wax_rectSPX*)&dstRect, (wax_borderSPX*)&dstFrame, (wax_ninePatch*)&patch, scale);
    }

    //.____ _beginCanvasUpdate() ______________________________________________

    bool PluginGfxDevice::_beginCanvasUpdate(CanvasRef canvasRef, Surface * pCanvasSurface, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
    {
		SizeSPX sz;
		PixelFormat format;

		if( canvasRef != CanvasRef::None )
		{
			auto info = canvas(canvasRef);
			sz = info.size;
			format = info.format;
		}
		else if( pCanvasSurface )
		{
			sz = pCanvasSurface->pixelSize()*64;
			format = pCanvasSurface->pixelFormat();
		}
		else
		{
			//TODO: Error handling!
			return false;
		}

		if( sz.isEmpty() )
		{
			//TODO: Error handling!
			return false;
		}

		if (nUpdateRects > 0)
		{
			RectSPX bounds = *pUpdateRects;
			for (int i = 1; i < nUpdateRects; i++)
				bounds.growToContain(pUpdateRects[i]);

			if (bounds.x < 0 || bounds.y < 0 || bounds.w > sz.w || bounds.h > sz.h)
			{
				wax::Core::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "UpdateRects contains rectangels outside canvas.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;
			}
		}

		// Push old values onto stack and set new ones.

		m_canvasStack.push_back({ m_canvas, m_pCanvasLayers });

		m_canvas.ref = canvasRef;
		m_canvas.pSurface = pCanvasSurface;
		m_canvas.size = sz;
		m_canvas.format = format;

		m_pCanvasLayers = pLayers;

		// Begin canvas update on host side.

		auto pCABILayers = wax_dynamic_cast<PluginCanvasLayers_p>(pLayers);
		wax_obj cAPILayers = pCABILayers ? pCABILayers->cObject() : 0;

		if (canvasRef != CanvasRef::None)
		{
			PluginCalls::gfxDevice->beginCanvasUpdateWithRef(m_cDevice, (wax_canvasRef)canvasRef, nUpdateRects, (wax_rectSPX*)pUpdateRects, cAPILayers, startLayer);
		}
		else
		{
			PluginCalls::gfxDevice->beginCanvasUpdateWithSurface(m_cDevice, static_cast<PluginSurface*>(pCanvasSurface)->cObject(), nUpdateRects, (wax_rectSPX*)pUpdateRects, cAPILayers, startLayer);
		}
		return true;
    }

} // namespace wg

