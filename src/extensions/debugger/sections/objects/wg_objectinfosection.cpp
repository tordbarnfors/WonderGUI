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
#include "wg_objectinfosection.h"


namespace wg
{

	const TypeInfo ObjectInfoSection::TYPEINFO = { "ObjectInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ObjectInfoSection::ObjectInfoSection(const DebugTheme& theme, IDebugContext* pContext, Object * pInspected)
		: TypedInfoSection<Object>( theme, pContext, Object::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			intRow    ( "Refcount: ",      [](Object* o) { return o->refcount(); } ),
			intRow    ( "Weak pointers: ", [](Object* o) { return o->weakPointers(); } ),
			pointerRow( "Finalizer: ",     [](Object* o) { return (void*) o->finalizer().rawPtr(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
