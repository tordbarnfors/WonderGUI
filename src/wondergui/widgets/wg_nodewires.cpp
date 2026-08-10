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

#include <utility>

namespace wg
{
	using namespace Util;

	const TypeInfo NodeWires::TYPEINFO = { "NodeWires", &Widget::TYPEINFO };

	//____ destructor ____________________________________________________________

	NodeWires::~NodeWires()
	{
		if( m_pObserved )
			m_pObserved->_removeObserver(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& NodeWires::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ attachTo() ____________________________________________________________

	bool NodeWires::attachTo( NodePanel * pNodeSource )
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

	void NodeWires::detach()
	{
		if( m_pObserved )
		{
			m_pObserved->_removeObserver(this);
			m_pObserved = nullptr;
			m_wires.clear();
		}
	}

	//____ addWire() _____________________________________________________________

	bool NodeWires::addWire( int fromNode, Placement fromPos, int toNode, Placement toPos )
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

		_updateWirePositionDirection( m_wires.back(), &(*itFrom), &(*itTo) );

		return true;
	}

	//____ removeWire() __________________________________________________________

	bool NodeWires::removeWire( int fromNode, int toNode )
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

	//____ setWireColor() ________________________________________________________

	bool NodeWires::setWireColor( HiColor color )
	{
		if( !color.isValid() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Color is invalid.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_wireColor = color;
		_requestRender();
		return true;
	}

	//____ setWireStub() ____________________________________________________

	void NodeWires::setWireStub( pts length )
	{
		if( length < 0.0 || length > 100 )
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "WireStub capped to be between 0.0 and 100.", this, &TYPEINFO, __func__, __FILE__, __LINE__);

			limit( length, 0.0, 100 );
		}

		m_wireStub = length;
		_requestRender();
	}


	//____ setWireThickness() ____________________________________________________

	void NodeWires::setWireThickness( pts thickness )
	{
		if( thickness < 0.1 || thickness > 100 )
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "Wire thickness capped to be between 0.1 and 100.", this, &TYPEINFO, __func__, __FILE__, __LINE__);

			limit( thickness, 0.1, 100 );
		}

		m_wireThickness = thickness;
		_refreshRenderMargin();
		_requestRender();
	}

	//____ _refreshRenderMargin() ___________________________________________________

	void NodeWires::_refreshRenderMargin()
	{
		m_renderMargin = alignUp((ptsToSpx(m_wireThickness, m_scale) / 2) + 128);
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX NodeWires::_defaultSize(int scale) const
	{
		if( m_pObserved )
			return m_pObserved->_defaultSize(scale);

		return Widget::_defaultSize(scale);
	}

	//____ _render() _____________________________________________________________

	void NodeWires::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Widget::_render( pDevice, _canvas, _window );

		spx wireThickness = ptsToSpx(m_wireThickness, m_scale);

		if( m_bOrthogonal )
		{
			CoordSPX coordList[8];

			for( auto& wire : m_wires )
			{
				if( wire.bVisible )
				{
					int nCoords =  _routeOrthogonal( wire.fromPos + _canvas.pos(), wire.fromDirection, wire.toPos + _canvas.pos(), wire.toDirection, coordList );

					for( int i = 0 ; i < nCoords -1 ; i++ )			// First and last is from/toPos, which we already have.
						pDevice->drawLine(coordList[i], coordList[i+1], m_wireColor, wireThickness );
				}
			}
		}
		else
		{
			for( auto& wire : m_wires )
			{
				if( wire.bVisible )
					pDevice->drawLine(wire.fromPos + _canvas.pos(), wire.toPos + _canvas.pos(), m_wireColor, wireThickness );
			}
		}
	}

	//____ _resize() _____________________________________________________________

	void NodeWires::_resize(const SizeSPX& size, int scale)
	{
		Widget::_resize(size, scale);
		_refreshRenderMargin();
	}

	//____ _nodeCanvasResized() __________________________________________________

	void NodeWires::_nodeCanvasResized(SizeSPX oldSize, SizeSPX newSize)
	{
		_requestResize();
	}

	//____ _nodeCanvasDestroyed() ________________________________________________

	void NodeWires::_nodeCanvasDestroyed()
	{
		m_pObserved = nullptr;
		m_wires.clear();
	}

	//____ _nodeMovedOrResized() _________________________________________________

	void NodeWires::_nodeMovedOrResized(int nodeId, const RectSPX& oldGeo, const RectSPX& newGeo)
	{
		for( auto& wire : m_wires )
		{
			if( wire.bVisible && (wire.fromNode == nodeId || wire.toNode == nodeId) )
			{
				_requestRenderWire( wire );
				_updateWirePositionDirection( wire, &(*m_pObserved->nodes.find(wire.fromNode)), &(*m_pObserved->nodes.find(wire.toNode)) );
				_requestRenderWire( wire );
			}
		}
	}

	//____ _nodeAdded() __________________________________________________________

	void NodeWires::_nodeAdded(int nodeId, const RectSPX& geo, bool bVisible )
	{
		// Do nothing.
	}

	//____ _nodeRemoved() ________________________________________________________

	void NodeWires::_nodeRemoved(int nodeId)
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

	void NodeWires::_nodeHidden(int nodeId)
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

	void NodeWires::_nodeUnhidden(int nodeId)
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

	//____ _updateWirePositionDirection() ________________________________________________

	void NodeWires::_updateWirePositionDirection( Wire& wire, NodePanel::Node * pFromNode, NodePanel::Node * pToNode )
	{

		const auto& fromRect = pFromNode->slot()->_geo();
		const auto& toRect = pToNode->slot()->_geo();

		CoordSPX fromPos = Util::placementToOfs(wire.fromPlacement, fromRect);
		CoordSPX toPos = Util::placementToOfs(wire.toPlacement, toRect);

		wire.fromPos = fromPos;
		wire.toPos = toPos;

		wire.fromDirection = _placementToDirection(wire.fromPlacement, wire.fromPos, wire.toPos);
		wire.toDirection = _placementToDirection(wire.toPlacement, wire.toPos, wire.fromPos);
	}

	//____ _placementToDirection() _______________________________________________

	Direction NodeWires::_placementToDirection( Placement placement, CoordSPX myPos, CoordSPX otherPos )
	{
		switch( placement )
		{
			case Placement::Undefined:
			case Placement::Center:
			{
				if( abs(myPos.x - otherPos.x) > abs(myPos.y - otherPos.y) )
				{
					if( myPos.x > otherPos.x )
						return Direction::Left;
					else
						return Direction::Right;
				}
				else
				{
					if( myPos.y > otherPos.y )
						return Direction::Up;
					else
						return Direction::Down;
				}

			}

			case Placement::NorthWest:
			case Placement::NorthEast:
			case Placement::North:
				return Direction::Up;
			case Placement::East:
				return Direction::Right;
			case Placement::SouthEast:
			case Placement::SouthWest:
			case Placement::South:
				return Direction::Down;
			case Placement::West:
				return Direction::Left;
		}

	}


	//____ _requestRenderWire() __________________________________________________

	void NodeWires::_requestRenderWire( const Wire& wire )
	{
		RectSPX area = { wire.fromPos, wire.toPos };

		if( m_bOrthogonal)
		{
			CoordSPX coordList[6];

			int nCoords =  _routeOrthogonal( wire.fromPos, wire.fromDirection, wire.toPos, wire.toDirection, coordList );

			for( int i = 1 ; i < nCoords -1 ; i++ )			// First and last is from/toPos, which we already have.
				area.growToContain(coordList[i]);
		}

		area += BorderSPX( m_renderMargin );
		_requestRender(area);
 	}

	//____ _routeOrthogonal() ____________________________________________________

	int NodeWires::_routeOrthogonal( CoordSPX beginPos, Direction beginDir, CoordSPX endPos, Direction endDir, CoordSPX coordList[6] )
	{

		spx		stubLength = ptsToSpx(m_wireStub, m_scale);

		int			nCoords = 0;

		if( int(beginDir) > int(endDir) )
		{
			std::swap(beginPos, endPos);
			std::swap(beginDir, endDir);
		}

		coordList[nCoords++] = beginPos;

		if( beginDir == Direction::Up )
		{
			if( endDir == Direction::Up )				// Up -> Up
			{
				spx yPos = std::min( beginPos.y, endPos.y ) - stubLength;

				coordList[nCoords++] = { beginPos.x, yPos };
				coordList[nCoords++] = { endPos.x, yPos };
			}
			else if( endDir == Direction::Right )		// Up -> Right
			{
				if( endPos.x + stubLength < beginPos.x || (endPos.x < beginPos.x && abs(beginPos.y -endPos.y) < stubLength*2) )		// Line up and then to the right
				{
					if( endPos.y <= beginPos.y - stubLength )
						coordList[nCoords++] = { beginPos.x, endPos.y };
					else
					{
						CoordSPX beginStub = { beginPos.x, beginPos.y - stubLength };

						spx midX = (endPos.x + beginPos.x)/2;

						coordList[nCoords++] = beginStub;
						coordList[nCoords++] = { midX, beginStub.y };
						coordList[nCoords++] = { midX, endPos.y };
					}
				}
				else							// Line up and then to the left
				{
					CoordSPX beginStub = { beginPos.x, beginPos.y - stubLength };
					CoordSPX endStub = { endPos.x + stubLength, endPos.y };

					if( beginStub.y  > endPos.y )
						beginStub.y = (endPos.y + beginPos.y)/2;	// Split in middle instead.

					coordList[nCoords++] = beginStub;
					coordList[nCoords++] = { endStub.x, beginStub.y };
					coordList[nCoords++] = endStub;
				}
			}
			else if( endDir == Direction::Down )		// Up -> Down
			{
				if( beginPos.x != endPos.x || beginPos.y < endPos.y )
				{
					if( ( beginPos.y - stubLength * 2 >= endPos.y ) || (beginPos.y >= endPos.y && abs(beginPos.x -endPos.x) < stubLength*2) )
					{
						spx midY = (beginPos.y + endPos.y) / 2;

						coordList[nCoords++] = { beginPos.x, midY };
						coordList[nCoords++] = { endPos.x, midY };
					}
					else
					{
						CoordSPX beginStub = { beginPos.x, beginPos.y - stubLength };
						CoordSPX endStub = { endPos.x, endPos.y + stubLength };

						spx midX = (beginPos.x + endPos.x) / 2;

						coordList[nCoords++] = beginStub;
						coordList[nCoords++] = { midX, beginStub.y };
						coordList[nCoords++] = { midX, endStub.y };
						coordList[nCoords++] = endStub;
					}
				}
			}
			else										// Up -> Left
			{
				if( endPos.x - stubLength > beginPos.x || (endPos.x > beginPos.x && abs(beginPos.y -endPos.y) < stubLength*2) )		// Line up and then to the right
				{
					if( endPos.y <= beginPos.y - stubLength )
						coordList[nCoords++] = { beginPos.x, endPos.y };
					else
					{
						CoordSPX beginStub = { beginPos.x, beginPos.y - stubLength };

						spx midX = (endPos.x + beginPos.x)/2;

						coordList[nCoords++] = beginStub;
						coordList[nCoords++] = { midX, beginStub.y };
						coordList[nCoords++] = { midX, endPos.y };
					}
				}
				else							// Line up and then to the left
				{
					CoordSPX beginStub = { beginPos.x, beginPos.y - stubLength };
					CoordSPX endStub = { endPos.x - stubLength, endPos.y };

					if( beginStub.y  > endPos.y )
						beginStub.y = (endPos.y + beginPos.y)/2;	// Split in middle instead.

					coordList[nCoords++] = beginStub;
					coordList[nCoords++] = { endStub.x, beginStub.y };
					coordList[nCoords++] = endStub;
				}
			}

		}
		else if( beginDir == Direction::Right )
		{
			if( endDir == Direction::Right )			// Right -> Right
			{
				spx xPos = std::max( beginPos.x, endPos.x ) + stubLength;

				coordList[nCoords++] = { xPos, beginPos.y };
				coordList[nCoords++] = { xPos, endPos.y };
			}
			else if( endDir == Direction::Down )		// Right -> Down
			{
				if( endPos.y + stubLength < beginPos.y || (endPos.y < beginPos.y && abs(beginPos.x -endPos.x) < stubLength*2) )
				{
					if( endPos.x >= beginPos.x + stubLength || (endPos.x > beginPos.x && (beginPos.y < endPos.y + stubLength*2)) )
						coordList[nCoords++] = { endPos.x, beginPos.y };
					else
					{
						CoordSPX beginStub = { beginPos.x + stubLength, beginPos.y };

						spx midY = (endPos.y + beginPos.y)/2;

						coordList[nCoords++] = beginStub;
						coordList[nCoords++] = { beginStub.x, midY };
						coordList[nCoords++] = { endPos.x, midY };
					}
				}
				else
				{
					CoordSPX beginStub = { beginPos.x + stubLength, beginPos.y };
					CoordSPX endStub = { endPos.x, endPos.y + stubLength };

					if( beginStub.x  < endPos.x )
						beginStub.x = (endPos.x + beginPos.x)/2;	// Split in middle instead.

					coordList[nCoords++] = beginStub;
					coordList[nCoords++] = { beginStub.x, endStub.y };
					coordList[nCoords++] = endStub;
				}

			}
			else										// Right -> Left
			{
				if( beginPos.y != endPos.y || beginPos.x > endPos.x )
				{
					if( ( beginPos.x + stubLength * 2 <= endPos.x ) || (beginPos.x <= endPos.x && abs(beginPos.y -endPos.y) < stubLength*2) )
					{
						spx midX = (beginPos.x + endPos.x) / 2;

						coordList[nCoords++] = { midX, beginPos.y };
						coordList[nCoords++] = { midX, endPos.y };
					}
					else
					{
						CoordSPX beginStub = { beginPos.x + stubLength, beginPos.y };
						CoordSPX endStub = { endPos.x - stubLength, endPos.y };

						spx midY = (beginPos.y + endPos.y) / 2;

						coordList[nCoords++] = beginStub;
						coordList[nCoords++] = { beginStub.x, midY };
						coordList[nCoords++] = { endStub.x, midY };
						coordList[nCoords++] = endStub;
					}
				}

			}

		}
		else if( beginDir == Direction::Down )
		{
			if( endDir == Direction::Down )			// Down -> Down
			{
				spx yPos = std::max( beginPos.y, endPos.y ) + stubLength;

				coordList[nCoords++] = { beginPos.x, yPos };
				coordList[nCoords++] = { endPos.x, yPos };
			}
			else										// Down -> Left
			{
				if( endPos.x - stubLength > beginPos.x || (endPos.x > beginPos.x && abs(beginPos.y - endPos.y) < stubLength*2) )		// Line down and then to the right
				{
					if( endPos.y >= beginPos.y + stubLength || (endPos.y >= beginPos.y && endPos.x < beginPos.x + stubLength*2)  )
						coordList[nCoords++] = { beginPos.x, endPos.y };
					else
					{
						CoordSPX beginStub = { beginPos.x, beginPos.y + stubLength };

						spx midX = (endPos.x + beginPos.x)/2;

						coordList[nCoords++] = beginStub;
						coordList[nCoords++] = { midX, beginStub.y };
						coordList[nCoords++] = { midX, endPos.y };
					}
				}
				else							// Line down and then to the left
				{
					CoordSPX beginStub = { beginPos.x, beginPos.y + stubLength };
					CoordSPX endStub = { endPos.x - stubLength, endPos.y };

					if( beginStub.y < endPos.y )
						beginStub.y = (endPos.y + beginPos.y)/2;	// Split in middle instead.

					coordList[nCoords++] = beginStub;
					coordList[nCoords++] = { endStub.x, beginStub.y };
					coordList[nCoords++] = endStub;
				}

			}

		}
		else											// Left -> Left
		{
			spx xPos = std::min( beginPos.x, endPos.x ) - stubLength;

			coordList[nCoords++] = { xPos, beginPos.y };
			coordList[nCoords++] = { xPos, endPos.y };
		}

		coordList[nCoords++] = endPos;
		return nCoords;
	}

}
