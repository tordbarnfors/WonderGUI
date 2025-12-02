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

#include <wg_dx12backend.h>
#include <wg_dx12surfacefactory.h>
#include <wg_dx12edgemapfactory.h>

#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo DX12Backend::TYPEINFO = { "DX12Backend", &GfxBackend::TYPEINFO };

	//____ create() ______________________________________________________________

	DX12Backend_p DX12Backend::create()
	{
		return DX12Backend_p(new DX12Backend());
	}

	//____ Constructor ___________________________________________________________

	DX12Backend::DX12Backend()
	{
		m_pSurfaceFactory = DX12SurfaceFactory::create();
		m_pEdgemapFactory = DX12EdgemapFactory::create();
	}

	//____ Destructor ____________________________________________________________

	DX12Backend::~DX12Backend()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12Backend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ beginRender() _______________________________________________________

	void DX12Backend::beginRender()
	{
	}

	//____ endRender() _________________________________________________________

	void DX12Backend::endRender()
	{
	}

	//____ beginSession() _____________________________________________________

	void DX12Backend::beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo)
	{
	}

	//____ endSession() _______________________________________________________

	void DX12Backend::endSession()
	{
	}

	//____ setCanvas() ________________________________________________________

	void DX12Backend::setCanvas(Surface* pSurface)
	{
	}

	void DX12Backend::setCanvas(CanvasRef ref)
	{
	}

	//____ setObjects() ________________________________________________________

	void DX12Backend::setObjects(Object* const* pBeg, Object* const* pEnd)
	{
	}

	//____ setRects() __________________________________________________________

	void DX12Backend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
	}

	//____ setColors() _________________________________________________________

	void DX12Backend::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
	}

	//____ setTransforms() _____________________________________________________

	void DX12Backend::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
	}

	//____ processCommands() ___________________________________________________

	void DX12Backend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd)
	{
	}

	//____ setDefaultCanvas() ___________________________________________

	bool DX12Backend::setDefaultCanvas(SizeSPX size, int scale)
	{
		m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;
		return true;
	}

	//____ canvasInfo() ________________________________________________________

	const CanvasInfo* DX12Backend::canvasInfo(CanvasRef ref) const
	{
		if (ref == CanvasRef::Default)
			return &m_defaultCanvas;
		else
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only Default canvas is supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return &m_dummyCanvas;
		}
	}

	//____ surfaceFactory() ____________________________________________________

	SurfaceFactory_p DX12Backend::surfaceFactory()
	{
		return m_pSurfaceFactory;
	}

	//____ edgemapFactory() ____________________________________________________

	EdgemapFactory_p DX12Backend::edgemapFactory()
	{
		return m_pEdgemapFactory;
	}

	//____ maxEdges() ___________________________________________________________

	int DX12Backend::maxEdges() const
	{
		return 0;
	}

	//____ canBeBlitSource() __________________________________________________

	bool DX12Backend::canBeBlitSource(const TypeInfo& type) const
	{
		return true;
	}

	//____ canBeCanvas() ________________________________________________________

	bool DX12Backend::canBeCanvas(const TypeInfo& type) const
	{
		return true;
	}

	//____ waitForCompletion() __________________________________________________

	void DX12Backend::waitForCompletion()
	{
	}

}