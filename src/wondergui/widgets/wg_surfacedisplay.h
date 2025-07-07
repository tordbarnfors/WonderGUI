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
#ifndef WG_SURFACEDISPLAY_DOT_H
#define WG_SURFACEDISPLAY_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_gfxdevice.h>

namespace wg
{

	class SurfaceDisplay;
	typedef	StrongPtr<SurfaceDisplay>	SurfaceDisplay_p;
	typedef	WeakPtr<SurfaceDisplay>		SurfaceDisplay_wp;





	//____ SurfaceDisplay ____________________________________________________________
	/**
	 **/

	class SurfaceDisplay : public Widget
	{

	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
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
			Surface_p		surface;
			Coord			surfaceOffset;
			Placement		surfacePlacement = Placement::Center;
			bool			tabLock = false;
			String			tooltip;
			float			zoom = 1.f;
			bool			zoomToFit = false;
		};

		//.____ Creation ______________________________________________________

		static SurfaceDisplay_p	create() { return SurfaceDisplay_p(new SurfaceDisplay()); }
		static SurfaceDisplay_p	create(const Blueprint& blueprint ) { return SurfaceDisplay_p(new SurfaceDisplay(blueprint)); }

		//.____ Identification ________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		void		setSurface(Surface* pSurface);
		Surface_p	surface() const { return m_pSurface; }

		void		setZoomToFit(bool activate);
		bool		zoomToFit() const { return m_bZoomToFit; }

		void		setZoom(float factor);
		float		zoom() const { return m_zoom; }

		void		setOffset(Coord offset);
		Coord		offset() const { return m_offset; }

		void		setUserZoomable(float min, float max );
		float		minUserZoom() const { return m_minZoom; }
		float		maxUserZoom() const { return m_maxZoom; }

		void		setSurfacePlacement(Placement placement);
		Placement	surfacePlacement() const { return m_placement;  }

		//		void		setGrid(int x, int y, HiColor color, float thickness);


		//.____ Internal ____________________________________________

		SizeSPX	_defaultSize(int scale) const override;
		spx		_matchingHeight(spx width, int scale) const override;
		spx		_matchingWidth(spx height, int scale) const override;

	protected:
		SurfaceDisplay() {};
		template<class BP> SurfaceDisplay(const BP& bp) : Widget(bp)
		{
			m_placement = bp.surfacePlacement;
			m_offset = bp.surfaceOffset;

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

			if (bp.surface)
				setSurface(bp.surface);
		}
		
		virtual ~SurfaceDisplay();

		void			_surfaceModified(int nRects, const RectSPX* pRects);
		RectSPX			_surfaceRectToCanvasRect(const RectSPX& surfaceRect);
		RectSPX			_surfaceDisplayRect(int scale) const;
		bool			_adjustZoom();
		bool			_adjustOffset();

		void			_resize(const SizeSPX& size, int scale) override;
		void			_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool			_alphaTest(const CoordSPX& ofs) override;

		Surface_p		m_pSurface;
		float			m_minZoom = 1.f;
		float			m_maxZoom = 1.f;
		float			m_zoom = 1.f;
		Coord			m_offset;			// Offset in points for surface displayed, in surface coordinates. {0,0} = surface centered in widget.
		bool			m_bZoomToFit = true;
		Placement		m_placement = Placement::Center;
		int				m_surfaceObserverId = 0;
	};



} // namespace wg
#endif //WG_SURFACEDISPLAY_DOT_H
