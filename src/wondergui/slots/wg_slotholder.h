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
#ifndef WG_SLOTHOLDER_DOT_H
#define WG_SLOTHOLDER_DOT_H
#pragma once

#include <wg_geo.h>


namespace wg
{
	class Container;
	class Root;
	class Widget;
	class StaticSlot;
	class Object;
	struct TypeInfo;


	//____ SlotHolder ______________________________________________________________

	class SlotHolder		/** @private */
	{
	public:
		virtual Container * _container() = 0;
		virtual Root *		_root() = 0;
		virtual int			_scale() const = 0;

		virtual const TypeInfo& _slotTypeInfo(const StaticSlot * pSlot) const = 0;

		virtual RectSPX		_slotGeo( const StaticSlot * pSlot ) const = 0;
		virtual void		_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) = 0;
		
		virtual RectSPX		_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const = 0;
		virtual RectSPX		_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const = 0;

		virtual RectSPX		_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const = 0;
		virtual Rect		_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const = 0;

		virtual bool		_isChildVisible( const StaticSlot * pSlot ) const = 0;
		virtual RectSPX		_childWindowSection( const StaticSlot * pSlot ) const = 0;			// Returns the window section within the childs canvas.

		virtual void		_childRequestRender( StaticSlot * pSlot, const RectSPX& rect ) = 0;
		virtual void		_childRequestResize( StaticSlot * pSlot ) = 0;

		virtual bool		_childRequestFocus( StaticSlot * pSlot, Widget * pWidget, bool bRaiseWindow = false ) = 0;					// Request focus on behalf of me, child or grandchild.
		virtual bool		_childReleaseFocus( StaticSlot * pSlot, Widget * pWidget ) = 0;

		virtual void		_childRequestInView( StaticSlot * pSlot ) = 0;
		virtual void		_childRequestInView( StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea ) = 0;

		virtual Widget *	_prevChild( const StaticSlot * pSlot ) const = 0;
		virtual Widget *	_nextChild( const StaticSlot * pSlot ) const = 0;

		virtual void		_releaseChild(StaticSlot * pSlot) = 0;
		virtual void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild) = 0;

		virtual void		_didAddSlots(StaticSlot * pSlot, int nb) = 0;
		virtual void		_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) = 0;
		virtual void		_willEraseSlots(StaticSlot * pSlot, int nb) = 0;
	};

} // namespace wg
#endif //WG_SLOTHOLDER_DOT_H
