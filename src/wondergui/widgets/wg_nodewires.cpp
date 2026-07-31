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

#include <wg_nodewires.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Nodewires::TYPEINFO = { "Nodewires", &Widget::TYPEINFO };

	//____ destructor ____________________________________________________________

	Nodewires::~Nodewires()
	{
		if( m_pObserved )
			m_pObserved->_removeObserver(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Nodewires::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ attachTo() ____________________________________________________________

	bool Nodewires::attachTo( NodePanel * pNodeSource )
	{
		if( m_pObserved )
		{
			m_pObserved->_removeObserver(this);
			m_pObserved = nullptr;
			m_wires.clear();
		}

		if( pNodeSource )
		{
			bool success = pNodeSource->_addObserver(this);
			if( success )
				m_pObserved = pNodeSource;
			return success;
		}
		else
			return true;
	}

	//____ detach() ______________________________________________________________

	void Nodewires::detach()
	{
		if( m_pObserved )
		{
			m_pObserved->_removeObserver(this);
			m_pObserved = nullptr;
			m_wires.clear();
		}
	}

	//____ addWire() _____________________________________________________________

	bool Nodewires::addWire( int fromNode, Placement fromPos, int toNode, Placement toPos )
	{
		if( !m_pObserved )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "You need to attach to a NodePanel before you can add any wires.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if( std::any_of(m_wires.begin(), m_wires.end(), [fromNode,toNode] (const Wire& wire) { return wire.fromNode == fromNode && wire.toNode == toNode; }))
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "A wire going from and to the same specified nodes already exists.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		auto itFrom = m_pObserved->nodes.find(fromNode);
		auto itTo = m_pObserved->nodes.find(toNode);


		if( itFrom == m_pObserved->nodes.end() || itTo == m_pObserved->nodes.end() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "One or both of the specified nodes does not exist in the NodePanel that we are attached to.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_wires.push_back({fromNode, fromPos, toNode, toPos, itFrom->isVisible() && itTo->isVisible(), CoordSPX(), CoordSPX() });

		_updateWirePositions( m_wires.back(), &(*itFrom), &(*itTo) );

		return true;
	}

	//____ removeWire() __________________________________________________________

	bool Nodewires::removeWire( int fromNode, int toNode )
	{
		auto it = std::find_if(m_wires.begin(), m_wires.end(), [fromNode,toNode](const Wire& wire) { return wire.fromNode == fromNode && wire.toNode == toNode; });

		if( it == m_wires.end() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "No wire going from and to the specified nodes exists.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_wires.erase(it);
		return true;
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX Nodewires::_defaultSize(int scale) const
	{
		if( m_pObserved )
			return m_pObserved->_defaultSize(scale);

		return Widget::_defaultSize(scale);
	}

	//____ _render() _____________________________________________________________

	void Nodewires::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Widget::_render( pDevice, _canvas, _window );


		for( auto& wire : m_wires )
		{
			if( wire.bVisible )
				pDevice->drawLine(wire.fromPos + _canvas.pos(), wire.toPos + _canvas.pos(), m_wireColor, m_wireThickness );
		}


	}

	//____ _nodeCanvasResized() __________________________________________________

	void Nodewires::_nodeCanvasResized(SizeSPX oldSize, SizeSPX newSize)
	{
		_requestResize();
	}

	//____ _nodeCanvasDestroyed() ________________________________________________

	void Nodewires::_nodeCanvasDestroyed()
	{
		m_pObserved = nullptr;
		m_wires.clear();
	}

	//____ _nodeMovedOrResized() _________________________________________________

	void Nodewires::_nodeMovedOrResized(int nodeId, const RectSPX& oldGeo, const RectSPX& newGeo)
	{
		for( auto& wire : m_wires )
		{
			if( wire.bVisible && (wire.fromNode == nodeId || wire.toNode == nodeId) )
			{
				_requestRenderWire( wire );
				_updateWirePositions( wire, &(*m_pObserved->nodes.find(wire.fromNode)), &(*m_pObserved->nodes.find(wire.toNode)) );
				_requestRenderWire( wire );
			}
		}
	}

	//____ _nodeAdded() __________________________________________________________

	void Nodewires::_nodeAdded(int nodeId, const RectSPX& geo, bool bVisible )
	{
		// Do nothing.
	}

	//____ _nodeRemoved() ________________________________________________________

	void Nodewires::_nodeRemoved(int nodeId)
	{
		//TODO: Only render what is needed for removed wires.

		auto it = std::remove_if(m_wires.begin(), m_wires.end(), [nodeId,this](const Wire& wire)
		{
			if( wire.fromNode == nodeId || wire.toNode == nodeId )
			{
				this->_requestRenderWire( wire );
				return true;
			}
			else
				return false;
		} );

		if( it != m_wires.end() )
		{
			m_wires.erase( it, m_wires.end() );
			_requestRender();
		}
	}

	//____ _nodeHidden() _________________________________________________________

	void Nodewires::_nodeHidden(int nodeId)
	{
		for( auto& wire : m_wires )
		{
			if( wire.bVisible && (wire.fromNode == nodeId || wire.toNode == nodeId) )
			{
				_requestRenderWire( wire );
				wire.bVisible = false;
			}
		}
	}

	//____ _nodeUnhidden() _______________________________________________________

	void Nodewires::_nodeUnhidden(int nodeId)
	{
		for( auto& wire : m_wires )
		{
			if( (wire.fromNode == nodeId && m_pObserved->nodes.find(wire.toNode)->isVisible()) ||
				(wire.toNode == nodeId && m_pObserved->nodes.find(wire.fromNode)->isVisible()) )
			{
				wire.bVisible = true;
				_requestRenderWire( wire );
			}
		}
	}

	//____ _updateWirePositions() ________________________________________________

	void Nodewires::_updateWirePositions( Wire& wire, Node * pFromNode, Node * pToNode )
	{
		auto& fromRect = pFromNode->geoSPX();
		auto& toRect = pToNode->geoSPX();

		CoordSPX fromPos = Util::placementToOfs(wire.fromPlacement, fromRect);
		CoordSPX toPos = Util::placementToOfs(wire.toPlacement, toRect);

		wire.fromPos = fromPos;
		wire.toPos = toPos;
	}

	//____ _requestRenderWire() __________________________________________________

	void Nodewires::_requestRenderWire( const Wire& wire )
	{
		RectSPX area = { wire.fromPos, SizeSPX(wire.toPos - wire.fromPos) };

		if( area.w < 0 )
		{
			area.x += area.w;
			area.w = -area.w;
		}

		if( area.h < 0 )
		{
			area.y += area.h;
			area.h = -area.h;
		}

		area += BorderSPX( m_renderMargin );

		_requestRender(area);
	}


}
