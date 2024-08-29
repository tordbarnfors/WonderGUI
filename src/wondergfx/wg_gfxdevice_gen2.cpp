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

#include <algorithm>
#include <cmath>
#include <cstring>

#include <wg_gfxdevice_gen2.h>
#include <wg_geo.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <wg_gradyent.h>

using namespace std;
using namespace wg::Util;

namespace wg
{

	const TypeInfo GfxDeviceGen2::TYPEINFO = { "GfxDeviceGen2", &GfxDevice::TYPEINFO };

	const TypeInfo s_surfaceType = { "Unkown SurfaceType, backend missing", &Surface::TYPEINFO };



	const int GfxDeviceGen2::s_blitFlipOffsets[GfxFlip_size][2] = { { 0,0 },				// Normal
															{ 1,0 },				// FlipX
															{ 0,1 },				// FlipY
															{ 0,1 },				// Rot90
															{ 0,0 },				// Rot90FlipX
															{ 1,1 },				// Rot90FlipY
															{ 1,1 },				// Rot180
															{ 0,1 },				// Rot180FlipX
															{ 1,0 },				// Rot180FlipY
															{ 1,0 },				// Rot270
															{ 1,1 },				// Rot270FlipX
															{ 0,0 } };				// Rot270FlipY

	const int GfxDeviceGen2::s_blitFlipTransforms[GfxFlip_size][2][2] = {
	{ 1,0,0,1 },			// Normal
	{ -1,0,0,1 },			// FlipX
	{ 1,0,0,-1 },			// FlipY
	{ 0,-1,1,0 },			// Rot90
	{ 0,1,1,0 },			// Rot90FlipX
	{ 0,-1,-1,0 },			// Rot90FlipY
	{ -1,0,0,-1 },			// Rot180
	{ 1,0,0,-1 },			// Rot180FlipX
	{ -1,0,0,1 },			// Rot180FlipY
	{ 0,1,-1,0 },			// Rot270
	{ 0,-1,-1,0 },			// Rot270FlipX
	{ 0,1,1,0 }				// Rot270FlipY
	};


//____ create _____________________________________________________________

GfxDeviceGen2_p GfxDeviceGen2::create( GfxBackend * pBackend )
{
	return GfxDeviceGen2_p( new GfxDeviceGen2(pBackend) );
}

//____ constructor _____________________________________________________________

GfxDeviceGen2::GfxDeviceGen2( GfxBackend * pBackend ) : m_pBackend(pBackend)
{
}

//____ Destructor _________________________________________________________

GfxDeviceGen2::~GfxDeviceGen2()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& GfxDeviceGen2::typeInfo(void) const
{
	return TYPEINFO;
}

//____ surfaceType() __________________________________________________________

const TypeInfo& GfxDeviceGen2::surfaceType(void) const
{
	return m_pBackend ? m_pBackend->surfaceType() : s_surfaceType;
}


//____ setBackend() ___________________________________________________________

bool GfxDeviceGen2::setBackend( GfxBackend * pBackend )
{
	if( m_bRendering )
	{
		//TODO: Error handling!!!
		
		return  false;
	}
	
	m_pBackend = pBackend;
	return true;
}

//____ canvas() ______________________________________________________________

const CanvasInfo& GfxDeviceGen2::canvas() const
{
	if (m_canvasStack.empty())
		return m_dummyCanvasInfo;
	else
		return m_canvasStack.back().info;
}

const CanvasInfo GfxDeviceGen2::canvas(CanvasRef ref) const
{
	const CanvasInfo * pInfo = m_pBackend ? m_pBackend->canvasInfo(ref) : nullptr;
	
	if( pInfo )
		return * pInfo;
	else
		return m_dummyCanvasInfo;
}

//____ canvasLayers() _____________________________________________________

CanvasLayers_p GfxDeviceGen2::canvasLayers() const
{
	if (m_canvasStack.empty())
		return nullptr;

	return m_canvasStack.back().pLayerInfo;
}

//____ surfaceFactory() _______________________________________________________

SurfaceFactory_p GfxDeviceGen2::surfaceFactory()
{
	return m_pBackend ? m_pBackend->surfaceFactory() : nullptr;
}

//____ edgemapFactory() _______________________________________________________

EdgemapFactory_p GfxDeviceGen2::edgemapFactory()
{
	return m_pBackend ? m_pBackend->edgemapFactory() : nullptr;
}


//____ canvasSize() _______________________________________________________

SizeSPX GfxDeviceGen2::canvasSize() const
{
	if (m_canvasStack.empty())
		return { 0,0 };

	return m_canvasStack.back().info.size;
}

//____ setClipList() ______________________________________________________

bool GfxDeviceGen2::setClipList(int nRectangles, const RectSPX* pRectangles)
{
	if (!m_pActiveClipList)
		return false;

	RectSPX bounds;
	if (nRectangles == 0)
		bounds = { m_pActiveClipList->bounds.x, m_pActiveClipList->bounds.y, 0, 0 };
	else
	{
		bounds = *pRectangles;
		for (int i = 1; i < nRectangles; i++)
			bounds.growToContain(pRectangles[i]);


		if (!m_pActiveCanvas->updateRects.bounds.contains(bounds) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "ClipList contains rectangels outside canvas update rectangles. ClipList not set.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}
	}

	m_pActiveClipList->nRects = nRectangles;
	m_pActiveClipList->pRects = pRectangles;
	m_pActiveClipList->bounds = bounds;
	return true;
}

//____ resetClipList() ____________________________________________________

void GfxDeviceGen2::resetClipList()
{
	if (m_pActiveClipList)
		* m_pActiveClipList = m_pActiveCanvas->updateRects;
}

//____ pushClipList() _____________________________________________________

bool GfxDeviceGen2::pushClipList(int nRectangles, const RectSPX* pRectangles)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling.

		return false;
	}

	RectSPX bounds;
	if (nRectangles == 0)
		bounds = { m_pActiveClipList->bounds.x, m_pActiveClipList->bounds.y, 0, 0 };
	else
	{
		bounds = *pRectangles;
		for (int i = 1; i < nRectangles; i++)
			bounds.growToContain(pRectangles[i]);


		if (!m_pActiveCanvas->updateRects.bounds.contains(bounds))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "ClipList contains rectangels outside canvas update rectangles. ClipList not set.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}
	}

	m_pActiveCanvas->clipListStack.emplace_back( ClipList{ nRectangles, pRectangles, bounds });

	m_pActiveClipList = &m_pActiveCanvas->clipListStack.back();
	return true;
}

//____ popClipList() ______________________________________________________

bool GfxDeviceGen2::popClipList()
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return false;
	}

	if (m_pActiveCanvas->clipListStack.size() == 1)
	{
		//TODO: Error handling!

		return false;
	}

	m_pActiveCanvas->clipListStack.pop_back();
	m_pActiveClipList = &m_pActiveCanvas->clipListStack.back();
	return true;
}

//____ clipList() _________________________________________________________

const RectSPX*	GfxDeviceGen2::clipList() const
{
	if (!m_pActiveClipList)
		return nullptr;

	return m_pActiveClipList->pRects;
}

//____ clipListSize() _____________________________________________________

int GfxDeviceGen2::clipListSize() const
{
	if (!m_pActiveClipList)
		return 0;

	return m_pActiveClipList->nRects;
}

//____ clipBounds() _______________________________________________________

const RectSPX& GfxDeviceGen2::clipBounds() const
{
	if (!m_pActiveClipList)
		return m_dummyRect;

	return m_pActiveClipList->bounds;
}

//____ setTint() _______________________________________________________________

void GfxDeviceGen2::setTint(HiColor color)
{
	if (color != m_renderState.tintColor || m_renderState.pTintmap)
	{
		m_renderState.tintColor = color;
		m_renderState.pTintmap = nullptr;
		m_renderState.tintmapRect.clear();

		m_stateChanges |= int(StateChange::TintColor);
	}
}

void GfxDeviceGen2::setTint(const RectSPX& rect, Tintmap* pTintmap)
{
	if (pTintmap != m_renderState.pTintmap || rect != m_renderState.tintmapRect )
	{
		m_renderState.tintColor = HiColor::Undefined;
		m_renderState.pTintmap = pTintmap;
		m_renderState.tintmapRect = rect;

		m_stateChanges |= int(StateChange::TintColor);
	}
}

//____ clearTint() _____________________________________________________________

void GfxDeviceGen2::clearTint()
{
	if (m_renderState.pTintmap || m_renderState.tintColor.isUndefined() )
		m_stateChanges |= int(StateChange::TintColor);

	m_renderState.pTintmap = nullptr;
	m_renderState.tintmapRect.clear();
	m_renderState.tintColor = HiColor::Undefined;
}

//____ isTinting() _____________________________________________________________

bool GfxDeviceGen2::isTinting() const
{
	return (m_renderState.pTintmap || !m_renderState.tintColor.isUndefined());
}

//____ setTintColor() ________________________________________________________

void GfxDeviceGen2::setTintColor(HiColor color)
{
	setTint(color);
}

//____ tintmap() _______________________________________________________________

Tintmap_p GfxDeviceGen2::tintmap() const
{
	return m_renderState.pTintmap;
}

//____ tintmapRect() _________________________________________________________

RectSPX GfxDeviceGen2::tintmapRect() const
{
	return m_renderState.tintmapRect;
}

//____ tintColor() ________________________________________________________

HiColor GfxDeviceGen2::tintColor() const
{
	return m_renderState.tintColor;
}

//____ setTintGradient() __________________________________________________

void GfxDeviceGen2::setTintGradient(const RectSPX& rect, const Gradient& gradient)
{
	HiColor bottom( (gradient.bottomLeft.r + gradient.bottomRight.r) / 2,
						(gradient.bottomLeft.g + gradient.bottomRight.g) / 2,
						(gradient.bottomLeft.b + gradient.bottomRight.b) / 2,
						(gradient.bottomLeft.a + gradient.bottomRight.a) / 2 );

	HiColor top( (gradient.topLeft.r + gradient.topRight.r) / 2,
		(gradient.topLeft.g + gradient.topRight.g) / 2,
		(gradient.topLeft.b + gradient.topRight.b) / 2,
		(gradient.topLeft.a + gradient.topRight.a) / 2);

	HiColor left( (gradient.topLeft.r + gradient.bottomLeft.r) / 2,
		(gradient.topLeft.g + gradient.bottomLeft.g) / 2,
		(gradient.topLeft.b + gradient.bottomLeft.b) / 2,
		(gradient.topLeft.a + gradient.bottomLeft.a) / 2);

	HiColor right( (gradient.topRight.r + gradient.bottomRight.r) / 2,
		(gradient.topRight.g + gradient.bottomRight.g) / 2,
		(gradient.topRight.b + gradient.bottomRight.b) / 2,
		(gradient.topRight.a + gradient.bottomRight.a) / 2);

	float yDiff = (bottom.r - top.r) * (bottom.r - top.r) +
					(bottom.g - top.g) * (bottom.g - top.g) +
					(bottom.b - top.b) * (bottom.b - top.b) +
					(bottom.a - top.a) * (bottom.a - top.a);

	float xDiff = (left.r - right.r) * (left.r - right.r) +
		(left.g - right.g) * (left.g - right.g) +
		(left.b - right.b) * (left.b - right.b) +
		(left.a - right.a) * (left.a - right.a);

	Gradyent_p pGradyent;

	if (xDiff > yDiff)
		pGradyent = Gradyent::create(HiColor::White, HiColor::White, left, right);
	else
		pGradyent = Gradyent::create(top, bottom, HiColor::White, HiColor::White);

	setTint(rect, pGradyent);
}

//____ clearTintGradient() ________________________________________________

void GfxDeviceGen2::clearTintGradient()
{
	clearTint();
}

//____ setBlendMode() _____________________________________________________

bool GfxDeviceGen2::setBlendMode(BlendMode blendMode)
{
	if (blendMode != m_renderState.blendMode)
	{
		//TODO: Check that blendMode is supported.

		m_renderState.blendMode = blendMode;

		m_stateChanges |= int(StateChange::BlendMode);
	}

	return true;
}

//____ blendMode() ________________________________________________________

BlendMode GfxDeviceGen2::blendMode() const
{
	return m_renderState.blendMode;
}

//____ setBlitSource() ____________________________________________________

bool GfxDeviceGen2::setBlitSource(Surface* pSource)
{
	if (pSource && pSource->typeInfo() != surfaceType())
	{
		//TODO: Error handling!

		return false;
	}

	if (pSource != m_renderState.blitSource)
	{
		m_renderState.blitSource = pSource;
		m_stateChanges |= int(StateChange::BlitSource);
	}

	return true;
}

//____ blitSource() _______________________________________________________

Surface_p GfxDeviceGen2::blitSource() const
{
	return m_renderState.blitSource;
}

//____ setMorphFactor() ___________________________________________________

void GfxDeviceGen2::setMorphFactor(float factor)
{
	if (factor != m_renderState.morphFactor)
	{
		m_renderState.morphFactor = factor;
		m_stateChanges |= int(StateChange::MorphFactor);
	}
}

//____ morphFactor() ______________________________________________________

float GfxDeviceGen2::morphFactor() const
{
	return m_renderState.morphFactor;
}

//____ setBlurbrush() __________________________________________________________

void GfxDeviceGen2::setBlurbrush(Blurbrush* pBrush)
{
	if (pBrush != m_renderState.pBlurbrush)
	{
		m_renderState.pBlurbrush = pBrush;
		m_stateChanges |= int(StateChange::Blur);
	}
}

//____ blurbrush() _____________________________________________________________

Blurbrush_p	GfxDeviceGen2::blurbrush() const
{
	return m_renderState.pBlurbrush;
}

//____ setFixedBlendColor() _______________________________________________

void GfxDeviceGen2::setFixedBlendColor(HiColor color)
{
	if (m_renderState.fixedBlendColor != color)
	{
		m_renderState.fixedBlendColor = color;
		m_stateChanges |= int(StateChange::FixedBlendColor);
	}
}

//____ fixedBlendColor() __________________________________________________

HiColor GfxDeviceGen2::fixedBlendColor() const
{
	return m_renderState.fixedBlendColor;
}

//____ setRenderLayer() ___________________________________________________

void GfxDeviceGen2::setRenderLayer(int layer)
{

	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (layer == m_pActiveCanvas->activeLayer)
		return;

	// Switch layer

	m_pActiveCanvas->activeLayer = layer;
	m_pActiveLayer = &m_pActiveCanvas->layers[layer];

	// Set all state-change flags to force a full check on next command.

	m_stateChanges = 0xFF;

}

//____ renderLayer() ______________________________________________________

int GfxDeviceGen2::renderLayer() const
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return 0;
	}

	return m_pActiveCanvas->activeLayer;
}

//____ beginRender() ______________________________________________________

bool GfxDeviceGen2::beginRender()
{
	if (m_bRendering)
	{
		//TODO: Error handling!

		return false;
	}

	if( !m_pBackend )
	{
		//TODO: Error handling!

		return false;
	}
	
	m_bRendering = true;
	return true;
}

//____ endRender() ________________________________________________________

bool GfxDeviceGen2::endRender()
{
	if (!m_bRendering)
	{
		//TODO: Error handling!

		return false;
	}

	m_bRendering = false;

	if (!m_canvasStack.empty())
	{
		//TODO: Error handling!
	}

	return true;
}

//____ isRendering() ______________________________________________________

bool GfxDeviceGen2::isRendering()
{
	return m_bRendering;
}

//____ isIdle() ___________________________________________________________

bool GfxDeviceGen2::isIdle()
{
	return !m_bRendering;
}

//____ flush() ____________________________________________________________

void GfxDeviceGen2::flush()
{
	return;
}

//____ _beginCanvasUpdate() _______________________________________________

bool GfxDeviceGen2::_beginCanvasUpdate(CanvasRef ref, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer)
{
	// Error checking

	if (ref == CanvasRef::None && pCanvas == nullptr)
	{
		//TODO: Error handling!

		return false;
	}

	if (pCanvas && !pCanvas->canBeCanvas())
	{
		//TODO: Error handling!

		return false;
	}

	// Save renderState for current canvas
	// and reset it.

	if (m_pActiveCanvas)
	{
		m_pActiveCanvas->savedState = m_renderState;
		_resetState(m_renderState);
	}

	// Add our canvas entry

	m_canvasStack.emplace_back();
	auto& entry = m_canvasStack.back();

	// Fill in info.

	if (ref != CanvasRef::None)
	{
		entry.info = canvas(ref);
	}
	else
	{
		entry.info.ref = CanvasRef::None;
		entry.info.pSurface = pCanvas;
		entry.info.format = pCanvas->pixelFormat();
		entry.info.scale = pCanvas->scale();
		entry.info.size = pCanvas->pixelSize() * 64;
	}

	// Fill in update rects.

	if (nUpdateRects > 0)
	{
		RectSPX bounds = pUpdateRects[0];
		for (int i = 1; i < nUpdateRects; i++)
			bounds.growToContain(pUpdateRects[i]);

		entry.updateRects.nRects = nUpdateRects;
		entry.updateRects.pRects = pUpdateRects;
		entry.updateRects.bounds = bounds;
	}
	else
	{
		entry.updateRects.nRects = 1;
		entry.updateRects.pRects = &entry.updateRects.bounds;
		entry.updateRects.bounds = entry.info.size;
	}

	// Fill in clip-rects.

	entry.clipListStack.emplace_back(entry.updateRects);
	
	// Setup layers

	entry.pLayerInfo = pLayers;

	int nLayers = pLayers ? pLayers->size() + 1 : 1;
	entry.layers.resize(nLayers);

	int activeLayer = 0;
	if (startLayer != -1)
		activeLayer = startLayer;
	else if (pLayers)
		activeLayer = pLayers->baseLayer();

	entry.activeLayer = activeLayer;

	// Setup pointers

	m_pActiveClipList = &entry.clipListStack.back();
	m_pActiveLayer = &entry.layers[activeLayer];
	m_pActiveCanvas = &entry;

	// Reset render states

	_resetState(m_renderState);


	// Run clear functions (if present)
	// They will end up as commands first in the command buffers.

	if (pLayers)
	{
		if (pLayers->m_clearCanvasFunc)
		{
			entry.activeLayer = 0;
			pLayers->m_clearCanvasFunc(this);
			entry.activeLayer = activeLayer;
		}

		if (activeLayer != 0 && pLayers->m_layers[activeLayer].clearFunc)
			pLayers->m_layers[activeLayer].clearFunc(this);
	}

	//

	return true;
}

//____ endCanvasUpdate() __________________________________________________

void GfxDeviceGen2::endCanvasUpdate()
{
	if (m_canvasStack.empty())
	{
		//TODO: Errorhandling!!!
		
		return;
	}
	
	flattenLayers();
	m_canvasStack.pop_back();
	
	if (!m_canvasStack.empty())
	{
		m_renderState = m_canvasStack.back().savedState;
		m_pActiveCanvas = &m_canvasStack.back();
		m_pActiveLayer = &m_pActiveCanvas->layers[m_pActiveCanvas->activeLayer];
		m_pActiveClipList = &m_pActiveCanvas->clipListStack.back();
	}
	else
	{
		m_pActiveCanvas = nullptr;
		m_pActiveLayer = nullptr;
		m_pActiveClipList = nullptr;
	}
}

//____ flattenLayers() ____________________________________________________

void GfxDeviceGen2::flattenLayers()
{
	//TODO: All kinds of layer magic...
	
	
	auto& canvasData = m_canvasStack.back();

	if( canvasData.info.ref != CanvasRef::None )
		m_pBackend->setCanvas(canvasData.info.ref);
	else
		m_pBackend->setCanvas(canvasData.info.pSurface);

	if( !canvasData.transforms.empty() )
		m_pBackend->setTransforms( canvasData.transforms.data(), canvasData.transforms.data() + canvasData.transforms.size() );
	
	if( !canvasData.objects.empty() )
		m_pBackend->setObjects( canvasData.objects.data(), canvasData.objects.data() + canvasData.objects.size() );
	
	for (int i = 0; i < canvasData.layers.size(); i++)
	{
		spx * pCoordsBeg = canvasData.layers[i].coords.data();
		spx * pCoordsEnd = pCoordsBeg + canvasData.layers[i].coords.size();
		m_pBackend->setCoords(pCoordsBeg, pCoordsEnd );

		HiColor* pColorsBeg = canvasData.layers[i].colors.data();
		HiColor* pColorsEnd = pColorsBeg + canvasData.layers[i].colors.size();
		m_pBackend->setColors(pColorsBeg, pColorsEnd);

		int32_t* pCommandsBeg = canvasData.layers[i].commands.data();
		int32_t* pCommandsEnd = pCommandsBeg + canvasData.layers[i].commands.size();
		m_pBackend->processCommands( pCommandsBeg, pCommandsEnd );
	}
	
	// Release objects
	
	for (auto pObj : canvasData.objects)
	{
		if( pObj )
			pObj->release();
	}

}

//____ fill() _____________________________________________________________

void GfxDeviceGen2::fill(HiColor color)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	// Skip calls that won't affect destination

	if (color.a == 0 && (m_renderState.blendMode == BlendMode::Blend))
		return;

	//

	if (m_stateChanges != 0)
		_encodeStateChanges();

	m_pActiveLayer->commands.push_back(int(Command::Fill));
	m_pActiveLayer->commands.push_back(m_pActiveClipList->nRects);

	m_pActiveLayer->colors.push_back(color);

	const RectSPX* pRect = m_pActiveClipList->pRects;
	auto& coords = m_pActiveLayer->coords;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		coords.emplace_back(pRect->x);
		coords.emplace_back(pRect->y);
		coords.emplace_back(pRect->w);
		coords.emplace_back(pRect->h);
	}
}

//____ fill() _____________________________________________________________

void GfxDeviceGen2::fill(const RectSPX& rect, HiColor color)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	// Skip calls that won't affect destination

	if (!rect.isOverlapping(m_pActiveClipList->bounds))
		return;

	if (color.a == 0 && (m_renderState.blendMode == BlendMode::Blend))
		return;

	// Process rects and put coords in list first.
	// That way we can skip fill with possible state change if no rects get through.

	int nRects = 0;
	auto& coords = m_pActiveLayer->coords;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], rect);

		if (!clipped.isEmpty())
		{
			coords.emplace_back(clipped.x);
			coords.emplace_back(clipped.y);
			coords.emplace_back(clipped.w);
			coords.emplace_back(clipped.h);

			nRects++;
		}
	}

	if (nRects > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		m_pActiveLayer->commands.push_back(int(Command::Fill));
		m_pActiveLayer->commands.push_back(nRects);

		m_pActiveLayer->colors.push_back(color);
	}
}

//____ plotPixels() ____________________________________________________________

void GfxDeviceGen2::plotPixels(int nCoords, const CoordSPX* pCoords, const HiColor* pColors)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	int nCoordsPassed = 0;

	auto& coords = m_pActiveLayer->coords;
	auto& commands = m_pActiveLayer->commands;
	auto& colors = m_pActiveLayer->colors;

	for (int i = 0; i < nCoords; i++)
	{
		for (int r = 0; r < m_pActiveClipList->nRects; r++)
		{
			if (m_pActiveClipList->pRects[r].contains(pCoords[i]) )
			{
				coords.emplace_back( align(pCoords[i].x));
				coords.emplace_back( align(pCoords[i].y));

				colors.push_back(pColors[i]);

				nCoordsPassed++;
				break;
			}
		}
	}

	if (nCoordsPassed > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		commands.push_back(int(Command::Plot));
		commands.push_back(nCoordsPassed);						// Space for ammount
	}
}

//____ drawLine() (start/direction) __________________________________________

void GfxDeviceGen2::drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness)
{
	RectSPX rect;

	switch (dir)
	{
	case Direction::Up:
		rect.x = begin.x - thickness / 2;
		rect.y = begin.y - length;
		rect.w = thickness;
		rect.h = length;
		break;
	case Direction::Down:
		rect.x = begin.x - thickness / 2;
		rect.y = begin.y ;
		rect.w = thickness;
		rect.h = length;
		break;
	case Direction::Left:
		rect.x = begin.x;
		rect.y = begin.y - thickness / 2;
		rect.w = length;
		rect.h = thickness;
		break;
	case Direction::Right:
		rect.x = begin.x - length;
		rect.y = begin.y - thickness / 2;
		rect.w = length;
		rect.h = thickness;
		break;
	}

	fill(rect, color);

}

//____ drawLine() (from/to) __________________________________________

void GfxDeviceGen2::drawLine(CoordSPX beg, CoordSPX end, HiColor color, spx thickness)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	// Skip calls that won't affect destination

	if (color.a == 0 && (m_renderState.blendMode == BlendMode::Blend))
		return;




	if (m_stateChanges != 0)
		_encodeStateChanges();

	int nRects = 0;

	auto& commands = m_pActiveLayer->commands;
	int commandsOfs = (int) commands.size();
	
	m_pActiveLayer->commands.push_back(int(Command::Line));
	m_pActiveLayer->commands.push_back(thickness);
	m_pActiveLayer->commands.push_back(nRects);

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX rect = m_pActiveClipList->pRects[i];
		rect.x -= (thickness/2)+1;
		rect.y -= (thickness/2)+1;
		rect.w += thickness+1;
		rect.h += thickness+1;

		if ( rect.intersectsWithOrContains(beg,end, 8) );
		{
			commands.push_back(m_pActiveClipList->pRects[i].x);
			commands.push_back(m_pActiveClipList->pRects[i].y);
			commands.push_back(m_pActiveClipList->pRects[i].w);
			commands.push_back(m_pActiveClipList->pRects[i].h);

			nRects++;
			break;
		}
	}

	if (nRects > 0)
	{
		commands[commandsOfs + 2] = nRects;

		m_pActiveLayer->coords.push_back(beg.x);
		m_pActiveLayer->coords.push_back(beg.y);
		m_pActiveLayer->coords.push_back(end.x);
		m_pActiveLayer->coords.push_back(end.y);

		m_pActiveLayer->colors.push_back(color);
	}
	else
	{
		commands.resize(commandsOfs);
	}
}

//____ blit() ________________________________________________________________

void GfxDeviceGen2::blit(CoordSPX dest)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}
	
	if (m_renderState.blitSource == nullptr)
		return;
	
	_transformBlitSimple({ dest, m_renderState.blitSource->pixelSize()*64 }, { 0,0 }, 0, Command::Blit);
}

void GfxDeviceGen2::blit(CoordSPX dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}
	
	if (m_renderState.blitSource == nullptr)
		return;

	_transformBlitSimple({ dest, src.size() }, src.pos(), 0, Command::Blit);
}

//____ flipBlit() _________________________________________________________

void GfxDeviceGen2::flipBlit(CoordSPX dest, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	SizeSPX srcSize = m_renderState.blitSource->pixelSize() * 64;

	spx ofsX = srcSize.w * s_blitFlipOffsets[(int)flip][0];
	spx ofsY = srcSize.h * s_blitFlipOffsets[(int)flip][1];

	SizeSPX dstSize = srcSize;
	if (s_blitFlipTransforms[(int)flip][0][0] == 0)
		swap(dstSize.w, dstSize.h);

	_transformBlitSimple({ dest, dstSize }, { ofsX, ofsY }, int(flip), Command::Blit);
}

void GfxDeviceGen2::flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	spx ofsX = src.x + src.w * s_blitFlipOffsets[(int)flip][0];
	spx ofsY = src.y + src.h * s_blitFlipOffsets[(int)flip][1];

	SizeSPX dstSize = src.size();
	if (s_blitFlipTransforms[(int)flip][0][0] == 0)
		swap(dstSize.w, dstSize.h);


	_transformBlitSimple({ dest, dstSize }, { ofsX, ofsY }, int(flip), Command::Blit);
}

//____ stretchBlit() ___________________________________________________________

void GfxDeviceGen2::stretchBlit(const RectSPX& dest)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	stretchBlit(dest, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64));
}

void GfxDeviceGen2::stretchBlit(const RectSPX& dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr || dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
		return;

	if (dest.w == src.w && dest.h == src.h)
	{
		// This is a 1:1 blit, let's use the fast alternative.

		_transformBlitSimple(dest, src.pos(), 0, Command::Blit);
	}
	else
	{
		Transform	mtx;

/*
 if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
 {
	 if (dest.w == 64)
		 mtx[0][0] = 0;
	 else
		 mtx[0][0] = binalInt(src.w-64) * (BINAL_MUL/64) / ((dest.w/64) - 1);

	 mtx[0][1] = 0;
	 mtx[1][0] = 0;

	 if( dest.h == 64)
		 mtx[1][1] = 0;
	 else
		 mtx[1][1] = binalInt(src.h-64) * (BINAL_MUL/64) / ((dest.h/64) - 1);
 }
 */
		
		
		if (m_renderState.blitSource->sampleMethod() == SampleMethod::Bilinear)
		{
			if (dest.w == 64)
				mtx.xx = 0;
			else
				mtx.xx = ((src.w - 65) / 64.f) / ((dest.w / 64) - 1);

			mtx.xy = 0;
			mtx.yx = 0;

			if (dest.h == 64)
				mtx.yy = 0;
			else
				mtx.yy = ((src.h - 65) / 64.f) / ((dest.h / 64) - 1);
		}
		else
		{
			// We want last src sample to be taken as close to the end of the source
			// rectangle as possible in order to get a more balanced representation.

			mtx.xx = src.w / 64.f / (dest.w / 64);
			mtx.xy = 0;
			mtx.yx = 0;
			mtx.yy = src.h / 64.f / (dest.h / 64);
		}

		_transformBlitComplex(dest, { src.x, src.y }, mtx, Command::Blit);
	}
}

//____ stretchFlipBlit() _____________________________________________________

void GfxDeviceGen2::stretchFlipBlit(const RectSPX& dest, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	stretchFlipBlit(dest, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64), flip);
}

void GfxDeviceGen2::stretchFlipBlit(const RectSPX& dest, const RectSPX& src, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	float scaleX, scaleY;
	float ofsX, ofsY;

	float srcW = src.w / 64.f;
	float srcH = src.h / 64.f;

	if (m_renderState.blitSource->sampleMethod() == SampleMethod::Bilinear)
	{
		srcW -= 1.f;
		srcH -= 1.f;

		if (dest.w == 64)
			scaleX = 0;
		else
			scaleX = srcW / ((dest.w / 64) - 1);

		if (dest.h == 64)
			scaleY = 0;
		else
			scaleY = srcH / ((dest.h / 64) - 1);

		ofsX = src.x + (srcW * s_blitFlipOffsets[(int)flip][0]);
		ofsY = src.y + (srcH * s_blitFlipOffsets[(int)flip][1]);
	}
	else
	{

		//TODO: Not sure about this -1. Was -0.99 when dealing with spx and floats which looked suspicious.

		scaleX = (srcW / (dest.w / 64)) - 1;
		scaleY = (srcH / (dest.h / 64)) - 1;

		ofsX = (src.x / 64.f) + (srcW - scaleX) * s_blitFlipOffsets[(int)flip][0];
		ofsY = (src.y / 64.f) + (srcH - scaleY) * s_blitFlipOffsets[(int)flip][1];
	}

	Transform	mtx;

	mtx.xx = scaleX * s_blitFlipTransforms[(int)flip][0][0];
	mtx.xy = scaleY * s_blitFlipTransforms[(int)flip][0][1];
	mtx.yx = scaleX * s_blitFlipTransforms[(int)flip][1][0];
	mtx.yy = scaleY * s_blitFlipTransforms[(int)flip][1][1];

	_transformBlitComplex(dest, { int(ofsX*64), int(ofsY*64) }, mtx, Command::Blit);
}

//____ precisionBlit() ____________________________________________________

void GfxDeviceGen2::precisionBlit(const RectSPX& dest, const RectF& src)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	Transform	mtx;

	if (m_renderState.blitSource->sampleMethod() == SampleMethod::Bilinear)
	{
		mtx.xx = src.w / dest.w;
		mtx.xy = 0;
		mtx.yx = 0;
		mtx.yy = src.h / dest.h;
	}
	else
	{
		mtx.xx = src.w / dest.w;
		mtx.xy = 0;
		mtx.yx = 0;
		mtx.yy = src.h / dest.h;
	}

	_transformBlitComplex(dest, { int(src.x), int(src.y) }, mtx, Command::Blit);
}

//____ transformBlit() ________________________________________________

void GfxDeviceGen2::transformBlit(const RectSPX& dest, CoordF src, const Transform& transform)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	Command cmd = m_renderState.blitSource->isTiling() ? Command::Tile : Command::ClipBlit;

	_transformBlitComplex(dest, { int(src.x), int(src.y) }, transform, cmd);
}

//____ rotScaleBlit() _____________________________________________________

void GfxDeviceGen2::rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
		return;

	if (pSource->m_size.w * scale < 1.f || pSource->m_size.h * scale < 1.f)			// Values very close to zero gives overflow in calculations.
		return;

	CoordF		src;
	Transform	mtx;

	float	sz = (float)sin(-rotationDegrees * 3.14159265 / 180);
	float	cz = (float)cos(-rotationDegrees * 3.14159265 / 180);

	scale = 1.f / scale;

	mtx.xx = cz * scale;
	mtx.xy = sz * scale;

	mtx.yx = -sz * scale;
	mtx.yy = cz * scale;

	src = { srcCenter.x * pSource->m_size.w*64, srcCenter.y * pSource->m_size.h*64  };

	//		src.x -= dest.w / 2.f * mtx[0][0] + dest.h / 2.f * mtx[1][0];
	//		src.y -= dest.w / 2.f * mtx[0][1] + dest.h / 2.f * mtx[1][1];

	src.x -= dest.w * destCenter.x * mtx.xx + dest.h * destCenter.y * mtx.yx;
	src.y -= dest.w * destCenter.x * mtx.xy + dest.h * destCenter.y * mtx.yy;

	Command cmd = pSource->isTiling() ? Command::Tile : Command::ClipBlit;

	_transformBlitComplex(dest, { int(src.x),int(src.y) }, mtx, cmd);
}

//____ tile() _____________________________________________________________

void GfxDeviceGen2::tile(const RectSPX& dest, CoordSPX shift)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
		return;

	if (!pSource->isTiling())
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_transformBlitSimple(dest, shift, 0, Command::Tile);
}

//____ flipTile() _________________________________________________________

void GfxDeviceGen2::flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
		return;

	if (!pSource->isTiling())
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	SizeSPX srcSize = pSource->pixelSize() * 64;

	spx ofsX = srcSize.w * s_blitFlipOffsets[(int)flip][0];
	spx ofsY = srcSize.h * s_blitFlipOffsets[(int)flip][1];

	ofsX += shift.x * s_blitFlipTransforms[(int)flip][0][0] + shift.y * s_blitFlipTransforms[(int)flip][1][0];
	ofsY += shift.x * s_blitFlipTransforms[(int)flip][0][1] + shift.y * s_blitFlipTransforms[(int)flip][1][1];

	SizeSPX dstSize = dest.size();
	if (s_blitFlipTransforms[(int)flip][0][0] == 0)
		swap(dstSize.w, dstSize.h);

	_transformBlitSimple({ dest.pos(), dstSize }, { ofsX, ofsY }, (int)flip, Command::Tile);
}

//____ scaleTile() _________________________________________________________

void GfxDeviceGen2::scaleTile(const RectSPX& dest, float scale, CoordSPX shift)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
		return;

	if (!pSource->isTiling())
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	Transform	mtx;

	mtx.xx = 1 / scale;
	mtx.xy = 0;
	mtx.yx = 0;
	mtx.yy = 1 / scale;

	CoordSPX sh = { int(shift.x / scale), int(shift.y / scale) };

	_transformBlitComplex(dest, sh, mtx, Command::Tile);
}

//____ scaleFlipTile() _________________________________________________________

void GfxDeviceGen2::scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
		return;

	if (!pSource->isTiling())
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling surface", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	Transform	mtx;

	mtx.xx = s_blitFlipTransforms[(int)flip][0][0] / scale;
	mtx.xy = s_blitFlipTransforms[(int)flip][0][1] / scale;
	mtx.yx = s_blitFlipTransforms[(int)flip][1][0] / scale;
	mtx.yy = s_blitFlipTransforms[(int)flip][1][1] / scale;

	SizeI srcSize = pSource->pixelSize();
	spx ofsX = (srcSize.w - 1) * s_blitFlipOffsets[(int)flip][0] * 64;
	spx ofsY = (srcSize.h - 1) * s_blitFlipOffsets[(int)flip][1] * 64;

	ofsX += shift.x * mtx.xx + shift.y * mtx.yx;
	ofsY += shift.x * mtx.xy + shift.y * mtx.yy;

	_transformBlitComplex(dest, { ofsX,ofsY }, mtx, Command::Tile);
}

//____ blur() ________________________________________________________________

void GfxDeviceGen2::blur(CoordSPX dest)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	_transformBlitSimple({ dest, m_renderState.blitSource->pixelSize() * 64 }, { 0,0 }, 0, Command::Blur);
}

void GfxDeviceGen2::blur(CoordSPX dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	_transformBlitSimple({ dest, src.size() }, src.pos(), 0, Command::Blur);
}

//____ stretchBlur() ___________________________________________________________

void GfxDeviceGen2::stretchBlur(const RectSPX& dest)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	stretchBlur(dest, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64));
}

void GfxDeviceGen2::stretchBlur(const RectSPX& dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr || dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
		return;

	if (dest.w == src.w && dest.h == src.h)
	{
		// This is a 1:1 blit, let's use the fast alternative.

		_transformBlitSimple(dest, src.pos(), 0, Command::Blur);
	}
	else
	{
		Transform	mtx;

		if (m_renderState.blitSource->sampleMethod() == SampleMethod::Bilinear)
		{
			if (dest.w == 64)
				mtx.xx = 0;
			else
				mtx.xx = ((src.w - 64) / 64.f) / ((dest.w / 64) - 1);

			mtx.xy = 0;
			mtx.yx = 0;

			if (dest.h == 64)
				mtx.yy = 0;
			else
				mtx.yy = ((src.h - 64) / 64.f) / ((dest.h / 64) - 1);
		}
		else
		{
			// We want last src sample to be taken as close to the end of the source
			// rectangle as possible in order to get a more balanced representation.

			mtx.xx = src.w / 64 / (dest.w / 64);
			mtx.xy = 0;
			mtx.yx = 0;
			mtx.yy = src.h / 64 / (dest.h / 64);
		}

		_transformBlitComplex(dest, { src.x, src.y }, mtx, Command::Blur);
	}
}
 
//____ transformBlur() ________________________________________________

void GfxDeviceGen2::transformBlur(const RectSPX& dest, CoordF src, const Transform& transform)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	_transformBlitComplex(dest, { int(src.x), int(src.y) }, transform, Command::Blur);
}

//____ rotScaleBlur() _____________________________________________________

void GfxDeviceGen2::rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
		return;

	if (pSource->m_size.w * scale < 1.f || pSource->m_size.h * scale < 1.f)			// Values very close to zero gives overflow in calculations.
		return;

	CoordSPX	src;
	Transform	mtx;

	float	sz = (float)sin(-rotationDegrees * 3.14159265 / 180);
	float	cz = (float)cos(-rotationDegrees * 3.14159265 / 180);

	scale = 1.f / scale;

	mtx.xx = cz * scale;
	mtx.xy = sz * scale;

	mtx.yx = -sz * scale;
	mtx.yy = cz * scale;

	src = { int(srcCenter.x * pSource->m_size.w * 64), int(srcCenter.y * pSource->m_size.h * 64) };

	//		src.x -= dest.w / 2.f * mtx[0][0] + dest.h / 2.f * mtx[1][0];
	//		src.y -= dest.w / 2.f * mtx[0][1] + dest.h / 2.f * mtx[1][1];

	src.x -= dest.w * destCenter.x * mtx.xx + dest.h * destCenter.y * mtx.yx;
	src.y -= dest.w * destCenter.x * mtx.xy + dest.h * destCenter.y * mtx.yy;

	_transformBlitComplex(dest, { src.x,src.y }, mtx, Command::Blur);
}

//____ blitNinePatch() ________________________________________________

void GfxDeviceGen2::blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "No blit source set", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	int srcScale = pSource->scale();

	const RectSPX srcRect = align(ptsToSpx(patch.block, srcScale));
	const BorderSPX srcFrame = align(ptsToSpx(patch.frame, srcScale));


	if (srcRect.w == dstRect.w && srcRect.h == dstRect.h && srcFrame == dstFrame)
	{
		blit(dstRect.pos(), srcRect);
		return;
	}

	if ((srcFrame.isEmpty() || dstFrame.isEmpty()) && patch.rigidPartXSections == YSections::None && patch.rigidPartYSections == XSections::None)
	{
		stretchBlit(dstRect, srcRect);
		return;
	}

	//TODO: Optimize! Call transformBlit directly instead of going through stretchBlit(), reuse transforms where possible.
	//TODO: Optimize! Use blit instead of stretchBlit on opportunity,fill if center is only 1 pixel and just blit corners if not stretched.

	SizeSPX srcMidSize(srcRect.w - srcFrame.left - srcFrame.right, srcRect.h - srcFrame.top - srcFrame.bottom);
	SizeSPX dstMidSize(dstRect.w - dstFrame.left - dstFrame.right, dstRect.h - dstFrame.top - dstFrame.bottom);

	spx rigidPartXOfs = align(ptsToSpx(patch.rigidPartXOfs, srcScale));
	spx rigidPartXLength = align(ptsToSpx(patch.rigidPartXLength, srcScale));

	spx rigidPartYOfs = align(ptsToSpx(patch.rigidPartYOfs, srcScale));
	spx rigidPartYLength = align(ptsToSpx(patch.rigidPartYLength, srcScale));

	spx rigidPartXLengthDst = align(ptsToSpx(patch.rigidPartXLength, scale));
	spx rigidPartYLengthDst = align(ptsToSpx(patch.rigidPartYLength, scale));


	if (srcFrame.top + dstFrame.top > 0)
	{
		RectSPX	srcNW(srcRect.x, srcRect.y, srcFrame.left, srcFrame.top);
		RectSPX	srcN(srcRect.x + srcFrame.left, srcRect.y, srcMidSize.w, srcFrame.top);
		RectSPX	srcNE(srcRect.x + srcRect.w - srcFrame.right, srcRect.y, srcFrame.right, srcFrame.top);

		RectSPX	dstNW(dstRect.x, dstRect.y, dstFrame.left, dstFrame.top);
		RectSPX	dstN(dstRect.x + dstFrame.left, dstRect.y, dstMidSize.w, dstFrame.top);
		RectSPX	dstNE(dstRect.x + dstRect.w - dstFrame.right, dstRect.y, dstFrame.right, dstFrame.top);

		if (srcNW.w + dstNW.w > 0)
			stretchBlit(dstNW, srcNW);

		if (srcN.w + dstN.w > 0)
		{
			if ((patch.rigidPartXSections & YSections::Top) != YSections::None)
				_stretchBlitWithRigidPartX(srcN, dstN, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
			else
				stretchBlit(dstN, srcN);
		}

		if (srcNE.w + dstNE.w > 0)
			stretchBlit(dstNE, srcNE);
	}


	if (srcMidSize.h > 0 && dstMidSize.h > 0)
	{
		RectSPX	srcW(srcRect.x, srcRect.y + srcFrame.top, srcFrame.left, srcMidSize.h);
		RectSPX	srcC(srcRect.x + srcFrame.left, srcRect.y + srcFrame.top, srcMidSize.w, srcMidSize.h);
		RectSPX	srcE(srcRect.x + srcRect.w - srcFrame.right, srcRect.y + srcFrame.top, srcFrame.right, srcMidSize.h);

		RectSPX	dstW(dstRect.x, dstRect.y + dstFrame.top, dstFrame.left, dstMidSize.h);
		RectSPX	dstC(dstRect.x + dstFrame.left, dstRect.y + dstFrame.top, dstMidSize.w, dstMidSize.h);
		RectSPX	dstE(dstRect.x + dstRect.w - dstFrame.right, dstRect.y + dstFrame.top, dstFrame.right, dstMidSize.h);

		if (srcW.w + dstW.w > 0)
		{
			if ((patch.rigidPartYSections & XSections::Left) != XSections::None)
				_stretchBlitWithRigidPartY(srcW, dstW, rigidPartYOfs, rigidPartYLength, rigidPartYLengthDst);
			else
				stretchBlit(dstW, srcW);
		}

		if (srcC.w + dstC.w > 0)
		{
			if ((patch.rigidPartYSections & XSections::Center) != XSections::None)
			{
				spx topSrcLen = rigidPartYOfs;
				spx bottomSrcLen = srcC.h - rigidPartYOfs - rigidPartYLength;

				spx totalSrcLen = topSrcLen + bottomSrcLen;

				spx midDstLen = std::min(rigidPartYLengthDst, dstC.h);

				spx topDstLen = (dstC.h - midDstLen) * topSrcLen / totalSrcLen;
				spx bottomDstLen = dstC.h - midDstLen - topDstLen;


				if (topDstLen > 0)
				{
					RectSPX dst(dstC.x, dstC.y, dstC.w, topDstLen);
					RectSPX src(srcC.x, srcC.y, srcC.w, topSrcLen);

					if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
						_stretchBlitWithRigidPartX(src, dst, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
					else
						stretchBlit(dst, src);
				}

				if (midDstLen > 0)
				{
					RectSPX dst(dstC.x, dstC.y + topDstLen, dstC.w, midDstLen);
					RectSPX src(srcC.x, srcC.y + topSrcLen, srcC.w, rigidPartYLength);

					if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
						_stretchBlitWithRigidPartX(src, dst, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
					else
						stretchBlit(dst, src);
				}

				if (bottomDstLen > 0)
				{
					RectSPX dst(dstC.x, dstC.y + topDstLen + midDstLen, dstC.w, bottomDstLen);
					RectSPX src(srcC.x, srcC.y + topSrcLen + rigidPartYLength, srcC.w, bottomSrcLen);

					if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
						_stretchBlitWithRigidPartX(src, dst, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
					else
						stretchBlit(dst, src);
				}
			}
			else
			{
				if ((patch.rigidPartXSections & YSections::Center) != YSections::None)
					_stretchBlitWithRigidPartX(srcC, dstC, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
				else
					stretchBlit(dstC, srcC);
			}
		}


		if (srcE.w + dstE.w > 0)
		{
			if ((patch.rigidPartYSections & XSections::Right) != XSections::None)
				_stretchBlitWithRigidPartY(srcE, dstE, rigidPartYOfs, rigidPartYLength, rigidPartYLengthDst);
			else
				stretchBlit(dstE, srcE);
		}

	}

	if (srcFrame.bottom + dstFrame.bottom > 0)
	{
		RectSPX	srcSW(srcRect.x, srcRect.y + srcRect.h - srcFrame.bottom, srcFrame.left, srcFrame.bottom);
		RectSPX	srcS(srcRect.x + srcFrame.left, srcRect.y + srcRect.h - srcFrame.bottom, srcMidSize.w, srcFrame.bottom);
		RectSPX	srcSE(srcRect.x + srcRect.w - srcFrame.right, srcRect.y + srcRect.h - srcFrame.bottom, srcFrame.right, srcFrame.bottom);

		RectSPX	dstSW(dstRect.x, dstRect.y + dstRect.h - dstFrame.bottom, dstFrame.left, dstFrame.bottom);
		RectSPX	dstS(dstRect.x + dstFrame.left, dstRect.y + dstRect.h - dstFrame.bottom, dstMidSize.w, dstFrame.bottom);
		RectSPX	dstSE(dstRect.x + dstRect.w - dstFrame.right, dstRect.y + dstRect.h - dstFrame.bottom, dstFrame.right, dstFrame.bottom);

		if (srcSW.w + dstSW.w > 0)
			stretchBlit(dstSW, srcSW);

		if (srcS.w + dstS.w > 0)
		{
			if ((patch.rigidPartXSections & YSections::Bottom) != YSections::None)
				_stretchBlitWithRigidPartX(srcS, dstS, rigidPartXOfs, rigidPartXLength, rigidPartXLengthDst);
			else
				stretchBlit(dstS, srcS);
		}

		if (srcSE.w + dstSE.w > 0)
			stretchBlit(dstSE, srcSE);
	}
}

//____ drawEdgemap() ___________________________________________________________

void GfxDeviceGen2::drawEdgemap(CoordSPX dest, Edgemap* pEdgemap)
{
	flipDrawEdgemap(dest, pEdgemap, GfxFlip::None);
}

//____ flipDrawEdgemap() _______________________________________________________

void GfxDeviceGen2::flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	RectSPX rect = { dest, pEdgemap->pixelSize() * 64 };

	int nRects = 0;
	auto& coords = m_pActiveLayer->coords;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], rect);

		if (!clipped.isEmpty())
		{
			coords.emplace_back(clipped.x);
			coords.emplace_back(clipped.y);
			coords.emplace_back(clipped.w);
			coords.emplace_back(clipped.h);

			nRects++;
		}
	}

	if (nRects > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		m_pActiveLayer->commands.push_back(int(Command::DrawEdgemap));
		m_pActiveLayer->commands.push_back(int(m_pActiveCanvas->objects.size()));
		m_pActiveLayer->commands.push_back(dest.x);
		m_pActiveLayer->commands.push_back(dest.y);
		m_pActiveLayer->commands.push_back(int(flip));
		m_pActiveLayer->commands.push_back(nRects);

		m_pActiveCanvas->objects.emplace_back(pEdgemap);
	}
}



//____ _stretchBlitWithRigidPartX() _______________________________________

void GfxDeviceGen2::_stretchBlitWithRigidPartX(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst)
{
	int leftSrcLen = rigidPartOfs;
	int rightSrcLen = src.w - rigidPartOfs - rigidPartLength;

	int totalSrcLen = leftSrcLen + rightSrcLen;

	int midDstLen = std::min(rigidPartLengthDst, dst.w);

	int leftDstLen = (dst.w - midDstLen) * leftSrcLen / totalSrcLen;
	int rightDstLen = dst.w - midDstLen - leftDstLen;

	if (leftDstLen > 0)
		stretchBlit(RectSPX(dst.x, dst.y, leftDstLen, dst.h), RectSPX(src.x, src.y, leftSrcLen, src.h));

	if (midDstLen > 0)
		stretchBlit(RectSPX(dst.x + leftDstLen, dst.y, midDstLen, dst.h), RectSPX(src.x + leftSrcLen, src.y, rigidPartLength, src.h));

	if (rightDstLen > 0)
		stretchBlit(RectSPX(dst.x + leftDstLen + midDstLen, dst.y, rightDstLen, dst.h), RectSPX(src.x + leftSrcLen + rigidPartLength, src.y, rightSrcLen, src.h));
}

//____ _stretchBlitWithRigidPartY() _______________________________________

void GfxDeviceGen2::_stretchBlitWithRigidPartY(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst)
{
	int topSrcLen = rigidPartOfs;
	int bottomSrcLen = src.h - rigidPartOfs - rigidPartLength;

	int totalSrcLen = topSrcLen + bottomSrcLen;

	int midDstLen = std::min(rigidPartLengthDst, dst.h);

	int topDstLen = (dst.h - midDstLen) * topSrcLen / totalSrcLen;
	int bottomDstLen = dst.h - midDstLen - topDstLen;

	if (topDstLen > 0)
		stretchBlit(RectSPX(dst.x, dst.y, dst.w, topDstLen), RectSPX(src.x, src.y, src.w, topSrcLen));

	if (midDstLen > 0)
		stretchBlit(RectSPX(dst.x, dst.y + topDstLen, dst.w, midDstLen), RectSPX(src.x, src.y + topSrcLen, src.w, rigidPartLength));

	if (bottomDstLen > 0)
		stretchBlit(RectSPX(dst.x, dst.y + topDstLen + midDstLen, dst.w, bottomDstLen), RectSPX(src.x, src.y + topSrcLen + rigidPartLength, src.w, bottomSrcLen));
}



//____ _transformBlitSimple() ______________________________________________________

void GfxDeviceGen2::_transformBlitSimple(const RectSPX& _dest, CoordSPX src, int transformOfs, Command cmd)
{
	// Skip calls that won't affect destination

	if (!_dest.isOverlapping(m_pActiveClipList->bounds))
		return;

	// Process rects and put coords in list first.
	// That way we can skip blit with possible state change if no rects get through.

	int nRects = 0;
	auto& coords = m_pActiveLayer->coords;

	//TODO: Proper 26:6 support
	RectSPX dest = align(_dest);

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX rect = RectSPX::overlap(m_pActiveClipList->pRects[i], dest);

		if( !rect.isEmpty() )
		{
			coords.emplace_back(rect.x);
			coords.emplace_back(rect.y);
			coords.emplace_back(rect.w);
			coords.emplace_back(rect.h);

			nRects++;
		}
	}
	
	if( nRects > 0 )
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		m_pActiveLayer->commands.push_back(int(cmd));
		m_pActiveLayer->commands.push_back(nRects);
		
		m_pActiveLayer->commands.push_back(transformOfs);
		m_pActiveLayer->commands.push_back( align(src.x) );
		m_pActiveLayer->commands.push_back( align(src.y) );
		m_pActiveLayer->commands.push_back( dest.x );
		m_pActiveLayer->commands.push_back( dest.y );
	}
}

//____ _transformBlitComplex() _______________________________________________

void GfxDeviceGen2::_transformBlitComplex(const RectSPX& _dest, CoordSPX src, const Transform& matrix, Command cmd)
{
	// Clip and render the patches

	if (!_dest.isOverlapping(m_pActiveClipList->bounds))
		return;

	//TODO: Proper 26:6 support
	RectSPX dest = align(_dest);

	const RectI& clip = dest;

	int nRects = 0;
	auto& coords = m_pActiveLayer->coords;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX rect = RectSPX::overlap(m_pActiveClipList->pRects[i], dest);

		if (!rect.isEmpty())
		{
			coords.emplace_back(rect.x);
			coords.emplace_back(rect.y);
			coords.emplace_back(rect.w);
			coords.emplace_back(rect.h);

			nRects++;
		}
	}

	if (nRects > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		int transformOfs = int(m_pActiveCanvas->transforms.size()) + GfxFlip_size;
		m_pActiveCanvas->transforms.emplace_back(matrix);

		m_pActiveLayer->commands.push_back(int(cmd));
		m_pActiveLayer->commands.push_back(nRects);

		m_pActiveLayer->commands.push_back(transformOfs);
		m_pActiveLayer->commands.push_back(src.x);
		m_pActiveLayer->commands.push_back(src.y);
		m_pActiveLayer->commands.push_back(dest.x);
		m_pActiveLayer->commands.push_back(dest.y);
	}
}



//____ _resetState() ___________________________________________________________

void GfxDeviceGen2::_resetState(RenderState& state)
{
	state.blitSource = nullptr;
	state.tintColor = HiColor::Undefined;
	state.tintmapRect.clear();
	state.pTintmap = nullptr;
	state.blendMode = BlendMode::Blend;
	state.morphFactor = 0.5f;
	state.fixedBlendColor = HiColor::White;
	state.pBlurbrush = nullptr;

	m_stateChanges = 0;
}


//____ _encodeStateChanges() ______________________________________________

void GfxDeviceGen2::_encodeStateChanges()
{

	RenderState& newState = m_renderState;
	RenderState& encodedState = m_pActiveLayer->encodedState;

	//

	auto& cmdBuffer = m_pActiveLayer->commands;
	auto& colorBuffer = m_pActiveLayer->colors;

	int commandOfs = (int) cmdBuffer.size();

	cmdBuffer.push_back(int(Command::StateChange));
	cmdBuffer.push_back(0);										// States changed to be filled in later.

	uint8_t	statesChanged = 0;

	if (m_stateChanges & uint8_t(StateChange::BlitSource))
	{

		Surface* pSource = newState.blitSource.rawPtr();
		Surface* pOldSource = encodedState.blitSource.rawPtr();

		if (pSource != pOldSource)
		{
			cmdBuffer.push_back((int)m_pActiveCanvas->objects.size());
			m_pActiveCanvas->objects.push_back(pSource);
			if( pSource )
				pSource->retain();

			encodedState.blitSource = pSource;
			statesChanged |= int(StateChange::BlitSource);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::BlendMode))
	{
		if (newState.blendMode != encodedState.blendMode)
		{
			cmdBuffer.push_back(int(newState.blendMode));
			encodedState.blendMode = newState.blendMode;
			statesChanged |= int(StateChange::BlendMode);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::TintColor))
	{
		if (newState.tintColor != encodedState.tintColor ||
			newState.pTintmap != encodedState.pTintmap ||
			newState.tintmapRect != encodedState.tintmapRect)
		{
			if (!newState.tintColor.isUndefined())
			{
				colorBuffer.push_back(newState.tintColor);
				encodedState.tintColor = newState.tintColor;
				statesChanged |= int(StateChange::TintColor);
			}
			else
			{
				Object* pTintmap = newState.pTintmap.rawPtr();

				cmdBuffer.push_back((int)m_pActiveCanvas->objects.size());
				m_pActiveCanvas->objects.push_back(pTintmap);
				
				if( pTintmap )
					pTintmap->retain();

				cmdBuffer.push_back(newState.tintmapRect.x);
				cmdBuffer.push_back(newState.tintmapRect.y);
				cmdBuffer.push_back(newState.tintmapRect.w);
				cmdBuffer.push_back(newState.tintmapRect.h);

				encodedState.pTintmap = newState.pTintmap;
				encodedState.tintmapRect = newState.tintmapRect;

				statesChanged |= int(StateChange::TintMap);
			}
		}
	}

	if (m_stateChanges & uint8_t(StateChange::MorphFactor))
	{
		if (newState.morphFactor != encodedState.morphFactor)
		{
			cmdBuffer.push_back(newState.morphFactor * 4096);
			encodedState.morphFactor = newState.morphFactor;
			statesChanged |= int(StateChange::MorphFactor);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::FixedBlendColor))
	{
		if (newState.fixedBlendColor != encodedState.fixedBlendColor)
		{
			colorBuffer.push_back(newState.fixedBlendColor);
			encodedState.fixedBlendColor = newState.fixedBlendColor;
			statesChanged |= int(StateChange::FixedBlendColor);
		}

	}

	if (m_stateChanges & uint8_t(StateChange::Blur))
	{
		if (newState.pBlurbrush != encodedState.pBlurbrush)
		{
			Object* pBlurbrush = newState.pBlurbrush.rawPtr();

			cmdBuffer.push_back((int)m_pActiveCanvas->objects.size());
			m_pActiveCanvas->objects.push_back(pBlurbrush);
			if( pBlurbrush )
				pBlurbrush->retain();

			encodedState.pBlurbrush = newState.pBlurbrush;
			statesChanged |= int(StateChange::Blur);

		}
	}

	//

	if (statesChanged == 0)
		cmdBuffer.resize(commandOfs);
	else
		cmdBuffer[commandOfs + 1] = statesChanged;

	m_stateChanges = 0;
}


} // namspace wg
