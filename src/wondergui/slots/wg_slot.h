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


#ifndef WG_SLOT_DOT_H
#define WG_SLOT_DOT_H
#pragma once

#include <wg_widget.h>

namespace wg
{
	//____ StaticSlot __________________________________________________________

	class StaticSlot
	{
		friend class Widget;
		friend class RootPanel;
		friend class SlotIterator;
		friend class SlotHolder;
		template<class S, int X> friend class SlotArray;

	public:

		//.____ Blueprint _______________________________________________________

		struct Blueprint
		{
			// No members for base slot type
		};
		
		//.____ Identification ________________________________________________

		const TypeInfo& typeInfo(void) const { return m_pHolder->_slotTypeInfo(this); }

		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		inline Widget_p widget() const { return Widget_p(m_pWidget); }
		inline Widget*	rawWidgetPtr() const { return m_pWidget; }
		inline bool		isEmpty() const { return !m_pWidget; }

		//.____ Geometry ______________________________________________________

		inline Coord	pos() const { return Util::spxToPts(m_pHolder->_slotGeo(this).pos(),m_pHolder->_scale()); }
		inline Size		size() const { return Util::spxToPts(m_pHolder->_slotGeo(this).size(),m_pHolder->_scale()); }
		inline Rect		geo() const { return Util::spxToPts(m_pHolder->_slotGeo(this), m_pHolder->_scale()); }

		//.____ Operators __________________________________________

		inline operator Widget_p() const { return widget(); }

		inline bool operator==(Widget * other) const { return other == _widget(); }
		inline bool operator!=(Widget * other) const { return other != _widget(); }

		inline Widget* operator->() const { return _widget(); }

		//.____ Internal ______________________________________________________

		inline CoordSPX	_pos() const { return m_pHolder->_slotGeo(this).pos(); }
		inline SizeSPX	_size() const { return m_pHolder->_slotGeo(this).size(); }
		inline RectSPX	_geo() const { return m_pHolder->_slotGeo(this); }

		
		inline Widget * _widget() const { return m_pWidget; }

		inline void _setWidget(Widget* pWidget)
		{
			if (m_pWidget)
			{
				if (m_pWidget == pWidget)
					return;

				m_pWidget->_setSlot(nullptr);
				m_pWidget->_decRefCount();
			}

			m_pWidget = pWidget;

			if (pWidget)
			{
				pWidget->_incRefCount();
				pWidget->releaseFromParent();
				pWidget->_setSlot(this);
			}
		}

		inline void _clearWidget()
		{
			if (m_pWidget)
			{
				m_pWidget->_setSlot(nullptr);
				m_pWidget->_decRefCount();
				m_pWidget = nullptr;
			}
		}
		
		
	protected:
		const static bool safe_to_relocate = true;

		StaticSlot(SlotHolder * pHolder) : m_pHolder(pHolder) {}

		StaticSlot(const StaticSlot& o) noexcept			// Copy constructor needed for containers. Should not copy the widget pointer.
		{
			m_pHolder = o.m_pHolder;
		}

		
		StaticSlot(StaticSlot&& o) noexcept
		{
			m_pWidget = o.m_pWidget;
			m_pHolder = o.m_pHolder;
			if (m_pWidget)
			{
				m_pWidget->_setSlot(this);
				o.m_pWidget = nullptr;
			}
		}

		// IMPORTANT! SlotVector assumes that Slot destructor doesn't need to be called
		// if content has been moved to another slot!

		~StaticSlot()
		{
			if (m_pWidget != nullptr)
			{
				m_pWidget->_setSlot(nullptr);
				m_pWidget->_decRefCount();
			}
		}

		StaticSlot& operator=(const StaticSlot& o) noexcept	// Assignment operators needed for STL containers. Should not copy the widget pointer.
		{
			m_pHolder = o.m_pHolder;
			return *this;
		}
		
		StaticSlot& operator=(StaticSlot&& o) noexcept
		{
			if (m_pWidget)
			{
				m_pWidget->_setSlot(nullptr);
				m_pWidget->_decRefCount();
			}

			m_pWidget = o.m_pWidget;
			//			m_pHolder = o.m_pHolder;

			if (m_pWidget)
			{
				m_pWidget->_setSlot(this);
				o.m_pWidget = nullptr;
			}

			return *this;
		}

		bool _setBlueprint( const Blueprint& bp ) { return true; };
		
		inline void _relink() { m_pWidget->_setSlot( this ); }


		inline SlotHolder * _holder() { return m_pHolder; }
		inline const SlotHolder * _holder() const { return m_pHolder; }

		inline void		_setSize( SizeSPX size, int scale ) const { m_pWidget->_resize(size, scale); }

		Widget *		m_pWidget = nullptr;
		SlotHolder *	m_pHolder;
	};

	//____ DynamicSlot __________________________________________________________

	class DynamicSlot : public StaticSlot
	{
		friend class Widget;
		friend class SlotIterator;
		friend class SlotHolder;

	public:
		
		//.____ Creation ______________________________________________________

		DynamicSlot() : StaticSlot(nullptr) {}

		DynamicSlot(SlotHolder* pHolder) : StaticSlot(pHolder) {}

		DynamicSlot(const DynamicSlot& o) noexcept : StaticSlot(o.m_pHolder)			// Copy constructor needed for STL containers. Should not copy the widget pointer.
		{
		}
		
		DynamicSlot(DynamicSlot&& o) noexcept : StaticSlot(o.m_pHolder)
		{
			m_pWidget = o.m_pWidget;
			if (m_pWidget)
			{
				m_pWidget->_setSlot(this);
				o.m_pWidget = nullptr;
			}
		}

		DynamicSlot& operator=(const DynamicSlot& o) noexcept	// Assignment operators needed for STL containers. Should not copy the widget pointer.
		{
			m_pHolder = o.m_pHolder;
			return *this;
		}
		
		DynamicSlot& operator=(DynamicSlot&& o) noexcept
		{
			if (m_pWidget)
			{
				m_pWidget->_setSlot(nullptr);
				m_pWidget->_decRefCount();
			}

			m_pWidget = o.m_pWidget;
			//			m_pHolder = o.m_pHolder;

			if (m_pWidget)
			{
				m_pWidget->_setSlot(this);
				o.m_pWidget = nullptr;
			}

			return *this;
		}


		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		inline void		setWidget(const Widget_p& pWidget) { if (pWidget == m_pWidget) return; m_pHolder->_replaceChild(this, pWidget); }

		//.____ Operators __________________________________________

		inline void operator=(Widget * pWidget) { setWidget(pWidget); }



	protected:
		const static bool safe_to_relocate = true;

	};


}

#endif //WG_SLOT_DOT_H
