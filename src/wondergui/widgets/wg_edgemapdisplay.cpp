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
#include <wg_edgemapdisplay.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>

namespace wg
{
	using namespace Util;

	const TypeInfo EdgemapDisplay::TYPEINFO = { "EdgemapDisplay", &Widget::TYPEINFO };


	//____ destructor _____________________________________________________________

	EdgemapDisplay::~EdgemapDisplay()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& EdgemapDisplay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setEdgemap() _______________________________________________________

	void EdgemapDisplay::setEdgemap(Edgemap* pEdgemap)
	{
		if (pEdgemap == m_pEdgemap)
			return;

		m_pEdgemap = pEdgemap;

		_requestRender();
	}

	//____ setEdgemapPlacement() ______________________________________________

	void EdgemapDisplay::setEdgemapPlacement(Placement placement)
	{
		m_placement = placement;
		_requestRender();
	}

	//____ _defaultSize() ___________________________________________________

	SizeSPX EdgemapDisplay::_defaultSize(int scale) const
	{
		SizeSPX wantedSize = m_pEdgemap ? m_pEdgemap->pixelSize() * 64 : SizeSPX();
		return wantedSize + m_skin.contentBorderSize(scale);
	}

	//____ _matchingHeight() __________________________________________________

	spx EdgemapDisplay::_matchingHeight(spx width, int scale) const
	{
		auto padding = m_skin.contentBorderSize(scale);
		auto wantedHeight = m_pEdgemap ? m_pEdgemap->pixelSize().h * 64 : 0;

		spx height = wantedHeight + padding.h;
		return height;
	}

	//____ _matchingWidth() ___________________________________________________

	spx EdgemapDisplay::_matchingWidth(spx height, int scale) const
	{
		auto padding = m_skin.contentBorderSize(scale);
		auto wantedWidth = m_pEdgemap ? m_pEdgemap->pixelSize().w * 64 : 0;

		spx width = wantedWidth + padding.w;
		return width;
	}

	//____ _resize() __________________________________________________________

	void EdgemapDisplay::_resize(const SizeSPX& size, int scale)
	{
		Widget::_resize(size, scale);
	}

	//____ _render() __________________________________________________________

	void EdgemapDisplay::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		if (!m_pEdgemap)
		{
			Widget::_render(pDevice, _canvas, _window);
			return;
		}

		RectSPX canvas;
		ClipPopData clipPop;

		if (!m_skin.isEmpty())
		{
			m_skin.render(pDevice, _canvas, m_scale, m_state);
			canvas = _contentRect(_canvas);

			clipPop = limitClipList(pDevice, canvas);
		}
		else
			canvas = _canvas;

		SizeI edgemapSize = m_pEdgemap ? m_pEdgemap->pixelSize() * 64 : SizeI();

		RectSPX destRect = Util::placementToRect(m_placement, canvas, edgemapSize);

		pDevice->drawEdgemap(destRect.pos(), m_pEdgemap);

		popClipList(pDevice, clipPop);
	}

	//____ _alphaTest() _______________________________________________________

	bool EdgemapDisplay::_alphaTest(const CoordSPX& _ofs)
	{
/*
		if( m_pEdgemap )
		{
			RectSPX canvas 		= _contentRect(m_size);
			RectSPX destRect 	= _edgemapDisplayRect(m_scale) + canvas.pos();

			float relativeX = (_ofs.x - destRect.x) / (float) destRect.w;
			float relativeY = (_ofs.y - destRect.y) / (float) destRect.h;

			SizeSPX surfSize = m_pSurface->pixelSize() * 64;
			CoordSPX ofsInSurface = { spx(relativeX * surfSize.w), spx(relativeY * surfSize.h) };

			if( RectSPX(0,0,surfSize).contains(ofsInSurface) )
			{
				if( m_pSurface->alpha(ofsInSurface) > 0 )
					return true;
			}
		}
*/

		return Widget::_alphaTest(_ofs);
	}

} // namespace wg
