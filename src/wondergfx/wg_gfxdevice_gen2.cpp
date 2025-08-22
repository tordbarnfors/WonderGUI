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
#include <wg_waveform.h>

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

//____ setTintColor() _______________________________________________________________

void GfxDeviceGen2::setTintColor(HiColor color)
{
	assert(color == HiColor::Undefined || color.isValid());

	if (color != m_renderState.tintColor)
	{
		m_renderState.tintColor = color;
		m_stateChanges |= int(StateChange::TintColor);
	}
}

//____ clearTintColor() _____________________________________________________________

void GfxDeviceGen2::clearTintColor()
{
	if (!m_renderState.tintColor.isUndefined() )
	{
		m_stateChanges |= int(StateChange::TintColor);
		m_renderState.tintColor = HiColor::Undefined;
	}
}

//____ hasTintColor() _________________________________________________________

bool GfxDeviceGen2::hasTintColor() const
{
	return m_renderState.tintColor != HiColor::Undefined;
}

//____ tintColor() ________________________________________________________

HiColor GfxDeviceGen2::tintColor() const
{
	return m_renderState.tintColor == HiColor::Undefined ? HiColor::White : m_renderState.tintColor;
}


//____ setTintmap() ___________________________________________________________

void GfxDeviceGen2::setTintmap(const RectSPX& rect, Tintmap* pTintmap)
{
	assert(pTintmap);

	if (pTintmap != m_renderState.pTintmap || rect != m_renderState.tintmapRect )
	{
		m_renderState.pTintmap = pTintmap;
		m_renderState.tintmapRect = rect;

		m_stateChanges |= int(StateChange::TintMap);
	}
}

//____ clearTintmap() _____________________________________________________________

void GfxDeviceGen2::clearTintmap()
{
	if (m_renderState.pTintmap)
		m_stateChanges |= int(StateChange::TintMap);

	m_renderState.pTintmap = nullptr;
	m_renderState.tintmapRect.clear();
}

//____ hasTintmap() _________________________________________________________

bool GfxDeviceGen2::hasTintmap() const
{
	return m_renderState.pTintmap != nullptr;
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

//____ setTintGradient() __________________________________________________

void GfxDeviceGen2::setTintGradient(const RectSPX& rect, const Gradient& gradient)
{
	setTintmap(rect, Gradyent::create( gradient ));
}

//____ clearTintGradient() ________________________________________________

void GfxDeviceGen2::clearTintGradient()
{
	clearTintmap();
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
	m_pBackend->beginRender();
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
	m_pBackend->endRender();

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
		m_pActiveCanvas->savedStateChanges = m_stateChanges;
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

	// Clear session info

	memset(&entry.sessionInfo, 0, sizeof(entry.sessionInfo));

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

	// Create canvases for render layers that need them

	for (int i = 1; i < nLayers; i++)
	{
		auto& info = pLayers->m_layers[i - 1];
		auto& layer = entry.layers[i];

		if (info.format != PixelFormat::Undefined || info.blendFunc || info.clearFunc || info.preBlendFunc)
		{
			// Create our layer canvas

			layer.pLayerCanvas = m_pBackend->surfaceFactory()->createSurface(WGBP(Surface,
				_.size = entry.info.size / 64,
				_.format = info.format,
				_.canvas = true));

			setRenderLayer(i);

			if (info.clearFunc)
			{
				info.clearFunc(this);
			}
			else
			{
				setBlendMode(BlendMode::Replace);
				fill(HiColor::TransparentBlack);
				setBlendMode(BlendMode::Blend);
			}
		}
	}

	// Encode clearing of base canvas if clearCanvasFunc set.

	if (pLayers)
	{
		if (pLayers->m_clearCanvasFunc)
		{
			entry.activeLayer = 0;
			m_pActiveLayer = &entry.layers[0];

			pLayers->m_clearCanvasFunc(this);

			entry.activeLayer = activeLayer;
			m_pActiveLayer = &entry.layers[activeLayer];
		}
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
	
	_doFlattenLayers();
	m_canvasStack.pop_back();
	
	if (!m_canvasStack.empty())
	{
		m_renderState = m_canvasStack.back().savedState;
		m_stateChanges = m_canvasStack.back().savedStateChanges;
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
/*
	Flatten layers within current cliprects and resets canvas for new
	drawing operations.

	This means that any drawing performed outside current cliprects is lost.
*/

void GfxDeviceGen2::flattenLayers()
{
	_doFlattenLayers(); 
	_resetCanvas();
}


//____ clearLayers() ___________________________________________________________
/*
	Encodes clear commands into all layers. 
	
	Resets blendmode, tint and renderlayer.

	Current cliprects clips the clearing.
*/

void GfxDeviceGen2::clearLayers()
{
	auto& canvasEntry = m_canvasStack.back();

	setBlendMode(BlendMode::Blend);
	clearTintmap();
	clearTintColor();

	for (int i = 1; i < canvasEntry.layers.size(); i++)
	{
		auto& info = canvasEntry.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasEntry.layers[i];

		if (layer.pLayerCanvas)
		{
			setRenderLayer(i);

			if (info.clearFunc)
			{
				info.clearFunc(this);
			}
			else
			{
				setBlendMode(BlendMode::Replace);
				fill(HiColor::TransparentBlack);
				setBlendMode(BlendMode::Blend);
			}
		}
	}

	setRenderLayer(0);
}


//____ _resetCanvas() __________________________________________________________
/*
	Prepares the top canvas in the stack for a new round of rendering
	after having flattened the layers, this involves:

	* Clearing all buffers (commands, colors, coords, objects and stransforms)
	* Clear session info
	* Resetting all layers render states.
*/

void GfxDeviceGen2::_resetCanvas()
{
	auto& canvasEntry = m_canvasStack.back();

	canvasEntry.transforms.clear();

	// Clear session info

	memset(&canvasEntry.sessionInfo, 0, sizeof(canvasEntry.sessionInfo));


	for (int i = 0; i < canvasEntry.layers.size(); i++)
	{
		auto& info = canvasEntry.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasEntry.layers[i];

		layer.objects.clear();
		layer.colors.clear();
		layer.rects.clear();
		layer.commands.clear();

		layer.latestCommand = Command::None;
		layer.latestCommandOfs = -1;

		_resetState(layer.encodedState);

		layer.finalCommandsOfs = -1;
	}
}


//____ _doFlattenLayers() ____________________________________________________

void GfxDeviceGen2::_doFlattenLayers()
{
	auto& canvasData = m_canvasStack.back();

	// Finalize additional layers

	for (int i = 1; i < canvasData.layers.size(); i++)
	{
		auto& info = canvasData.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasData.layers[i];

		if (layer.pLayerCanvas)
		{
			setRenderLayer(i);

			// Encode preBlend call if any.

			if (info.preBlendFunc)
			{
				setBlendMode(BlendMode::Blend);
				clearTintmap();
				clearTintColor();
				info.preBlendFunc(this);
			}

			// Store length of commands before changing canvas.

			layer.finalCommandsOfs = (int) layer.commands.size();

			// Reset latestCommand since we need to keep these separated.

			layer.latestCommand = Command::None;
			layer.latestCommandOfs = -1;

			// Encode preBlendCanvasFunc if any.

			if (info.preBlendCanvasFunc)
			{
				setBlendMode(BlendMode::Blend);
				clearTintmap();
				clearTintColor();
				info.preBlendCanvasFunc(this);
			}

			// Encode blendFunc if present, otherwise encode default blit.

			setBlendMode(BlendMode::Blend);
			clearTintmap();
			clearTintColor();
			setBlitSource(layer.pLayerCanvas);

			if (info.blendFunc)
				info.blendFunc(this);
			else
				blit({ 0,0 });

			setBlitSource(nullptr);				// Prevent layerCanvas from being retained.
		}
		else
		{
			// Encode preBlendCanvasFunc if set

			if (info.preBlendCanvasFunc)
			{
				setRenderLayer(i);		// Write our commands to buffer to be processed
				setBlendMode(BlendMode::Blend);
				clearTintmap();
				clearTintColor();

				info.preBlendCanvasFunc(this);
			}

		}
	}

	// Encode finalizeCanvasFunc (if specified) into our last layer.

	if (canvasData.pLayerInfo && canvasData.pLayerInfo->m_finalizeCanvasFunc)
	{
		setRenderLayer(int(canvasData.layers.size())-1);					// Write to buffer to be processed
		setBlendMode(BlendMode::Blend);
		clearTintmap();
		clearTintColor();

		canvasData.pLayerInfo->m_finalizeCanvasFunc(this);
	}

	// Start backend session

	canvasData.sessionInfo.nObjects = 0;
	canvasData.sessionInfo.nTransforms = (int) canvasData.transforms.size();
	canvasData.sessionInfo.nSetCanvas = (int) (canvasData.layers.size()-1)*2;
	canvasData.sessionInfo.nColors = 0;
	canvasData.sessionInfo.nRects = 0;

	for (auto& layer : canvasData.layers)
	{
		canvasData.sessionInfo.nColors += layer.colors.size();
		canvasData.sessionInfo.nRects += layer.rects.size();
		canvasData.sessionInfo.nObjects += layer.objects.size();
	}

	if( canvasData.sessionInfo.nRects == 0 )
		return;			// No rendering to be done.

	m_pBackend->beginSession(canvasData.info.ref, canvasData.info.pSurface, canvasData.updateRects.nRects, canvasData.updateRects.pRects, &canvasData.sessionInfo);

	// Send transforms
		
	if( !canvasData.transforms.empty() )
		m_pBackend->setTransforms( canvasData.transforms.data(), canvasData.transforms.data() + canvasData.transforms.size() );
		
	// Render base layer

	auto& baseLayer = canvasData.layers[0];

	if( !baseLayer.commands.empty() )
	{
		if( !baseLayer.objects.empty() )
			m_pBackend->setObjects( baseLayer.objects.data(), baseLayer.objects.data() + baseLayer.objects.size() );

		RectSPX* pRectsBeg = baseLayer.rects.data();
		RectSPX* pRectsEnd = pRectsBeg + baseLayer.rects.size();
		m_pBackend->setRects(pRectsBeg, pRectsEnd);

		HiColor* pColorsBeg = baseLayer.colors.data();
		HiColor* pColorsEnd = pColorsBeg + baseLayer.colors.size();
		m_pBackend->setColors(pColorsBeg, pColorsEnd);

		uint8_t* pCommandsBeg = baseLayer.commands.data();
		uint8_t* pCommandsEnd = pCommandsBeg + baseLayer.commands.size();
		m_pBackend->processCommands( (uint16_t*) pCommandsBeg, (uint16_t*) pCommandsEnd);
	}


	// Render additional layers

	for (int i = 1; i < canvasData.layers.size(); i++)
	{
		auto& info = canvasData.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasData.layers[i];

		if( layer.commands.size() == 0 )
			continue;

		// Check if we can do direct rendering

		if (layer.pLayerCanvas )
		{
			setRenderLayer(i);

			// Set layerCanvas as output destination

			m_pBackend->setCanvas(layer.pLayerCanvas);

			// Send buffers and layer-specific commands to backend

			if( !layer.objects.empty() )
				m_pBackend->setObjects( layer.objects.data(), layer.objects.data() + layer.objects.size() );

			RectSPX* pRectsBeg = layer.rects.data();
			RectSPX* pRectsEnd = pRectsBeg + layer.rects.size();
			m_pBackend->setRects(pRectsBeg, pRectsEnd);

			HiColor* pColorsBeg = layer.colors.data();
			HiColor* pColorsEnd = pColorsBeg + layer.colors.size();
			m_pBackend->setColors(pColorsBeg, pColorsEnd);

			uint8_t* pCommandsBeg = layer.commands.data();
			uint8_t* pCommandsEnd = pCommandsBeg + layer.finalCommandsOfs;
			m_pBackend->processCommands( (uint16_t*) pCommandsBeg, (uint16_t*) pCommandsEnd);

			// Set base canvas

			if (canvasData.info.ref != CanvasRef::None)
				m_pBackend->setCanvas(canvasData.info.ref);
			else
				m_pBackend->setCanvas(canvasData.info.pSurface);

			// Send preBlendCanvas and blend commands to backend
			// causing blending of layer canvas 

			pCommandsBeg = layer.commands.data() + layer.finalCommandsOfs;
			pCommandsEnd = layer.commands.data() + layer.commands.size();
			m_pBackend->processCommands( (uint16_t*) pCommandsBeg, (uint16_t*) pCommandsEnd);

		}
		else
		{
			// Send buffers and commands to backend

			RectSPX* pRectsBeg = layer.rects.data();
			RectSPX* pRectsEnd = pRectsBeg + layer.rects.size();
			m_pBackend->setRects(pRectsBeg, pRectsEnd);

			HiColor* pColorsBeg = layer.colors.data();
			HiColor* pColorsEnd = pColorsBeg + layer.colors.size();
			m_pBackend->setColors(pColorsBeg, pColorsEnd);

			uint8_t* pCommandsBeg = layer.commands.data();
			uint8_t* pCommandsEnd = pCommandsBeg + layer.commands.size();
			m_pBackend->processCommands( (uint16_t*) pCommandsBeg, (uint16_t*) pCommandsEnd);
		}
	}
	
	// End backend session

	m_pBackend->endSession();

	// Release objects

	for (auto& layer : canvasData.layers )
	{
		for (auto pObj : layer.objects)
		{
			if( pObj )
				pObj->release();
		}
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

	int nRects = m_pActiveClipList->nRects;

	if( m_pActiveLayer->latestCommand == Command::Fill && m_pActiveLayer->colors.back() == color && m_pActiveLayer->commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() + nRects < 65536 )
	{
		m_pActiveLayer->commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() += nRects;
	}
	else
	{
		m_pActiveLayer->latestCommand = Command::Fill;
		m_pActiveLayer->latestCommandOfs = (int) m_pActiveLayer->commands.size();

		m_pActiveLayer->commands.secureSpace(4);
		m_pActiveLayer->commands.pushUnchecked(uint16_t(Command::Fill));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(nRects));
	}


	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < nRects; i++)
		rects.emplace_back(m_pActiveClipList->pRects[i]);

	m_pActiveLayer->colors.push_back(color);

	m_pActiveCanvas->sessionInfo.nFill+= nRects;
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
	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], rect);

		if (!clipped.isEmpty())
		{
			rects.emplace_back(clipped);
			nRects++;
		}
	}

	if (nRects > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		if( m_pActiveLayer->latestCommand == Command::Fill && m_pActiveLayer->colors.back() == color && m_pActiveLayer->commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() + nRects < 65536 )
		{
			m_pActiveLayer->commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() += nRects;
		}
		else
		{
			m_pActiveLayer->latestCommand = Command::Fill;
			m_pActiveLayer->latestCommandOfs = (int) m_pActiveLayer->commands.size();

			m_pActiveLayer->commands.secureSpace(4);
			m_pActiveLayer->commands.pushUnchecked(uint16_t(Command::Fill));
			m_pActiveLayer->commands.pushUnchecked(uint16_t(nRects));

			m_pActiveLayer->colors.push_back(color);
		}


		m_pActiveCanvas->sessionInfo.nFill += nRects;
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

	auto& commands = m_pActiveLayer->commands;

	if( m_pActiveLayer->latestCommand == Command::Line && commands.at(m_pActiveLayer->latestCommandOfs+4).as<uint16_t>() < 65535 )
	{
		commands.at(m_pActiveLayer->latestCommandOfs+4).as<uint16_t>() += 1;
	}
	else
	{
		m_pActiveLayer->latestCommand = Command::Line;
		m_pActiveLayer->latestCommandOfs = (int) m_pActiveLayer->commands.size();

		commands.secureSpace(8);
		commands.pushUnchecked(uint16_t(Command::Line));
		commands.pushUnchecked(uint16_t(m_pActiveClipList->nRects));
		commands.pushUnchecked(uint16_t(1));							// nLines.
		commands.pushUnchecked(uint16_t(0));							// padding

		const RectSPX* pClipRects = m_pActiveClipList->pRects;

		for (int i = 0; i < m_pActiveClipList->nRects; i++)
			m_pActiveLayer->rects.push_back(pClipRects[i]);

		m_pActiveCanvas->sessionInfo.nLineClipRects += m_pActiveClipList->nRects;
	}

	commands.secureSpace(20);
	commands.pushUnchecked(beg);
	commands.pushUnchecked(end);
	commands.pushUnchecked(uint16_t(thickness));
	commands.pushUnchecked(uint16_t(0));						// padding

	m_pActiveLayer->colors.push_back(color);

	m_pActiveCanvas->sessionInfo.nLines++;
	m_pActiveCanvas->sessionInfo.nLineCoords+=2;
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

		_transformBlitComplex(dest, { src.x*16, src.y*16 }, mtx, Command::Blit);
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

	_transformBlitComplex(dest, { int(ofsX*1024), int(ofsY*1024) }, mtx, Command::Blit);
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

	_transformBlitComplex(dest, { int(src.x*16), int(src.y*16) }, mtx, Command::Blit);
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

	_transformBlitComplex(dest, { int(src.x*16), int(src.y*16) }, transform, cmd);
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

	_transformBlitComplex(dest, { int(src.x*16),int(src.y*16) }, mtx, cmd);
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

	CoordSPX sh = { int(shift.x*16 / scale), int(shift.y*16 / scale) };

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
	spx ofsX = (srcSize.w - 1) * s_blitFlipOffsets[(int)flip][0] * 1024;
	spx ofsY = (srcSize.h - 1) * s_blitFlipOffsets[(int)flip][1] * 1024;

	ofsX += shift.x * 16 * mtx.xx + shift.y * 16 * mtx.yx;
	ofsY += shift.x * 16 * mtx.xy + shift.y * 16 * mtx.yy;

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

	if( m_renderState.pBlurbrush == nullptr )
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

	if( m_renderState.pBlurbrush == nullptr )
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

	if( m_renderState.pBlurbrush == nullptr )
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

	if( m_renderState.pBlurbrush == nullptr )
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

		_transformBlitComplex(dest, { src.x*16, src.y*16 }, mtx, Command::Blur);
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

	if( m_renderState.pBlurbrush == nullptr )
	{
		//TODO: Error handling!

		return;
	}

	if (m_renderState.blitSource == nullptr)
		return;

	_transformBlitComplex(dest, { int(src.x*16), int(src.y*16) }, transform, Command::Blur);
}

//____ rotScaleBlur() _____________________________________________________

void GfxDeviceGen2::rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
{
	if (!m_pActiveCanvas)
	{
		//TODO: Error handling!

		return;
	}

	if( m_renderState.pBlurbrush == nullptr )
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

	src = { int(srcCenter.x * pSource->m_size.w * 1024), int(srcCenter.y * pSource->m_size.h * 1024) };

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

	SizeSPX edgemapSize = pEdgemap->pixelSize() * 64;
	const int(&transform)[2][2] = s_blitFlipTransforms[(int)flip];

	RectSPX rect;

	rect.x = dest.x;
	rect.y = dest.y;
	rect.w = edgemapSize.w * abs(transform[0][0]) + edgemapSize.h * abs(transform[1][0]);
	rect.h = edgemapSize.w * abs(transform[0][1]) + edgemapSize.h * abs(transform[1][1]);


	int nRects = 0;
	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], rect);

		if (!clipped.isEmpty())
		{
			rects.emplace_back(clipped);
			nRects++;
		}
	}

	if (nRects > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		m_pActiveLayer->latestCommand = Command::DrawEdgemap;

		m_pActiveLayer->commands.secureSpace(16);

		m_pActiveLayer->commands.pushUnchecked(uint16_t(Command::DrawEdgemap));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(nRects));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(flip));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(0));									// padding
		m_pActiveLayer->commands.pushUnchecked(dest);

		m_pActiveLayer->objects.push_back(pEdgemap);
		pEdgemap->retain();

		m_pActiveCanvas->sessionInfo.nEdgemapDraws++;
		m_pActiveCanvas->sessionInfo.nObjects++;
	}
}

//____ drawWave() ____________________________________________________________

void GfxDeviceGen2::drawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor fill)
{
	flipDrawWave(dest, pTopBorder, pBottomBorder, fill, GfxFlip::None);
}

//____ flipDrawWave() __________________________________________________________

void GfxDeviceGen2::flipDrawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor fill, GfxFlip flip)
{
	if (!m_pActiveCanvas || !m_pBackend )
	{
		//TODO: Error handling!

		return;
	}

	RectI pixDest = dest / 64;

	// Create waveform

	auto pWave = Waveform::create( WGBP(Waveform, 
		_.size = dest.size()/64, 
		_.color = fill,
		_.topOutlineThickness = pTopBorder->thickness, 
		_.bottomOutlineThickness = pBottomBorder->thickness,
		_.outlineColor = pTopBorder->color ),
		m_pBackend->edgemapFactory() );

	int samplesWanted = pixDest.w + 1;

	// Reserve temporary work buffers, we need to convert samples from 24:8 to spx (26:6) format.

	auto pBuffer = GfxBase::memStackAlloc(samplesWanted * sizeof(spx) * 2 );
	auto pTopWaveSamples = (spx*)pBuffer;
	auto pBottomWaveSamples = pTopWaveSamples + samplesWanted;

	// Fill in top border samples

	int spl = 0;
	int copySamples = std::min(samplesWanted, pTopBorder->length);

	while (spl < copySamples)
	{
		pTopWaveSamples[spl] = pTopBorder->pWave[spl];
		spl++;
	}

	while (spl < samplesWanted)
		pTopWaveSamples[spl++] = pTopBorder->hold;

	// Fill in bottom border samples

	spl = 0;
	copySamples = std::min(samplesWanted, pBottomBorder->length);

	while (spl < copySamples)
	{
		pBottomWaveSamples[spl] = pBottomBorder->pWave[spl];
		spl++;
	}

	while (spl < samplesWanted)
		pBottomWaveSamples[spl++] = pBottomBorder->hold;

	// Set samples and return temporary memory

	pWave->setSamples(0, samplesWanted, pTopWaveSamples, pBottomWaveSamples);

	GfxBase::memStackFree(samplesWanted * sizeof(spx) * 2);

	// Render

	auto pEdgemap = pWave->refresh();

	flipDrawEdgemap(dest.pos(), pEdgemap, flip);
}

//____ drawSegments() ________________________________________________________

void GfxDeviceGen2::drawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, TintMode tintMode)
{
	flipDrawSegments(dest, nSegments, pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, GfxFlip::None, tintMode);
}

//____ flipDrawSegments() ______________________________________________________

void GfxDeviceGen2::flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode)
{
	if (!m_pActiveCanvas || !m_pBackend)
	{
		//TODO: Error handling!

		return;
	}


	auto pFactory = m_pBackend->edgemapFactory();

	auto pEdgemap = pFactory->createEdgemap(WGBP(Edgemap,
		_.size = dest.size() / 64,
		_.segments = nSegments,
		_.colors = pSegmentColors
	));

	pEdgemap->importSamples(SampleOrigo::Top, pEdgeStrips, 0, nSegments-1, 0, nEdgeStrips, 1, edgeStripPitch );

	flipDrawEdgemap(dest.pos(), pEdgemap, flip);
}

//____ drawElipse() ______________________________________________________

void GfxDeviceGen2::drawElipse(const RectSPX& canvas, spx thickness, HiColor fillColor, spx outlineThickness, HiColor outlineColor)
{
	auto pCurveTab = GfxBase::curveTab();
	int curveTabSize = GfxBase::curveTabSize();


	// Center and corners in 26.6 format.

	int x1 = (int)canvas.x;
	int y1 = (int)canvas.y;
	int x2 = (int)(canvas.x + canvas.w);
	int y2 = (int)(canvas.y + canvas.h);

	CoordI center = { (x1 + x2) / 2, (y1 + y2) / 2 };

	// Outer rect of elipse rounded to full pixels.

	RectI outerRect;
	outerRect.x = x1 >> 6;
	outerRect.y = y1 >> 6;
	outerRect.w = ((x2 + 63) >> 6) - outerRect.x;
	outerRect.h = ((y2 + 63) >> 6) - outerRect.y;

	// Adjusted clip

	RectI clip = RectI::overlap(m_pActiveClipList->bounds / 64, outerRect);
	if (clip.w == 0 || clip.h == 0)
		return;

	int clipLeft = clip.x - outerRect.x;

	// Calculate maximum width and height from center for the 4 edges of the elipse.

	int radiusY[4];
	radiusY[0] = (y2 - y1) / 2;
	radiusY[1] = (int)(radiusY[0] - outlineThickness);
	radiusY[2] = (int)(radiusY[1] - thickness);
	radiusY[3] = (int)(radiusY[2] - outlineThickness);

	int radiusX[4];
	radiusX[0] = (x2 - x1) / 2;
	radiusX[1] = (int)(radiusX[0] - outlineThickness);
	radiusX[2] = (int)(radiusX[1] - thickness);
	radiusX[3] = (int)(radiusX[2] - outlineThickness);

	// Reserve buffer for our line traces

	int samplePoints = clip.w + 1;

	int bufferSize = samplePoints * sizeof(int) * 4 * 2;		// length+1 * sizeof(int) * 4 separate traces * 2 halves.
	int* pBuffer = (int*)GfxBase::memStackAlloc(bufferSize);

	// Do line traces.

	int yMid = (center.y & 0xFFFFFFC0) - outerRect.y * 64;
	int yAdjust = center.y & 0x3F;						// Compensate for center not being on pixel boundary.
	int centerOfs = center.x - (outerRect.x << 6);
	int samplePitch = 4;

	for (int edge = 0; edge < 4; edge++)
	{
		int* pOutUpper = pBuffer + edge;
		int* pOutLower = pBuffer + 3 - edge + samplePoints * 4;

		if (radiusX[edge] <= 0 || radiusY[edge] <= 0)
		{
			for (int sample = 0; sample < samplePoints; sample++)
			{
				pOutUpper[sample * samplePitch] = yMid;
				pOutLower[sample * samplePitch] = yMid;
			}
		}
		else
		{
			//TODO: We need to handle subpixels better here.
			// We should increase curvePos with the real xStart offset (could be as much as 63/64 of a pixel)
			// and handle xMid similarly.

			int xStart = (centerOfs - radiusX[edge] + 63) >> 6;		// First pixel-edge inside curve.
			int xMid = (centerOfs) >> 6;							// Pixel edge on or right before center.
			int xEnd = (centerOfs + radiusX[edge]) >> 6;			// Last pixel-edge inside curve.


			int curveInc = int(int64_t((65536 * 64) - 1) * (curveTabSize - 1) / radiusX[edge]); // Keep as many decimals as possible, this is important!
			int curvePos = int((((radiusX[edge] - centerOfs) & 0x3F) * ((int64_t)curveInc)) >> 6);

			if (clipLeft > 0)
			{
				xStart -= clipLeft;
				xMid -= clipLeft;
				xEnd -= clipLeft;

				if (xStart < 0)
					curvePos += (-xStart) * curveInc;
			}

			// Clip xStart, xMid and xEnd

			if (xEnd >= samplePoints)
			{
				xEnd = samplePoints - 1;

				xStart = min(xStart, xEnd);
				xMid = min(xMid, xEnd);
			}

			//

			int sample = 0;
			while (sample < xStart)
			{
				pOutUpper[sample * samplePitch] = yMid;
				pOutLower[sample * samplePitch] = yMid;
				sample++;
			}

			while (sample <= xMid)
			{
				int i = curvePos >> 16;
				uint32_t f = curvePos & 0xFFFF;

				uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
				int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16);  // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

				pOutUpper[sample * samplePitch] = yMid + yAdjust - height;
				pOutLower[sample * samplePitch] = yMid + yAdjust + height;
				sample++;
				curvePos += curveInc;
			}

			curvePos = (curveTabSize - 1) * 65536 * 2 - curvePos;

			while (sample <= xEnd)
			{
				int i = curvePos >> 16;
				uint32_t f = curvePos & 0xFFFF;

				uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
				int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16); // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

				pOutUpper[sample * samplePitch] = yMid + yAdjust - height;
				pOutLower[sample * samplePitch] = yMid + yAdjust + height;
				sample++;
				curvePos -= curveInc;
			}

			while (sample < samplePoints)
			{
				pOutUpper[sample * samplePitch] = yMid;
				pOutLower[sample * samplePitch] = yMid;
				sample++;
			}

			// Take care of left and right edges that needs more calculations to get the angle right.

			int pixFracLeft = (xStart << 6) - (centerOfs - radiusX[edge]);
			int pixFracRight = (centerOfs + radiusX[edge]) & 0x3F;

			if (pixFracLeft > 0 && xStart > 0 && xStart <= samplePoints)
			{
				pOutUpper[(xStart - 1) * samplePitch] = pOutUpper[xStart * samplePitch] + (yMid + yAdjust - pOutUpper[xStart * samplePitch]) * 64 / pixFracLeft;
				pOutLower[(xStart - 1) * samplePitch] = pOutLower[xStart * samplePitch] + (yMid + yAdjust - pOutLower[xStart * samplePitch]) * 64 / pixFracLeft;

			}
			if (pixFracRight > 0 && xEnd < samplePoints - 1 && xEnd >= -1)
			{
				pOutUpper[(xEnd + 1) * samplePitch] = pOutUpper[xEnd * samplePitch] + (yMid + yAdjust - pOutUpper[xEnd * samplePitch]) * 64 / pixFracRight;
				pOutLower[(xEnd + 1) * samplePitch] = pOutLower[xEnd * samplePitch] + (yMid + yAdjust - pOutLower[xEnd * samplePitch]) * 64 / pixFracRight;
			}

		}
	}

	// Split clip rectangles into upper and lower half, so we clip at the middle.

	int split = min(clip.y + clip.h, outerRect.y + (yMid >> 6)) * 64;

	int nClipRects = m_pActiveClipList->nRects;
	auto pClipRects = m_pActiveClipList->pRects;

	int clipBufferSize = sizeof(RectSPX) * nClipRects * 2;
	RectSPX* pTopClips = (RectSPX*)GfxBase::memStackAlloc(clipBufferSize);
	RectSPX* pBottomClips = pTopClips + nClipRects;
	int nTopClips = 0;
	int nBottomClips = 0;

	for (int i = 0; i < nClipRects; i++)
	{
		const RectSPX& clip = pClipRects[i];

		if (clip.y < split)
		{
			pTopClips[nTopClips] = clip;
			if (clip.bottom() > split)
				pTopClips[nTopClips].h = split - clip.y;
			nTopClips++;
		}
		if (clip.bottom() > split)
		{
			pBottomClips[nBottomClips] = clip;
			if (clip.y < split)
			{
				pBottomClips[nBottomClips].h -= split - clip.y;
				pBottomClips[nBottomClips].y = split;
			}
			nBottomClips++;
		}
	}

	// Render columns

	HiColor	col[5];
	col[0] = HiColor::Transparent;
	col[1] = outlineColor;
	col[2] = fillColor;
	col[3] = outlineColor;
	col[4] = HiColor::Transparent;

	const RectI* pOldClipRects = pClipRects;
	int nOldClipRects = nClipRects;

	setClipList(nTopClips, pTopClips);
	drawSegments(RectSPX(clip.x, outerRect.y, clip.w, outerRect.h) * 64, 5, col, samplePoints, pBuffer, 4);
	setClipList(nBottomClips, pBottomClips);
	drawSegments(RectSPX(clip.x, outerRect.y, clip.w, outerRect.h) * 64, 5, col, samplePoints, pBuffer + samplePoints * 4, 4);
	setClipList(nOldClipRects, pOldClipRects);

	// Free temporary work memory

	GfxBase::memStackFree(clipBufferSize);
	GfxBase::memStackFree(bufferSize);

}

//____ drawPieChart() _____________________________________________________

void GfxDeviceGen2::drawPieChart(const RectSPX& _canvas, float start, int nSlices, const float* _pSliceSizes, const HiColor* pSliceColors, float hubSize, HiColor hubColor, HiColor backColor, bool bRectangular)
{
	static const int c_maxSlices = c_maxSegments - 2;

	if (nSlices < 0 || nSlices > c_maxSlices)
	{
		//TODO: Error handling;
		return;
	}

	//TODO: Early out if no slices, no hud and no background
	//TODO: Replace with fill if no hub, no slices.

	RectI canvas = _canvas / 64;
	canvas.w = (canvas.w + 1) & 0xFFFFFFFC;
	canvas.h = (canvas.h + 1) & 0xFFFFFFFC;

	if (canvas.w <= 0 || canvas.h <= 0 || (hubSize == 1.f && !bRectangular))
		return;


	// Setup our slices

	struct Slice
	{
		float ofs;
		float size;
		HiColor color;
	};

	Slice slices[c_maxSlices + 2];			// Maximum two extra slices in the end. Beginning offset + end transparency.

	float totalSize = 0.f;

	// Trim our slices, so we have a length of most 1.0.

	Slice trimmedSlices[c_maxSlices + 1];

	float ofs = start;
	for (int i = 0; i < nSlices; i++)
	{
		if (ofs >= 1.f)
			ofs = fmod(ofs, 1.f);

		float sliceSize = _pSliceSizes[i];

		if (totalSize + sliceSize >= 1.f)
		{
			trimmedSlices[i] = { ofs, 1.f - totalSize, pSliceColors[i] };
			totalSize = 1.f;
			nSlices = i + 1;
			break;
		}


		trimmedSlices[i] = { ofs,sliceSize, pSliceColors[i] };
		totalSize += sliceSize;
		ofs += sliceSize;
	}

	// Adding an empty slice at end if totalSize < 1.0

	if (totalSize < 0.9999f)
	{
		if (ofs >= 1.f)
			ofs = fmod(ofs, 1.f);

		trimmedSlices[nSlices++] = { ofs,1.f - totalSize, backColor };
	}

	// Find first slice (one with smallest offset)

	int		firstSlice = 0, lastSlice;
	float	firstSliceOfs = 1.f;

	for (int i = 0; i < nSlices; i++)
	{
		if (trimmedSlices[i].ofs < firstSliceOfs)
		{
			firstSlice = i;
			firstSliceOfs = trimmedSlices[i].ofs;
		}

	}

	// Find last slice

	if (firstSlice == 0)
		lastSlice = nSlices - 1;
	else
		lastSlice = firstSlice - 1;

	// Take care of possible rounding errors on inparameters

	if (firstSliceOfs < 0.0001f)
		trimmedSlices[firstSlice].ofs = firstSliceOfs = 0.f;

	// Rearrange our slices so we start from offset 0. Possibly adding one more slice in the beginning.

	int sliceIdx = 0;

	if (nSlices == 1)
	{
		slices[sliceIdx++] = { 0.f, 1.f, trimmedSlices[0].color };
	}
	else
	{
		// Fill in rollover from last slice (or transparent gap) due to rotation

		if (firstSliceOfs > 0.f)
			slices[sliceIdx++] = { 0.f, firstSliceOfs, trimmedSlices[lastSlice].color };

		// Our buffer is circular, take care of slices from first slice to end of buffer.

		for (int i = firstSlice; i < nSlices; i++)
			slices[sliceIdx++] = trimmedSlices[i];

		// Take care of slices from beginning of buffer to last slice.

		if (lastSlice < firstSlice)
		{
			for (int i = 0; i <= lastSlice; i++)
				slices[sliceIdx++] = trimmedSlices[i];
		}

		// Correct for any rollover or inprecision for last slice

		slices[sliceIdx - 1].size = 1.f - slices[sliceIdx - 1].ofs;
	}

	nSlices = sliceIdx;			// Repurpose this variable

	// Slices now in order, lets render the quadrants

	int quadW = canvas.w / 2, quadH = canvas.h / 2;

	RectI quadCanvas[4] = { {canvas.x + quadW, canvas.y, quadW, quadH },
							{canvas.x + quadW, canvas.y + quadH, quadW, quadH},
							{canvas.x, canvas.y + quadH, quadW, quadH},
							{canvas.x, canvas.y, quadW, quadH} };

	GfxFlip quadFlip[4] = { GfxFlip::None, GfxFlip::Rot90, GfxFlip::Rot180, GfxFlip::Rot270 };

	HiColor colors[c_maxSegments];

	int maxSegments = nSlices + 2;
	int edgePitch = maxSegments - 1;
	int bufferSize = (quadW + 1) * edgePitch * sizeof(int);

	int* pBuffer = (int*)GfxBase::memStackAlloc(bufferSize);


	auto pCurveTab = GfxBase::curveTab();
	int curveTabSize = GfxBase::curveTabSize();

	// Setting the outer edge (same for all quads)

	if (bRectangular)
	{
		int* pEdge = pBuffer;
		for (int i = 0; i <= quadW; i++)
		{
			*pEdge = 0;
			pEdge += edgePitch;
		}
	}
	else
	{
		int* pEdge = pBuffer;

		int curveTabInc = ((curveTabSize << 16) - 1) / (quadW);

		int curveTabOfs = 0;
		for (int i = 0; i <= quadW; i++)
		{
			*pEdge = (quadH << 6) - ((pCurveTab[(curveTabSize - 1) - (curveTabOfs >> 16)] * quadH) >> 10);
			pEdge += edgePitch;
			curveTabOfs += curveTabInc;
		}
	}

	// Generating an inner edge if we have one.
	// Storing it separately, used for all 4 quads. Copied into right place later.

	int* pHubBuffer = nullptr;
	int hubBufferSize = 0;
	if (hubSize > 0.f)
	{
		hubBufferSize = (quadW + 1) * sizeof(int);
		pHubBuffer = (int*)GfxBase::memStackAlloc(hubBufferSize);

		int* p = pHubBuffer;

		int ringW = int(hubSize * quadW);
		int ringH = int(hubSize * quadH);

		int inc = ((curveTabSize << 16) - 1) / (ringW);
		int ofs = 0;

		for (int i = 0; i <= ringW; i++)
		{
			*p++ = (quadH << 6) - ((pCurveTab[(curveTabSize - 1) - (ofs >> 16)] * ringH) >> 10);
			ofs += inc;
		}

		int maxVal = quadH << 6;
		for (int i = ringW + 1; i <= quadW; i++)
			*p++ = maxVal;
	}

	//

	for (int quad = 0; quad < 4; quad++)
	{
		int nSegments = 0;
		Slice* pSlice = slices;
		Slice* pSliceEnd = slices + nSlices;

		float quadStartOfs = 0.25f * quad;

		// Add background as first segment

		colors[nSegments] = backColor;
		nSegments++;

		// Find first slice to include

		while (pSlice != pSliceEnd && pSlice->ofs + pSlice->size < quadStartOfs)
			pSlice++;

		colors[nSegments] = pSlice->color;
		nSegments++;
		pSlice++;

		// Generate edges for all following slices included

		while (pSlice != pSliceEnd && pSlice->ofs < quadStartOfs + 0.25f)
		{
			int* pEdge = pBuffer + nSegments - 1;

			// Set startvalue and decrease per strip.

			int value = quadH << 6;
			int dec;

			float rot = (pSlice->ofs - quadStartOfs);

			if (rot == 0.f)
			{
				value = 0;
				dec = 0;
			}
			else
			{
				rot *= 3.14159265358979f * 2;
				float s = sin(rot);
				float c = cos(rot);

				float decF = (c / s) * (quadH << 12) / float(quadW);

				if (decF > 400000 * 4096)
					decF = 400000 * 4096;

				dec = int(decF);
			}

			// Fill in the edge

			int strip;
			int precisionValue = value << 6;
			for (strip = 0; strip <= quadW && value >= pEdge[-1]; strip++)
			{
				*pEdge = value;
				pEdge += edgePitch;
				precisionValue -= dec;
				value = precisionValue >> 6;
			}

			while (strip <= quadW)
			{
				*pEdge = pEdge[-1];
				pEdge += edgePitch;
				strip++;
			}

			colors[nSegments] = pSlice->color;
			nSegments++;
			pSlice++;
		}

		// Add edge for hub if present

		if (pHubBuffer)
		{
			int* pEdge = pBuffer + nSegments - 1;

			for (int i = 0; i <= quadW; i++)
			{
				int value = pHubBuffer[i];
				*pEdge = value;

				int* pPrev = pEdge - 1;
				while (*pPrev > value)
				{
					*pPrev = value;
					pPrev--;
				}

				pEdge += edgePitch;
			}

			colors[nSegments] = hubColor;
			nSegments++;
		}

		// Draw

		int* pEdges = pBuffer;
		HiColor* pColors = colors;
		if (bRectangular)
		{
			pEdges++;
			pColors++;
			nSegments--;
		}

		int16_t		alphaCheck = 0;
		for (int i = 0; i < nSegments; i++)
			alphaCheck |= pColors[i].a;

		if (alphaCheck == 0)
			continue;

		if (nSegments == 1)
			fill(quadCanvas[quad], pColors[0]);
		else
			flipDrawSegments(quadCanvas[quad] * 64, nSegments, pColors, quadW + 1, pEdges, edgePitch, quadFlip[quad], TintMode::Flat );
	}

	if (hubBufferSize != 0)
		GfxBase::memStackFree(hubBufferSize);

	GfxBase::memStackFree(bufferSize);
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
	auto& rects = m_pActiveLayer->rects;

	//TODO: Proper 26:6 support
	RectSPX dest = align(_dest);

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX rect = RectSPX::overlap(m_pActiveClipList->pRects[i], dest);

		if( !rect.isEmpty() )
		{
			rects.emplace_back(rect);
			nRects++;
		}
	}
	
	if( nRects > 0 )
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		auto& commands = m_pActiveLayer->commands;

		if( cmd == m_pActiveLayer->latestCommand && commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() + nRects < 65536 )
		{
			commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() += nRects;
			commands.secureSpace(20*nRects);
		}
		else
		{
			m_pActiveLayer->latestCommand = cmd;
			m_pActiveLayer->latestCommandOfs = (int) m_pActiveLayer->commands.size();

			commands.secureSpace(4+20*nRects);

			commands.pushUnchecked(uint16_t(cmd));
			commands.pushUnchecked(uint16_t(nRects));
		}

		int srcX = align(src.x)*16;		// Higher resolution on source than SPX: 22:10.
		int srcY = align(src.y)*16;		// Higher resolution on source than SPX: 22:10.

		for( int i = 0 ; i < nRects ; i++ )
		{
			commands.pushUnchecked(srcX);				// Higher resolution on source than SPX: 22:10.
			commands.pushUnchecked(srcY);				// Higher resolution on source than SPX: 22:10.
			commands.pushUnchecked(dest.pos());

			commands.pushUnchecked(uint16_t(transformOfs));
			commands.pushUnchecked(uint16_t(0));								// Padding
		}

		if (cmd == Command::Blur)
			m_pActiveCanvas->sessionInfo.nBlur += nRects;
		else
			m_pActiveCanvas->sessionInfo.nBlit += nRects;
	}
}

//____ _transformBlitComplex() _______________________________________________

void GfxDeviceGen2::_transformBlitComplex(const RectSPX& _dest, CoordI src, const Transform& matrix, Command cmd)
{
	// Clip and render the patches

	if (!_dest.isOverlapping(m_pActiveClipList->bounds))
		return;

	//TODO: Proper 26:6 support
	RectSPX dest = align(_dest);

	const RectI& clip = dest;

	int nRects = 0;
	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX rect = RectSPX::overlap(m_pActiveClipList->pRects[i], dest);

		if (!rect.isEmpty())
		{
			rects.emplace_back(rect);
			nRects++;
		}
	}

	if (nRects > 0)
	{
		if (m_stateChanges != 0)
			_encodeStateChanges();

		auto& commands = m_pActiveLayer->commands;

		if( cmd == m_pActiveLayer->latestCommand && commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() + nRects < 65536 )
		{
			commands.at(m_pActiveLayer->latestCommandOfs+2).as<uint16_t>() += nRects;

			commands.secureSpace(20*nRects);
		}
		else
		{
			m_pActiveLayer->latestCommand = cmd;
			m_pActiveLayer->latestCommandOfs = (int) m_pActiveLayer->commands.size();

			commands.secureSpace(4+20*nRects);

			commands.pushUnchecked(uint16_t(cmd));
			commands.pushUnchecked(uint16_t(nRects));
		}


		int transformOfs = int(m_pActiveCanvas->transforms.size()) + GfxFlip_size;
		m_pActiveCanvas->transforms.emplace_back(matrix);

		for( int i = 0 ; i < nRects ; i++ )
		{
			commands.pushUnchecked(src);				// Higher resolution on source than SPX: 22:10.
			commands.pushUnchecked(dest.pos());

			commands.pushUnchecked(uint16_t(transformOfs));
			commands.pushUnchecked(uint16_t(0));								// Padding
		}

		if (cmd == Command::Blur)
			m_pActiveCanvas->sessionInfo.nBlur += nRects;
		else
			m_pActiveCanvas->sessionInfo.nBlit += nRects;
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
	// If this is the first state-change in a layer above baseLayer we need to
	// force set some states to their default values. They might have been set
	// in previous layer.

	bool bForceSetStates = false;

//	if(m_pActiveCanvas->activeLayer > 0 && m_pActiveLayer->commands.size() == 0 )
//	{
//		bForceSetStates = true;
//		m_stateChanges |= ( uint8_t(StateChange::TintColor) /*| uint8_t(StateChange::BlendMode) | uint8_t(StateChange::MorphFactor) | uint8_t(StateChange::FixedBlendColor)*/ );
//	}

	//

	RenderState& newState = m_renderState;
	RenderState& encodedState = m_pActiveLayer->encodedState;

	//

	auto& cmdBuffer = m_pActiveLayer->commands;
	auto& colorBuffer = m_pActiveLayer->colors;

	int commandOfs = (int) cmdBuffer.size();

	cmdBuffer.push(uint16_t(Command::StateChange));
	cmdBuffer.push(uint16_t(0));										// States changed to be filled in later.

	uint8_t	statesChanged = 0;

	if (m_stateChanges & uint8_t(StateChange::BlitSource))
	{

		Surface* pSource = newState.blitSource.rawPtr();
		Surface* pOldSource = encodedState.blitSource.rawPtr();

		if (pSource != pOldSource)
		{
			m_pActiveLayer->objects.push_back(pSource);
			if( pSource )
				pSource->retain();

			encodedState.blitSource = pSource;
			statesChanged |= int(StateChange::BlitSource);
		}
	}

	if ((m_stateChanges & uint8_t(StateChange::TintColor)) || (m_stateChanges & uint8_t(StateChange::TintMap)))
	{
		// Backend uses either TintColor or Tintmap while frontend can combine both.
		// If frontend uses both, one of the colorstrips needs to be tinted with TintColor.

		if (bForceSetStates ||
			(newState.tintColor != encodedState.tintColor) ||
			(newState.pTintmap != encodedState.pTintmap) ||
			(newState.tintmapRect != encodedState.tintmapRect) )
		{
			if( newState.pTintmap )
			{
				assert(!bForceSetStates);

				Tintmap* pTintmap = newState.pTintmap.rawPtr();

				assert(pTintmap);

				int tintmapStartOfs = (int) colorBuffer.size();
				int nHorrColors = 0;
				int nVertColors = 0;

				if (pTintmap->isHorizontal())
				{
					nHorrColors = newState.tintmapRect.w/64;
					int ofs = tintmapStartOfs;

					colorBuffer.resize(ofs + nHorrColors );
					pTintmap->exportHorizontalColors(newState.tintmapRect.w, &colorBuffer[ofs]);
				}

				if (pTintmap->isVertical())
				{
					nVertColors = newState.tintmapRect.h/64;
					int ofs = tintmapStartOfs + nHorrColors;

					colorBuffer.resize(ofs + nVertColors );
					pTintmap->exportVerticalColors(newState.tintmapRect.h, &colorBuffer[ofs]);
				}

				if( newState.tintColor != HiColor::Undefined )
				{
					// We need to tint either the horizontal or vertical colors

					HiColor * pBeg = &colorBuffer[tintmapStartOfs];
					HiColor * pEnd = nHorrColors > 0 ? pBeg + nHorrColors : pBeg + nVertColors;

					if( nVertColors > 0 && nVertColors < nHorrColors )
					{
						pBeg = pEnd;
						pEnd += nVertColors;
					}

					for( auto p = pBeg ; p < pEnd ; p++ )
						*p *= newState.tintColor;
				}

				cmdBuffer.secureSpace(sizeof(newState.tintmapRect) + 2*4);

				cmdBuffer.pushUnchecked(newState.tintmapRect);
				cmdBuffer.pushUnchecked(nHorrColors);
				cmdBuffer .pushUnchecked(nVertColors);

				encodedState.pTintmap = newState.pTintmap;
				encodedState.tintmapRect = newState.tintmapRect;

				statesChanged |= int(StateChange::TintMap);
			}
			else
			{
				auto tintColor = newState.tintColor.isUndefined() ? HiColor::White : newState.tintColor;
				colorBuffer.push_back(tintColor);
				encodedState.tintColor = tintColor;

				statesChanged |= int(StateChange::TintColor);
			}
		}
	}

	if (m_stateChanges & uint8_t(StateChange::BlendMode))
	{
//		if (bForceSetStates || newState.blendMode != encodedState.blendMode)
		if (newState.blendMode != encodedState.blendMode)
		{
			cmdBuffer.push(uint16_t(newState.blendMode));
			encodedState.blendMode = newState.blendMode;
			statesChanged |= int(StateChange::BlendMode);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::MorphFactor))
	{
//		if (bForceSetStates || newState.morphFactor != encodedState.morphFactor)
		if (newState.morphFactor != encodedState.morphFactor)
		{
			cmdBuffer.push(uint16_t(newState.morphFactor * 4096));
			encodedState.morphFactor = newState.morphFactor;
			statesChanged |= int(StateChange::MorphFactor);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::FixedBlendColor))
	{
//		if (bForceSetStates || newState.fixedBlendColor != encodedState.fixedBlendColor)
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
			auto pBlurbrush = newState.pBlurbrush.rawPtr();

			cmdBuffer.push(uint16_t(pBlurbrush->size()));

			const float* pRed		= pBlurbrush->red();
			const float* pGreen		= pBlurbrush->green();
			const float* pBlue		= pBlurbrush->blue();

			cmdBuffer.secureSpace(9*3*2);

			for (int i = 0; i < 9; i++)
				cmdBuffer.pushUnchecked(uint16_t(pRed[i]*32768));

			for (int i = 0; i < 9; i++)
				cmdBuffer.pushUnchecked(uint16_t(pGreen[i]*32768));

			for (int i = 0; i < 9; i++)
				cmdBuffer.pushUnchecked(uint16_t(pBlue[i]*32768));

			encodedState.pBlurbrush = newState.pBlurbrush;
			statesChanged |= int(StateChange::Blur);
		}
	}

	//

	if (statesChanged == 0)
		cmdBuffer.resize(commandOfs);
	else
	{
		cmdBuffer.at(commandOfs + 2).as<uint16_t>() = statesChanged;

		cmdBuffer.align(4);

		m_pActiveCanvas->sessionInfo.nStateChanges++;

		m_pActiveLayer->latestCommand = Command::StateChange;
	}

	m_stateChanges = 0;

}

} // namspace wg
