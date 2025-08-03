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

#ifndef	WG_TEXT_DOT_H
#define WG_TEXT_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_statictext.h>


#include <wg_base.h>

namespace wg
{

	class String;
	class CharSeq;
	class CharBuffer;


	//____ Text __________________________________________________________________

	class Text : public StaticText
	{
	public:

		//.____ Blueprint ______________________________________________________

		struct Blueprint
		{
			TextLayout_p	layout;
			TextStyle_p		style;
			String			text;
		};

		Text(Widget * pWidget) : StaticText(pWidget) {}
		virtual ~Text() {}

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Content _____________________________________________

		inline void			clear() { _clear(); }

		inline void			setText(const char * str) { _setText(CharSeq(str)); }
		inline void			setText(const CharSeq& seq) { _setText(seq); }
		inline void			setText(const CharBuffer * buffer) { _setText(buffer); }
		inline void			setText(const String& str) { _setText(str); }

		inline int			append(const CharSeq& seq) { return _append(seq); }
		inline int			insert(int ofs, const CharSeq& seq) { return _insert(ofs, seq); }
		inline int			replace(int ofs, int nDelete, const CharSeq& seq) { return _replace(ofs, nDelete, seq); }
		inline int			erase(int ofs, int len) { return _erase(ofs, len); }

		//.____ Appearance _____________________________________________

		inline void			setCharStyle(TextStyle * pStyle) { _setCharStyle(pStyle); }
		inline void			setCharStyle(TextStyle * pStyle, int ofs, int len) { _setCharStyle(pStyle, ofs, len); }

		inline void			clearCharStyle() { _clearCharStyle(); }
		inline void			clearCharStyle(int ofs, int len) { _clearCharStyle(ofs, len); }

		//.____ Internal ______________________________________________________

		void				_initFromBlueprint(const Blueprint& blueprint);

	};


} // namespace wg
#endif //WG_TEXT_DOT_H
