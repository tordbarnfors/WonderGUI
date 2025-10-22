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

#include <vector>
#include <wg_container.h>
#include <wg_panel.h>
#include <wg_rootpanel.h>
#include <wg_patches.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Container::TYPEINFO = { "Container", &Widget::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Container::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ isContainer() ______________________________________________________________

	bool Container::isContainer() const
	{
		return true;
	}

	//____ setSkin() _____________________________________________________________

	void Container::setSkin(Skin* pNewSkin)
	{
		Skin_p pOldSkin = m_skin.get();

		m_skin.set(pNewSkin);

		BorderSPX oldOverflow = pOldSkin ? pOldSkin->_overflow(m_scale) : BorderSPX();
		BorderSPX newOverflow = pNewSkin ? pNewSkin->_overflow(m_scale) : BorderSPX();
			
		if( oldOverflow != newOverflow )
			_refreshOverflow();

		_requestRender( RectSPX(m_size) + BorderSPX::max(oldOverflow,newOverflow) );
		
		if (!pNewSkin || !pOldSkin || pNewSkin->_contentBorderSize(m_scale) != pOldSkin->_contentBorderSize(m_scale) ||
			pNewSkin->_defaultSize(m_scale) != pOldSkin->_defaultSize(m_scale) || pNewSkin->_minSize(m_scale) != pOldSkin->_minSize(m_scale))
			_requestResize();
	}

	//____ setUsePickingHandles() ____________________________________________

	void Container::setUsePickHandles( bool bUseHandles )
	{
		m_bUsePickHandles = bUseHandles;
	}

	//____ contains() _________________________________________________________

	bool Container::contains(const Widget * pWidget ) const
	{
		if (!pWidget)
			return false;

		pWidget = pWidget->parent();
		while (pWidget)
		{
			if (pWidget == this)
				return true;
			pWidget = pWidget->parent();
		}

		return false;
	}

	//____ _childLocalToGlobal() _______________________________________________

	RectSPX Container::_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return _toGlobal(rect + _slotGeo(pSlot).pos());
	}

	//____ _globalToChildLocal() ________________________________________________

	RectSPX Container::_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return _toLocal(rect) - _slotGeo(pSlot).pos();

	}

	//____ _globalPtsToChildLocalSpx() ___________________________________________

	RectSPX Container::_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const
	{
		RectSPX rectSPX = m_pHolder ? m_pHolder->_globalPtsToChildLocalSpx(m_pSlot, rect) : Util::align(Util::ptsToSpx(rect, m_scale));

		return rectSPX - _slotGeo(pSlot).pos();
	}

	//____ _childLocalSpxToGlobalPts() ___________________________________________

	Rect Container::_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& _rect) const
	{
		RectSPX rect = _rect + _slotGeo(pSlot).pos();
		
		if( m_pHolder )
			return m_pHolder->_childLocalSpxToGlobalPts( m_pSlot, rect );
		else
			return Util::spxToPts(rect, m_scale);
	}

	//____ _isChildVisible() __________________________________________________

	bool Container::_isChildVisible( const StaticSlot * pSlot ) const
	{
		return true;
	}

	//____ _childWindowSection() ____________________________________________________

	RectSPX Container::_childWindowSection( const StaticSlot * pSlot ) const
	{
		return RectSPX( 0,0, pSlot->_widget()->_size() );
	}

	//____ _container() _____________________________________________________________

	Container * Container::_container()
	{
		return this;
	}

	//____ _root() ___________________________________________________________________

	Root * Container::_root()
	{
		return m_pHolder ? m_pHolder->_root() : nullptr;
	}

	//____ _scale() ________________________________________________________________

	int Container::_scale() const
	{
		return m_scale;
	}

	//____ _childOverflowChanged() _______________________________________________

	void Container::_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow )
	{
		_overflowChanged( oldOverflow, newOverflow );
	}

	//____ _childRequestFocus() ______________________________________________________

	bool Container::_childRequestFocus(StaticSlot * pSlot, Widget * pWidget, bool bRaiseWindow )
	{
		return m_pHolder ? m_pHolder->_childRequestFocus( m_pSlot, pWidget, bRaiseWindow ) : false;
	}

	//____ _ChildReleaseFocus() ______________________________________________________

	bool Container::_childReleaseFocus(StaticSlot * pSlot, Widget * pWidget )
	{
		return m_pHolder ? m_pHolder->_childReleaseFocus( m_pSlot, pWidget ) : false;
	}

	//____ _childRequestInView() _____________________________________________________

	void Container::_childRequestInView(StaticSlot * pSlot )
	{
		if( m_pHolder )
		{
			RectSPX area = _slotGeo( pSlot );
			m_pHolder->_childRequestInView( m_pSlot, area, area );
		}
	}

	void Container::_childRequestInView(StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea )
	{
		if( m_pHolder )
		{
			CoordSPX pos( _slotGeo( pSlot ).pos() );
			m_pHolder->_childRequestInView( m_pSlot, mustHaveArea + pos, niceToHaveArea + pos );
		}
	}

	//____ _didAddSlots() _____________________________________________________

	void Container::_didAddSlots(StaticSlot * pSlot, int nb)
	{
		return;				// By default we don't support adding/moving/erasing slots
	}

	//____ _didMoveSlots() ____________________________________________________

	void Container::_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb)
	{
		return;				// By default we don't support adding/moving/erasing slots
	}

	//____ _willEraseSlots() __________________________________________________

	void Container::_willEraseSlots(StaticSlot * pSlot, int nb)
	{
		return;				// By default we don't support adding/moving/erasing slots
	}

	//____ _findWidget() ____________________________________________________________

	Widget * Container::_findWidget( const CoordSPX& ofs, SearchMode mode )
	{
		SlotWithGeo	child;
		_firstSlotWithGeo(child);

		while( child.pSlot )
		{
			if( child.geo.contains( ofs ) )
			{
				Widget * pWidget = child.pSlot->_widget();

				if (pWidget->isContainer())
				{
					Widget * pRes = static_cast<Container*>(pWidget)->_findWidget(ofs - child.geo.pos(), mode);
					if (pRes)
						return pRes;
				}
				else if( mode == SearchMode::Geometry || pWidget->_markTest( ofs - child.geo.pos() ) )
					return pWidget;
			}
			_nextSlotWithGeo( child );
		}

		// Check against ourselves

		if( mode == SearchMode::Geometry || _markTest(ofs) )
			return this;

		return nullptr;
	}

	//____ _getModalOverlay() _________________________________________________

	ModalOverlay *  Container::_getModalOverlay() const
	{
		const Container * p = _parent();

		if( p )
			return p->_getModalOverlay();
		else
			return 0;
	}

	//____ _getPopupOverlay() _________________________________________________

	PopupOverlay * Container::_getPopupOverlay() const
	{
		const Container * p = _parent();

		if( p )
			return p->_getPopupOverlay();
		else
			return 0;
	}

	//____ _renderBounds() ____________________________________________________

	RectSPX Container::_renderBounds() const
	{
		if (m_bOverflow)
			return RectSPX(m_size) + m_overflow;
		else
			return m_skin.renderBounds(m_size, m_scale);
	}


	//____ _overflow() ________________________________________________________

	BorderSPX Container::_overflow() const
	{
		return m_overflow;
	}


	//____ _setState() ______________________________________________________

	void Container::_setState( State state )
	{
		State oldState = m_state;

		Widget::_setState(state);						// Doing this call first is an optimization, possibly less dirty rects generated.

		if( oldState.isDisabled() != state.isDisabled() )
		{
			bool bDisabled = state.isDisabled();
			Widget * p = _firstChild();
			while( p )
			{
				p->setDisabled(bDisabled);
				p = p->_nextSibling();
			}
		}

		if( oldState.isSelekted() != state.isSelekted() )
		{
			bool bSelected = state.isSelekted();
			Widget * p = _firstChild();
			while( p )
			{
				State s = p->m_state;
				s.setSelekted(bSelected);
				p->_setState( s );
				p = p->_nextSibling();
			}
		}
	}

	//____ _render() _____________________________________________________
	// Default implementation for panel rendering patches.

	void Container::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		class WidgetRenderContext
		{
		public:
			WidgetRenderContext() : pWidget(0) {}
			WidgetRenderContext( Widget * pWidget, const RectSPX& geo, const RectSPX& renderArea ) : pWidget(pWidget), geo(geo), renderArea(renderArea) {}

			Widget *	pWidget;
			RectSPX		geo;
			RectSPX		renderArea;
			ClipPopData clipPop;
		};


		// Render container itself

		m_skin.render(pDevice, _canvas, m_scale, m_state );

		// Render children

		RectSPX	dirtBounds = pDevice->clipBounds();

		if( m_bSiblingsOverlap )
		{
			// Create WidgetRenderContext's for siblings that might get dirty patches

			std::vector<WidgetRenderContext> renderList;

			SlotWithGeo child;
			_firstSlotWithGeo( child );
			while(child.pSlot)
			{
				RectSPX renderArea = (child.geo + _canvas.pos()) + child.pSlot->_widget()->_overflow();

				if( renderArea.isOverlapping( dirtBounds ) )
					renderList.push_back( WidgetRenderContext(child.pSlot->_widget(), child.geo + _canvas.pos(), renderArea ) );

				_nextSlotWithGeo( child );
			}

			// Collect dirty patches from gfxDevice

			int nClipRects = pDevice->clipListSize();
			auto pClipRects = pDevice->clipList();
			PatchesSPX patches( nClipRects );

			for( int i = 0 ; i < nClipRects ; i++ )
				patches.push(pClipRects[i]);

			// Go through WidgetRenderContexts, push and mask dirt

			int lastWithDirt = int(renderList.size()) -1;
			for (unsigned int i = 0 ; i < renderList.size(); i++)
			{
				WidgetRenderContext * p = &renderList[i];

				PatchesSPX childPatches(patches, p->renderArea);						// Limit patches to childs geo

				if( childPatches.isEmpty() )
				{
					p->pWidget = nullptr;
				}
				else
				{
					p->clipPop = patchesToClipList(pDevice, p->renderArea, childPatches);
					p->pWidget->_maskPatches( patches, p->geo, p->renderArea );		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

					if( patches.isEmpty() )
					{
						lastWithDirt = int(i);	// Prevent poping clipLists that have not been set.
						break;
					}
				}
			}

			// Go through WidgetRenderContexts and render the patches in reverse order (topmost child rendered last).

			for (int i = lastWithDirt; i >= 0; i--)
			{
				WidgetRenderContext * p = &renderList[i];
				if( p->pWidget )
				{
					p->pWidget->_render( pDevice, p->geo, p->geo );
					popClipList(pDevice,p->clipPop);
				}
			}
		}
		else
		{
			SlotWithGeo child;
			_firstSlotWithGeo( child );

			while(child.pSlot)
			{
				RectSPX canvas = child.geo + _canvas.pos();
				RectSPX renderArea = (child.geo + _canvas.pos()) + child.pSlot->_widget()->_overflow();

				if (renderArea.isOverlapping(dirtBounds))
				{
					ClipPopData popData = limitClipList(pDevice, renderArea );

					if( pDevice->clipListSize() > 0 )
						child.pSlot->_widget()->_render(pDevice, canvas, canvas);

					popClipList( pDevice, popData );
				}
				_nextSlotWithGeo( child );
			}

		}
	}

	//____ _resize() __________________________________________________________

	void Container::_resize(const SizeSPX& size, int scale)
	{
		int oldScale = m_scale;

		Widget::_resize(size, scale);

		// This is not optimal. Overflow might be refreshed twice. Once now
		// and once later when subclass goes through its children.

		if( scale != oldScale && m_skin.overflowsGeo() )
			_refreshOverflow();
	}

	//____ _calcOverflow() ____________________________________________________

	BorderSPX Container::_calcOverflow()
	{
		BorderSPX 	overflow = m_skin.overflow(m_scale);
		RectSPX		overflowGeo = RectSPX{ 0,0,m_size } + overflow;

		SlotWithGeo slot;
		_firstSlotWithGeo(slot);
		while (slot.pSlot)
		{
			if (slot.pSlot->_widget()->_hasOverflow())
				overflowGeo.growToContain(slot.geo + slot.pSlot->_widget()->_overflow());

			_nextSlotWithGeo(slot);
		}

		overflow = BorderSPX::diff(RectSPX{ 0,0,m_size }, overflowGeo);

		return overflow;
	}


	//____ _refreshOverflow() ____________________________________________________

	void Container::_refreshOverflow()
	{
		auto overflow = _calcOverflow();

		if( overflow != m_overflow )
		{
			auto oldOverflow = m_overflow;
			m_overflow = overflow;

			m_bOverflow = !overflow.isEmpty();
			
			_overflowChanged(oldOverflow, m_overflow);
		}
	}

	//____ _addChildOverflow() ____________________________________________________

	void Container::_addChildOverflow( const RectSPX& childGeo, const BorderSPX& childOverflow )
	{
		if( m_overflow.top >= childOverflow.top && m_overflow.right >= childOverflow.right && m_overflow.bottom >= childOverflow.bottom && m_overflow.left >= childOverflow.left )
			return;
		
		RectSPX childOverflowGeo = childGeo + childOverflow;
		RectSPX overflowGeo = RectSPX{0,0,m_size} + m_overflow;

		if( overflowGeo.contains(childOverflowGeo) )
			return;
		
		auto oldOverflow = m_overflow;
		m_overflow.growToContain(BorderSPX::diff(overflowGeo, childOverflowGeo));

		m_bOverflow = true;
		
		_overflowChanged(oldOverflow, m_overflow);
	}

	//____ _removeChildOverflow() ____________________________________________________

	void Container::_removeChildOverflow( const RectSPX& childGeo, const BorderSPX& childOverflow )
	{
		if( childOverflow.isEmpty() )
			return;

		RectSPX childOverflowGeo = childGeo + childOverflow;
		RectSPX overflowGeo = RectSPX{0,0,m_size} + m_overflow;
		
		if( childOverflowGeo.top() == overflowGeo.top() || childOverflowGeo.right() == overflowGeo.right() ||
		   childOverflowGeo.bottom() == overflowGeo.bottom() || childOverflowGeo.left() == overflowGeo.left() )
		{
			_refreshOverflow();
		}
	}

	//____ _moveChildOverflow() _______________________________________________
		   
	void Container::_moveChildOverflow(StaticSlot * pSlot, const RectSPX& oldGeo, const RectSPX& newGeo)
	{
		//TODO: Implement optimized solution.
			
		_refreshOverflow();
	}
		   
	//____ _maskPatches() __________________________________________________________

	void Container::_maskPatches( PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip )
	{
		RectSPX coverage = m_skin.coverage(geo, m_scale, m_state);
		
		patches.sub( RectSPX::overlap(coverage,clip) );

		if( coverage.contains(_contentRect(geo)) )
			return;										// No need to loop through children, skins coverage contains them all.
		
		SlotWithGeo child;
		_firstSlotWithGeo( child );

		while(child.pSlot)
		{
			child.pSlot->_widget()->_maskPatches( patches, child.geo + geo.pos(), clip );
			_nextSlotWithGeo( child );
		}
	}

} // namespace wg
