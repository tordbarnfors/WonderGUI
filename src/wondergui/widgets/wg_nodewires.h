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
#ifndef WG_NODEWIRES_DOT_H
#define WG_NODEWIRES_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_nodepanel.h>

namespace wg
{
class NodeWires;
typedef StrongPtr<NodeWires>	NodeWires_p;
typedef WeakPtr<NodeWires>		NodeWires_wp;

class NodeWires : public Widget, public NodePanel::Observer
{
public:

	struct Blueprint
	{
		Object_p		baggage;
		bool			disabled = false;
		bool			dropTarget = false;
		Finalizer_p		finalizer = nullptr;
		int				id = 0;
		MarkPolicy		markPolicy = MarkPolicy::Undefined;
		bool			pickable = false;
		uint8_t			pickCategory = 0;
		bool			pickHandle = false;
		PointerStyle	pointer = PointerStyle::Undefined;
		bool			selectable = false;
		Skin_p			skin;
		bool			stickyFocus = false;
		bool			tabLock = false;
		String			tooltip;

		HiColor			wireColor = HiColor::Black;
		pts				wireThickness = 1.f;
	};

	//.____ Creation __________________________________________

	static NodeWires_p	create() { return NodeWires_p(new NodeWires()); }
	static NodeWires_p	create( const Blueprint& blueprint ) { return NodeWires_p(new NodeWires(blueprint)); }

	//.____ Identification __________________________________________

	const TypeInfo&	typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	//.____ Misc _____________________________________________________________

	bool	attachTo( NodePanel * pNodeSource );
	void	detach();

	bool	addWire( int fromNode, Placement fromPos, int toNode, Placement toPos );
	bool	removeWire( int fromNode, int toNode );

	bool	setWireColor( HiColor color );
	HiColor	wireColor() const { return m_wireColor; }

	void	setWireThickness( pts thickness );
	pts		wireThickness() const { return m_wireThickness; }

private:
	NodeWires() {};
	template< class BP> NodeWires( const BP& bp ) : Widget(bp)
	{
		m_wireColor = bp.wireColor;
		m_wireThickness = bp.wireThickness;
		_refreshRenderMargin();

	}

	virtual ~NodeWires();

	struct Wire
	{
		int			fromNode;
		Placement	fromPlacement;
		int			toNode;
		Placement	toPlacement;

		bool		bVisible;
		CoordSPX	fromPos;
		CoordSPX	toPos;
	};


	SizeSPX		_defaultSize(int scale) const override;
	void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
	void		_resize(const SizeSPX& size, int scale) override;

	void 		_requestRenderWire( const Wire& wire );
	void		_updateWirePositions( Wire& wire, NodePanel::Node * pFromNode, NodePanel::Node * pToNode  );
	void		_refreshRenderMargin();


//	void 		_requestRenderAffectedWires( int nodeId );


	// Overloaded from NodeObserver

	void	_nodeCanvasResized(SizeSPX oldSize, SizeSPX newSize) override;
	void	_nodeCanvasDestroyed() override;
	void	_nodeMovedOrResized(int nodeId, const RectSPX& oldGeo, const RectSPX& newGeo) override;
	void	_nodeAdded(int nodeId, const RectSPX& geo, bool bVisible ) override;
	void	_nodeRemoved(int nodeId) override;
	void	_nodeHidden(int nodeId) override;
	void	_nodeUnhidden(int nodeId) override;


	std::vector<Wire>	m_wires;

	NodePanel *		m_pObserved = nullptr;

	spx				m_renderMargin = 64+64; 			// Size of border around wire positions that needs to be rendered to be on the safe side.
	HiColor			m_wireColor = HiColor::Black;
	pts				m_wireThickness = 1;
};


}

#endif //WG_NODEWIRES_DOT_H
