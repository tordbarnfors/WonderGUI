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
#ifndef	WG_ROOT_DOT_H
#define WG_ROOT_DOT_H
#pragma once

#include <wg_widget.h>

namespace wg
{
	class Root;
	typedef	StrongPtr<Root>		Root_p;
	typedef	WeakPtr<Root>		Root_wp;


	class Root : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		virtual int			scale() const = 0;
		virtual Rect		geo() const = 0;
				
		//.____ Appearance ____________________________________________________

		virtual PointerStyle pointerStyle() const = 0;
		
		//.____ Misc ___________________________________________________________
		
		virtual void		addDirtyPatch(const RectSPX& rect) = 0;
		virtual bool		addPreRenderCall(Widget* pWidget) = 0;

		virtual Widget_p	findWidget(const Coord& ofs, SearchMode mode) = 0;

		//.____ Internal _______________________________________________________
		
		virtual Widget *	_child() const = 0;
		virtual Widget *	_focusedChild() const = 0;

	};


} // namespace wg
#endif //WG_ROOT_DOT_H
