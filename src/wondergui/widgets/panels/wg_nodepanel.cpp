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

//TODO: Proper handling of overflow!

//TODO: Callbacks for limiting movement.
//TODO: Selected state på selected node.

namespace wg
{
	using namespace Util;

	template class DynamicSlotVector<NodePanelSlot>;
	template class PanelTemplate<NodePanelSlot>;

	const TypeInfo NodePanel::TYPEINFO = { "NodePanel", &Panel::TYPEINFO };
	const TypeInfo NodePanelSlot::TYPEINFO = { "NodePanelSlot", &PanelSlot::TYPEINFO };


	//____ NodePanelSlot::setCenter() ___________________________________________

	Coord NodePanelSlot::setCenter(Coord pos)
	{
		NodePanel * pHolder = static_cast<NodePanel*>(_holder());

		int scale = pHolder->_scale();

		CoordSPX posSPX = ptsToSpx(pos, scale) + pHolder->m_skin.contentOfs(pHolder->m_scale, pHolder->m_state);
		pHolder->_updateNodeGeo(this,posSPX,true);

		return m_center;
	}

	//____ NodePanelSlot::setCenterNormalized() __________________________________

	CoordF NodePanelSlot::setCenterNormalized(CoordF pos)
	{
		NodePanel * pHolder = static_cast<NodePanel*>(_holder());

		int scale = pHolder->_scale();
		RectSPX rect = pHolder->_contentRect();


		CoordSPX posSPX = {spx(rect.x + rect.w * pos.x), spx(rect.y + rect.h * pos.y) };
		pHolder->_updateNodeGeo(this,posSPX,true);


		Size size = spxToPts(rect.size(), scale);
		return { m_center.x / float(size.w), m_center.y / float(size.h) };
	}

	//____ NodePanelSlot::centerNormalized() _____________________________________

	CoordF NodePanelSlot::centerNormalized() const
	{
		auto pHolder = static_cast<const NodePanel*>(_holder());

		Size size = spxToPts(pHolder->m_size, pHolder->m_scale);
		return { m_center.x / float(size.w), m_center.y / float(size.h) };
	}

	//____ NodePanelSlot::_setBlueprint()  _______________________________________

	bool NodePanelSlot::_setBlueprint(const Blueprint& bp)
	{
		m_bVisible = bp.visible;
		m_nodeId = bp.nodeId;

		if( bp.centerNormalized.x >= 0 && bp.centerNormalized.y >= 0 )
		{
			auto pNodePanel = static_cast<NodePanel*>(m_pHolder->_container());

			SizeSPX sizeSPX = pNodePanel->_contentRect().size();
			Size size = spxToPts(sizeSPX, pNodePanel->m_scale);

			m_center = { bp.centerNormalized.x * size.w, bp.centerNormalized.y * size.h };
		}
		else
			m_center = bp.center;

		return true;
	}

	//____ NodeVector::find() ____________________________________________________

	NodePanel::NodeVector::iterator NodePanel::NodeVector::find(int nodeId)
	{
		auto it = std::lower_bound( m_entries.begin(), m_entries.end(), nodeId, [](const Node& node, int nodeId) { return node.id() < nodeId; } );
		if( it != m_entries.end() && it->id() == nodeId )
			return it;
		else
			return m_entries.end();
	}

	//____ NodeVector::has() _____________________________________________________

	bool NodePanel::NodeVector::has(int nodeId)
	{
		auto it = std::lower_bound( m_entries.begin(), m_entries.end(), nodeId, [](const Node& node, int nodeId) { return node.id() < nodeId; } );
		if( it != m_entries.end() && it->id() == nodeId )
			return true;

		return false;
	}

	//____ constructor ____________________________________________________________

	NodePanel::NodePanel()
	{
		m_bSiblingsOverlap = true;
	}

	//____ Destructor _____________________________________________________________

	NodePanel::~NodePanel()
	{
		for( auto pObserver : m_observers )
			pObserver->_nodeCanvasDestroyed();
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

	//____ clearNodePosModifier() ________________________________________________

	void NodePanel::clearNodePosModifier()
	{
		m_nodePosModifier = nullptr;
	}

	//____ setNodePosModifier() __________________________________________________

	void NodePanel::setNodePosModifier( const std::function<Coord(const NodePanel * pPanel, NodeVector::const_iterator nodeIt, Coord pos)>& callback )
	{
		m_nodePosModifier = callback;
	}

	//____ setNodeConstraint() ___________________________________________________

	void NodePanel::setNodeConstraint( NodeConstraint constraint )
	{
		if( constraint != m_nodeConstraint )
		{
			m_nodeConstraint = constraint;

			//TODO: Update positions

		}

	}

	//____ _addObserver() _________________________________________________________

	bool NodePanel::_addObserver( Observer * pObserver )
	{
		// Add observer

		m_observers.push_back(pObserver);

		// Set canvas size.

		pObserver->_nodeCanvasResized({0,0}, m_size);

		// Send all existing (and visible) nodes to newly added Observer

		for( auto& slot : slots )
		{
			if( slot.m_bVisible )
				pObserver->_nodeAdded(slot.m_nodeId, slot.m_geo, slot.m_bVisible);
		}

		return true;
	}

	//____ _removeObserver() ______________________________________________________

	bool NodePanel::_removeObserver( Observer * pObserver )
	{
		auto it = std::find( m_observers.begin(), m_observers.end(), pObserver);

		if( it != m_observers.end() )
		{
			m_observers.erase(it);
			return true;
		}
		else
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "Attempt to remove observer not in list. Ignoring.", this, &NodePanel::TYPEINFO, __func__, __FILE__, __LINE__ );
			return false;
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

				CoordSPX pointerPos = _toLocal( pMsg->pointerSpxPos() );

				for (auto& slot : slots)
				{
					if( slot.isVisible() && slot._geo().contains(pointerPos) )
					{
						m_pDraggedChild = slot._widget();
						m_draggedChildStartPos = slot.m_center;
						slots.moveToFront(&slot);

						State s = slot._widget()->state();
						s.setSelekted(true);
						slot._widget()->_setState(s);

						pMsg->swallow();
						break;
					}
				}
				break;
			}

			case MsgType::MouseRepeat:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);
				if ( m_pDraggedChild && pMsg->button() == MouseButton::Left)
					pMsg->swallow();
				break;
			}

			case MsgType::MouseRelease:

				if( m_pDraggedChild )
				{
					m_pDraggedChild = nullptr;
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseDrag:
			{
				if (m_pDraggedChild)
				{
					auto pSlot = static_cast<NodePanelSlot*>(m_pDraggedChild->_slot());

					auto pMsg = static_cast<MouseDragMsg*>(_pMsg);
					Coord newPos = m_draggedChildStartPos + spxToPts(pMsg->_draggedTotal(), m_scale);

					if( m_nodePosModifier )
						newPos = m_nodePosModifier(this, nodes.find(pSlot->nodeId()), newPos );

					CoordSPX newPosSPX = ptsToSpx(newPos,m_scale) + _contentRect().pos();

					_updateNodeGeo(pSlot, newPosSPX, true );

					pMsg->swallow();
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

	//____ _resize() _____________________________________________________________

	void NodePanel::_resize(const SizeSPX& size, int scale)
	{
		SizeSPX oldSize = m_size;
		bool bScaleChanged = scale != m_scale;
		PanelTemplate::_resize(size,scale);

		for( auto& slot : slots )
		{
			auto pChild = slot._widget();

			SizeSPX newSize = pChild->_defaultSize(scale);

			if( bScaleChanged || pChild->_size() != newSize )
				pChild->_resize(newSize, scale);

			_updateNodeGeo( &slot, ptsToSpx( slot.m_center, m_scale) + _contentRect().pos(), false );
		}

		if( m_size != oldSize )
		{
			for( auto pObserver : m_observers )
				pObserver->_nodeCanvasResized(oldSize,m_size);
		}
	}

	//____ _slotTypeInfo() _________________________________________________________

	const TypeInfo& NodePanel::_slotTypeInfo(const StaticSlot* pSlot) const
	{
		return NodePanelSlot::TYPEINFO;
	}

	//____ _childRequestRender() _________________________________________________

	void NodePanel::_childRequestRender(StaticSlot* pSlot, const RectSPX& rect)
	{
		_requestRender(rect + static_cast<NodePanelSlot*>(pSlot)->m_geo.pos());
	}

	//____ _childRequestResize() _________________________________________________

	void NodePanel::_childRequestResize(StaticSlot* _pSlot)
	{
		auto pSlot = static_cast<NodePanelSlot*>(_pSlot);

		auto pChild = pSlot->_widget();

		SizeSPX newSize = pChild->_defaultSize(m_scale);

		if( pChild->_size() != newSize )
			pChild->_resize(newSize, m_scale);

		_updateNodeGeo(pSlot, ptsToSpx( pSlot->m_center, m_scale) + _contentRect().pos(), true );
	}

	//____ _releaseChild() ____________________________________________________

	void NodePanel::_releaseChild(StaticSlot * pSlot)
	{
		slots.erase(static_cast<NodePanelSlot*>(pSlot));
	}

	//____ _replaceChild() _____________________________________________________

	void NodePanel::_replaceChild(StaticSlot * _pSlot, Widget * pNewChild)
	{
		if( pNewChild == nullptr )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Slot in NodePanel must contain pointer to widget and not nullptr.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		int index = slots._releaseUpdateIndex(pNewChild, int(static_cast<NodePanelSlot*>(_pSlot) - slots.begin()));
		auto& slot = slots.at(index);

		Widget_p pOldChild = slot._widget();

		slot._setWidget(pNewChild);

		if( pOldChild )
			_requestRender( slot.m_geo + pOldChild->_overflow() );

		_updateNodeGeo(&slot, ptsToSpx( slot.m_center, m_scale) + _contentRect().pos(), false);

		_requestRender( slot.m_geo + pNewChild->_overflow() );
	}

	//____ _didAddSlots() ________________________________________________________

	void NodePanel::_didAddSlots(StaticSlot* _pSlot, int nb)
	{
		//TODO: No handling of duplicate nodeId !

		RectSPX contentRect = _contentRect();

		auto pSlot = static_cast<NodePanelSlot*>(_pSlot);
		for (int i = 0; i < nb; i++)
		{
			// Add slot, resize widget and request render

			CoordSPX pos = ptsToSpx(pSlot->m_center, m_scale) + contentRect.pos();
			SizeSPX size = pSlot->_widget()->_defaultSize(m_scale);

			pos.x -= size.w/2;
			pos.y -= size.h/2;

			align(pos);

			// Constrain to our content rect

			RectSPX constrainer = m_nodeConstraint == NodeConstraint::Bounds ? contentRect
													: align( contentRect + BorderSPX(size.w/2,size.h/2) );

			pSlot->m_geo = constrainer.limit(RectSPX(pos, size));

			pSlot->_widget()->_resize(size, m_scale);

			pSlot->m_center = spxToPts( pSlot->m_geo.center() - contentRect.pos(), m_scale );

			// Set NodeId

			if( pSlot->m_nodeId == 0 )
			{
				if( nodes.m_entries.empty() )
					pSlot->m_nodeId = 1;
				else
					pSlot->m_nodeId = nodes.m_entries.back().id() + 1;
			}

			// Request render

			if( pSlot->m_bVisible )
				_requestRender(pSlot->m_geo + pSlot->_widget()->_overflow());

			// Update Node array

			auto it = std::lower_bound(nodes.m_entries.begin(), nodes.m_entries.end(), pSlot->m_nodeId, [](const Node& pos, int nodeId){ return pos.id() < nodeId; });
			nodes.m_entries.insert(it, {pSlot->m_nodeId, pSlot->_widget()});

			// Notify observers

			for( auto pObserver : m_observers )
				pObserver->_nodeAdded(pSlot->m_nodeId, pSlot->m_geo, pSlot->m_bVisible);

			pSlot++;
		}
	}

	//____ _didMoveSlots() _______________________________________________________

	void NodePanel::_didMoveSlots(StaticSlot* pFrom, StaticSlot* pTo, int nb)
	{
		NodePanelSlot * pSlot = static_cast<NodePanelSlot*>(pTo);

		RectSPX dirtyRect = pSlot->m_geo + pSlot->m_pWidget->_overflow();

		for( int i = 1 ; i < nb ; i++ )
			dirtyRect.growToContain(pSlot[i].m_geo + pSlot[i].m_pWidget->_overflow());

		_requestRender(dirtyRect);
	}

	//____ _willEraseSlots() _____________________________________________________

	void NodePanel::_willEraseSlots(StaticSlot* _pSlot, int nb)
	{
		auto pSlot = static_cast<NodePanelSlot*>(_pSlot);
		for (int i = 0; i < nb; i++)
		{
			// Clean up canvas

			if( pSlot->m_bVisible )
				_requestRender(pSlot->m_geo + pSlot->_widget()->_overflow());

			// Update Node array

			int nodeId = pSlot->m_nodeId;
			auto it = std::find_if(nodes.m_entries.begin(), nodes.m_entries.end(), [nodeId](const Node& node) {return node.m_id == nodeId;});

			nodes.m_entries.erase(it);

			// Notify observers

			for( auto pObserver : m_observers )
				pObserver->_nodeRemoved(pSlot->m_nodeId);

			pSlot++;
		}
	}

	//____ _hideSlots() __________________________________________________________

	void NodePanel::_hideSlots(StaticSlot* _pSlot, int nb)
	{
		auto pSlot = static_cast<NodePanelSlot*>(_pSlot);
		for (int i = 0; i < nb; i++)
		{
			pSlot->m_bVisible = false;

			if( pSlot->m_bVisible )
			{
				_requestRender(pSlot->m_geo + pSlot->_widget()->_overflow());

				for( auto pObserver : m_observers )
					pObserver->_nodeHidden(pSlot->m_nodeId);
			}

			pSlot++;
		}
	}

	//____ _unhideSlots() ________________________________________________________

	void NodePanel::_unhideSlots(StaticSlot* _pSlot, int nb)
	{
		auto pSlot = static_cast<NodePanelSlot*>(_pSlot);
		for (int i = 0; i < nb; i++)
		{
			if( !pSlot->m_bVisible )
			{
				pSlot->m_bVisible = true;

				_requestRender(pSlot->m_geo + pSlot->_widget()->_overflow());

				for( auto pObserver : m_observers )
					pObserver->_nodeUnhidden(pSlot->m_nodeId);
			}

			pSlot++;
		}
	}

	//____ _updateNodeGeo() _________________________________________________________

	void NodePanel::_updateNodeGeo( NodePanelSlot * pSlot, CoordSPX center, bool bRequestRender )
	{
		RectSPX contentRect = _contentRect();
		Widget * pChild = pSlot->_widget();


		CoordSPX newPos = center;
		
		SizeSPX sizeSPX = pChild->_defaultSize(m_scale);

		newPos.x -= sizeSPX.w/2;
		newPos.y -= sizeSPX.h/2;

		align(newPos);

		// Constrain to our content rect

		RectSPX constrainer = m_nodeConstraint == NodeConstraint::Bounds ? contentRect
												: align( contentRect + BorderSPX(sizeSPX.w/2,sizeSPX.h/2) );

		RectSPX newGeo = constrainer.limit(RectSPX(newPos, sizeSPX));
		RectSPX oldGeo = pSlot->m_geo;

		if( newGeo == oldGeo )
			return;							// No change

		// Update child

		if( pChild->_size() != newGeo.size() || pChild->_scale() != m_scale )
			pChild->_resize(newGeo.size(), m_scale);

		// Update slot

		if( bRequestRender )
		{
			_requestRender(oldGeo + pSlot->_widget()->_overflow());	// Request render of old position.
			_requestRender(newGeo + pSlot->_widget()->_overflow());	// Request render of new position.
		}

		pSlot->m_geo = newGeo;

		pSlot->m_center = spxToPts(newGeo.center() - _contentRect().pos(), m_scale);

		// Notify observers

		for( auto pObserver : m_observers )
			pObserver->_nodeMovedOrResized(pSlot->m_nodeId, oldGeo, newGeo);

	}
}
