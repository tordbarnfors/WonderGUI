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
#ifndef	WG_PART_DOT_H
#define	WG_PART_DOT_H
#pragma once

#include <wg_widget.h>

namespace wg
{

	//____ Part ______________________________________________________________________

	class Part	/** @private */
{
public:
	
	Part( Widget * pWidget ) : m_pWidget(pWidget) {}
	
	//.____ Identification ________________________________________________

	virtual const TypeInfo& typeInfo(void) const;
	const static TypeInfo	TYPEINFO;

	//.____ Internal _______________________________________________________
	
	inline CoordSPX	_pos() const { return m_pWidget->_partPos(this); }
	inline int		_scale() const { return m_pWidget->_scale(); }
	inline SizeSPX	_size() const { return m_pWidget->_partSize(this); }
	inline RectSPX	_geo() const { return m_pWidget->_partGeo(this); }
	inline CoordSPX _toGlobal(const CoordSPX& coord) const { return m_pWidget->_partToGlobal(this, coord); }
	inline CoordSPX _toGlobal(const RectSPX& rect) const { return m_pWidget->_partToGlobal(this, rect); }
	inline CoordSPX _toLocal(const CoordSPX& coord) const { return m_pWidget->_partToLocal(this, coord); }
	inline CoordSPX _toLocal(const RectSPX& rect) const { return m_pWidget->_partToLocal(this, rect); }

	inline RectSPX	_globalGeo() const { return m_pWidget->_globalPartGeo(this); }
	inline Widget*	_widget() const { return m_pWidget;  }


	protected:

		inline void		_requestRender() const { m_pWidget->_partRequestRender(this); }
		inline void		_requestRender(const RectSPX& rect) const { m_pWidget->_partRequestRender(this, rect); }
		inline void		_requestResize() const { m_pWidget->_partRequestResize(this); }
		inline void		_requestVisibility() const { m_pWidget->_partRequestInView(this); }
		inline void		_requestVisibility(const RectSPX& mustHave, const RectSPX& niceToHave) const { m_pWidget->_partRequestInView(this, mustHave, niceToHave); }
		inline void		_requestFocus(bool bRaiseWindow = false) const { m_pWidget->_partRequestFocus(this, bRaiseWindow); }

		inline void		_notify(PartNotif notification, int value, void * pData) { m_pWidget->_receivePartNotif(this, notification, value, pData); }

	protected:
		Widget *		m_pWidget;
	};


} // namespace wg
#endif //WG_PART_DOT_H
