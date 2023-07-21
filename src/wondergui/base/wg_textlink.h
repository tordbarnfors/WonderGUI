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

#ifndef WG_TEXTLINK_DOT_H
#define WG_TEXTLINK_DOT_H
#pragma once

#include <string>


#include <wg_pointers.h>
#include <wg_geo.h>

namespace wg
{
	class TextStyle;
	typedef	StrongPtr<TextStyle>		TextStyle_p;
	typedef	WeakPtr<TextStyle>		TextStyle_wp;

	class TextLink;
	typedef	StrongPtr<class TextLink> TextLink_p;
	typedef	WeakPtr<class TextLink> TextLink_wp;


	//____ TextLink _____________________________________________________________


	class TextLink : public Object
	{
	public:
		//.____ Creation __________________________________________

		static TextLink_p create( const std::string& link ) { return TextLink_p(new TextLink(link)); }
		static TextLink_p create( const std::string& link, TextStyle * pStyle ) { return TextLink_p(new TextLink(link, pStyle)); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc ___________________________________________________________

		const std::string&		link() const { return m_link; }
		TextStyle_p				style() const;

	private:
		TextLink( const std::string& link );
		TextLink( const std::string& link, TextStyle * style );
		~TextLink();

		std::string				m_link;
		TextStyle_p				m_pStyle;

	};


} // namespace wg
#endif //WG_TEXTLINK_DOT_H
