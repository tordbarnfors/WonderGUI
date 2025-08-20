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

#include <wg_reordercapsule.h>
#include <wg_msg.h>
#include <wg_colorskin.h>
#include <wg_dataset.h>
#include <wg_packpanel.h>
#include <wg_msgrouter.h>
#include <wg_sizecapsule.h>

namespace wg
{
	using namespace Util;

	const TypeInfo ReorderCapsule::TYPEINFO = { "ReorderCapsule", &Capsule::TYPEINFO };

	//____ create() ______________________________________________________________

	ReorderCapsule_p ReorderCapsule::create()
	{
		return ReorderCapsule_p(new ReorderCapsule());
	}

	ReorderCapsule_p ReorderCapsule::create(const Blueprint& blueprint)
	{
		return ReorderCapsule_p(new ReorderCapsule(blueprint));
	}

	//____ constructor ____________________________________________________________

	ReorderCapsule::ReorderCapsule()
	{
		m_bPickable = true;
		m_markPolicy = MarkPolicy::Geometry;
		_init();
	}

	//____ _init() _______________________________________________________________

	void ReorderCapsule::_init()
	{
		if( !m_pTransition )
			m_pTransition = ValueTransition::create(300000, TransitionCurve::EaseOut);

		m_pHoveredPosFiller = Filler::create( WGBP(Filler, _.skin = m_pTransitionSkin, _.markPolicy = MarkPolicy::Geometry ));
		m_pPrevPosFiller = Filler::create(WGBP(Filler, _.skin = m_pTransitionSkin, _.markPolicy = MarkPolicy::Geometry ));
	}

	//____ destructor _____________________________________________________________

	ReorderCapsule::~ReorderCapsule()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ReorderCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ isReordering() ________________________________________________________

	bool ReorderCapsule::isReordering() const
	{
		return m_pickState != PickState::Unpicked;
	}

	//____ setTransition() _______________________________________________________

	void ReorderCapsule::setTransition(ValueTransition* pTransition)
	{
		m_pTransition = pTransition;
	}

	//____ setTransitionDelay() __________________________________________________

	void ReorderCapsule::setTransitionDelay(int microsec)
	{
		m_transitionDelay = microsec;
	}

	//____ setTransitionSkin() ___________________________________________________

	void ReorderCapsule::setTransitionSkin(Skin* pSkin)
	{
		if (pSkin != m_pTransitionSkin)
		{
			m_pTransitionSkin = pSkin;
			m_pHoveredPosFiller->setSkin(pSkin);
			m_pPrevPosFiller->setSkin(pSkin);
		}
	}

	//____ setDragOutside() ______________________________________________________

	void ReorderCapsule::setDragOutside( bool bDragOutside )
	{
		m_bDragOutside = bDragOutside;
	}

	//____ _receive() ____________________________________________________________

	void ReorderCapsule::_receive(Msg* _pMsg)
	{
		switch( _pMsg->type())
		{
			case MsgType::Pick:
			{
				auto pMsg = static_cast<PickMsg*>(_pMsg);

				// Find the panel inside us that we are affecting.

				Container* pContainer = _ourContainer();

				// Find out which child we picked

				auto pickOfs = ptsToSpx( pMsg->pickOfs(), m_scale );

				Widget_p pWidget = _findWidget(pickOfs, SearchMode::ActionTarget);	// Widget_p since we need to secure refcount.

				while( pWidget != nullptr && pWidget != this && pWidget->parent() != pContainer )
					pWidget = pWidget->parent();

				//

				if( pWidget && pWidget != this )
				{
					auto pPackPanel = static_cast<PackPanel*>(pContainer);
					auto pSlot = pPackPanel->slots.find(pWidget);

					Coord offset = -(pMsg->pointerPos() - pWidget->globalGeo().pos());

					auto pDataset = DropData::create({pWidget,pSlot->weight()});
					pMsg->setContent(DropType::Widget, m_pickCategory, pDataset);

					m_pHoveredPosFiller->setDefaultSize(pWidget->defaultSize() );
					pSlot->setWidget(m_pHoveredPosFiller);

					auto pSizeCapsule = SizeCapsule::create();
					pSizeCapsule->setDefaultSize(pWidget->size());		// Should keep its size while reordering.
					pSizeCapsule->slot = pWidget;

					pMsg->setDragWidget(pSizeCapsule, offset );
					pMsg->setHotspot(Placement::Center);

					if( !m_bDragOutside )
					{
						Rect confinement = pPackPanel->globalGeo();

						if( pPackPanel->axis() == Axis::X )
						{
							confinement.y += confinement.h/2;
							confinement.h = 0;
						}
						else
						{
							confinement.x += confinement.w/2;
							confinement.w = 0;
						}

						pMsg->setDragConfinement(confinement);
					}

					m_pPicked = pWidget;
					m_bPickedFromMe = true;



					m_pickedPos = int( pSlot - pPackPanel->slots.begin());
					m_pickedWeight = pSlot->weight();

					m_markedPos = m_pickedPos;
					m_hoveredPos = m_pickedPos;

					m_transitionProgress = m_pTransition->duration();

					m_pickState = PickState::Remaining;

					_startReceiveUpdates();
					m_bReceivingUpdates = true;

				}

				break;
			}

			case MsgType::PickedEnter:
			{
				auto pMsg = static_cast<PickedEnterMsg*>(_pMsg);
				if( pMsg->target() != this )
				{
					_markPosition(-1);
				}
				break;
			}

			case MsgType::PickedLeave:
			{
				auto pMsg = static_cast<PickedEnterMsg*>(_pMsg);
				if( pMsg->target() != this )
				{
					_markPosition(m_pickedPos);
				}
				break;
			}

 			case MsgType::PickedCancel:
			{
				_endTransition();
				m_pickState = PickState::Canceled;
				break;
			}

			case MsgType::PickedDeliver:
			{
				_endTransition();
				m_pickState = PickState::Completed;
				break;
			}

			case MsgType::DropProbe:
			{
				auto pMsg = static_cast<DropProbeMsg*>(_pMsg);

				if (pMsg->dropType() == DropType::Widget && pMsg->category() == m_pickCategory)
				{
					pMsg->accept();

					if( !m_bPickedFromMe )
					{
						auto pCasted = wg_dynamic_cast<StrongPtr<DropData>>(pMsg->dataset());
						m_pPicked = pCasted->data.pWidget;
						m_pickedWeight = pCasted->data.weight;

						if( m_pickState == PickState::Finishing )
						{
							// Special treatment, transition from earlier leave is still running.
							// We need to start from same state.

//							m_transitionProgress = 0;

							m_pickedPos = -1;
							m_prevPos = -1;
							m_markedPos = -1;

							m_pPrevPosFiller->releaseFromParent();


						}

						m_pickState = PickState::Unpicked;
					}
				}
				break;
			}

			case MsgType::DropEnter:
			{
				if( !m_bPickedFromMe )
				{
					if( !m_bReceivingUpdates )
					{
						_startReceiveUpdates();
						m_bReceivingUpdates = true;
					}
				}


				// No break, continue to DropMove!
			}

			case MsgType::DropMove:
			{
				auto pMsg = static_cast<DropMsg*>(_pMsg);

				Coord hoverCoord = pMsg->dropPos() - globalGeo().pos();

				Widget * pWidget = _findWidget( Util::ptsToSpx(hoverCoord, m_scale), SearchMode::ActionTarget);

				PackPanel* pOurContainer = static_cast<PackPanel*>(_ourContainer());


				while (pWidget && pWidget != this && pWidget->parent() != pOurContainer)
					pWidget = pWidget->parent();

				if (pWidget && pWidget->parent() == pOurContainer)
				{
					if (pWidget == m_pHoveredPosFiller)
						break;

					if (pWidget == m_pPrevPosFiller)
					{
						_markPosition(m_prevPos);
						break;
					}

					Rect markedGeo = pWidget->globalGeo();

					int index = pOurContainer->slots.index(pWidget);
					int compareIndex = index;

					if (m_hoveredPos >= 0 && m_hoveredPos < compareIndex)
						index--;

					if (m_prevPos >= 0 && m_prevPos < compareIndex)
						index--;

					if (pOurContainer->axis() == Axis::X)
					{
						if (pMsg->dropPos().x > markedGeo.x + markedGeo.w / 2)
							index++;
					}
					else
					{
						if (pMsg->dropPos().y > markedGeo.y + markedGeo.h / 2)
							index++;
					}

					_markPosition(index);

				}

				break;
			}

			case MsgType::DropLeave:
				_markPosition(m_pickedPos);

				if( !m_bPickedFromMe )
				{
					m_pickState = PickState::Finishing;
				}
				break;

			case MsgType::DropDeliver:
			{
				auto pMsg = static_cast<DropDeliverMsg*>(_pMsg);

				pMsg->accept();

				if( !m_bPickedFromMe )
				{
					_endTransition();
					m_pickState = PickState::Completed;
				}
				break;
			}
				
			default:
				break;
		}
	}

	//____ _update() _____________________________________________________________

	void ReorderCapsule::_update(int microPassed, int64_t microsecTimestamp)
	{
		auto pPackPanel = static_cast<PackPanel*>(_ourContainer());

		if (m_bTransitioning)
		{
			int duration = m_pTransition->duration();

			m_transitionProgress += microPassed;
			if (m_transitionProgress >= duration)
			{
				m_transitionProgress = duration;
				m_bTransitioning = false;
			}

			Size targetSize = m_pPicked->defaultSize();
			Size startSize = m_transitionStartSize;

			int progress = std::max(0, m_transitionProgress);

			if (pPackPanel->axis() == Axis::X)
			{
				pts res = m_pTransition->snapshot(progress, startSize.w, targetSize.w);
				m_pHoveredPosFiller->setDefaultSize({ res, m_transitionStartSize.h });
				m_pPrevPosFiller->setDefaultSize({ targetSize.w - res, m_transitionStartSize.h });
			}
			else
			{
				pts res = m_pTransition->snapshot(progress, startSize.h, targetSize.h);
				m_pHoveredPosFiller->setDefaultSize({ m_transitionStartSize.w, res });
				m_pPrevPosFiller->setDefaultSize({ m_transitionStartSize.w, targetSize.h - res });
			}

		}

		if (m_delayCountdown > 0)
			m_delayCountdown = std::max( 0, m_delayCountdown - microPassed);


		if( m_delayCountdown == 0 && m_bTransitioning == false && m_markedPos != m_hoveredPos )
			_startTransition(m_markedPos);

		switch (m_pickState)
		{
		case PickState::Canceled:
		{
			_markPosition(m_pickedPos);
			m_pickState = PickState::Finishing;
			m_dropPos = m_pickedPos;
			break;
		}

		case PickState::Completed:
		{
			m_pickState = PickState::Finishing;
			m_dropPos = m_markedPos;
			break;
		}

		case PickState::Finishing:
		{
			if (!m_bTransitioning && m_delayCountdown == 0)
			{
				// Remove fillers.

				m_pHoveredPosFiller->releaseFromParent();
				m_pPrevPosFiller->releaseFromParent();

				// Reinsert picked widget.

				if( m_bPickedFromMe )
				{
					_stopReceiveUpdates();
					m_bReceivingUpdates = false;
					m_bPickedFromMe = false;
				}

				if( m_dropPos >= 0 )
				{
					pPackPanel->slots.insert(m_dropPos, m_pPicked)->setWeight(m_pickedWeight);
					m_dropPos = -1;

					Base::msgRouter()->post(PingMsg::create(this));	//TODO: Should not send this message if we just canceled.
				}


				m_pPicked = nullptr;
				m_pickedPos = -1;
				m_prevPos = -1;
				m_hoveredPos = -1;
				m_markedPos = -1;
				m_pickState = PickState::Unpicked;
			}
		}

		default:
			break;
		}

	}

	//____ _ourContainer() ________________________________________________________

	Container* ReorderCapsule::_ourContainer()
	{
		Widget* p = slot._widget();
		while (p->isInstanceOf(Capsule::TYPEINFO))
			p = static_cast<Capsule*>(p)->slot.widget();

		if (p->isContainer())
			return static_cast<Container*>(p);
		else
			return nullptr;
	}

	//____ _markPosition() ____________________________________________________

	void ReorderCapsule::_markPosition(int pos)
	{
		if (pos != m_markedPos)
		{
			m_markedPos = pos;
			m_delayCountdown = m_transitionDelay;
		}
	}

	//____ _startTransition() _________________________________________________

	void ReorderCapsule::_startTransition(int pos)
	{
		auto pPackPanel = static_cast<PackPanel*>(_ourContainer());

		if (pos == m_hoveredPos)
			return;											// No movement.

		if (m_transitionProgress > m_pTransition->duration() / 2 )
		{
			std::swap(m_prevPos, m_hoveredPos);
			std::swap(m_pPrevPosFiller, m_pHoveredPosFiller);
		}

		Size	startSize;

		if( pos == -1 )
		{
			startSize = m_pHoveredPosFiller->defaultSize();
			m_hoveredPos = pos;
			m_pHoveredPosFiller->releaseFromParent();
		}
		else if (pos == m_hoveredPos)
		{
			startSize = m_pHoveredPosFiller->defaultSize();
		}
		else
		{
			startSize = m_pPicked->defaultSize();

			if (pPackPanel->axis() == Axis::X)
				startSize.w = 0;
			else
				startSize.h = 0;

			int newHoveredPos = pos;

			if (m_prevPos >= 0 && m_prevPos < pos)
				pos++;

			if (m_hoveredPos >= 0 && m_hoveredPos < pos)
				pos++;

			m_hoveredPos = newHoveredPos;
			 
			m_pHoveredPosFiller->setDefaultSize(startSize);

			pPackPanel->slots.insert(pos, m_pHoveredPosFiller)->setWeight(m_pickedWeight);
		}

		m_transitionProgress = 0;
		m_bTransitioning = true;
		m_transitionStartSize = startSize;


	}

	//____ _endTransition() ______________________________________________________

	void ReorderCapsule::_endTransition()
	{
		m_transitionProgress = m_pTransition->duration();
		m_bTransitioning = false;
	}


} // namespace wg
