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

#include <wg_togglegroup.h>
#include <wg_togglebutton.h>
#include <wg_msgrouter.h>

namespace wg
{

	const TypeInfo ToggleGroup::TYPEINFO = { "ToggleGroup", &Object::TYPEINFO };


	//____ constructor _____________________________________________________________

	ToggleGroup::ToggleGroup()
	{
	}

	//____ Destructor ______________________________________________________________

	ToggleGroup::~ToggleGroup()
	{
		for( unsigned int i = 0 ; i < m_entries.size() ; i++ )
			m_entries[i]->_setToggleGroup(0);
}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ToggleGroup::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setRequireChecked() _______________________________________________________
	/**
	 * Sets the requireChecked flag of this group, which means that there will always
	 * be a checked widget in the group as long as the group is not empty. The default state
	 * of this flag is TRUE.
	 *
	 * @param bRequire 	Set to TRUE to require a member to always be checked. Set to
	 * 					false to allow all members to be unchecked.
	 *
	 * Setting this flag in an empty ToggleGroup will result in the first widget that is added
	 * to automatically be checked. If the ToggleGroup is not empty but has no checked
	 * member, the first member of the group will be checked.
	 *
	 * It will not be possible to uncheck a widget when requireChecked is TRUE, the only way
	 * to uncheck a member is to check another one. If the checked member of
	 * the group is removed, the first member will be checked.
	 *
	 **/

	void ToggleGroup::setRequireChecked(bool bRequire)
	{
		m_bRequireChecked = bRequire;

		if( bRequire && !m_pChecked && !m_entries.empty() )
			m_entries[0]->setChecked(true);
	}

	//____ add() ___________________________________________________________________
	/**
	 * Add specified ToggleButton to this group, making it behave like a RadioButton
	 * against the rest of the members of this group.
	 *
	 * @pToggleButton	ToggleButton to be added to this ToggleGroup.
	 *
	 * A ToggleButton can only belong to one ToggleGroup at the same time. If added
	 * ToggleButton is member of another group, it will automatically be removed from
	 * that group.
	 *
	 * If the group is empty and it is set to require a member to be selected (default
	 * behavior) the added widget will automatically be selected. If the added member
	 * is in a selected state but the group already has a selected member, the added
	 * member is unselected.
	 *
	 **/

	void ToggleGroup::add( ToggleButton * pToggleButton )
	{
		ToggleButton * p = pToggleButton;
		if( p )
		{
			if( p->isChecked() )
			{
				if( m_pChecked )
					p->setChecked(false);
				else
					m_pChecked = p;
			}

			p->_setToggleGroup(this);
			m_entries.push_back( p );
		}

		// Select first entry if none selected and we require one to be

		if( m_bRequireChecked && !m_pChecked  )
			pToggleButton->setChecked(true);
	}

	//____ remove() ________________________________________________________________
	/**
	 * Removes specified ToggleButton from this group.
	 *
	 * If the removed ToggleButton was selected, it will keep its selected state when removed.
	 * If the group requires a member to be selected (default behavior), the first member
	 * of the group will be selected.
	 *
	 **/


	bool ToggleGroup::remove( ToggleButton * pToggleButton )
	{
		if( pToggleButton && pToggleButton->_toggleGroup() == this )
		{
			pToggleButton->_setToggleGroup(0);
			_remove( pToggleButton );
			return true;
		}

		return false;
	}

	//____ clear() _________________________________________________________________
	/**
	 * Removes all members from this group.
	 *
	 * The selected member (if any) will keep its selected state upon removal.
	 *
	 **/

	void ToggleGroup::clear()
	{
		for( unsigned int i = 0 ; i < m_entries.size() ; i++ )
			m_entries[i]->_setToggleGroup(0);

		m_pChecked = 0;
		m_entries.clear();
	}

	//____ get() ___________________________________________________________________
	/**
	 * Get a specific member of the group
	 *
	 * @param index		Index (0+) of the member to get.
	 *
	 *
	 * @return Pointer to member at index or null if index was out of bounds.
	 **/

	ToggleButton_p ToggleGroup::get( int index )
	{
		if( index >= 0 && index < (int) m_entries.size() )
			return ToggleButton_p(m_entries[index]);

		return ToggleButton_p();
	}

	//____ size() __________________________________________________________________
	/**
	 * Get the number of members in the group.
	 *
	 * @return Number of members in this ToggleGroup.
	 **/

	int ToggleGroup::size() const
	{
		return (int) m_entries.size();
	}

	//____ selected() ______________________________________________________________
	/**
	 * Get the selected widget.
	 *
	 * @return A pointer to the widget of the group that is selected or null if none.
	 *
	 **/

	ToggleButton_p ToggleGroup::selected() const
	{
		return ToggleButton_p(m_pChecked);
	}

	//____ _remove() _______________________________________________________________

	void ToggleGroup::_remove( ToggleButton * pButton )
	{
		for( std::vector<ToggleButton*>::iterator it = m_entries.begin() ; it != m_entries.end() ; ++it )
		{
			if( * it == pButton )
			{
				m_entries.erase( it );
				break;
			}
		}

		if( pButton == m_pChecked )
		{
			m_pChecked = 0;
			if( m_bRequireChecked && !m_entries.empty() )
				m_entries[0]->setChecked(true);
		}
	}

	//____ _check() _______________________________________________________________

	void ToggleGroup::_check( ToggleButton * pButton )
	{
		ToggleButton * pOldChecked = m_pChecked;

		m_pChecked = pButton;

		if( pOldChecked && pOldChecked != pButton )
			pOldChecked->setChecked(false);

//		Base::msgRouter()->post(SelectMsg::create(pButton));
//		Base::msgRouter()->post(SelectMsg::create(this));
	}

	//____ _uncheck() _____________________________________________________________

	bool ToggleGroup::_uncheck( ToggleButton * pButton )
	{
		if( pButton ==  m_pChecked )
		{
			if( m_bRequireChecked )
				return false;

			m_pChecked = 0;
//			Base::msgRouter()->post(UnselectMsg::create(pButton));
//			Base::msgRouter()->post(UnselectMsg::create(this));
		}
		return true;
	}

} // namespace wg
