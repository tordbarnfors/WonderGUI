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
#include <functional>

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

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Finalizer_p				finalizer = nullptr;
			int						id = 0;
			std::string				link;
			std::string				tooltip;
		};


		//.____ Creation __________________________________________

		static TextLink_p create() { return TextLink_p(new TextLink()); }
		static TextLink_p create( const Blueprint& blueprint ) { return TextLink_p(new TextLink(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		inline int				id() const { return m_id; }

		//.____ Content ___________________________________________________________

		void					setLink(const std::string& link) { m_link = link; }
		const std::string&		link() const { return m_link; }

		void					setTooltip(const std::string& tooltip) { m_tooltip = tooltip; }	
		const std::string&		tooltip() const { return m_tooltip; }

	private:
		TextLink() {};
		TextLink( const Blueprint& bp );
		~TextLink();

		std::string				m_link;
		std::string				m_tooltip;
		int						m_id = 0;

	};


} // namespace wg
#endif //WG_TEXTLINK_DOT_H
