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

#include <wg_nodepanel.h>
#include <wg_msg.h>

#include <wg_dynamicslotvector.impl.h>
#include <wg_panel.impl.h>

namespace wg
{
	using namespace Util;

	template class DynamicSlotVector<NodePanelSlot>;
	template class PanelTemplate<NodePanelSlot>;

	const TypeInfo NodePanel::TYPEINFO = { "NodePanel", &Panel::TYPEINFO };
	const TypeInfo NodePanelSlot::TYPEINFO = { "NodePanelSlot", &PanelSlot::TYPEINFO };


	//____ NodePanelSlot::_setBlueprint()  __________________________________________________________

	bool NodePanelSlot::_setBlueprint(const Blueprint& bp)
	{
		m_bVisible = bp.visible;
		return true;
	}



	//____ constructor ____________________________________________________________

	NodePanel::NodePanel()
	{
		m_bSiblingsOverlap = true;
	}

	//____ Destructor _____________________________________________________________

	NodePanel::~NodePanel()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& NodePanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDefaultSize() __________________________________________________

	void NodePanel::setDefaultSize(Size size)
	{
		if (size != m_defaultSize)
		{
			m_defaultSize = size;
			_requestResize();
		}
	}

	//____ _defaultSize() __________________________________________________

	SizeSPX NodePanel::_defaultSize(int scale) const
	{
		return Util::ptsToSpx(m_defaultSize, scale);
	}

	//____ _receive() _________________________________________________________

	void NodePanel::_receive(Msg* _pMsg)
	{
		switch (_pMsg->type())
		{
			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() != MouseButton::Left)
					break;

				for (auto& slot : slots)
				{
					if( slot.isVisible() && slot._geo().contains(pMsg->pointerSpxPos()) )
					{
						m_pDraggedChild = slot._widget();
						m_draggedChildStartPos = slot._geo().pos();
						pMsg->swallow();
						break;
					}
				}
				break;
			}

			case MsgType::MouseRelease:
				m_pDraggedChild = nullptr;
				break;

			case MsgType::MouseDrag:
			{
				if (m_pDraggedChild)
				{
					auto pMsg = static_cast<MouseDragMsg*>(_pMsg);
					CoordSPX draggedTotal = pMsg->_draggedTotal();

					CoordSPX newPos = m_draggedChildStartPos + draggedTotal;

					auto pSlot = static_cast<NodePanelSlot*>(m_pDraggedChild->_slot());

					_requestRender(pSlot->_geo() + pSlot->_widget()->_overflow());	// Request render of old position.
					pSlot->m_geo.setPos(newPos);
					_requestRender(pSlot->_geo() + pSlot->_widget()->_overflow());	// Request render of new position.
				}

				break;
			}

			default:
				break;
		}

		PanelTemplate::_receive(_pMsg);

	}

	//____ _render() _______________________________________________________________

	void NodePanel::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Panel::_render(pDevice, _canvas, _window);
	}


	//____ _slotTypeInfo() _________________________________________________________

	const TypeInfo& NodePanel::_slotTypeInfo(const StaticSlot* pSlot) const
	{
		return NodePanelSlot::TYPEINFO;
	}

	void NodePanel::_childRequestRender(StaticSlot* pSlot, const RectSPX& rect)
	{
		_requestRender(rect + static_cast<NodePanelSlot*>(pSlot)->m_geo.pos());
	}

	void NodePanel::_childRequestResize(StaticSlot* pSlot)
	{
	}

	void NodePanel::_releaseChild(StaticSlot* pSlot)
	{
	}

	void NodePanel::_replaceChild(StaticSlot* pSlot, Widget* pNewChild)
	{
	}

	void NodePanel::_didAddSlots(StaticSlot* _pSlot, int nb)
	{
		auto pSlot = static_cast<NodePanelSlot*>(_pSlot);
		for (int i = 0; i < nb; i++)
		{
			pSlot->m_geo = pSlot->_widget()->_defaultSize(m_scale);
			pSlot->_widget()->_resize(pSlot->m_geo.size(), m_scale);
			pSlot->m_bVisible = true;
			_requestRender(pSlot->m_geo + pSlot->_widget()->_overflow());
			pSlot++;
		}
	}
	
	void NodePanel::_didMoveSlots(StaticSlot* pFrom, StaticSlot* pTo, int nb)
	{
	}

	void NodePanel::_willEraseSlots(StaticSlot* pSlot, int nb)
	{
	}

	void NodePanel::_hideSlots(StaticSlot*, int nb)
	{
	}

	void NodePanel::_unhideSlots(StaticSlot*, int nb)
	{
	}

}