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

#ifndef	WG_CHART_DOT_H
#define WG_CHART_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_dynamicvector.h>
#include <wg_transitions.h>
#include <wg_gradient.h>
#include <wg_dynamictext.h>
#include <wg_glow.h>

namespace wg
{
class Chart;



	class GridLine : protected TextItem, protected SkinSlot::Holder
	{
		friend class Chart;
	public:

		struct Blueprint
		{
			HiColor			color = HiColor::Undefined;
			
			String			label;
			Coord			labelAdjustment;
			bool			labelAtEnd = false;
			Placement		labelPlacement = Placement::Undefined;
			Skin_p			labelSkin;

			float			pos = 0.f;

			TextLayout_p	textLayout;
			TextStyle_p		textStyle;

			pts				thickness = 0;					// 0 = Undefined.
			bool			visible = true;
		};


		GridLine(const Blueprint& bp)
			: m_labelSkin(this),
			m_value(bp.pos),
			m_color(bp.color),
			m_thickness(bp.thickness),
			m_bVisible(bp.visible),
			m_label(bp.label),
			m_labelAdjustment(bp.labelAdjustment),
			m_bLabelAtEnd(bp.labelAtEnd),
			m_pTextLayout(bp.textLayout),
			m_labelPlacement(bp.labelPlacement),
			m_pTextStyle(bp.textStyle)
		{
			m_labelSkin.set(bp.labelSkin);
		};

		GridLine()
			: m_labelSkin(this)
		{
			// This constructor is only used to expand vector at insertion of multiple elements.
			// We don't care about setting default values, they should be overwritten immediately anyway.
			
		};

		virtual ~GridLine() = default;

	protected:

		SizeSPX			_textSize() const override;
		State			_textState() const override;
		TextStyle*		_textStyle() const override;

		const Char*		_textBegin() const override;
		int 			_textLength() const override;

		bool			_caretVisible() const override;
		int				_caretOffset() const override;
		std::tuple<int, int> _selectedText() const override;		// Begin/end of selection

		void			_mapperRequestRender() override;
		void			_mapperRequestRender(const RectSPX& rect) override;
		void			_mapperRequestResize() override;

		int				_scale() const override;

		float			_skinValue(const SkinSlot* pSlot) const override;
		float			_skinValue2(const SkinSlot* pSlot) const override;
		State			_skinState(const SkinSlot* pSlot) const override;

		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;

		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;



		//

		void			_resizeLabel(int scale, int oldScale );


		Chart*			m_pDisplay = nullptr;

		RectSPX			m_labelGeo;

		float			m_value;
		HiColor			m_color;
		pts				m_thickness;

		bool			m_bVisible;
		bool			m_bLabelAtEnd;

		String			m_label;
		Coord			m_labelAdjustment;
		TextLayout_p	m_pTextLayout;
		Placement		m_labelPlacement = Placement::Undefined;	// Defaults to West or South depending on x- or y-line.
		TextStyle_p		m_pTextStyle;
		SkinSlot		m_labelSkin;
	};


	class Chart;
	typedef	StrongPtr<Chart>	Chart_p;
	typedef	WeakPtr<Chart>		Chart_wp;

	class Chart : public Widget, private DynamicVector<GridLine>::Holder
	{
		friend class GridLine;
	public:


		//.____ Components _______________________________________

		DynamicVector<GridLine>	xLines;
		DynamicVector<GridLine>	yLines;
		Glow					glow;

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance _______________________________________________

		void	setDisplayRange(float ceiling, float floor, ValueTransition * pTransition = nullptr );

		float	displayCeiling() const { return m_displayCeiling;  }
		float	displayFloor() const { return m_displayFloor; }


		//.____ Internal _________________________________________________

	protected:
		Chart();
		
		template<class BP> Chart( const BP& bp ) : xLines(this), yLines(this), glow(this),
			m_displaySkin(this), m_labelSkin(this), Widget(bp)
		{
			m_displayCeiling = bp.displayCeiling;
			m_displayFloor = bp.displayFloor;
			m_displaySkin.set(bp.displaySkin);

			m_gridColor		= bp.gridColor;
			m_gridThickness = bp.gridThickness;
			m_pTextStyle	= bp.textStyle;
			m_labelSkin.set(bp.labelSkin);

			m_leftLabelPlacement = bp.leftLabelPlacement;
			m_rightLabelPlacement = bp.rightLabelPlacement;
			m_topLabelPlacement = bp.topLabelPlacement;
			m_bottomLabelPlacement = bp.bottomLabelPlacement;

			m_leftLabelSpacing = bp.leftLabelSpacing;
			m_rightLabelSpacing = bp.rightLabelSpacing;
			m_topLabelSpacing = bp.topLabelSpacing;
			m_bottomLabelSpacing = bp.bottomLabelSpacing;

			glow._initFromBlueprint(bp.glow);
		}

		virtual ~Chart();

		virtual void	_fullRefreshOfChart() = 0;
		virtual void	_renderCharts(GfxDevice* pDevice, const RectSPX& canvas) = 0;

		void			_resize(const SizeSPX& size, int scale) override;
		void			_update(int microPassed, int64_t microsecTimestamp) override;
		void			_render(GfxDevice* pDevice, const RectSPX& canvas, const RectSPX& window) override;

		void			_requestRenderChartArea();

		//

		void		_didAddEntries(GridLine* pEntry, int nb) override;
		void		_didMoveEntries(GridLine* pFrom, GridLine* pTo, int nb) override;
		void		_willEraseEntries(GridLine* pEntry, int nb) override;

		RectSPX			m_chartCanvas;

		float			m_displayCeiling = 0.f;
		float			m_displayFloor = 1.f;

	private:

		bool		_recalcChartCanvas();
		void		_repositionAllLabels();
		CoordSPX	_sideLabelOffset(GridLine* pLine);
		CoordSPX	_topBottomLabelOffset(GridLine* pLine);

		void		_stopRangeTransition();

		SkinSlot		m_displaySkin;

		HiColor			m_gridColor = Color::DarkGray;
		pts				m_gridThickness = 1;
		TextStyle_p		m_pTextStyle;
		TextLayout_p	m_pTextLayout;

		SkinSlot		m_labelSkin;
		Placement		m_leftLabelPlacement = Placement::West;
		Placement		m_rightLabelPlacement = Placement::East;
		Placement		m_topLabelPlacement = Placement::North;
		Placement		m_bottomLabelPlacement = Placement::South;
		pts				m_topLabelSpacing = 1;
		pts				m_bottomLabelSpacing = 1;
		pts				m_leftLabelSpacing = 4;
		pts				m_rightLabelSpacing = 4;

		Surface_p		m_pChartCanvas;

		ValueTransition_p	m_pRangeTransition;
		int					m_rangeTransitionProgress = 0;

		float			m_startDisplayCeiling = 0.f;
		float			m_endDisplayCeiling = 0.f;
		float			m_startDisplayFloor = 1.f;
		float			m_endDisplayFloor = 1.f;
	};




} // namespace wg
#endif //CHART_DOT_H
