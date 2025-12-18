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
#include <wg_slottable.h>

using namespace wg;


//____ at() ___________________________________________________________________

DynamicSlot& SlotTable::at(int row, int column)
{
	if( row < 0 || row >= m_nRows || column < 0 || column >= m_nColumns )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Row or column is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return m_dummySlot;
	}
	
	return m_slots[row*m_nColumns+column];
}

const DynamicSlot& SlotTable::at(int row, int column) const
{
	if( row < 0 || row >= m_nRows || column < 0 || column >= m_nColumns )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Row or column is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return m_dummySlot;
	}
	
	return m_slots[row* m_nColumns +column];
}
		
//____ replaceRow() ___________________________________________________________

int SlotTable::replaceRow(int index, const std::initializer_list<Widget_p>& widgets)
{
	if( index < 0 || index >= m_nRows )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Row index is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return 0;
	}
	
	DynamicSlot * p = m_slots.data() + index * m_nColumns;
	
	int nb = std::min(int(widgets.size()), m_nColumns );
	
	for( auto it = widgets.begin() ; it < widgets.begin() + nb ; it++ )
	{
		p->_setWidget(*it);
		p++;
	}
	
	_slotsUpdated(index*m_nColumns, Axis::X, nb);
	return nb;
}

//____ replaceColumn() ________________________________________________________

int SlotTable::replaceColumn(int index, const std::initializer_list<Widget_p>& widgets)
{
	if( index < 0 || index >= m_nColumns )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Column index is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return 0;
	}
	
	DynamicSlot * p = m_slots.data() + index;

	int nb = std::min(int(widgets.size()), m_nRows );
	
	for( auto it = widgets.begin() ; it < widgets.begin() + nb ; it++ )
	{
		p->_setWidget(*it);
		p += m_nColumns;
	}
	
	_slotsUpdated(index, Axis::Y, nb);
	return nb;
}

//____ clearRows() ____________________________________________________________

void SlotTable::clearRows(int index, int nb )
{
	if( index < 0 || index + nb > m_nRows )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Specified range is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return;
	}

	// Turn index and nb from specifying rows to slots.

	index *= m_nColumns;		
	nb *= m_nColumns;

	//

	DynamicSlot* p = m_slots.data() + index;

	for( int x = 0 ; x < nb ; x++ )
		p[x]._clearWidget();

	_slotsUpdated(index, Axis::X, nb);
}

//____ clearColumns() _________________________________________________________

void SlotTable::clearColumns(int index, int nb )
{
	if( index < 0 || index + nb > m_nColumns )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Specified range is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return;
	}
	
	for( int x = index ; x < index + nb ; x++ )
	{
		DynamicSlot * p = m_slots.data() + x;

		for( int y = 0 ; y < m_nRows ; y++ )
			p[y*m_nColumns]._clearWidget();
	}
	
	_slotsUpdated(index, Axis::Y, nb*m_nRows);
}

//____ replaceWidgets() _______________________________________________________

int SlotTable::replaceWidgets(iterator it, Axis axis, const std::initializer_list<Widget_p>& widgets)
{
	int ofs = int(it - m_slots.begin());
		
	auto itWidgets = widgets.begin();
	
	int nb = 0;
	
	if( axis == Axis::X )
	{
		int ofs2 = ofs;
		while( itWidgets != widgets.end() )
		{
			m_slots[ofs2]._setWidget(* itWidgets++);
			ofs2++;
			nb++;
			if( ofs2 == m_slots.size() )
				break;
		}
	}
	else
	{
		int x = ofs % m_nColumns;
		int y = ofs / m_nColumns;
		
		while( itWidgets != widgets.end() )
		{
			m_slots[x + y * m_nColumns]._setWidget(* itWidgets++);
			y++;
			nb++;
			if( y == m_nRows )
			{
				y = 0;
				x++;
				if( x == m_nColumns )
					break;
			}
		}
	}
	
	_slotsUpdated(ofs, axis, nb );
	return nb;
}

//____ clearWidgets() _________________________________________________________

void SlotTable::clearWidgets(iterator it, Axis axis, int nb )
{
	int ofs = int(it - m_slots.begin());

	if (ofs + nb > m_slots.size())
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Specified range is out of range.", m_pTableHolder->_object(), nullptr, __func__, __FILE__, __LINE__);
		return;
	}

	if (axis == Axis::X)
	{
		for (int i = ofs; i < ofs + nb; i++)
			m_slots[i]._clearWidget();
	}
	else
	{
		int x = ofs % m_nColumns;
		int y = ofs / m_nColumns;

		for (int i = 0; i < nb; i++)
		{
			m_slots[x + y * m_nColumns]._clearWidget();
			y++;
			if (y == m_nRows)
			{
				y = 0;
				x++;
			}
		}
	}

	_slotsUpdated(ofs, axis, nb);
}

//____ _insertRows() ______________________________________________________________

void SlotTable::_insertRows( int ofs, int nb )
{
	m_nRows += nb;

	if (m_nColumns == 0)
		return;

	m_slots.insert(m_slots.begin() + ofs * m_nColumns, nb * m_nColumns, DynamicSlot(m_pHolder) );
}

//____ _deleteRows() ______________________________________________________________

void SlotTable::_deleteRows(int ofs, int nb)
{
	m_nRows -= nb;

	if (m_nColumns == 0)
		return;

	auto first = m_slots.begin() + ofs * m_nColumns;
	auto last = first + nb * m_nColumns;
	m_slots.erase(first, last);
}


//____ _insertColumns() ______________________________________________________________

void SlotTable::_insertColumns(int ofs, int nb)
{
	if (m_nRows == 0)
	{
		m_nColumns += nb;
		return;
	}

	m_slots.resize((m_nColumns+nb) * m_nRows, DynamicSlot(m_pHolder) );

	auto src = m_slots.begin() + m_nRows * m_nColumns;
	auto dest = m_slots.end();

	int nLeft	= ofs;
	int nAdded	= nb;
	int nRight	= m_nColumns - nLeft;

	while (src > m_slots.begin() + m_nColumns )
	{
		for (int i = 0; i < nRight; i++)
			* --dest = std::move(* --src);

		dest -= nAdded;

		for (int i = 0; i < nLeft; i++)
			*--dest = std::move(*--src);
	}
	
	// Last row is special case since we can't move from/to same slot.
	
	for (int i = 0; i < nRight; i++)
		* --dest = std::move(* --src);

	m_nColumns += nb;
}

//____ _deleteColumns() ______________________________________________________________

void SlotTable::_deleteColumns(int ofs, int nb)
{
	if (m_nRows == 0)
	{
		m_nColumns -= nb;
		return;
	}

	auto src = m_slots.begin();
	auto dest = m_slots.begin();

	int nLeft = ofs;
	int nErased = nb;
	int nRight = m_nColumns - nLeft - nErased;

	// First row is special case since we can't move from/to same slot.

	src += nLeft + nErased;
	dest += nLeft;

	for (int i = 0; i < nRight; i++)
		*dest++ = std::move(*src++);

	while (src < m_slots.end())
	{
		for (int i = 0; i < nLeft; i++)
			*dest++ = std::move(*src++);

		src += nErased;

		for (int i = 0; i < nRight; i++)
			*dest++ = std::move(*src++);
	}

	m_nColumns -= nb; 

	m_slots.resize(m_nColumns * m_nRows, DynamicSlot(m_pHolder) );
}

//____ _resize() ______________________________________________________________

void SlotTable::_resize(int nbRows, int nbColumns)
{
	if( m_slots.empty() )
	{
		m_slots.resize(nbColumns * nbRows, DynamicSlot(m_pHolder) );
		m_nRows = nbRows;
		m_nColumns = nbColumns;
	}
	else if( nbRows == 0 || nbColumns == 0 )
	{
		m_slots.clear();
		m_nRows = nbRows;
		m_nColumns = nbColumns;
	}
	else
	{
		// It would be faster, but more complicated, to resize both rows and columns simultaneously.
		
		if( nbRows < m_nRows )
			_deleteRows(nbRows, m_nRows - nbRows);
		
		if( nbColumns < m_nColumns )
			_deleteColumns(nbColumns, m_nColumns - nbColumns);
		else if( nbColumns > m_nColumns )
			_insertColumns(m_nColumns, nbColumns - m_nColumns);
		
		if( nbRows > m_nRows )
			_insertRows(m_nRows, nbRows - m_nRows);
	}
}

//____ _reserve() _____________________________________________________________

void SlotTable::_reserve( int rows, int columns )
{
	m_slots.reserve(rows * columns);	
}

//____ _shrinkToFit() _____________________________________________________________

void SlotTable::_shrinkToFit()
{
	m_slots.shrink_to_fit();
}

//____ _slotsUpdated() ________________________________________________________

void SlotTable::_slotsUpdated(int start, Axis axis, int nb)
{
	m_pTableHolder->_refreshSlots(start,axis,nb);
}

//____ _clearRows() ____________________________________________________________

void SlotTable::_clearRows( int startIndex, int nb )
{
	DynamicSlot * p = m_slots.data() + startIndex * m_nColumns;

	for( int i = 0 ; i < nb * m_nColumns ; i++ )
		p[i]._clearWidget();
}

//____ _clearColumns() ____________________________________________________________

void SlotTable::_clearColumns( int startIndex, int nb )
{
	DynamicSlot * p = m_slots.data() + startIndex;

		for( int y = 0 ; y < m_nRows ; y++ )
		{
			for( int x = 0 ; x < nb ; x++ )
				p[x]._clearWidget();
			
			p += m_nColumns;
		}
}
