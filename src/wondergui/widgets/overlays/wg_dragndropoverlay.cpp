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
#include <wg_dragndropoverlay.h>
#include <wg_base.h>
#include <wg_msgrouter.h>
#include <wg_msg.h>
#include <wg_gfxdevice.h>
#include <wg_surfacefactory.h>
#include <wg_patches.h>
#include <wg_image.h>
#include <wg_root.h>



#include <wg_filler.h>
#include <wg_boxskin.h>

namespace wg
{
	using namespace Util;
	const TypeInfo DragNDropOverlay::TYPEINFO = { "DragNDropOverlay", &Overlay::TYPEINFO };
	const TypeInfo DragNDropOverlay::Slot::TYPEINFO = { "DragNDropOverlay::Slot", &Overlay::Slot::TYPEINFO };

	//____ constructor ____________________________________________________________

	DragNDropOverlay::DragNDropOverlay() : m_dragSlot(this)
	{
		m_dragStartTreshold = 3;
	}

	//____ Destructor _____________________________________________________________

	DragNDropOverlay::~DragNDropOverlay()
	{
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& DragNDropOverlay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ isDragInProgress() ____________________________________________________

	bool DragNDropOverlay::isDragInProgress() const
	{
		return m_dragState != DragState::Idle;
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	DragNDropOverlay::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		if (pSlot == &mainSlot)
			return DynamicSlot::TYPEINFO;

		return Slot::TYPEINFO;
	}

	//____ _findWidget() __________________________________________________________

	Widget * DragNDropOverlay::_findWidget( const CoordSPX& ofs, SearchMode mode )
	{
		// Widget being dragged is totally transparent to the mouse, so we just
		// forward to our child.

		if( mainSlot._widget() )
		{
			Widget * pWidget = mainSlot._widget();

			if (pWidget->isContainer())
			{
				Widget * pRes = static_cast<Container*>(pWidget)->_findWidget(ofs, mode);
				if (pRes)
					return pRes;
			}
			else if (mode == SearchMode::Geometry || mainSlot._widget()->_markTest(ofs))
				return mainSlot._widget();
		}
		else if( mode == SearchMode::Geometry )
			return this;

		return nullptr;
	}

	//____ _childRequestResize() ______________________________________________

	void DragNDropOverlay::_childRequestResize(StaticSlot * pSlot)
	{
		if( pSlot == &mainSlot )
			_requestResize();
		else
		{
			auto p = static_cast<Slot*>(pSlot);

			SizeSPX pref = p->_widget()->_defaultSize(m_scale);
			SizeSPX max = SizeSPX::max(pref, p->m_geo.size());

			RectSPX renderBounds = RectSPX{ p->m_geo.pos(), max } + p->_widget()->_overflow();

			_requestRender(RectSPX::overlap({ 0,0,m_size }, renderBounds));
			p->_setSize(pref, m_scale);
		}
	}

	//____ _releaseChild() ____________________________________________________

	void DragNDropOverlay::_releaseChild(StaticSlot * pSlot)
	{
		if (pSlot == &mainSlot)
			Overlay::_releaseChild(pSlot);
		else
			_replaceDragWidget(nullptr);
	}

	//____ _beginOverlaySlots() _______________________________________________

	const Overlay::Slot * DragNDropOverlay::_beginOverlaySlots() const
	{
		return &m_dragSlot;
	}

	//____ _endOverlaySlots() ________________________________________________

	const Overlay::Slot * DragNDropOverlay::_endOverlaySlots() const
	{
		if( m_dragSlot._widget() )
			return (&m_dragSlot) + 1;
		else
			return &m_dragSlot;
	}

	//____ _sizeOfOverlaySlot() _______________________________________________

	int DragNDropOverlay::_sizeOfOverlaySlot() const
	{
		return sizeof(Slot);
	}

	//____ _onRequestRender() _______________________________________________

	void DragNDropOverlay::_onRequestRender(const RectSPX& rect, const Overlay::Slot * pSlot)
	{
		// We don't mask against drag widget, it is assumed to be too small/transparent/irregular
		// for that to make sense.

		// Clip our geometry and pass it on

		_requestRender( RectSPX::overlap( rect, RectSPX(0,0,m_size) ));
	}

	//____ _receive() __________________________________________________________

	void DragNDropOverlay::_receive( Msg * _pMsg )
	{
		switch (_pMsg->type())
		{
			case MsgType::MouseDrag:
			{
				auto pMsg = static_cast<MouseDragMsg*>(_pMsg);
				if (pMsg->button() != MouseButton::Left)
					break;

				switch (m_dragState)
				{
					case DragState::Picking:
					{
						CoordSPX total = pMsg->_draggedTotal();
						if (abs(total.x) + abs(total.y) > ptsToSpx(m_dragStartTreshold,m_scale))
						{
							CoordSPX pickOfsSPX = pMsg->_startPos() - m_pPicked->_globalGeo().pos();

							Coord pickOfs = spxToPts(pickOfsSPX, m_pPicked->scale());

							Base::msgRouter()->post(new PickMsg(m_pPicked, pickOfs, this, pMsg->modKeys(), pMsg->pointerPos()));
							m_dragState = DragState::Picked;
						}
						break;
					}

					case DragState::Dragging:
					{
						CoordSPX pointerPos = pMsg->pointerSpxPos();
						CoordSPX hotspotSPX =  pointerPos + m_hotspotOfs;

						if( m_dragRestriction != RectSPX(0,0,0,0) )
						{
							hotspotSPX = m_dragRestriction.limit(hotspotSPX);
							pointerPos = hotspotSPX - m_hotspotOfs;
						}

						Coord    hotspotPTS = spxToPts(hotspotSPX, _root()->scale());

						// Move the drag-widget onscreen.

						BorderSPX overflow = m_dragSlot._widget() ? m_dragSlot._widget()->_overflow() : BorderSPX();

						_requestRender(m_dragSlot.m_geo + overflow);
						m_dragSlot.m_geo.setPos( align(pointerPos + m_dragWidgetOfs) );

						_requestRender(m_dragSlot.m_geo + overflow);

// MOVE TO TICK!						// Check if we entered/left a (possible) target.

						CoordSPX ofs = _toLocal(hotspotSPX);

						Widget * pProbed = _findWidget(ofs, SearchMode::ActionTarget );

						while (pProbed && pProbed != this && !pProbed->isDropTarget())
							pProbed = pProbed->parent();

						if( pProbed && pProbed != this && pProbed != m_pProbed )
						{
							m_pProbed = pProbed;
							Base::msgRouter()->post(new DropProbeMsg(pProbed, m_dropType, m_category, m_pDataset, m_pPicked, this, pMsg->modKeys(), hotspotPTS ));
						}

						break;
					}

					case DragState::Targeting:
					{
						CoordSPX pointerPos = pMsg->pointerSpxPos();
						CoordSPX hotspotSPX =  pointerPos + m_hotspotOfs;

						if( m_dragRestriction != RectSPX(0,0,0,0) )
						{
							hotspotSPX = m_dragRestriction.limit(hotspotSPX);
							pointerPos = hotspotSPX - m_hotspotOfs;
						}

						Coord    hotspotPTS = spxToPts(hotspotSPX, _root()->scale());

						// Move the drag-widget onscreen.

						BorderSPX overflow = m_dragSlot._widget() ? m_dragSlot._widget()->_overflow() : BorderSPX();

						_requestRender(m_dragSlot.m_geo + overflow);
						m_dragSlot.m_geo.setPos( align(pointerPos + m_dragWidgetOfs) );
						_requestRender(m_dragSlot.m_geo + overflow);


// MOVE TO TICK!                        // Check if our target has changed

						CoordSPX ofs = _toLocal(hotspotSPX);

						Widget * pHovered = _findWidget(ofs, SearchMode::ActionTarget );

						while (pHovered && pHovered != this && !pHovered->isDropTarget())
							pHovered = pHovered->parent();

						if( pHovered != m_pTargeted )
						{
							// Untarget previous target. Probing possibly new target we leave for next round.

							if( m_pTargeted )
							{
								Base::msgRouter()->post(new DropLeaveMsg(m_pTargeted, m_dropType, m_category, m_pDataset, m_pPicked, pMsg->modKeys(), hotspotPTS));
								Base::msgRouter()->post(new PickedLeaveMsg(m_pPicked,m_pTargeted));
							}

							m_pTargeted = nullptr;
							m_dragState = DragState::Dragging;
						}

						// Send move messages to targeted widget

						if( m_pTargeted )                                   // Check our weak pointer just in case it has been deleted...
						{
							Base::msgRouter()->post(new DropMoveMsg(m_pTargeted, m_dropType, m_category, m_pDataset, m_pPicked, m_dragSlot._widget(), this, pMsg->modKeys(), hotspotPTS));
						}

						break;
					}

					default:
						break;
				}

				break;
			}

			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);
				if (pMsg->button() != MouseButton::Left || m_dragState != DragState::Idle || pMsg->sourceRawPtr() == nullptr )
					break;

				auto pSource = static_cast<Widget*>(pMsg->originalSource().rawPtr());

				while (pSource && pSource != this && !pSource->isPickable() && !pSource->isPickHandle())
					pSource = pSource->_parent();

				Widget * pPickHandle = nullptr;

				if(pSource && pSource != this && pSource->isPickHandle() )
				{
					pPickHandle = pSource;

					while (pSource && pSource != this && !pSource->isPickable())
						pSource = pSource->_parent();
				}

				if (pSource && pSource != this && pSource->isPickable())
				{
					if( pSource->isContainer() && static_cast<Container*>(pSource)->usePickHandles() && pPickHandle == nullptr )
						break;					// Container uses pickHandles but no pickHandle was picked.

					m_pPicked = pSource;
					m_dragState = DragState::Picking;
				}
				break;
			}

			case MsgType::MouseRelease:
			{
				auto pMsg = static_cast<MouseReleaseMsg*>(_pMsg);
				if (pMsg->button() != MouseButton::Left)
					break;

				switch (m_dragState)
				{
					case Picking:
					{
						m_pPicked = nullptr;
						m_dragState = DragState::Idle;
						break;
					}
					case Picked:
					{
						assert(false);		// There should be no mouse release msg before our PickMsg is returned.
					}
					case Dragging:
					{
						_cancel();
						break;
					}
					case Targeting:
					{
						if( m_pTargeted )
						{
							CoordSPX pointerPos = pMsg->pointerSpxPos();
							CoordSPX hotspotSPX =  pointerPos + m_hotspotOfs;

							if( m_dragRestriction != RectSPX(0,0,0,0) )
							{
								hotspotSPX = m_dragRestriction.limit(hotspotSPX);
								pointerPos = hotspotSPX - m_hotspotOfs;
							}

							Coord    hotspotPTS = spxToPts(hotspotSPX, _root()->scale());

							Base::msgRouter()->post(new DropDeliverMsg(m_pTargeted, m_dropType, m_category, m_pDataset, m_pPicked, this, pMsg->modKeys(), hotspotPTS));
							m_dragState = DragState::Delivering;
//							Base::msgRouter()->post(new DropLeaveMsg(m_pTargeted, m_dropType, m_category, m_pDataset, m_pPicked, pMsg->modKeys(), pMsg->pointerPos()));
							m_pTargeted = nullptr;
						}
						else
							_cancel();
						break;
					}
					default:
						break;
				}
				break;
			}

			case MsgType::Pick:
			{
				auto pMsg = static_cast<PickMsg*>(_pMsg);

				assert(m_dragState == DragState::Picked);

				if (pMsg->hasDataset() && m_dragState == DragState::Picked )
				{
					// Grab data from the message

					m_dropType = pMsg->dropType();
					m_category = pMsg->category();
					m_pDataset = pMsg->dataset();
					m_dragRestriction = _toLocal(ptsToSpx(pMsg->dragConfinement(), m_scale));

					// Set/generate drag widget (widget actually dragged across the screen)

					Widget_p pDragWidget = pMsg->dragWidget();
					SizeSPX    dragWidgetSize;

					if (pDragWidget)
					{
						pDragWidget->releaseFromParent();
						m_dragWidgetOfs = ptsToSpx(pMsg->dragWidgetPointerOfs(),m_scale);

						dragWidgetSize = pDragWidget->_defaultSize(m_scale);
						pDragWidget->_resize(dragWidgetSize, m_scale);

						m_dragSlot._setWidget(pDragWidget);

						if( pMsg->hotspot() == Placement::Undefined )
							m_hotspotOfs.clear();
						else
							m_hotspotOfs = Util::placementToOfs(pMsg->hotspot(), {m_dragWidgetOfs, dragWidgetSize} );
					}
					else
					{
						m_dragWidgetOfs = ptsToSpx(Coord(0,0) - pMsg->pickOfs(),m_scale);
						dragWidgetSize = m_pPicked->_size();                             //TODO: Possible source of error, if picked widget changes size before the render call.
					}

					CoordSPX mousePos = _toLocal(ptsToSpx(pMsg->pointerPos(),m_scale));
					m_dragSlot.m_geo = { align((mousePos + m_dragWidgetOfs)), dragWidgetSize };

					RectSPX renderBounds = m_dragSlot.m_geo + (pDragWidget ? pDragWidget->_overflow() : BorderSPX());
					_requestRender(renderBounds);
					m_dragState = DragState::Dragging;
				}
				else
					_cancel();

				break;
			}

			case MsgType::DropProbe:
			{
				auto pMsg = static_cast<DropProbeMsg*>(_pMsg);

				// Check if our probe was acccepted and in that case start targeting.

				if( pMsg->isAccepted() )
				{
					Widget * pTargeted = static_cast<Widget*>(pMsg->sourceRawPtr());


					Base::msgRouter()->post(new DropEnterMsg(pTargeted, m_dropType, m_category, m_pDataset, m_pPicked, m_dragSlot._widget(),  this, pMsg->modKeys(), pMsg->dropPos()));
					Base::msgRouter()->post(new PickedEnterMsg(m_pPicked, pTargeted));

					m_pProbed = nullptr;
					m_pTargeted = pTargeted;
					m_dragState = DragState::Targeting;
				}

				break;
			}

			case MsgType::DropMove:
			{
				auto pMsg = static_cast<DropMoveMsg*>(_pMsg);

				// Check if we need to change drag widget

				if( pMsg->dragWidget() != m_dragSlot._widget())
					_replaceDragWidget(pMsg->dragWidget());

				break;
			}

			case MsgType::DropEnter:
			{
				auto pMsg = static_cast<DropEnterMsg*>(_pMsg);

				// Check if we need to change drag widget

				if( pMsg->dragWidget() != m_dragSlot._widget())
					_replaceDragWidget(pMsg->dragWidget());

				break;
			}

			case MsgType::DropDeliver:
			{
				auto pMsg = static_cast<DropDeliverMsg*>(_pMsg);

				// Check if our delivery was accepted and in that case complete, otherwise cancel.

				if( pMsg->isAccepted() )
					_complete( pMsg->deliveredTo() );
				else
				{
					Base::msgRouter()->post(new DropLeaveMsg(m_pTargeted, m_dropType, m_category, m_pDataset, m_pPicked, pMsg->modKeys(), pMsg->dropPos()));
					m_pTargeted = nullptr;

					_cancel();
				}
				break;
			}


			default:
				break;
		}

		Overlay::_receive(_pMsg);
	}

	//____ _cancel() ________________________________________________________________

	void DragNDropOverlay::_cancel()
	{
		if( m_dragSlot._widget())
		{
			BorderSPX overflow = m_dragSlot._widget() ? m_dragSlot._widget()->_overflow() : BorderSPX();

			_requestRender(m_dragSlot.m_geo + overflow);
			m_dragSlot._setWidget(nullptr);
		}

		Base::msgRouter()->post(new PickedCancelMsg(m_pPicked));
		m_pPicked = nullptr;
		m_dropType = DropType::Undefined;
		m_dragRestriction = {0, 0, 0, 0};
		m_category = 0;
		m_pDataset = nullptr;
		m_dragState = DragState::Idle;
	}

	//____ _complete() _______________________________________________________________

	void DragNDropOverlay::_complete( Widget * pDeliveredTo )
	{
		assert( !m_pTargeted );

		if( m_dragSlot._widget())
		{
			BorderSPX overflow = m_dragSlot._widget() ? m_dragSlot._widget()->_overflow() : BorderSPX();

			_requestRender(m_dragSlot.m_geo + overflow);
			m_dragSlot._setWidget(nullptr);
		}

		Base::msgRouter()->post(new PickedDeliverMsg(m_pPicked, pDeliveredTo));
		m_pPicked = nullptr;
		m_dropType = DropType::Undefined;
		m_dragRestriction = {0, 0, 0, 0};
		m_category = 0;
		m_pDataset = nullptr;
		m_dragState = DragState::Idle;
	}

	//____ _replaceDragWidget() ______________________________________________________

	void DragNDropOverlay::_replaceDragWidget( Widget * pNewWidget )
	{
		if (pNewWidget)
			pNewWidget->releaseFromParent();

		RectSPX oldRenderBounds = m_dragSlot._widget() ? m_dragSlot._widget()->_renderBounds() : RectSPX();

		m_dragSlot._setWidget(pNewWidget );
		SizeSPX newSize = pNewWidget ? m_dragSlot._widget()->_defaultSize(m_scale) : SizeSPX();
		RectSPX newRenderBounds = pNewWidget ? RectSPX{0,0,newSize} + pNewWidget->_overflow() : RectSPX();

		m_dragSlot.m_geo.setSize(newSize);

		RectSPX combRenderBounds = RectSPX::boundsExcludingEmpty(oldRenderBounds, newRenderBounds) + m_dragSlot.m_geo.pos();

		if( !combRenderBounds.isEmpty() )
			_requestRender( combRenderBounds );

	}


	//____ _render() __________________________________________________________

	void DragNDropOverlay::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		// Generate drag widget as an image of picked widget if missing and needed./*/*

		if( m_dragState == DragState::Dragging && !m_dragSlot._widget())
		{
			SizeSPX sz = m_pPicked->_size();

			auto pFactory = pDevice->surfaceFactory();
			auto pCanvas = pFactory->createSurface( WGBP(Surface, _.size = sz/64, _.format = PixelFormat::BGRA_8, _.canvas = true) );
			pCanvas->fill( HiColor::Transparent );

			pDevice->beginCanvasUpdate(pCanvas);
			m_pPicked->_render(pDevice, sz, sz);
			pDevice->endCanvasUpdate();

			auto pImage = Image::create();
			pImage->setImage( pCanvas );

			//TODO: Allow for tinted image.

			m_dragSlot._setWidget(pImage);
			m_dragSlot._setSize(sz, m_scale);
		}

		Overlay::_render(pDevice,_canvas,_window);
	}


} // namespace wg
