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

//____ tintColor() ________________________________________________________

void GfxDeviceGen2::setTintColor(HiColor color)
{
	if (color != m_renderState.tintColor)
	{
		m_renderState.tintColor = color;
		m_renderState.stateChanges |= int(StateChange::TintColor);
	}
}

//____ tintColor() ________________________________________________________

HiColor GfxDeviceGen2::tintColor() const
{
	return m_renderState.tintColor;
}

//____ setTintGradient() __________________________________________________

void GfxDeviceGen2::setTintGradient(const RectSPX& rect, const Gradient& gradient)
{
	// Gen2 does not have tint gradients. Maybe we try to translate them into
	// Tintmaps, but probably not. Probably better to change the calls
}

//____ clearTintGradient() ________________________________________________

void GfxDeviceGen2::clearTintGradient()
{
	// Gen2 does not have tint gradients. Maybe we try to translate them into
	// Tintmaps, but probably not. Probably better to change the calls
}

//____ setTintmap() _______________________________________________________

void GfxDeviceGen2::setTintmap(const RectSPX& rect, Tintmap* pTintmap)
{
	if (pTintmap != m_renderState.pTintmap || rect != m_renderState.tintmapRect )
	{
		m_renderState.pTintmap = pTintmap;
		m_renderState.tintmapRect = rect;

		m_renderState.stateChanges |= int(StateChange::TintMap);
	}
}

//____ clearTintmap() _____________________________________________________

void GfxDeviceGen2::clearTintmap()
{
	if (m_renderState.pTintmap)
	{
		m_renderState.pTintmap = nullptr;
		m_renderState.tintmapRect.clear();

		m_renderState.stateChanges |= int(StateChange::TintMap);
	}
}

//____ setBlendMode() _____________________________________________________

bool GfxDeviceGen2::setBlendMode(BlendMode blendMode)
{
	if (blendMode != m_renderState.blendMode)
	{
		//TODO: Check that blendMode is supported.

		m_renderState.blendMode = blendMode;
		m_renderState.stateChanges |= int(StateChange::BlendMode);
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
	if (pSource->typeInfo() != surfaceType())
	{
		//TODO: Error handling!

		return false;
	}

	if (pSource != m_renderState.blitSource)
	{
		m_renderState.blitSource = pSource;
		m_renderState.stateChanges |= int(StateChange::BlitSource);
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
		m_renderState.stateChanges |= int(StateChange::MorphFactor);
	}
}

//____ morphFactor() ______________________________________________________

float GfxDeviceGen2::morphFactor() const
{
	return m_renderState.morphFactor;
}

//____ setBlurMatrices() __________________________________________________

void GfxDeviceGen2::setBlurMatrices(spx radius, const float red[9], const float green[9], const float blue[9])
{
	bool bModified = radius != m_renderState.blurRadius;

	if (!bModified)
	{
		for (int i = 0; i < 9; i++)
		{
			if (red[i] != m_renderState.blurMtxR[i] || green[i] != m_renderState.blurMtxG[i] || blue[i] != m_renderState.blurMtxB[i])
			{
				bModified = true;
				break;
			}
		}
	}

	if (bModified)
	{
		m_renderState.blurRadius = radius;

		for (int i = 0; i < 9; i++)
		{
			m_renderState.blurMtxR[i] = red[i];
			m_renderState.blurMtxG[i] = green[i];
			m_renderState.blurMtxB[i] = blue[i];
		}

		m_renderState.stateChanges |= int(StateChange::Blur);
	}
}

//____ setFixedBlendColor() _______________________________________________

void GfxDeviceGen2::setFixedBlendColor(HiColor color)
{
	if (m_renderState.fixedBlendColor != color)
	{
		m_renderState.fixedBlendColor = color;
		m_renderState.stateChanges |= int(StateChange::FixedBlendColor);
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


	// Collect all RenderState changes done over this session of rendering
	// to the current layer and add them to the other layers.

	auto sessionStateChanges = m_pActiveLayer->currentState.stateChanges | m_renderState.stateChanges;

	for (auto& layer : m_pActiveCanvas->layers)
	{
		layer.currentState.stateChanges |= sessionStateChanges;
	}

	// Any stateChanges still not applied to our layer are remembered

	m_pActiveLayer->currentState.stateChanges = m_renderState.stateChanges;

	// Switch layer

	m_pActiveCanvas->activeLayer = layer;
	m_pActiveLayer = &m_pActiveCanvas->layers[layer];

	// No new render states but mark that stored ones needs to be set.

	m_renderState.stateChanges = int(StateChange::IncludeFromLayer);
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

		int32_t* pCommandsBeg = canvasData.layers[i].commands.data();
		int32_t* pCommandsEnd = pCommandsBeg + canvasData.layers[i].commands.size();
		m_pBackend->processCommands( pCommandsBeg, pCommandsEnd );
	}
	
	// Release objects
	
	for( auto pObj : canvasData.objects )
		pObj->release();

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

	if (m_renderState.stateChanges != 0)
		_encodeStateChanges();

	m_pActiveLayer->commands.push_back(int(Command::Fill));
	m_pActiveLayer->commands.push_back(m_pActiveClipList->nRects);

	m_pActiveLayer->commands.push_back( color.r );
	m_pActiveLayer->commands.push_back( color.g );
	m_pActiveLayer->commands.push_back( color.b );
	m_pActiveLayer->commands.push_back( color.a );
	

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

		if( !clipped.isEmpty() )
		{
			coords.emplace_back(clipped.x);
			coords.emplace_back(clipped.y);
			coords.emplace_back(clipped.w);
			coords.emplace_back(clipped.h);

			nRects++;
		}
	}

	if( nRects > 0 )
	{
		if (m_renderState.stateChanges != 0)
			_encodeStateChanges();

		m_pActiveLayer->commands.push_back(int(Command::Fill));
		m_pActiveLayer->commands.push_back(nRects);

		m_pActiveLayer->commands.push_back( color.r );
		m_pActiveLayer->commands.push_back( color.g );
		m_pActiveLayer->commands.push_back( color.b );
		m_pActiveLayer->commands.push_back( color.a );
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
	RectI dest = roundToPixels(_dest);

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
		if (m_renderState.stateChanges != 0)
			_encodeStateChanges();

		m_pActiveLayer->commands.push_back(int(cmd));
		m_pActiveLayer->commands.push_back(nRects);
		
		m_pActiveLayer->commands.push_back(transformOfs);
		m_pActiveLayer->commands.push_back( roundToPixels(src.x) );
		m_pActiveLayer->commands.push_back( roundToPixels(src.y) );
		m_pActiveLayer->commands.push_back( dest.x );
		m_pActiveLayer->commands.push_back( dest.y );
	}
}

//____ _encodeStateChanges() ______________________________________________

void GfxDeviceGen2::_encodeStateChanges()
{

	uint8_t	statesChanged = m_renderState.stateChanges;

	// Include stored changes if they have not been encoded yet

	if (statesChanged & uint8_t(StateChange::IncludeFromLayer))
	{
		statesChanged |= m_pActiveLayer->currentState.stateChanges;
		m_pActiveLayer->currentState.stateChanges = 0;
		statesChanged &= ~uint8_t(StateChange::IncludeFromLayer);
	}

	//

	auto& cmdBuffer = m_pActiveLayer->commands;

	cmdBuffer.push_back(int(Command::StateChange));
	cmdBuffer.push_back(statesChanged);

	if (statesChanged & uint8_t(StateChange::BlitSource))
	{
		Object* pSource = m_renderState.blitSource.rawPtr();

		cmdBuffer.push_back( (int) m_pActiveCanvas->objects.size());
		m_pActiveCanvas->objects.push_back(pSource);
		pSource->retain();
	}

	if (statesChanged & uint8_t(StateChange::BlendMode))
	{
		cmdBuffer.push_back(int(m_renderState.blendMode));
	}


	if (statesChanged & uint8_t(StateChange::TintColor))
	{
		cmdBuffer.push_back(m_renderState.tintColor.r);
		cmdBuffer.push_back(m_renderState.tintColor.g);
		cmdBuffer.push_back(m_renderState.tintColor.b);
		cmdBuffer.push_back(m_renderState.tintColor.a);
	}

	if (statesChanged & uint8_t(StateChange::TintMap))
	{
		Object* pTintmap = m_renderState.pTintmap.rawPtr();

		cmdBuffer.push_back( (int) m_pActiveCanvas->objects.size());
		m_pActiveCanvas->objects.push_back(pTintmap);
		pTintmap->retain();

		cmdBuffer.push_back(m_renderState.tintmapRect.x);
		cmdBuffer.push_back(m_renderState.tintmapRect.y);
		cmdBuffer.push_back(m_renderState.tintmapRect.w);
		cmdBuffer.push_back(m_renderState.tintmapRect.h);
	}

	if (statesChanged & uint8_t(StateChange::MorphFactor))
	{
		cmdBuffer.push_back(m_renderState.morphFactor * 4096);
	}

	if (statesChanged & uint8_t(StateChange::FixedBlendColor))
	{
		cmdBuffer.push_back(m_renderState.fixedBlendColor.r);
		cmdBuffer.push_back(m_renderState.fixedBlendColor.g);
		cmdBuffer.push_back(m_renderState.fixedBlendColor.b);
		cmdBuffer.push_back(m_renderState.fixedBlendColor.a);
	}

	if (statesChanged & uint8_t(StateChange::Blur))
	{
		//TODO: Implement!!!		
	}


	// Store state changes for this layer session.

	m_pActiveLayer->currentState.stateChanges |= m_renderState.stateChanges;

	m_renderState.stateChanges = 0;
}


} // namspace wg
