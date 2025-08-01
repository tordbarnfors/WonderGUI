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

#ifndef	WG_STATICTEXT_DOT_H
#define WG_STATICTEXT_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_textbase.h>


#include <wg_base.h>

namespace wg
{

	class String;
	class CharSeq;
	class CharBuffer;


	//____ StaticText __________________________________________________________________

	class StaticText : public TextBase
	{
	public:

		//.____ Blueprint ______________________________________________________

		struct Blueprint
		{
			TextLayout_p	layout;
			TextStyle_p		style;
		};

		StaticText(Widget * pWidget );


		//.____ Content _____________________________________________

		inline 	String			text() const { return TextBase::_getString(); }

		inline int				length() const { return m_charBuffer.length(); }
		inline bool				isEmpty() const { return length() == 0 ? true : false; }

		TextLink_p			markedLink() const;

		//.____ Internal ______________________________________________________

		void				_initFromBlueprint(const Blueprint& blueprint);

		virtual void		_receive( Msg * pMsg );

	protected:


	protected:
		TextLink_p			m_pMarkedLink;	// Character offset for beginning of marked or focused link
	};


} // namespace wg
#endif //WG_STATICTEXT_DOT_H
