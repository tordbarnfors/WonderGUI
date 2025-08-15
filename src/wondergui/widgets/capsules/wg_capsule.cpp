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

#include <wg_capsule.h>
#include <wg_patches.h>

namespace wg
{

	const TypeInfo Capsule::TYPEINFO = { "Capsule", &Container::TYPEINFO };

	//____ create() _______________________________________________________________

	Capsule_p Capsule::create()
	{
		return Capsule_p(new Capsule());
	}

	Capsule_p Capsule::create(const Blueprint& blueprint)
	{
		return Capsule_p(new Capsule(blueprint));
	}


	//____ constructor ____________________________________________________________

	Capsule::Capsule() : slot(this)
	{
	}

	//____ Destructor _____________________________________________________________

	Capsule::~Capsule()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Capsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _matchingHeight() ________________________________________________________

	spx Capsule::_matchingHeight( spx width, int scale ) const
	{
		if( slot._widget() )
		{
			SizeSPX padding = m_skin.contentBorderSize(scale);

			return slot._widget()->_matchingHeight( width-padding.w, scale ) + padding.h;
		}
		else
			return Widget::_matchingHeight(width, scale);
	}

	//____ _matchingWidth() ________________________________________________________

	spx Capsule::_matchingWidth( spx height, int scale ) const
	{
		if( slot._widget() )
		{
			SizeSPX padding = m_skin.contentBorderSize(scale);

			return slot._widget()->_matchingWidth(height - padding.h, scale) + padding.w;
		}
		else
			return Widget::_matchingWidth(height, scale);
	}

	//____ _defaultSize() ___________________________________________________________

	SizeSPX Capsule::_defaultSize( int scale ) const
	{
		if (slot._widget())
			return slot._widget()->_defaultSize(scale) + m_skin.contentBorderSize(scale);

		return Widget::_defaultSize(scale);
	}

	//____ _minSize() ___________________________________________________________

	SizeSPX Capsule::_minSize(int scale) const
	{
		if (slot._widget())
			return slot._widget()->_minSize(scale) + m_skin.contentBorderSize(scale);
		else
			return Widget::_minSize(scale);
	}

	 //____ maxSize() ______________________________________________________________

	SizeSPX Capsule::_maxSize(int scale) const
	{
		if (slot._widget())
			return slot._widget()->_maxSize(scale) + m_skin.contentBorderSize(scale);
		else
			return Widget::_maxSize(scale);
	}

	//____ _slotGeo() ___________________________________________________________

	RectSPX Capsule::_slotGeo( const StaticSlot * pSlot ) const
	{
		return m_skin.contentRect( m_size, m_scale, m_state );
	}

	//____ _childRequestRender() _________________________________________________

	void Capsule::_childRequestRender( StaticSlot * pSlot, const RectSPX& rect )
	{
		_requestRender( rect + m_skin.contentOfs( m_scale, m_state ));
	}

	//____ _childRequestResize() _________________________________________________

	void Capsule::_childRequestResize( StaticSlot * pSlot )
	{
		_requestResize();
	}

	//____ _prevChild() __________________________________________________________

	Widget * Capsule::_prevChild( const StaticSlot * pSlot ) const
	{
		return nullptr;
	}

	//____ _nextChild() __________________________________________________________

	Widget * Capsule::_nextChild( const StaticSlot * pSlot ) const
	{
		return nullptr;
	}

	//____ _maskPatches() ________________________________________________________

	void Capsule::_maskPatches( PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip )
	{
		RectSPX coverage = m_skin.coverage(geo, m_scale, m_state);
		
		patches.sub( RectSPX::overlap(coverage,clip) );

		if( coverage.contains(_contentRect(geo)) )
			return;										// No need to loop through children, skins coverage contains them all.

		if( slot._widget() )
			slot._widget()->_maskPatches(patches, _contentRect(geo), clip);
	}

	//____ _resize() ____________________________________________________________

	void Capsule::_resize( const SizeSPX& size, int scale )
	{
		Container::_resize( size, scale );

		if ( slot._widget())
		{
			SizeSPX sz = size - m_skin.contentBorderSize(scale);
			// Make sure size isn't negative
			if (sz.w < 0)
				sz.w = 0;
			if (sz.h < 0)
				sz.h = 0;
			slot._widget()->_resize(sz, scale);
		}
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	Capsule::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		return DynamicSlot::TYPEINFO;
	}

	//____ _firstChild() ____________________________________________________________

	Widget* Capsule::_firstChild() const
	{
		return slot._widget();
	}

	//____ _lastChild() _____________________________________________________________

	Widget* Capsule::_lastChild() const
	{
		return slot._widget();
	}

	//____ _firstSlotWithGeo() _____________________________________________________

	void Capsule::_firstSlotWithGeo( SlotWithGeo& package ) const
	{
		if (slot.isEmpty())
		{
			package.pSlot = nullptr;
		}
		else
		{
			package.pSlot = &slot;
			package.geo = m_skin.contentRect(RectSPX(0,0,m_size), m_scale, m_state);
		}
	}

	//____ _nextSlotWithGeo() ______________________________________________________

	void Capsule::_nextSlotWithGeo( SlotWithGeo& package ) const
	{
		package.pSlot = nullptr;
	}

	//____ _releaseChild() ____________________________________________________

	void Capsule::_releaseChild(StaticSlot * _pSlot)
	{
		slot._clearWidget();
		_refreshOverflow();
		_requestRender();
		_requestResize();
	}

	//____ _replaceChild() ____________________________________________________________

	void Capsule::_replaceChild( StaticSlot * pSlot, Widget * pWidget )
	{
		slot._setWidget(pWidget );

		if (pWidget)
		{
			SizeSPX sz = m_size - m_skin.contentBorderSize(m_scale);
			pWidget->_resize(sz, m_scale);
		}

		_refreshOverflow();
		_requestRender();
		_requestResize();
	}



} // namespace wg
