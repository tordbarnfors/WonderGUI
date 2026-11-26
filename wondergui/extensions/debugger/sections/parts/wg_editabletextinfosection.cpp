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
#include "wg_editabletextinfosection.h"
#include <wg_enumextras.h>

namespace wg
{

	const TypeInfo EditableTextInfoSection::TYPEINFO = { "EditableTextInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	EditableTextInfoSection::EditableTextInfoSection(const DebugTheme& theme, IDebugContext* pContext, EditableText * pInspected)
		: TypedInfoSection<EditableText>( theme, pContext, EditableText::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow( "Edit mode: ", [](EditableText* t) { return toString(t->editMode()); } ),
			intRow ( "Max lines: ", [](EditableText* t) { return t->maxLines(); } ),
			intRow ( "Max chars: ", [](EditableText* t) { return t->maxChars(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& EditableTextInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
