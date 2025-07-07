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
/*
	KNOWN LIMITATIONS

	* Row skins are static, they don't react to states and can't be animated.
	
*/




#ifndef WG_TABLEPANEL_DOT_H
#define WG_TABLEPANEL_DOT_H
#pragma once


#include <wg_container.h>
#include <wg_slottable.h>
#include <wg_packlayout.h>
#include <wg_dynamicvector.h>

namespace wg
{

	class TablePanel;
	typedef	StrongPtr<TablePanel>	TablePanel_p;
	typedef	WeakPtr<TablePanel>		TablePanel_wp;


//____ TablePanelRow ______________________________________________________

	class TablePanelRow
	{
		friend class TablePanel;
	public:
		struct Blueprint
		{
			pts		maxHeight = 0;
			pts		minHeight = 0;
			bool	visible = true;
			float	weight = 1.f;
		};
	
	
		TablePanelRow() {};
		TablePanelRow( const Blueprint& bp ) : 
			m_bVisible(bp.visible), m_weight(bp.weight), 
			m_maxHeight(bp.maxHeight), m_minHeight(bp.minHeight) {}
		
		inline void		hide();
		inline void		unhide();

		inline void		setVisible(bool bVisible);
		inline bool		isVisible() const { return m_bVisible; }
		
		inline void		setWeight(float weight);
		inline float	weight() const { return m_weight; }
		
		inline void		setMinHeight(pts minHeight);
		inline pts		minHeight() const { return m_minHeight; }

		inline void		setMaxHeight(pts maxHeight);
		inline pts		maxHeight() const { return m_maxHeight; }

	protected:
		
		struct Cache
		{
			spx		defaultHeight = 0;
			spx		heightForColumnWidth = 0;		// Height needed for current column widths.
			spx		maxHeight = spx_max;
			spx		minHeight = 0;
		};

		pts				m_minHeight = 0;
		pts				m_maxHeight = 0;

		TablePanel*		m_pTable = nullptr;
		bool			m_bVisible = true;
		bool			m_bModified = false;
		float			m_weight = 1.f;
		spx				m_height = 0;
		Cache			m_cache;

	};

	//____ TablePanelColumn ______________________________________________________

	class TablePanelColumn
	{
		friend class TablePanel;
	public:

		struct Blueprint
		{
			pts		maxWidth = 0;
			pts		minWidth = 0;

			bool	visible = true;
			float	weight = 1.f;
		};

		TablePanelColumn() {};
		TablePanelColumn( const Blueprint& bp ) : m_bVisible(bp.visible), m_weight(bp.weight) {}
		
		inline void		hide();
		inline void		unhide();

		inline void		setVisible(bool bVisible);
		inline bool		isVisible() const { return m_bVisible; }
		
		inline void		setWeight(float weight);
		inline float	weight() const { return m_weight; }
	
		inline void		setMinWidth(pts width);
		inline pts		minWidth() const { return m_minWidth; }

		inline void		setMaxWidth(pts width);
		inline pts		maxWidth() const { return m_maxWidth; }

	protected:

		struct Cache
		{
			spx		defaultWidth = 0;
			spx		minWidth = 0;
			spx		maxWidth = spx_max;

		};

		pts				m_minWidth = 0;
		pts				m_maxWidth = 0;

		bool			m_bVisible = true;
		bool			m_bModified = false;
		float			m_weight = 1.f;

		TablePanel *	m_pTable = nullptr;
		spx				m_width = 0;
		
		Cache			m_cache;
	};



	//____ TablePanel _________________________________________________________

	class TablePanel : 	public Container, SlotTable::Holder, 
						protected DynamicVector<TablePanelRow>::Holder,
						protected DynamicVector<TablePanelColumn>::Holder
	{

		friend class TablePanelRow;
		friend class TablePanelColumn;
		
	public:

		using		row_iterator = DynamicVector<TablePanelRow>::iterator;
		using		column_iterator = DynamicVector<TablePanelColumn>::iterator;

		
		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			Object_p		baggage;

			PackLayout_p	columnLayout;
			int				columns = 2;

			pts				columnSpacing		= 0;
			pts				columnSpacingAfter	= 0;
			pts				columnSpacingBefore	= 0;

			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			MaskOp			maskOp = MaskOp::Recurse;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;

			PackLayout_p	rowLayout;
			int				rows = 2;

			Skin_p			rowSkin;
			Skin_p			rowSkin2;

			pts				rowSpacing			= 0;
			pts				rowSpacingAfter		= 0;
			pts				rowSpacingBefore	= 0;

			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;

		};
		
		//.____ Creation __________________________________________

		static TablePanel_p	create() { return TablePanel_p(new TablePanel()); }
		static TablePanel_p	create( const Blueprint& blueprint ) { return TablePanel_p(new TablePanel(blueprint)); }

		//.____ Components _______________________________________

		DynamicVector<TablePanelRow>	rows;
		DynamicVector<TablePanelColumn>	columns;
		SlotTable						slots;
		
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance _____________________________________________________
		
		void			setRowSkins(Skin* pSkin1, Skin* pSkin2 = nullptr);

		//.____ Layout ______________________________________________________________

		void			setRowLayout(PackLayout* pLayout);
		PackLayout_p	rowLayout() const { return m_pLayoutY; }

		void			setColumnLayout(PackLayout* pLayout);
		PackLayout_p	columnLayout() const { return m_pLayoutX; }

		void			setRowSpacing(pts between) { setRowSpacing(0, between, 0); }
		void			setRowSpacing(pts before, pts between, pts after);

		void			setColumnSpacing(pts between) { setColumnSpacing(0, between, 0); }
		void			setColumnSpacing(pts before, pts between, pts after);

		void			setRowWeight(int index, int amount, float weight);
		void			setRowWeight(row_iterator  beg, row_iterator end, float weight);
		void			setRowWeight(int index, int amount, std::initializer_list<float> weights);
		void			setRowWeight(row_iterator beg, row_iterator end, std::initializer_list<float> weights);
		
		void			setColumnWeight(int index, int amount, float weight);
		void			setColumnWeight(column_iterator  beg, column_iterator end, float weight);
		void			setColumnWeight(int index, int amount, std::initializer_list<float> weights);
		void			setColumnWeight(column_iterator beg, column_iterator end, std::initializer_list<float> weights);
		
		void			setMinRowHeight(int index, int amount, pts height);
		void			setMinRowHeight(row_iterator  beg, row_iterator end, pts height);
		void			setMinRowHeight(int index, int amount, std::initializer_list<float> heights);
		void			setMinRowHeight(row_iterator beg, row_iterator end, std::initializer_list<float> heights);

		void			setMaxRowHeight(int index, int amount, pts height);
		void			setMaxRowHeight(row_iterator  beg, row_iterator end, pts height);
		void			setMaxRowHeight(int index, int amount, std::initializer_list<float> heights);
		void			setMaxRowHeight(row_iterator beg, row_iterator end, std::initializer_list<float> heights);

		void			setMinColumnWidth(int index, int amount, pts width);
		void			setMinColumnWidth(column_iterator  beg, column_iterator end, pts width);
		void			setMinColumnWidth(int index, int amount, std::initializer_list<float> widths);
		void			setMinColumnWidth(column_iterator beg, column_iterator end, std::initializer_list<float> widths);

		void			setMaxColumnWidth(int index, int amount, pts width);
		void			setMaxColumnWidth(column_iterator  beg, column_iterator end, pts width);
		void			setMaxColumnWidth(int index, int amount, std::initializer_list<float> widths);
		void			setMaxColumnWidth(column_iterator beg, column_iterator end, std::initializer_list<float> widths);

		//.____ Content _________________________________________________________

		void			resize( int rows, int columns );

		void			hideRows(int index, int amount);
		void			hideRows(row_iterator beg, row_iterator end);

		void			unhideRows(int index, int amount);
		void			unhideRows(row_iterator beg, row_iterator end);

		void			hideColumns(int index, int amount);
		void			hideColumns(column_iterator beg, column_iterator end);

		void			unhideColumns(int index, int amount);
		void			unhideColumns(column_iterator beg, column_iterator end);

		//.____ Internal ______________________________________________________

		spx				_matchingHeight(spx width, int scale) const override;
		spx				_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;
		SizeSPX			_minSize(int scale) const override;
		SizeSPX			_maxSize(int scale) const override;

		Widget*			_findWidget(const CoordSPX& ofs, SearchMode mode) override;

		
	protected:
		TablePanel();
		template<class BP> TablePanel(const BP& bp) : Container(bp), rows(this), columns(this), slots(this,this)
		{
			slots._resize(bp.rows, bp.columns);
			rows._resize(bp.rows);
			columns._resize(bp.columns);

			// Set m_pTable for new columns and rows

			for (auto& row : rows)
				row.m_pTable = this;

			for (auto& column : columns)
				column.m_pTable = this;

			//

			m_nVisibleRows = bp.rows;
			m_nVisibleColumns = bp.columns;

			m_spacingX[0] = bp.columnSpacingBefore;
			m_spacingX[1] = bp.columnSpacing;
			m_spacingX[2] = bp.columnSpacingAfter;

			m_spacingY[0] = bp.rowSpacingBefore;
			m_spacingY[1] = bp.rowSpacing;
			m_spacingY[2] = bp.rowSpacingAfter;


			m_pLayoutX = bp.columnLayout;
			m_pLayoutY = bp.rowLayout;
			
			m_pRowSkins[0] = bp.rowSkin;
			m_pRowSkins[1] = bp.rowSkin2;
			
			_updateMinDefaultSize();
			_refreshOverflow();						// Our row skins might have overflow.
		}

		virtual ~TablePanel();

		// Overloaded from Container

		RectSPX			_slotGeo( const StaticSlot * pSlot ) const override;				///< Get the local position of a child.
		void			_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) override;

		bool			_isChildVisible( const StaticSlot * pSlot ) const override;

		void			_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override;
		void			_childRequestResize(StaticSlot * pSlot) override;

		Widget *		_prevChild(const StaticSlot * pSlot) const override;
		Widget *		_nextChild(const StaticSlot * pSlot) const override;

		void			_releaseChild(StaticSlot * pSlot) override;
		void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override;

		Widget *		_firstChild() const override;
		Widget *		_lastChild() const override;

		void			_firstSlotWithGeo( SlotWithGeo& package ) const override;
		void			_nextSlotWithGeo( SlotWithGeo& package ) const override;

		
		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		void			_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		void			_resize( const SizeSPX& size, int scale ) override;

		BorderSPX		_calcOverflow() override;

		bool			_alphaTest(const CoordSPX& ofs) override;

		// For DynamicVector holder
		
		void			_didAddEntries(TablePanelRow * pEntry, int nb) override;
		void			_didMoveEntries(TablePanelRow * pFrom, TablePanelRow * pTo, int nb) override;
		void			_willEraseEntries(TablePanelRow * pEntry, int nb) override;

		void			_didAddEntries(TablePanelColumn * pEntry, int nb) override;
		void			_didMoveEntries(TablePanelColumn * pFrom, TablePanelColumn * pTo, int nb) override;
		void			_willEraseEntries(TablePanelColumn * pEntry, int nb) override;
		
		// For TableSlot holder
		
		void			_refreshSlots(int ofs, Axis axis, int nSlots) override;
		Object*			_object() override;
		
		//
		
		void			_hideRows(TablePanelRow * pStart, int nb);
		void			_unhideRows(TablePanelRow * pStart, int nb);

		void			_hideColumns(TablePanelColumn * pStart, int nb);
		void			_unhideColumns(TablePanelColumn * pStart, int nb);

		void			_reweightRows(TablePanelRow * pEntry, int nb, float weight);
		void			_reweightRows(TablePanelRow * pEntry, int nb, const float * pWeights);

		void			_reweightColumns(TablePanelColumn * pEntry, int nb, float weight);
		void			_reweightColumns(TablePanelColumn * pEntry, int nb, const float * pWeights);

		void			_setMinHeights(TablePanelRow* pEntry, int nb, pts height);
		void			_setMinHeights(TablePanelRow* pEntry, int nb, const pts* pHeights);

		void			_setMaxHeights(TablePanelRow* pEntry, int nb, pts height);
		void			_setMaxHeights(TablePanelRow* pEntry, int nb, const pts* pHeights);

		void			_setMinWidths(TablePanelColumn* pEntry, int nb, pts width);
		void			_setMinWidths(TablePanelColumn* pEntry, int nb, const pts* pWidths);

		void			_setMaxWidths(TablePanelColumn* pEntry, int nb, pts height);
		void			_setMaxWidths(TablePanelColumn* pEntry, int nb, const pts* pWidths);


		//
		
		bool			_refreshColumns();
		bool			_refreshRows();
		
		bool			_refreshRowHeightForColumnWidth();
		
		void			_updateAllChildSizes();
		void			_updateModifiedChildSizes();
		
		bool			_refreshRowCache( int row, TablePanelRow::Cache& cache, int scale ) const;
		bool			_refreshColumnCache( int column, TablePanelColumn::Cache& cache, int scale ) const;
		
		SizeSPX			_sumOfPadding(int scale) const;

		void			_updateMinDefaultSize();
		
		void			_rowVisibilityChanged( int change );
		void			_columnVisibilityChanged( int change );

		void			_rowParamsChanged();
		void			_columnParamsChanged();

		int				_markedRow(const CoordSPX& ofs);


		SizeSPX			m_minSize;
		SizeSPX			m_defaultSize;
		
		PackLayout_p	m_pLayoutX;
		PackLayout_p	m_pLayoutY;
		
		pts				m_spacingX[3] = {0,0,0};
		pts				m_spacingY[3] = {0,0,0};
		
		Skin_p			m_pRowSkins[2];

		int				m_nVisibleColumns = 0;
		int				m_nVisibleRows = 0;

		bool			m_bChildrenWithOverflow = false;


	};


//____ TablePanelRow::hide() __________________________________________________

void TablePanelRow::hide()
{
	m_pTable->_hideRows(this, 1);
}

//____ TablePanelRow::unhide() __________________________________________________

void TablePanelRow::unhide()
{
	m_pTable->_unhideRows(this, 1);
}

//____ TablePanelRow::setVisible() __________________________________________________

void TablePanelRow::setVisible(bool bVisible)
{
	if (bVisible)
		m_pTable->_unhideRows(this, 1);
	else
		m_pTable->_hideRows(this, 1);
}

//____ TablePanelRow::setWeight() __________________________________________________

void TablePanelRow::setWeight(float weight)
{
	if (weight != m_weight)
		m_pTable->_reweightRows(this, 1, weight);
}

//____ TablePanelRow::setMinHeight() __________________________________________

void TablePanelRow::setMinHeight(pts height)
{
	if (height != m_minHeight)
		m_pTable->_setMinHeights(this, 1, height);
}

//____ TablePanelRow::setMaxHeight() __________________________________________

void TablePanelRow::setMaxHeight(pts height)
{
	if (height != m_maxHeight)
		m_pTable->_setMaxHeights(this, 1, height);
}

//____ TablePanelColumn::hide() __________________________________________________

void TablePanelColumn::hide()
{
	m_pTable->_hideColumns(this, 1);
}

//____ TablePanelColumn::unhide() __________________________________________________

void TablePanelColumn::unhide()
{
	m_pTable->_unhideColumns(this, 1);
}

//____ TablePanelColumn::setVisible() __________________________________________________

void TablePanelColumn::setVisible(bool bVisible)
{
	if (bVisible)
		m_pTable->_unhideColumns(this, 1);
	else
		m_pTable->_hideColumns(this, 1);
}

//____ TablePanelColumn::setWeight() __________________________________________________

void TablePanelColumn::setWeight(float weight)
{
	if (weight != m_weight)
		m_pTable->_reweightColumns(this, 1, weight);
}

//____ TablePanelColumn::setMinWidth() __________________________________________

void TablePanelColumn::setMinWidth(pts width)
{
	if (width != m_minWidth)
		m_pTable->_setMinWidths(this, 1, width);
}

//____ TablePanelColumn::setMaxWidth() ___________________________________________

void TablePanelColumn::setMaxWidth(pts width)
{
	if (width != m_maxWidth)
		m_pTable->_setMaxWidths(this, 1, width);
}

} // namespace wg
#endif //WG_TABLEPANEL_DOT_H
