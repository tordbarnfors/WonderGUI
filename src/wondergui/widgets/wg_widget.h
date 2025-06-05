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

#ifndef WG_WIDGET_DOT_H
#define WG_WIDGET_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_string.h>
#include <wg_receiver.h>
#include <wg_slotholder.h>
#include <wg_skinslot.h>
#include <wg_patches.h>
#include <wg_util.h>

#include <wg_base.h>

namespace wg
{
	class GfxDevice;
	class Container;
	class List;
	class Capsule;
	class Layer;
	class MsgRouter;
	class SlotHolder;
    class Component;

	class Widget;
	typedef	StrongPtr<Widget>	Widget_p;
	typedef	WeakPtr<Widget>		Widget_wp;

	class Container;
	typedef	StrongPtr<Container>	Container_p;
	typedef	WeakPtr<Container>	Container_wp;

	class Msg;
	typedef	StrongPtr<Msg>			Msg_p;
	typedef	WeakPtr<Msg>			Msg_wp;


	/**
	 * @brief Base class for widgets.
	 *
	 * Widget is the base class for all widgets, providing common functionality.
	 */

	class Widget : public Receiver, protected SkinSlot::Holder
	{
		friend class Container;
		friend class Component;
		friend class StaticSlot;
		friend class DynamicSlot;
		friend class Base;
		friend class InputHandler;
		friend class WidgetInfoPanel;

		template<class S> friend class StaticSlotVector;
		template<class S> friend class DynamicSlotVector;
		template<class S, int X> friend class SlotArray;

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
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = true;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Identification ________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		inline void			setId(int id);
		inline int			id() const;

		virtual bool		isContainer() const;

		//.____ Geometry ______________________________________________________

		inline int			scale() const;

		inline const Size	size() const;
		inline Rect			globalGeo() const;

		inline Coord		toGlobal( const Coord& coord ) const;
		inline Rect			toGlobal(const Rect& rect) const;
		inline Coord		toLocal( const Coord& coord ) const;
		inline Rect			toLocal(const Rect& rect) const;

		inline pts			matchingHeight( pts width ) const;
		inline pts			matchingWidth( pts height ) const;

		inline Size			defaultSize() const;
		inline Size			minSize() const;
		inline Size			maxSize() const;

		inline const SizeSPX& spxSize() const { return _size(); }
		inline RectSPX		spxGlobalGeo() const { return _globalGeo(); }

		//.____ Hierarchy _____________________________________________________

		inline Widget_p		nextSibling() const;
		inline Widget_p		prevSibling() const;
		Container_p			parent() const;

		void				releaseFromParent();

		bool				isDescendantOf(Widget* pAncestor);

		Widget_p			commonAncestor(Widget* pOtherWidget);

		//.____ State _________________________________________________________

		inline const State&	state() const;

		void				setDisabled(bool bEnabled);
		inline bool			isDisabled() const;

		void				setSelectable(bool bSelectable);
		inline bool			isSelectable() const;

//		virtual bool		setSelected( bool bSelected );
//		inline bool			isSelected();

		virtual bool		setChecked( bool bChecked );
		inline bool			isChecked();

		virtual bool		setFlagged( bool bFlagged );
		inline bool			isFlagged();

		bool				grabFocus(bool bRaiseWindow = false);
		bool				releaseFocus();
		bool				isFocused();


		//.____ Appearance ____________________________________________________

		virtual void		setSkin(Skin* pSkin);
		inline Skin_p		skin() const;

		inline void			setTooltip(const String& str);
		inline virtual String tooltip() const;

		inline void			setPointerStyle(PointerStyle style);
		virtual PointerStyle pointerStyle() const;

		void setShowToolTip(bool showToolTip);

		//.____ Behavior ______________________________________________________

		inline void			setMarkPolicy(MarkPolicy policy);
		inline MarkPolicy	markPolicy() const;

		inline void			setTabLock(bool bLock);
		inline bool			isTabLocked() const;

		void                setPickable( bool bPickable, uint8_t category = 0 );
		inline bool         isPickable() const;
		inline uint8_t      pickCategory() const;

		void                setPickHandle(bool bHandle);
		inline bool         isPickHandle() const;

		void                setDropTarget( bool bDropTarget );
		inline bool         isDropTarget() const;

		void				setStickyFocus(bool bSticky);
		inline bool			hasStickyFocus() const { return m_bStickyFocus; }


		//.____ Misc __________________________________________________________

		bool				markTest( const Coord& ofs );
		void 				receive( Msg * pMsg ) override final;

		void				setBaggage(Object * pBaggage);
		bool				hasBaggage() const;
		Object_p			baggage() const;

		void				bringIntoView( Border margin = 0);
		void				bringIntoView( Rect area, Border margin = 0 );

		Surface_p			screenshot( const Surface::Blueprint& bp = Surface::Blueprint() );
		Surface_p			screenshot(const Rect& rect, const Surface::Blueprint& bp = Surface::Blueprint() );

        inline bool         mouseOnWidget(const Coord& coord) const;

		//.____ Internal ______________________________________________________

		inline Container* _parent() const { return m_pHolder ? m_pHolder->_container() : nullptr; }
		inline Widget* _nextSibling() const { return m_pHolder ? m_pHolder->_nextChild(m_pSlot) : nullptr; }
		inline Widget* _prevSibling() const { return m_pHolder ? m_pHolder->_prevChild(m_pSlot) : nullptr; }

		virtual spx			_matchingHeight(spx width, int scale) const;
		virtual spx			_matchingWidth(spx height, int scale) const;

		virtual SizeSPX		_defaultSize(int scale) const;
		virtual SizeSPX		_minSize(int scale) const;
		virtual SizeSPX		_maxSize(int scale) const;

		inline const SizeSPX& _size() const;
		inline int			_scale() const override;


		inline RectSPX		_globalGeo() const;

		inline CoordSPX		_toGlobal(const CoordSPX& coord) const;
		inline RectSPX		_toGlobal(const RectSPX& rect) const;
		inline CoordSPX		_toLocal(const CoordSPX& coord) const;
		inline RectSPX		_toLocal(const RectSPX& rect) const;

		virtual	bool		_markTest(const CoordSPX& ofs);

		virtual void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip);

		virtual void    	_preRender();
		virtual void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window);

		virtual void		_resize(const SizeSPX& size, int scale);

		inline bool			_hasOverflow() { return m_bOverflow; }
		virtual BorderSPX	_overflow() const;
		virtual RectSPX		_renderBounds() const;

		virtual void		_setState(State state);

		virtual void		_receive(Msg* pMsg);
		virtual	bool		_alphaTest(const CoordSPX& ofs);

		virtual SizeSPX		_windowPadding() const;	// Padding of window before we get to (scrollable) content.

		StaticSlot* 		_slot() const { return m_pSlot; }

	protected:
		Widget();
		template<class BP> Widget( const BP& bp ) : m_skin(this)
		{
//			m_pContextAtConstruction = Base::context().rawPtr();

			m_pBaggage		= bp.baggage;
			m_bDropTarget	= bp.dropTarget;
			m_id			= bp.id;

			if( bp.markPolicy != MarkPolicy::Undefined )
				m_markPolicy = bp.markPolicy;

			m_bPickable		= bp.pickable;
			m_pickCategory	= bp.pickCategory;
			m_bPickHandle	= bp.pickHandle;
			m_pointerStyle	= bp.pointer;
			m_bSelectable	= bp.selectable;
			m_bStickyFocus	= bp.stickyFocus;
			m_bTabLock		= bp.tabLock;
			m_tooltip		= bp.tooltip;

			if( bp.finalizer )
				setFinalizer(bp.finalizer);

			if (bp.skin)
			{
				m_skin.set(bp.skin);
				m_bOverflow = bp.skin->_overflowsGeo();
			}

			if( bp.disabled )
				setDisabled(bp.disabled);
		}

		virtual ~Widget();

		void				_setSlot(StaticSlot * pSlot);
		SlotHolder *		_holder() { return m_pHolder; }
		const SlotHolder *	_holder() const { return m_pHolder; }

		int					_listAncestors(Widget* array[], int max);

		int64_t				_startReceiveUpdates();
		void				_stopReceiveUpdates();

		bool            	_requestPreRenderCall();

		inline RectSPX		_contentRect() const { return m_skin.contentRect(m_size, m_scale, m_state); }
		inline RectSPX		_contentRect(const RectSPX& canvas) const { return m_skin.contentRect(canvas, m_scale, m_state); }
		inline SizeSPX		_contentBorderSize(int scale) const { return m_skin.contentBorderSize(scale); }

		// Convenient calls to holder

		inline void			_overflowChanged( const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) { if( m_pHolder ) m_pHolder->_childOverflowChanged( m_pSlot, oldOverflow, newOverflow ); }

		inline void			_requestRender() { if( m_pHolder ) m_pHolder->_childRequestRender( m_pSlot, _renderBounds() ); }
		inline void			_requestRender( const RectSPX& rect ) { if( m_pHolder ) m_pHolder->_childRequestRender( m_pSlot, rect ); }
		inline void			_requestResize() { if( m_pHolder ) m_pHolder->_childRequestResize( m_pSlot ); }
		inline void			_requestInView() const { if( m_pHolder ) m_pHolder->_childRequestInView( m_pSlot ); }
		inline void			_requestInView( const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea ) const { if( m_pHolder ) m_pHolder->_childRequestInView( m_pSlot, mustHaveArea, niceToHaveArea ); }

		inline RectSPX		_windowSection() const { if( m_pHolder ) return m_pHolder->_childWindowSection( m_pSlot ); return RectSPX(); }


		// Methods for components to access

		virtual State		_componentState(const Component* pComponent) const;
		virtual CoordSPX	_componentPos( const Component * pComponent ) const;
		virtual SizeSPX		_componentSize( const Component * pComponent ) const;
		virtual RectSPX		_componentGeo( const Component * pComponent ) const;
		virtual CoordSPX	_globalComponentPos( const Component * pComponent ) const;
		virtual RectSPX		_globalComponentGeo( const Component * pComponent ) const;

		virtual CoordSPX	_componentToGlobal( const Component * pComponent, CoordSPX coord ) const;
		virtual CoordSPX	_componentToGlobal( const Component * pComponent, RectSPX rect ) const;

		virtual CoordSPX	_componentToLocal( const Component * pComponent, CoordSPX coord ) const;
		virtual CoordSPX	_componentToLocal( const Component * pComponent, RectSPX rect ) const;


		virtual void		_componentRequestRender( const Component * pComponent );
		virtual void		_componentRequestRender( const Component * pComponent, const RectSPX& rect );
		virtual void		_componentRequestResize( const Component * pComponent );

		virtual void		_componentRequestFocus( const Component * pComponent, bool bRaiseWindow );
		virtual void		_componentRequestInView( const Component * pComponent );
		virtual void		_componentRequestInView( const Component * pComponent, const RectSPX& mustHave, const RectSPX& niceToHave );

		virtual void		_receiveComponentNotif( Component * pComponent, ComponentNotif notification, int value, void * pData );

		// Methods for skin to access

		float			_skinValue(const SkinSlot* pSlot) const override;
		float			_skinValue2(const SkinSlot* pSlot) const override;

		State			_skinState(const SkinSlot* pSlot) const override;

		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;

		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;

//		GUIContext *	m_pContextAtConstruction = nullptr;

		Object_p		m_pBaggage;

		SkinSlot		m_skin;

		SlotHolder *	m_pHolder = nullptr;
		StaticSlot *	m_pSlot = nullptr;

		String			m_tooltip;
		bool 			m_showToolTip = true;

		int				m_id = 0;
		PointerStyle	m_pointerStyle = PointerStyle::Undefined;
		MarkPolicy		m_markPolicy = MarkPolicy::AlphaTest;
		State			m_state;						// Current state of widget.

		uint16_t		m_scale = 64;

		bool            m_bPickable = false;			// Set if this widget accepts to be the source of drag-n-drop operations.
		bool			m_bPickHandle = false;
		uint8_t         m_pickCategory = 0;				// Category of drag-n-drop operations. User defined.

		bool            m_bDropTarget = false;			// Set if this widget accepts to be the target of drag-n-drop operations.

		bool			m_bTabLock = false;				// If set, the widget prevents focus shifting away from it with tab.
		bool			m_bSelectable = true;			// Set if widget is allowed to be selected.
		uint8_t			m_receivingUpdateCounter = 0;	//
		bool			m_bPressed = false;				// Keeps track of pressed button when mouse leaves/re-enters widget.
		bool			m_bStickyFocus = false;			// Set if widget should keep keyboard focus when mouse button pressed outside it.
		bool			m_bOverflow = false;


		SizeSPX			m_size = { 256 * 64,256 * 64 };	// Current size of widget.
	};


	//____ Inline methods __________________________________________________________


	//____ setId() __________________________
	/**
	* @brief	Set a user specified ID for this widget.
	*
	* Sets a user specified ID, which can be read with id() and used to identify this
	* widget. It has no other purpose. It is not used internally and does not need to be unique.
	*
	* Default ID is 0.
	*/

	void Widget::setId(int id)
	{
		m_id = id;
	}

	//____ id() __________________________
	/**
	* @brief	Read the ID set through setId().
	*
	* Reads the user specified ID.
	*
	* @return	The ID earlier specified with setId() or 0.
	*/

	int Widget::id() const
	{
		return m_id;
	}

	//____ scale() _______________________________________
	/**
	 * @brief Get scale of widget.
	 *
	 * Get the scale of the widget.
	 *
	 * The scale value specifies the size of a point in subpixels (1/64th pixel),
	 * thus is 64 the default scale where one point equals one pixel. A scale
	 * value of 128 doubles the size of the widget (if allowed by the parent)
	 * and all content within, including fonts and graphics.
	 *
	 * @return Scale measured in subpixels per point.
	 */

	int Widget::scale() const
	{
		return m_scale;
	}

	//____ size() __________________________
	/**
	 * @brief	Get the size of of widget.
	 *
	 * Get the size (width and height) of widget.
	 *
	 * @return Size of widget.
	 */
	const Size Widget::size() const
	{
		return Util::spxToPts(_size(), m_scale);
	}

	//____ globalGeo() __________________________
	/**
	 * @brief	Get the global geometry of widget.
	 *
	 * Get position and size of widget in the global coordinate space.
	 * If the widget doesn't have a parent, it's position defaults to (0,0).
	 *
	 * @return Geometry of widget in the global coordinate space.
	 */
	Rect Widget::globalGeo() const
	{
		if( m_pHolder )
			return m_pHolder->_childLocalSpxToGlobalPts(m_pSlot, m_size);
		else
			return size();
	}

	//____ toGlobal() __________________________
	/**
	 * @brief Convert coordinate from local to global space
	 *
	 * Convert coordinate from widgets local coordinate space to the global coordinate space.
	 *
	 * @param coord		Coordinate in widgets local coordinate space.
	 *
	 * Please note that the widgets local coordinate space originates from the top-left
	 * corner of its geometry box and is NOT the same as the (parents) local coordinate
	 * space in which it lives.
	 * The coordinate (0,0) is always the top-left corner of the widget.
	 *
	 * @return Coordinate in gobal coordinate space.
	 */

	Coord Widget::toGlobal(const Coord& coord) const
	{
		if( m_pHolder )
			return m_pHolder->_childLocalSpxToGlobalPts(m_pSlot, Util::ptsToSpx(coord,m_scale));
		else
			return coord;
	}

	//____ toGlobal() __________________________
	/**
	 * @brief Convert rectangle from local to global space
	 *
	 * Convert rectangle from widgets local coordinate space to the global coordinate space.
	 *
	 * @param rect		Rectangle in widgets local coordinate space.
	 *
	 * Please note that the widgets local coordinate space originates from the top-left
	 * corner of its geometry box and is NOT the same as the (parents) local coordinate
	 * space in which it lives.
	 * The coordinate (0,0) is always the top-left corner of the widget.
	 *
	 * @return Rectangle in gobal coordinate space.
	 */

	Rect Widget::toGlobal(const Rect& rect) const
	{
		if( m_pHolder )
			return m_pHolder->_childLocalSpxToGlobalPts(m_pSlot, Util::ptsToSpx(rect,m_scale));
		else
			return rect;
	}

	//____ toLocal() ____________________________________________________________
	/**
	 * @brief Convert coordinate from global to local space
	 *
	 * Convert coordinate from the global coordinate space to widgets local coordinate space.
	 *
	 * @param coord		Coordinate in global coordinate space.
	 *
	 * Please note that the widgets local coordinate space originates from the top-left
	 * corner of its geometry box and is NOT the same as the (parents) local coordinate
	 * space in which it lives.
	 * The coordinate (0,0) is always the top-left corner of the widget.
	 *
	 * @return Coordinate in widgets local coordinate space.
	 */

	Coord Widget::toLocal(const Coord& coord) const
	{
		if( m_pHolder )
		{
			RectSPX localSPX = m_pHolder->_globalPtsToChildLocalSpx(m_pSlot, coord);
			return Util::spxToPts(localSPX.pos(), m_scale);
		}
		else
			return coord;
	}

	//____ toLocal() ____________________________________________________________
	/**
	 * @brief Convert rectangle from global to local space
	 *
	 * Convert rectangle from the global coordinate space to widgets local coordinate space.
	 *
	 * @param rect		Rectangle in global coordinate space.
	 *
	 * Please note that the widgets local coordinate space originates from the top-left
	 * corner of its geometry box and is NOT the same as the (parents) local coordinate
	 * space in which it lives.
	 * The coordinate (0,0) is always the top-left corner of the widget.
	 *
	 * @return Rectangle in widgets local coordinate space.
	 */

	Rect Widget::toLocal(const Rect& rect) const
	{
		if( m_pHolder )
		{
			RectSPX localSPX = m_pHolder->_globalPtsToChildLocalSpx(m_pSlot, rect);
			return Util::spxToPts(localSPX, m_scale);
		}
		else
			return rect;
	}

	//____ matchingHeight() _______________________________________________________
	/**
	 * @brief Get the height recommended by the widget for the specified width.
	 *
	 * Get the height wanted by the widget for the specified width.
	 *
	 * @param width		Width in points.
	 *
	 * This method is used by containers to get the recommended height of a widget for which
	 * it has already decided the width.
	 *
	 * @return The recommended height for the given width.
	 */

	pts Widget::matchingHeight( pts width ) const
	{
		return Util::spxToPts(_matchingHeight( Util::ptsToSpx(width, m_scale), m_scale ), m_scale);
	}

	//____ matchingWidth() _______________________________________________________
	/**
	 * @brief Get the width recommended by the widget for the specified height.
	 *
	 * Get the width wanted by the widget for the specified height.
	 *
	 * @param height	Height in points.
	 *
	 * This method is used by containers to get the recommended width of a widget for which
	 * it has already decided the height.
	 *
	 * @return The recommended width for the given height.
	 */

	pts Widget::matchingWidth( pts height ) const
	{
		return Util::spxToPts(_matchingWidth(Util::ptsToSpx(height, m_scale), m_scale), m_scale);		// Default is to stick with default height no matter what width.
	}

	//____ defaultSize() ________________________________________________________
	/**
	 * @brief Get the widgets default size.
	 *
	 * Get the widgets default size.
	 *
	 * Each widget has its own default size, which is depending on things such as
	 * skinning, content and (in the case of containers) size and layout of children.
	 *
	 * A container holding a widget will strive to provide the widget its default size, given
	 * the constraints and limitations the container needs to work with. If a container can't
	 * provide a widget its default size, it is likely to decide the closest width or height
	 * that it can provide and then make a second call to either matchingWidth() or matchingHeight()
	 * after which it will decide the size of the child.
	 *
	 * @return The default size of the widget.
	 */

	Size Widget::defaultSize() const
	{
		return Util::spxToPts(_defaultSize(m_scale), m_scale);
	}

	//____ minSize() ______________________________________________________________
	/**
	 * @brief Get the widgets recommended minimum size.
	 *
	 * Get the widgets recommended minimum size.
	 *
	 * Each widget has its own minimum size, which is depending on things such as
	 * skinning, content and (in the case of containers) size and layout of children.
	 *
	 * The minimum size is only a hint for the widgets parent, which should strive to not
	 * make a child smaller than its minimum size, but is allowed to set the child to
	 * any size it decides, including 0.0.
	 *
	 * @return The minimum size of the widget.
	 */

	Size Widget::minSize() const
	{
		return Util::spxToPts(_minSize(m_scale), m_scale);
	}

	//____ maxSize() ______________________________________________________________
	/**
	 * @brief Get the widgets recommended maximum size.
	 *
	 * Get the widgets recommended maximum size.
	 *
	 * Each widget has its own maximum size, which is depending on things such as
	 * skinning, content and (in the case of containers) size and layout of children.
	 *
	 * The maximum size is only a hint for the widgets parent, which should strive to not
	 * make a child larger than its maximum size, but is allowed to set the child to
	 * any reasonable size it decides.
	 *
	 * @return The maximum size of the widget.
	 */

	//____ maxSize() ______________________________________________________________

	Size Widget::maxSize() const
	{
		return Util::spxToPts(_maxSize(m_scale), m_scale);
	}

	//____ size() __________________________

	const SizeSPX& Widget::_size() const
	{
		return m_size;
	}

	//____ _scale() ___________________________________________________________

	int Widget::_scale() const
	{
		return m_scale;
	}

    //____ mouseOnWidget() ___________________________________________________________

    bool Widget::mouseOnWidget(const Coord& coord) const
    {
        const auto geo = Util::spxToPts(_globalGeo(), m_scale);
        return coord.x >= geo.x  && coord.x < geo.x + geo.w &&
               coord.y >= geo.y  && coord.y < geo.y + geo.h;
    }

	//____ _globalGeo() __________________________

	RectSPX Widget::_globalGeo() const
	{
		return  m_pHolder ? m_pHolder->_childLocalToGlobal(m_pSlot, m_size) : RectSPX(m_size);
	}

	//____ _toGlobal() __________________________

	CoordSPX Widget::_toGlobal(const CoordSPX& coord) const
	{
		return m_pHolder ? (CoordSPX) m_pHolder->_childLocalToGlobal(m_pSlot, coord) : coord;
	}

	RectSPX Widget::_toGlobal(const RectSPX& rect) const
	{
		return m_pHolder ? m_pHolder->_childLocalToGlobal(m_pSlot, rect) : rect;
	}

	//____ _toLocal() ____________________________________________________________

	CoordSPX Widget::_toLocal(const CoordSPX& coord) const
	{
		return m_pHolder ? (CoordSPX) m_pHolder->_globalToChildLocal(m_pSlot, coord) : coord;
	}

	RectSPX Widget::_toLocal(const RectSPX& rect) const
	{
		return m_pHolder ? m_pHolder->_globalToChildLocal(m_pSlot, rect) : rect;
	}

	//____ state() ____________________________________________________________
	/**
	 * @brief	Return the state of widget.
	 *
	 * Returns a reference to the widgets state object, containing basic state information
	 * such as if it is disabled, has mouse pointer inside its geometry, is pressed or selected.
	 *
	 * This is just for information. WonderGUI doesn't allow the state to be manipulated except
	 * as allowed by specific API:s, like setEnabled().
	 *
	 * @return Reference to state object representing the current state of the widget.
	 */

	const State& Widget::state() const
	{
		return m_state;
	}

	//____ isDisabled() ____________________________________________________________
	/**
	 * @brief	Get the enabled/disabled state of widget.
	 *
	 * @return 	True if widget is disabled, otherwise false.
	 */

	bool Widget::isDisabled() const
	{
		return m_state.isDisabled();
	}

	//____ isSelectable() ____________________________________________________________
	/**
	 * @brief	Get the selectable flag of widget.
	 *
	 * Get the selectable flag of widget, which determines if widget should be considered
	 * selectable when placed in lists and such.
	 *
	 * @return 	True if widget is selectable, otherwise false.
	 */

	bool Widget::isSelectable() const
	{
		return m_bSelectable;
	}

	//____ isChecked() __________________________________________________________

	inline bool Widget::isChecked()
	{
		return m_state.isChecked();
	};

	//____ isFlagged() __________________________________________________________

	inline bool Widget::isFlagged()
	{
		return m_state.isFlagged();
	};

	//____ nextSibling() ____________________________________________________________
	/**
	 * @brief Get next sibling.
	 *
	 * Get the widgets next sibling. Order of siblings is determined by parent
	 * but is generally either left to right, top to bottom or background to foreground.
	 *
	 * @return Pointer to next sibling or null if none.
	 */

	Widget_p Widget::nextSibling() const
	{
		if( m_pHolder )
			return m_pHolder->_nextChild( m_pSlot );
		return nullptr;
	}

	//____ prevSibling() ____________________________________________________________
	/**
	 * @brief Get previous sibling.
	 *
	 * Get the widgets previous sibling. Order of siblings is determined by parent
	 * but is generally either left to right, top to bottom or background to foreground.
	 *
	 * @return Pointer to previous sibling or null if none.
	 */

	Widget_p Widget::prevSibling() const
	{
		if( m_pHolder )
			return m_pHolder->_prevChild( m_pSlot );
		return 0;
	}

	//____ setTooltip() _______________________________________________________
	/**
	 * @brief Set widgets tooltip string.
	 *
	 * Sets a string that is used as the tooltip for the widget. A tooltip with this
	 * text will be displayed by the TooltipOverlay (if one is present in the hierarchy) when
	 * the mouse hovers over the widget.
	 *
	 * Note: A widget may override the tooltip set by this method and display another
	 * text, depending on the situation and exact position of the pointer.
	 *
	 */

	void Widget::setTooltip(const String& str)
	{
		m_tooltip = str;
	}



	//____ tooltip() __________________________________________________________
	/**
	 * @brief Get widgets tooltip string.
	 *
	 * Gets the string to be displayed in a tooltip when the mouse hovers over the widget.
	 *
	 * Note: This string might be different from what has been set using setTooltip().
	 * Some widgets can decide to display something else depending on the situation
	 * and exact position of the pointer.
	 *
	 * @return void
	 */

	String Widget::tooltip() const
	{
		if(m_showToolTip)
		{
			return m_tooltip;
		}
		return String("");
	}

	//____ skin() _____________________________________________________________

	Skin_p Widget::skin() const
	{
		return m_skin.get();
	}

	//____ setPointerStyle() __________________________________________________

	void Widget::setPointerStyle(PointerStyle style)
	{
		m_pointerStyle = style;
	}

	//____ setMarkPolicy() ___________________________________________________

	void Widget::setMarkPolicy(MarkPolicy policy)
	{
		m_markPolicy = policy;
	}

	//____ markPolicy() ______________________________________________________

	MarkPolicy Widget::markPolicy() const
	{
		return m_markPolicy;
	}

	//____ setTabLock() _______________________________________________________

	void Widget::setTabLock(bool bLock)
	{
		m_bTabLock = bLock;
	}

	//____ isTabLocked() ______________________________________________________

	bool Widget::isTabLocked() const
	{
		return m_bTabLock;
	}

	//____ isPickable() _______________________________________________________

	bool Widget::isPickable() const
	{
		return m_bPickable;
	}

	//____ pickCategory() _____________________________________________________

	uint8_t Widget::pickCategory() const 
	{ 
		return m_pickCategory; 
	}

	//____ isPickHandle() ____________________________________________________

	bool Widget::isPickHandle() const
	{
		return m_bPickHandle;
	}

	//____ isDropTarget() ____________________________________________________

	bool Widget::isDropTarget() const
	{
		return m_bDropTarget;
	}


} // namespace wg
#endif
