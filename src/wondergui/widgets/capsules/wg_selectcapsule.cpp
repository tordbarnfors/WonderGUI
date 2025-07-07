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

	//____ selectAll() ________________________________________________________

	void SelectCapsule::selectAll()
	{
		auto pContainer = _topContainer();
		_setSelModeRecursively(pContainer, true);
	}

	//____ unselectAll() ______________________________________________________

	void SelectCapsule::unselectAll()
	{
		auto pContainer = _topContainer();
		_setSelModeRecursively(pContainer, false);
	}

	//____ _setSelModeRecursively() ___________________________________________

	void SelectCapsule::_setSelModeRecursively(Container * pContainer, bool bSelected)
	{
		auto p = pContainer->_firstChild();

		while (p)
		{
			if (p->isSelectable())
			{
				State s = p->state();
				if (s.isSelected() != bSelected)
				{
					s.setSelected(bSelected);
					p->_setState(s);
				}
			}

			if (p->isContainer() && m_bRecursive)
				_setSelModeRecursively(static_cast<Container*>(p), bSelected);

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

					switch (m_selectMode)
					{
						case SelectMode::Unselectable:
							break;

						case SelectMode::SingleEntry:
						{
							if (pWidget->isSelectable())
							{
								if (!pWidget->state().isSelected())
								{
									unselectAll();
									pWidget->_setState(pWidget->state() + State::Selected);
								}
							}
							else
								unselectAll();

							break;
						}
						case SelectMode::MultiEntries:
						{
							if ((pMsg->modKeys() & ModKeys::StdCtrl) == 0)
								unselectAll();

							if (pWidget->isSelectable())
							{
								State s = pWidget->state();
								s.setSelected(!s.isSelected());
								pWidget->_setState(s);
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
							}

							break;
						}
					}
				}

				break;
			}

			default:
				break;

		}
	}

	//____ _topContainer() ____________________________________________________

	Container* SelectCapsule::_topContainer()
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



}