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
		template<class S> friend class DynamicSlotVector;

		//.____ Blueprint _______________________________________________________

		struct Blueprint
		{
			bool	visible = true;
		};

		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

	protected:

		NodePanelSlot(SlotHolder* pHolder) : PanelSlot(pHolder) {}

		NodePanelSlot(NodePanelSlot&& o) = default;
		NodePanelSlot& operator=(NodePanelSlot&& o) = default;

		bool _setBlueprint(const Blueprint& bp);

	};


	//____ NodePanel __________________________________________________________

	class NodePanel : public PanelTemplate<NodePanelSlot>
	{
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

		//.____ Identification __________________________________________
		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ________________________________________________________

		void			setDefaultSize(Size size);

		//.____ Internal ______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;

	protected:

		NodePanel();
		template<class BP> NodePanel(const BP& bp) : PanelTemplate(bp)
		{
		}

		~NodePanel();

		// Overloaded from Widget

		void		_receive(Msg* pMsg) override;
		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		
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


	};


}

#endif //WG_NODEPANEL_DOT_H