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
#ifndef	WG_DYNAMICTEXT_DOT_H
#define WG_DYNAMICTEXT_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_statictext.h>


#include <wg_base.h>

namespace wg
{


	//____ DynamicText __________________________________________________________________

	class DynamicText : public StaticText
	{
	public:

		//.____ Blueprint ______________________________________________________

		struct Blueprint
		{
			TextLayout_p	layout;
			TextStyle_p		style;
			std::string		text;
		};

		DynamicText(Widget * pWidget) : StaticText(pWidget) {}
		virtual ~DynamicText() {}

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Content _____________________________________________

		inline void			clear() { _clear(); }

		inline void			setText(std::string_view text) { _setText(text); }			///< Set UTF-8 encoded text, which may contain markup.

		inline int			append(std::string_view text) { return _append(text); }
		inline int			insert(int ofs, std::string_view text) { return _insert(ofs, text); }
		inline int			replace(int ofs, int nDelete, std::string_view text) { return _replace(ofs, nDelete, text); }
		inline int			erase(int ofs, int len) { return _erase(ofs, len); }

		//.____ Internal ______________________________________________________

		void				_initFromBlueprint(const Blueprint& blueprint);

	};


} // namespace wg
#endif //WG_DYNAMICTEXT_DOT_H
