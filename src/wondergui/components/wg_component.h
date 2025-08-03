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

#ifndef	WG_COMPONENT_DOT_H
#define	WG_COMPONENT_DOT_H
#pragma once

#include <wg_widget.h>

namespace wg
{

	//____ Component _________________________________________________________________

	class Component	/** @private */
{
public:
	
	Component( Widget * pWidget ) : m_pWidget(pWidget) {}
	
	//.____ Identification ________________________________________________

	virtual const TypeInfo& typeInfo(void) const;
	const static TypeInfo	TYPEINFO;

	//.____ Internal _______________________________________________________
	
	inline CoordSPX	_pos() const { return m_pWidget->_componentPos(this); }
	inline int		_scale() const { return m_pWidget->_scale(); }
	inline SizeSPX	_size() const { return m_pWidget->_componentSize(this); }
	inline RectSPX	_geo() const { return m_pWidget->_componentGeo(this); }
	inline CoordSPX _toGlobal(const CoordSPX& coord) const { return m_pWidget->_componentToGlobal(this, coord); }
	inline CoordSPX _toGlobal(const RectSPX& rect) const { return m_pWidget->_componentToGlobal(this, rect); }
	inline CoordSPX _toLocal(const CoordSPX& coord) const { return m_pWidget->_componentToLocal(this, coord); }
	inline CoordSPX _toLocal(const RectSPX& rect) const { return m_pWidget->_componentToLocal(this, rect); }

	inline RectSPX	_globalGeo() const { return m_pWidget->_globalComponentGeo(this); }
	inline Widget*	_widget() const { return m_pWidget;  }


	protected:

		inline void		_requestRender() const { m_pWidget->_componentRequestRender(this); }
		inline void		_requestRender(const RectSPX& rect) const { m_pWidget->_componentRequestRender(this, rect); }
		inline void		_requestResize() const { m_pWidget->_componentRequestResize(this); }
		inline void		_requestVisibility() const { m_pWidget->_componentRequestInView(this); }
		inline void		_requestVisibility(const RectSPX& mustHave, const RectSPX& niceToHave) const { m_pWidget->_componentRequestInView(this, mustHave, niceToHave); }
		inline void		_requestFocus(bool bRaiseWindow = false) const { m_pWidget->_componentRequestFocus(this, bRaiseWindow); }

		inline void		_notify(ComponentNotif notification, int value, void * pData) { m_pWidget->_receiveComponentNotif(this, notification, value, pData); }

	protected:
		Widget *		m_pWidget;
	};


} // namespace wg
#endif //WG_COMPONENT_DOT_H
