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

#include <wg_payload.h>

namespace wg
{
	const char BasicPayload::CLASSNAME[] = { "BasicPayload" };


	//____ isInstanceOf() _________________________________________________________

	bool BasicPayload::isInstanceOf(const char * pClassName) const
	{
		if (pClassName == CLASSNAME)
			return true;

		return Object::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * BasicPayload::className(void) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	BasicPayload_p BasicPayload::cast(Object * pObject)
	{
		if (pObject && pObject->isInstanceOf(CLASSNAME))
			return BasicPayload_p(static_cast<BasicPayload*>(pObject));

		return 0;
	}
}
