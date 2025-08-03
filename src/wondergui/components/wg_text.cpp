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


#include <wg_text.h>

namespace wg
{
	const TypeInfo	Text::TYPEINFO = { "Text", &StaticText::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Text::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ initFromBlueprint() ________________________________________________

	void Text::_initFromBlueprint(const Blueprint& bp)
	{
		if (bp.style)
			setStyle(bp.style);

		if (bp.layout)
			setLayout(bp.layout);

		if (!bp.text.isEmpty())
			_setText(bp.text);
	}
}