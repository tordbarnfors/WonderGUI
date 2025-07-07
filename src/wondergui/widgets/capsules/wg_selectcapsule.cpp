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

#include <wg_selectcapsule.h>
#include <wg_msg.h>
#include <wg_msgrouter.h>

namespace wg
{

	const TypeInfo SelectCapsule::TYPEINFO = { "SelectCapsule", &Capsule::TYPEINFO };

	
	//____ create _____________________________________________________________

	SelectCapsule_p	SelectCapsule::create()
	{
		return SelectCapsule_p(new SelectCapsule());
	}

	SelectCapsule_p	SelectCapsule::create(const Blueprint& blueprint)
	{
		return SelectCapsule_p(new SelectCapsule(blueprint));
	}

	//____ constructor ________________________________________________________

	SelectCapsule::SelectCapsule()
	{

	}

	//____ destructor _________________________________________________________

	SelectCapsule::~SelectCapsule()
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SelectCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ select() ___________________________________________________________

	bool SelectCapsule::select(Widget* pWidget)
	{
		if (_isWidgetUnderOurControl(pWidget) == false)
			return false;

		// 

		if (m_selectMode == SelectMode::SingleEntry)
		{
			std::vector<Widget_p>	unselectedList;

			_setSelModeRecursively(_topContainer(), false, &unselectedList, pWidget);

			// We don't post messages for the selected one since it is already know by caller,
			// but we do post for those that are indirectly unselected.

			if (!unselectedList.empty())
				Base::msgRouter()->post(UnselectedMsg::create(this, unselectedList));
		}

		State s = pWidget->state();
		if (!s.isSelected())
			pWidget->_setState(s + State::Selected);
		return true;
	}

	//____ unselect() _________________________________________________________

	bool SelectCapsule::unselect(Widget* pWidget)
	{
		if (_isWidgetUnderOurControl(pWidget) == false)
			return false;

		// 

		State s = pWidget->state();
		if (s.isSelected())
			pWidget->_setState(s - State::Selected);
		return true;
	}

	//____ selectAll() ________________________________________________________

	void SelectCapsule::selectAll()
	{
		auto pContainer = _topContainer();
		_setSelModeRecursively(pContainer, true, nullptr);
	}

	//____ unselectAll() ______________________________________________________

	void SelectCapsule::unselectAll()
	{
		auto pContainer = _topContainer();
		_setSelModeRecursively(pContainer, false, nullptr);
	}

	//____ findOneSelected() __________________________________________________

	Widget_p SelectCapsule::findOneSelected() const
	{
		return _findOneSelectedRecursively(_topContainer());
	}

	//____ findAllSelected() __________________________________________________

	std::vector<Widget_p> SelectCapsule::findAllSelected() const
	{
		std::vector<Widget_p> selected;
		_findAllSelectedRecursively(_topContainer(), &selected);

		return selected;
	}


	//____ setSelectMode() ____________________________________________________

	void SelectCapsule::setSelectMode(SelectMode mode)
	{
		if (m_selectMode != mode)
		{
			m_selectMode = mode;
			
			if (m_selectMode == SelectMode::SingleEntry)
			{
				//We unselect all but first encountered when swithing to single entry.

				auto pTopContainer = _topContainer();

				auto pFound = _findOneSelectedRecursively(pTopContainer);
				if (pFound)
				{
					std::vector<Widget_p>	unselected;
					_setSelModeRecursively(pTopContainer, false, &unselected, pFound);

					if( !unselected.empty() )
						Base::msgRouter()->post(UnselectedMsg::create(this, unselected));
				}
			}
		}
	}

	//____ setRecursive() _____________________________________________________

	void SelectCapsule::setRecursive(bool bRecursive)
	{
		if (m_bRecursive != bRecursive)
		{
			m_bRecursive = bRecursive;

			if (!m_bRecursive)
			{
				// We must unselect all selected widgets that can't be selected in non-recursive mode.

				std::vector<Widget_p>	unselected;
				auto pContainer = _topContainer();
				auto p = pContainer->_firstChild();

				while (p)
				{
					if (p->isContainer())
						_setSelModeRecursively(static_cast<Container*>(p), false, &unselected);

					p = pContainer->_nextChild(p->_slot());
				}

				if (!unselected.empty())
					Base::msgRouter()->post(UnselectedMsg::create(this, unselected));
			}
		}

	}

	//____ _isWidgetUnderOurControl() _________________________________________

	bool SelectCapsule::_isWidgetUnderOurControl(Widget* pWidget)
	{
		if (!pWidget)
			return false;

		if (!pWidget->isSelectable())
			return false;

		// Check that widget is one that we controls.

		auto pContainer = pWidget->parent();
		auto pTopContainer = _topContainer();

		while (pContainer && pContainer != pTopContainer)
			pContainer = pContainer->parent();

		return (pContainer != nullptr);
	}


	//____ _findOneSelectedRecursively() _________________________________________

	Widget * SelectCapsule::_findOneSelectedRecursively(const Container* pContainer) const
	{
		auto p = pContainer->_firstChild();

		while (p)
		{
			if (p->state().isSelected())
				return p;

			if (p->isContainer() && m_bRecursive)
			{ 
				auto pFound = _findOneSelectedRecursively(static_cast<Container*>(p));
				if (pFound)
					return pFound;
			}

			p = pContainer->_nextChild(p->_slot());
		}

		return nullptr;
	}

	//____ _findAllSelectedRecursively() _________________________________________

	void SelectCapsule::_findAllSelectedRecursively(const Container* pContainer, std::vector<Widget_p>* pList) const
	{
		auto p = pContainer->_firstChild();

		while (p)
		{
			if (p->state().isSelected())
				pList->push_back(p);
	
			if (p->isContainer() && m_bRecursive)
				_findAllSelectedRecursively(static_cast<Container*>(p), pList);

			p = pContainer->_nextChild(p->_slot());
		}
	}


	//____ _setSelModeRecursively() ___________________________________________

	void SelectCapsule::_setSelModeRecursively(Container * pContainer, bool bSelected, std::vector<Widget_p> * pList, Widget * pException)
	{
		auto p = pContainer->_firstChild();

		while (p)
		{
			if (p != pException)
			{
				if (p->isSelectable())
				{
					State s = p->state();
					if (s.isSelected() != bSelected)
					{
						s.setSelected(bSelected);
						p->_setState(s);

						if (pList)
							pList->push_back(p);
					}
				}

				if (p->isContainer() && m_bRecursive)
					_setSelModeRecursively(static_cast<Container*>(p), bSelected, pList);
			}

			p = pContainer->_nextChild(p->_slot());
		}
	}

	//____ _receive() _________________________________________________________
	
	void SelectCapsule::_receive(Msg* _pMsg)
	{
		switch (_pMsg->type())
		{
			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() == MouseButton::Left)
				{
					auto pWidget = static_cast<Widget*>(pMsg->originalSource().rawPtr());

					auto pTopContainer = _topContainer();

					while (pWidget && pWidget->parent() != pTopContainer)
					{
						if (m_bRecursive && pWidget->isSelectable())
							break;

						pWidget = pWidget->parent();
					}

					if (!pWidget)
						break;						// Should never happen but avoids compiler warnings.	

					std::vector<Widget_p> selectedList;
					std::vector<Widget_p> unselectedList;


					switch (m_selectMode)
					{
						case SelectMode::Unselectable:
							break;

						case SelectMode::SingleEntry:
						{


							if( !pWidget->state().isSelected() )
							{

								auto pContainer = _topContainer();
								_setSelModeRecursively(pContainer, false, &unselectedList);

								if (pWidget->isSelectable())
								{
									pWidget->_setState(pWidget->state() + State::Selected);

									selectedList.push_back(pWidget);
								}
							}


							break;
						}
						case SelectMode::MultiEntries:
						{
							if ((pMsg->modKeys() & ModKeys::StdCtrl) == 0)
							{
								auto pContainer = _topContainer();
								_setSelModeRecursively(pContainer, false, &unselectedList, pWidget);
							}

							if (pWidget->isSelectable())
							{
								State s = pWidget->state();

								if ((pMsg->modKeys() & ModKeys::StdCtrl))
								{
									s.setSelected(!s.isSelected());
									pWidget->_setState(s);

									if (s.isSelected())
										selectedList.push_back(pWidget);
									else
										unselectedList.push_back(pWidget);
								}
								else if (!s.isSelected())
								{
									s.setSelected(true);
									pWidget->_setState(s);
									selectedList.push_back(pWidget);
								}
							}

							break;
						}
						case SelectMode::FlipOnSelect:
						{
							if (pWidget->isSelectable())
							{
								State s = pWidget->state();
								s.setSelected(!s.isSelected());
								pWidget->_setState(s);

								if (s.isSelected())
									selectedList.push_back(pWidget);
								else
									unselectedList.push_back(pWidget);
							}

							break;
						}
					}

					// Send messages

					if (!unselectedList.empty())
						Base::msgRouter()->post( UnselectedMsg::create(this, unselectedList) );

					if (!selectedList.empty())
						Base::msgRouter()->post( SelectedMsg::create(this, selectedList) );
				}


				break;
			}

			default:
				break;

		}
	}

	//____ _topContainer() ____________________________________________________

	const Container* SelectCapsule::_topContainer() const
	{
		Widget * pWidget = slot._widget();

		while (pWidget && pWidget->isContainer())
		{
			auto pContainer = static_cast<Container*>(pWidget);

			if (pWidget->isInstanceOf(Capsule::TYPEINFO))
				pWidget = pContainer->firstChild();
			else
				return pContainer;
		}

		return this;
	}

	Container* SelectCapsule::_topContainer()
	{
		Widget* pWidget = slot._widget();

		while (pWidget && pWidget->isContainer())
		{
			auto pContainer = static_cast<Container*>(pWidget);

			if (pWidget->isInstanceOf(Capsule::TYPEINFO))
				pWidget = pContainer->firstChild();
			else
				return pContainer;
		}

		return this;
	}


}