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
#include <algorithm>
#include <new>
#include <cmath>
#include <cstring>

#include <wg_gfxdevice_gen2.h>
#include <wg_tinttools.h>
#include <wg_geo.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <wg_edgemapwave.h>

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

//____ setBackend() ___________________________________________________________

bool GfxDeviceGen2::setBackend( GfxBackend * pBackend )
{
	if( m_bRendering )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Cannot change backend while rendering.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
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

//____ warpmapFactory() _______________________________________________________

WarpmapFactory_p GfxDeviceGen2::warpmapFactory()
{
	return m_pBackend ? m_pBackend->warpmapFactory() : nullptr;
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

	// Can not add more lines to existing command if clip list changed.

	for( auto& layer : m_pActiveCanvas->layers )
	{
		if( layer.latestCommand == Command::Line )
			layer.latestCommand = Command::None;
	}

	return true;
}

//____ resetClipList() ____________________________________________________

void GfxDeviceGen2::resetClipList()
{
	if (m_pActiveClipList)
		* m_pActiveClipList = m_pActiveCanvas->updateRects;

	// Can not add more lines to existing command if clip list changed.

	for( auto& layer : m_pActiveCanvas->layers )
	{
		if( layer.latestCommand == Command::Line )
			layer.latestCommand = Command::None;
	}
}

//____ pushClipList() _____________________________________________________

bool GfxDeviceGen2::pushClipList(int nRectangles, const RectSPX* pRectangles)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No active canvas. Cannot push clip list.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
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

	// Can not add more lines to existing command if clip list changed.

	for( auto& layer : m_pActiveCanvas->layers )
	{
		if( layer.latestCommand == Command::Line )
			layer.latestCommand = Command::None;
	}

	return true;
}

//____ popClipList() ______________________________________________________

bool GfxDeviceGen2::popClipList()
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No active canvas. Cannot pop clip list.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return false;
	}

	if (m_pActiveCanvas->clipListStack.size() == 1)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Clip list stack underflow. Cannot pop clip list.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return false;
	}

	m_pActiveCanvas->clipListStack.pop_back();
	m_pActiveClipList = &m_pActiveCanvas->clipListStack.back();

	// Can not add more lines to existing command if clip list changed.

	for( auto& layer : m_pActiveCanvas->layers )
	{
		if( layer.latestCommand == Command::Line )
			layer.latestCommand = Command::None;
	}

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
	if (color == m_renderState.tintColor)
		return;

	if( !color.isValid() && color != HiColor::Undefined )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid tint color.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
#ifdef NDEBUG
		return;
#else
		color = HiColor::ShowError;
#endif
	}

	m_renderState.tintColor = color;
	m_stateChanges |= int(StateChange::TintColor);
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


//____ setTint() ______________________________________________________________
/**
 * @brief Set a Tint (gradient) that everything drawn is multiplied with.
 *
 * The Tint is placed in given rectangle, e.g. its geometry, which is relative
 * to the rectangle, is mapped onto it. Drawing outside the rectangle is still
 * tinted, following the Tint's spread.
 *
 * The Tint is multiplied with the tint color, if any.
 *
 * @param rect	Rectangle to place the Tint in, in canvas spx.
 * @param pTint	The Tint. Nullptr clears the Tint.
 */

void GfxDeviceGen2::setTint(const RectSPX& rect, Tint* pTint)
{
	if (!pTint)
	{
		clearTint();
		return;
	}

	if (pTint != m_renderState.pTint || rect != m_renderState.tintRect )
	{
		m_renderState.pTint = pTint;
		m_renderState.tintRect = rect;

		m_stateChanges |= int(StateChange::Tint);
	}
}

//____ clearTint() ____________________________________________________________

void GfxDeviceGen2::clearTint()
{
	if (m_renderState.pTint)
		m_stateChanges |= int(StateChange::Tint);

	m_renderState.pTint = nullptr;
	m_renderState.tintRect.clear();
}

//____ hasTint() ______________________________________________________________

bool GfxDeviceGen2::hasTint() const
{
	return m_renderState.pTint != nullptr;
}

//____ tint() _________________________________________________________________

Tint_p GfxDeviceGen2::tint() const
{
	return m_renderState.pTint;
}

//____ tintRect() _____________________________________________________________

RectSPX GfxDeviceGen2::tintRect() const
{
	return m_renderState.tintRect;
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
	if (pSource && !m_pBackend->canBeBlitSource(pSource->typeInfo()))
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Surface can not be used as blit source.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No active canvas. Cannot set render layer.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (layer < 0 || layer >= int(m_pActiveCanvas->layers.size()))
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Specified layer out of bounds.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No active canvas. Cannot get render layer.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	return m_pActiveCanvas->activeLayer;
}

//____ setSubPixelFill() ______________________________________________________

void GfxDeviceGen2::setSubPixelFill(bool bSubPixelFill)
{
	m_bSubPixelFill = bSubPixelFill;
}



//____ beginRender() ______________________________________________________

bool GfxDeviceGen2::beginRender()
{
	if (m_bRendering)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Already in rendering state.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}

	if( !m_pBackend )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No backend assigned.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Not in rendering state.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}

	m_bRendering = false;
	m_pBackend->endRender();

	if (!m_canvasStack.empty())
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Rendering ended with active canvas updates.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Either CanvasRef or Surface must be specified.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return false;
	}

	if (pCanvas && (!m_pBackend->canBeCanvas(pCanvas->typeInfo()) || !pCanvas->canBeCanvas()))
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Surface can not be used as canvas.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return false;
	}

	// Out of bounds start layer falls back to the default rather than failing,
	// since callers don't always check our return value before ending the update.

	if (startLayer != -1 && (startLayer < 0 || startLayer > (pLayers ? pLayers->size() : 0)))
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "Start layer out of bounds, using default instead.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		startLayer = -1;
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
		entry.info.colorSpace = pCanvas->colorSpace();
		entry.info.bigEndian = pCanvas->isBigEndian();
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
				_.colorSpace = info.colorSpace == ColorSpace::Undefined ? entry.info.colorSpace : info.colorSpace,
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

	// Clearing the layers left us on the last of them, go back to the one we start on.

	setRenderLayer(activeLayer);

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No active canvas to end.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		
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
	clearTint();
	clearTintColor();

	for (size_t i = 1; i < canvasEntry.layers.size(); i++)
	{
		auto& info = canvasEntry.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasEntry.layers[i];

		if (layer.pLayerCanvas)
		{
			setRenderLayer((int)i);

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


	for (size_t i = 0; i < canvasEntry.layers.size(); i++)
	{
		auto& layer = canvasEntry.layers[i];

		layer.objects.clear();
		layer.colors.clear();
		layer.rects.clear();
		layer.commands.clear();

		// Forget what the object cache knew. Should the generation wrap around, entries
		// from four billion sessions ago would look valid, so start over properly.

		if (++layer.objectCacheGeneration == 0)
		{
			for (auto& entry : layer.objectCache)
				entry = ObjectCacheEntry();

			layer.objectCacheGeneration = 1;
		}

		layer.latestCommand = Command::None;
		layer.latestCommandOfs = -1;

		_resetState(layer.encodedState);

		layer.finalCommandsOfs = -1;
	}

	// Our render state is kept, so set all state-change flags to have it
	// encoded again into the cleared layers on next command.

	m_stateChanges = 0xFF;
}


//____ _doFlattenLayers() ____________________________________________________

void GfxDeviceGen2::_doFlattenLayers()
{
	auto& canvasData = m_canvasStack.back();

	// Finalize additional layers

	for (size_t i = 1; i < canvasData.layers.size(); i++)
	{
		auto& info = canvasData.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasData.layers[i];

		if (layer.pLayerCanvas)
		{
			setRenderLayer((int)i);

			// Encode preBlend call if any.

			if (info.preBlendFunc)
			{
				setBlendMode(BlendMode::Blend);
				clearTint();
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
				clearTint();
				clearTintColor();
				info.preBlendCanvasFunc(this);
			}

			// Encode blendFunc if present, otherwise encode default blit.

			setBlendMode(BlendMode::Blend);
			clearTint();
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
				setRenderLayer((int)i);		// Write our commands to buffer to be processed
				setBlendMode(BlendMode::Blend);
				clearTint();
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
		clearTint();
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

		if( !baseLayer.rects.empty() )
		{
			RectSPX* pRectsBeg = baseLayer.rects.data();
			RectSPX* pRectsEnd = pRectsBeg + baseLayer.rects.size();
			m_pBackend->setRects(pRectsBeg, pRectsEnd);
		}

		if( !baseLayer.rects.empty() )
		{
			HiColor* pColorsBeg = baseLayer.colors.data();
			HiColor* pColorsEnd = pColorsBeg + baseLayer.colors.size();
			m_pBackend->setColors(pColorsBeg, pColorsEnd);
		}

		uint8_t* pCommandsBeg = baseLayer.commands.data();
		uint8_t* pCommandsEnd = pCommandsBeg + baseLayer.commands.size();
		m_pBackend->processCommands( (uint16_t*) pCommandsBeg, (uint16_t*) pCommandsEnd);
	}


	// Render additional layers

	for (size_t i = 1; i < canvasData.layers.size(); i++)
	{
		auto& info = canvasData.pLayerInfo->m_layers[i - 1];
		auto& layer = canvasData.layers[i];

		if( layer.commands.empty() )
			continue;

		// Check if we can do direct rendering

		if (layer.pLayerCanvas )
		{
			setRenderLayer((int)i);

			// Set layerCanvas as output destination

			m_pBackend->setCanvas(layer.pLayerCanvas);

			// Send buffers and layer-specific commands to backend

			if( !layer.objects.empty() )
				m_pBackend->setObjects( layer.objects.data(), layer.objects.data() + layer.objects.size() );

			if( !layer.rects.empty() )
			{
				RectSPX* pRectsBeg = layer.rects.data();
				RectSPX* pRectsEnd = pRectsBeg + layer.rects.size();
				m_pBackend->setRects(pRectsBeg, pRectsEnd);
			}

			if( !layer.colors.empty() )
			{
				HiColor* pColorsBeg = layer.colors.data();
				HiColor* pColorsEnd = pColorsBeg + layer.colors.size();
				m_pBackend->setColors(pColorsBeg, pColorsEnd);
			}

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

			if( !layer.objects.empty() )
				m_pBackend->setObjects( layer.objects.data(), layer.objects.data() + layer.objects.size() );

			if( !layer.rects.empty() )
			{
				RectSPX* pRectsBeg = layer.rects.data();
				RectSPX* pRectsEnd = pRectsBeg + layer.rects.size();
				m_pBackend->setRects(pRectsBeg, pRectsEnd);
			}

			if( !layer.colors.empty() )
			{
				HiColor* pColorsBeg = layer.colors.data();
				HiColor* pColorsEnd = pColorsBeg + layer.colors.size();
				m_pBackend->setColors(pColorsBeg, pColorsEnd);
			}

			uint8_t* pCommandsBeg = layer.commands.data();
			uint8_t* pCommandsEnd = pCommandsBeg + layer.commands.size();
			m_pBackend->processCommands( (uint16_t*) pCommandsBeg, (uint16_t*) pCommandsEnd);
		}
	}
	
	// End backend session

	m_pBackend->endSession();

	// Notify updated surface

	Surface * pUpdatedSurf = canvasData.info.pSurface;
	if( !pUpdatedSurf )
		pUpdatedSurf = m_pBackend->canvasInfo(canvasData.info.ref)->pSurface;

	if( pUpdatedSurf && pUpdatedSurf->hasObservers() )
	{
		m_pBackend->waitForCompletion();

		int nRects = canvasData.updateRects.nRects;
		auto pSpxRects = canvasData.updateRects.pRects;

		int bufferSize = nRects * sizeof(RectI);
		auto pPixelRects = (RectI*) GfxBase::memStackAlloc( bufferSize );

		for( int i = 0 ; i < nRects ; i++ )
			pPixelRects[i] = pSpxRects[i]/64;

		pUpdatedSurf->notifyObservers(nRects, pPixelRects);

		GfxBase::memStackFree( bufferSize );
	}

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to fill.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to fill.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

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

	RectSPX fillRect;
	if( m_bSubPixelFill )
		fillRect = rect;
	else
	{
		fillRect.x = rect.x & 0xFFFFFFC0;
		fillRect.y = rect.y & 0xFFFFFFC0;
		fillRect.w = ((rect.x + rect.w) & 0xFFFFFFC0) - fillRect.x;
		fillRect.h = ((rect.y + rect.h) & 0xFFFFFFC0) - fillRect.y;
	}

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], fillRect);

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

void GfxDeviceGen2::drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness, LineCaps caps)
{
	if (dir == Direction::Undefined)
		return;

	if (!caps.isNone())
	{
		CoordSPX end = begin;

		switch (dir)
		{
		case Direction::Up:		end.y -= length; break;
		case Direction::Down:	end.y += length; break;
		case Direction::Left:	end.x -= length; break;
		case Direction::Right:	end.x += length; break;
		case Direction::Undefined: break;
		}

		drawLine(begin, end, color, thickness, caps);
		return;
	}

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
		rect.y = begin.y;
		rect.w = thickness;
		rect.h = length;
		break;
	case Direction::Left:
		rect.x = begin.x - length;
		rect.y = begin.y - thickness / 2;
		rect.w = length;
		rect.h = thickness;
		break;
	case Direction::Right:
		rect.x = begin.x;
		rect.y = begin.y - thickness / 2;
		rect.w = length;
		rect.h = thickness;
		break;
	case Direction::Undefined:
		break;
	}

	fill(rect, color);

}

//____ drawLine() (from/to) __________________________________________

void GfxDeviceGen2::drawLine(CoordSPX beg, CoordSPX end, HiColor color, spx thickness, LineCaps caps)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to draw line on.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	// Skip calls that won't affect destination

	if (color.a == 0 && (m_renderState.blendMode == BlendMode::Blend))
		return;

	if (m_stateChanges != 0)
		_encodeStateChanges();

	auto& commands = m_pActiveLayer->commands;

	// Lines are added to the latest line command, unless it is full or with or without caps when
	// this line isn't, as backends can draw them in separate batches. The last word of the latest
	// command is the caps of its last line.

	uint16_t capsWord = GfxBackend::encodeLineCaps(caps);

	if( m_pActiveLayer->latestCommand == Command::Line && commands.at(m_pActiveLayer->latestCommandOfs+4).as<uint16_t>() < 65535 &&
		(commands.at(commands.size() - 2).as<uint16_t>() == 0) == (capsWord == 0) )
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
	commands.pushUnchecked(capsWord);

	m_pActiveLayer->colors.push_back(color);

	m_pActiveCanvas->sessionInfo.nLines++;
	m_pActiveCanvas->sessionInfo.nLineCoords+=2;
}

//____ _isShapeColorDrawn() __________________________________________________
// Undefined colors are not drawn. Like fill(), transparent colors draw nothing in Blend mode.

bool GfxDeviceGen2::_isShapeColorDrawn(HiColor color) const
{
	return !color.isUndefined() && !(color.a == 0 && m_renderState.blendMode == BlendMode::Blend);
}

//____ _addShapeClipRects() __________________________________________________
// Adds the clip rectangles within the pixels touched by area, so that they stay
// pixel aligned. Returns their number.

int GfxDeviceGen2::_addShapeClipRects(const RectSPX& area)
{
	RectSPX bounds;
	bounds.x = area.x & ~63;
	bounds.y = area.y & ~63;
	bounds.w = ((area.x + area.w + 63) & ~63) - bounds.x;
	bounds.h = ((area.y + area.h + 63) & ~63) - bounds.y;

	int nRects = 0;
	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], bounds);

		if (!clipped.isEmpty())
		{
			rects.emplace_back(clipped);
			nRects++;
		}
	}

	return nRects;
}

//____ drawRoundedBox() ______________________________________________________

void GfxDeviceGen2::drawRoundedBox(const RoundedBoxSpec& spec)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to draw rounded box to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	const RectSPX& rect = spec.rect;

	if (rect.w <= 0 || rect.h <= 0 || !rect.isOverlapping(m_pActiveClipList->bounds))
		return;

	BorderSPX outline = {	std::max(spec.outline.top, 0), std::max(spec.outline.right, 0),
							std::max(spec.outline.bottom, 0), std::max(spec.outline.left, 0) };

	RectSPX innerRect(rect.x + outline.left, rect.y + outline.top,
					  rect.w - outline.left - outline.right, rect.h - outline.top - outline.bottom);

	bool bInner = innerRect.w > 0 && innerRect.h > 0;

	bool bHasOutline = (outline.top | outline.right | outline.bottom | outline.left) != 0 && _isShapeColorDrawn(spec.outlineColor);
	bool bHasFill = bInner && _isShapeColorDrawn(spec.fillColor);

	if (!bHasFill && !bHasOutline)
		return;

	spx radius[4] = { spec.radius[0], spec.radius[1], spec.radius[2], spec.radius[3] };
	Util::clampCornerRadii(rect.size(), radius);

	// Square, untinted boxes aligned to pixels are just fills.

	if ((radius[0] | radius[1] | radius[2] | radius[3]) == 0 && !spec.pFillTint && !spec.pOutlineTint &&
		((rect.x | rect.y | rect.w | rect.h | outline.top | outline.right | outline.bottom | outline.left) & 63) == 0)
	{
		if (bHasOutline)
		{
			if (!bInner)
				fill(rect, spec.outlineColor);
			else
			{
				RectSPX parts[4] = {	{ rect.x, rect.y, rect.w, outline.top },
										{ rect.x, innerRect.y, outline.left, innerRect.h },
										{ innerRect.x + innerRect.w, innerRect.y, outline.right, innerRect.h },
										{ rect.x, innerRect.y + innerRect.h, rect.w, outline.bottom } };

				for (auto& part : parts)
					if (part.w > 0 && part.h > 0)
						fill(part, spec.outlineColor);
			}
		}

		if (bHasFill)
			fill(innerRect, spec.fillColor);

		return;
	}

	int nRects = _addShapeClipRects(rect);

	if (nRects == 0)
		return;

	if (m_stateChanges != 0)
		_encodeStateChanges();

	// Encode tints

	uint16_t	tintWords[2][TintTools::c_maxEncodedTintWords];
	HiColor		tintColors[2][TintTools::c_maxEncodedTintColors];
	int			nTintWords[2] = { 0, 0 };
	int			nTintColors[2] = { 0, 0 };

	Tint* pTints[2] = { bHasFill ? spec.pFillTint : nullptr, bHasOutline ? spec.pOutlineTint : nullptr };

	for (int i = 0; i < 2; i++)
	{
		if (pTints[i])
			nTintWords[i] = TintTools::encodeTint(pTints[i], rect, tintWords[i], tintColors[i], nTintColors[i]);
	}

	uint16_t flags = 0;
	if (bHasFill)
		flags |= uint16_t(GfxBackend::RoundedBoxFlags::HasFill);
	if (bHasOutline)
		flags |= uint16_t(GfxBackend::RoundedBoxFlags::HasOutline);
	if (pTints[0])
		flags |= uint16_t(GfxBackend::RoundedBoxFlags::HasFillTint);
	if (pTints[1])
		flags |= uint16_t(GfxBackend::RoundedBoxFlags::HasOutlineTint);

	int nWords = GfxBackend::c_roundedBoxPayloadWords + nTintWords[0] + nTintWords[1];
	int nColors = (bHasFill ? 1 : 0) + (bHasOutline ? 1 : 0) + nTintColors[0] + nTintColors[1];

	// Write command, aligned to 32 bits.

	auto& commands = m_pActiveLayer->commands;

	commands.secureSpace((GfxBackend::c_drawShapeHeaderWords + nWords + 1) * 2);

	if (commands.size() % 4 != 0)
		commands.pushUnchecked(uint16_t(Command::None));

	m_pActiveLayer->latestCommand = Command::DrawShape;
	m_pActiveLayer->latestCommandOfs = (int)commands.size();

	commands.pushUnchecked(uint16_t(Command::DrawShape));
	commands.pushUnchecked(uint16_t(GfxBackend::ShapeKind::RoundedBox));
	commands.pushUnchecked(uint16_t(nRects));
	commands.pushUnchecked(uint16_t(nColors));
	commands.pushUnchecked(uint16_t(nWords));
	commands.pushUnchecked(uint16_t(0));

	commands.pushUnchecked(rect);

	for (int i = 0; i < 4; i++)
		commands.pushUnchecked(radius[i]);

	commands.pushUnchecked(outline.top);
	commands.pushUnchecked(outline.right);
	commands.pushUnchecked(outline.bottom);
	commands.pushUnchecked(outline.left);

	commands.pushUnchecked(spx(0));					// Softness, reserved.
	commands.pushUnchecked(flags);
	commands.pushUnchecked(uint16_t(0));

	for (int i = 0; i < 2; i++)
		for (int w = 0; w < nTintWords[i]; w++)
			commands.pushUnchecked(tintWords[i][w]);

	// Colors

	auto& colors = m_pActiveLayer->colors;

	if (bHasFill)
		colors.push_back(spec.fillColor);
	if (bHasOutline)
		colors.push_back(spec.outlineColor);

	for (int i = 0; i < 2; i++)
		colors.insert(colors.end(), tintColors[i], tintColors[i] + nTintColors[i]);

	auto& info = m_pActiveCanvas->sessionInfo;
	info.nShapeDraws++;
	info.nShapeSegments += 2;
	info.nShapeTints += (pTints[0] ? 1 : 0) + (pTints[1] ? 1 : 0);
}

//____ drawRing() ____________________________________________________________

void GfxDeviceGen2::drawRing(const RingSpec& spec)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to draw ring to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	const RectSPX& rect = spec.rect;

	if (rect.w <= 0 || rect.h <= 0 || !rect.isOverlapping(m_pActiveClipList->bounds))
		return;

	int nSlices = spec.nSlices;

	if (nSlices < 0 || (nSlices > 0 && !spec.pSlices))
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid slices.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (nSlices > c_maxRingSlices)
	{
		GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "Too many slices, the ones beyond maxRingSlices() are not drawn.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		nSlices = c_maxRingSlices;
	}

	auto isDrawn = [this](HiColor color) { return _isShapeColorDrawn(color); };

	spx radius = std::min(rect.w, rect.h) / 2;
	spx outline = std::max(spec.outline, 0);
	bool bHole = spec.thickness > 0 && spec.thickness < radius;

	bool bOutline = outline > 0 && isDrawn(spec.outlineColor);
	bool bHub = bHole && isDrawn(spec.hubColor);
	bool bBack = isDrawn(spec.backColor);

	bool bAnySlice = false;
	for (int i = 0; i < nSlices; i++)
		if (spec.pSlices[i].size > 0.f && isDrawn(spec.pSlices[i].color))
			bAnySlice = true;

	if (!bAnySlice && !bOutline && !bHub && !bBack)
		return;

	// Clip against the circle if nothing is drawn outside it.

	RectSPX area = rect;

	if (!bBack && !spec.bSquare)
		area = RectSPX(rect.x + rect.w / 2 - radius, rect.y + rect.h / 2 - radius, radius * 2, radius * 2);

	int nRects = _addShapeClipRects(area);

	if (nRects == 0)
		return;

	if (m_stateChanges != 0)
		_encodeStateChanges();

	// Header, counts filled in when known. Aligned to 32 bits.

	auto& commands = m_pActiveLayer->commands;
	auto& colors = m_pActiveLayer->colors;

	commands.secureSpace((GfxBackend::c_drawShapeHeaderWords + GfxBackend::c_ringPayloadWords + nSlices * GfxBackend::c_ringSliceWords + 1) * 2);

	if (commands.size() % 4 != 0)
		commands.pushUnchecked(uint16_t(Command::None));

	m_pActiveLayer->latestCommand = Command::DrawShape;
	m_pActiveLayer->latestCommandOfs = (int)commands.size();

	int headerOfs = (int)commands.size();
	int nColorsBefore = (int)colors.size();

	commands.pushUnchecked(uint16_t(Command::DrawShape));
	commands.pushUnchecked(uint16_t(GfxBackend::ShapeKind::Ring));
	commands.pushUnchecked(uint16_t(nRects));
	commands.pushUnchecked(uint16_t(0));						// nColors, filled in below.
	commands.pushUnchecked(uint16_t(0));						// nWords, filled in below.
	commands.pushUnchecked(uint16_t(0));

	int payloadOfs = (int)commands.size();

	uint16_t flags = 0;
	if (spec.bSquare)
		flags |= uint16_t(GfxBackend::RingFlags::Square);
	if (bOutline)
		flags |= uint16_t(GfxBackend::RingFlags::HasOutline);
	if (bHub)
		flags |= uint16_t(GfxBackend::RingFlags::HasHub);
	if (bBack)
		flags |= uint16_t(GfxBackend::RingFlags::HasBack);
	if (bOutline && spec.pOutlineTint)
		flags |= uint16_t(GfxBackend::RingFlags::HasOutlineTint);
	if (bHub && spec.pHubTint)
		flags |= uint16_t(GfxBackend::RingFlags::HasHubTint);
	if (bBack && spec.pBackTint)
		flags |= uint16_t(GfxBackend::RingFlags::HasBackTint);

	float start = std::isfinite(spec.start) ? spec.start - std::floor(spec.start) : 0.f;

	commands.pushUnchecked(rect);
	commands.pushUnchecked(spec.thickness);
	commands.pushUnchecked(outline);
	commands.pushUnchecked(spx(0));								// Softness, reserved.
	commands.pushUnchecked(flags);
	commands.pushUnchecked(uint16_t(nSlices));
	commands.pushUnchecked(uint32_t(double(start) * GfxBackend::c_shapeFullTurn) % GfxBackend::c_shapeFullTurn);

	int nTints = 0;

	for (int i = 0; i < nSlices; i++)
	{
		const RingSlice& slice = spec.pSlices[i];

		float size = slice.size > 0.f ? std::min(slice.size, 1.f) : 0.f;		// Also for NaN.
		bool bDrawn = size > 0.f && isDrawn(slice.color);

		uint16_t sliceFlags = 0;
		if (bDrawn)
			sliceFlags |= uint16_t(GfxBackend::RingSliceFlags::Drawn);
		if (bDrawn && slice.pTint)
			sliceFlags |= uint16_t(GfxBackend::RingSliceFlags::Tinted);

		commands.pushUnchecked(uint32_t(double(size) * GfxBackend::c_shapeFullTurn));
		commands.pushUnchecked(sliceFlags);
		commands.pushUnchecked(uint16_t(0));
	}

	// Colors

	if (bOutline)
		colors.push_back(spec.outlineColor);
	if (bHub)
		colors.push_back(spec.hubColor);
	if (bBack)
		colors.push_back(spec.backColor);

	for (int i = 0; i < nSlices; i++)
		if (spec.pSlices[i].size > 0.f && isDrawn(spec.pSlices[i].color))
			colors.push_back(spec.pSlices[i].color);

	// Tints, encoded one at a time.

	auto encodeTint = [&](Tint* pTint)
	{
		uint16_t	words[TintTools::c_maxEncodedTintWords];
		HiColor		tintColors[TintTools::c_maxEncodedTintColors];
		int			nTintColors;

		int nWords = TintTools::encodeTint(pTint, rect, words, tintColors, nTintColors);

		commands.secureSpace(nWords * 2);
		for (int w = 0; w < nWords; w++)
			commands.pushUnchecked(words[w]);

		colors.insert(colors.end(), tintColors, tintColors + nTintColors);
		nTints++;
	};

	if (flags & uint16_t(GfxBackend::RingFlags::HasOutlineTint))
		encodeTint(spec.pOutlineTint);
	if (flags & uint16_t(GfxBackend::RingFlags::HasHubTint))
		encodeTint(spec.pHubTint);
	if (flags & uint16_t(GfxBackend::RingFlags::HasBackTint))
		encodeTint(spec.pBackTint);

	for (int i = 0; i < nSlices; i++)
	{
		const RingSlice& slice = spec.pSlices[i];
		if (slice.size > 0.f && isDrawn(slice.color) && slice.pTint)
			encodeTint(slice.pTint);
	}

	// Fill in counts

	int nWords = ((int)commands.size() - payloadOfs) / 2;
	int nColors = (int)colors.size() - nColorsBefore;

	commands.at(headerOfs + 6).as<uint16_t>() = uint16_t(nColors);
	commands.at(headerOfs + 8).as<uint16_t>() = uint16_t(nWords);

	auto& info = m_pActiveCanvas->sessionInfo;
	info.nShapeDraws++;
	info.nShapeSegments += 3 + nSlices;
	info.nShapeTints += nTints;
}

//____ blit() ________________________________________________________________

void GfxDeviceGen2::blit(CoordSPX dest)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}
	
	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}
	
	_transformBlitSimple({ dest, m_renderState.blitSource->pixelSize()*64 }, { 0,0 }, 0, Command::Blit);
}

void GfxDeviceGen2::blit(CoordSPX dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}
	
	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_transformBlitSimple({ dest, src.size() }, src.pos(), 0, Command::Blit);
}

//____ flipBlit() _________________________________________________________

void GfxDeviceGen2::flipBlit(CoordSPX dest, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	stretchBlit(dest, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64));
}

void GfxDeviceGen2::stretchBlit(const RectSPX& _dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	// We draw whole pixels, so the scale is calculated from the destination aligned
	// to them. Otherwise the scale doesn't match the pixels drawn and samples can end
	// up outside the source.

	RectSPX dest = align(_dest);

	if (dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
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
				mtx.xx = (std::max(src.w - 65, 0) / 64.f) / ((dest.w / 64) - 1);

			mtx.xy = 0;
			mtx.yx = 0;

			if (dest.h == 64)
				mtx.yy = 0;
			else
				mtx.yy = (std::max(src.h - 65, 0) / 64.f) / ((dest.h / 64) - 1);
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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	stretchFlipBlit(dest, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64), flip);
}

void GfxDeviceGen2::stretchFlipBlit(const RectSPX& _dest, const RectSPX& src, GfxFlip flip)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	// Scale is calculated from the destination aligned to whole pixels, see stretchBlit().

	RectSPX dest = align(_dest);

	if (dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
		return;

	float scaleX, scaleY;
	float ofsX, ofsY;

	float srcW = src.w / 64.f;
	float srcH = src.h / 64.f;

	// Number of destination pixels along source x and y. Rotations swap them.

	bool bRotated = (s_blitFlipTransforms[(int)flip][0][0] == 0);

	int destAlongSrcX = (bRotated ? dest.h : dest.w) / 64;
	int destAlongSrcY = (bRotated ? dest.w : dest.h) / 64;

	if (m_renderState.blitSource->sampleMethod() == SampleMethod::Bilinear)
	{
		srcW = std::max(srcW - 1.f, 0.f);
		srcH = std::max(srcH - 1.f, 0.f);

		if (destAlongSrcX == 1)
			scaleX = 0;
		else
			scaleX = srcW / (destAlongSrcX - 1);

		if (destAlongSrcY == 1)
			scaleY = 0;
		else
			scaleY = srcH / (destAlongSrcY - 1);

		ofsX = (src.x / 64.f) + (srcW * s_blitFlipOffsets[(int)flip][0]);
		ofsY = (src.y / 64.f) + (srcH * s_blitFlipOffsets[(int)flip][1]);
	}
	else
	{
		// Last sample is taken scale pixels from the end of the source, which is where a
		// flipped axis starts.

		scaleX = srcW / destAlongSrcX;
		scaleY = srcH / destAlongSrcY;

		ofsX = (src.x / 64.f) + (srcW - scaleX) * s_blitFlipOffsets[(int)flip][0];
		ofsY = (src.y / 64.f) + (srcH - scaleY) * s_blitFlipOffsets[(int)flip][1];
	}

	Transform	mtx;

	mtx.xx = scaleX * s_blitFlipTransforms[(int)flip][0][0];
	mtx.xy = scaleY * s_blitFlipTransforms[(int)flip][0][1];
	mtx.yx = scaleX * s_blitFlipTransforms[(int)flip][1][0];
	mtx.yy = scaleY * s_blitFlipTransforms[(int)flip][1][1];

	// Source position is in 1/1024 pixels. Rounded up, since a flipped axis counts down
	// from it and would end up just outside the source if it started a bit too low.

	_transformBlitComplex(dest, { int(std::ceil(ofsX*1024)), int(std::ceil(ofsY*1024)) }, mtx, Command::Blit);
}

//____ precisionBlit() ____________________________________________________

void GfxDeviceGen2::precisionBlit(const RectSPX& _dest, const RectF& src)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	// Scale is calculated from the destination aligned to whole pixels, see stretchBlit().

	RectSPX dest = align(_dest);

	if (dest.w == 0 || dest.h == 0 || src.w <= 0 || src.h <= 0)
		return;

	// The source rectangle is mapped linearly onto the destination. That keeps blits of
	// adjacent parts of a source consistent with each other, so a stretched image can be
	// drawn piece by piece.

	Transform	mtx;
	CoordF		srcPos = { src.x / 64.f, src.y / 64.f };		// In pixels.

	mtx.xx = src.w / dest.w;
	mtx.xy = 0;
	mtx.yx = 0;
	mtx.yy = src.h / dest.h;

	if (m_renderState.blitSource->sampleMethod() != SampleMethod::Bilinear)
	{
		_transformBlitComplex(dest, { int(srcPos.x*1024), int(srcPos.y*1024) }, mtx, Command::Blit);
		return;
	}

	// A bilinear sample also reads the next pixel, so samples need to stay at least a little
	// bit before the last pixel of the source surface. Destination pixels whose samples would
	// reach beyond that are drawn with the edge of the source instead, as a separate blit
	// with no scale in that direction. So along each axis there are one or two parts.

	struct Part
	{
		spx		destBegin;
		spx		destLength;
		float	srcPos;				// In pixels.
		float	scale;
	};

	auto split = [](float pos, float scale, int surfPixels, spx destBegin, int destPixels, Part* pParts) -> int
	{
		float max = std::max(surfPixels - 1 - 1/64.f, 0.f);

		pos = std::clamp(pos, 0.f, max);

		int inside = destPixels;					// Destination pixels with samples inside the source.
		while (inside > 1 && pos + scale * (inside - 1) > max)
			inside--;

		pParts[0] = { destBegin, inside * 64, pos, scale };

		if (inside == destPixels)
			return 1;

		pParts[1] = { destBegin + inside * 64, (destPixels - inside) * 64, max, 0.f };
		return 2;
	};

	SizeI surfSize = m_renderState.blitSource->pixelSize();

	Part	partsX[2], partsY[2];
	int		nPartsX = split(srcPos.x, mtx.xx, surfSize.w, dest.x, dest.w / 64, partsX);
	int		nPartsY = split(srcPos.y, mtx.yy, surfSize.h, dest.y, dest.h / 64, partsY);

	for (int y = 0; y < nPartsY; y++)
	{
		for (int x = 0; x < nPartsX; x++)
		{
			const Part& px = partsX[x];
			const Part& py = partsY[y];

			mtx.xx = px.scale;
			mtx.yy = py.scale;

			_transformBlitComplex({ px.destBegin, py.destBegin, px.destLength, py.destLength }, { int(px.srcPos*1024), int(py.srcPos*1024) }, mtx, Command::Blit);
		}
	}
}

//____ transformBlit() ________________________________________________

void GfxDeviceGen2::transformBlit(const RectSPX& dest, CoordF src, const Transform& transform)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	Command cmd = m_renderState.blitSource->isTiling() ? Command::Tile : Command::ClipBlit;

	_transformBlitComplex(dest, { int(src.x*16), int(src.y*16) }, transform, cmd);
}

//____ rotScaleBlit() _____________________________________________________

void GfxDeviceGen2::rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource->m_size.w * scale < 1.f || pSource->m_size.h * scale < 1.f)			// Values very close to zero gives overflow in calculations.
		return;

	CoordF		src;
	Transform	mtx;

	float	sz = std::sin(-rotationDegrees * 3.14159265f / 180.f);
	float	cz = std::cos(-rotationDegrees * 3.14159265f / 180.f);

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to tile to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	auto pSource = m_renderState.blitSource;

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to tile to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	auto pSource = m_renderState.blitSource;

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to tile to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	auto pSource = m_renderState.blitSource;

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to tile to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	auto pSource = m_renderState.blitSource;

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blur to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if( m_renderState.pBlurbrush == nullptr )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Blurbrush set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_transformBlitSimple({ dest, m_renderState.blitSource->pixelSize() * 64 }, { 0,0 }, 0, Command::Blur);
}

void GfxDeviceGen2::blur(CoordSPX dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blur to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.pBlurbrush == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Blurbrush set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_transformBlitSimple({ dest, src.size() }, src.pos(), 0, Command::Blur);
}

//____ stretchBlur() ___________________________________________________________

void GfxDeviceGen2::stretchBlur(const RectSPX& dest)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blur to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.pBlurbrush == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Blurbrush set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	stretchBlur(dest, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64));
}

void GfxDeviceGen2::stretchBlur(const RectSPX& _dest, const RectSPX& src)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blur to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.pBlurbrush == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Blurbrush set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	// Scale is calculated from the destination aligned to whole pixels, see stretchBlit().

	RectSPX dest = align(_dest);

	if (dest.w == 0 || dest.h == 0 || src.w == 0 || src.h == 0)
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
				mtx.xx = (std::max(src.w - 64, 0) / 64.f) / ((dest.w / 64) - 1);

			mtx.xy = 0;
			mtx.yx = 0;

			if (dest.h == 64)
				mtx.yy = 0;
			else
				mtx.yy = (std::max(src.h - 64, 0) / 64.f) / ((dest.h / 64) - 1);
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

		_transformBlitComplex(dest, { src.x*16, src.y*16 }, mtx, Command::Blur);
	}
}
 
//____ transformBlur() ________________________________________________

void GfxDeviceGen2::transformBlur(const RectSPX& dest, CoordF src, const Transform& transform)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blur to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.pBlurbrush == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Blurbrush set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_transformBlitComplex(dest, { int(src.x*16), int(src.y*16) }, transform, Command::Blur);
}

//____ rotScaleBlur() _____________________________________________________

void GfxDeviceGen2::rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter, CoordF destCenter)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blur to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.pBlurbrush == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No Blurbrush set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	auto pSource = m_renderState.blitSource;

	if (pSource->m_size.w * scale < 1.f || pSource->m_size.h * scale < 1.f)			// Values very close to zero gives overflow in calculations.
		return;

	CoordSPX	src;
	Transform	mtx;

	float	sz = std::sin(-rotationDegrees * 3.14159265f / 180.f);
	float	cz = std::cos(-rotationDegrees * 3.14159265f / 180.f);

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
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to blit to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

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
	if(!pEdgemap)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Null edgemap pointer passed to drawEdgemap.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to draw edgemap to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	SizeSPX edgemapSize = pEdgemap->pixelSize() * 64;
	const int(&transform)[2][2] = s_blitFlipTransforms[(int)flip];

	RectSPX rect;

	rect.x = dest.x;
	rect.y = dest.y;
	rect.w = edgemapSize.w * abs(transform[0][0]) + edgemapSize.h * abs(transform[1][0]);
	rect.h = edgemapSize.w * abs(transform[0][1]) + edgemapSize.h * abs(transform[1][1]);

	_encodeEdgemapDraw(rect, dest, pEdgemap, flip, Command::DrawEdgemap);
}

//____ tileEdgemap() ___________________________________________________________

void GfxDeviceGen2::tileEdgemap(const RectSPX& dest, Edgemap* pEdgemap, CoordSPX shift)
{
	flipTileEdgemap(dest, pEdgemap, GfxFlip::None, shift);
}

//____ flipTileEdgemap() _______________________________________________________
/**
 * @brief Fill a rectangle with an edgemap, repeated horizontally and vertically.
 *
 * The edgemap is flipped/rotated before it is repeated. The top-left corner of
 * one copy of the flipped edgemap is placed at dest.pos() - shift, so shift is given
 * in canvas space and moves the pattern the opposite way, like scrolling.
 *
 * The edgemap needs to be tiling, see Edgemap::Blueprint::tiling.
 */

void GfxDeviceGen2::flipTileEdgemap(const RectSPX& dest, Edgemap* pEdgemap, GfxFlip flip, CoordSPX shift)
{
	if(!pEdgemap)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Null edgemap pointer passed to tileEdgemap.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to tile edgemap to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	if (!pEdgemap->isTiling())
	{
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::FailedPrerequisite, "Tile method called on non-tiling edgemap",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_encodeEdgemapDraw(dest, dest.pos() - shift, pEdgemap, flip, Command::TileEdgemap);
}

//____ _encodeEdgemapDraw() ____________________________________________________
/*
	Encodes a DrawEdgemap or TileEdgemap command for the parts of area that are
	within the clip rectangles. Origo is the top-left corner of the (flipped)
	edgemap, or of one of the copies when tiling.
*/

void GfxDeviceGen2::_encodeEdgemapDraw(const RectSPX& area, CoordSPX origo, Edgemap* pEdgemap, GfxFlip flip, Command cmd)
{
	int nRects = 0;
	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], area);

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

		m_pActiveLayer->latestCommand = cmd;

		uint32_t edgemapIndex = _objectIndex(pEdgemap);

		m_pActiveLayer->commands.secureSpace(20);

		m_pActiveLayer->commands.pushUnchecked(uint16_t(cmd));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(nRects));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(flip));
		m_pActiveLayer->commands.pushUnchecked(uint16_t(0));									// padding
		m_pActiveLayer->commands.pushUnchecked(edgemapIndex);
		m_pActiveLayer->commands.pushUnchecked(origo);

		m_pActiveCanvas->sessionInfo.nEdgemapDraws++;
	}
}

//____ warpBlit() ____________________________________________________________

void GfxDeviceGen2::warpBlit(CoordSPX dest, Warpmap* pWarpmap, CoordSPX sourceOffset)
{
	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	warpBlit(dest, pWarpmap, RectSPX(0, 0, m_renderState.blitSource->pixelSize() * 64), sourceOffset);
}

void GfxDeviceGen2::warpBlit(CoordSPX dest, Warpmap* pWarpmap, const RectSPX& source, CoordSPX sourceOffset)
{
	if (m_renderState.blitSource == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No blit source set.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (source.w <= 0 || source.h <= 0)
		return;

	_encodeWarp(dest, pWarpmap, nullptr, &source, sourceOffset);
}

//____ warpDrawEdgemap() _____________________________________________________

void GfxDeviceGen2::warpDrawEdgemap(CoordSPX dest, Warpmap* pWarpmap, Edgemap* pEdgemap, CoordSPX sourceOffset)
{
	if (!pEdgemap)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Null edgemap pointer passed to warpDrawEdgemap.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_encodeWarp(dest, pWarpmap, pEdgemap, nullptr, sourceOffset);
}

//____ _encodeWarp() __________________________________________________________
/*
	Encodes a WarpBlit (pSource set) or WarpDrawEdgemap (pEdgemap set) command for the parts of
	the rectangle of the Warpmap that are within the clip rectangles. The Warpmap is placed on
	whole pixels.
*/

void GfxDeviceGen2::_encodeWarp(CoordSPX dest, Warpmap* pWarpmap, Edgemap* pEdgemap, const RectSPX* pSource, CoordSPX sourceOffset)
{
	if (!pWarpmap)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Null warpmap pointer passed to warp method.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to draw to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	RectSPX area = { Util::roundToPixels(dest) * 64, pWarpmap->pixelSize() * 64 };

	int nRects = 0;
	auto& rects = m_pActiveLayer->rects;

	for (int i = 0; i < m_pActiveClipList->nRects; i++)
	{
		RectSPX clipped = RectSPX::overlap(m_pActiveClipList->pRects[i], area);

		if (!clipped.isEmpty())
		{
			rects.emplace_back(clipped);
			nRects++;
		}
	}

	if (nRects == 0)
		return;

	if (m_stateChanges != 0)
		_encodeStateChanges();

	Command cmd = pEdgemap ? Command::WarpDrawEdgemap : Command::WarpBlit;

	m_pActiveLayer->latestCommand = cmd;

	uint32_t warpmapIndex = _objectIndex(pWarpmap);
	uint32_t edgemapIndex = pEdgemap ? _objectIndex(pEdgemap) : 0;

	m_pActiveLayer->commands.secureSpace(pSource ? 40 : 28);

	m_pActiveLayer->commands.pushUnchecked(uint16_t(cmd));
	m_pActiveLayer->commands.pushUnchecked(uint16_t(nRects));
	m_pActiveLayer->commands.pushUnchecked(warpmapIndex);

	if (pEdgemap)
		m_pActiveLayer->commands.pushUnchecked(edgemapIndex);

	m_pActiveLayer->commands.pushUnchecked(dest);
	m_pActiveLayer->commands.pushUnchecked(sourceOffset);

	if (pSource)
		m_pActiveLayer->commands.pushUnchecked(*pSource);

	m_pActiveCanvas->sessionInfo.nWarpDraws++;
}

//____ drawWave() ____________________________________________________________

void GfxDeviceGen2::drawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor fill)
{
	flipDrawWave(dest, pTopBorder, pBottomBorder, fill, GfxFlip::None);
}

//____ flipDrawWave() __________________________________________________________

void GfxDeviceGen2::flipDrawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor fill, GfxFlip flip)
{
	if (!m_pActiveCanvas )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No active canvas to draw wave to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return;
	}

	RectI pixDest = dest / 64;

	// Create wave

	auto pWave = EdgemapWave::create( WGBP(EdgemapWave,
		_.size = pixDest.size()*64,
		_.color = fill,
		_.factory = m_pBackend->edgemapFactory(),
		_.flip = flip,
		_.pos = dest.pos(),
		_.topOutlineThickness = pTopBorder->thickness,
		_.bottomOutlineThickness = pBottomBorder->thickness,
		_.outlineColor = pTopBorder->color ) );

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

	pWave->refresh();
	pWave->draw(this);
}

//____ drawSegments() ________________________________________________________

void GfxDeviceGen2::drawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, TintMode tintMode)
{
	flipDrawSegments(dest, nSegments, pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, GfxFlip::None, tintMode);
}

//____ flipDrawSegments() ______________________________________________________

void GfxDeviceGen2::flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode)
{
	if (!m_pActiveCanvas)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "No active canvas to draw segments to.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
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
/*
	Deprecated, use drawRing(). Draws the largest circle centered in canvas: a ring of given
	thickness with an outline of outlineThickness along both edges. Edges reaching past the
	center leave a disc of what is inside them.
*/

void GfxDeviceGen2::drawElipse(const RectSPX& canvas, spx thickness, HiColor fillColor, spx outlineThickness, HiColor outlineColor)
{
	thickness = std::max(thickness, 0);
	outlineThickness = std::max(outlineThickness, 0);

	spx radius = std::min(canvas.w, canvas.h) / 2;
	spx total = thickness + outlineThickness * 2;

	if (total <= 0 || radius <= 0)
		return;

	RingSlice slice;
	slice.size = 1.f;
	slice.color = fillColor;

	RingSpec ring;
	ring.rect = canvas;
	ring.nSlices = 1;
	ring.pSlices = &slice;
	ring.outlineColor = outlineColor;

	if (total < radius || thickness + outlineThickness >= radius)
	{
		// Ring with outline along both edges, or disc with outline.

		ring.thickness = total < radius ? total : 0;
		ring.outline = outlineThickness;
		drawRing(ring);
	}
	else
	{
		// Only the inner outline reaches the center: disc of outline color, fill on top.

		slice.color = outlineColor;
		drawRing(ring);

		if (thickness > 0)
		{
			slice.color = fillColor;
			ring.rect = RectSPX(canvas.x + outlineThickness, canvas.y + outlineThickness, canvas.w - outlineThickness * 2, canvas.h - outlineThickness * 2);
			ring.thickness = thickness;
			drawRing(ring);
		}
	}
}

//____ drawPieChart() _____________________________________________________
/*
	Deprecated, use drawRing(). Draws a pie chart in the largest circle centered in canvas,
	or filling canvas if bRectangular. hubSize is the size of the hole as a fraction of the
	radius. backColor fills the rest of canvas and what the slices don't reach.
*/

void GfxDeviceGen2::drawPieChart(const RectSPX& canvas, float start, int nSlices, const float* pSliceSizes, const HiColor* pSliceColors, float hubSize, HiColor hubColor, HiColor backColor, bool bRectangular)
{
	if (nSlices < 0 || (nSlices > 0 && (!pSliceSizes || !pSliceColors)))
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid slices.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (hubSize >= 1.f && !bRectangular)			// Hub covering all, nothing drawn as before.
		return;

	int bufferSize = sizeof(RingSlice) * std::max(nSlices, 1);
	auto slices = (RingSlice*) GfxBase::memStackAlloc(bufferSize);

	for (int i = 0; i < nSlices; i++)
		new (&slices[i]) RingSlice{ pSliceSizes[i], pSliceColors[i], nullptr };

	spx radius = std::min(canvas.w, canvas.h) / 2;

	RingSpec ring;
	ring.rect = canvas;
	ring.bSquare = bRectangular;
	ring.start = start;
	ring.nSlices = nSlices;
	ring.pSlices = slices;
	ring.hubColor = hubColor;
	ring.backColor = backColor;

	if (hubSize > 0.f)
		ring.thickness = std::max(spx(radius * (1.f - std::min(hubSize, 1.f))), spx(1));

	drawRing(ring);

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


		int transformOfs = int(m_pActiveCanvas->transforms.size()) + GfxBackend::NbStandardTransforms;
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
	state.tintRect.clear();
	state.pTint = nullptr;
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
			cmdBuffer.push(_objectIndex(pSource));

			encodedState.blitSource = pSource;
			statesChanged |= int(StateChange::BlitSource);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::TintColor))
	{
		auto tintColor = newState.tintColor.isUndefined() ? HiColor::White : newState.tintColor;

		if (tintColor != encodedState.tintColor)
		{
			colorBuffer.push_back(tintColor);
			encodedState.tintColor = tintColor;

			statesChanged |= int(StateChange::TintColor);
		}
	}

	if (m_stateChanges & uint8_t(StateChange::Tint))
	{
		if (newState.pTint != encodedState.pTint || newState.tintRect != encodedState.tintRect)
		{
			uint16_t	words[TintTools::c_maxEncodedTintWords];
			HiColor		colors[TintTools::c_maxEncodedTintColors];
			int			nColors;

			int nWords = TintTools::encodeTint(newState.pTint, newState.tintRect, words, colors, nColors);

			cmdBuffer.secureSpace(nWords * 2);
			for (int i = 0; i < nWords; i++)
				cmdBuffer.pushUnchecked(words[i]);

			colorBuffer.insert(colorBuffer.end(), colors, colors + nColors);

			encodedState.pTint = newState.pTint;
			encodedState.tintRect = newState.tintRect;

			statesChanged |= int(StateChange::Tint);
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

//____ _objectIndex() __________________________________________________________
/*
	Returns the index of the object in the object table of the active layer, adding
	it (and retaining it until the session has been rendered) if it isn't there.
	Null is added like any other object, it stands for no blit source.

	The cache is a hint only, see ObjectCacheEntry. The hash is Fibonacci hashing of
	the address, which spreads objects allocated close together over the entries.
*/

uint32_t GfxDeviceGen2::_objectIndex(Object* pObject)
{
	auto& layer = *m_pActiveLayer;

	uint64_t hash = (uint64_t(uintptr_t(pObject)) * 0x9E3779B97F4A7C15ull) >> (64 - c_objectCacheBits);
	auto& entry = layer.objectCache[hash];

	if (entry.generation == layer.objectCacheGeneration && entry.pObject == pObject)
		return entry.index;

	uint32_t index = (uint32_t) layer.objects.size();

	layer.objects.push_back(pObject);
	if (pObject)
		pObject->retain();

	entry.pObject = pObject;
	entry.index = index;
	entry.generation = layer.objectCacheGeneration;

	return index;
}

} // namspace wg
