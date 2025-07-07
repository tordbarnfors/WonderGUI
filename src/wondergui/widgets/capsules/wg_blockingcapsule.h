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
#ifndef WG_BLOCKINGCAPSULE_DOT_H
#define WG_BLOCKINGCAPSULE_DOT_H
#pragma once

#include <vector>

#include <wg_capsule.h>


//TODO: Need to support skin padding.

namespace wg
{

	class BlockingCapsule;
	typedef	StrongPtr<BlockingCapsule>		BlockingCapsule_p;
	typedef	WeakPtr<BlockingCapsule>		BlockingCapsule_wp;

	//____ BlockingCapsule ________________________________________________________

	class BlockingCapsule : public Capsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			bool			active = true;
			bool			autoDeactivate = false;
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			bool			inverted = false;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			int				scale = 0;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static BlockingCapsule_p	create();
		static BlockingCapsule_p	create(const Blueprint& blueprint);

		//.____ Components _______________________________________

		std::vector<Widget_wp>		blockedWidgets;
		std::vector<Rect>			blockedAreas;

		
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior ____________________________________________________________
		
		void	setActive(bool bActive);
		bool	isActive() const { return m_bActive; };
		
		void	setInverted(bool bInverted);
		bool	isInverted() const { return m_bInverted; }
		
		void	setAutoDeactivate(bool bAutoDeactivate);
		bool	isAutoDeactivate() const { return m_bAutoDeactivate; }

		
		//.____ Internal _________________________________________________

		Widget* _findWidget(const CoordSPX& ofs, SearchMode mode) override;


	protected:
		BlockingCapsule();
		
		template<class BP> BlockingCapsule( const BP& bp ) : Capsule(bp)
		{
			m_bAutoDeactivate	= bp.autoDeactivate;
			m_bInverted			= bp.inverted;
			m_bActive			= bp.active;
			
			if (bp.child)
				slot.setWidget(bp.child);
		}
		
		virtual ~BlockingCapsule();
		
		void	_receive(Msg* pMsg) override;
		
	private:
		
		bool	m_bInverted = false;			// Set to block all but the widgets in "blocked".
		bool	m_bActive = true;
		bool	m_bAutoDeactivate = false;
	};


} // namespace wg
#endif //WG_BLOCKINGCAPSULE_DOT_H
