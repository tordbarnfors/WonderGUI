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
#ifndef WG_CANVASDISPLAY_DOT_H
#define WG_CANVASDISPLAY_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_skin.h>
#include <wg_gradient.h>
#include <wg_transitions.h>

namespace wg
{

	class CanvasDisplay;
	typedef	StrongPtr<CanvasDisplay>	CanvasDisplay_p;
	typedef	WeakPtr<CanvasDisplay>		CanvasDisplay_wp;

	class CanvasCapsule;
	typedef	StrongPtr<CanvasCapsule>	CanvasCapsule_p;
	typedef	WeakPtr<CanvasCapsule>		CanvasCapsule_wp;


	//____ CanvasDisplay ____________________________________________________________
	/**
	 * @brief Secondary display of the canvas of a CanvasCapsule.
	 *
	 **/


	class CanvasDisplay : public Widget
	{
		friend CanvasCapsule;

	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			BlendMode		blendMode		= BlendMode::Blend;
			CanvasCapsule_p	canvas;
			Size			defaultSize = { -1,-1 };
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			Placement		placement = Placement::Center;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			skinAroundCanvas = false;
			bool			stickyFocus = false;
			bool			tabLock = false;
			HiColor			tintColor		= HiColor::Undefined;
			Gradient		tintGradient;
			String			tooltip;
		};


		//.____ Creation __________________________________________

		static CanvasDisplay_p create() { return CanvasDisplay_p(new CanvasDisplay()); }
		static CanvasDisplay_p create( const Blueprint& blueprint ) { return CanvasDisplay_p(new CanvasDisplay( blueprint )); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		void		setCanvas(CanvasCapsule* pCanvas);
		CanvasCapsule_p	canvas() const { return m_pCanvas.rawPtr(); }

		void		setPlacement(Placement placement);
		Placement	placement() const { return m_placement; }

		void		setTintColor(HiColor color, ColorTransition* pTransition = nullptr);
		void		setTintGradient(const Gradient& gradient, ColorTransition* pTransition = nullptr);
		void		setBlendMode(BlendMode mode);


		//.____ Geometry ____________________________________________

		void		setDefaultSize( const Size& size );

	protected:
		CanvasDisplay();
		
		template<class BP> CanvasDisplay(const BP& bp) : Widget(bp)
		{
			m_defaultSize	= bp.defaultSize;
			m_placement		= bp.placement;

			m_blendMode		= bp.blendMode;
			
			if( bp.tintColor != HiColor::Undefined )
				m_tintColor	= bp.tintColor;

			m_gradient		= bp.tintGradient;
			m_bSkinAroundCanvas = bp.skinAroundCanvas;

			if (bp.canvas)
				setCanvas(bp.canvas);
		}
		
		virtual ~CanvasDisplay();

		SizeSPX		_defaultSize(int scale) const override;
		spx			_matchingHeight(spx width, int scale) const override;
		spx			_matchingWidth(spx height, int scale) const override;

		void		_update(int microPassed, int64_t microsecTimestamp) override;
		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		RectSPX		_canvasWindow(RectSPX window) const;
		bool		_alphaTest(const CoordSPX& ofs) override;


		// Called by CanvasCapsule

		void		_canvasReset( SizeSPX canvasSize );
		void		_canvasDirtyRect(const RectSPX);


	private:
		Size				m_defaultSize;
		CanvasCapsule_wp	m_pCanvas;
		SizeSPX				m_canvasSize;
		Placement			m_placement = Placement::Center;
		bool				m_bSkinAroundCanvas = false;

		HiColor				m_tintColor = HiColor::White;
		Gradient			m_gradient;
		BlendMode			m_blendMode = BlendMode::Blend;

		// Transitions

		HiColor				m_startTintColor;
		HiColor				m_endTintColor;

		Gradient			m_startGradient;
		Gradient			m_endGradient;

		ColorTransition_p	m_pTintColorTransition;
		ColorTransition_p	m_pGradientTransition;

		int					m_tintColorTransitionProgress = 0;
		int					m_gradientTransitionProgress = 0;
	};



} // namespace wg
#endif //WG_CANVASDISPLAY_DOT_H
