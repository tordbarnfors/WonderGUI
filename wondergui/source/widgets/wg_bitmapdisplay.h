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
#ifndef WG_BITMAPDISPLAY_DOT_H
#define WG_BITMAPDISPLAY_DOT_H
#pragma once


#include <wg_widget.h>
#include <wax_device.h>

namespace wg
{

	class BitmapDisplay;
	typedef	StrongPtr<BitmapDisplay>	BitmapDisplay_p;
	typedef	WeakPtr<BitmapDisplay>		BitmapDisplay_wp;





	//____ BitmapDisplay ____________________________________________________________
	/**
	 **/

	class BitmapDisplay : public Widget
	{

	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Bitmap_p		bitmap;
			Coord			bitmapOffset;
			Placement		bitmapPlacement = Placement::Center;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			float			maxUserZoom = 1.f;
			float			minUserZoom = 1.f;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			std::string			tooltip;
			Object_p		userData;
			float			zoom = 1.f;
			bool			zoomToFit = false;
		};

		//.____ Creation ______________________________________________________

		static BitmapDisplay_p	create() { return BitmapDisplay_p(new BitmapDisplay()); }
		static BitmapDisplay_p	create(const Blueprint& blueprint ) { return BitmapDisplay_p(new BitmapDisplay(blueprint)); }

		//.____ Identification ________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		void		setBitmap(Bitmap* pBitmap);
		Bitmap_p	bitmap() const { return m_pBitmap; }

		void		setZoomToFit(bool activate);
		bool		zoomToFit() const { return m_bZoomToFit; }

		void		setZoom(float factor);
		float		zoom() const { return m_zoom; }

		void		setOffset(Coord offset);
		Coord		offset() const { return m_offset; }

		void		setUserZoomable(float min, float max );
		float		minUserZoom() const { return m_minZoom; }
		float		maxUserZoom() const { return m_maxZoom; }

		void		setBitmapPlacement(Placement placement);
		Placement	bitmapPlacement() const { return m_placement;  }

		//		void		setGrid(int x, int y, Color color, float thickness);


		//.____ Internal ____________________________________________

		SizeSPX	_defaultSize(int scale) const override;
		spx		_matchingHeight(spx width, int scale) const override;
		spx		_matchingWidth(spx height, int scale) const override;

	protected:
		BitmapDisplay() {};
		template<class BP> BitmapDisplay(const BP& bp) : Widget(bp)
		{
			m_placement = bp.bitmapPlacement;
			m_offset = bp.bitmapOffset;

			if (bp.zoomToFit)
			{
				m_bZoomToFit = true;
			}
			else
			{
				m_bZoomToFit = false;
				m_zoom = bp.zoom;

				m_minZoom = bp.minUserZoom;
				m_maxZoom = bp.maxUserZoom;
			}

			if (bp.bitmap)
				setBitmap(bp.bitmap);
		}
		
		virtual ~BitmapDisplay();

		void			_bitmapModified(int nRects, const RectI* pRects);
		RectSPX			_bitmapRectToCanvasRect(const RectSPX& bitmapRect);
		RectSPX			_bitmapDisplayRect(int scale) const;
		bool			_adjustZoom();
		bool			_adjustOffset();

		void			_resize(const SizeSPX& size, int scale) override;
		void			_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool			_alphaTest(const CoordSPX& ofs) override;

		Bitmap_p		m_pBitmap;
		float			m_minZoom = 1.f;
		float			m_maxZoom = 1.f;
		float			m_zoom = 1.f;
		Coord			m_offset;			// Offset in points for bitmap displayed, in bitmap coordinates. {0,0} = bitmap centered in widget.
		bool			m_bZoomToFit = true;
		Placement		m_placement = Placement::Center;
		int				m_bitmapObserverId = 0;
	};



} // namespace wg
#endif //WG_BITMAPDISPLAY_DOT_H
