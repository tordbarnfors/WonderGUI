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
#ifndef WG_REORDERCAPSULE_DOT_H
#define WG_REORDERCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_transitions.h>
#include <wg_filler.h>
#include <wg_dataset.h>

namespace wg
{
	class ReorderCapsule;
	typedef	StrongPtr<ReorderCapsule>	ReorderCapsule_p;
	typedef	WeakPtr<ReorderCapsule>		ReorderCapsule_wp;

	//____ ReorderCapsule ______________________________________________________

	class ReorderCapsule : public Capsule
	{
	public:

		struct _dropdata{
			Widget_p	pWidget;
			float		weight;
		};

		typedef Dataset<_dropdata> DropData;


	public:
		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p			baggage;
			Widget_p			child;
			bool				disabled		= false;
			bool				dragOutside		= false;
			bool				dropTarget		= true;
			Finalizer_p			finalizer		= nullptr;
			int					id				= 0;
			MarkPolicy			markPolicy		= MarkPolicy::Geometry;			// Differs from widget default!
			bool				pickable		= true;
			uint8_t				pickCategory	= 0;
			bool				pickHandle		= false;
			PointerStyle		pointer			= PointerStyle::Undefined;
			bool				selectable		= false;
			Skin_p				skin;
			bool				stickyFocus 	= false;
			bool				tabLock			= false;
			String				tooltip;
			ValueTransition_p	transition;
			int					transitionDelay = 200000;
			Skin_p				transitionSkin;
			bool				usePickHandles = false;

		};


		//.____ Creation __________________________________________

		static ReorderCapsule_p	create();
		static ReorderCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ State __________________________________________________________

		bool					isReordering() const;

		//.____ Appearance __________________________________________________________

		void					setTransitionSkin(Skin* pSkin);
		Skin_p					transitionSkin() const { return m_pTransitionSkin; }

		//.____ Behavior ____________________________________________________

		void					setTransition(ValueTransition* pTransition);
		ValueTransition_p		transition() const { return m_pTransition; }

		void					setTransitionDelay(int microsec);
		int						transitionDelay() const { return m_transitionDelay; }

		void					setDragOutside( bool bDragOutside );
		bool					dragOutside() const { return m_bDragOutside; }

	protected:
		ReorderCapsule();
		void _init();

		template<class BP> ReorderCapsule( const BP& bp ) : Capsule(bp)
		{
			m_pTransition 		= bp.transition;
			m_transitionDelay	= bp.transitionDelay;
			m_pTransitionSkin	= bp.transitionSkin;
			m_bDragOutside		= bp.dragOutside;

			_init();

			if (bp.child)
				 slot.setWidget(bp.child);
		}

		virtual ~ReorderCapsule();

		void		_receive(Msg* pMsg) override;
		void		_update(int microPassed, int64_t microsecTimestamp) override;
		Container*	_ourContainer();
		void		_markPosition(int pos);
		void		_startTransition(int pos);
		void		_endTransition();

		bool				m_bReceivingUpdates = false;
		bool				m_bDragOutside = false;

		ValueTransition_p	m_pTransition;
		bool				m_bTransitioning = false;
		int					m_transitionProgress;
		Size				m_transitionStartSize;

		int					m_prevPos = -1;
		int					m_hoveredPos = -1;
		int					m_dropPos = -1;
		int					m_markedPos = -1;

		Filler_p			m_pHoveredPosFiller;
		Filler_p			m_pPrevPosFiller;
		int					m_transitionDelay = 200000;


		Widget_p			m_pPicked;
		int					m_pickedPos = -1;			// Offset in PackPanel
		float				m_pickedWeight;			// Weight of picked widget

		int					m_bPickedFromMe = false;

		int					m_delayCountdown = 0;

		Skin_p				m_pTransitionSkin;		//TODO: Use SkinSlots instead, one for transition in and one for transition out.
													// Would allow for animated drop-points.

		enum class PickState
		{
			Unpicked,
			Remaining,
			Canceled,
			Completed,
			Finishing
		};


		PickState			m_pickState = PickState::Unpicked;
	};


} // namespace wg
#endif //WG_REORDERCAPSULE_DOT_H

