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
#include <wg_bitmapdisplay.h>
#include <wax_device.h>
#include <wg_util.h>

namespace wg
{
	using namespace Util;

	const TypeInfo BitmapDisplay::TYPEINFO = { "BitmapDisplay", &Widget::TYPEINFO };


	//____ destructor _____________________________________________________________

	BitmapDisplay::~BitmapDisplay()
	{
		if (m_pBitmap)
			m_pBitmap->removeObserver(m_bitmapObserverId);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BitmapDisplay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setBitmap() _______________________________________________________

	void BitmapDisplay::setBitmap(Bitmap* pBitmap)
	{
		if (pBitmap == m_pBitmap)
			return;

		if (m_pBitmap)
			m_pBitmap->removeObserver(m_bitmapObserverId);

		m_pBitmap = pBitmap;

		_adjustZoom();
		_adjustOffset();

		if (pBitmap)
			m_bitmapObserverId = pBitmap->addObserver([this](int nRects, const RectI* pRects) 
				{
					this->_bitmapModified(nRects, pRects);
				});


		_requestRender();
	}

	//____ setZoomToFit() _____________________________________________________

	void BitmapDisplay::setZoomToFit(bool activate)
	{
		if (activate != m_bZoomToFit)
		{
			m_bZoomToFit = activate;
			if (_adjustZoom())
				_requestRender();
		}
	}

	//____ setZoom() __________________________________________________________

	void BitmapDisplay::setZoom(float factor)
	{
		m_bZoomToFit = false;
		if( factor != m_zoom )
		{
			m_zoom = factor;
			_adjustOffset();
			_requestResize();
			_requestRender();
		}
	}

	//____ setOffset() ________________________________________________________

	void BitmapDisplay::setOffset(Coord offset)
	{
		m_offset = offset;
		if( _adjustOffset() )
			_requestRender();
	}

	//____ setUserZoomable() __________________________________________________

	void BitmapDisplay::setUserZoomable(float min, float max)
	{
		m_bZoomToFit = false;
		m_minZoom = min;
		m_maxZoom = max;
	}

	//____ setBitmapPlacement() ______________________________________________

	void BitmapDisplay::setBitmapPlacement(Placement placement)
	{
		m_placement = placement;
		if (_adjustOffset())
			_requestRender();
	}


	//____ _defaultSize() ___________________________________________________

	SizeSPX BitmapDisplay::_defaultSize(int scale) const
	{
		return _bitmapDisplayRect(scale) + m_skin.contentBorderSize(scale);
	}

	//____ _matchingHeight() __________________________________________________

	spx BitmapDisplay::_matchingHeight(spx width, int scale) const
	{
		auto padding = m_skin.contentBorderSize(scale);
		auto wantedSize = _bitmapDisplayRect(scale);

		spx height;
		if (m_bZoomToFit && wantedSize.w != 0)
		{
			width -= padding.w;
			height = align(wantedSize.h * width / wantedSize.w);
		}
		else
			height = wantedSize.h;


		height += padding.h;
		return height;
	}

	//____ _matchingWidth() ___________________________________________________

	spx BitmapDisplay::_matchingWidth(spx height, int scale) const
	{
		auto padding = m_skin.contentBorderSize(scale);
		auto wantedSize = _bitmapDisplayRect(scale);

		spx width;
		if (m_bZoomToFit && wantedSize.h != 0)
		{
			height -= padding.h;
			width = align(wantedSize.w * height / wantedSize.h);
		}
		else
			width = wantedSize.w;

		width += padding.w;
		return width;
	}

	//____ _resize() __________________________________________________________

	void BitmapDisplay::_resize(const SizeSPX& size, int scale)
	{
		Widget::_resize(size, scale);

		_adjustZoom();
		_adjustOffset();
	}

	//____ _render() __________________________________________________________

	void BitmapDisplay::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		if (!m_pBitmap)
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

		RectSPX destRect = _bitmapDisplayRect(m_scale) + canvas.pos();


		pDevice->setBlitSource(m_pBitmap);
		pDevice->stretchBlit( destRect );

		popClipList(pDevice, clipPop);
	}

	//____ _alphaTest() _______________________________________________________

	bool BitmapDisplay::_alphaTest(const CoordSPX& _ofs)
	{
		if( m_pBitmap )
		{
			RectSPX canvas 		= _contentRect(m_size);
			RectSPX destRect 	= _bitmapDisplayRect(m_scale) + canvas.pos();

			float relativeX = (_ofs.x - destRect.x) / (float) destRect.w;
			float relativeY = (_ofs.y - destRect.y) / (float) destRect.h;

			SizeSPX bitmapSize = m_pBitmap->pixelSize() * 64;
			CoordSPX ofsInBitmap = { spx(relativeX * bitmapSize.w), spx(relativeY * bitmapSize.h) };

			if( RectSPX(0,0,bitmapSize).contains(ofsInBitmap) )
			{
				if( m_pBitmap->alpha(ofsInBitmap) > 0 )
					return true;
			}
		}

		return Widget::_alphaTest(_ofs);
	}

	//____ _bitmapModified() _________________________________________________

	void BitmapDisplay::_bitmapModified(int nRects, const RectI* pRects)
	{
		auto contentRect = _contentRect();

		for (int i = 0; i < nRects; i++)
		{
			auto rectInCanvas = alignUp(_bitmapRectToCanvasRect(pRects[i]*64));
			_requestRender(RectSPX::bounds(contentRect, rectInCanvas));
		}
	}

	//____ _bitmapRectToCanvasRect() _________________________________________

	RectSPX BitmapDisplay::_bitmapRectToCanvasRect(const RectSPX& bitmapRect)
	{
		auto bitmapSize = m_pBitmap->pixelSize();

		RectF relativeBitmapRect = RectF(bitmapRect);
		relativeBitmapRect /= SizeF(bitmapSize) * 64;

		RectSPX canvasRect = _bitmapDisplayRect(m_scale);

		canvasRect.x += canvasRect.w * relativeBitmapRect.x;
		canvasRect.y += canvasRect.h * relativeBitmapRect.y;
		canvasRect.w *= relativeBitmapRect.w;
		canvasRect.h *= relativeBitmapRect.h;

		return canvasRect;
	}

	//____ _bitmapDisplayRect() ______________________________________________

	RectSPX BitmapDisplay::_bitmapDisplayRect(int scale) const
	{
		if (m_pBitmap)
		{
			Size bitmapSize = m_pBitmap->pointSize();

			SizeSPX size = align(ptsToSpx(bitmapSize * m_zoom, scale));
			CoordSPX pos = align( CoordSPX( m_size.w/2 - (size.w/2 + m_offset.x * size.w / bitmapSize.w), m_size.h/2 -(size.h/2 + m_offset.y * size.h / bitmapSize.h) ) );

			return { pos, size };
		}

		return SizeSPX();
	}

	//____ _adjustZoom() ______________________________________________________

	bool BitmapDisplay::_adjustZoom()
	{
		if (m_bZoomToFit && m_pBitmap)
		{
			RectSPX canvas = _contentRect();
			SizeSPX destRectSize = _bitmapDisplayRect(m_scale);

			float widthScaling = canvas.w / (float)destRectSize.w;
			float heightScaling = canvas.h / (float)destRectSize.h;

			float zoom = std::min(widthScaling, heightScaling);
			if (zoom != m_zoom)
			{
				m_zoom = zoom;
				return true;
			}
		}

		return false;
	}


	//____ _adjustOffset() ____________________________________________________

	bool BitmapDisplay::_adjustOffset()
	{
		auto dispRect = _bitmapDisplayRect(m_scale);

		CoordSPX pos;

		if (dispRect.w < m_size.w)
		{
			switch (m_placement)
			{
				default:
				case Placement::West:
				case Placement::NorthWest:
				case Placement::SouthWest:
					pos.x = 0;
					break;

				case Placement::North:
				case Placement::Center:
				case Placement::South:
					pos.x = align((m_size.w - dispRect.w) / 2);
					break;

				case Placement::East:
				case Placement::NorthEast:
				case Placement::SouthEast:
					pos.x = m_size.w - dispRect.w;
					break;
			}
		}
		else
		{
			pos.x = dispRect.x;

			if (pos.x > 0)
				pos.x = 0;
			else if (pos.x + dispRect.w < m_size.w)
				pos.x = m_size.w - dispRect.w;
		}

		if (dispRect.h < m_size.h)
		{
			switch (m_placement)
			{
				default:
				case Placement::NorthWest:
				case Placement::North:
				case Placement::NorthEast:
					pos.y = 0;
					break;

				case Placement::West:
				case Placement::Center:
				case Placement::East:
					pos.y = align((m_size.h - dispRect.h) / 2);
					break;

				case Placement::SouthWest:
				case Placement::South:
				case Placement::SouthEast:
					pos.y = m_size.h - dispRect.h;
					break;
			}
		}
		else
		{
			pos.y = dispRect.y;

			if (pos.y > 0)
				pos.y = 0;
			else if (pos.y + dispRect.h < m_size.h)
				pos.y = m_size.h - dispRect.h;
		}

		if (pos != dispRect.pos())
		{
			Size size = m_pBitmap ? m_pBitmap->pointSize() : Size{0,0};
			
			if (pos.x != dispRect.x)
				m_offset.x = -(size.w * pos.x / dispRect.w);

			if (pos.y != dispRect.y)
				m_offset.y = -(size.h * pos.y / dispRect.h);

			return true;		// Offset modified
		}

		return false;			// Offset not modified
	}


} // namespace wg
