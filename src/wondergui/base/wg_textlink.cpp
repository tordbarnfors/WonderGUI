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
#include <wg_textlink.h>
#include <wg_textstyle.h>

namespace wg
{

	const TypeInfo TextLink::TYPEINFO = { "TextLink", &Object::TYPEINFO };

	//____ constructor _________________________________________________________

	TextLink::TextLink( const Blueprint& bp ) : m_link(bp.link), m_tooltip(bp.tooltip), m_id(bp.id)
	{
		if(bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ destructor __________________________________________________________

	TextLink::~TextLink()
	{
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextLink::typeInfo(void) const
	{
		return TYPEINFO;
	}



} // namespace wg
