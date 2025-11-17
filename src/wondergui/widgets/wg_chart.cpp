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

#include	<wg_chart.h>
#include	<wg_gfxdevice.h>
#include	<wg_base.h>

#include	<cstring>

namespace wg
{

	const TypeInfo Chart::TYPEINFO = { "Chart", &Widget::TYPEINFO };


	//____ constructor ____________________________________________________________

	Chart::Chart() : xLines(this), yLines(this), glow(this),
		m_displaySkin(this), m_labelSkin(this)
	{
	}

	//____ destructor _____________________________________________________________

	Chart::~Chart()
	{
		// Remove TextItems from TextLayout

		//TODO: This here and in didAddEntries, didMoveEntries, willEraseEntries could
		// be handled with constructors, move constructors and destructors in GridLine instead.

		TextLayout* pDefaultLayout = m_pTextLayout ? m_pTextLayout : Base::defaultTextLayout();

		for( auto& line : xLines )
		{
			TextLayout* pLayout = line.m_pTextLayout ? line.m_pTextLayout.rawPtr() : pDefaultLayout;
			pLayout->removeText(&line);
		}

		for (auto& line : yLines)
		{
			TextLayout* pLayout = line.m_pTextLayout ? line.m_pTextLayout.rawPtr() : pDefaultLayout;
			pLayout->removeText(&line);
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Chart::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDisplayRange() __________________________________________________

	void Chart::setDisplayRange(float ceiling, float floor, ValueTransition * pTransition )
	{
		if (ceiling == m_displayCeiling && floor == m_displayFloor)
			return;

		if( pTransition )
		{
			if( !m_pRangeTransition )
				_startReceiveUpdates();

			m_pRangeTransition = pTransition;
			m_rangeTransitionProgress = 0;

			m_endDisplayFloor = floor;
			m_endDisplayCeiling = ceiling;

			m_startDisplayFloor = m_displayFloor;
			m_startDisplayCeiling = m_displayCeiling;
		}
		else
		{
			_stopRangeTransition();

			m_displayCeiling = ceiling;
			m_displayFloor = floor;
		}

		_fullRefreshOfChart();
		_repositionAllLabels();
	}

	//____ _stopRangeTransition() ________________________________________________

	void Chart::_stopRangeTransition()
	{
		if( m_pRangeTransition )
		{
			_stopReceiveUpdates();
			m_pRangeTransition = nullptr;
			m_rangeTransitionProgress = 0;
		}
	}


	//____ _update() __________________________________________________________

	void Chart::_update(int microPassed, int64_t microsecTimestamp)
	{
		if( m_pRangeTransition )
		{
			int timestamp = m_rangeTransitionProgress + microPassed;

			if (timestamp >= m_pRangeTransition->duration())
			{
				m_displayFloor = m_endDisplayFloor;
				m_displayCeiling = m_endDisplayCeiling;
				_stopRangeTransition();
			}
			else
			{
				m_rangeTransitionProgress = timestamp;
				m_displayFloor = m_pRangeTransition->snapshot(timestamp, m_startDisplayFloor, m_endDisplayFloor);
				m_displayCeiling = m_pRangeTransition->snapshot(timestamp, m_startDisplayCeiling, m_endDisplayCeiling);
			}

			_fullRefreshOfChart();
			_repositionAllLabels();
		}

		glow._update(microPassed);
	}


	//____ _render() ____________________________________________________________

	void Chart::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Widget::_render(pDevice, _canvas, _window);

		RectSPX graphCanvas = m_chartCanvas + _canvas.pos();

		if (!m_displaySkin.isEmpty())
		{
			if (m_bOpaqueChart)
			{
				PatchesSPX patches;

				patches.push(pDevice->clipListSize(), pDevice->clipList() );
				patches.sub(graphCanvas);

				pDevice->pushClipList(patches.size(), patches.begin());
				m_displaySkin.render(pDevice, graphCanvas + m_displaySkin.contentBorder(m_scale, m_state), m_scale, m_state);
				pDevice->popClipList();
			}
			else
				m_displaySkin.render(pDevice, graphCanvas + m_displaySkin.contentBorder(m_scale, m_state), m_scale, m_state);
		}

		float	rangeMin = std::min(m_displayCeiling, m_displayFloor);
		float	rangeMax = std::max(m_displayCeiling, m_displayFloor);

		// Render grid lines (if at bottom)

		if( !m_bGridLinesOnTop )
		_renderGridLines(pDevice, graphCanvas, rangeMin, rangeMax);

		// Render graphs

		if (glow.isActive())
		{
			if (!m_pChartCanvas)
			{
				SizeI chartSize = graphCanvas.size() / 64;

				m_pChartCanvas = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
					_.size = chartSize,
					_.format = PixelFormat::BGRA_8,
					_.canvas = true));
			}

			pDevice->beginCanvasUpdate(m_pChartCanvas);
			pDevice->setBlendMode(BlendMode::Replace);
			pDevice->fill(HiColor::Transparent);
			pDevice->setBlendMode(BlendMode::Blend);
			_renderCharts( pDevice, graphCanvas.size() );
			pDevice->endCanvasUpdate();

			glow._render(pDevice, graphCanvas, m_pChartCanvas, graphCanvas);

			pDevice->setBlitSource(m_pChartCanvas);
			pDevice->stretchBlit(graphCanvas);
		}
		else
			_renderCharts(pDevice,graphCanvas);

		// Render grid lines (if on top)

		if (m_bGridLinesOnTop)
			_renderGridLines(pDevice, graphCanvas, rangeMin, rangeMax);


		// Render labels

		for (auto& line : xLines)
		{
			if (line.m_bVisible && line.m_value >= rangeMin && line.m_value <= rangeMax && !line.m_label.isEmpty())
			{
				RectSPX geo = line.m_labelGeo + _canvas.pos();

				SkinSlot* pSkin = line.m_labelSkin.isEmpty() ? &m_labelSkin : &line.m_labelSkin;

				if (!pSkin->isEmpty())
				{
					pSkin->render(pDevice, geo, m_scale, m_state, line.m_value);
					geo = pSkin->contentRect(geo, m_scale, m_state);
				}

				TextLayout* pLayout = line.m_pTextLayout ? line.m_pTextLayout : (m_pTextLayout ? m_pTextLayout : Base::defaultTextLayout());

				pLayout->render(&line, pDevice, geo);
			}
		}

		for (auto& line : yLines)
		{
			if (line.m_bVisible && !line.m_label.isEmpty())
			{
				RectSPX geo = line.m_labelGeo + _canvas.pos();

				SkinSlot* pSkin = line.m_labelSkin.isEmpty() ? &m_labelSkin : &line.m_labelSkin;

				if (!pSkin->isEmpty())
				{
					pSkin->render(pDevice, geo, m_scale, m_state, line.m_value);
					geo = pSkin->contentRect(geo, m_scale, m_state);
				}

				TextLayout* pLayout = line.m_pTextLayout ? line.m_pTextLayout : (m_pTextLayout ? m_pTextLayout : Base::defaultTextLayout());

				pLayout->render(&line, pDevice, geo);
			}
		}



	}

	//____ _renderGridLines() ________________________________________________________

	void Chart::_renderGridLines(GfxDevice* pDevice, const RectSPX& canvas, float rangeMin, float rangeMax)
	{
		// Render grid xLines

		{
			for (auto& line : xLines)
			{
				if (line.m_bVisible && line.m_value >= rangeMin && line.m_value <= rangeMax)
				{
					float valueFactor = m_chartCanvas.h / (m_displayFloor - m_displayCeiling);

					CoordSPX pos = canvas.pos();
					pos.y += (line.m_value - m_displayCeiling) * valueFactor;

					HiColor color = line.m_color == HiColor::Undefined ? m_gridColor : line.m_color;
					pts thickness = line.m_thickness > 0 ? line.m_thickness : m_gridThickness;

					pDevice->drawLine(pos, Direction::Right, canvas.w, color, thickness * m_scale);
				}
			}
		}

		// Render grid yLines

		{
			for (auto& line : yLines)
			{
				if (line.m_bVisible)
				{
					CoordSPX pos = canvas.pos();
					pos.x += line.m_value * canvas.w;

					HiColor color = line.m_color == HiColor::Undefined ? m_gridColor : line.m_color;
					pts thickness = line.m_thickness > 0 ? line.m_thickness : m_gridThickness;

					pDevice->drawLine(pos, Direction::Down, canvas.h, color, thickness * m_scale);
				}
			}
		}
	}


	//____ _requestRenderChartArea() ____________________________________________

	void Chart::_requestRenderChartArea()
	{
		_requestRender(m_chartCanvas);
	}


	//____ _didAddEntries() ___________________________________________________

	void Chart::_didAddEntries(GridLine* pEntry, int nb)
	{
		// Add new TextItems to TextLayout

		TextLayout* pDefaultLayout = m_pTextLayout ? m_pTextLayout : Base::defaultTextLayout();

		for (int i = 0; i < nb; i++)
		{
			pEntry[i].m_pDisplay = this;
			
			TextLayout* pLayout = pEntry[i].m_pTextLayout ? pEntry[i].m_pTextLayout.rawPtr() : pDefaultLayout;
			pLayout->addText(&pEntry[i]);
		}

		//

		_recalcChartCanvas();
		_repositionAllLabels();
		_requestRender();
	}

	//____ _didMoveEntries() __________________________________________________

	void Chart::_didMoveEntries(GridLine* pFrom, GridLine* pTo, int nb)
	{
		// Notify TextLayout of moved TextItems

		TextLayout* pDefaultLayout = m_pTextLayout ? m_pTextLayout : Base::defaultTextLayout();

		for (int i = 0; i < nb; i++)
		{
			TextLayout* pLayout = pTo[i].m_pTextLayout ? pTo[i].m_pTextLayout.rawPtr() : pDefaultLayout;
			pLayout->didMoveText(&pFrom[i], &pTo[i]);
		}

		// Re-render in case 

		_requestRender();
	}

	//____ _willEraseEntries() ________________________________________________

	void Chart::_willEraseEntries(GridLine* pEntry, int nb)
	{
		// Remove TextItems from TextLayout

		TextLayout* pDefaultLayout = m_pTextLayout ? m_pTextLayout : Base::defaultTextLayout();

		for (int i = 0; i < nb; i++)
		{
			TextLayout* pLayout = pEntry[i].m_pTextLayout ? pEntry[i].m_pTextLayout.rawPtr() : pDefaultLayout;
			pLayout->removeText(&pEntry[i]);
		}

		//

		if (_recalcChartCanvas())
			_repositionAllLabels();

		_requestRender();
	}

	//____ _recalcGraphCanvas() _______________________________________________

	bool Chart::_recalcChartCanvas()
	{
		RectSPX contentGeo = m_skin.isEmpty() ? RectSPX(0,0,m_size) : m_skin.contentRect(RectSPX(0,0,m_size),m_scale, m_state);

		BorderSPX margin;

		if (!m_displaySkin.isEmpty())
			margin = m_displaySkin.contentBorder(m_scale, m_state);



		// Space needed left of graph for left labels, we include all labels, including
		// the invisible and out of range ones se we don't need to adjust if they pop in.

		for (auto& line : xLines)
		{
			if (!line.m_label.isEmpty())
			{
				CoordSPX ofs = _sideLabelOffset(&line);

				if (line.m_bLabelAtEnd)
				{
					ofs.x += line.m_labelGeo.size().w;

					if (ofs.x > margin.right)
						margin.right = ofs.x;
				}
				else
				{
					ofs.x *= -1;

					if (ofs.x > margin.left)
						margin.left = ofs.x;
				}
			}
		}

		// Space needed below graph for bottom labels, we include all labels, including
		// the invisible ones so we don't need to adjust when they are made visible.

		for (auto& line : yLines)
		{
			if (!line.m_label.isEmpty())
			{
				CoordSPX ofs = _topBottomLabelOffset(&line);

				if (line.m_bLabelAtEnd)
				{
					ofs.y *= -1;

					if (ofs.y > margin.top)
						margin.top = ofs.y;
				}
				else
				{
					ofs.y += line.m_labelGeo.size().h;

					if (ofs.y > margin.bottom)
						margin.bottom = ofs.y;
				}
			}
		}


		
		// Adjust top and bottom margins so no side labels sticks out above/below canvas.

		float	rangeMin = std::min(m_displayCeiling, m_displayFloor);
		float	rangeMax = std::max(m_displayCeiling, m_displayFloor);

		float valueFactor = contentGeo.h / (m_displayFloor - m_displayCeiling);

		if (!xLines.isEmpty())
		{
			// Find top & bottom line with label.

			GridLine* pTop = nullptr;
			GridLine* pBottom = nullptr;

			for (auto& line : xLines)
			{
				if (line.m_bVisible && !line.m_label.isEmpty() && line.m_value >= rangeMin && line.m_value <= rangeMax )
				{
					if (pTop == nullptr || line.m_value > pTop->m_value)
						pTop = &line;

					if (pBottom == nullptr || line.m_value < pBottom->m_value)
						pBottom = &line;

				}
			}

			if (m_displayFloor > m_displayCeiling)
				std::swap(pTop, pBottom);

			if (pTop)		// If we have top we also have bottom.
			{
				spx topLabelOfs = _sideLabelOffset(pTop).y;
				spx bottomLabelOfs = _sideLabelOffset(pBottom).y + pBottom->m_labelGeo.h;


				bool bModified;
				do {
					bModified = false;

					spx canvasHeight = contentGeo.h - margin.top - margin.bottom;

					spx topLineOfs = (pTop->m_value - m_displayCeiling) * (canvasHeight / (m_displayFloor - m_displayCeiling));
					if (margin.top < -(topLineOfs + topLabelOfs) )
					{
						margin.top = -(topLineOfs + topLabelOfs);
						bModified = true;
					}

					spx bottomLineOfs = (pBottom->m_value - m_displayCeiling) * (canvasHeight / (m_displayFloor - m_displayCeiling));
					if (margin.bottom < (bottomLineOfs + bottomLabelOfs ) - canvasHeight )
					{
						margin.bottom = (bottomLineOfs + bottomLabelOfs) - canvasHeight;
						bModified = true;
					}

				} while (bModified == true);

			}
		}

		// Adjust side margins so no bottom labels sticks out left/right of canvas.

		if (!yLines.isEmpty())
		{
			// Find leftmost and rightmost line with label.

			GridLine* pLeftmost = nullptr;
			GridLine* pRightmost = nullptr;

			for (auto& line : yLines)
			{
				if (line.m_bVisible && !line.m_label.isEmpty() && line.m_value >= 0.f && line.m_value <= 1.f)
				{
					if (pLeftmost == nullptr || line.m_value < pLeftmost->m_value)
						pLeftmost = &line;

					if (pRightmost == nullptr || line.m_value > pRightmost->m_value)
						pRightmost = &line;

				}
			}

			if (pLeftmost)		// If we have leftmost we also have rightmost.
			{
				spx leftLabelOfs = _topBottomLabelOffset(pLeftmost).x;
				spx rightLabelOfs = _topBottomLabelOffset(pRightmost).x + pRightmost->m_labelGeo.w;

				bool bModified;
				do {
					bModified = false;

					spx canvasWidth = contentGeo.w - margin.left - margin.right;

					spx leftLineOfs = pLeftmost->m_value * canvasWidth;
					if (margin.left < -(leftLineOfs + leftLabelOfs))
					{
						margin.left = -(leftLineOfs + leftLabelOfs);
						bModified = true;
					}

					spx rightLineOfs = pRightmost->m_value * canvasWidth;
					if (margin.right < (rightLineOfs + rightLabelOfs) - canvasWidth)
					{
						margin.right = (rightLineOfs + rightLabelOfs) - canvasWidth;
						bModified = true;
					}

				} while (bModified == true);

			}
		}


		// Finish up

		RectSPX graphCanvas = Util::alignDown(contentGeo - margin);

		if (graphCanvas == m_chartCanvas)
			return false;

		m_chartCanvas = graphCanvas;
		return true;
	}

	//____ _repositionAllLabels() _____________________________________________

	void Chart::_repositionAllLabels()
	{
		float	rangeMin = std::min(m_displayCeiling, m_displayFloor);
		float	rangeMax = std::max(m_displayCeiling, m_displayFloor);

		float valueFactor = m_chartCanvas.h / (m_displayFloor - m_displayCeiling);

		for (auto& line : xLines)
		{
			if (line.m_bVisible && line.m_value >= rangeMin && line.m_value <= rangeMax)
			{
				CoordSPX pos = line.m_bLabelAtEnd ? m_chartCanvas.topRight() : m_chartCanvas.topLeft();
				pos.y += (line.m_value - m_displayCeiling) * valueFactor;

				pos += _sideLabelOffset(&line);
				line.m_labelGeo.setPos(Util::align(pos));
			}
		}

		for (auto& line : yLines)
		{
			if (line.m_bVisible)
			{
				CoordSPX pos = line.m_bLabelAtEnd ? m_chartCanvas.topLeft() : m_chartCanvas.bottomLeft();
				pos.x += line.m_value * m_chartCanvas.w;

				pos += _topBottomLabelOffset(&line); 
				line.m_labelGeo.setPos(Util::align(pos));
			}
		}
	}


	//____ _sideLabelOffset() ___________________________________________________

	CoordSPX Chart::_sideLabelOffset(GridLine* pLine)
	{
		spx spacing = Util::ptsToSpx(pLine->m_bLabelAtEnd ? m_rightLabelSpacing : m_leftLabelSpacing, m_scale);

		Placement defaultPlacement = pLine->m_bLabelAtEnd ? m_rightLabelPlacement : m_leftLabelPlacement;
		Placement placement = pLine->m_labelPlacement != Placement::Undefined ? pLine->m_labelPlacement : defaultPlacement;
		CoordSPX pos = Util::placeRectAroundCoord(placement, { 0,0 }, pLine->m_labelGeo.size() + SizeSPX(2 * spacing, 0));
		pos += Util::ptsToSpx(pLine->m_labelAdjustment, m_scale);
		pos.x += spacing;
		return pos;
	}

	//____ _topBottomLabelOffset() ___________________________________________________

	CoordSPX Chart::_topBottomLabelOffset(GridLine* pLine)
	{
		spx spacing = Util::ptsToSpx(pLine->m_bLabelAtEnd ? m_topLabelSpacing : m_bottomLabelSpacing, m_scale);

		Placement defaultPlacement = pLine->m_bLabelAtEnd ? m_topLabelPlacement : m_bottomLabelPlacement;
		Placement placement = pLine->m_labelPlacement != Placement::Undefined ? pLine->m_labelPlacement : defaultPlacement;
		CoordSPX pos = Util::placeRectAroundCoord(placement, { 0,0 }, pLine->m_labelGeo.size() + SizeSPX(0, 2 * spacing));
		pos += Util::ptsToSpx(pLine->m_labelAdjustment, m_scale);
		pos.y += spacing;
		return pos;
	}

	//____ _resize() __________________________________________________________

	void Chart::_resize(const SizeSPX& size, int scale)
	{
		if( size != m_size )
			m_pChartCanvas = nullptr;

		int oldScale = m_scale;
		Widget::_resize(size, scale);

		// Resize labels if scale has changed

		if (scale != oldScale)
		{
			for (auto& gridLine : xLines)
				gridLine._resizeLabel(scale, oldScale);

			for (auto& gridLine : yLines)
				gridLine._resizeLabel(scale, oldScale);
		}

		_recalcChartCanvas();
		_repositionAllLabels();

		_fullRefreshOfChart();
	}

	//____ _textSize() ________________________________________________________

	SizeSPX GridLine::_textSize() const
	{
		if (!m_labelSkin.isEmpty())
			return m_labelGeo.size() - m_labelSkin.contentBorderSize(m_pDisplay->_scale());
		else if( !m_pDisplay->m_labelSkin.isEmpty() )
			return m_labelGeo.size() - m_pDisplay->m_labelSkin.contentBorderSize(m_pDisplay->_scale());
		else
			return m_labelGeo.size();
	}

	//____ _textState() _______________________________________________________

	State GridLine::_textState() const
	{
		return m_pDisplay->m_state;
	}

	//____ _textStyle() _______________________________________________________

	TextStyle* GridLine::_textStyle() const
	{
		if (m_pTextStyle)
			return m_pTextStyle;
		else if (m_pDisplay->m_pTextStyle)
			return m_pDisplay->m_pTextStyle;

		return Base::defaultStyle();
	}

	//____ _textBegin() _______________________________________________________

	const Char* GridLine::_textBegin() const
	{
		return m_label.chars();
	}

	//____ _textLength() ______________________________________________________

	int GridLine::_textLength() const
	{
		return m_label.length();
	}

	//____ _caretVisible() ____________________________________________________

	bool GridLine::_caretVisible() const
	{
		return false;
	}

	//____ _caretOffset() _____________________________________________________

	int GridLine::_caretOffset() const
	{
		return 0;
	}

	//____ _selectedText() ____________________________________________________

	std::tuple<int, int> GridLine::_selectedText() const
	{
		return std::make_tuple(0, 0);
	}

	//____ _mapperRequestRender() _____________________________________________

	void GridLine::_mapperRequestRender()
	{
		RectSPX textGeo = m_labelGeo;

		if (!m_labelSkin.isEmpty() )
			textGeo -= m_labelSkin.contentBorder(m_pDisplay->m_scale, m_pDisplay->m_state);
		else if (!m_pDisplay->m_labelSkin.isEmpty())
			textGeo -= m_pDisplay->m_labelSkin.contentBorder(m_pDisplay->m_scale, m_pDisplay->m_state);

		m_pDisplay->_requestRender(textGeo);
	}

	void GridLine::_mapperRequestRender(const RectSPX& rect)
	{
		CoordSPX textPos = m_labelGeo.pos();

		if (!m_labelSkin.isEmpty())
			textPos += m_labelSkin.contentOfs(m_pDisplay->m_scale, m_pDisplay->m_state);
		else if (!m_pDisplay->m_labelSkin.isEmpty())
			textPos += m_pDisplay->m_labelSkin.contentOfs(m_pDisplay->m_scale, m_pDisplay->m_state);


		m_pDisplay->_requestRender(rect + textPos);
	}

	//____ _mapperRequestResize() _____________________________________________

	void GridLine::_mapperRequestResize()
	{
		_resizeLabel(m_pDisplay->m_scale, m_pDisplay->m_scale);
	}

	//____ _scale() ___________________________________________________________

	int GridLine::_scale() const
	{
		return m_pDisplay->m_scale;
	}

	//____ _resizeLabel() _____________________________________________________

	void GridLine::_resizeLabel(int scale, int oldScale)
	{
		TextLayout* pLayout			= m_pTextLayout ? m_pTextLayout : (m_pDisplay->m_pTextLayout ? m_pDisplay->m_pTextLayout : Base::defaultTextLayout() );
		SkinSlot *	pLabelSkin		= m_labelSkin.isEmpty() ? &m_pDisplay->m_labelSkin : &m_labelSkin;
		SizeSPX		oldLabelPadding;
		SizeSPX		newLabelPadding;

		if( !pLabelSkin->isEmpty() )
		{
			oldLabelPadding = pLabelSkin->contentBorderSize(oldScale);
			newLabelPadding = pLabelSkin->contentBorderSize(scale);
		}

		SizeSPX newSize = pLayout->defaultSize(this, scale);
		SizeSPX oldSize;
		
		if( !m_labelGeo.size().isEmpty() )
			oldSize = m_labelGeo.size() - oldLabelPadding;

		m_labelGeo.setSize(newSize + newLabelPadding);
		pLayout->onResized(this, newSize, oldSize, scale, oldScale);
	}

	//____ _skinValue() _______________________________________________________

	float GridLine::_skinValue(const SkinSlot* pSlot) const
	{
		return 1.f;
	}

	//____ _skinValue2() ______________________________________________________

	float GridLine::_skinValue2(const SkinSlot* pSlot) const
	{
		return -1.f;
	}

	//____ _skinState() _______________________________________________________

	State GridLine::_skinState(const SkinSlot* pSlot) const
	{
		return m_pDisplay->state();
	}

	//____ _skinSize() ________________________________________________________

	SizeSPX GridLine::_skinSize(const SkinSlot* pSlot) const
	{
		return m_labelGeo.size();
	}

	//____ _skinRequestRender() _______________________________________________

	void GridLine::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		m_pDisplay->_requestRender(rect + m_labelGeo.pos());
	}
}
