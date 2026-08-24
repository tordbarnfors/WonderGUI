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

		Coord	setCenter(Coord pos);					// Note: center position is within parent contentRect, not canvas.
		Coord	center() const { return m_center; };	// "-

	protected:

		NodePanelSlot(SlotHolder* pHolder) : PanelSlot(pHolder) {}

		NodePanelSlot(NodePanelSlot&& o) = default;
		NodePanelSlot& operator=(NodePanelSlot&& o) = default;

		bool _setBlueprint(const Blueprint& bp);

		int 	m_nodeId;
		Coord 	m_center;
	};



	//____ NodePanel __________________________________________________________

	class NodePanel : public PanelTemplate<NodePanelSlot>
	{
		friend class Node;
		friend class NodePanelSlot;

	public:

		//_____ Node _________________________________________________________________

		class Node
		{
			friend class NodePanel;

		public:
			inline int 				id() const { return m_id; }
			inline Widget * 		widget() const { return m_pWidget; }

			void					setVisible(bool bVisible) { ((NodePanelSlot*) m_pWidget->_slot())->setVisible(bVisible); }
			inline bool				isVisible() const { return ((NodePanelSlot*) m_pWidget->_slot())->m_bVisible; }

			Coord					setCenter(Coord pos) { return ((NodePanelSlot*) m_pWidget->_slot())->setCenter(pos); }
			Coord					center() const { return ((NodePanelSlot*) m_pWidget->_slot())->center(); }

			Rect					geo() const;

			inline	DynamicSlotVector<NodePanelSlot>::iterator	slot() const { return (NodePanelSlot*) m_pWidget->_slot(); }

		private:
			Node(int nodeId, Widget * pWidget) : m_id(nodeId), m_pWidget(pWidget) {};

			int			m_id;
			Widget * 	m_pWidget;

		};

		//____ NodeConstraint _______________________________________________________

		enum class NodeConstraint
		{
			Center,
			Bounds
		};

		//____ NodeVector ____________________________________________________________

		class NodeVector : public StaticVector<Node>
		{
			friend class NodePanel;
		public:
			iterator 	find(int nodeId);
			bool 		has(int nodeId);
		};

		//____ Observer __________________________________________________________

		class Observer
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

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Size			defaultSize = {256,256};
			bool			disabled = false;
			MouseButton		dragButton = MouseButton::Left;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			MaskOp			maskOp = MaskOp::Skip;
			NodeConstraint	nodeConstraint = NodeConstraint::Bounds;
			std::function<Coord(const NodePanel * pPanel, NodeVector::const_iterator nodeIt, Coord pos)> nodePosModifier;
			bool			normalized = false;					// True = Node positioning go from 0.0 to 1.0.
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
		void			setNodePosModifier( const std::function<Coord(const NodePanel * pPanel, NodeVector::const_iterator nodeIt, Coord pos)>& callback );

		void			setNodeConstraint( NodeConstraint constraint );
		NodeConstraint	nodeConstraint() const { return m_nodeConstraint; }

		void			selectNode( int nodeId );
		int				selectedNode() const { return m_pSelectedChild ? static_cast<NodePanelSlot*>(m_pSelectedChild->_slot())->m_nodeId : 0; }

		//.____ Internal ______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;
		bool			_addObserver( Observer * pObserver );
		bool			_removeObserver( Observer * pObserver );

	protected:

		NodePanel();
		template<class BP> NodePanel(const BP& bp) : PanelTemplate(bp)
		{
			m_nodeConstraint	= bp.nodeConstraint;
			m_defaultSize		= bp.defaultSize;
			m_nodePosModifier	= bp.nodePosModifier;
			m_bNormalized		= bp.normalized;
			m_dragButton		= bp.dragButton;

			m_size				= Util::ptsToSpx(m_defaultSize,64);
		}

		~NodePanel();

		void		_updateNodeGeo( NodePanelSlot * pSlot, Coord center, bool bRequestRender );


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

		Widget*		m_pSelectedChild = nullptr;
		Widget*		m_pDraggedChild = nullptr;
		Coord		m_draggedChildStartPos;

		NodeConstraint	m_nodeConstraint = NodeConstraint::Bounds;

		bool			m_bNormalized = false;
		MouseButton		m_dragButton = MouseButton::Left;

		std::vector<Observer*>	m_observers;
		std::function<Coord(const NodePanel * pPanel, NodeVector::const_iterator nodeIt, Coord pos)> m_nodePosModifier;
	};
}

#endif //WG_NODEPANEL_DOT_H
