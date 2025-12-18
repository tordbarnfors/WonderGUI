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
#include <wg_renderlayercapsule.h>
#include <wg_gfxdevice.h>
#include <wg_patches.h>

namespace wg
{
	using namespace Util;

	const TypeInfo RenderLayerCapsule::TYPEINFO = { "RenderLayerCapsule", &Capsule::TYPEINFO };

	//____ create() ______________________________________________________________

	RenderLayerCapsule_p RenderLayerCapsule::create() 
	{ 
		return RenderLayerCapsule_p(new RenderLayerCapsule()); 
 	}

	RenderLayerCapsule_p RenderLayerCapsule::create(const Blueprint& blueprint) 
	{ 
		return RenderLayerCapsule_p(new RenderLayerCapsule(blueprint)); 
	}

	//____ constructor ____________________________________________________________

	RenderLayerCapsule::RenderLayerCapsule()
	{
	}

	//____ destructor _____________________________________________________________

	RenderLayerCapsule::~RenderLayerCapsule()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RenderLayerCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setRenderLayer() ______________________________________________________

	void RenderLayerCapsule::setRenderLayer(int layer)
	{
		if (layer != m_renderLayer)
		{
			m_renderLayer = layer;
			_requestRender();
		}
	}

	//____ _render() ________________________________________________________

	void RenderLayerCapsule::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		// Render our skin

		m_skin.render(pDevice, _canvas, m_scale, m_state);

		if (!slot._widget())
			return;

		int oldLayer = pDevice->renderLayer();
		if( m_renderLayer >= 0 )
			pDevice->setRenderLayer(m_renderLayer);

		RectSPX canvas = m_skin.contentRect(_canvas, m_scale, m_state);
		slot._widget()->_render(pDevice, canvas, canvas);


		if( m_renderLayer >= 0 )
			pDevice->setRenderLayer(oldLayer);
	}

} // namespace wg
