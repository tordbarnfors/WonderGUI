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
#ifndef WG_NODEPANEL_DOT_H
#define WG_NODEPANEL_DOT_H
#pragma once

#include <wg_panel.h>
#include <wg_staticvector.h>

namespace wg
{
	class	NodePanel;
	typedef	StrongPtr<NodePanel>	NodePanel_p;
	typedef	WeakPtr<NodePanel>		NodePanel_wp;

	class Node;

	//____ NodeObserver __________________________________________________________

	class NodeObserver
	{
	public:
		virtual void	_nodeCanvasResized(SizeSPX oldSize, SizeSPX newSize) = 0;
		virtual void	_nodeCanvasDestroyed() = 0;
		virtual void	_nodeMovedOrResized(int nodeId, const RectSPX& oldGeo, const RectSPX& newGeo) = 0;
		virtual void	_nodeAdded(int nodeId, const RectSPX& geo, bool bVisible ) = 0;
		virtual void	_nodeRemoved(int nodeId) = 0;
		virtual void	_nodeHidden(int nodeId) = 0;
		virtual void	_nodeUnhidden(int nodeId) = 0;
	};


	//____ NodeVector ____________________________________________________________

	class NodeVector : public StaticVector<Node>
	{
		friend class NodePanel;
	public:
		iterator 	find(int nodeId);
		bool 		has(int nodeId);
	};


	//____ NodePanelSlot __________________________________________________________

	class NodePanelSlot : public PanelSlot
	{
	public:

		friend class NodePanel;
		friend class Node;
		template<class S> friend class DynamicSlotVector;

		//.____ Blueprint _______________________________________________________

		struct Blueprint
		{
			Coord	center;
			int		nodeId = 0;
			bool	visible = true;
		};

		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

		int		nodeId() const { return m_nodeId; };

		//.____ Geometry ______________________________________________________

		void	setCenter(Coord pos);
		Coord	center() const { return m_center; };


	protected:

		NodePanelSlot(SlotHolder* pHolder) : PanelSlot(pHolder) {}

		NodePanelSlot(NodePanelSlot&& o) = default;
		NodePanelSlot& operator=(NodePanelSlot&& o) = default;

		bool _setBlueprint(const Blueprint& bp);

		int 	m_nodeId;
		Coord 	m_center;

	};

	//_____ Node _________________________________________________________________

	class Node
	{
		friend class NodePanel;

	public:
		inline int 				id() const { return m_id; }
		inline Widget * 		widget() const { return m_pWidget; }
		inline const RectSPX& 	geoSPX() const { return ((NodePanelSlot*) m_pWidget->_slot())->m_geo; }
		inline const CoordSPX 	centerSPX() const { return ((NodePanelSlot*) m_pWidget->_slot())->m_geo.center(); }

		void					setVisible(bool bVisible) { ((NodePanelSlot*) m_pWidget->_slot())->setVisible(bVisible); }
		inline bool				isVisible() const { return ((NodePanelSlot*) m_pWidget->_slot())->m_bVisible; }

		Coord					setCenter(Coord pos) { ((NodePanelSlot*) m_pWidget->_slot())->setCenter(pos); }
		Coord					center() const { return ((NodePanelSlot*) m_pWidget->_slot())->center(); }

		inline	DynamicSlotVector<NodePanelSlot>::iterator	slot() const { return (NodePanelSlot*) m_pWidget->_slot(); }

	private:
		Node(int nodeId, Widget * pWidget) : m_id(nodeId), m_pWidget(pWidget) {};

		int			m_id;
		Widget * 	m_pWidget;

	};



	//____ NodePanel __________________________________________________________

	class NodePanel : public PanelTemplate<NodePanelSlot>
	{
		friend class Node;
		friend class NodePanelSlot;

	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			MaskOp			maskOp = MaskOp::Skip;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation ______________________________________________________

		static NodePanel_p	create() { return NodePanel_p(new NodePanel()); }
		static NodePanel_p	create(const Blueprint& blueprint) { return NodePanel_p(new NodePanel(blueprint)); }

		//.____ Components _______________________________________

		NodeVector		nodes;

		//.____ Identification __________________________________________
		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ________________________________________________________

		void			setDefaultSize(Size size);

		//.____ Misc ________________________________________________________________

		void			clearNodePosModifier();
		void			setNodePosModifier( const std::function<CoordSPX(const NodePanel * pPanel, NodeVector::const_iterator nodeIt, CoordSPX pos)>& callback );

		//.____ Internal ______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;
		bool			_addObserver( NodeObserver * pObserver );
		bool			_removeObserver( NodeObserver * pObserver );

	protected:

		NodePanel();
		template<class BP> NodePanel(const BP& bp) : PanelTemplate(bp)
		{
		}

		~NodePanel();

		void		_updateNodeGeo( NodePanelSlot * pSlot, CoordSPX center, bool bRequestRender );


		// Overloaded from Widget

		void		_receive(Msg* pMsg) override;
		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		void		_resize(const SizeSPX& size, int scale) override;

		// Overloaded from Container

		const TypeInfo& _slotTypeInfo(const StaticSlot* pSlot) const override;

		void		_childRequestRender(StaticSlot* pSlot, const RectSPX& rect) override;
		void		_childRequestResize(StaticSlot* pSlot) override;

		void		_releaseChild(StaticSlot* pSlot) override;
		void		_replaceChild(StaticSlot* pSlot, Widget* pNewChild) override;

		void		_didAddSlots(StaticSlot* pSlot, int nb) override;
		void		_didMoveSlots(StaticSlot* pFrom, StaticSlot* pTo, int nb) override;
		void		_willEraseSlots(StaticSlot* pSlot, int nb) override;
		void		_hideSlots(StaticSlot*, int nb) override;
		void		_unhideSlots(StaticSlot*, int nb) override;

		//

		Size		m_defaultSize = { 256, 256 };

		Widget*		m_pDraggedChild = nullptr;
		CoordSPX	m_draggedChildStartPos;



		std::vector<NodeObserver*>	m_observers;
		std::function<CoordSPX(const NodePanel * pPanel, NodeVector::const_iterator nodeIt, CoordSPX pos)> m_nodePosModifier;
	};
}

#endif //WG_NODEPANEL_DOT_H
