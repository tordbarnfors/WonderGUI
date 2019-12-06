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

#include <wg_component.h>
#include <wg_strongcomponentptr.h>

namespace wg
{

	//____ ptr() _______________________________________________________________

	/** @brief Get a pointer to this interface.
	 *
	 *
	 */

	Component_p	Component::ptr()
	{
		return Component_p(this);
	}

	//____ object() _______________________________________________________________

	/** @brief Get a pointer to the object providing this interface.
	 *
	 *
	 */

	Object_p Component::object()
	{
		return Object_p( _object() );
	};


}
